/* 
 * File:   SerialDevice.cpp
 * Author: fabian
 * 
 * Created on 6. August 2015, 18:42
 */

#include "SerialDevice.h"

#include <wiringSerial.h>

SerialDevice::SerialDevice(string device, int baud) {
    this->device = device;
    
    this->baud = baud;
    this->fileDescriptor = -1;
}
 
SerialDevice::~SerialDevice() {
    if(fileDescriptor != -1)
        close();
}


bool SerialDevice::open() {
    fileDescriptor = serialOpen(device.c_str(), baud);
    
    if(fileDescriptor == -1)
        return false;
    return true;
}

void SerialDevice::close() {
    if(fileDescriptor != -1)
        serialClose(fileDescriptor) ;
}

void SerialDevice::putChar(unsigned char c) {
    serialPutchar(fileDescriptor, c) ;
}

void SerialDevice::putString(string s) {
    serialPuts(fileDescriptor, s.c_str()) ;
}

bool SerialDevice::dataAvailable() {
    return serialDataAvail(fileDescriptor) ;
}

char SerialDevice::getChar() {
    return serialGetchar(fileDescriptor) ;
}

void SerialDevice::flush() {
    serialFlush(fileDescriptor) ;
}