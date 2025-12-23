/* source/draw/sgl_draw_line.c
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

#include <sgl_core.h>
#include <sgl_log.h>
#include <sgl_draw.h>
#include <sgl_math.h>


/**
 * @brief draw a horizontal line with alpha
 * @param surf surface
 * @param y y coordinate
 * @param x1 x start coordinate
 * @param x2 x end coordinate
 * @param width line width
 * @param color line color
 * @param alpha alpha of color
 * @return none
 */
void sgl_draw_fill_hline(sgl_surf_t *surf, int16_t y, int16_t x1, int16_t x2, int16_t width, sgl_color_t color, uint8_t alpha)
{
    sgl_area_t clip;
    sgl_color_t *buf = NULL;
    sgl_area_t coords = {
        .x1 = x1,
        .y1 = y,
        .x2 = x2,
        .y2 = y + width - 1,
    };

    if (!sgl_surf_clip(surf, &coords, &clip)) {
        return;
    }

    for (int y = clip.y1; y <= clip.y2; y++) {
        buf = sgl_surf_get_buf(surf,  clip.x1 - surf->x1, y - surf->y1);
        for (int x = clip.x1; x <= clip.x2; x++, buf++) {
            *buf = alpha == SGL_ALPHA_MAX ? color : sgl_color_mixer(color, *buf, alpha);
        }
    }
}


/**
 * @brief draw a vertical line with alpha
 * @param surf surface
 * @param x x coordinate
 * @param y1 y start coordinate
 * @param y2 y end coordinate
 * @param width line width
 * @param color line color
 * @param alpha alpha of color
 * @return none
 */
void sgl_draw_fill_vline(sgl_surf_t *surf, int16_t x, int16_t y1, int16_t y2, int16_t width, sgl_color_t color, uint8_t alpha)
{
    sgl_area_t clip;
    sgl_color_t *buf = NULL;
    sgl_area_t coords = {
        .x1 = x,
        .y1 = y1,
        .x2 = x + width - 1,
        .y2 = y2,
    };

    if (!sgl_surf_clip(surf, &coords, &clip)) {
        return;
    }

    for (int y = clip.y1; y <= clip.y2; y++) {
        buf = sgl_surf_get_buf(surf,  clip.x1 - surf->x1, y - surf->y1);
        for (int x = clip.x1; x <= clip.x2; x++, buf++) {
            *buf = (alpha == SGL_ALPHA_MAX ? color : sgl_color_mixer(color, *buf, alpha));
        }
        buf += surf->pitch;
    }
}

#include <math.h>//待添加SGL sqrtf函数，支持后取消使用
typedef struct {
    int32_t x1, y1, x2, y2; // 裁剪矩形
} Scissor;
/**
 * SDF draw anti-aliased line
 * @param thickness line thickness (in pixels)
 */
void draw_line_sdf(sgl_surf_t *surf, int16_t x1, int16_t y1, int16_t x2, int16_t y2, 
                   int16_t thickness, sgl_color_t color, Scissor scissor) {
    
    float radius = (float)thickness * 0.5f;
    
    // 1. Calculate bounding box of the line (with 1 pixel expansion for anti-aliasing)
    int bb_x_min = (int)(fminf((float)x1, (float)x2) - radius - 1.0f);
    int bb_y_min = (int)(fminf((float)y1, (float)y2) - radius - 1.0f);
    int bb_x_max = (int)(fmaxf((float)x1, (float)x2) + radius + 1.0f);
    int bb_y_max = (int)(fmaxf((float)y1, (float)y2) + radius + 1.0f);

    // 2. Intersect with clipping region (Scissor)
    int render_x_min = sgl_clamp(bb_x_min, scissor.x1, scissor.x2);
    int render_y_min = sgl_clamp(bb_y_min, scissor.y1, scissor.y2);
    int render_x_max = sgl_clamp(bb_x_max, scissor.x1, scissor.x2);
    int render_y_max = sgl_clamp(bb_y_max, scissor.y1, scissor.y2);

    // Line segment vector
    float dx = (float)(x2 - x1);
    float dy = (float)(y2 - y1);
    // Use SGL's sqrt function for calculating line length
    float length = (float)sqrtf((uint32_t)(dx*dx + dy*dy)); // Actual length of the line segment
    
    if (length < 0.0001f) return; // Avoid division by zero

    // Unit direction vector
    float ux = dx / length;
    float uy = dy / length;

    // 3. Iterate only within the clipped bounding box
    for (int y = render_y_min; y <= render_y_max; y++) {
        for (int x = render_x_min; x <= render_x_max; x++) {
            
            // Calculate vector from start point to current pixel
            float vx = (float)x - x1;
            float vy = (float)y - y1;
            
            // Calculate projection parameter t of the point on the line direction
            float t = vx * ux + vy * uy;
            
            // Clamp t to [0, length] range to ensure the projected point is on the line segment
            t = sgl_clamp(t, 0.0f, length);
            
            // Calculate projected point coordinates
            float proj_x = x1 + t * ux;
            float proj_y = y1 + t * uy;
            
            // Calculate shortest distance from point to line segment
            float dist_x = (float)x - proj_x;
            float dist_y = (float)y - proj_y;
            float dist = (float)sqrtf((uint32_t)(dist_x * dist_x + dist_y * dist_y));

            // 4. Anti-aliasing calculation
            // Alpha logic: completely opaque if distance < radius-0.5, completely transparent if distance > radius+0.5
            float edge0 = radius - 0.5f;
            float edge1 = radius + 0.5f;
            float alpha_f = sgl_clamp((edge1 - dist) / (edge1 - edge0), 0.0f, 1.0f);
            
            if (alpha_f > 0.0f) {
                uint8_t alpha = (uint8_t)(alpha_f * 255);
                sgl_color_t *buf = sgl_surf_get_buf(surf, x - surf->x1, y - surf->y1);
                *buf = (alpha == SGL_ALPHA_MAX ? color : sgl_color_mixer(color, *buf, alpha));
            }
        }
    }
}


/**
 * @brief draw a line
 * @param surf surface
 * @param desc line description
 * @return none
 */
void sgl_draw_line(sgl_surf_t *surf, sgl_draw_line_t *desc)
{
    uint8_t alpha = desc->alpha;

    int16_t x1 = desc->start.x;
    int16_t y1 = desc->start.y;
    int16_t x2 = desc->end.x;
    int16_t y2 = desc->end.y;

    if (y1 == y2) {
        sgl_draw_fill_hline(surf, y1, x1, x2, desc->width, desc->color, alpha);
    }
    else if (x1 == x2) {
        sgl_draw_fill_vline(surf, x1, y1, y2, desc->width, desc->color, alpha);
    }
    else {
        // 使用SDF算法绘制抗锯齿斜线
        Scissor scissor = {
            .x1 = surf->x1,
            .y1 = surf->y1,
            .x2 = surf->x2,
            .y2 = surf->y2
        };
        
        draw_line_sdf(surf, x1, y1, x2, y2, desc->width, desc->color, scissor);
    }
}
