/* source/widgets/sgl_ext_img.h
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

#ifndef __SGL_EXT_IMG_H__
#define __SGL_EXT_IMG_H__

#include <sgl_core.h>
#include <sgl_draw.h>
#include <sgl_math.h>
#include <sgl_log.h>
#include <sgl_mm.h>
#include <sgl_cfgfix.h>
#include <string.h>

/**
 * description:
 * 1. Extern Flash image object:
 *      you can use this object to draw image from external flash memory
 *      for example:
 *          void flash_port_read_data_from_flash(uint8_t *addr, uint8_t *buf, uint32_t len)
 *          {
 *              //read the data to the buf from the addr of flash
 *              Flash_Read(addr, buf, len);
 *          }
 *          sgl_pixmap_t test_pixmap = {
 *              .width = 142,
 *              .height = 69,
 *              .bitmap = addr_of_extern_flash,
 *              .format = SGL_PIXMAP_FMT_RGB565,
 *          };
 *          sgl_obj_t *ext_img = sgl_ext_img_create(NULL);
 *          sgl_obj_set_pos(ext_img, 10, 10);
 *          sgl_obj_set_size(ext_img, 142, 69);
 *          sgl_ext_img_set_read_ops(ext_img, flash_port_read_data_from_flash);
 *  
 * 2. RLE compress image object:
 *      you can use this object to draw image from external flash memory
 *      for example:
            extern const uint8_t pixmap_data[19596];
 *          sgl_pixmap_t test_pixmap = {
 *              .width = 142,
 *              .height = 69,
 *              .bitmap = pixmap_data,
 *              .format = SGL_PIXMAP_FMT_RLE_RGB565,
 *          };
 *          sgl_obj_t *ext_img = sgl_ext_img_create(NULL);
 *          sgl_obj_set_pos(ext_img, 10, 10);
 *          sgl_obj_set_size(ext_img, 142, 69);
 */


/**
 * @brief sgl ext_img struct
 * @obj: sgl general object
 * @desc: pointer to ext_img draw descriptor
 */
typedef struct sgl_ext_img {
    sgl_obj_t       obj;
    const sgl_pixmap_t *pixmap;
    void            (*read)(const uint8_t *addr, uint8_t *out, uint32_t len_bytes);
    uint8_t         alpha;
    uint8_t         bitmap_num;
    uint8_t         bitmap_idx;
    uint8_t         bitmap_auto;
    /* RLE compress context */
    sgl_color_t     color;
    uint16_t        remainder;
    uint32_t        index;
}sgl_ext_img_t;

/**
 * @brief create an ext_img object
 * @param parent parent of the ext_img
 * @return ext_img object
 */
sgl_obj_t* sgl_ext_img_create(sgl_obj_t* parent);

/**
 * @brief set ext_img pixmap
 * @param obj ext_img object
 * @param pixmap ext_img pixmap
 * @return none
 */
static inline void sgl_ext_img_set_pixmap(sgl_obj_t *obj, const sgl_pixmap_t *pixmap)
{
    SGL_ASSERT(obj != NULL);
    ((sgl_ext_img_t*)obj)->pixmap = pixmap;
}

/**
 * @brief set ext_img read operation
 * @param obj ext_img object
 * @param read ext_img read operation
 * @return none
 */
static inline void sgl_ext_img_set_read_ops(sgl_obj_t *obj, void (*read)(const uint8_t *addr, uint8_t *out, uint32_t len_bytes))
{
    SGL_ASSERT(obj != NULL);
    ((sgl_ext_img_t*)obj)->read = read;
}

/**
 * @brief set ext_img alpha
 * @param obj ext_img object
 * @param alpha ext_img alpha
 * @return none
 */
static inline void sgl_ext_img_set_alpha(sgl_obj_t *obj, uint8_t alpha)
{
    SGL_ASSERT(obj != NULL);
    ((sgl_ext_img_t*)obj)->alpha = alpha;
    sgl_obj_set_dirty(obj);
}

/**
 * @brief set ext_img bitmap number
 * @param obj ext_img object
 * @param num ext_img bitmap number
 * @param auto_refresh ext_img bitmap auto refresh
 * @return none
 * @note if auto_refresh is true, the ext_img will refresh automatically after bitmap flush conplete
 */
static inline void sgl_ext_img_set_bitmap_num(sgl_obj_t *obj, uint8_t num, bool auto_refresh)
{
    SGL_ASSERT(obj != NULL);
    ((sgl_ext_img_t*)obj)->bitmap_num = num;
    ((sgl_ext_img_t*)obj)->bitmap_auto = auto_refresh ? 1 : 0;
}

/**
 * @brief set ext_img next bitmap
 * @param obj ext_img object
 * @return none
 */
static inline void sgl_ext_img_set_next_bitmap(sgl_obj_t *obj)
{
    SGL_ASSERT(obj != NULL);
    sgl_ext_img_t *ext_img = (sgl_ext_img_t*)obj;
    uint8_t index = ext_img->index + 1;
    ext_img->bitmap_idx = (index < ext_img->bitmap_num ? index : 0);
}

#endif // !__SGL_EXT_IMG_H__
