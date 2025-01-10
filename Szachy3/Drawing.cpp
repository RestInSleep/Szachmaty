
#include "Drawing.h"
#include "board.h"
#include <algorithm>
#include <vector>
#include <cmath>
#include <d2d1.h>
#include <dwrite.h>
#include <combaseapi.h>
#include <wincodec.h>
#include <string>
#include <string_view>
#include <iostream>
#include <unordered_map>
#include <cwchar>
#include <cmath>

using D2D1::RenderTargetProperties;
using D2D1::RectF;
using D2D1::Point2F;
using D2D1::HwndRenderTargetProperties;
using D2D1::SizeU;
using D2D1::Point2F;
using D2D1::StrokeStyleProperties;
using D2D1::BezierSegment;
using D2D1::QuadraticBezierSegment;
using std::sin;
using std::cos;
using D2D1::Matrix3x2F;
using D2D1::BezierSegment;
using D2D1::QuadraticBezierSegment;
using D2D1::BitmapProperties;
using D2D1::PixelFormat;
using std::vector;
using std::string;
using std::string_view;
using std::min;
using std::unordered_map;
using std::fmod;


namespace {
	
	
	// indexing goes from A1 to H1 are 0-7, A2 to H2 are 8-15, etc.

	vector<pair<int, char>> positionToPieceMap;


	
}

void DrawingChessboardInterface::setPulsingIndex(int index) {
	pulsing = TRUE;
	pulsing_index = index;
	pulse_value = 0.0f;
}

void DrawingChessboardInterface::unsetPulsingIndex() {
	pulsing = FALSE;
	pulsing_index = -1;
}

void DrawingChessboardInterface::increasePulse() {
	if (pulsing) {
		pulse_value += pulse_increase;
		pulse_value = fmod(pulse_value, 3.141529);
	}
}

void DrawingChessboardInterface::setPickedFigure(int index) {
	pickedFigure = index;
	isPicked = TRUE;
}
void DrawingChessboardInterface::unsetPickedFigure() {
	pickedFigure = -1;
	isPicked = FALSE;
}

void DrawingChessboardInterface::setPositionToPieceMap(const vector<pair<int, char>>& map) {
	positionToPieceMap = map;
}

void DrawingChessboardInterface::changeTurn() {
		turn = (turn + 1) % 2;
		if (turn == 0) {
			setCurrentMessage(whiteTurn);
		}
		else {
			setCurrentMessage(blackTurn);
		}
}

void DrawingChessboardInterface::ReportError(const wchar_t* message)
{
	wchar_t errorMessage[256];
	swprintf_s(errorMessage, L"%s\nError code: 0x%08X", message, hr);
	MessageBox(nullptr, errorMessage, L"Error", MB_ICONERROR);
}



void DrawingChessboardInterface::setCurrentMessage(const WCHAR* m) {
	currentMessage = m;
}



void DrawingChessboardInterface::calculateChessboardRects(RECT client_rect) {
	// we want the chessboard to be fully visible, so we  look upon the smaller dimension
	chessboard = vector<D2D1_RECT_F>(board_height * board_width);
	UINT32 wnd_width = client_rect.right - client_rect.left;
	UINT32 wnd_height = client_rect.bottom - client_rect.top;

	UINT32 window_size = min(wnd_width, wnd_height);
	UINT32 board_width_pixels = window_size / 2;
	UINT32 board_height_pixels = board_width_pixels;
	tile_width = board_width_pixels / board_width;
	tile_height = tile_width;

	UINT32 left_bottom_x = (wnd_width - board_width_pixels) / 2;
	UINT32 left_bottom_y = (wnd_height - board_height_pixels) / 2 + board_height_pixels;

	UINT32 first_tile_x = left_bottom_x;
	UINT32 first_tile_y = left_bottom_y - tile_height;
	for (int i = 0; i < board_height; ++i) {
		for (int j = 0; j < board_width; ++j) {
			chessboard[i * board_width + j] = D2D1::RectF(
				first_tile_x + j * tile_width,
				first_tile_y - i * tile_height,
				first_tile_x + (j + 1) * tile_width,
				first_tile_y - (i - 1) * tile_height
			);
		}
		
	}
}


int DrawingChessboardInterface::isMouseOnTile() {
	for (UINT i = 0; i < board_height * board_width; ++i) {
		if (chessboard[i].left <= mouse_x && chessboard[i].right >= mouse_x && chessboard[i].top <= mouse_y && chessboard[i].bottom >= mouse_y) {
			return i;
		}
	}
	return -1;
}


void DrawingChessboardInterface::set_mouse_position(FLOAT x, FLOAT y) {
	mouse_x = x;
	mouse_y = y;
}

template <class T> void SafeRelease(T** ppT)
{
	if (*ppT)
	{
		(*ppT)->Release();
		*ppT = NULL;
	}
}

HRESULT LoadBitmapFromFile(
	ID2D1RenderTarget* pRenderTarget,
	IWICImagingFactory* pIWICFactory,
	PCWSTR uri,
	UINT destinationWidth,
	UINT destinationHeight,
	ID2D1Bitmap** ppBitmap
) {
	if (!pRenderTarget || !pIWICFactory || !ppBitmap) {
		MessageBox(nullptr, L"Invalid argument in LoadBitmapFromFile", L"Error", MB_OK);
		return E_INVALIDARG;
	}

	*ppBitmap = NULL;

	IWICBitmapDecoder* pDecoder = NULL;
	IWICBitmapFrameDecode* pSource = NULL;
	IWICBitmapScaler* pScaler = NULL;
	IWICFormatConverter* pConverter = NULL;

	HRESULT hr = pIWICFactory->CreateDecoderFromFilename(
		uri,
		NULL,
		GENERIC_READ,
		WICDecodeMetadataCacheOnLoad,
		&pDecoder
	);

	if (FAILED(hr)) {
		MessageBox(nullptr, (std::wstring(L"Failed to create decoder for file: ") + uri).c_str(), L"Error", MB_OK);
		return hr;
	}

	// Get the first frame of the image.
	if (SUCCEEDED(hr)) {
		hr = pDecoder->GetFrame(0, &pSource);
		if (FAILED(hr)) {
			MessageBox(nullptr, (std::wstring(L"Failed to get frame for file: ") + uri).c_str(), L"Error", MB_OK);
		}
	}

	// Scale the image if necessary.
	if (SUCCEEDED(hr) && (destinationWidth != 0 || destinationHeight != 0)) {
		hr = pIWICFactory->CreateBitmapScaler(&pScaler);
		if (FAILED(hr)) {
			MessageBox(nullptr, (std::wstring(L"Failed to create bitmap scaler for file: ") + uri).c_str(), L"Error", MB_OK);
		}
		else {
			hr = pScaler->Initialize(
				pSource,
				destinationWidth,
				destinationHeight,
				WICBitmapInterpolationModeFant
			);
			if (FAILED(hr)) {
				MessageBox(nullptr, (std::wstring(L"Failed to initialize scaler for file: ") + uri).c_str(), L"Error", MB_OK);
			}
		}
	}

	// Use the scaler if it was created, otherwise use the source frame.
	IWICBitmapSource* pBitmapSource = pSource;
	if (pScaler) {
		pBitmapSource = pScaler;
	}

	// Convert the image format to 32bppPBGRA.
	if (SUCCEEDED(hr)) {
		hr = pIWICFactory->CreateFormatConverter(&pConverter);
		if (FAILED(hr)) {
			MessageBox(nullptr, (std::wstring(L"Failed to create format converter for file: ") + uri).c_str(), L"Error", MB_OK);
		}
	}

	if (SUCCEEDED(hr)) {
		hr = pConverter->Initialize(
			pBitmapSource,
			GUID_WICPixelFormat32bppPBGRA,
			WICBitmapDitherTypeNone,
			NULL,
			0.f,
			WICBitmapPaletteTypeMedianCut
		);
		if (FAILED(hr)) {
			MessageBox(nullptr, (std::wstring(L"Failed to initialize format converter for file: ") + uri).c_str(), L"Error", MB_OK);
		}
	}

	// Create a Direct2D bitmap from the WIC bitmap.
	if (SUCCEEDED(hr)) {
		hr = pRenderTarget->CreateBitmapFromWicBitmap(
			pConverter,
			NULL,
			ppBitmap
		);
		if (FAILED(hr)) {
			MessageBox(nullptr, (std::wstring(L"Failed to create D2D bitmap for file: ") + uri).c_str(), L"Error", MB_OK);
		}
	}

	// Release resources.
	SafeRelease(&pDecoder);
	SafeRelease(&pSource);
	SafeRelease(&pScaler);
	SafeRelease(&pConverter);

	return hr;
}

HRESULT DrawingChessboardInterface::load_pieces_and_tiles_bitmaps() {
	 // Ensure `hr` is declared properly
	for (UINT i = 0; i < 12; ++i) {
		hr = LoadBitmapFromFile(d2d_render_target, wic_factory, pieces_file_names[i], 100, 100, &PieceBitmaps[i]);
		if (FAILED(hr)) {
			// Use a wstring to format the error message
			std::wstring errorMessage = L"Failed to load piece bitmap number " + std::to_wstring(i);
			MessageBox(nullptr, errorMessage.c_str(), L"Error", MB_OK);
			return hr;
		}
	}
	for (UINT i = 0; i < 3; ++i) {
		hr = LoadBitmapFromFile(d2d_render_target, wic_factory, tiles_file_names[i], 100, 100, &TileBitmaps[i]);
		if (FAILED(hr)) {
			// Use a wstring to format the error message
			std::wstring errorMessage = L"Failed to load tile bitmap number " + std::to_wstring(i);
			MessageBox(nullptr, errorMessage.c_str(), L"Error", MB_OK);
			return hr;
		}
	}
	return S_OK; // Return success if all bitmaps are loaded
}



HRESULT DrawingChessboardInterface::initialize() {
	hr = D2D1CreateFactory(
		D2D1_FACTORY_TYPE_SINGLE_THREADED, &d2d_factory);
	if (FAILED(hr)) {
		MessageBox(hwnd, L"Failed to create D2D factory", L"Error", MB_OK);
		return 1;
	}
	RECT rc;
	if (GetClientRect(hwnd, &rc) == FALSE) {
		MessageBox(hwnd, L"Failed to get client rect", L"Error", MB_OK);
		return 1;
	}
	hr = (d2d_factory)->CreateHwndRenderTarget(
		RenderTargetProperties(),
		HwndRenderTargetProperties(
			hwnd,
			SizeU(
				static_cast<UINT32>(rc.right) -
				static_cast<UINT32>(rc.left),
				static_cast<UINT32>(rc.bottom) -
				static_cast<UINT32>(rc.top)
			)),
		&d2d_render_target);
	if (FAILED(hr)) {
		MessageBox(hwnd, L"Failed to create render target", L"Error", MB_OK);
		return 1;
	}
	hr = DWriteCreateFactory(
		DWRITE_FACTORY_TYPE_SHARED,
		__uuidof(IDWriteFactory),
		reinterpret_cast<IUnknown**>(&write_factory)
	);
	if (FAILED(hr)) {
		MessageBox(hwnd, L"Failed to create write factory", L"Error", MB_OK);
		return 1;
	}
	
	hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
	if (FAILED(hr)) {
		MessageBox(hwnd, L"Failed to initialize COM", L"Error", MB_OK);
		return 1;
	}

	hr = CoCreateInstance(
		CLSID_WICImagingFactory,
		nullptr,
		CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(&wic_factory));
	
	if (FAILED(hr)) {
		MessageBox(hwnd, L"Failed to create WIC factory", L"Error", MB_OK);
		return 1;
	}

	hr = load_pieces_and_tiles_bitmaps();

	calculateChessboardRects(rc);

	return S_OK;

}


HRESULT DrawingChessboardInterface::createRenderResources(HWND hwnd, ID2D1Factory7* d2d_factory, ID2D1HwndRenderTarget* d2d_render_target) {
	hr = S_OK;
	if (d2d_render_target == nullptr) {
		RECT rc;
		if (GetClientRect(hwnd, &rc) == FALSE) {
			MessageBox(hwnd, L"Failed to get client rect", L"Error", MB_OK);
			return -1;
		}
		hr = d2d_factory->CreateHwndRenderTarget(
			RenderTargetProperties(),
			HwndRenderTargetProperties(
				hwnd,
				SizeU(
					static_cast<UINT32>(rc.right) -
					static_cast<UINT32>(rc.left),
					static_cast<UINT32>(rc.bottom) -
					static_cast<UINT32>(rc.top)
				)),
			&d2d_render_target);
		if (FAILED(hr)) {
			MessageBox(hwnd, L"Failed to create render target", L"Error", MB_OK);
		}
		calculateChessboardRects(rc);
	}
	
	return hr;
}


HRESULT DrawingChessboardInterface::drawChessboard() {

	hr = createRenderResources(hwnd, d2d_factory, d2d_render_target);
	if (FAILED(hr)) {
		MessageBox(hwnd, L"Failed to get client rect", L"Error", MB_OK);
		return hr;
	}
	hr = d2d_render_target->CreateSolidColorBrush(black_color, &brush);
	if (FAILED(hr)) {
		MessageBox(hwnd, L"Failed to create brush", L"Error", MB_OK);
		SafeRelease(&brush);
		return hr;
	}
	hr = d2d_factory->CreateStrokeStyle(
		StrokeStyleProperties(
			D2D1_CAP_STYLE_ROUND,
			D2D1_CAP_STYLE_ROUND,
			D2D1_CAP_STYLE_ROUND,
			D2D1_LINE_JOIN_MITER,
			10.0f,
			D2D1_DASH_STYLE_SOLID,
			0.0f),
		nullptr,
		0,
		&brush_style);
	if (FAILED(hr)) {
		MessageBox(hwnd, L"Failed to create brush style", L"Error", MB_OK);
		SafeRelease(&brush);
		SafeRelease(&brush_style);
		return hr;
	}
	
	hr = write_factory->CreateTextFormat(
		L"Verdana",
		nullptr,
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		80.0f,
		L"en-us",
		&text_format
	);

	hr = write_factory->CreateTextFormat(
		L"Impact",
		nullptr,
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		30.0f,
		L"en-us",
		&text_format2);

	if (FAILED(hr)) {
		MessageBox(hwnd, L"Failed to create text format", L"Error", MB_OK);
		SafeRelease(&brush);
		SafeRelease(&brush_style);
		return hr;
	}


	d2d_render_target->BeginDraw();

	d2d_render_target->Clear(D2D1::ColorF(D2D1::ColorF::LightCyan));
	
	for (int i{ 0 }; i < 8; ++i) {
		for (int j{ 0 }; j < 8; ++j) {
			d2d_render_target->DrawBitmap(TileBitmaps[(i + j) % 2], chessboard[i * 8 + j]);
		}
	}
	for (int i{ 0 }; i < 8; ++i) {
		d2d_render_target->DrawText(
			numbers[i], wcslen(numbers[i]),
			text_format2,
			D2D1::RectF(chessboard[i * 8].left - tile_height / 2.5f, chessboard[i * 8].top + tile_height / 3, chessboard[i * 8].left, chessboard[i * 8].bottom),
			brush
		);
	}

	// write letters
	for (int i{ 0 }; i < 8; ++i) {
		d2d_render_target->DrawText(
			letters[i], wcslen(letters[i]),
			text_format2,
			D2D1::RectF(chessboard[i].left + tile_height / 2.50f, chessboard[i].bottom , chessboard[i].right, chessboard[i].bottom + tile_height ),
			brush
		);
	}

	
	char picked_symbol;
	for (auto& [index, symbol] : positionToPieceMap) {
		if (index != pickedFigure) {
			if (pulsing && index == pulsing_index) {
				d2d_render_target->SetTransform(D2D1::Matrix3x2F::Scale(1.0f + 0.5f * sin(pulse_value), 0.5f * 1.0f + sin(pulse_value), Point2F(chessboard[index].left + tile_width / 2.0f, chessboard[index].top + tile_height / 2.0f)));
				d2d_render_target->DrawBitmap(PieceBitmaps[symbolToBitmap[symbol]], chessboard[index]);
				d2d_render_target->SetTransform(D2D1::Matrix3x2F::Identity());
			}
			else
			d2d_render_target->DrawBitmap(PieceBitmaps[symbolToBitmap[symbol]], chessboard[index]);
		}
		else {
			picked_symbol = symbol;
		}
	}
	if (isPicked) {
		d2d_render_target->DrawBitmap(PieceBitmaps[symbolToBitmap[picked_symbol]], D2D1::RectF(mouse_x - tile_width / 2.0f, mouse_y - tile_width / 2.0f, mouse_x + tile_width / 2.0f, mouse_y + tile_width / 2.0f));
	}

	d2d_render_target->DrawText(
		currentMessage, wcslen(currentMessage),
		text_format,
		D2D1::RectF(100.0f, 100.0f, 1000.0f, 1000.0f),
		brush
	);

	

		d2d_render_target->EndDraw();

	
	if (FAILED(hr || hr == D2DERR_RECREATE_TARGET)) {
		SafeRelease(&brush);
		SafeRelease(&d2d_render_target);
		return hr;
	}

	return hr;
}

void DrawingChessboardInterface::cleanup() {
	d2d_render_target->Release();
	d2d_factory->Release();
}
