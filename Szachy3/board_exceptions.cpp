//
// Created by Jan Jagodziński on 04/01/2025.
//

#include "board_exceptions.h"


const char* NoPieceAtPositionException::what() const noexcept {
    return message.c_str();
}

const char* NotYourTurnException::what() const noexcept {
    return message.c_str();
}

const char* InvalidMoveException::what() const noexcept {
    return message.c_str();
}