#ifndef _PIXBLK_ENGINE_H_
#define _PIXBLK_ENGINE_H_

#define PIXBLK_ENGINE_MAX_X                 256
#define PIXBLK_ENGINE_MAX_Y                 256
#define PIXBLK_MAXWIDTH                     64

typedef struct {
    pg_simple_2d_obj_t              simple_2d_obj;

    uint32_t        pix_bm_width, pix_bm_height;
    uint8_t         pix_bm[PIXBLK_ENGINE_MAX_Y][PIXBLK_ENGINE_MAX_X];

    uint32_t        interval_width;             /* number of pix */

    uint32_t        pixblk_width;
    uint8_t         pixblk_buf[PIXBLK_MAXWIDTH][PIXBLK_MAXWIDTH * 4];
    uint8_t         *pixblk_buf_line[PIXBLK_MAXWIDTH];

    uint32_t        frameline_pix_color;
} pixblk_obj_t;

int pixblk_obj_init(pixblk_obj_t *pixblk, pg_pos_t *topleft_pos, uint32_t pix_bm_width, uint32_t pix_bm_height, uint32_t interval_width, uint32_t pixblk_width, uint8_t *pkxblk_buf, uint32_t frameline_pix_color);

#endif

