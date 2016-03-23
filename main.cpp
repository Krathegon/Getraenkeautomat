/* 
 * File:   main.cpp
 * Author: fabian
 *
 * Created on 5. August 2015, 14:03
 */

#include <cstdlib>
#include <iostream>
#include <thread>
#include <bitset>
#include <chrono>
#include <cerrno>
#include <wiringPi.h>
#include <cpr/cpr.h>
#include <json.hpp>

#include "NXP.h"
#include "SerialDevice.h"
#include "CWBoard.h"
#include "RFIDBoard.h"
#include "Card.h"

#define RFID_RESET      1
#define LED_RED         4
#define LED_GREEN       5
#define LED_ON          LOW
#define LED_OFF         HIGH

#define CARD_TIMEOUT    30

#define REST_API        "http://localhost:8080/api/"
#define CARDS_API       REST_API "cards/"
#define HISTORY_API     REST_API "history/create"

using namespace std;
using namespace cpr;
using json = nlohmann::json;

void initGPIO() {
    wiringPiSetup();

    // set modes of used GPIO pins
    pinMode(RFID_RESET, OUTPUT);
    pinMode(LED_RED, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);
    
    // turn off both LEDs
    digitalWrite(LED_RED, LED_OFF);
    digitalWrite(LED_GREEN, LED_OFF);
    
    // reset RFID board
    digitalWrite(RFID_RESET, HIGH);
    digitalWrite(RFID_RESET, LOW);
}

bool init() {
    initGPIO();
    
    // init serial devices
    if(!cwBoard.open()) {
        cout << "Can't connect to CW-Board: " << strerror(errno) << endl;
        return false;
    }
    
    if(!rfid.open()) {
        cout << "Can't connect to RFID-Board: " << strerror(errno) << endl;
        return false;
    }
    
    if(!nxp.init()) {
        cout << "Can't connect to NXP Board!" << endl;
       return false; 
    }
    
    // first of all check if cw board is reachable
    if(!cwBoard.sendActiveCommand(CMD_TEST)) {
        cout << "There's a problem with the UART-connection of the CW-Board!" << endl;
        return false;
    }
    
    cout << "Init finished!" << endl;
    return true;
}

void switchLED(int led, int state) {
    // if led isn't supported, do nothing
    if(!(led == LED_RED || led == LED_GREEN))
        return;
    
    digitalWrite(led, state);
}

bool validCard(Card &card) {
    cout << "Checking if card with ID " << card.id << " and type " << card.type << " is valid ..." << endl;
    
    Response r = Get( Url{CARDS_API + card.id} );
    
    if(r.status_code == 404) {
        cout << "Card " << card.id << " not found, saving in DB..." << endl;
        r = Put( Url{CARDS_API + card.id}, Parameters{{"type", card.type}} );
        
        // TODO: handle response failure
        if(r.status_code != 201) {
            cout << "Couldn't save Card, Reason: " << r.text << endl;
        }
        
        return false;
    }
    
    cout << "Parsing JSON returned from server " << CARDS_API << " ..." << endl;
    json responseCard = json::parse(r.text);
    
    cout << "JSON parsed, extracting user ..." << endl;
    json user = responseCard["user"];
    
    if(user.empty()) {
        cout << "Card " << card.id << " has no user!" << endl;
        return false;
    }
    
    cout << "Card " << card.id << " responds to user " << user["firstname"] << " " << user["lastname"] << "." << endl;
    return true;
}

void setScannedTime(Card &card) {
    card.scanned = chrono::system_clock::now();
}

bool getCard(Card &card) {
    // check NFC
    if(nxp.detectCard()) {
        card = nxp.getCard();
        setScannedTime(card);
        
        cout << "NXP Card found: " << "ID->" << card.id << "  Type->" << card.type << endl; 
        return true;
    }

    // check RFID
    if(rfid.dataAvailable()) {
        card = rfid.getCard();
        setScannedTime(card);
        
        cout << "RFID Card found: " << "ID->" << card.id << "  Type->" << card.type << endl; 
        return true;
    }
    
    return false;
}

void cardError() {
    // let the red LED blink 3 times every 200ms
    cout << "Card Error!" << endl;
    for(int i=0; i<6; i++) {
        switchLED(LED_RED, i%2);
        this_thread::sleep_for(chrono::milliseconds(200));
    }
}

void cardTimeout() {
    cout << "Card Timeout! (LEDs off)" << endl;
    cwBoard.sendActiveCommand(CMD_NO_CARD);

    // deactivate LED
    switchLED(LED_GREEN, LED_OFF);
}

void processSelection(Card &card, char slot) {
    cout << "Selection: " << to_string(slot) << endl;
    
    Response r = Post( Url{HISTORY_API}, Payload{{"card_id", card.id},{"slot", to_string(slot)}} );
    
    // TODO: handle response failure
    if(r.status_code != 201) {
        cout << "Couldn't save History, Reason: " << r.text << endl;
    } else {
        cout << "Answer: " << r.text << endl;
    }
}

void motorFail() {
    // TODO: handle motor failure
    cout << "Motor Failure!" << endl;
}

void handleEmpty(unsigned int emptySlots) {
    // TODO: safe empty slots
    cout << "Empty: " << bitset<6>(emptySlots) << endl;
}

int main(int argc, char **argv)
{
    Card card, oldCard;
    bool active = false;
    
    if(!init()) {
        exit(EXIT_FAILURE);
    }

    while(1)
    {
        if(!active && getCard(card)) {
            if(!card.equals(oldCard)) {
                // now check if valid
                if(validCard(card)) {
                    if(cwBoard.sendActiveCommand(CMD_CARD_ACTIVE)) {
                        switchLED(LED_GREEN, LED_ON);
                        active = true;
                    }
                    else {
                        cout << "Retry limit has been reached." << endl;
                    }
                } else {
                    cardError();
                }
                
                oldCard = card;
            }
        } else {
            oldCard={};
        }
        // check commands from CW-Board
        if(cwBoard.dataAvailable()) {
            Command retCommand = cwBoard.getCommand();

            if(sameCommand(retCommand, CMD_SLOT)) {
                if(active) {
                    processSelection(card, retCommand.last);

                    // deactivate LED
                    switchLED(LED_GREEN, LED_OFF);
                    active = false;
                } else {
                    cout << "Command not allowed: " << retCommand.first << retCommand.last << endl;
                    cwBoard.putCommand(CMD_ERROR);
                }
            }
            else if(sameCommand(retCommand, CMD_MOTOR_FAIL)) {
                motorFail();
            }
            else if(sameCommand(retCommand, CMD_EMPTY)) {
                handleEmpty(retCommand.last);
            }
            else {
                cout << "Couldn't understand Command: " << retCommand.first << retCommand.last << endl;
                cwBoard.putCommand(CMD_ERROR);
            }
        }
            
        chrono::duration<double> diff = chrono::system_clock::now()-card.scanned;

        if(active && (diff.count() > CARD_TIMEOUT)) {
            cardTimeout();
            active = false;
        }
        
        this_thread::sleep_for(chrono::milliseconds(200));
    }
    
    return 0;
}