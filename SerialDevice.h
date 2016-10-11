/* 
 * File:   SerialDevice.h
 * Author: fabian
 *
 * Created on 6. August 2015, 18:42
 */

#ifndef SERIALDEVICE_H
#define	SERIALDEVICE_H
#include <string>

#define CARRIAGE_RETURN '\r'
#define NEWLINE '\n'

using namespace std;

class SerialDevice {
public:
    SerialDevice(string interface, int baud);
    virtual ~SerialDevice();
    bool open();
    void close();
    void tryReconnect();
    void setDevice();
    void putChar(unsigned char c);
    void putString(string s);
    bool dataAvailable(); 
    char getChar();
    void flush();
    
    string interface;
    string device;
    int baud;
    
private:
    int fileDescriptor;
};

#endif	/* SERIALDEVICE_H */