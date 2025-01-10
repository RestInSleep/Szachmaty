//
// Created by Jan Jagodziński on 04/01/2025.
//

#ifndef UNTITLED24_BOARD_EXCEPTIONS_H
#define UNTITLED24_BOARD_EXCEPTIONS_H

#include <string>
#include "board.h"


#include <exception>
#include <string>

class NoPieceAtPositionException : public std::exception {
private:
    std::string message;

public:
    explicit NoPieceAtPositionException(const std::string& position)
            : message("No piece at position: " + position) {}

    [[nodiscard]] const char* what() const noexcept override;
};

class NotYourTurnException : public std::exception {
private:
    std::string message;

public:
    explicit NotYourTurnException(Color turn)
            : message(std::string("Not your turn: ") + ((turn == WHITE) ? "WHITE" : "BLACK")) {}

    [[nodiscard]] const char* what() const noexcept override;
};

class InvalidMoveException : public std::exception {
private:
    std::string message;

public:
    explicit InvalidMoveException(const std::string& move)
            : message("Invalid move: " + move) {}

    [[nodiscard]] const char* what() const noexcept override;
};
#endif //UNTITLED24_BOARD_EXCEPTIONS_H
