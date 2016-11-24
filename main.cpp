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
#include "Logger.h"

#define RFID_RESET      1
#define LED_RED         4
#define LED_GREEN       5
#define LED_ON          LOW
#define LED_OFF         HIGH

#define CARD_TIMEOUT    10

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
    digitalWrite(RFID_RESET, LOW);
    this_thread::sleep_for(chrono::milliseconds(100));
    digitalWrite(RFID_RESET, HIGH);
}

bool init() {
    initGPIO();
    
    log4cpp::Category& root = logger.getLogger();
    
    // init serial devices
    if(!cwBoard.open()) {
        root.error(string("Can't connect to CW-Board: ").append(strerror(errno)));
        return false;
    }
    
    if(!rfid.open()) {
        root.error(string("Can't connect to RFID-Board: ").append(strerror(errno)));
        return false;
    }
    
    if(!nxp.init()) {
        root.error("Can't connect to NXP Board: ");
        return false; 
    }
    
    // first of all check if cw board is reachable
    if(!cwBoard.sendActiveCommand(CMD_TEST)) {
        root.error("There's a problem with the UART-connection of the CW-Board!");
        return false;
    }
    
    root.info("Init finished!");
    return true;
}

void switchLED(int led, int state) {
    // if led isn't supported, do nothing
    if(!(led == LED_RED || led == LED_GREEN))
        return;
    
    digitalWrite(led, state);
}

bool validCard(Card &card) {
    log4cpp::Category& root = logger.getLogger();
    
    root.info(string("Checking if card with ID ").append(card.id).append(" and type ")
            .append(card.type).append(" is valid ..."));
    
    Response r = Get( Url{CARDS_API + card.id} );
    
    if(r.error) {
        root.error(string("Couldn't connect to Jetty: ").append(r.error.message));
        return false;
    }
    
    if(r.status_code == 404) {
        root.info(string("Card ").append(card.id).append(" not found, saving in DB..."));
        r = Put( Url{CARDS_API + card.id}, Parameters{{"type", card.type}} );
        
        // TODO: handle response failure
        if(r.status_code != 201) {
            root.error(string("Couldn't save Card, Reason: ").append(r.text));
        }
        
        return false;
    }
    
    root.info(string("Parsing JSON returned from server ").append(CARDS_API).append(" ..."));
    json responseCard = json::parse(r.text);
    
    root.info("JSON parsed, extracting user ...");
    json user = responseCard["user"];
    
    if(user.empty()) {
        root.error(string("Card ").append(card.id).append(" has no user!"));
        return false;
    }
    
    root.info(string("Card ").append(card.id).append(" responds to user ")
            .append(user["firstname"]).append(" ").append(user["lastname"]).append("."));
    return true;
}

void setScannedTime(Card &card) {
    card.scanned = chrono::system_clock::now();
}

bool getCard(Card &card) {
    log4cpp::Category& root = logger.getLogger();
    
    // check NFC
    if(nxp.detectCard()) {
        card = nxp.getCard();
        setScannedTime(card);
        
        root.info(string("NXP Card found: ID->").append(card.id).append("  Type->").append(card.type));
        return true;
    }

    // check RFID
    if(rfid.dataAvailable()) {
        card = rfid.getCard();
        setScannedTime(card);
        
        root.info(string("RFID Card found: ID->").append(card.id).append("  Type->").append(card.type));
        return true;
    }
    
    return false;
}

void cardError() {
    log4cpp::Category& root = logger.getLogger();
    
    root.info("Card Error!");
    
    // let the red LED blink 3 times every 200ms
    for(int i=0; i<6; i++) {
        switchLED(LED_RED, i%2);
        this_thread::sleep_for(chrono::milliseconds(200));
    }
}

void cardTimeout() {
    log4cpp::Category& root = logger.getLogger();
    
    root.info("Card Timeout! (LEDs off)");
    cwBoard.sendActiveCommand(CMD_NO_CARD);

    // deactivate LED
    switchLED(LED_GREEN, LED_OFF);
}

void processSelection(Card &card, char slot) {
    log4cpp::Category& root = logger.getLogger();
    
    root.info(string("Selection: ").append(to_string(slot)));
    
    Response r = Post( Url{HISTORY_API}, Payload{{"card_id", card.id},{"slot", to_string(slot)}} );
    
    // TODO: handle response failure
    if(r.status_code != 201) {
        root.error(string("Couldn't save History, Reason: ").append(r.text));
    }
}

void motorFail() {
    // TODO: handle motor failure
    log4cpp::Category& root = logger.getLogger();
    
    root.error("Motor Failure!");
}

void handleEmpty(unsigned int emptySlots) {
    // TODO: safe empty slots
    log4cpp::Category& root = logger.getLogger();
    
    root.info(string("Empty: ").append(bitset<6>(emptySlots).to_string()));
}

int main(int argc, char **argv)
{
    Card card, oldCard;
    bool active = false;
    
    log4cpp::Category& root = logger.getLogger();
    
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
                        root.error("Retry limit for command CARD_ACTIVE has been reached.");
                    }
                } else {
                    cardError();
                }
                
                oldCard = card;
            }
        } else {
            oldCard={};
            rfid.flush();
        }
        // check commands from CW-Board
        if(cwBoard.dataAvailable()) {
            Command retCommand = cwBoard.getCommand();

            if(sameCommand(retCommand, CMD_SLOT)) {
                processSelection(card, retCommand.last);

                // deactivate LED
                switchLED(LED_GREEN, LED_OFF);
                active = false;
            }
            else if(sameCommand(retCommand, CMD_MOTOR_FAIL)) {
                motorFail();
            }
            else if(sameCommand(retCommand, CMD_EMPTY)) {
                handleEmpty(retCommand.last);
            }
            else {
                root.error(string("Couldn't understand Command: ")
                    .append(to_string(retCommand.first)).append(to_string(retCommand.last)));
                
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