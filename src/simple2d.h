#ifndef _2DPG_SIMPLE2D_H_
#define _2DPG_SIMPLE2D_H_

typedef struct {
    float x, y;
} pg_pos_t;

typedef struct {
    float   x, y;
} pg_vector_t;

static inline void pg_vector_normalize(pg_vector_t *v)
{
    double len = sqrt(v->x * v->x + v->y * v->y);
    v->x /= len;
    v->y /= len;
}

typedef struct {
    float       theta_radian;
    pg_vector_t dir;
    float       velocity;           /* unit/second */
} pg_movement_vector_t;

struct _pg_simple_2d_obj;

typedef int (*pg_obj_draw)(pg_win_ab_t *window, struct _pg_simple_2d_obj *obj);
typedef int (*pg_obj_tick)(struct _pg_simple_2d_obj *obj, float delta_second);


#define PG_SIMPLE_2D_OBJ_MAX_HEIGHT          32

typedef struct _pg_simple_2d_obj {
    int                     initdone;
    int                     w, h;
    pg_pos_t                topleft_pos;
    pg_movement_vector_t    mv;
    pg_obj_draw             obj_draw;
    pg_obj_tick             obj_tick;

    cpkl_listhead_t         obj_list_entry;

    uint8_t                 *obj_frame_buf;
    uint8_t                 *obj_frame_buf_line[PG_SIMPLE_2D_OBJ_MAX_HEIGHT];
    uint32_t                fb_size;
} pg_simple_2d_obj_t;

typedef struct _pg_simple_2d_ctx {
    pg_win_ab_t         window;
    uint64_t            sys_tick_cycle;         /* ms */
    uint64_t            sys_tick_count;         /* count */

    uint32_t            n_obj;
    cpkl_listhead_t     obj_list_head;
} pg_simple_2d_ctx_t;

int pg_simple_2d_init(pg_simple_2d_ctx_t *ctx);
int pg_simple_2d_add_obj(pg_simple_2d_ctx_t *ctx, pg_simple_2d_obj_t *obj);

int pg_simple_2d_pos_change(pg_simple_2d_obj_t *obj, float delta_second);
int pg_simple_2d_obj_init(pg_simple_2d_obj_t *obj, int w, int h);
int pg_simple_2d_obj_set_mv(pg_simple_2d_obj_t *obj, float theta_radian, float velocity);
int pg_simple_2d_obj_mv_rotate(pg_simple_2d_obj_t *obj, float theta_radian);

#endif
