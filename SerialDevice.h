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
    SerialDevice(string device, int baud);
    virtual ~SerialDevice();
    bool open();
    void close();
    void putChar(unsigned char c);
    void putString(string s);
    bool dataAvailable(); 
    char getChar();
    void flush();
    
private:
    string device;
    int baud;
    int fileDescriptor;
};

#endif	/* SERIALDEVICE_H */