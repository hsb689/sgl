#include <sgl_core.h>
#include <sgl_draw.h>
#include <sgl_math.h>
#include <sgl_log.h>
#include <sgl_mm.h>
#include <sgl_theme.h>
#include <sgl_cfgfix.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "sgl_scope.h"



// 绘制虚线的辅助函数
static void draw_dashed_line(sgl_surf_t *surf, sgl_draw_line_t *line)
{
    int16_t x0 = line->start.x;
    int16_t y0 = line->start.y;
    int16_t x1 = line->end.x;
    int16_t y1 = line->end.y;
    
    int16_t dx = abs(x1 - x0);
    int16_t dy = abs(y1 - y0);
    int16_t sx = (x0 < x1) ? 1 : -1;
    int16_t sy = (y0 < y1) ? 1 : -1;
    int16_t err = dx - dy;
    int16_t e2;
    
    int16_t dash_len = 0;
    const int16_t dash_pattern = 5; // 虚线段长度
    
    sgl_area_t clip_area = {
        .x1 = surf->x,
        .y1 = surf->y,
        .x2 = surf->x + surf->w - 1,
        .y2 = surf->y + surf->h - 1
    };
    
    while (1) {
        // 绘制虚线段
        if (dash_len < dash_pattern) {
            // 检查点是否在裁剪区域内
            if (x0 >= clip_area.x1 && x0 <= clip_area.x2 && y0 >= clip_area.y1 && y0 <= clip_area.y2) {
                sgl_color_t *buf = sgl_surf_get_buf(surf, x0 - surf->x, y0 - surf->y);
                *buf = line->color;
            }
            dash_len++;
        } else if (dash_len < 2 * dash_pattern) {
            // 跳过绘制段
            dash_len++;
        } else {
            // 重置虚线段计数
            dash_len = 0;
        }
        
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}


static void custom_draw_line(sgl_surf_t *surf, sgl_pos_t start, sgl_pos_t end, sgl_color_t color, int16_t width)
{
    int16_t x0 = start.x;
    int16_t y0 = start.y;
    int16_t x1 = end.x;
    int16_t y1 = end.y;
    
    // 处理线宽为0或负数的情况
    if (width <= 0) return;
    
    // 处理线宽为1的情况，直接使用Bresenham算法
    if (width == 1) {
        int16_t dx = abs(x1 - x0);
        int16_t dy = abs(y1 - y0);
        int16_t sx = (x0 < x1) ? 1 : -1;
        int16_t sy = (y0 < y1) ? 1 : -1;
        int16_t err = dx - dy;
        int16_t e2;
        
        sgl_area_t clip_area = {
            .x1 = surf->x,
            .y1 = surf->y,
            .x2 = surf->x + surf->w - 1,
            .y2 = surf->y + surf->h - 1
        };
        
        while (1) {
            // 检查点是否在裁剪区域内
            if (x0 >= clip_area.x1 && x0 <= clip_area.x2 && y0 >= clip_area.y1 && y0 <= clip_area.y2) {
                sgl_color_t *buf = sgl_surf_get_buf(surf, x0 - surf->x, y0 - surf->y);
                *buf = color;
            }
            
            if (x0 == x1 && y0 == y1) break;
            e2 = 2 * err;
            if (e2 > -dy) {
                err -= dy;
                x0 += sx;
            }
            if (e2 < dx) {
                err += dx;
                y0 += sy;
            }
        }
        return;
    }
    
    // 处理线宽大于1的情况，使用主线条加垂直偏移的方法
    int16_t dx = abs(x1 - x0);
    int16_t dy = abs(y1 - y0);
    int16_t sx = (x0 < x1) ? 1 : -1;
    int16_t sy = (y0 < y1) ? 1 : -1;
    int16_t err = dx - dy;
    int16_t e2;
    
    sgl_area_t clip_area = {
        .x1 = surf->x,
        .y1 = surf->y,
        .x2 = surf->x + surf->w - 1,
        .y2 = surf->y + surf->h - 1
    };
    
    // 计算线宽半径
    int16_t half_width = width / 2;
    
    while (1) {
        // 绘制当前点及其垂直方向上的点（形成线宽效果）
        for (int16_t w = -half_width; w <= half_width; w++) {
            int16_t px, py;
            
            // 根据线段的主要方向确定偏移方向
            if (dx > dy) {  // 主要沿X轴方向
                px = x0;
                py = y0 + w;
            } else {  // 主要沿Y轴方向
                px = x0 + w;
                py = y0;
            }
            
            // 检查点是否在裁剪区域内
            if (px >= clip_area.x1 && px <= clip_area.x2 && py >= clip_area.y1 && py <= clip_area.y2) {
                sgl_color_t *buf = sgl_surf_get_buf(surf, px - surf->x, py - surf->y);
                *buf = color;
            }
        }
        
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}
// 示波器绘制回调函数
static void scope_construct_cb(sgl_surf_t *surf, sgl_obj_t* obj, sgl_event_t *evt)
{
    sgl_scope_t *scope = (sgl_scope_t*)obj;
    
    if(evt->type == SGL_EVENT_DRAW_MAIN) {
        // 检查对象是否在屏幕区域内，避免无效绘制
        if (obj->area.x2 < surf->x || obj->area.x1 >= surf->x + surf->w ||
            obj->area.y2 < surf->y || obj->area.y1 >= surf->y + surf->h) {
            return; // 对象完全在屏幕外，无需绘制
        }
        
        // 绘制背景
        sgl_draw_rect_t bg_rect = {
            .color = scope->bg_color,
            .alpha = scope->alpha,
            .radius = 0,
            .border = 0,
        };
        sgl_draw_rect(surf, &obj->area, &obj->coords, &bg_rect);
        
        // 绘制外边框
        if (scope->show_outer_border) {
            sgl_draw_line_t outer_border_line = {
                .color = scope->outer_border_color,
                .width = 1,
                .alpha = scope->alpha,
            };
            
            // 绘制左边框
            outer_border_line.start.x = obj->coords.x1 - 1;
            outer_border_line.start.y = obj->coords.y1 - 1;
            outer_border_line.end.x = obj->coords.x1 - 1;
            outer_border_line.end.y = obj->coords.y2 + 1;
            sgl_draw_line(surf, &outer_border_line);
            
            // 绘制右边框
            outer_border_line.start.x = obj->coords.x2 + 1;
            outer_border_line.start.y = obj->coords.y1 - 1;
            outer_border_line.end.x = obj->coords.x2 + 1;
            outer_border_line.end.y = obj->coords.y2 + 1;
            sgl_draw_line(surf, &outer_border_line);
            
            // 绘制上边框
            outer_border_line.start.x = obj->coords.x1 - 1;
            outer_border_line.start.y = obj->coords.y1 - 1;
            outer_border_line.end.x = obj->coords.x2 + 1;
            outer_border_line.end.y = obj->coords.y1 - 1;
            sgl_draw_line(surf, &outer_border_line);
            
            // 绘制下边框
            outer_border_line.start.x = obj->coords.x1 - 1;
            outer_border_line.start.y = obj->coords.y2 + 1;
            outer_border_line.end.x = obj->coords.x2 + 1;
            outer_border_line.end.y = obj->coords.y2 + 1;
            sgl_draw_line(surf, &outer_border_line);
        }
        
        // 绘制边框
        sgl_draw_line_t border_line = {
            .color = scope->border_color,
            .width = 1,
            .alpha = scope->alpha,
        };
        
        // 绘制左边框
        border_line.start.x = obj->coords.x1;
        border_line.start.y = obj->coords.y1;
        border_line.end.x = obj->coords.x1;
        border_line.end.y = obj->coords.y2;
        sgl_draw_line(surf, &border_line);
        
        // 绘制右边框
        border_line.start.x = obj->coords.x2;
        border_line.start.y = obj->coords.y1;
        border_line.end.x = obj->coords.x2;
        border_line.end.y = obj->coords.y2;
        sgl_draw_line(surf, &border_line);
        
        // 绘制上边框
        border_line.start.x = obj->coords.x1;
        border_line.start.y = obj->coords.y1;
        border_line.end.x = obj->coords.x2;
        border_line.end.y = obj->coords.y1;
        sgl_draw_line(surf, &border_line);
        
        // 绘制下边框
        border_line.start.x = obj->coords.x1;
        border_line.start.y = obj->coords.y2;
        border_line.end.x = obj->coords.x2;
        border_line.end.y = obj->coords.y2;
        sgl_draw_line(surf, &border_line);
        
        // 计算波形显示参数
        uint16_t display_min = scope->min_value;
        uint16_t display_max = scope->max_value;
        
        // 如果启用自动缩放，则使用运行时极值
        if (scope->auto_scale) {
            display_min = scope->running_min;
            display_max = scope->running_max;
            
            // 添加一些边距使波形不会触及边界
            uint16_t margin = (display_max - display_min) / 10;
            if (margin == 0) margin = 1;
            
            display_min = (display_min > margin) ? display_min - margin : 0;
            display_max = (display_max + margin < 0xFFFF) ? display_max + margin : 0xFFFF;
        }
        
        // 如果最小值和最大值相同，则稍微分开一点避免除零错误
        if (display_min == display_max) {
            if (display_max < 0xFFFF) {
                display_max++;
            } else {
                display_min--;
            }
        }
        
        // 绘制网格线
        sgl_draw_line_t grid_line = {
            .color = scope->grid_color,
            .width = 1,
            .alpha = scope->alpha,
        };
        
        int16_t width = obj->coords.x2 - obj->coords.x1;
        int16_t height = obj->coords.y2 - obj->coords.y1;
        int16_t x_center = (obj->coords.x1 + obj->coords.x2) / 2;
        int16_t y_center = obj->coords.y1 + (int32_t)(height * (display_max - (display_min + display_max) / 2)) / (display_max - display_min);
        
        // 绘制中心水平线（显示范围的中点）
        grid_line.color = scope->grid_color;
        grid_line.start.x = obj->coords.x1;
        grid_line.start.y = y_center;
        grid_line.end.x = obj->coords.x2;
        grid_line.end.y = y_center;
        
        if (scope->grid_style == 1) {
            // 绘制虚线
            draw_dashed_line(surf, &grid_line);
        } else {
            // 绘制实线
            sgl_draw_line(surf, &grid_line);
        }
        
        grid_line.start.x = x_center;
        grid_line.start.y = obj->coords.y1;
        grid_line.end.x = x_center;
        grid_line.end.y = obj->coords.y2;
        
        if (scope->grid_style == 1) {
            // 绘制虚线
            draw_dashed_line(surf, &grid_line);
        } else {
            // 绘制实线
            sgl_draw_line(surf, &grid_line);
        }
        
        // 绘制垂直网格线
        grid_line.color = scope->grid_color;
        for (int i = 1; i < 10; i++) {
            int16_t x_pos = obj->coords.x1 + (width * i / 10);
            grid_line.start.x = x_pos;
            grid_line.start.y = obj->coords.y1;
            grid_line.end.x = x_pos;
            grid_line.end.y = obj->coords.y2;
            
            if (scope->grid_style == 1) {
                // 绘制虚线
                draw_dashed_line(surf, &grid_line);
            } else {
                // 绘制实线
                sgl_draw_line(surf, &grid_line);
            }
        }
        
        // 绘制水平网格线
        for (int i = 1; i < 10; i++) {
            int16_t y_pos = obj->coords.y1 + (height * i / 10);
            grid_line.start.x = obj->coords.x1;
            grid_line.start.y = y_pos;
            grid_line.end.x = obj->coords.x2;
            grid_line.end.y = y_pos;
            
            if (scope->grid_style == 1) {
                // 绘制虚线
                draw_dashed_line(surf, &grid_line);
            } else {
                // 绘制实线
                sgl_draw_line(surf, &grid_line);
            }
        }
        
        // 绘制Y轴标签（仅在启用显示且设置了字体时）
        if (scope->show_y_labels && scope->y_label_font) {
            char label_text[16];
            sgl_area_t text_area = {
                .x1 = obj->coords.x1 + 2,
                .y1 = obj->coords.y1,
                .x2 = obj->coords.x1 + 50,
                .y2 = obj->coords.y2
            };
            
            // 显示最大值
            sprintf(label_text, "%u", display_max);
            sgl_draw_string(surf, &text_area, obj->coords.x1 + 2, obj->coords.y1 + 2, 
                           label_text, scope->y_label_color, scope->alpha, scope->y_label_font);
            
            // 显示最小值
            sprintf(label_text, "%u", display_min);
            sgl_draw_string(surf, &text_area, obj->coords.x1 + 2, obj->coords.y2 - scope->y_label_font->font_height - 2, 
                           label_text, scope->y_label_color, scope->alpha, scope->y_label_font);
            
            // 显示中间值
            uint16_t mid_value = (display_max + display_min) / 2;
            sprintf(label_text, "%u", mid_value);
            sgl_draw_string(surf, &text_area, obj->coords.x1 + 2, y_center - scope->y_label_font->font_height/2, 
                           label_text, scope->y_label_color, scope->alpha, scope->y_label_font);
        }
        
        // 绘制波形数据
        if (scope->data_count > 1) {
            sgl_pos_t start, end;
            
            // 确定要显示的数据点数
            int display_points = scope->max_display_points > 0 ? scope->max_display_points : SCOPE_DATA_SIZE;
            if (display_points > SCOPE_DATA_SIZE) display_points = SCOPE_DATA_SIZE;
            
            // 从右向左绘制波形
            int data_points = scope->data_count < display_points ? scope->data_count : display_points;
            
            // 计算起始点（最右边的点）
            int last_index = (scope->current_index == 0) ? SCOPE_DATA_SIZE - 1 : scope->current_index - 1;
            uint16_t last_value = scope->data[last_index];
            
            // 限制数值在显示范围内
            if (last_value < display_min) last_value = display_min;
            if (last_value > display_max) last_value = display_max;
            
            start.x = obj->coords.x2;  // 最右边
            start.y = obj->coords.y2 - ((int32_t)(last_value - display_min) * height) / (display_max - display_min);
            
            // 绘制波形，从右向左
            for (int i = 1; i < data_points; i++) {
                int index = (scope->current_index >= i) ? scope->current_index - i : SCOPE_DATA_SIZE - (i - scope->current_index);
                int prev_index = (scope->current_index >= i + 1) ? scope->current_index - (i + 1) : SCOPE_DATA_SIZE - (i + 1 - scope->current_index);
                
                uint16_t current_value = scope->data[prev_index];
                
                // 限制数值在显示范围内
                if (current_value < display_min) current_value = display_min;
                if (current_value > display_max) current_value = display_max;
                
                end.x = obj->coords.x2 - (i * width / (data_points - 1));  // 从右向左移动
                end.y = obj->coords.y2 - ((int32_t)(current_value - display_min) * height) / (display_max - display_min);
                
                custom_draw_line(surf, start, end, scope->waveform_color, scope->line_width);
                
                start = end;
            }
        }
    }
}


// 创建示波器对象
sgl_obj_t* sgl_scope_create(sgl_obj_t* parent)
{
    sgl_scope_t *scope = sgl_malloc(sizeof(sgl_scope_t));
    if(scope == NULL) {
        return NULL;
    }
    
    memset(scope, 0, sizeof(sgl_scope_t));
    
    sgl_obj_t *obj = &scope->obj;
    sgl_obj_init(obj, parent);
    obj->construct_fn = scope_construct_cb;
    
    // 初始化默认参数
    scope->waveform_color = sgl_rgb(0, 255, 0);  // 绿色波形
    scope->bg_color = sgl_rgb(0, 0, 0);    // 黑色背景
    scope->grid_color = sgl_rgb(50, 50, 50); // 灰色网格线
    scope->border_color = sgl_rgb(100, 100, 100); // 灰色边框
    scope->outer_border_color = sgl_rgb(150, 150, 150); // 浅灰色外边框
    scope->min_value = 0;
    scope->max_value = 0xFFFF;
    scope->running_min = 0xFFFF;  // 初始化运行时最小值
    scope->running_max = 0;       // 初始化运行时最大值
    scope->auto_scale = 1;  // 默认启用自动缩放
    scope->line_width = 2;  // 默认线条宽度为2
    scope->max_display_points = 0; // 默认显示所有点
    scope->show_y_labels = 0;      // 默认不显示Y轴标签
    scope->alpha = SGL_ALPHA_MAX;  // 默认不透明
    scope->show_outer_border = 0;  // 默认不显示外边框
    scope->grid_style = 0;         // 默认实线网格
    scope->y_label_font = NULL;    // 默认无字体
    scope->y_label_color = sgl_rgb(255, 255, 255); // 默认白色标签
    scope->data_count = 0;  // 初始没有数据
    
    return obj;
}

// 设置示波器数据点 
void sgl_scope_set_data(sgl_obj_t* obj, uint16_t value)
{
    sgl_scope_t *scope = (sgl_scope_t*)obj;
    
    // 更新运行极值（仅当启用 auto_scale）
    if (scope->auto_scale) {
        if (value < scope->running_min) scope->running_min = value;
        if (value > scope->running_max) scope->running_max = value;
    }
    
    scope->data[scope->current_index] = value;
    scope->current_index = (scope->current_index + 1) % SCOPE_DATA_SIZE;
    
    // 更新数据计数
    if (scope->data_count < SCOPE_DATA_SIZE) {
        scope->data_count++;
    }
    
    sgl_obj_set_dirty(obj);
}


// 获取示波器数据点
uint16_t sgl_scope_get_data(sgl_obj_t* obj, int index)
{
    sgl_scope_t *scope = (sgl_scope_t*)obj;
    if (index < 0 || index >= SCOPE_DATA_SIZE) 
        return 0;
    
    return scope->data[index];
}

// 设置示波器最大显示点数
void sgl_scope_set_max_display_points(sgl_obj_t* obj, uint8_t max_points)
{
    sgl_scope_t *scope = (sgl_scope_t*)obj;
    scope->max_display_points = max_points;
    sgl_obj_set_dirty(obj);
}

// 设置示波器波形颜色
void sgl_scope_set_waveform_color(sgl_obj_t* obj, sgl_color_t color)
{
    sgl_scope_t *scope = (sgl_scope_t*)obj;
    scope->waveform_color = color;
    sgl_obj_set_dirty(obj);
}

// 设置示波器背景色
void sgl_scope_set_bg_color(sgl_obj_t* obj, sgl_color_t color)
{
    sgl_scope_t *scope = (sgl_scope_t*)obj;
    scope->bg_color = color;
    sgl_obj_set_dirty(obj);
}

// 设置示波器网格线颜色
void sgl_scope_set_grid_color(sgl_obj_t* obj, sgl_color_t color)
{
    sgl_scope_t *scope = (sgl_scope_t*)obj;
    scope->grid_color = color;
    sgl_obj_set_dirty(obj);
}

// 设置示波器显示范围
void sgl_scope_set_range(sgl_obj_t* obj, uint16_t min_value, uint16_t max_value)
{
    sgl_scope_t *scope = (sgl_scope_t*)obj;
    scope->min_value = min_value;
    scope->max_value = max_value;
    scope->auto_scale = 0;  // 禁用自动缩放
    sgl_obj_set_dirty(obj);
}

// 设置示波器线条宽度
void sgl_scope_set_line_width(sgl_obj_t* obj, uint8_t width)
{
    sgl_scope_t *scope = (sgl_scope_t*)obj;
    scope->line_width = width;
    sgl_obj_set_dirty(obj);
}

// 启用/禁用自动缩放
void sgl_scope_enable_auto_scale(sgl_obj_t* obj, uint8_t enable)
{
    sgl_scope_t *scope = (sgl_scope_t*)obj;
    scope->auto_scale = enable;
    sgl_obj_set_dirty(obj);
}

// 设置示波器透明度
void sgl_scope_set_alpha(sgl_obj_t* obj, uint8_t alpha)
{
    sgl_scope_t *scope = (sgl_scope_t*)obj;
    scope->alpha = alpha;
    sgl_obj_set_dirty(obj);
}

// 设置是否显示Y轴标签
void sgl_scope_show_y_labels(sgl_obj_t* obj, uint8_t show)
{
    sgl_scope_t *scope = (sgl_scope_t*)obj;
    scope->show_y_labels = show;
    sgl_obj_set_dirty(obj);
}

// 设置Y轴标签字体
void sgl_scope_set_y_label_font(sgl_obj_t* obj, const sgl_font_t *font)
{
    sgl_scope_t *scope = (sgl_scope_t*)obj;
    scope->y_label_font = font;
    sgl_obj_set_dirty(obj);
}

// 设置Y轴标签颜色
void sgl_scope_set_y_label_color(sgl_obj_t* obj, sgl_color_t color)
{
    sgl_scope_t *scope = (sgl_scope_t*)obj;
    scope->y_label_color = color;
    sgl_obj_set_dirty(obj);
}

// 设置示波器边框颜色
void sgl_scope_set_border_color(sgl_obj_t* obj, sgl_color_t color)
{
    sgl_scope_t *scope = (sgl_scope_t*)obj;
    scope->border_color = color;
    sgl_obj_set_dirty(obj);
}

// 设置示波器外边框颜色
void sgl_scope_set_outer_border_color(sgl_obj_t* obj, sgl_color_t color)
{
    sgl_scope_t *scope = (sgl_scope_t*)obj;
    scope->outer_border_color = color;
    sgl_obj_set_dirty(obj);
}

// 设置是否显示外边框
void sgl_scope_show_outer_border(sgl_obj_t* obj, uint8_t show)
{
    sgl_scope_t *scope = (sgl_scope_t*)obj;
    scope->show_outer_border = show;
    sgl_obj_set_dirty(obj);
}

// 设置网格线样式（0-实线，1-虚线）
void sgl_scope_set_grid_style(sgl_obj_t* obj, uint8_t style)
{
    sgl_scope_t *scope = (sgl_scope_t*)obj;
    scope->grid_style = style;
    sgl_obj_set_dirty(obj);
}
