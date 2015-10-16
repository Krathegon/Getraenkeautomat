/* 
 * File:   CWBoard.cpp
 * Author: fabian
 * 
 * Created on 9. August 2015, 21:33
 */

#include "CWBoard.h"

const int CW_BAUDRATE  = 9600;
const char CW_DEVICE[] = "/dev/ttyUSB0";

CWBoard cwBoard(CW_DEVICE, CW_BAUDRATE);

using namespace std;

void CWBoard::putCommand(const Command command) {
    putChar(command.first);
    putChar(command.last);
    putChar(NEWLINE);
}

bool CWBoard::sendActiveCommand(Command command) {
    Command retCommand;
    bool success = false;
    
    if(sameCommand(command, CMD_ERROR))
        return false;

    for(int i=0; i<MAX_RETRY; i++) {
        cwBoard.putCommand(command);

        this_thread::sleep_for(chrono::milliseconds(50));
        retCommand = cwBoard.getCommand();

        if(sameCommand(retCommand, CMD_OK)) {
            success = true;
            break;
        }

        cout << "Resending command..." << endl;
    }
    
    return success;
}

Command CWBoard::getCommand() {
    Command command = {};
    
    if(dataAvailable()) {
        char byte;
        
        command.first = getChar();
        command.last = getChar();
        
        // if next char isn't NEWLINE, clear command to force resending
        if(getChar() != NEWLINE) {
            command.first = 0;
            command.last = 0;
        }
    }
    
    return command;
}
