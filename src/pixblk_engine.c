#include "pub.h"

static int pkxblk_obj_draw(pg_win_ab_t *window, pg_simple_2d_obj_t *obj)
{
    pixblk_obj_t *pixblk = CPKL_GETCONTAINER(obj, pixblk_obj_t, simple_2d_obj);
    int x, y;
    uint32_t topleft_x, topleft_y;

    for (y = 0; y < pixblk->pix_bm_height; y++) {
        topleft_y = 2 + obj->topleft_pos.y + y * (pixblk->interval_width + pixblk->pixblk_width);
        for (x = 0; x < pixblk->pix_bm_width; x++) {
            topleft_x = 2 + obj->topleft_pos.x + x * (pixblk->interval_width + pixblk->pixblk_width);
            if (pixblk->pix_bm[y][x]) {
                screen_draw_texture(window, pixblk->pixblk_buf_line,
                    topleft_x, topleft_y, pixblk->pixblk_width, pixblk->pixblk_width);
            }
        }
    }

    /* draw frame */
    topleft_y = pixblk->simple_2d_obj.topleft_pos.y;
    for (x = 0; x < pixblk->simple_2d_obj.w; x++) {
        topleft_x = pixblk->simple_2d_obj.topleft_pos.x + x;
        *((uint32_t *)&(window->screen_fb_line[topleft_y][topleft_x * 4])) = pixblk->frameline_pix_color;
    }
    topleft_y = pixblk->simple_2d_obj.topleft_pos.y + pixblk->simple_2d_obj.h - 1;
    for (x = 0; x < pixblk->simple_2d_obj.w; x++) {
        topleft_x = pixblk->simple_2d_obj.topleft_pos.x + x;
        *((uint32_t *)&(window->screen_fb_line[topleft_y][topleft_x * 4])) = pixblk->frameline_pix_color;
    }
    topleft_x = pixblk->simple_2d_obj.topleft_pos.x;
    for (y = 0; y < pixblk->simple_2d_obj.h; y++) {
        topleft_y = pixblk->simple_2d_obj.topleft_pos.y + y;
        *((uint32_t *)&(window->screen_fb_line[topleft_y][topleft_x * 4])) = pixblk->frameline_pix_color;
    }
    topleft_x = pixblk->simple_2d_obj.topleft_pos.x + pixblk->simple_2d_obj.w - 1;
    for (y = 0; y < pixblk->simple_2d_obj.h; y++) {
        topleft_y = pixblk->simple_2d_obj.topleft_pos.y + y;
        *((uint32_t *)&(window->screen_fb_line[topleft_y][topleft_x * 4])) = pixblk->frameline_pix_color;
    }

    return 0;
}

static int pkxblk_obj_tick(pg_simple_2d_obj_t *obj, float delta_second)
{
    pixblk_obj_t *pixblk = CPKL_GETCONTAINER(obj, pixblk_obj_t, simple_2d_obj);
    static int i;

    pixblk->pix_bm[(i / pixblk->pix_bm_width) % pixblk->pix_bm_height][i % pixblk->pix_bm_width] ^= 1;
    i++;

    return 0;
}

int pixblk_obj_init(pixblk_obj_t *pixblk, pg_pos_t *topleft_pos, uint32_t pix_bm_width, uint32_t pix_bm_height, uint32_t interval_width, uint32_t pixblk_width, uint8_t *pkxblk_buf, uint32_t frameline_pix_color)
{
    int y;
    memset(pixblk, 0, sizeof(pixblk_obj_t));

    pixblk->simple_2d_obj.w = (interval_width + pixblk_width) * (pix_bm_width - 1) + pixblk_width + 4;
    pixblk->simple_2d_obj.h = (interval_width + pixblk_width) * (pix_bm_height - 1) + pixblk_width + 4;
    pixblk->simple_2d_obj.topleft_pos = *topleft_pos;
    pixblk->simple_2d_obj.obj_draw = pkxblk_obj_draw;
    pixblk->simple_2d_obj.obj_tick = pkxblk_obj_tick;

    pixblk->pix_bm_width = pix_bm_width;
    pixblk->pix_bm_height = pix_bm_height;
    pixblk->interval_width = interval_width;

    pixblk->pixblk_width = pixblk_width;
    if (pkxblk_buf) {
        for (y = 0; y < pixblk_width; y++) {
            memcpy(pixblk->pixblk_buf[y], pkxblk_buf + y * pixblk_width * 4, pixblk_width * 4);
            pixblk->pixblk_buf_line[y] = pixblk->pixblk_buf[y];
        }
    } else {

    }
    pixblk->frameline_pix_color = frameline_pix_color;

    pixblk->simple_2d_obj.initdone = 1;

    return 0;
}

