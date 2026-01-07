/* source/draw/sgl_draw_icon.c
 *
 * MIT License
 *
 * Copyright(c) 2023-present All contributors of SGL  
 * Document reference link: https://sgl-docs.readthedocs.io
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
 * @brief draw icon with alpha
 * @param surf   surface
 * @param area   area of icon
 * @param coords coords of icon
 * @param icon   icon pixmap
 * @param alpha  alpha of icon
 */
void sgl_draw_icon( sgl_surf_t *surf, sgl_area_t *area, sgl_area_t *coords, const sgl_icon_pixmap_t *icon, uint8_t alpha)
{
    sgl_area_t clip = SGL_AREA_MAX;
    sgl_color_t *buf = NULL, *blend = NULL, color;
    uint16_t w = coords->x2 - coords->x1 + 1;
    uint16_t h = coords->y2 - coords->y1 + 1;
    uint8_t pbuf = 0;

    if (!sgl_area_selfclip(&clip, area)) {
        return;
    }

    uint32_t scale_x = ((icon->width << 10) / w);
    uint32_t scale_y = ((icon->height << 10) / h);
    uint32_t step_x = 0, step_y = 0;

    buf = sgl_surf_get_buf(surf, clip.x1 - surf->x1, clip.y1 - surf->y1);

    for (int y = clip.y1; y <= clip.y2; y++) {
        blend = buf;
        step_y = (scale_y * (y - coords->y1)) >> 10;
        for (int x = clip.x1; x <= clip.x2; x++, blend++) {
            step_x = (scale_x * (x - coords->x1)) >> 10;
            pbuf = icon->bitmap[step_x + step_y * icon->width];
            if (pbuf & 0x80) {
                color = sgl_rgb322_to_color(pbuf);
                *blend = (alpha == SGL_ALPHA_MAX ? color : sgl_color_mixer(color, *blend, alpha));
            }
        }
        buf += surf->w;
    }
}
