#ifndef _SNAKE_H_
#define _SNAKE_H_

typedef enum {
    SUBOBJ_SNAKE_OP_UP = 0,
    SUBOBJ_SNAKE_OP_DOWN,
    SUBOBJ_SNAKE_OP_LEFT,
    SUBOBJ_SNAKE_OP_RIGHT,
    SUBOBJ_SNAKE_OP_SPEEDUP,
    SUBOBJ_SNAKE_OP_SPEEDDOWN,
    SUBOBJ_SNAKE_OP_RESET,
    SUBOBJ_SNAKE_OP_P1_PRI,
    SUBOBJ_SNAKE_OP_P2_PRI,
} snake_op_type_e;

#define SUBOBJ_SNAKE_MSGFIFO_LENGTH         256
#define SUBOBJ_SNAKE_DESTFOOD_MAX           8

typedef struct {
    snake_op_type_e     type;
} snake_msgfifo_entry_t;

typedef enum {
    SUBOBJ_SNAKE_INITING,
    SUBOBJ_SNAKE_RUNNING,
    SUBOBJ_SNAKE_STOP,
} snake_state_e;

typedef struct {
    pg_pos_t            pos;
    cpkl_listhead_t     snake_entry;
} snake_body_t;

struct _pixblk_subobj_snake;

typedef int (*snake_kbop_cb)(struct _pixblk_subobj_snake *snake, snake_op_type_e op);

typedef enum {
    SNAKE_FOOD_TYPE_INVALID,
    SNAKE_FOOD_TYPE_NORMAL = 1,
    SNAKE_FOOD_TYPE_OP_SPEEDUP,
    SNAKE_FOOD_TYPE_OP_SNAKE_SHRINK,
} snake_food_type_e;

typedef struct {
    int                 valid;
    pg_pos_t            pos;
    snake_food_type_e   type;
} snake_destfood_desc_t;

typedef struct _pixblk_subobj_snake {
    pixblk_obj_t        pixblk;

    snake_state_e       state;

    int                 snake_idx;              /* 0->p1 1->p2 */
    int                 speed_grade;            /* 0, 1, 2, 3 ... */
    int                 tick_sum;

    int                 tick_drive_step_suppress;   /* 抑制systick触发的移动 */

    snake_op_type_e     snake_direct;
    int                 n_entry;
    cpkl_listhead_t     snake_head;

    snake_kbop_cb       kbop;

    struct {
        uint32_t        tail, head;
        snake_msgfifo_entry_t   entry[SUBOBJ_SNAKE_MSGFIFO_LENGTH];
    } msgfifo;

    /*  */
    snake_destfood_desc_t   dest_food_list[SUBOBJ_SNAKE_DESTFOOD_MAX];

    struct {
        int             speedup_p;              /* p = 1/speedup_p */
        int             shrink_p;
    };
} pixblk_subobj_snake_t;

int snake_init(pixblk_subobj_snake_t *snake, pg_pos_t *topleft_pos, int snake_idx);

static inline int snake_destfood_find(pixblk_subobj_snake_t *snake, snake_food_type_e type)
{
    int i;
    for (i = 0; i < SUBOBJ_SNAKE_DESTFOOD_MAX; i++) {
        if ((snake->dest_food_list[i].valid) && (snake->dest_food_list[i].type == type)) {
            return i;
        }
    }

    return -1;
}

static inline int snake_destfood_emptyslot(pixblk_subobj_snake_t *snake)
{
    int i;
    for (i = 0; i < SUBOBJ_SNAKE_DESTFOOD_MAX; i++) {
        if (snake->dest_food_list[i].valid == 0) {
            return i;
        }
    }

    return -1;
}

#endif

