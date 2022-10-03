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
            switch (pixblk->pix_bm[y][x]) {
            case PIXBLK_DRAWPATTEN_NONE:
            case PIXBLK_DRAWPATTEN_NORMAL:
                screen_draw_texture(window, pixblk->pixblk_pattern[pixblk->pix_bm[y][x]].pixblk_buf_line,
                    topleft_x, topleft_y, pixblk->pixblk_width, pixblk->pixblk_width);
                break;
            case PIXBLK_DRAWPATTEN_NORMAL_BLINK:
                if (pixblk->blink_flag) {
                    screen_draw_texture(window, pixblk->pixblk_pattern[pixblk->pix_bm[y][x]].pixblk_buf_line,
                        topleft_x, topleft_y, pixblk->pixblk_width, pixblk->pixblk_width);
                }
                break;
            default:
                assert(0);
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

int pixblk_obj_init(pixblk_obj_t *pixblk, pg_pos_t *topleft_pos, uint32_t pix_bm_width, uint32_t pix_bm_height, uint32_t interval_width, uint32_t pixblk_width, uint8_t *pkxblk_buf[], uint32_t frameline_pix_color)
{
    int i, y;
    memset(pixblk, 0, sizeof(pixblk_obj_t));

    pixblk->simple_2d_obj.w = (interval_width + pixblk_width) * (pix_bm_width - 1) + pixblk_width + 4;
    pixblk->simple_2d_obj.h = (interval_width + pixblk_width) * (pix_bm_height - 1) + pixblk_width + 4;
    pixblk->simple_2d_obj.topleft_pos = *topleft_pos;
    pixblk->simple_2d_obj.obj_draw = pkxblk_obj_draw;

    pixblk->pix_bm_width = pix_bm_width;
    pixblk->pix_bm_height = pix_bm_height;

    /* blink param */
    pixblk->blink_flag = 0;
    pixblk->tick_sum = 0;
    pixblk->blink_cycle = 0.1;

    pixblk->interval_width = interval_width;

    pixblk->pixblk_width = pixblk_width;
    for (i = 0; i < PIXBLK_DRAWPATTEN_COUNT; i++) {
        for (y = 0; y < pixblk_width; y++) {
            if (pkxblk_buf[i]) {
                memcpy(pixblk->pixblk_pattern[i].pixblk_buf[y], pkxblk_buf[i] + y * pixblk_width * 4, pixblk_width * 4);
            } else {

            }

            pixblk->pixblk_pattern[i].pixblk_buf_line[y] = pixblk->pixblk_pattern[i].pixblk_buf[y];
        }
    }

    pixblk->frameline_pix_color = frameline_pix_color;

    pixblk->simple_2d_obj.initdone = 1;

    return 0;
}

int pixblk_obj_tick(pixblk_obj_t *pixblk, float delta_second)
{
    int n_tick;

    pixblk->tick_sum++;
    n_tick = pixblk->blink_cycle / delta_second;
    if (pixblk->tick_sum < n_tick) {
        return 0;
    }

    pixblk->blink_flag ^= 1;
    pixblk->tick_sum = 0;

    return 0;
}

