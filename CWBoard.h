/* 
 * File:   CWBoard.h
 * Author: fabian
 *
 * Created on 9. August 2015, 21:33
 */

#ifndef CWBOARD_H
#define	CWBOARD_H

#include "protocol.h"
#include "SerialDevice.h"
#include <string>
#include <chrono>
#include <iostream>
#include <thread>

#define MAX_RETRY       5

using namespace std;

class CWBoard : public SerialDevice {
public:
    CWBoard(string device, int baud) : SerialDevice(device, baud) {};
    void putCommand(const Command command);
    bool sendActiveCommand(Command command);
    Command getCommand();
private:
};

extern CWBoard cwBoard;

#endif	/* CWBOARD_H */

