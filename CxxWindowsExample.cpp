#include <Windows.h>
#include <functional>
#include <string>
#include "Parser.cpp"
#include "Rasterizer.cpp"

int WindowWidth = 800;
int WindowHeight = 500;
HWND Window;
TT::Font* font;

LRESULT CALLBACK EventHandler(HWND, UINT, WPARAM, LPARAM);
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
    FILE* file = nullptr;
    std::wstring filename = ...; //(!!!) SET PATH TO THE .ttf FILE HERE 
    _wfopen_s(&file, filename.c_str(), L"rb");
    font = TT::ParseFont(file);

    ::SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_SYSTEM_AWARE); //needed for correct visualization 

    WNDCLASSW wc{ 0 };
    wc.lpfnWndProc = EventHandler;
    wc.hInstance = hInstance;
    wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
    wc.lpszClassName = L"Window";
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.hIcon = NULL;
    wc.hCursor = NULL;
    wc.lpszMenuName = NULL;

    RegisterClassW(&wc);

    Window = CreateWindowW(L"Window", L"Window", (WS_POPUP | WS_MINIMIZEBOX), 100, 100, ::WindowWidth, ::WindowHeight, NULL, NULL, hInstance, NULL);

    ShowWindow(Window, SW_NORMAL);

    MSG message;
    while (GetMessage(&message, NULL, 0, 0))
    {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }

    delete font;

    return 0;
}

LRESULT CALLBACK EventHandler(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_SIZE)
    {
        RECT clientRectangle;
        GetClientRect(hwnd, &clientRectangle);
    }
    else if (message == WM_DISPLAYCHANGE)
    {
        UpdateWindow(hwnd);
    }
    else if (message == WM_PAINT)
    {
        RECT rc;
        GetClientRect(Window, &rc);
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(Window, &ps);

        BYTE bitmapinfo[FIELD_OFFSET(BITMAPINFO, bmiColors) + (3 * sizeof(DWORD))];
        BITMAPINFOHEADER& header = *(BITMAPINFOHEADER*)bitmapinfo;
        header.biSize = sizeof(BITMAPINFOHEADER);
        header.biWidth = WindowWidth;
        header.biHeight = WindowHeight;
        header.biPlanes = 1;
        header.biBitCount = 32;
        header.biCompression = BI_RGB;
        header.biSizeImage = 0;
        header.biClrUsed = 0;
        header.biClrImportant = 0;

        unsigned char* TT_Canvas = new unsigned char[WindowWidth * WindowHeight * 4];

        for (int i = 0; i < WindowWidth * WindowHeight * 4; i++)
        {
            TT_Canvas[i] = 255;
        }

        TT::DrawString(
                L"Example string",
                font,
                TT_Canvas,
                TT_Rasterizer::ColorComponentOrder::BGRA, //SetDIBitsToDevice() uses BGR order
                WindowWidth,
                WindowHeight,
                30, //x (in pixels)
                30, //y (in pixels)
                80, //size (in pixels)
                TT::Colors.CornflowerBlue);

        SetDIBitsToDevice(hdc, 0, 0, WindowWidth, WindowHeight, 0, 0, 0, WindowHeight, TT_Canvas, (BITMAPINFO*)&header, DIB_RGB_COLORS);

        delete[] TT_Canvas;

        EndPaint(hwnd, &ps);

        return 0;
    }
    else if (message == WM_DESTROY)
    {
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(hwnd, message, wParam, lParam);
}
