#include "pub.h"

static pg_win_ab_t *g_cur_win_ab;

static LRESULT screen_events(HWND hWnd, UINT msg,
	WPARAM wParam, LPARAM lParam) {
    pg_win_ab_t *cur_win_ab;

    if ((g_cur_win_ab == NULL) || (g_cur_win_ab->screen_handle == NULL)) {
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    cur_win_ab = g_cur_win_ab;
	if (msg == WM_CLOSE) {
		cur_win_ab->exit_sig = 1;
	}
	if (((msg == WM_KEYDOWN) || (msg == WM_KEYUP)) && (wParam == VK_ESCAPE)) {
		cur_win_ab->exit_sig = 1;
	}
	if (cur_win_ab->msg_cb) {
		cur_win_ab->msg_cb(cur_win_ab->msg_cb_param, msg, wParam, lParam);
	}

#if 0
	switch (msg) {
	case WM_CLOSE: cur_win_ab->exit_sig = 1; break;
	case WM_KEYDOWN: cur_win_ab->screen_keys[wParam & 511] = 1; break;
	case WM_KEYUP: cur_win_ab->screen_keys[wParam & 511] = 0; break;
	default: return DefWindowProc(hWnd, msg, wParam, lParam);
	}
    if (cur_win_ab->screen_keys[VK_ESCAPE]) {
        cur_win_ab->exit_sig = 1;
    }
	return 0;
#else
	return DefWindowProc(hWnd, msg, wParam, lParam);
#endif
}

// 初始化窗口并设置标题
int screen_init(pg_win_ab_t *wind, int w, int h, const TCHAR *title, wind_msg_cb msg_cb, void *msg_cb_param) {
	WNDCLASS wc = { CS_BYTEALIGNCLIENT, (WNDPROC)screen_events, 0, 0, 0,
		NULL, NULL, NULL, NULL, _T("SCREEN3.1415926") };
	BITMAPINFO bi = { { sizeof(BITMAPINFOHEADER), w, -h, 1, 32, BI_RGB,
		w * h * 4, 0, 0, 0, 0 }  };
	RECT rect = { 0, 0, w, h };
	int wx, wy, sx, sy, i;
	LPVOID ptr;
	HDC hDC;

	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.hInstance = GetModuleHandle(NULL);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	if (!RegisterClass(&wc)) return -1;

	wind->msg_cb = msg_cb;
	wind->msg_cb_param = msg_cb_param;

	wind->screen_handle = CreateWindow(_T("SCREEN3.1415926"), title,
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		0, 0, 0, 0, NULL, NULL, wc.hInstance, NULL);
	if (wind->screen_handle == NULL) return -2;

    wind->exit_sig = 0;
	hDC = GetDC(wind->screen_handle);
	wind->screen_dc = CreateCompatibleDC(hDC);
	ReleaseDC(wind->screen_handle, hDC);

	wind->screen_hb = CreateDIBSection(wind->screen_dc, &bi, DIB_RGB_COLORS, &ptr, 0, 0);
	if (wind->screen_hb == NULL) return -3;

	wind->screen_ob = (HBITMAP)SelectObject(wind->screen_dc, wind->screen_hb);
	wind->screen_fb = (unsigned char*)ptr;
	wind->screen_w = w;
	wind->screen_h = h;
	for (i = 0; i < h; i++) {
		wind->screen_fb_line[i] = wind->screen_fb + w * i * 4;
	}

	AdjustWindowRect(&rect, GetWindowLong(wind->screen_handle, GWL_STYLE), 0);
	wx = rect.right - rect.left;
	wy = rect.bottom - rect.top;
	sx = (GetSystemMetrics(SM_CXSCREEN) - wx) / 2;
	sy = (GetSystemMetrics(SM_CYSCREEN) - wy) / 2;
	if (sy < 0) sy = 0;
	SetWindowPos(wind->screen_handle, NULL, sx, sy, wx, wy, (SWP_NOCOPYBITS | SWP_NOZORDER | SWP_SHOWWINDOW));
	SetForegroundWindow(wind->screen_handle);

	ShowWindow(wind->screen_handle, SW_NORMAL);
	// screen_dispatch();

	memset(wind->screen_keys, 0, sizeof(int) * 512);
	memset(wind->screen_fb, 0, w * h * 4);
    wind->fb_size = w * h * 4;

    g_cur_win_ab = wind;

	return 0;
}

int screen_unint(pg_win_ab_t *wind) {
	if (wind->screen_dc) {
		if (wind->screen_ob) {
			SelectObject(wind->screen_dc, wind->screen_ob);
			wind->screen_ob = NULL;
		}
		DeleteDC(wind->screen_dc);
		wind->screen_dc = NULL;
	}
	if (wind->screen_hb) {
		DeleteObject(wind->screen_hb);
		wind->screen_hb = NULL;
	}
	if (wind->screen_handle) {
		CloseWindow(wind->screen_handle);
		wind->screen_handle = NULL;
	}

    g_cur_win_ab = NULL;
	return 0;
}

void screen_dispatch() {
	MSG msg;
	while (1) {
		if (!PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) break;
		if (!GetMessage(&msg, NULL, 0, 0)) break;
		DispatchMessage(&msg);
	}
}

void screen_update(pg_win_ab_t *wind) {
	HDC hDC = GetDC(wind->screen_handle);
	BitBlt(hDC, 0, 0, wind->screen_w, wind->screen_h, wind->screen_dc, 0, 0, SRCCOPY);
	ReleaseDC(wind->screen_handle, hDC);
}

void screen_clean(pg_win_ab_t *wind, int clean_color) {
    memset(wind->screen_fb, clean_color, wind->fb_size);
}