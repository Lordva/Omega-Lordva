#include <kandinsky/rect.h>
#include <kandinsky/pixel.h>
#include <string.h>

KDRect KDRectZero = {.x = 0, .y = 0, .width = 0, .height = 0};

void KDFillRect(KDRect rect, KDColor color) {
  KDPoint p;
  for (p.x = rect.x; p.x<(rect.x+rect.width); p.x++) {
    for (p.y = rect.y; p.y<(rect.y+rect.height); p.y++) {
      KDSetPixel(p, color);
    }
  }
}
