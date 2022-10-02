#ifndef _2DPG_WIN_ABSTRACT_H_
#define _2DPG_WIN_ABSTRACT_H_

#include <windows.h>
#include <tchar.h>

#define PGWINAB_MAX_LINE        2048

typedef unsigned int IUINT32;

typedef int (*wind_msg_cb)(void *param, UINT msg, WPARAM wParam, LPARAM lParam);

typedef struct {
    HWND screen_handle;         // 主窗口 HWND
    HDC screen_dc;              // 配套的 HDC
    HBITMAP screen_hb;          // DIB
    HBITMAP screen_ob;          // 老的 BITMAP

    unsigned char *screen_fb;   // frame buffer
    unsigned char *screen_fb_line[PGWINAB_MAX_LINE];

    int screen_w, screen_h, fb_size;

    volatile int exit_sig;
    int screen_keys[512];       // 当前键盘按下状态

    wind_msg_cb msg_cb;;
    void *msg_cb_param;
} pg_win_ab_t;

int screen_init(pg_win_ab_t *wind, int w, int h, const TCHAR *title, wind_msg_cb msg_cb, void *msg_cb_param);
int screen_unint(pg_win_ab_t *wind);
void screen_dispatch(void);
void screen_update(pg_win_ab_t *wind);
void screen_clean(pg_win_ab_t *wind, int clean_color);
int screen_draw_texture(pg_win_ab_t *window, uint8_t *texture[], uint32_t topleft_pos_x, uint32_t topleft_pos_y, uint32_t width, uint32_t height);

#endif
