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

Card::Card(Card &card) {
    this->id = card.id;
    this->type = card.type;
    this->scanned = card.scanned;
}

Card::Card(string id, string type) {
    this->id = id;
    this->type = type;
}

bool Card::equals(Card &card) {
    return (card.id == this->id) && (card.type == this->type);
}