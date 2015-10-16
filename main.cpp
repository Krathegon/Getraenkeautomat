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

using namespace std;

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
}

void switchLED(int led, int state) {
    // if led isn't supported, do nothing
    if(!(led == LED_RED || led == LED_GREEN))
        return;
    
    digitalWrite(led, state);
}

bool validCard(Card &card) {
    // TODO: check if card is valid (via web app)
    
    return true;
}

bool getCard(Card &card) {
    // check NFC
    if(nxp.detectCard()) {
        card = nxp.getCard();
        cout << "NXP Card found: " << "ID->" << card.id << "  Type->" << card.type << endl; 
        return true;
    }

    // check RFID
    if(rfid.dataAvailable()) {
        card = rfid.getCard();
        cout << "RFID Card found: " << "ID->" << card.id << "  Type->" << card.type << endl; 
        return true;
    }
    
    return false;
}

void cardError() {
    // let the red LED blink 3 times every 200ms
    for(int i=0; i<6; i++) {
        switchLED(LED_RED, i%2);
        this_thread::sleep_for(chrono::milliseconds(200));
    }
}

void setScannedTime(Card &card) {
    card.scanned = chrono::system_clock::now();
}

void cardTimeout() {
    cout << "Card Timeout! (LEDs off)" << endl;
    cwBoard.sendActiveCommand(CMD_NO_CARD);

    // deactivate LED
    switchLED(LED_GREEN, LED_OFF);
}

void processSelection(Command &retCommand) {
    // TODO: safe selection via REST
    cout << "Selection: " << retCommand.first << char(retCommand.last+'0') << endl;
}

void motorFail() {
    // TODO: handle motor failure
    cout << "Motor Failure!" << endl;
}

void handleEmpty(Command &retCommand) {
    // TODO: safe empty slots
    cout << "Empty: " << retCommand.first << bitset<8>(retCommand.last) << endl;
}

int main(int argc, char **argv)
{
    Card card;
    bool active = false;
    
    init();

    while(1)
    {
        if(getCard(card)) {
            // now check if valid
            if(validCard(card)) {
                setScannedTime(card);
                
                if(!active) {
                    if(cwBoard.sendActiveCommand(CMD_CARD_ACTIVE)) {
                        switchLED(LED_GREEN, LED_ON);
                        active = true;
                    }
                    else {
                        cout << "Retry limit has been reached." << endl;
                    }
                }
            } else if(!active) {
                cardError();
            }
        }
        // check commands from CW-Board
        if(cwBoard.dataAvailable()) {
            Command retCommand = cwBoard.getCommand();

            if(sameCommand(retCommand, CMD_SLOT)) {
                if(active) {
                    processSelection(retCommand);

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
                handleEmpty(retCommand);
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
    }
    
    return 0;
}