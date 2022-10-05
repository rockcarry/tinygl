#include <windows.h>
#include <pthread.h>
#include "defines.h"
#include "texture.h"
#include "wingdi.h"

typedef struct {
    HWND      hwnd;
    HDC       hdc;
    HBITMAP   hbmp;
    TEXTURE   texture;
    pthread_t hthread;
    #define FLAG_CLOSED (1 << 0)
    uint32_t  flags;
} WINGDI;

#define TINYGL_WND_CLASS TEXT("TinyGLWndClass")
#define TINYGL_WND_NAME  TEXT("TinyGLWindow"  )

static void gdi_texture_lock(TEXTURE *t) {}

static void gdi_texture_unlock(TEXTURE *t)
{
    WINGDI *win = container_of(t, WINGDI, texture);
    InvalidateRect(win->hwnd, NULL, FALSE);
}

static LRESULT CALLBACK WINGDI_WNDPROC(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps = {0};
    HDC        hdc = NULL;
    WINGDI    *win = (WINGDI*)GetWindowLong(hwnd, GWL_USERDATA);
    switch (uMsg) {
    case WM_TIMER: case WM_KEYUP: case WM_KEYDOWN: case WM_SYSKEYUP: case WM_SYSKEYDOWN: case WM_MOUSEMOVE: case WM_MOUSEWHEEL:
    case WM_LBUTTONUP: case WM_LBUTTONDOWN: case WM_RBUTTONUP: case WM_RBUTTONDOWN: case WM_MBUTTONUP: case WM_MBUTTONDOWN:
        return 0;
    case WM_PAINT:
        hdc = BeginPaint(hwnd, &ps);
        BitBlt(hdc, ps.rcPaint.left, ps.rcPaint.top,
            ps.rcPaint.right  - ps.rcPaint.left,
            ps.rcPaint.bottom - ps.rcPaint.top,
            win->hdc, ps.rcPaint.left, ps.rcPaint.top, SRCCOPY);
        EndPaint(hwnd, &ps);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

static void* wingdi_thread_proc(void *param)
{
    WINGDI   *win  = (WINGDI*)param;
    WNDCLASS  wc   = {0};
    RECT      rect = {0};
    MSG       msg  = {0};
    int       x, y, w, h;

    wc.lpfnWndProc   = WINGDI_WNDPROC;
    wc.hInstance     = GetModuleHandle(NULL);
    wc.hIcon         = LoadIcon  (NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
    wc.lpszClassName = TINYGL_WND_CLASS;
    if (!RegisterClass(&wc)) return NULL;

    win->hwnd = CreateWindow(TINYGL_WND_CLASS, TINYGL_WND_NAME, WS_SYSMENU|WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, win->texture.w, win->texture.h,
        NULL, NULL, wc.hInstance, NULL);
    if (!win->hwnd) goto done;

    SetWindowLong(win->hwnd, GWL_USERDATA, (LONG)win);
    GetClientRect(win->hwnd, &rect);
    w = win->texture.w + (win->texture.w - rect.right );
    h = win->texture.h + (win->texture.h - rect.bottom);
    x = (GetSystemMetrics(SM_CXSCREEN) - w) / 2;
    y = (GetSystemMetrics(SM_CYSCREEN) - h) / 2;
    x = x > 0 ? x : 0;
    y = y > 0 ? y : 0;

    MoveWindow  (win->hwnd, x, y, w, h, FALSE);
    ShowWindow  (win->hwnd, SW_SHOW);
    UpdateWindow(win->hwnd);

    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage (&msg);
    }

done:
    if (win->hwnd) { DestroyWindow(win->hwnd); win->hwnd = NULL; }
    win->flags |= FLAG_CLOSED;
    return NULL;
}

void* wingdi_init(int w, int h)
{
    WINGDI *win = calloc(1, sizeof(WINGDI));
    if (!win) return NULL;

    BITMAPINFO bmpinfo = {};
    BITMAP     bitmap  = {};

    bmpinfo.bmiHeader.biSize        =  sizeof(bmpinfo);
    bmpinfo.bmiHeader.biWidth       =  w;
    bmpinfo.bmiHeader.biHeight      = -h;
    bmpinfo.bmiHeader.biPlanes      =  1;
    bmpinfo.bmiHeader.biBitCount    =  32;
    bmpinfo.bmiHeader.biCompression =  BI_RGB;

    win->hdc  = CreateCompatibleDC(NULL);
    win->hbmp = CreateDIBSection(win->hdc, &bmpinfo, DIB_RGB_COLORS, (void**)&(win->texture.data), NULL, 0);
    if (!win->hdc || !win->hbmp || !win->texture.data) goto failed;

    GetObject(win->hbmp, sizeof(BITMAP), &bitmap);
    SelectObject(win->hdc, win->hbmp);
    win->texture.w = w;
    win->texture.h = h;
    win->texture.lock   = gdi_texture_lock;
    win->texture.unlock = gdi_texture_unlock;

    pthread_create(&win->hthread, NULL, wingdi_thread_proc, win);
    return win;

failed:
    wingdi_free(win, 0);
    return NULL;
}

void wingdi_free(void *ctx, int close)
{
    WINGDI *win = (WINGDI*)ctx;
    if (close) PostMessage(win->hwnd, WM_CLOSE, 0, 0);
    if (win->hthread) pthread_join(win->hthread, NULL);
    if (win->hdc    ) DeleteDC    (win->hdc );
    if (win->hbmp   ) DeleteObject(win->hbmp);
    free(win);
}

void wingdi_set(void *ctx, char *name, void *data) {}

void* wingdi_get(void *ctx, char *name)
{
    WINGDI *win = (WINGDI*)ctx;
    if (!ctx || !name) return NULL;
    if (strcmp(name, "texture") == 0) return &win->texture;
    if (strcmp(name, "state"  ) == 0) return (win->flags & FLAG_CLOSED) ? "closed" : "running";
    return NULL;
}

#ifdef _TEST_WINGDI_
int main(void)
{
    wingdi_free(wingdi_init(640, 480), 0);
    return 0;
}
#endif
