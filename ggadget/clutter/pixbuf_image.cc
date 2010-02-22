/*
  Copyright 2008 Google Inc.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

/*
  Portions:
  Copyright 2008-2010 Intel Corp.

  Authors:
  Iain Holmes <iain@linux.intel.com>
  Roger WANG <roger.wang@intel.com>
*/

#include <cmath>
#include <string>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <cairo.h>
#include <ggadget/logger.h>
#include <ggadget/color.h>
#include "cairo_graphics.h"
#include "cairo_canvas.h"
#include "pixbuf_image.h"
#include "utilities.h"

namespace ggadget {
namespace clutter {

#define MULT(d,c,a,t) t = c * a + 0x7f; d = ((t >> 8) + t) >> 8;

static void SetSourcePixbuf(cairo_t *cr, GdkPixbuf *pixbuf)
{
  cairo_surface_t *surface;
  cairo_format_t format;
  cairo_user_data_key_t key;
  guint width, height, rowstride;
  int cairo_rowstride;
  int n_channels;
  guchar *data, *pixels;
  int y;

  width = gdk_pixbuf_get_width(pixbuf);
  height = gdk_pixbuf_get_height(pixbuf);
  rowstride = gdk_pixbuf_get_rowstride(pixbuf);
  pixels = gdk_pixbuf_get_pixels(pixbuf);
  n_channels = gdk_pixbuf_get_n_channels (pixbuf);

  if (n_channels == 3) {
    format = CAIRO_FORMAT_RGB24;
  } else {
    format = CAIRO_FORMAT_ARGB32;
  }

  cairo_rowstride = cairo_format_stride_for_width(format, width);
  data = (guchar *) g_malloc(height * cairo_rowstride);
  surface = cairo_image_surface_create_for_data((unsigned char *)data, format,
                                                width, height, cairo_rowstride);
  cairo_surface_set_user_data(surface, &key, data,
                              (cairo_destroy_func_t) g_free);
  for(y = 0; y < cairo_image_surface_get_height(surface); y++) {
    guchar *s = pixels;
    guchar *d = data;
    guint x;

    if (n_channels == 3) {
      for (x = 0; x < width; x++) {
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
        d[0] = s[2];
        d[1] = s[1];
        d[2] = s[0];
#else
        d[1] = s[0];
        d[2] = s[1];
        d[3] = s[2];
#endif
        s += 3;
        d += 4;
      }
    } else {
      guint tmp;

      for (x = 0; x < width; x++) {
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
        MULT(d[0], s[2], s[3], tmp);
        MULT(d[1], s[1], s[3], tmp);
        MULT(d[2], s[0], s[3], tmp);
        d[3] = s[3];
#else
        d[0] = s[3];
        MULT(d[1], s[0], s[3], tmp);
        MULT(d[2], s[1], s[3], tmp);
        MULT(d[3], s[2], s[3], tmp);
#endif

        s += 4;
        d += 4;
      }
    }

    pixels += rowstride;
    data += cairo_rowstride;
  }

  cairo_set_source_surface (cr, surface, 0.0, 0.0);
  cairo_surface_destroy (surface);
}

class PixbufImage::Impl {
 public:
  Impl(const std::string &data, bool is_mask)
      : fully_opaque_(false), width_(0), height_(0), canvas_(NULL) {
    // No zoom for PixbufImage.
    GdkPixbuf *pixbuf = LoadPixbufFromData(data);
    if (pixbuf) {
      int w = gdk_pixbuf_get_width(pixbuf);
      int h = gdk_pixbuf_get_height(pixbuf);
      width_ = w;
      height_ = h;
      if (is_mask) {
        // clone pixbuf with alpha channel and free the old one.
        // black color will be set to fully transparent.
        GdkPixbuf *a_pixbuf = gdk_pixbuf_add_alpha(pixbuf, TRUE, 0, 0, 0);
        g_object_unref(pixbuf);
        pixbuf = a_pixbuf;
      } else if (!gdk_pixbuf_get_has_alpha(pixbuf)) {
        fully_opaque_ = true;
      } else if (gdk_pixbuf_get_colorspace(pixbuf) == GDK_COLORSPACE_RGB &&
                 gdk_pixbuf_get_bits_per_sample(pixbuf) == 8 &&
                 gdk_pixbuf_get_n_channels(pixbuf) == 4) {
        // Check each pixel for opaque.
        int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
        guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);
        fully_opaque_ = true;
        for (int y = 0; y < h && fully_opaque_; ++y) {
          for (int x = 0; x < w; ++x) {
            // The fourth byte in each pixel cell is alpha.
            guchar *p = pixels + y * rowstride + x * 4 + 3;
            if (*p != 255) {
              fully_opaque_ = false;
              break;
            }
          }
        }
      }

      cairo_format_t fmt = (is_mask ? CAIRO_FORMAT_A8 : CAIRO_FORMAT_ARGB32);
      canvas_ = new CairoCanvas(1, width_, height_, fmt);
      // Draw the image onto the canvas.
      cairo_t *cr = canvas_->GetContext();
      SetSourcePixbuf (cr, pixbuf);
      cairo_paint(cr);

      g_object_unref(pixbuf);
    }
  }

  ~Impl() {
    DestroyCanvas(canvas_);
  }

  bool fully_opaque_;
  double width_;
  double height_;
  CairoCanvas *canvas_;
};

// Currently graphics is not used.
PixbufImage::PixbufImage(const CairoGraphics * /* graphics */,
                         const std::string &tag, const std::string &data,
                         bool is_mask)
  : CairoImageBase(tag, is_mask),
    impl_(new Impl(data, is_mask)) {
}

PixbufImage::~PixbufImage() {
  delete impl_;
  impl_ = NULL;
}

bool PixbufImage::IsValid() const {
  return impl_->canvas_ != NULL;
}

CanvasInterface *PixbufImage::GetCanvas() const {
  return impl_->canvas_;
}

double PixbufImage::GetWidth() const {
  return impl_->width_;
}

double PixbufImage::GetHeight() const {
  return impl_->height_;
}

bool PixbufImage::IsFullyOpaque() const {
  return impl_->fully_opaque_;
}

} // namespace clutter
} // namespace ggadget
