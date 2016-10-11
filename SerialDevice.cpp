/* 
 * File:   SerialDevice.cpp
 * Author: fabian
 * 
 * Created on 6. August 2015, 18:42
 */

#include "SerialDevice.h"

#include <wiringSerial.h>
#include <dirent.h>
#include <iostream>
#include <cstring>
#include <chrono>
#include <cerrno>

#define TTY_FOLDER      "/sys/class/tty/"

SerialDevice::SerialDevice(string interface, int baud) {
    this->baud = baud;
    this->interface = interface;
    this->fileDescriptor = -1;
    
    setDevice();
}
 
SerialDevice::~SerialDevice() {
    if(fileDescriptor != -1)
        close();
}

void SerialDevice::setDevice() {
    DIR *dir;
    struct dirent *dirent;
    
    if((dir  = opendir(TTY_FOLDER)) == NULL) {
        cout << "Error opening " << TTY_FOLDER << ": " << strerror(errno) << endl;
        // TODO: vernünftige errno
        exit(-1);
    }

    while ((dirent = readdir(dir)) != NULL) {
        string dev = string(dirent->d_name);
        if(dev.find(interface, 0) != dev.npos) {
            device = "/dev/" + dev;
            cout << "Found device " << device << endl;
            break;
        }
    }
    // TODO: error if no device found!
    
    closedir(dir);
}

void SerialDevice::tryReconnect() {
    setDevice();
    open();
}

bool SerialDevice::open() {
    fileDescriptor = serialOpen(device.c_str(), baud);
    
    if(fileDescriptor == -1)
        return false;
    return true;
}

void SerialDevice::close() {
    if(fileDescriptor != -1)
        serialClose(fileDescriptor);
}

void SerialDevice::putChar(unsigned char c) {
    serialPutchar(fileDescriptor, c);
}

void SerialDevice::putString(string s) {
    serialPuts(fileDescriptor, s.c_str());
}

bool SerialDevice::dataAvailable() {
    bool available = serialDataAvail(fileDescriptor);
    
    if(available == -1) {
        tryReconnect();
        return false;
    }
    
    return available;
}

char SerialDevice::getChar() {
    return serialGetchar(fileDescriptor);
}

void SerialDevice::flush() {
    serialFlush(fileDescriptor);
}