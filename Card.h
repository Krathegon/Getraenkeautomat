/* 
 * File:   Card.h
 * Author: fabian
 *
 * Created on 9. August 2015, 20:16
 */
 
#ifndef CARD_H
#define	CARD_H

#include <string>
#include <chrono>

using namespace std;

class Card {
public:
    Card();
    Card(string id, string type);
    ~Card();
    
    string id;
    string type;
    std::chrono::system_clock::time_point scanned;
};

#endif	/* CARD_H */
