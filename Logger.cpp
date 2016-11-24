/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Logger.cpp
 * Author: fabian
 * 
 * Created on October 13, 2016, 7:25 PM
 */

#include "Logger.h"

using namespace log4cpp;

Logger logger;

Logger::Logger() {
    PropertyConfigurator::configure(std::string(PROPERTIES_FILE));
}

Category& Logger::getLogger() {
    return Category::getRoot();
}