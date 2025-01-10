// board.h
#ifndef UNTITLED24_BOARD_H
#define UNTITLED24_BOARD_H
#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <string_view>
#include <vector>

using std::string;
using std::string_view;
using std::pair;
using std::unique_ptr;
using std::vector;
class Board;

constexpr int board_width{8};
constexpr int board_height{8};
constexpr string_view figures_format{"RNBQKBNR"};

constexpr int white_pawns_row_index{1};
constexpr int black_pawns_row_index{board_height - 2};
constexpr int white_back_row_index{0};
constexpr int black_back_row_index{board_height - 1};

enum class PieceType {
    PAWN,
    ROOK,
    KNIGHT,
    BISHOP,
    QUEEN,
    KING,
    NO_PIECE
};

using enum PieceType;

enum class Color {
    WHITE,
    BLACK,
    NO_COLOR
};

using enum Color;

class Piece {
    private:
    protected:
        Color color;
        std::weak_ptr<Board> board;
        int column;
        int row;
        bool captured{false};
        bool hasMoved{false};
        PieceType type;

public:
        Piece(int row, int column, Color color) : row(row), column(column), color(color) {}
        virtual ~Piece() = default;  // Added virtual destructor
        [[nodiscard]] virtual char getSymbol() const;
        void setBoard(std::weak_ptr<Board> board);
        [[nodiscard]] pair<int, int> getPosition() const;
        [[nodiscard]] Color getColor() const;
        [[nodiscard]] virtual bool amIAttacking(int index) const = 0;
        bool isCaptured() const;
        [[nodiscard]] PieceType getType() const;
        void setCaptured(bool captured);
        void setRow(int row);
        void setColumn(int column);
        void setHasMoved(bool hasMoved);
        [[nodiscard]] bool getHasMoved() const;
        [[nodiscard]] virtual bool canMove(int index) const;
        vector<int> getPossibleMoves(int index);
};



// Piece definitions with corrected constructor parameter order
class Pawn : public Piece {
private:
	constexpr static char symbol{ 'P' };
public:
    Pawn(int row, int column, Color color) : Piece(row, column, color) {
        type = PAWN;
    }
    [[nodiscard]] char getSymbol() const override;
    [[nodiscard]] bool amIAttacking(int index) const override;
    bool canMove(int index) const override;
};

class Rook : public Piece {
private:
    constexpr static char symbol{ 'R' };
public:
    Rook(int row, int column, Color color) : Piece(row, column, color) {
        type = ROOK;
    }
    [[nodiscard]] bool amIAttacking(int index) const override;
};

class Knight : public Piece {
private:
    constexpr static char symbol{ 'N' };
public:
    Knight(int row, int column, Color color) : Piece(row, column, color) {
        type = KNIGHT;
    }
    [[nodiscard]] bool amIAttacking(int index) const override;
};

class Bishop : public Piece {
private:
    constexpr static char symbol{'B'};
public:
    Bishop(int row, int column, Color color) : Piece(row, column, color) {
        type = BISHOP;
    }
    [[nodiscard]] bool amIAttacking(int index) const override;
};

class Queen : public Piece {
private:
	constexpr static char symbol{ 'Q' };
public:
    Queen(int row, int column, Color color) : Piece(row, column, color) {
        type = QUEEN;
    }
    [[nodiscard]] bool amIAttacking(int index) const override;
};

class King : public Piece {
private:
	constexpr static char symbol{ 'K' };
public:
    King(int row, int column, Color color) : Piece(row, column, color) {
        type = KING;
    }
    [[nodiscard]] bool amIAttacking(int index) const override;
};

class Board : public std::enable_shared_from_this<Board> {
private:
    std::unordered_map<int, std::unique_ptr<Piece>> position_map;
    bool gameEnded{false};
    Color turn{Color::WHITE};
    Color winner{NO_COLOR};
    bool whiteChecked{false};
    bool blackChecked{false};
    pair<int, int> whiteKingPosition;
    pair<int, int> blackKingPosition;
public:
    void init();
    static int getPositionIndex(int row, int column);
    static pair<int, int> getPosition(int index);
    bool isPositionOccupied(int index) const;
    static string getNotation(int index);
	static int indexFromNotation(string_view notation);
    unique_ptr<Piece>& getPiece(int index);
    string boardString();
	string boardStringWithPossibleMoves(int from);

    static bool isPawnAttacking(int position_index, int target, Color c);
    bool isRookAttacking(int position_index, int target) const;
    static bool isKnightAttacking(int position_index, int target);
    bool isBishopAttacking(int position_index, int target) const;
    bool isQueenAttacking(int position_index, int target);
    static bool isKingAttacking(int position_index, int target);
    bool checkIfChecked(Color c); // actually checks
    bool isChecked(Color c) const; //  only getter
    void move(int from, int to);
    int numberOfPieces() const;
    Color getTurn() const;
    vector<pair<int, char>> indexToPieceMap() const;

};
#endif //UNTITLED24_BOARD_H