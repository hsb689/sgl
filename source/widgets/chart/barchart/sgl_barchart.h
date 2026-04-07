/* source/widgets/chart/barchart/sgl_barchart.h
 *
 * MIT License
 */

#ifndef __SGL_BARCHART_H__
#define __SGL_BARCHART_H__

#include <sgl_core.h>
#include <sgl_draw.h>
#include <sgl_math.h>
#include <sgl_log.h>
#include <sgl_mm.h>
#include <sgl_cfgfix.h>
#include <string.h>
#include <stdbool.h>
#if (CONFIG_SGL_ANIMATION)
#include <sgl_anim.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define SGL_BARCHART_MAX_AUTO_TICKS      (8)
#define SGL_BARCHART_DEFAULT_DIVISIONS   (4)
#define SGL_BARCHART_OPEN_ANIM_DURATION  (600U)

typedef enum sgl_barchart_axis_id {
    SGL_BARCHART_AXIS_X = 0,
    SGL_BARCHART_AXIS_Y = 1,
} sgl_barchart_axis_id_t;

typedef enum sgl_barchart_open_anim_dir {
    SGL_BARCHART_OPEN_ANIM_NONE        = 0,
    SGL_BARCHART_OPEN_ANIM_FROM_LEFT   = 1,
    SGL_BARCHART_OPEN_ANIM_FROM_BOTTOM = 2,
} sgl_barchart_open_anim_dir_t;

typedef enum sgl_barchart_orientation {
    SGL_BARCHART_ORIENTATION_VERTICAL   = 0,
    SGL_BARCHART_ORIENTATION_HORIZONTAL = 1,
} sgl_barchart_orientation_t;

typedef struct sgl_barchart_axis {
    int32_t           min;
    int32_t           max;
    int32_t           step;
    const sgl_font_t *label_font;
    sgl_color_t       grid_color;
    sgl_color_t       label_color;
    uint8_t           auto_scale   : 1;
    uint8_t           show_grid    : 1;
    uint8_t           grid_dashed  : 1;
    uint8_t           show_labels  : 1;
    uint8_t           show_ticks   : 1;
    uint8_t           reserved     : 3;
    uint8_t           auto_divisions;
    uint8_t           grid_alpha;
    uint8_t           label_alpha;
} sgl_barchart_axis_t;

typedef struct sgl_barchart_series {
    const int32_t *y_data;
    const char    *label;
    sgl_color_t    color;
    uint16_t       point_count;
    uint8_t        alpha;
} sgl_barchart_series_t;

typedef struct sgl_barchart {
    sgl_obj_t              obj;
    uint8_t                alpha;
    sgl_color_t            bg_color;
    uint8_t                bg_alpha;
    sgl_color_t            border_color;
    sgl_barchart_axis_t    x_axis;
    sgl_barchart_axis_t    y_axis;
    uint8_t                series_count;
    sgl_barchart_series_t *series;
    const char           **x_labels;
    uint8_t                x_label_count;
    sgl_area_t             plot_rel_rect;
    int16_t                layout_left_margin;
    int16_t                layout_top_margin;
    int16_t                layout_right_margin;
    int16_t                layout_bottom_margin;
    uint8_t                bar_gap;
    uint8_t                category_gap;
    uint8_t                custom_plot_rect  : 1;
    uint8_t                open_anim_enable  : 1;
    uint8_t                open_anim_playing : 1;
    uint8_t                orientation       : 1;
    uint8_t                open_anim_dir     : 2;
    uint8_t                reserved_flags    : 2;
    uint16_t               open_anim_duration;
    uint32_t               open_anim_start_tick;
#if (CONFIG_SGL_ANIMATION)
    sgl_anim_path_algo_t   open_anim_path;
#endif
} sgl_barchart_t;

sgl_obj_t *sgl_barchart_create(sgl_obj_t *parent);
void sgl_barchart_set_series_count(sgl_obj_t *obj, uint8_t count);
void sgl_barchart_set_series_y_array(sgl_obj_t *obj, uint8_t index,
                                     const int32_t *y_data,
                                     uint16_t point_count);
void sgl_barchart_set_x_labels(sgl_obj_t *obj, const char **labels, uint8_t count);
void sgl_barchart_update(sgl_obj_t *obj);
void sgl_barchart_update_value(sgl_obj_t *obj, uint8_t series_index, uint16_t point_index);

static inline sgl_barchart_axis_t *sgl_barchart_get_axis(sgl_barchart_t *chart, sgl_barchart_axis_id_t axis)
{
    return (axis == SGL_BARCHART_AXIS_Y) ? &chart->y_axis : &chart->x_axis;
}

static inline void sgl_barchart_set_axis_range(sgl_obj_t *obj, sgl_barchart_axis_id_t axis,
                                               int32_t min, int32_t max)
{
    SGL_ASSERT(obj != NULL);
    sgl_barchart_t *chart = sgl_container_of(obj, sgl_barchart_t, obj);
    sgl_barchart_axis_t *a = sgl_barchart_get_axis(chart, axis);
    a->min = min;
    a->max = max;
    a->auto_scale = 0;
    sgl_obj_set_dirty(obj);
}

static inline void sgl_barchart_enable_axis_auto_scale(sgl_obj_t *obj, sgl_barchart_axis_id_t axis, bool enable)
{
    SGL_ASSERT(obj != NULL);
    sgl_barchart_t *chart = sgl_container_of(obj, sgl_barchart_t, obj);
    sgl_barchart_get_axis(chart, axis)->auto_scale = (uint8_t)enable;
    sgl_obj_set_dirty(obj);
}

static inline void sgl_barchart_set_axis_step(sgl_obj_t *obj, sgl_barchart_axis_id_t axis, int32_t step)
{
    SGL_ASSERT(obj != NULL);
    sgl_barchart_t *chart = sgl_container_of(obj, sgl_barchart_t, obj);
    sgl_barchart_get_axis(chart, axis)->step = step;
    sgl_obj_set_dirty(obj);
}

static inline void sgl_barchart_set_axis_auto_divisions(sgl_obj_t *obj, sgl_barchart_axis_id_t axis, uint8_t divisions)
{
    SGL_ASSERT(obj != NULL);
    sgl_barchart_t *chart = sgl_container_of(obj, sgl_barchart_t, obj);
    sgl_barchart_get_axis(chart, axis)->auto_divisions = divisions ? divisions : SGL_BARCHART_DEFAULT_DIVISIONS;
    sgl_obj_set_dirty(obj);
}

static inline void sgl_barchart_enable_axis_grid(sgl_obj_t *obj, sgl_barchart_axis_id_t axis, bool enable)
{
    SGL_ASSERT(obj != NULL);
    sgl_barchart_t *chart = sgl_container_of(obj, sgl_barchart_t, obj);
    sgl_barchart_get_axis(chart, axis)->show_grid = (uint8_t)enable;
    sgl_obj_set_dirty(obj);
}

static inline void sgl_barchart_set_axis_grid_style(sgl_obj_t *obj, sgl_barchart_axis_id_t axis, uint8_t dashed)
{
    SGL_ASSERT(obj != NULL);
    sgl_barchart_t *chart = sgl_container_of(obj, sgl_barchart_t, obj);
    sgl_barchart_get_axis(chart, axis)->grid_dashed = dashed ? 1 : 0;
    sgl_obj_set_dirty(obj);
}

static inline void sgl_barchart_set_axis_grid_color(sgl_obj_t *obj, sgl_barchart_axis_id_t axis,
                                                    sgl_color_t color, uint8_t alpha)
{
    SGL_ASSERT(obj != NULL);
    sgl_barchart_t *chart = sgl_container_of(obj, sgl_barchart_t, obj);
    sgl_barchart_axis_t *a = sgl_barchart_get_axis(chart, axis);
    a->grid_color = color;
    a->grid_alpha = alpha;
    sgl_obj_set_dirty(obj);
}

static inline void sgl_barchart_enable_axis_labels(sgl_obj_t *obj, sgl_barchart_axis_id_t axis, bool enable)
{
    SGL_ASSERT(obj != NULL);
    sgl_barchart_t *chart = sgl_container_of(obj, sgl_barchart_t, obj);
    sgl_barchart_get_axis(chart, axis)->show_labels = (uint8_t)enable;
    sgl_obj_set_dirty(obj);
}

static inline void sgl_barchart_set_axis_label_font(sgl_obj_t *obj, sgl_barchart_axis_id_t axis,
                                                    const sgl_font_t *font)
{
    SGL_ASSERT(obj != NULL);
    sgl_barchart_t *chart = sgl_container_of(obj, sgl_barchart_t, obj);
    sgl_barchart_get_axis(chart, axis)->label_font = font;
    sgl_obj_set_dirty(obj);
}

static inline void sgl_barchart_set_axis_label_color(sgl_obj_t *obj, sgl_barchart_axis_id_t axis,
                                                     sgl_color_t color, uint8_t alpha)
{
    SGL_ASSERT(obj != NULL);
    sgl_barchart_t *chart = sgl_container_of(obj, sgl_barchart_t, obj);
    sgl_barchart_axis_t *a = sgl_barchart_get_axis(chart, axis);
    a->label_color = color;
    a->label_alpha = alpha;
    sgl_obj_set_dirty(obj);
}

static inline void sgl_barchart_enable_axis_ticks(sgl_obj_t *obj, sgl_barchart_axis_id_t axis, bool enable)
{
    SGL_ASSERT(obj != NULL);
    sgl_barchart_t *chart = sgl_container_of(obj, sgl_barchart_t, obj);
    sgl_barchart_get_axis(chart, axis)->show_ticks = (uint8_t)enable;
    sgl_obj_set_dirty(obj);
}

static inline void sgl_barchart_set_bg_color(sgl_obj_t *obj, sgl_color_t color)
{
    SGL_ASSERT(obj != NULL);
    sgl_container_of(obj, sgl_barchart_t, obj)->bg_color = color;
    sgl_obj_set_dirty(obj);
}

static inline void sgl_barchart_set_bg_alpha(sgl_obj_t *obj, uint8_t alpha)
{
    SGL_ASSERT(obj != NULL);
    sgl_container_of(obj, sgl_barchart_t, obj)->bg_alpha = alpha;
    sgl_obj_set_dirty(obj);
}

static inline void sgl_barchart_set_border_color(sgl_obj_t *obj, sgl_color_t color)
{
    SGL_ASSERT(obj != NULL);
    sgl_container_of(obj, sgl_barchart_t, obj)->border_color = color;
    sgl_obj_set_dirty(obj);
}

static inline void sgl_barchart_set_alpha(sgl_obj_t *obj, uint8_t alpha)
{
    SGL_ASSERT(obj != NULL);
    sgl_container_of(obj, sgl_barchart_t, obj)->alpha = alpha;
    sgl_obj_set_dirty(obj);
}

static inline void sgl_barchart_set_plot_area_rel(sgl_obj_t *obj,
                                                  int16_t offset_left,
                                                  int16_t offset_top,
                                                  int16_t width,
                                                  int16_t height)
{
    SGL_ASSERT(obj != NULL);
    sgl_barchart_t *chart = sgl_container_of(obj, sgl_barchart_t, obj);
    if (width <= 0 || height <= 0) {
        chart->custom_plot_rect = 0;
        sgl_obj_set_dirty(obj);
        return;
    }
    chart->plot_rel_rect.x1 = offset_left;
    chart->plot_rel_rect.y1 = offset_top;
    chart->plot_rel_rect.x2 = (int16_t)(offset_left + width - 1);
    chart->plot_rel_rect.y2 = (int16_t)(offset_top + height - 1);
    chart->custom_plot_rect = 1;
    sgl_obj_set_dirty(obj);
}

static inline void sgl_barchart_reset_plot_area(sgl_obj_t *obj)
{
    SGL_ASSERT(obj != NULL);
    sgl_container_of(obj, sgl_barchart_t, obj)->custom_plot_rect = 0;
    sgl_obj_set_dirty(obj);
}

static inline void sgl_barchart_set_layout_padding(sgl_obj_t *obj,
                                                   int16_t left,
                                                   int16_t top,
                                                   int16_t right,
                                                   int16_t bottom)
{
    SGL_ASSERT(obj != NULL);
    sgl_barchart_t *chart = sgl_container_of(obj, sgl_barchart_t, obj);
    chart->layout_left_margin = left;
    chart->layout_top_margin = top;
    chart->layout_right_margin = right;
    chart->layout_bottom_margin = bottom;
    chart->custom_plot_rect = 0;
    sgl_obj_set_dirty(obj);
}

static inline void sgl_barchart_set_series_color(sgl_obj_t *obj, uint8_t index,
                                                 sgl_color_t color, uint8_t alpha)
{
    SGL_ASSERT(obj != NULL);
    sgl_barchart_t *chart = sgl_container_of(obj, sgl_barchart_t, obj);
    if (chart->series == NULL || index >= chart->series_count) {
        SGL_LOG_ERROR("sgl_barchart_set_series_color: invalid index %d", index);
        return;
    }
    chart->series[index].color = color;
    chart->series[index].alpha = alpha;
    sgl_obj_set_dirty(obj);
}

static inline void sgl_barchart_set_series_label(sgl_obj_t *obj, uint8_t index, const char *label)
{
    SGL_ASSERT(obj != NULL);
    sgl_barchart_t *chart = sgl_container_of(obj, sgl_barchart_t, obj);
    if (chart->series == NULL || index >= chart->series_count) {
        SGL_LOG_ERROR("sgl_barchart_set_series_label: invalid index %d", index);
        return;
    }
    chart->series[index].label = label;
    sgl_obj_set_dirty(obj);
}

static inline void sgl_barchart_set_bar_spacing(sgl_obj_t *obj, uint8_t bar_gap, uint8_t category_gap)
{
    SGL_ASSERT(obj != NULL);
    sgl_barchart_t *chart = sgl_container_of(obj, sgl_barchart_t, obj);
    chart->bar_gap = bar_gap;
    chart->category_gap = category_gap;
    sgl_obj_set_dirty(obj);
}

static inline void sgl_barchart_set_orientation(sgl_obj_t *obj, sgl_barchart_orientation_t orientation)
{
    SGL_ASSERT(obj != NULL);
    sgl_barchart_t *chart = sgl_container_of(obj, sgl_barchart_t, obj);
    chart->orientation = (uint8_t)orientation;
    sgl_obj_set_dirty(obj);
}

static inline void sgl_barchart_enable_open_anim(sgl_obj_t *obj, bool enable)
{
    SGL_ASSERT(obj != NULL);
    sgl_barchart_t *chart = sgl_container_of(obj, sgl_barchart_t, obj);
    chart->open_anim_enable = (uint8_t)enable;
    chart->open_anim_playing = 0;
    sgl_obj_set_dirty(obj);
}

static inline void sgl_barchart_set_open_anim_dir(sgl_obj_t *obj, sgl_barchart_open_anim_dir_t dir)
{
    SGL_ASSERT(obj != NULL);
    sgl_container_of(obj, sgl_barchart_t, obj)->open_anim_dir = (uint8_t)dir;
    sgl_obj_set_dirty(obj);
}

static inline void sgl_barchart_set_open_anim_duration(sgl_obj_t *obj, uint16_t duration)
{
    SGL_ASSERT(obj != NULL);
    sgl_container_of(obj, sgl_barchart_t, obj)->open_anim_duration = duration ? duration : SGL_BARCHART_OPEN_ANIM_DURATION;
    sgl_obj_set_dirty(obj);
}

#if (CONFIG_SGL_ANIMATION)
static inline void sgl_barchart_set_open_anim_path(sgl_obj_t *obj, sgl_anim_path_algo_t path_algo)
{
    SGL_ASSERT(obj != NULL);
    sgl_container_of(obj, sgl_barchart_t, obj)->open_anim_path = path_algo;
}
#endif

#ifdef __cplusplus
}
#endif

#endif
