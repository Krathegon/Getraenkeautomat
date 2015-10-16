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
const char RFID_DEVICE[] = "/dev/ttyAMA0";

RFIDBoard rfid(RFID_DEVICE, RFID_BAUDRATE);

using namespace std;

RFIDBoard::RFIDBoard(string device, int baud) : SerialDevice(device, baud) {
    card.type = string("RFID");
}

Card RFIDBoard::getCard() {
    card.id.clear();
    
    if(dataAvailable()) {
        char byte;
        
        while(dataAvailable()) {
            byte = getChar();
            
            if(byte == CARRIAGE_RETURN) {
                continue;
            }
            if(byte == NEWLINE) {
                break;
            }
            
            card.id.push_back(byte);
        }
    }
    
    return card;
}

