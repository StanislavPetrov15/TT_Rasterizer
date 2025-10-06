#include <Windows.h>
#include <winuser.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "Parser.c"
#include "Rasterizer.c"

int WindowWidth = 800;
int WindowHeight = 500;
HWND Window;
struct Font* font;

LRESULT CALLBACK EventHandler(HWND, UINT, WPARAM, LPARAM);
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hinstance, LPSTR lpstr, int n)
{
    FILE* file = NULL;
    wchar_t* filename = ...; //(!!!) SET PATH TO THE .ttf FILE HERE
    _wfopen_s(&file, filename, L"rb");
    font = ParseFont(file);

    SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_SYSTEM_AWARE); //needed for correct visualization

    WNDCLASS wc = {};
    wc.lpfnWndProc = EventHandler;
    wc.hInstance = hInstance;
    wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
    wc.lpszClassName = "Window";
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.hIcon = NULL;
    wc.hCursor = NULL;
    wc.lpszMenuName = NULL;

    RegisterClass(&wc);
    Window = CreateWindow("Window", "Window", (WS_POPUP | WS_MINIMIZEBOX), 100, 100, WindowWidth, WindowHeight, NULL, NULL, hInstance, NULL);

    ShowWindow(Window, SW_NORMAL);

    MSG message;
    while (GetMessage(&message, NULL, 0, 0))
    {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }

    free(font);

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

        BITMAPINFOHEADER header;
        header.biSize = sizeof(BITMAPINFOHEADER);
        header.biWidth = WindowWidth;
        header.biHeight = WindowHeight;
        header.biPlanes = 1;
        header.biBitCount = 32;
        header.biCompression = BI_RGB;
        header.biSizeImage = 0;
        header.biClrUsed = 0;
        header.biClrImportant = 0;

        unsigned char* TT_Canvas = malloc(WindowWidth * WindowHeight * 4);

        for (int i = 0; i < WindowWidth * WindowHeight * 4; i++)
        {
            TT_Canvas[i] = 255;
        }

       DrawString(
           L"Example string",
           font,
           TT_Canvas,
           BGRA_ORDER, //SetDIBitsToDevice() uses BGR order
           WindowWidth,
           WindowHeight,
           30, //x (in pixels)
           30, //y (in pixels)
           80, //size (in pixels)
           C_CORNFLOWER_BLUE->R,
           C_CORNFLOWER_BLUE->G,
           C_CORNFLOWER_BLUE->B,
           -1);

        SetDIBitsToDevice(hdc, 0, 0, WindowWidth, WindowHeight, 0, 0, 0, WindowHeight, TT_Canvas, (BITMAPINFO*)&header, DIB_RGB_COLORS);

        free(TT_Canvas);

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

