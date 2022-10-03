#ifndef _SNAKE_H_
#define _SNAKE_H_

typedef enum {
    SUBOBJ_SNAKE_OP_UP = 0,
    SUBOBJ_SNAKE_OP_DOWN,
    SUBOBJ_SNAKE_OP_LEFT,
    SUBOBJ_SNAKE_OP_RIGHT,
    SUBOBJ_SNAKE_OP_SPEEDUP,
    SUBOBJ_SNAKE_OP_SPEEDDOWN,
} snake_op_type_e;

typedef enum {
    SUBOBJ_SNAKE_RUNNING,
    SUBOBJ_SNAKE_STOP,
    SUBOBJ_SNAKE_NEEDRESET,
} snake_state_e;

typedef struct {
    pg_pos_t            pos;
    cpkl_listhead_t     snake_entry;
} snake_body_t;

struct _pixblk_subobj_snake;

typedef int (*snake_kbop_cb)(struct _pixblk_subobj_snake *snake, snake_op_type_e op);

typedef struct _pixblk_subobj_snake {
    pixblk_obj_t        pixblk;

    snake_state_e       state;

    int                 speed_grade;            /* 0, 1, 2, 3 ... */
    int                 tick_sum;

    snake_op_type_e     snake_direct;
    int                 n_entry;
    cpkl_listhead_t     snake_head;

    snake_kbop_cb       kbop;

    /*  */
    int                 dest_apple_valid;
    pg_pos_t            dest_apple;
} pixblk_subobj_snake_t;

int snake_init(pixblk_subobj_snake_t *snake, pg_pos_t *topleft_pos);
int snake_reset(pixblk_subobj_snake_t *snake);

#endif

