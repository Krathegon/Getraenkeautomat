/* 
 * File:   RFIDBoard.cpp
 * Author: fabian
 * 
 * Created on 9. August 2015, 19:47
 */

#include <string>
#include <iostream>
#include "RFIDBoard.h"

const int RFID_BAUDRATE = 9600;
const char RFID_INTERFACE[] = "ttyAMA";

RFIDBoard rfid(RFID_INTERFACE, RFID_BAUDRATE);

using namespace std;

RFIDBoard::RFIDBoard(string interface, int baud) : SerialDevice(interface, baud) {
    card.type = string("RFID");
}

Card RFIDBoard::getCard() {
    card.id.clear();
    
    while(dataAvailable()) {
        char byte = getChar();
        cout << byte;

        if(byte == CARRIAGE_RETURN) {
            continue;
        }
        if(byte == NEWLINE) {
            break;
        }

        card.id.push_back(byte);
    }
    flush();
    
    return card;
}

