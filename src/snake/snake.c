#include "pub.h"

static float speed_grade_cycle[] = {
    1.2,
    1.0,
    0.8,
    0.6,
    0.4,
    0.2,
    0.1,
    0.08,
    0.06,
    0.04,
};

static pixblk_subobj_snake_t *g_snake[2];

static snake_body_t * snake_get_head(pixblk_subobj_snake_t *snake)
{
    cpkl_listhead_t *p;

    if (CPKL_LISTISEMPLY(&(snake->snake_head))) {
        return NULL;
    }
    p = snake->snake_head.next;
    return CPKL_GETCONTAINER(p, snake_body_t, snake_entry);
}

static snake_body_t * snake_get_tail(pixblk_subobj_snake_t *snake)
{
    cpkl_listhead_t *p;

    if (CPKL_LISTISEMPLY(&(snake->snake_head))) {
        return NULL;
    }
    p = snake->snake_head.prev;
    return CPKL_GETCONTAINER(p, snake_body_t, snake_entry);
}

static int snake_add_head(pixblk_subobj_snake_t *snake, pg_pos_t *pos)
{
    snake_body_t *body = (snake_body_t *)malloc(sizeof(snake_body_t));

    body->pos = *pos;
    cpkl_listadd(&(body->snake_entry), &(snake->snake_head));
    snake->n_entry++;

    snake->pixblk.pix_bm[(int)(pos->y)][(int)(pos->x)] = PIXBLK_DRAWPATTEN_NORMAL;

    return 0;
}

static int snake_add_tail(pixblk_subobj_snake_t *snake, pg_pos_t *pos)
{
    snake_body_t *body = (snake_body_t *)malloc(sizeof(snake_body_t));

    body->pos = *pos;
    cpkl_listadd_tail(&(body->snake_entry), &(snake->snake_head));
    snake->n_entry++;

    snake->pixblk.pix_bm[(int)(pos->y)][(int)(pos->x)] = PIXBLK_DRAWPATTEN_NORMAL;

    return 0;
}

static int snake_del_tail(pixblk_subobj_snake_t *snake)
{
    snake_body_t *body = snake_get_tail(snake);

    if (body == NULL) {
        return 0;
    }

    snake->n_entry--;
    cpkl_listdel(&(body->snake_entry));
    snake->pixblk.pix_bm[(int)(body->pos.y)][(int)(body->pos.x)] = PIXBLK_DRAWPATTEN_NONE;

    free(body);

    return 0;
}

static inline void snake_speedchange(pixblk_subobj_snake_t *snake, int speed_up)
{
    if (speed_up) {
        snake->speed_grade++;
        if (snake->speed_grade >= CPKL_ARRAY_SIZE(speed_grade_cycle)) {
            snake->speed_grade = CPKL_ARRAY_SIZE(speed_grade_cycle) - 1;
        }
    } else {
        snake->speed_grade--;
        if (snake->speed_grade < 0) {
            snake->speed_grade = 0;
        }
    }
}

static inline void snake_shrink(pixblk_subobj_snake_t *snake)
{
    if (snake->n_entry <= 4) {
        return;
    }

    int n = cpkl_ri_rand(1, 4);

    if (snake->snake_idx == 0) {
        cpkl_printf("snake %d shrink %d->%d\n",
            snake->snake_idx, snake->n_entry, snake->n_entry - n);
    } else {
        cpkl_printf("                              snake %d shringk %d->%d\n",
            snake->snake_idx, snake->n_entry, snake->n_entry - n);
    }

    while (n--) {
        snake_del_tail(snake);
    }
}

static int snake_add_dest_food(pixblk_subobj_snake_t *snake, snake_food_type_e food_type)
{
    int idx;
    uint32_t rand_num, x, y;
    pg_pos_t pos;
    uint32_t rand_list[PIXBLK_ENGINE_MAX_Y * PIXBLK_ENGINE_MAX_X], n_possible = 0;

    idx = snake_destfood_find(snake, food_type);
    if (idx != -1) {
        /* already exist, do nothing */
        return 0;
    }
    idx = snake_destfood_emptyslot(snake);
    if (idx == -1) {
        /* no empty slot, skip */
        return 0;
    }

    for (y = 0; y < snake->pixblk.pix_bm_height; y++) {
        for (x = 0; x < snake->pixblk.pix_bm_width; x++) {
            if (snake->pixblk.pix_bm[y][x] == PIXBLK_DRAWPATTEN_NONE) {
                rand_list[n_possible] = y * PIXBLK_ENGINE_MAX_X + x;
                n_possible++;
            }
        }
    }

    rand_num = cpkl_ri_rand(0, n_possible);
    pos.x = rand_list[rand_num] % PIXBLK_ENGINE_MAX_X;
    pos.y = rand_list[rand_num] / PIXBLK_ENGINE_MAX_X;

    switch (food_type) {
    case SNAKE_FOOD_TYPE_NORMAL:
        snake->dest_food_list[idx].type = food_type;
        snake->dest_food_list[idx].pos = pos;
        snake->dest_food_list[idx].valid = 1;
        snake->pixblk.pix_bm[(int)(pos.y)][(int)(pos.x)] = PIXBLK_DRAWPATTEN_NORMAL_BLINK_RED;
        break;
    case SNAKE_FOOD_TYPE_OP_SPEEDUP:
        snake->dest_food_list[idx].type = food_type;
        snake->dest_food_list[idx].pos = pos;
        snake->dest_food_list[idx].valid = 1;
        snake->pixblk.pix_bm[(int)(pos.y)][(int)(pos.x)] = PIXBLK_DRAWPATTEN_NORMAL_BLINK_GREEN;
        break;
    case SNAKE_FOOD_TYPE_OP_SNAKE_SHRINK:
        snake->dest_food_list[idx].type = food_type;
        snake->dest_food_list[idx].pos = pos;
        snake->dest_food_list[idx].valid = 1;
        snake->pixblk.pix_bm[(int)(pos.y)][(int)(pos.x)] = PIXBLK_DRAWPATTEN_NORMAL_BLINK_BLUE;
        break;
    default:
        break;
    }

    return 1;
}

static int snake_del_dest_food(pixblk_subobj_snake_t *snake, int food_idx)
{
    snake_destfood_desc_t *dest_food = &(snake->dest_food_list[food_idx]);

    if (dest_food->valid == 0) {
        return 0;
    }

    dest_food->valid = 0;
    snake->pixblk.pix_bm[(int)(dest_food->pos.y)][(int)(dest_food->pos.x)] = PIXBLK_DRAWPATTEN_NONE;

    return 0;
}

static int snake_collision_check(pixblk_subobj_snake_t *snake, pg_pos_t *pos)
{
    snake_body_t *p, *n;
    int cnt = 0;

    if ((pos->x < 0) || (pos->x >= snake->pixblk.pix_bm_width)) {
        return 1;
    }

    if ((pos->y < 0) || (pos->y >= snake->pixblk.pix_bm_height)) {
        return 1;
    }

    CPKL_LISTENTRYWALK_SAVE(p, n, snake_body_t, &(snake->snake_head), snake_entry) {
        if (pg_is_same_pos(pos, &(p->pos))) {
            return 1;
        }
        cnt++;
    }

    assert(cnt == snake->n_entry);

    return 0;
}

/*
 * return 1: eat food, need to insert new body
 *        0: not eat food
 */
static int snake_eat_food_proc(pixblk_subobj_snake_t *snake, pg_pos_t *pos)
{
    int i, ret = 0;
    snake_destfood_desc_t *destfood;
    for (i = 0; i < SUBOBJ_SNAKE_DESTFOOD_MAX; i++) {
        destfood = &(snake->dest_food_list[i]);
        if (destfood->valid == 0) {
            continue;
        }

        /**/
        if (pg_is_same_pos(pos, &(destfood->pos)) == 0) {
            continue;
        }

        switch (destfood->type) {
        case SNAKE_FOOD_TYPE_NORMAL:
            ret = 1;
            snake_del_dest_food(snake, i);
            break;
        case SNAKE_FOOD_TYPE_OP_SPEEDUP:
            ret = 0;
            snake_del_dest_food(snake, i);
            /* opponent speed up */
            snake_speedchange(g_snake[snake->snake_idx ^ 1], 1);
            snake_speedchange(g_snake[snake->snake_idx ^ 1], 1);

            break;
        case SNAKE_FOOD_TYPE_OP_SNAKE_SHRINK:
            ret = 0;
            snake_del_dest_food(snake, i);

            snake_shrink(g_snake[snake->snake_idx ^ 1]);
            break;
        default:
            break;
        }

        return ret;
    }

    return ret;
}

static int snake_normal_step(pixblk_subobj_snake_t *snake)
{
    snake_body_t *body;
    pg_pos_t next_pos;

    if (!CPKL_LISTISEMPLY(&(snake->snake_head))) {
        switch(snake->snake_direct) {
        case SUBOBJ_SNAKE_OP_UP:
            body = snake_get_head(snake);
            next_pos.x = body->pos.x;
            next_pos.y = body->pos.y - 1;
            break;
        case SUBOBJ_SNAKE_OP_DOWN:
            body = snake_get_head(snake);
            next_pos.x = body->pos.x;
            next_pos.y = body->pos.y + 1;
            break;
        case SUBOBJ_SNAKE_OP_LEFT:
            body = snake_get_head(snake);
            next_pos.x = body->pos.x - 1;
            next_pos.y = body->pos.y;
            break;
        case SUBOBJ_SNAKE_OP_RIGHT:
            body = snake_get_head(snake);
            next_pos.x = body->pos.x + 1;
            next_pos.y = body->pos.y;
            break;
        default:
            assert(0);
        }

        /* collision check */
        if (snake_collision_check(snake, &next_pos)) {
            snake_body_t *p, *n;
            CPKL_LISTENTRYWALK_SAVE(p, n, snake_body_t, &(snake->snake_head), snake_entry) {
                snake->pixblk.pix_bm[(int)(p->pos.y)][(int)(p->pos.x)] = PIXBLK_DRAWPATTEN_NORMAL_BLINK_RED;
            }
            snake->state = SUBOBJ_SNAKE_STOP;

            if (snake->snake_idx == 0) {
                cpkl_printf("snake %d gameover\n", snake->snake_idx);
            } else {
                cpkl_printf("                              snake %d gameover\n", snake->snake_idx);
            }


            return 0;
        }

        if (snake_eat_food_proc(snake, &next_pos)) {
            //
            snake_add_head(snake, &next_pos);

            if (snake->snake_idx == 0) {
                cpkl_printf("snake %d %4d\n", snake->snake_idx, snake->n_entry);
            } else {
                cpkl_printf("                              snake %d %4d\n", snake->snake_idx, snake->n_entry);
            }
        } else {
            snake_del_tail(snake);

            //
            snake_add_head(snake, &next_pos);
        }
    }

    return 0;
}

static int do_snake_reset(pixblk_subobj_snake_t *snake)
{
    int i;
    pg_pos_t pos;

    if (snake->state == SUBOBJ_SNAKE_RUNNING) {
        return 0;
    }

    for (i = 0; i < SUBOBJ_SNAKE_DESTFOOD_MAX; i++) {
        snake_del_dest_food(snake, i);
    }

    snake->state = SUBOBJ_SNAKE_RUNNING;

    snake->speed_grade = 0;
    snake->tick_sum = 0;

    snake->snake_direct = SUBOBJ_SNAKE_OP_RIGHT;
    snake->tick_drive_step_suppress = 0;

    while (!CPKL_LISTISEMPLY(&(snake->snake_head))) {
        snake_del_tail(snake);
    }
    assert(snake->n_entry == 0);

    for (i = 0; i < 3; i++) {
        pos.x = snake->pixblk.pix_bm_width / 2 - i;
        pos.y = snake->pixblk.pix_bm_height / 2;

        snake_add_tail(snake, &pos);
    }

    if (snake->snake_idx == 0) {
        cpkl_printf("snake %d reset\n", snake->snake_idx);
    } else {
        cpkl_printf("                              snake %d reset\n", snake->snake_idx);
    }

    return 0;
}

static int snake_msgfifo_enqueue(pixblk_subobj_snake_t *snake, snake_op_type_e op)
{
    if ((snake->msgfifo.tail - snake->msgfifo.head) >= SUBOBJ_SNAKE_MSGFIFO_LENGTH) {
        cpkl_printf("msg overflow, drop op %d\n", op);
        return -1;
    }

    snake->msgfifo.entry[snake->msgfifo.tail % SUBOBJ_SNAKE_MSGFIFO_LENGTH].type = op;
    snake->msgfifo.tail++;

    return 0;
}

static int snake_msgfifo_dequeue(pixblk_subobj_snake_t *snake)
{
    snake_op_type_e op;

    if (snake->msgfifo.tail == snake->msgfifo.head) {
        return 0;
    }

    op = snake->msgfifo.entry[snake->msgfifo.head % SUBOBJ_SNAKE_MSGFIFO_LENGTH].type;
    snake->msgfifo.head++;

    if ((op != SUBOBJ_SNAKE_OP_RESET) && (snake->state == SUBOBJ_SNAKE_STOP)) {
        return 0;
    }

    switch (op) {
    case SUBOBJ_SNAKE_OP_UP:
        if (snake->snake_direct != SUBOBJ_SNAKE_OP_DOWN) {
            snake->snake_direct = op;
        }
        break;
    case SUBOBJ_SNAKE_OP_DOWN:
        if (snake->snake_direct != SUBOBJ_SNAKE_OP_UP) {
            snake->snake_direct = op;
        }
        break;
    case SUBOBJ_SNAKE_OP_LEFT:
        if (snake->snake_direct != SUBOBJ_SNAKE_OP_RIGHT) {
            snake->snake_direct = op;
        }
        break;
    case SUBOBJ_SNAKE_OP_RIGHT:
        if (snake->snake_direct != SUBOBJ_SNAKE_OP_LEFT) {
            snake->snake_direct = op;
        }
        break;
    case SUBOBJ_SNAKE_OP_SPEEDUP:
        snake_speedchange(snake, 1);
        break;
    case SUBOBJ_SNAKE_OP_SPEEDDOWN:
        snake_speedchange(snake, 0);
        break;
    case SUBOBJ_SNAKE_OP_RESET:
        do_snake_reset(snake);
        break;
    }

    if ((op == SUBOBJ_SNAKE_OP_UP) ||
        (op == SUBOBJ_SNAKE_OP_DOWN) ||
        (op == SUBOBJ_SNAKE_OP_LEFT) ||
        (op == SUBOBJ_SNAKE_OP_RIGHT)) {
        snake_normal_step(snake);
        /* 手动控制移动后抑制一次systick触发的移动, 增强操作手感 */
        snake->tick_drive_step_suppress = 1;
    }

    return 0;
}

static int snake_kbop(pixblk_subobj_snake_t *snake, snake_op_type_e op)
{
    snake_msgfifo_enqueue(snake, op);

    return 0;
}

static int snake_obj_tick(pg_simple_2d_obj_t *obj, float delta_second)
{
    pixblk_obj_t *pixblk = CPKL_GETCONTAINER(obj, pixblk_obj_t, simple_2d_obj);
    pixblk_subobj_snake_t *snake = CPKL_GETCONTAINER(pixblk, pixblk_subobj_snake_t, pixblk);
    int n_tick, ret;

    pixblk_obj_tick(&(snake->pixblk), delta_second);

    snake_msgfifo_dequeue(snake);

    if (snake->state == SUBOBJ_SNAKE_RUNNING) {
        ret = snake_add_dest_food(snake, SNAKE_FOOD_TYPE_NORMAL);

        if (ret) {
            if (cpkl_ri_rand(0, snake->speedup_p) == 0) {
                snake_add_dest_food(snake, SNAKE_FOOD_TYPE_OP_SPEEDUP);
            }

            if (cpkl_ri_rand(0, snake->shrink_p) == 0) {
                snake_add_dest_food(snake, SNAKE_FOOD_TYPE_OP_SNAKE_SHRINK);
            }
        }
    }

    snake->tick_sum++;
    n_tick = speed_grade_cycle[snake->speed_grade] / delta_second;
    if (snake->tick_sum < n_tick) {
        return 0;
    }
    // cpkl_printf("%f\n", snake->tick_sum);

    snake->tick_sum = 0;

    switch (snake->state) {
    case SUBOBJ_SNAKE_INITING:
        break;
    case SUBOBJ_SNAKE_RUNNING:
        if (snake->tick_drive_step_suppress) {
            snake->tick_drive_step_suppress = 0;
        } else {
            snake_normal_step(snake);
        }

        break;
    case SUBOBJ_SNAKE_STOP:
        break;
    }

    return 0;
}

int snake_init(pixblk_subobj_snake_t *snake, pg_pos_t *topleft_pos, int snake_idx)
{
    uint32_t pixblk_raw_black[10 * 10];
    uint32_t pixblk_raw_red[10 * 10];
    uint32_t pixblk_raw_green[10 * 10];
    uint32_t pixblk_raw_glue[10 * 10];

    pixblk_pattern_init(pixblk_raw_black, 0xffffff);
    pixblk_pattern_init(pixblk_raw_red, 0xff0000);
    pixblk_pattern_init(pixblk_raw_green, 0x00ff00);
    pixblk_pattern_init(pixblk_raw_glue, 0x0000ff);

    uint8_t *pixblk_pattern[PIXBLK_DRAWPATTEN_COUNT] = {
        NULL,
        (uint8_t *)pixblk_raw_black,
        (uint8_t *)pixblk_raw_red,
        (uint8_t *)pixblk_raw_green,
        (uint8_t *)pixblk_raw_glue
    };

    memset(snake, 0, sizeof(pixblk_subobj_snake_t));
    pixblk_obj_init(&(snake->pixblk), topleft_pos,
        32, 24, 1, 10,
        pixblk_pattern,
        0xffffff);
    snake->pixblk.simple_2d_obj.obj_tick = snake_obj_tick;
    snake->snake_idx = snake_idx;
    snake->kbop = snake_kbop;

    /* P0: low posibility, P1: high posiblity */
    if (snake_idx == 0) {
        snake->speedup_p = 12;
        snake->shrink_p = 20;
    } else {
        snake->speedup_p = 4;
        snake->shrink_p = 10;
    }

    cpkl_initlisthead(&(snake->snake_head));

    do_snake_reset(snake);

    g_snake[snake_idx] = snake;

    return 0;
}

