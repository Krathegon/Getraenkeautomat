#ifndef _PROTOCOL_H
#define _PROTOCOL_H

struct Command {
    char first;
    char last;
};

constexpr Command CMD_OK          =   { 'O', 'K' };
constexpr Command CMD_ERROR       =   { 'X', 'X' };
constexpr Command CMD_TEST        =   { 'T', 'T' };
constexpr Command CMD_CARD_ACTIVE =   { 'C', 'A' };
constexpr Command CMD_NO_CARD     =   { 'N', 'C' };
constexpr Command CMD_MOTOR_FAIL  =   { 'M', 'F' };
constexpr Command CMD_SLOT        =   { 'S',  0  };
constexpr Command CMD_EMPTY       =   { 'E',  0  };

bool validCommand(const Command command);

bool sameCommand(const Command c1, const Command c2);

#endif /* _PROTOCOL_H */