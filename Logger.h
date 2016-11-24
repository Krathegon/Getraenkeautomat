/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Logger.h
 * Author: fabian
 *
 * Created on October 13, 2016, 7:24 PM
 */

#ifndef LOGGER_H
#define LOGGER_H

#include <log4cpp/Category.hh>
#include <log4cpp/PropertyConfigurator.hh>
#include <cstring>

#define PROPERTIES_FILE     "/home/pi/Getraenkeautomat/build/log4cpp.properties"

class Logger {
public:
    Logger();
    log4cpp::Category& getLogger();
};

extern Logger logger;

#endif /* LOGGER_H */

