
/*
EmbedDIBits.
AlphaBlend-ed octocat example based on the 4k framework.
*/


#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <mmsystem.h>

#include "example.h"


// Include generated sprite and declare octocat information:

#include "octocat.h"

HDC octocat_hdc;
HBITMAP octocat_bmp;

#define octocat_w 320
#define octocat_h 320

static const BITMAPINFO sprbmi = {
    { sizeof(BITMAPINFOHEADER), octocat_w, -octocat_h, 1, 32, BI_RGB, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0 }
};


// Alpha blending, semi-transparent:

BLENDFUNCTION blender = { AC_SRC_OVER, 0, 127, AC_SRC_ALPHA };


// Message handler:

LRESULT CALLBACK
WndProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CLOSE:
            PostQuitMessage(0);
            return 0;
        default:
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
}


// Initialization and shutdown:

bool
Initialize () {

    // no WinMain so get the module handle:
    g_hInstance = GetModuleHandle(NULL);
    if (g_hInstance == NULL)
        return false;

    // register the window class:
    WNDCLASS wc;

    wc.style         = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = g_hInstance;
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = WINDOW_TITLE;

    if (RegisterClass(&wc) == 0)
        return false;

    // create the window:
    g_hWnd = CreateWindow(
        WINDOW_TITLE,                                             // class name
        WINDOW_TITLE,                                             // title
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, // style
        CW_USEDEFAULT, CW_USEDEFAULT,                             // position
        CW_USEDEFAULT, CW_USEDEFAULT,                             // size
        NULL,                                                     // no parent
        NULL,                                                     // no menu
        g_hInstance,                                              // instance
        NULL                                                      // no special
    );

    if (g_hWnd == NULL)
        return false;

    // setup double buffering:
    g_hDC = GetDC(g_hWnd);
    if (g_hDC == NULL)
        return false;

    g_hMemDC = CreateCompatibleDC(g_hDC);
    if (g_hMemDC == NULL)
        return false;

    g_hMemBMP = CreateCompatibleBitmap(g_hDC, WINDOW_WIDTH, WINDOW_HEIGHT);
    if (g_hMemBMP == NULL)
        return false;

    SelectObject(g_hMemDC, g_hMemBMP);

    // color used to draw the background:
    g_hBlue = CreateSolidBrush(RGB(100, 149, 237));
    if (g_hBlue == NULL)
        return FALSE;

    // setup octocat:
    octocat_hdc = CreateCompatibleDC(g_hDC);
    if (octocat_hdc == NULL)
        return FALSE;

    octocat_bmp = CreateCompatibleBitmap(g_hDC, octocat_w, octocat_h);
    if (octocat_bmp == NULL)
        return FALSE;

    SelectObject(octocat_hdc, octocat_bmp);
    SetDIBits(octocat_hdc, octocat_bmp, 0, 320, octocat, &sprbmi, DIB_RGB_COLORS);

    return true;
}


void
Shutdown (UINT uExitCode) {

    // release resources:
    if (octocat_bmp != NULL)
        DeleteObject(octocat_bmp);

    if (octocat_hdc != NULL)
        DeleteDC(octocat_hdc);

    if (g_hBlue != NULL)
        DeleteObject(g_hBlue);

    if (g_hMemBMP != NULL)
        DeleteObject(g_hMemBMP);

    if (g_hMemDC != NULL)
        DeleteDC(g_hMemDC);

    if (g_hDC != NULL)
        ReleaseDC(g_hWnd, g_hDC);

    // destroy the window and unregister the class:
    if (g_hWnd != NULL)
        DestroyWindow(g_hWnd);

    WNDCLASS wc;
    if (GetClassInfo(g_hInstance, WINDOW_TITLE, &wc) != 0)
        UnregisterClass(WINDOW_TITLE, g_hInstance);

    // without WinMainCRTStartup() we must exit the process ourselves:
    ExitProcess(uExitCode);
}


 // Update step and main loop:

void
Update () {

    RECT rect = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
    FillRect(g_hMemDC, &rect, g_hBlue);

    // draw the octocat to the back buffer, with alpha blending:
    AlphaBlend(g_hMemDC, 0, 0, octocat_w, octocat_h, octocat_hdc,
                         0, 0, octocat_w, octocat_h, blender);

}


void
Loop () {
    MSG msg;
    bool done = false;

    // this game loop is far from ideal since rendering/logic are tied
    // and timeGetTime() and Sleep() are not precise, but it is
    // small and works well enough for simple 4k games/demos

    // for a better game loop see:
    // <http://gafferongames.com/game-physics/fix-your-timestep>

    while (!done) {
        DWORD dwStart = timeGetTime();

        // poll windows events:
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) != 0) {
            if (msg.message == WM_QUIT)
                done = true;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // update and copy the back buffer:
        Update();
        BitBlt(g_hDC, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, g_hMemDC, 0, 0, SRCCOPY);

        // sleep until next step:
        DWORD dwDelta = timeGetTime() - dwStart;
        if (dwDelta < STEP_RATE) {
            Sleep(STEP_RATE - dwDelta);
        }
    }
}


// A helper to resize the window with respect to the client area:

void
ResizeClientWindow (HWND hWnd, UINT uWidth, UINT uHeight) {
    RECT rcClient, rcWindow;

    GetClientRect(hWnd, &rcClient);
    GetWindowRect(hWnd, &rcWindow);

    MoveWindow(hWnd,
               rcWindow.left,
               rcWindow.top,
               uWidth + (rcWindow.right - rcWindow.left) - rcClient.right,
               uHeight + (rcWindow.bottom - rcWindow.top) - rcClient.bottom,
               FALSE);
}


// Entry point:

void
EntryPoint () {
    if (!Initialize()) {
        MessageBox(NULL, "Initialization failed.", "Error", MB_OK | MB_ICONERROR);
        Shutdown(1);
    }

    ResizeClientWindow(g_hWnd, WINDOW_WIDTH, WINDOW_HEIGHT);
    ShowWindow(g_hWnd, SW_SHOW);
    Loop();
    Shutdown(0);
}

