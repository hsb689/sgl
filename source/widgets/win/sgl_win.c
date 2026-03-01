/* source/widgets/sgl_win.c
 *
 * MIT License
 *
 * Copyright(c) 2023-present All contributors of SGL  
 * Document reference link: docs directory
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "sgl_win.h"

static void win_close_cb(sgl_event_t *evt)
{
    sgl_win_t *win = (sgl_win_t *)evt->param;
    if (evt->type == SGL_EVENT_RELEASED) {
        sgl_obj_set_destroyed(win->body);
        sgl_obj_set_destroyed(&win->obj);
    }
}

static void sgl_win_construct_cb(sgl_surf_t *surf, sgl_obj_t* obj, sgl_event_t *evt)
{
    sgl_win_t *win = sgl_container_of(obj, sgl_win_t, obj);
    sgl_rect_t bg = obj->coords;
    sgl_obj_t *body = win->body;
    sgl_obj_t *close = win->close;
    sgl_label_t *title_text = (sgl_label_t *)win->title;
    int16_t close_cx, close_cy, close_r;

    win->title_h = sgl_max(obj->radius, win->title_h);
    bg.y1 -= win->title_h;

    if (evt->type == SGL_EVENT_DRAW_MAIN) {
        sgl_area_selfclip(&body->area, &bg);
        sgl_draw_rect(surf, &obj->area, &bg, &win->bg);
        /* FIXME: body should be movable */
        //sgl_obj_set_pos(body, obj->coords.x1, obj->coords.y1 - win->title_h + obj->border);
        //sgl_obj_clear_all_dirty(body);
    }
    else if (evt->type == SGL_EVENT_DRAW_INIT) {
        win->title_h = sgl_max(win->title_h, sgl_font_get_height(title_text->font) + obj->border);
        close_r  = win->title_h * 6 / 8 - obj->border;
        close_cx = sgl_obj_get_width(obj) - close_r - obj->radius / 2;
        close_cy = (win->title_h - close_r + 1) / 2;

        sgl_obj_set_size(body, sgl_obj_get_width(obj), win->title_h + obj->border + obj->radius);
        sgl_obj_set_pos(body, obj->coords.x1, obj->coords.y1 - win->title_h + obj->border);

        sgl_obj_set_size(&title_text->obj, sgl_obj_get_width(obj) - win->title_h, win->title_h);
        sgl_obj_set_pos(&title_text->obj, 0, 0);

        sgl_obj_set_size(close, close_r, close_r);
        sgl_obj_set_pos(close, close_cx, close_cy);
        sgl_circle_set_radius(close, close_r);
        sgl_circle_set_alpha(close, win->bg.alpha);
    }
    else if (evt->type == SGL_EVENT_DESTROYED) {
        sgl_obj_set_destroyed(win->body);
    }
}

/**
 * @brief create a window object
 * @param parent parent of the window
 * @return window object
 */
sgl_obj_t* sgl_win_create(sgl_obj_t* parent)
{
    sgl_win_t *win = sgl_malloc(sizeof(sgl_win_t));
    if(win == NULL) {
        SGL_LOG_ERROR("sgl_win_create: malloc failed");
        return NULL;
    }

    /* set object all member to zero */
    memset(win, 0, sizeof(sgl_win_t));

    sgl_obj_t *obj = &win->obj;
    sgl_obj_init(&win->obj, parent);
    obj->construct_fn = sgl_win_construct_cb;
    obj->needinit = 1;

    win->bg.alpha = SGL_THEME_ALPHA;
    win->bg.color = SGL_THEME_COLOR;
    win->bg.radius = SGL_THEME_RADIUS;
    win->bg.border = 0;
    win->bg.border_color = SGL_THEME_BORDER_COLOR;

    sgl_obj_t *body = sgl_rect_create(obj->parent);
    if (body == NULL) {
        SGL_LOG_ERROR("sgl_win_create: sgl_rect_create failed");
        goto free_obj;
    }
    sgl_obj_t *title_text = sgl_label_create(body);
    if (title_text == NULL) {
        SGL_LOG_ERROR("sgl_win_create: sgl_label_create failed");
        goto free_body;
    }
    sgl_obj_t *close = sgl_circle_create(body);
    if (close == NULL) {
        SGL_LOG_ERROR("sgl_win_create: sgl_circle_create failed");
        goto free_title;
    }
    sgl_circle_set_border_width(close, 0);
    sgl_obj_set_event_cb(close, win_close_cb, win);
    sgl_circle_set_color(close, sgl_rgb(255, 90, 80));

    win->body = body;
    win->title = title_text;
    win->close = close;
    sgl_obj_move_down(body);
    return obj;

free_title:
    sgl_obj_delete(title_text);
free_body:
    sgl_obj_delete(body);
free_obj:
    sgl_obj_delete(obj);
    return NULL;
}
