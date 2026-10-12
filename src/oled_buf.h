// ===== oled_buf.h — primitivas estilo Adafruit_GFX sobre el buffer de U8g2 (SSD1306 128x64) =====
// Por qué no usar drawBox/drawDisc de U8g2: sus coordenadas son uint8_t, un valor negativo
// (ojo con la mirada a -9 px o un párpado en y = -1) da la vuelta y dibuja en otro lado.
// Acá todo se recorta contra 0..127 x 0..63 y se escribe directo en el buffer que U8g2 manda al panel.
//
// Formato del buffer de U8g2 (modo F, SSD1306): páginas de 8 filas; byte = x + (y >> 3) * 128, bit = y & 7.
// Uso: gfx.attach(u8g2.getBufferPtr()); eyes::render(gfx, ...); u8g2.sendBuffer();
#pragma once
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

class OledBuf {
 public:
  static const int W = 128, H = 64;

  void attach(uint8_t* buf) { buf_ = buf; }

  void fillScreen(uint8_t c) { memset(buf_, c ? 0xFF : 0x00, W * H / 8); }

  void drawPixel(int x, int y, uint8_t c) {
    if ((unsigned)x >= (unsigned)W || (unsigned)y >= (unsigned)H) return;
    uint8_t* p = &buf_[x + (y >> 3) * W];
    uint8_t m = 1 << (y & 7);
    if (c) *p |= m; else *p &= ~m;
  }

  bool getPixel(int x, int y) const {
    if ((unsigned)x >= (unsigned)W || (unsigned)y >= (unsigned)H) return false;
    return buf_[x + (y >> 3) * W] & (1 << (y & 7));
  }

  void drawFastHLine(int x, int y, int w, uint8_t c) {
    if ((unsigned)y >= (unsigned)H || w <= 0) return;
    if (x < 0) { w += x; x = 0; }
    if (x + w > W) w = W - x;
    if (w <= 0) return;
    uint8_t* p = &buf_[x + (y >> 3) * W];
    uint8_t m = 1 << (y & 7);
    if (c) { while (w--) *p++ |= m; }
    else { m = ~m; while (w--) *p++ &= m; }
  }

  void drawFastVLine(int x, int y, int h, uint8_t c) {
    if ((unsigned)x >= (unsigned)W || h <= 0) return;
    if (y < 0) { h += y; y = 0; }
    if (y + h > H) h = H - y;
    for (int e = y + h; y < e; y++) drawPixel(x, y, c);
  }

  void fillRect(int x, int y, int w, int h, uint8_t c) {
    if (w <= 0 || h <= 0) return;
    if (y < 0) { h += y; y = 0; }
    if (y + h > H) h = H - y;
    for (int e = y + h; y < e; y++) drawFastHLine(x, y, w, c);
  }

  void drawRect(int x, int y, int w, int h, uint8_t c) {
    if (w <= 0 || h <= 0) return;
    drawFastHLine(x, y, w, c); drawFastHLine(x, y + h - 1, w, c);
    drawFastVLine(x, y, h, c); drawFastVLine(x + w - 1, y, h, c);
  }

  // Bresenham
  void drawLine(int x0, int y0, int x1, int y1, uint8_t c) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;
    for (;;) {
      drawPixel(x0, y0, c);
      if (x0 == x1 && y0 == y1) break;
      int e2 = 2 * err;
      if (e2 >= dy) { err += dy; x0 += sx; }
      if (e2 <= dx) { err += dx; y0 += sy; }
    }
  }

  void fillCircle(int x0, int y0, int r, uint8_t c) {
    if (r < 0) return;
    drawFastVLine(x0, y0 - r, 2 * r + 1, c);
    fillCircleHalves(x0, y0, r, 3, 0, c);
  }

  void fillRoundRect(int x, int y, int w, int h, int r, uint8_t c) {
    if (w <= 0 || h <= 0) return;
    int rmax = (w < h ? w : h) / 2;
    if (r > rmax) r = rmax;
    if (r < 0) r = 0;
    fillRect(x + r, y, w - 2 * r, h, c);
    fillCircleHalves(x + w - r - 1, y + r, r, 1, h - 2 * r - 1, c);
    fillCircleHalves(x + r, y + r, r, 2, h - 2 * r - 1, c);
  }

  void drawRoundRect(int x, int y, int w, int h, int r, uint8_t c) {
    if (w <= 0 || h <= 0) return;
    int rmax = (w < h ? w : h) / 2;
    if (r > rmax) r = rmax;
    if (r < 0) r = 0;
    drawFastHLine(x + r, y, w - 2 * r, c);
    drawFastHLine(x + r, y + h - 1, w - 2 * r, c);
    drawFastVLine(x, y + r, h - 2 * r, c);
    drawFastVLine(x + w - 1, y + r, h - 2 * r, c);
    int f = 1 - r, ddx = 1, ddy = -2 * r, cx = 0, cy = r;
    int xl = x + r, xr = x + w - r - 1, yt = y + r, yb = y + h - r - 1;
    while (cx < cy) {
      if (f >= 0) { cy--; ddy += 2; f += ddy; }
      cx++; ddx += 2; f += ddx;
      drawPixel(xl - cy, yt - cx, c); drawPixel(xl - cx, yt - cy, c);
      drawPixel(xr + cx, yt - cy, c); drawPixel(xr + cy, yt - cx, c);
      drawPixel(xl - cy, yb + cx, c); drawPixel(xl - cx, yb + cy, c);
      drawPixel(xr + cx, yb + cy, c); drawPixel(xr + cy, yb + cx, c);
    }
  }

  // Relleno por scanlines: ordena por y y recorre los dos tramos (arista larga vs. cortas)
  void fillTriangle(int x0, int y0, int x1, int y1, int x2, int y2, uint8_t c) {
    if (y0 > y1) { swap_(y0, y1); swap_(x0, x1); }
    if (y1 > y2) { swap_(y2, y1); swap_(x2, x1); }
    if (y0 > y1) { swap_(y0, y1); swap_(x0, x1); }
    if (y0 == y2) {
      int a = x0, b = x0;
      if (x1 < a) a = x1; else if (x1 > b) b = x1;
      if (x2 < a) a = x2; else if (x2 > b) b = x2;
      drawFastHLine(a, y0, b - a + 1, c);
      return;
    }
    int dx01 = x1 - x0, dy01 = y1 - y0, dx02 = x2 - x0, dy02 = y2 - y0, dx12 = x2 - x1, dy12 = y2 - y1;
    int32_t sa = 0, sb = 0;
    int y, last = (y1 == y2) ? y1 : y1 - 1;
    for (y = y0; y <= last; y++) {
      int a = x0 + sa / dy01, b = x0 + sb / dy02;
      sa += dx01; sb += dx02;
      if (a > b) swap_(a, b);
      drawFastHLine(a, y, b - a + 1, c);
    }
    sa = (int32_t)dx12 * (y - y1);
    sb = (int32_t)dx02 * (y - y0);
    for (; y <= y2; y++) {
      int a = x1 + sa / dy12, b = x0 + sb / dy02;
      sa += dx12; sb += dx02;
      if (a > b) swap_(a, b);
      drawFastHLine(a, y, b - a + 1, c);
    }
  }

 private:
  uint8_t* buf_ = nullptr;

  static void swap_(int& a, int& b) { int t = a; a = b; b = t; }

  // corners: 1 = mitad derecha, 2 = mitad izquierda; delta estira el centro (para rectángulos redondeados)
  void fillCircleHalves(int x0, int y0, int r, uint8_t corners, int delta, uint8_t c) {
    int f = 1 - r, ddx = 1, ddy = -2 * r, x = 0, y = r, px = x, py = y;
    delta++;
    while (x < y) {
      if (f >= 0) { y--; ddy += 2; f += ddy; }
      x++; ddx += 2; f += ddx;
      if (x < (y + 1)) {
        if (corners & 1) drawFastVLine(x0 + x, y0 - y, 2 * y + delta, c);
        if (corners & 2) drawFastVLine(x0 - x, y0 - y, 2 * y + delta, c);
      }
      if (y != py) {
        if (corners & 1) drawFastVLine(x0 + py, y0 - px, 2 * px + delta, c);
        if (corners & 2) drawFastVLine(x0 - py, y0 - px, 2 * px + delta, c);
        py = y;
      }
      px = x;
    }
  }
};
