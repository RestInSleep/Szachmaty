#include "WinMain.h"
#include "Drawing.h"
#include "board.h"
#include "board_exceptions.h"
#include <d2d1_3.h>
#include <dwrite_3.h>




namespace {
	void InitTimer(HWND hwnd) {
		SetTimer(hwnd, 1, 1, nullptr);
	}
	void ReleaseTimer(HWND hwnd, UINT_PTR id) {
		KillTimer(hwnd, id);
	}
	HRESULT hr = S_OK;
	BOOL FIGURE_PICKED = FALSE;
	INT picked_figure = -1;
	std::shared_ptr<Board> board;
	INT from = -1;
	INT to = -1;
	vector<pair<int, char>> indexToPiece;
	unique_ptr<DrawingChessboardInterface> dci;
	bool pulsing = FALSE;
	int pulse_index = -1;
	int aux = 0;
}


LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	bool isButtonDown = (wParam & MK_LBUTTON) != 0;
	switch (uMsg)
	{
	case WM_CREATE:
		dci = std::make_unique<DrawingChessboardInterface>(hwnd);
		hr = dci->initialize();
		InitTimer(hwnd);
		board = std::make_shared<Board>();
		board->init();
		dci->setPositionToPieceMap(board->indexToPieceMap());
		return hr;

	case WM_CLOSE:
		if (MessageBox(hwnd, TEXT("Really quit?"), TEXT("My application"), MB_OKCANCEL | MB_ICONQUESTION) == IDOK) {
			DestroyWindow(hwnd);
			ReleaseTimer(hwnd, 1);
		}
		return 0;

	case WM_DESTROY:
		dci->cleanup();
		PostQuitMessage(0);

		return 0;

	case WM_PAINT:
		hr = dci->drawChessboard();
		ValidateRect(hwnd, nullptr);
		return hr;

	case WM_TIMER:
		dci->increasePulse();
		InvalidateRect(hwnd, nullptr, FALSE);
		return 0;
	case WM_LBUTTONDOWN:
		SetCursor(LoadCursor(NULL, IDC_HAND));
		dci->set_mouse_position(LOWORD(lParam), HIWORD(lParam));
		from = dci->isMouseOnTile();
		if (from != -1) {
				if (board->isPositionOccupied(from)) {
					if (board->getPiece(from)->getColor() == board->getTurn()) {
						FIGURE_PICKED = TRUE;
						picked_figure = from;
						dci->setPickedFigure(from);
					}
					else {
						dci->setCurrentMessage(wrongTurn);
					}
				}
		}
		InvalidateRect(hwnd, nullptr, FALSE);
		return hr;

	case WM_LBUTTONUP:
		SetCursor(LoadCursor(NULL, IDC_ARROW));
		dci->set_mouse_position(LOWORD(lParam), HIWORD(lParam));
		to = dci->isMouseOnTile();
		if (to != -1 && FIGURE_PICKED) {
						try {
							board->move(from, to);
							if (board->isChecked(WHITE)) {
								MessageBox(hwnd, L"White is checked", TEXT("Warning"), MB_OK);
							}
							if (board->isChecked(BLACK)) {
								MessageBox(hwnd, L"Black is checked", TEXT("Warning"), MB_OK);
							}
							dci->changeTurn();
							dci->unsetPickedFigure();
							FIGURE_PICKED = FALSE;
							pulsing = FALSE;
							pulse_index = -1;
							dci->unsetPulsingIndex();

						}
						catch (const InvalidMoveException& e) {
							dci->setCurrentMessage(invalidMove);
						}
						catch (const std::exception& e) {
						}

		}
		dci->unsetPickedFigure();
		dci->setPositionToPieceMap(board->indexToPieceMap());
		InvalidateRect(hwnd, nullptr, FALSE);
		return hr;
	case WM_MOUSEMOVE:
		// Change the cursor dynamically
		if (isButtonDown) {
			SetCursor(LoadCursor(NULL, IDC_HAND)); // Set to pointing hand cursor
		}
		else {
			SetCursor(LoadCursor(NULL, IDC_ARROW)); // Default arrow cursor
		}
		dci->set_mouse_position(LOWORD(lParam), HIWORD(lParam));
		aux = dci->isMouseOnTile();
		if (pulsing && aux != pulse_index) {
			pulsing = FALSE;
			pulse_index = -1;
			dci->unsetPulsingIndex();
		}
		else if (pulsing && aux == pulse_index) {
			pulsing = TRUE;
		}
		else if (aux != -1 && !FIGURE_PICKED && !pulsing && board->isPositionOccupied(aux) && board->getPiece(aux)->getColor() == board->getTurn()) {
			pulsing = TRUE;
			pulse_index = aux;
			dci->setPulsingIndex(aux);
		}
		InvalidateRect(hwnd, nullptr, FALSE);
		return hr;
	case WM_SIZE:
		dci->calculateChessboardRects({ 0, 0, LOWORD(lParam), HIWORD(lParam) });
		InvalidateRect(hwnd, nullptr, FALSE);
		return hr;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


INT WINAPI wWinMain(
	_In_ [[maybe_unused]] HINSTANCE instance,
	_In_opt_ [[maybe_unused]] HINSTANCE prev_instance,
	_In_ [[maybe_unused]] PWSTR cmd_line,
	_In_ [[maybe_unused]] INT cmd_show
) {
	WNDCLASSEX wc = {
		.cbSize = sizeof(WNDCLASSEX),
		.style = CS_HREDRAW | CS_VREDRAW,
		.lpfnWndProc = WindowProc,
		.cbClsExtra = 0,
		.cbWndExtra = 0,
		.hInstance = instance,
		.hIcon = nullptr,
		.hCursor = static_cast<HCURSOR>(LoadImage(nullptr, IDC_ARROW, IMAGE_CURSOR, LR_DEFAULTSIZE, LR_DEFAULTSIZE, LR_DEFAULTSIZE | LR_SHARED)),
		.hbrBackground = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH)),
		.lpszMenuName = nullptr,
		.lpszClassName = TEXT("WindowClass"),
	};

	ATOM atom  = RegisterClassEx(&wc);
	if (atom == 0) {
		MessageBox(nullptr, TEXT("RegisterClassEx failed"), TEXT("Error"), MB_OK);
	}


	RECT wr = { 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN) };
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

	HWND hwnd = CreateWindowEx(
		0,
		TEXT("WindowClass"),
		TEXT("Let's play!"),
		WS_OVERLAPPEDWINDOW,
		wr.left,                        // Adjusted x-coordinate
		wr.top,                         // Adjusted y-coordinate
		wr.right - wr.left,             // Adjusted width
		wr.bottom - wr.top,             // Adjusted height
		nullptr,
		nullptr,
		instance,
		nullptr
	);
	if (hwnd == nullptr) {
		return 0;
	}

	ShowWindow(hwnd, SW_MAXIMIZE);

	MSG msg = {};
	BOOL ret = 0;
	while (hr = GetMessage(&msg, nullptr, 0, 0) != 0) {
		if (hr == -1) {
			MessageBox(nullptr, L"GetMessage failed", L"Error", MB_OK);
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		
	}
	return 0;
}
