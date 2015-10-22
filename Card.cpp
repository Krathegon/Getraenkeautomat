/* 
 * File:   Card.cpp
 * Author: fabian
 * 
 * Created on 9. August 2015, 20:16
 */

#include "Card.h"

Card::Card() {
    id = "";
    type = "";
}

Card::Card(string id, string type) {
    this->id = id;
    this->type = type;
}