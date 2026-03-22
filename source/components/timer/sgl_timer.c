#include "sgl_timer.h"
#include <sgl_mm.h>

static SGL_LIST_HEAD(sgl_timer_head);

/**
 * @brief create a timer by dynamically allocating memory
 * @return Pointer to the timer structure
 */
sgl_timer_t* sgl_timer_create(void)
{
    return (sgl_timer_t*)sgl_malloc(sizeof(sgl_timer_t));
}


/**
 * @brief delete a timer if your time is created dynamically
 * @param timer Pointer to the timer structure to be removed
 * @return 0 if successful, -1 if failed
 */
int sgl_timer_delete(sgl_timer_t *timer)
{
    if (timer == NULL) {
        return -1;
    }

    if (timer->count != 0) {
        sgl_list_del_node(&timer->node);
    }
    sgl_free(timer);
    return 0;
}


/**
 * @brief setup a timer
 * @param timer Pointer to the timer structure
 * @param callback Callback function to be called when timer expires
 * @param interval Timer interval in ticks, ms
 * @param repeat_cnt Repeat count, -1 for infinite
 * @param user_data User data passed to callback function
 * @return true if successful, false if failed
 * @note Timer will be inserted in ascending order by interval
 */
bool sgl_timer_setup(sgl_timer_t *timer, sgl_timer_callback_t callback, uint32_t interval, int32_t repeat_cnt, void *user_data)
{
    if (timer == NULL || callback == NULL || interval == 0) {
        return false;
    }

    if (timer->callback != NULL) {
        return false;
    }

    timer->callback = callback;
    timer->user_data = user_data;
    timer->interval = interval;
    timer->count = repeat_cnt;

    sgl_list_add_node_at_tail(&sgl_timer_head, &timer->node);
    timer->last_tick = sgl_tick_get();

    return true;
}


/**
 * @brief Timer handler function, should be called periodically
 * @note This function checks all timers and executes callbacks if expired
 * @warning Must be called frequently enough to not miss timer events
 */
void sgl_timer_handler(void)
{
    uint32_t curr_tick = sgl_tick_get();
    sgl_timer_t *pos = NULL;
    sgl_timer_t *pos_n = NULL;

    sgl_list_for_each_entry_safe(pos, pos_n, &sgl_timer_head, sgl_timer_t, node) {

        if (curr_tick - pos->last_tick < pos->interval) {
            continue;
        }

        pos->callback(pos, pos->user_data);
        pos->last_tick = curr_tick;

        if (pos->count > 0) {
            pos->count--;
        }

        if (pos->count == 0) {
            sgl_list_del_node(&pos->node);
        }
    }
}
