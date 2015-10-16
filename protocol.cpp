/* 
 * File:   protocol.cpp
 * Author: fabian
 * 
 * Created on 13. August 2015, 22:33
 */

#include "protocol.h"
#include <iostream>

using namespace std;

bool validCommand(const Command command) {
    bool valid = false;
    
    switch(command.first) {
        case CMD_OK.first:
            if(command.last == CMD_OK.last) {
                valid = true;
            }
            break;
        case CMD_ERROR.first:
            if(command.last == CMD_ERROR.last) {
                valid = true;
            }
            break;
        case CMD_TEST.first:
            if(command.last == CMD_TEST.last) {
                valid = true;
            }
            break;
        case CMD_CARD_ACTIVE.first:
            if(command.last == CMD_CARD_ACTIVE.last) {
                valid = true;
            }
            break;
        case CMD_NO_CARD.first:
            if(command.last == CMD_NO_CARD.last) {
                valid = true;
            }
            break;
        case CMD_MOTOR_FAIL.first:
            if(command.last == CMD_MOTOR_FAIL.last) {
                valid = true;
            }
            break;
        case CMD_SLOT.first:
            if(command.last >= 1 && command.last <= 6) {
                valid = true;
            }
            break;
        case CMD_EMPTY.first:
            if(command.last >= 0 && command.last <= 32) {
                valid = true;
            }
            break;
    }
    
    return valid;
}

bool sameCommand(const Command c1, const Command c2) {
    if(c1.last == 0 || c2.last == 0) {
        return c1.first == c2.first;
    }
    return (c1.first == c2.first) && (c1.last == c2.last);
}