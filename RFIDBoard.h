/* 
 * File:   RFIDBoard.h
 * Author: fabian
 *
 * Created on 9. August 2015, 19:47
 */

#ifndef RFIDBOARD_H
#define	RFIDBOARD_H

#include "SerialDevice.h"
#include "Card.h"

class RFIDBoard : public SerialDevice {
public:
    RFIDBoard(string device, int baud);
    Card getCard(); 
private:
    Card card;
};

extern RFIDBoard rfid;

#endif	/* RFIDBOARD_H */

