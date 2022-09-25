#include "pub.h"

static int pg_simple_2d_wind_msg_cb(void *param, UINT msg, WPARAM wParam, LPARAM lParam)
{
    pg_simple_2d_ctx_t *ctx = (pg_simple_2d_ctx_t *)param;
    pg_simple_2d_obj_t *obj = NULL;
    /* get obj */
    if (CPKL_LISTISEMPLY(&(ctx->obj_list_head))) {
        return 0;
    }

    obj = CPKL_GETCONTAINER(ctx->obj_list_head.next, pg_simple_2d_obj_t, obj_list_entry);

    switch (msg) {
    case WM_KEYDOWN:
        if (wParam == 'Q') {
            cpkl_printf("Q\n");
            pg_simple_2d_obj_mv_rotate(obj, -PI/16);
        } else if (wParam == 'E') {
            cpkl_printf("E\n");
            pg_simple_2d_obj_mv_rotate(obj, PI/16);
        } else if (wParam == '9') {
            if (obj->mv.velocity > 0) {
                obj->mv.velocity--;
            }
        } else if (wParam == '0') {
            if (obj->mv.velocity < 100) {
                obj->mv.velocity++;
            }
        }

        break;
    default:
        /* do nothing */
    }

    return 0;
}

int pg_simple_2d_pos_change(pg_simple_2d_obj_t *obj, float delta_second)
{
    obj->topleft_pos.x += obj->mv.dir.x * obj->mv.velocity * delta_second;
    obj->topleft_pos.y += obj->mv.dir.y * obj->mv.velocity * delta_second;

    // cpkl_printf("%f\n", delta_second);

    return 0;
}

static void sys_tick(void *param)
{
    pg_simple_2d_ctx_t *ctx = (pg_simple_2d_ctx_t *)param;
    pg_simple_2d_obj_t *p, *n;
    // cpkl_printf("sys tick %d\n", ctx->sys_tick++);
    screen_clean(&(ctx->window), -0);

    CPKL_LISTENTRYWALK_SAVE(p, n, pg_simple_2d_obj_t, &(ctx->obj_list_head), obj_list_entry) {
        /* update pos */
        pg_simple_2d_pos_change(p, ctx->sys_tick_cycle / 1000.0);

        p->obj_draw(ctx, p);
    }

    screen_update(&(ctx->window));
}

static int pg_simple_2d_init_obj(pg_simple_2d_ctx_t *ctx)
{
    pg_simple_2d_obj_t *test_obj = malloc(sizeof(pg_simple_2d_obj_t));
    pg_simple_2d_obj_init(test_obj, 8, 16);
    memset(test_obj->obj_frame_buf, 0xa0, test_obj->fb_size);
    pg_simple_2d_obj_set_mv(test_obj, PI / 8, 20);
    pg_simple_2d_add_obj(ctx, test_obj);

    return 0;
}

int pg_simple_2d_init(pg_simple_2d_ctx_t *ctx)
{
    TCHAR *title = _T("2d playground");

    memset(ctx, 0, sizeof(pg_simple_2d_ctx_t));
    ctx->sys_tick_cycle = 25;           /* 25ms, 40fps */

    screen_init(&(ctx->window), 800, 600, title, pg_simple_2d_wind_msg_cb, ctx);

    cpkl_initlisthead(&(ctx->obj_list_head));

    pg_simple_2d_init_obj(ctx);

    cpkl_tmlkinit(1);
    cpkl_tmreg(ctx->sys_tick_cycle, sys_tick, ctx);

    return 0;
}

int pg_simple_2d_add_obj(pg_simple_2d_ctx_t *ctx, pg_simple_2d_obj_t *obj)
{
    cpkl_listadd_tail(&(obj->obj_list_entry), &(ctx->obj_list_head));
    ctx->n_obj++;

    return 0;
}

int pg_simple_2d_simple_obj_draw(pg_simple_2d_ctx_t *ctx, pg_simple_2d_obj_t *obj)
{
    int j;
    for (j = 0; j < obj->h; j++) {
        memcpy(ctx->window.screen_fb_line[(int)(obj->topleft_pos.y) + j] + ((int)(obj->topleft_pos.x)) * 4,
            obj->obj_frame_buf_line[j],
            obj->w * 4);
    }
    // cpkl_printf("x %d, y %d\n", obj->topleft_pos.x, obj->topleft_pos.y);

    return 0;
}

int pg_simple_2d_obj_init(pg_simple_2d_obj_t *obj, int w, int h)
{
    int i;
    memset(obj, 0, sizeof(pg_simple_2d_obj_t));

    if (h > PG_SIMPLE_2D_OBJ_MAX_HEIGHT) {
        return -1;
    }

    obj->w = w;
    obj->h = h;
    obj->obj_draw = pg_simple_2d_simple_obj_draw;
    cpkl_initlisthead(&(obj->obj_list_entry));

    obj->obj_frame_buf = malloc(w * h * 4);
    for (i = 0; i < h; i++) {
        obj->obj_frame_buf_line[i] = obj->obj_frame_buf + i * w * 4;
    }
    obj->fb_size = w * h * 4;

    obj->initdone = 1;

    return 0;
}

int pg_simple_2d_obj_set_mv(pg_simple_2d_obj_t *obj, float theta_radian, float velocity)
{
    if (obj->initdone == 0) {
        return -1;
    }

    obj->mv.theta_radian = theta_radian;
    obj->mv.dir.y = tan(theta_radian);
    obj->mv.dir.x = 1.0;
    pg_vector_normalize(&(obj->mv.dir));
    obj->mv.velocity = velocity;

    // cpkl_printf("%f %f %f\n", obj->mv.dir.x, obj->mv.dir.y, obj->mv.velocity);

    return 0;
}

int pg_simple_2d_obj_mv_rotate(pg_simple_2d_obj_t *obj, float theta_radian)
{
    if (obj->initdone == 0) {
        return 0;
    }

    obj->mv.theta_radian += theta_radian;
    obj->mv.dir.y = sin(obj->mv.theta_radian);
    obj->mv.dir.x = cos(obj->mv.theta_radian);
    pg_vector_normalize(&(obj->mv.dir));

    // cpkl_printf("%f %f %f\n", obj->mv.dir.y, obj->mv.dir.x, obj->mv.velocity);

    return 0;
}