#ifndef _2DPG_WIN_ABSTRACT_H_
#define _2DPG_WIN_ABSTRACT_H_

#include <windows.h>
#include <tchar.h>

typedef unsigned int IUINT32;

typedef struct {
    HWND screen_handle;         // 主窗口 HWND
    HDC screen_dc;              // 配套的 HDC
    HBITMAP screen_hb;          // DIB
    HBITMAP screen_ob;          // 老的 BITMAP
    unsigned char *screen_fb;   // frame buffer

    int screen_w, screen_h, fb_size;

    volatile int exit_sig;
    int screen_keys[512];       // 当前键盘按下状态
} pg_win_ab_t;

int screen_init(pg_win_ab_t *wind, int w, int h, const TCHAR *title);
int screen_unint(pg_win_ab_t *wind);
void screen_dispatch(void);
void screen_update(pg_win_ab_t *wind);
void screen_clean(pg_win_ab_t *wind, int clean_color);

#endif