#ifndef _PIXBLK_ENGINE_H_
#define _PIXBLK_ENGINE_H_

#define PIXBLK_ENGINE_MAX_X                 256
#define PIXBLK_ENGINE_MAX_Y                 256
#define PIXBLK_MAXWIDTH                     64

typedef enum {
    PIXBLK_DRAWPATTEN_NONE = 0,
    PIXBLK_DRAWPATTEN_NORMAL,
    PIXBLK_DRAWPATTEN_NORMAL_BLINK,
    PIXBLK_DRAWPATTEN_COUNT
} pixblk_draw_pattern_e;

struct _pixblk_obj;

typedef struct _pixblk_obj {
    pg_simple_2d_obj_t              simple_2d_obj;

    uint32_t        pix_bm_width, pix_bm_height;
    pixblk_draw_pattern_e   pix_bm[PIXBLK_ENGINE_MAX_Y][PIXBLK_ENGINE_MAX_X];

    uint32_t        blink_flag;
    int             tick_sum;
    float           blink_cycle;

    uint32_t        interval_width;             /* number of pix */

    uint32_t        pixblk_width;
    struct {
        uint8_t         pixblk_buf[PIXBLK_MAXWIDTH][PIXBLK_MAXWIDTH * 4];
        uint8_t         *pixblk_buf_line[PIXBLK_MAXWIDTH];
    } pixblk_pattern[PIXBLK_DRAWPATTEN_COUNT];

    uint32_t        frameline_pix_color;
} pixblk_obj_t;

int pixblk_obj_init(pixblk_obj_t *pixblk, pg_pos_t *topleft_pos, uint32_t pix_bm_width, uint32_t pix_bm_height, uint32_t interval_width, uint32_t pixblk_width, uint8_t *pkxblk_buf[], uint32_t frameline_pix_color);
int pixblk_obj_tick(pixblk_obj_t *pixblk, float delta_second);

#endif

