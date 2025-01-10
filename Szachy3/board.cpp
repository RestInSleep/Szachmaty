//
// Created by Jan Jagodziński on 30/12/2024.
//

#include <cassert>
#include <stdexcept>
#include "board.h"
#include "board_exceptions.h"
#include <string_view>
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
using std::invalid_argument;
using std::vector;

int Board::getPositionIndex(int row, int column) {
    return row * board_width + column;
}

pair<int, int> Board::getPosition(int index) {
    return {index / board_width, index % board_width};
}

string Board::getNotation(int index) {
    auto [row, column] = getPosition(index);
    return string(1, 'A' + column) + std::to_string(1 + row);
}

int Board::indexFromNotation(string_view notation) {
    char column = notation[0];
    if (column >= 'a' && column <= 'z') {
        column = std::toupper(column);
    }
    if (column < 'A' || column > 'A' + board_width - 1) {
        throw invalid_argument("Invalid column!");
    }
    string row_string = { notation.begin() + 1, notation.end()};
    int row = std::stoi(row_string) - 1;
    return row * board_width + (column - 'A');
}


pair<int, int> Piece::getPosition() const {
    return {row, column};
}

void Board::init() {
    // We assume, that first row on players side is
    // determined by FIGURE_ROW, while the second
    // is filled with pawns

    int count = 0;
    // assert exactly one king is present
    for (int col{0}; col < board_width; ++col) {
        if (figures_format[col] == 'K') {
            ++count;
        }
    }
    if (count != 1) {
        throw invalid_argument("Exactly one king should be present!");
    }
    auto self = shared_from_this();
    // Create pawns
    for (int col{0}; col < board_width; ++col) {
        position_map[getPositionIndex(white_pawns_row_index, col)] = std::make_unique<Pawn>(white_pawns_row_index, col, WHITE);
        position_map[getPositionIndex(black_pawns_row_index, col)] = std::make_unique<Pawn>(black_pawns_row_index, col, BLACK);
    }
    assert(figures_format.size() == board_width);
    for (int col{0}; col < board_width; ++col) {
        switch (figures_format[col]) {
            case 'R':
                position_map[getPositionIndex(white_back_row_index, col)] = std::make_unique<Rook>(white_back_row_index, col, WHITE);
                position_map[getPositionIndex(black_back_row_index, col)] = std::make_unique<Rook>(black_back_row_index, col, BLACK);
                break;
            case 'N':
                position_map[getPositionIndex(white_back_row_index, col)] = std::make_unique<Knight>(white_back_row_index, col, WHITE);
                position_map[getPositionIndex(black_back_row_index, col)] = std::make_unique<Knight>(black_back_row_index, col, BLACK);
                break;
            case 'B':
                position_map[getPositionIndex(white_back_row_index, col)] = std::make_unique<Bishop>(white_back_row_index, col, WHITE);
                position_map[getPositionIndex(black_back_row_index, col)] = std::make_unique<Bishop>(black_back_row_index, col, BLACK);
                break;
            case 'Q':
                position_map[getPositionIndex(white_back_row_index, col)] = std::make_unique<Queen>(white_back_row_index, col, WHITE);
                position_map[getPositionIndex(black_back_row_index, col)] = std::make_unique<Queen>(black_back_row_index, col, BLACK);
                break;
            case 'K':
                position_map[getPositionIndex(white_back_row_index, col)] = std::make_unique<King>(white_back_row_index, col, WHITE);
                position_map[getPositionIndex(black_back_row_index, col)] = std::make_unique<King>(black_back_row_index, col, BLACK);
                this->whiteKingPosition = {white_back_row_index, col};
                this->blackKingPosition = {black_back_row_index, col};
                break;
            default:
                throw std::runtime_error("Unknown figure!");

        }
    }
    for (auto& [index, piece] : position_map) {
        piece->setBoard(self);
    }
}

bool Board::isPositionOccupied(int index) const {
    return position_map.find(index) != position_map.end();
}

unique_ptr<Piece>& Board::getPiece(int index) {
    if (position_map.find(index) == position_map.end()) {
        throw NoPieceAtPositionException(getNotation(index));
    }
    return position_map[index];
}


void Piece::setBoard(std::weak_ptr<Board> b) {
    this->board = b;
}

Color Piece::getColor() const {
    return color;
}

void Piece::setCaptured(bool c) {
    this->captured = c;
}

string Board::boardString() {
    string board_str;
    for (int row{board_height - 1}; row >= 0; --row) {
        board_str += std::to_string(row + 1) + " ";
        for (int col{0}; col < board_width; ++col) {
            auto index = getPositionIndex(row, col);
            if (position_map.find(index) == position_map.end()) {
                board_str += '.';
            } else {
                board_str += (position_map[index]->getColor() == WHITE) ? std::tolower(position_map[index]->getSymbol()) : position_map[index]->getSymbol();
            }
        }
        board_str += '\n';
    }
    string columns;
    for (int col{0}; col < board_width; ++col) {
        columns += 'A' + col;
    }
    board_str += "  " + columns + '\n';
    return board_str;
}

string Board::boardStringWithPossibleMoves(int from) {
    if (!isPositionOccupied(from)) {
        throw NoPieceAtPositionException(getNotation(from));
    }
    vector<int> possible_moves = getPiece(from)->getPossibleMoves(from);
    string board_str;
    for (int row{ board_height - 1 }; row >= 0; --row) {
        board_str += std::to_string(row + 1) + " ";
        for (int col{ 0 }; col < board_width; ++col) {
            auto index = getPositionIndex(row, col);
            if (position_map.find(index) == position_map.end()) {
                if (std::find(possible_moves.begin(), possible_moves.end(), index) != possible_moves.end()) {
                    board_str += 'x';
                }
                else {
                    board_str += '.';
                }
            }
            else {
                board_str += (position_map[index]->getColor() == WHITE) ? std::tolower(position_map[index]->getSymbol()) : position_map[index]->getSymbol();
            }
        }
        board_str += '\n';
    }
    return board_str;
}



bool Board::isPawnAttacking(int position_index, int target, Color c) {
    auto [row, col] = Board::getPosition(position_index);
    auto [target_row, target_col] = Board::getPosition(target);
    if (c == WHITE) {
        return target_row == row + 1 && (target_col == col + 1 || target_col == col - 1);
    } else {
        return target_row == row - 1 && (target_col == col + 1 || target_col == col - 1);
    }
}


bool Pawn::amIAttacking(int index) const {
    return board.lock()->isPawnAttacking(Board::getPositionIndex(row, column), index, color);
}

bool Board::isRookAttacking(int position_index, int target) const {
    auto [row, col] = Board::getPosition(position_index);
    auto [target_row, target_col] = Board::getPosition(target);

    if (row == target_row) {
        int bigger = std::max(col, target_col);
        int smaller = std::min(col, target_col);
        for (int i{smaller + 1}; i < bigger; ++i) {
            if (isPositionOccupied(getPositionIndex(row, i))) {
                return false;
            }
        }
        return true;
    }
    if (col == target_col) {
        int bigger = std::max(row, target_row);
        int smaller = std::min(row, target_row);
        for (int i{smaller + 1}; i < bigger; ++i) {
            if (isPositionOccupied(getPositionIndex(i, col))) {
                return false;
            }
        }
        return true;
    }
    return false;
}

bool Rook::amIAttacking(int index) const {
    return board.lock()->isRookAttacking(Board::getPositionIndex(row, column), index);
}

bool Board::isKnightAttacking(int position_index, int target)  {
    auto [row, col] = Board::getPosition(position_index);
    auto [target_row, target_col] = Board::getPosition(target);
    return (std::abs(row - target_row) == 2 && std::abs(col - target_col) == 1) ||
           (std::abs(row - target_row) == 1 && std::abs(col - target_col) == 2);
}

bool Knight::amIAttacking(int index) const {
    return board.lock()->isKnightAttacking(Board::getPositionIndex(row, column), index);
}

bool Board::isBishopAttacking(int position_index, int target) const {
    auto [row, col] = Board::getPosition(position_index);
    auto [target_row, target_col] = Board::getPosition(target);
    if (std::abs(row - target_row) != std::abs(col - target_col)) {
        return false;
    }
    int row_step = (row < target_row) ? 1 : -1;
    int col_step = (col < target_col) ? 1 : -1;
    int i{row + row_step};
    int j{col + col_step};
    while (i != target_row) {
        if (isPositionOccupied(getPositionIndex(i, j))) {
            return false;
        }
        i += row_step;
        j += col_step;
    }
    return true;
}

bool Bishop::amIAttacking(int index) const {
    return this->board.lock()->isBishopAttacking(Board::getPositionIndex(row, column), index);
}

bool Board::isQueenAttacking(int position_index, int target) {
    return isRookAttacking(position_index, target) || isBishopAttacking(position_index, target);
}

bool Queen::amIAttacking(int index) const {
    return this->board.lock()->isQueenAttacking(Board::getPositionIndex(row, column), index);
}

bool Board::isKingAttacking(int position_index, int target) {
    auto [row, col] = Board::getPosition(position_index);
    auto [target_row, target_col] = Board::getPosition(target);
    return std::abs(row - target_row) <= 1 && std::abs(col - target_col) <= 1;
}

bool Pawn::canMove(int index) const {
    auto [row, col] = Board::getPosition(index);
    if (color == WHITE) {
        if (row == this->row + 1 && (col == this->column + 1 || col == this->column - 1) && this->board.lock()->isPositionOccupied(index) && this->board.lock()->getPiece(index)->getColor() == BLACK) {
            return true;
        }
        if (row == this->row + 1 && col == this->column && !this->board.lock()->isPositionOccupied(index)) {
            return true;
        }
        if (row == this->row + 2 && col == this->column && !hasMoved && !this->board.lock()->isPositionOccupied(index) && !this->board.lock()->isPositionOccupied(Board::getPositionIndex(row - 1, col))) {
            return true;
        }
    } else {
        if (row == this->row - 1 && (col == this->column + 1 || col == this->column - 1) && this->board.lock()->isPositionOccupied(index) && this->board.lock()->getPiece(index)->getColor() == WHITE) {
            return true;
        }
        if (row == this->row - 1 && col == this->column && !this->board.lock()->isPositionOccupied(index)) {
            return true;
        }
        if (row == this->row - 2 && col == this->column && !hasMoved && !this->board.lock()->isPositionOccupied(index) && !this->board.lock()->isPositionOccupied(Board::getPositionIndex(row + 1, col))) {
            return true;
        }
    }
    return false;
}


bool King::amIAttacking(int index) const {
    return Board::isKingAttacking(Board::getPositionIndex(row, column), index);
}

bool Piece::isCaptured() const {
    return captured;
}

PieceType Piece::getType() const {
    return type;
}

void Piece::setRow(int r) {
    this->row = r;
}

void Piece::setColumn(int c) {
    this->column = c;
}

void Piece::setHasMoved(bool h) {
    this->hasMoved = h;
}

bool Piece::canMove(int index) const {
    bool occupied = this->board.lock()->isPositionOccupied(index);
    if (occupied && this->board.lock()->getPiece(index)->getColor() == this->color) {
        return false;
    }
    return amIAttacking(index);
}


bool Board::checkIfChecked(Color c) {
    auto king_position = (c == WHITE) ? whiteKingPosition : blackKingPosition;
    for (auto& [index, piece] : position_map) {
        if (!piece->isCaptured()  && piece->getColor() != c && piece->amIAttacking(Board::getPositionIndex(king_position.first, king_position.second))) {
            return true;
        }
    }
    return false;
}

bool Board::isChecked(Color c) {
    return (c == WHITE) ? whiteChecked : blackChecked;
}

void Board::move(int from, int to) {
    if (!isPositionOccupied(from)) {
        throw NoPieceAtPositionException(getNotation(from));
    }
    auto& piece = getPiece(from);
    if (piece->getColor() != turn) {
        throw NotYourTurnException(turn);
    }

    if(!piece->canMove(to)) {
        throw InvalidMoveException(getNotation(from) + getNotation(to));
    }

    if (isPositionOccupied(to)) {
        auto& target = getPiece(to);
        if (getPiece(to)->getColor() == piece->getColor()) {
            throw InvalidMoveException(getNotation(from) + getNotation(to));
        }
        target->setCaptured(true);
        auto [row, col] = getPosition(to);
        piece->setRow(row);
        piece->setColumn(col);
        if (checkIfChecked(turn)) {
            target->setCaptured(false);
            piece->setRow(getPosition(from).first);
            piece->setColumn(getPosition(from).second);
            throw InvalidMoveException(getNotation(from) +"king checked when moving to" + getNotation(to));
        }
        position_map.erase(to);
        position_map[to] = std::move(piece);
        position_map.erase(from);
    } else {
        auto [row, col] = getPosition(to);
        piece->setRow(row);
        piece->setColumn(col);
        if (checkIfChecked(turn)) {
            piece->setRow(getPosition(from).first);
            piece->setColumn(getPosition(from).second);
            throw InvalidMoveException(getNotation(from) +"king checked when moving to" + getNotation(to));
        }
        position_map[to] = std::move(piece);
        position_map.erase(from);
    }
    position_map[to]->setHasMoved(true);
    turn = (turn == WHITE) ? BLACK : WHITE;

    if (checkIfChecked(turn)) {
        if (turn == WHITE) {
            whiteChecked = true;
        } else {
            blackChecked = true;
        }
    } else {
        if (turn == WHITE) {
            whiteChecked = false;
        } else {
            blackChecked = false;
        }
    }
}

int Board::numberOfPieces() const {
    return static_cast<int>(position_map.size());
}

vector<int> Piece::getPossibleMoves(int index) {
    vector<int> moves;
    auto x = this->board.lock()->getPosition(index);
    for (int i{ 0 }; i < board_height * board_width; ++i) {
        if (canMove(i)) {
            moves.push_back(i);
        }
    }
    return moves;
}

vector<int, char> Board::indexToPieceMap() const {
    vector<int, char> map;
    for (auto& [index, piece] : position_map) {
        map.push_back({index, piece->getSymbol()});
    }
    return map;
}

Color Board::getTurn() const {
    return turn;
}