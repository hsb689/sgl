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
#include <math.h>

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

/**
 * SDF draw anti-aliased line
 * @param thickness line thickness (in pixels)
 */
static int32_t sgl_capsule_sdf_optimized(int16_t px, int16_t py, int16_t ax, int16_t ay, int16_t bx, int16_t by)
{
	int64_t pax = px - ax, pay = py - ay, bax = bx - ax, bay = by - ay;
	int64_t b_sqd = bax * bax + bay * bay;
	int64_t h = (sgl_max(sgl_min((pax * bax + pay * bay), b_sqd), 0)) << 8;
	int64_t dx = (pax << 8) - bax * h / b_sqd;
	int64_t dy = (pay << 8) - bay * h / b_sqd;

	return sqrt(dx * dx + dy * dy);
}

void draw_line_sdf(sgl_surf_t *surf, sgl_area_t *area, int16_t x1, int16_t y1, int16_t x2, int16_t y2,
                                                  int16_t thickness, sgl_color_t color, uint8_t alpha)
{
	uint8_t c;
	int64_t len;
	sgl_area_t clip = SGL_AREA_MAX;
	sgl_color_t *buf = NULL;
	int16_t thick_half = (thickness >> 1);
	sgl_area_t c_rect = {.x1 = x1 - thick_half, .x2 = x2 + thick_half, .y1 = y1 - thick_half,.y2 = y2 + thick_half};

	sgl_surf_clip_area_return(surf, area, &clip);
	if (!sgl_area_selfclip(&clip, &c_rect)) {
		return;
	}

	for (int y = clip.y1; y <= clip.y2; y++) {
		buf = sgl_surf_get_buf(surf, clip.x1 - surf->x1, y - surf->y1);

		for (int x = clip.x1; x <= clip.x2; x++, buf++) {
			len = sgl_capsule_sdf_optimized(x, y, x1, y1, x2, y2);

			if (len <= (thick_half - 1) << 8) {
				*buf = (alpha == SGL_ALPHA_MAX ? color : sgl_color_mixer(color, *buf, alpha));
				continue;
			}

			if (len > ((thick_half - 1) << 8) && len < (thick_half << 8)) {
				c = len - ((thick_half - 1) << 8);

				if (alpha == SGL_ALPHA_MAX)
					*buf = sgl_color_mixer(*buf, color, c);
				else
					*buf = sgl_color_mixer(sgl_color_mixer(*buf, color, c), *buf, alpha);
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
void sgl_draw_line(sgl_surf_t *surf, sgl_area_t *area, sgl_draw_line_t *desc)
{
	uint8_t alpha = desc->alpha;
	int16_t x1 = desc->start.x;
	int16_t y1 = desc->start.y;
	int16_t x2 = desc->end.x;
	int16_t y2 = desc->end.y;

	draw_line_sdf(surf, area, x1, y1, x2, y2, desc->width, desc->color, alpha);
}
