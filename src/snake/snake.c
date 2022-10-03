#include "pub.h"

static float speed_grade_cycle[] = {
    1.2,
    1.0,
    0.8,
    0.6,
    0.4,
    0.2,
    0.1,
};

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

static int snake_add_dest_food(pixblk_subobj_snake_t *snake)
{
    uint32_t rand_num;
    pg_pos_t pos;
    if (snake->dest_apple_valid) {
        return 0;
    }

    rand_num = cpkl_ri_rand(0, snake->pixblk.pix_bm_width * snake->pixblk.pix_bm_height);
    pos.x = rand_num % snake->pixblk.pix_bm_width;
    pos.y = rand_num / snake->pixblk.pix_bm_width;

    snake->dest_apple = pos;
    snake->dest_apple_valid = 1;
    snake->pixblk.pix_bm[(int)(pos.y)][(int)(pos.x)] = PIXBLK_DRAWPATTEN_NORMAL_BLINK;

    return 0;
}

static int snake_del_dest_food(pixblk_subobj_snake_t *snake)
{
    if (snake->dest_apple_valid == 0) {
        return 0;
    }

    snake->dest_apple_valid = 0;
    snake->pixblk.pix_bm[(int)(snake->dest_apple.y)][(int)(snake->dest_apple.x)] = PIXBLK_DRAWPATTEN_NONE;

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

static int snake_normal_running(pixblk_subobj_snake_t *snake)
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
                snake->pixblk.pix_bm[(int)(p->pos.y)][(int)(p->pos.x)] = PIXBLK_DRAWPATTEN_NORMAL_BLINK;
            }
            snake->state = SUBOBJ_SNAKE_STOP;
            return 0;
        }

        if (pg_is_same_pos(&next_pos, &(snake->dest_apple)) &&
            (snake->dest_apple_valid)) {
            snake_del_dest_food(snake);
        } else {
            snake_del_tail(snake);
        }

        //
        snake_add_head(snake, &next_pos);
    }

    return 0;
}

static int do_snake_reset(pixblk_subobj_snake_t *snake)
{
    int i;
    pg_pos_t pos;

    snake->state = SUBOBJ_SNAKE_RUNNING;

    snake->speed_grade = 0;
    snake->tick_sum = 0;

    snake->snake_direct = SUBOBJ_SNAKE_OP_RIGHT;

    while (!CPKL_LISTISEMPLY(&(snake->snake_head))) {
        snake_del_tail(snake);
    }
    assert(snake->n_entry == 0);

    for (i = 0; i < 3; i++) {
        pos.x = snake->pixblk.pix_bm_width / 2 - i;
        pos.y = snake->pixblk.pix_bm_height / 2;

        snake_add_tail(snake, &pos);
    }

    return 0;
}

static int snake_obj_tick(pg_simple_2d_obj_t *obj, float delta_second)
{
    pixblk_obj_t *pixblk = CPKL_GETCONTAINER(obj, pixblk_obj_t, simple_2d_obj);
    pixblk_subobj_snake_t *snake = CPKL_GETCONTAINER(pixblk, pixblk_subobj_snake_t, pixblk);
    int n_tick;

    pixblk_obj_tick(&(snake->pixblk), delta_second);

    if ((snake->state == SUBOBJ_SNAKE_RUNNING) && (snake->dest_apple_valid == 0)) {
        snake_add_dest_food(snake);
    }

    snake->tick_sum++;
    n_tick = speed_grade_cycle[snake->speed_grade] / delta_second;
    if (snake->tick_sum < n_tick) {
        return 0;
    }
    // cpkl_printf("%f\n", snake->tick_sum);

    snake->tick_sum = 0;

    switch (snake->state) {
    case SUBOBJ_SNAKE_RUNNING:
        snake_normal_running(snake);
        break;
    case SUBOBJ_SNAKE_STOP:
        break;
    case SUBOBJ_SNAKE_NEEDRESET:
        do_snake_reset(snake);
        break;
    }

    return 0;
}

static int snake_kbop(pixblk_subobj_snake_t *snake, snake_op_type_e op)
{
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
        snake->speed_grade++;
        if (snake->speed_grade >= CPKL_ARRAY_SIZE(speed_grade_cycle)) {
            snake->speed_grade = CPKL_ARRAY_SIZE(speed_grade_cycle) - 1;
        }
        break;
    case SUBOBJ_SNAKE_OP_SPEEDDOWN:
        snake->speed_grade--;
        if (snake->speed_grade < 0) {
            snake->speed_grade = 0;
        }
        break;
    }

    return 0;
}

int snake_init(pixblk_subobj_snake_t *snake, pg_pos_t *topleft_pos)
{
    uint32_t pixblk_raw[5 * 5] = {
        0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff,
        0xffffff, 0,      0xffffff, 0     , 0xffffff,
        0xffffff, 0xffffff, 0,      0xffffff, 0xffffff,
        0xffffff, 0,      0xffffff, 0     , 0xffffff,
        0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff
    };
    uint32_t pixblk_raw_read[5 * 5] = {
        0xff0000, 0xff0000, 0xff0000, 0xff0000, 0xff0000,
        0xff0000, 0,      0xff0000, 0     , 0xff0000,
        0xff0000, 0xff0000, 0,      0xff0000, 0xff0000,
        0xff0000, 0,      0xff0000, 0     , 0xff0000,
        0xff0000, 0xff0000, 0xff0000, 0xff0000, 0xff0000
    };

    uint8_t *pixblk_pattern[PIXBLK_DRAWPATTEN_COUNT] = {
        NULL,
        (uint8_t *)pixblk_raw,
        (uint8_t *)pixblk_raw_read,
    };

    memset(snake, 0, sizeof(pixblk_subobj_snake_t));
    pixblk_obj_init(&(snake->pixblk), topleft_pos,
        64, 48, 1, 5,
        pixblk_pattern,
        0xffffff);
    snake->pixblk.simple_2d_obj.obj_tick = snake_obj_tick;

    cpkl_initlisthead(&(snake->snake_head));

    do_snake_reset(snake);

    snake->kbop = snake_kbop;

    return 0;
}

int snake_reset(pixblk_subobj_snake_t *snake)
{
    snake->state = SUBOBJ_SNAKE_NEEDRESET;
    return 0;
}


