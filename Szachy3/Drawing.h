#ifndef DRAWING_H
#define DRAWING_H

#include <d2d1_3.h>
#include <dwrite_3.h>
#include <vector>
#include <unordered_map>
#include <string>
using std::vector;
using std::pair;
using std::string;
using std::unordered_map;

namespace {
	const WCHAR whiteTurn[] = L"White\'s turn!";
	const WCHAR blackTurn[] = L"Black\'s turn!";
	const WCHAR wrongTurn[] = L"Not your turn!";
	const WCHAR invalidMove[] = L"Invalid move!";
    const WCHAR* letters[] = { L"A", L"B", L"C", L"D", L"E", L"F", L"G", L"H"};
	const WCHAR* numbers[] = { L"1", L"2", L"3", L"4", L"5", L"6", L"7", L"8" };
};



class DrawingChessboardInterface {
private:
	static const UINT white_index = 0;
	static const UINT black_index = 6;
	static const UINT pawn_index = 0;
	static const UINT rook_index = 1;
	static const UINT knight_index = 2;
	static const UINT bishop_index = 3;
	static const UINT queen_index = 4;
	static const UINT king_index = 5;
	ID2D1Bitmap* PieceBitmaps[12]; // for example, use: PieceBitmaps[white_index + pawn_index] if you want to access white pawn bitmap
	ID2D1Bitmap* TileBitmaps[3]; // for example, use: TileBitmaps[white_index] if you want to access white tile bitmap
	LPCWSTR pieces_file_names[12] = { L"white-pawn.png", L"white-rook.png",
		L"white-knight.png", L"white-bishop.png", L"white-queen.png", L"white-king.png", L"black-pawn.png",
		L"black-rook.png", L"black-knight.png", L"black-bishop.png", L"black-queen.png", L"black-king.png" };
	LPCWSTR tiles_file_names[3] = { L"white-tile.bmp", L"black-tile.bmp", L"transparent_red_for_check.bmp" };
	HWND hwnd;
	bool isPicked = FALSE;
	int pickedFigure = -1;

	FLOAT tile_width{ 0.0 };
	FLOAT tile_height{ 0.0 };
	vector<pair<int, char>> positionToPieceMap;

	ID2D1SolidColorBrush* brush{ nullptr };
	ID2D1RadialGradientBrush* radial_brush{ nullptr };
	ID2D1StrokeStyle* brush_style{ nullptr };
	HRESULT hr{ S_OK };
	IDWriteTextFormat* text_format{ nullptr };
	IDWriteTextFormat* text_format2{ nullptr };
	ID2D1Factory7* d2d_factory{ nullptr };
	IDWriteFactory* write_factory{ nullptr };
	ID2D1HwndRenderTarget* d2d_render_target{ nullptr };
	FLOAT mouse_x{ 0.0f };
	FLOAT mouse_y{ 0.0f };
	IWICImagingFactory* wic_factory{ nullptr };
	D2D1_COLOR_F const black_color = {
	.r = 0.0f, .g = 0.0f, .b = 0.0f, .a = 1.0f };
	vector<D2D1_RECT_F> chessboard;
	unordered_map<char, UINT> symbolToBitmap;
	BOOL isWhiteChecked = FALSE;
	BOOL isBlackChecked = FALSE;
	int whiteKingIndex{ 4 };
	int blackKingIndex{ 60 };
	const WCHAR* currentMessage;
	BOOL flipping_turn = FALSE;
	D2D1_ELLIPSE whiteToken;
	D2D1_ELLIPSE blackToken;
	INT turn = 0; // 0 - white, 1 - black
	BOOL pulsing = FALSE;
	int pulsing_index = -1; // piece bitmap index
	float pulse_value = 0.0f;
	float pulse_increase = 0.01f;

	



public:
	DrawingChessboardInterface(HWND hwnd) : hwnd(hwnd), symbolToBitmap({
		{'P', white_index + pawn_index},
		{'R', white_index + rook_index},
		{'N', white_index + knight_index},
		{'B', white_index + bishop_index},
		{'Q', white_index + queen_index},
		{'K', white_index + king_index},
		{'p', black_index + pawn_index},
		{'r', black_index + rook_index},
		{'n', black_index + knight_index},
		{'b', black_index + bishop_index},
		{'q', black_index + queen_index},
		{'k', black_index + king_index}
		}) 
	{
		setCurrentMessage(whiteTurn);
		whiteToken = D2D1::Ellipse(D2D1::Point2F(100.0f, 100.0f), 400.0f, 400.0f);
		blackToken = D2D1::Ellipse(D2D1::Point2F(100.0f, 100.0f), 400.0f, 400.0f);
	}
	void setPickedFigure(int index);
	void unsetPickedFigure();
	void setCurrentMessage(const WCHAR* m);
	void changeTurn();
	void setPositionToPieceMap(const std::vector<std::pair<int, char>>& map);
	HRESULT initialize();
	HRESULT drawChessboard();
	HRESULT createRenderResources(HWND hwnd, ID2D1Factory7* d2d_factory, ID2D1HwndRenderTarget* d2d_render_target);
	HRESULT load_pieces_and_tiles_bitmaps();
	void cleanup();
	void calculateChessboardRects(RECT client_rect);
	int isMouseOnTile();
	void set_mouse_position(FLOAT x, FLOAT y);
	void ReportError( const wchar_t* message);
	void increasePulse();
	void setPulsingIndex(int index);
	void unsetPulsingIndex();

};

#endif // DRAWING_H