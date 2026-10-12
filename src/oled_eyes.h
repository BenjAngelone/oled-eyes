// ===== oled_eyes.h — runtime de referencia (espejo 1:1 del motor del design system) =====
// Requiere una clase con la API de Adafruit_GFX (fillRect, fillRoundRect, fillCircle, fillTriangle,
// drawLine, drawFastH/VLine, drawPixel, drawRect, drawRoundRect) y oled_eyes_data.h.
// En este proyecto la implementa OledBuf (oled_buf.h) sobre el buffer de U8g2.
// Parches locales respecto al original del Design System (marcados con "LOCAL"):
//   1. TICK_MS se puede fijar con -DEYES_TICK_MS=50 (20 fps).
//   2. El estilo ST_MATRIZ (GFXcanvas1 de Adafruit) queda fuera salvo -DEYES_WITH_MATRIZ.
//   3. La espiral precalcula cos/sin de sus angulos fijos (misma figura, mucho menos CPU en ESP8266).
#pragma once
#include <math.h>
#include "oled_eyes_data.h"

namespace eyes {

static const int16_t XL = 36, XR = 92, CY = 32;
#ifndef EYES_TICK_MS
#define EYES_TICK_MS 33                          // LOCAL: 30 fps por defecto
#endif
static const uint16_t TICK_MS = EYES_TICK_MS;
inline int imax(int a, int b) { return a > b ? a : b; }
inline int imin(int a, int b) { return a < b ? a : b; }
inline float clampf(float v, float a, float b) { return v < a ? a : v > b ? b : v; }
enum Style : uint8_t { ST_BLOQUE = 0, ST_VISOR = 1, ST_ESCANER = 2, ST_MATRIZ = 3, ST_LENTE = 4, ST_CASCO = 5, ST_MANCUERNA = 6 };
// ST_MANCUERNA se dibuja como siempre (encendido sobre negro); el panel se invierte por hardware:
//   oled.invertDisplay(style == ST_MANCUERNA);   // llamalo al cambiar de estilo, no en cada frame
struct EyeBox { float x, y, w; };

// ---------- easing ----------
inline float ease(uint8_t e, float t) {
  switch (e) {
    case E_INQUAD: return t * t;
    case E_OUTQUAD: return t * (2 - t);
    case E_INOUTQUAD: return t < .5f ? 2 * t * t : -1 + (4 - 2 * t) * t;
    case E_INCUBIC: return t * t * t;
    case E_OUTCUBIC: { float u = 1 - t; return 1 - u * u * u; }
    case E_INOUTCUBIC: return t < .5f ? 4 * t * t * t : 1 - powf(-2 * t + 2, 3) / 2;
    case E_INOUTSINE: return -(cosf(M_PI * t) - 1) / 2;
    case E_OUTBACK: { const float c1 = 1.70158f, c3 = c1 + 1; return 1 + c3 * powf(t - 1, 3) + c1 * powf(t - 1, 2); }
    case E_OUTELASTIC: return (t <= 0 || t >= 1) ? t : powf(2, -10 * t) * sinf((t * 10 - .75f) * (2 * M_PI) / 3) + 1;
    case E_OUTBOUNCE: {
      const float n1 = 7.5625f, d1 = 2.75f;
      if (t < 1 / d1) return n1 * t * t;
      if (t < 2 / d1) { t -= 1.5f / d1; return n1 * t * t + .75f; }
      if (t < 2.5f / d1) { t -= 2.25f / d1; return n1 * t * t + .9375f; }
      t -= 2.625f / d1; return n1 * t * t + .984375f;
    }
    case E_ANTICIPATEOVERSHOOT: {
      const float s = 2.25f;
      if (t < .5f) { float x = t * 2; return .5f * x * x * ((s + 1) * x - s); }
      float x = t * 2 - 2; return .5f * (x * x * ((s + 1) * x + s) + 2);
    }
    default: return t;
  }
}

inline void copyPose(float* dst, const float* src) { for (uint8_t i = 0; i < NP; i++) dst[i] = src[i]; }

// aplica un parche (SET / ADD / MUL / BASE) relativo a la base
inline void applyPatch(float* cur, const float* base, const Patch* p, uint8_t n) {
  for (uint8_t i = 0; i < n; i++) {
    const Patch& q = p[i];
    switch (q.mode) {
      case M_SET: cur[q.p] = q.v; break;
      case M_ADD: cur[q.p] = base[q.p] + q.v; break;
      case M_MUL: cur[q.p] = base[q.p] * q.v; break;
      case M_BASE: cur[q.p] = base[q.p]; break;
    }
  }
}

// pose de la animación en t (ms). Recalcula las claves desde el inicio: ≤40 claves × 47 floats por tick.
inline void evalAnim(const Anim& a, const float* base, uint16_t t, float* out) {
  static Pose prev, cur;
  copyPose(prev, base);
  for (uint8_t i = 0; i < a.n; i++) {
    const Key& k = a.keys[i];
    if (k.toBase) copyPose(cur, base); else copyPose(cur, prev);
    if (k.patch) applyPatch(cur, base, k.patch, k.n);
    if (i == 0 && t == 0) { copyPose(out, cur); return; }
    if (i > 0 && t < k.t) {
      const Key& k0 = a.keys[i - 1];
      float u = float(t - k0.t) / float(imax(1, k.t - k0.t));
      float e = ease(k.ease, u);
      for (uint8_t p = 0; p < NP; p++) out[p] = DISCRETE[p] ? prev[p] : prev[p] + (cur[p] - prev[p]) * e;
      return;
    }
    copyPose(prev, cur);
  }
  copyPose(out, prev);  // t ≥ dur
}

// transición entre poses base; si cambia la forma, se esconde detrás de un parpadeo
inline void evalTrans(const float* from, const float* to, float k, uint8_t easeId, float* out) {
  k = clampf(k, 0, 1);
  bool hide = from[L_SHAPE] != to[L_SHAPE] || from[R_SHAPE] != to[R_SHAPE];
  float e = hide ? ease(E_INOUTCUBIC, k) : ease(easeId, k);
  for (uint8_t p = 0; p < NP; p++) out[p] = DISCRETE[p] ? (k < .5f ? from[p] : to[p]) : from[p] + (to[p] - from[p]) * e;
  if (hide) { float m = k < .5f ? 1 - ease(E_INQUAD, k * 2) : ease(E_OUTBACK, k * 2 - 1); out[L_OPEN] *= m; out[R_OPEN] *= m; }
}

// ---------- dibujo ----------
template <class G> void thickLine(G& g, float x0, float y0, float x1, float y1, float th) {
  float dx = x1 - x0, dy = y1 - y0, L = sqrtf(dx * dx + dy * dy); if (L < 1) L = 1;
  float nx = -dy / L * th / 2, ny = dx / L * th / 2;
  g.fillTriangle(lroundf(x0 + nx), lroundf(y0 + ny), lroundf(x1 + nx), lroundf(y1 + ny), lroundf(x1 - nx), lroundf(y1 - ny), 1);
  g.fillTriangle(lroundf(x0 + nx), lroundf(y0 + ny), lroundf(x1 - nx), lroundf(y1 - ny), lroundf(x0 - nx), lroundf(y0 - ny), 1);
}

// e = puntero al primer parámetro del ojo (L_W o R_W); los offsets siguen el orden del enum
enum { W_, H_, R_, OPEN_, LIDT_, TILTT_, LIDB_, TILTB_, PX_, PY_, PR_, HL_, TH_, DX_, DY_, S_, ROT_, SHAPE_ };

template <class G> void drawShape(G& g, const float* e, int side, float cx, float cy, uint32_t clock) {
  float s = fmaxf(0, e[S_]), w = e[W_] * s, h = e[H_] * s, m = fminf(e[W_], e[H_]) * s;
  switch ((uint8_t)e[SHAPE_]) {
    case SH_CORAZON: {
      float rr = m * .26f, yc = cy - m * .12f, xo = rr * .95f;
      g.fillCircle(lroundf(cx - xo), lroundf(yc), lroundf(rr), 1);
      g.fillCircle(lroundf(cx + xo), lroundf(yc), lroundf(rr), 1);
      g.fillTriangle(lroundf(cx - xo - rr * .98f), lroundf(yc + rr * .3f), lroundf(cx + xo + rr * .98f), lroundf(yc + rr * .3f), lroundf(cx), lroundf(cy + m * .46f), 1);
    } break;
    case SH_ESTRELLA: {
      float ro = m * .56f, ri = ro * .47f, a0 = (e[ROT_] - 90) * M_PI / 180, px[10], py[10];
      for (int i = 0; i < 10; i++) { float r = (i & 1) ? ri : ro, a = a0 + i * M_PI / 5; px[i] = cx + cosf(a) * r; py[i] = cy + sinf(a) * r; }
      for (int i = 0; i < 10; i++) { int j = (i + 1) % 10; g.fillTriangle(lroundf(cx), lroundf(cy), lroundf(px[i]), lroundf(py[i]), lroundf(px[j]), lroundf(py[j]), 1); }
    } break;
    case SH_X: {
      float hx = m * .36f, t = fmaxf(4, e[TH_] + 1);
      thickLine(g, cx - hx, cy - hx, cx + hx, cy + hx, t);
      thickLine(g, cx + hx, cy - hx, cx - hx, cy + hx, t);
    } break;
    case SH_ESPIRAL: {
      float rmax = m * .46f, rot = (e[ROT_] + clock * 360.f / 700) * M_PI / 180 * (side < 0 ? 1 : -1), lx = cx, ly = cy;
      // LOCAL: cos/sin(th) no dependen de la rotacion: se calculan una vez y se combinan con cos/sin(rot)
      // (suma de angulos). Mismo resultado, 2 trig por frame en vez de 128 (en ESP8266 el trompo bajaba a 15 fps).
      static float cth[64], sth[64]; static bool spiralInit = false;
      if (!spiralInit) { for (int i = 0; i < 64; i++) { float t = (i + 1) / 64.f * 2.6f * 2 * M_PI; cth[i] = cosf(t); sth[i] = sinf(t); } spiralInit = true; }
      float cr = cosf(rot), sr = sinf(rot);
      for (int i = 1; i <= 64; i++) {
        float r = rmax * i / 64.f, x = cx + (cth[i - 1] * cr - sth[i - 1] * sr) * r, y = cy + (sth[i - 1] * cr + cth[i - 1] * sr) * r;
        g.drawLine(lroundf(lx), lroundf(ly), lroundf(x), lroundf(y), 1);
        g.drawLine(lroundf(lx) + 1, lroundf(ly), lroundf(x) + 1, lroundf(y), 1);
        lx = x; ly = y;
      }
    } break;
    case SH_CHEVRON: {
      float ax = w * .3f, by = h * .28f, d = side < 0 ? 1 : -1;
      thickLine(g, cx - ax * d, cy - by, cx + ax * d, cy, 5);
      thickLine(g, cx + ax * d, cy, cx - ax * d, cy + by, 5);
      g.fillCircle(lroundf(cx + ax * d), lroundf(cy), 2, 1);
    } break;
    case SH_ARCO: {
      float x0 = cx - w / 2, y0 = cy - h / 2;
      g.fillRoundRect(lroundf(x0), lroundf(y0), lroundf(w), lroundf(h), lroundf(h / 2), 1);
      g.fillRoundRect(lroundf(x0) - 1, lroundf(y0 - e[TH_]), lroundf(w) + 2, lroundf(h), lroundf(h / 2), 0);
    } break;
  }
}

template <class G> EyeBox drawEye(G& g, const float* P, const float* e, int side, uint8_t style, uint32_t clock, int sx, int sy) {
  int16_t bx = side < 0 ? XL : XR;
  float cx = bx + P[F_GX] + e[DX_] + (64 - bx) * P[F_MERGE] + sx;
  float cy = CY + P[F_GY] + e[DY_] + sy;
  if (e[SHAPE_] > .5f) { drawShape(g, e, side, cx, cy, clock); EyeBox b = {cx, cy, fminf(e[W_], e[H_]) * fmaxf(0, e[S_]) * .75f}; return b; }
  float op = fmaxf(0, e[OPEN_]), th = fmaxf(1, e[TH_]);
  float dk = style == ST_MANCUERNA ? .56f : 1;                            // Mancuerna: punto al 56% del bloque
  float w = e[W_] * dk * (1 + .18f * (1 - op)), h = fmaxf(th, e[H_] * dk * op);   // squash & stretch
  if (w < 1) { EyeBox b = {cx, cy, 0}; return b; }
  float px = clampf(e[PX_], -1, 1), py = clampf(e[PY_], -1, 1);
  bool pupil = style == ST_VISOR || style == ST_LENTE;
  cx += px * (pupil ? 2 : 4); cy += py * (pupil ? 1.5f : 3);
  int wi = lroundf(w), hi = lroundf(h), x0 = lroundf(cx - w / 2), y0 = lroundf(cy - h / 2);
  int r = imin(style == ST_LENTE || style == ST_MANCUERNA ? 99 : lroundf(e[R_]), imin(hi / 2, wi / 2));
  g.fillRoundRect(x0, y0, wi, hi, r, 1);
  if (style == ST_VISOR) {
    int t = lroundf(th);
    if (hi > 2 * t + 2) {
      g.fillRoundRect(x0 + t, y0 + t, wi - 2 * t, hi - 2 * t, imax(0, r - t), 0);
      int pad = t + 2, pr = (int)floorf(fminf(e[PR_], fminf(hi / 2.f - pad, wi / 2.f - pad)));
      if (pr >= 2) {
        float rx = fmaxf(0, wi / 2.f - pr - pad), ry = fmaxf(0, hi / 2.f - pr - pad);
        int pcx = lroundf(x0 + wi / 2.f + px * rx), pcy = lroundf(y0 + hi / 2.f + py * ry);
        g.fillRoundRect(pcx - pr, pcy - pr, 2 * pr, 2 * pr, 1, 1);
        int gl = imin(lroundf(e[HL_]), pr - 2);
        if (e[HL_] >= 1 && pr >= 4) g.fillRect(pcx - pr + 2, pcy - pr + 2, gl, gl, 0);
      }
    }
  } else if (style == ST_LENTE) {
    int t = lroundf(th);
    if (hi > 2 * t + 2) {
      g.fillRoundRect(x0 + t, y0 + t, wi - 2 * t, hi - 2 * t, imax(0, r - t), 0);
      int lp = t + 1, lr = (int)floorf(fminf(e[PR_] + 3, fminf(hi / 2.f - lp, wi / 2.f - lp)));
      if (lr >= 4) {
        float lx = fmaxf(0, wi / 2.f - lr - lp), ly = fmaxf(0, hi / 2.f - lr - lp);
        int pcx = lroundf(x0 + wi / 2.f + px * lx), pcy = lroundf(y0 + hi / 2.f + py * ly);
        g.fillCircle(pcx, pcy, lr, 1); g.fillCircle(pcx, pcy, lr - 2, 0); g.fillCircle(pcx, pcy, imax(1, lr - 6), 1);
        if (lr - 6 >= 3) g.fillCircle(pcx, pcy, 1, 0);
      }
    }
  } else {
    if (style == ST_ESCANER) for (int y = y0 + 1; y < y0 + hi - 1; y += 2) g.drawFastHLine(x0, y, wi, 0);
    if (e[HL_] >= 1 && hi > 10) {
      int gl = lroundf(e[HL_]);
      g.fillRect(x0 + 4 + lroundf((px + 1) * .5f * (wi - 8 - gl) * .5f), y0 + 4 + lroundf((py + 1) * .5f * (hi - 8 - gl) * .3f), gl, gl, 0);
    }
  }
  // párpados: negro, siempre al final; se desvanecen con el ojo casi cerrado
  float lk = clampf((op - .1f) / .3f, 0, 1);
  float lidT = clampf(e[LIDT_], 0, 1) * lk, lidB = clampf(e[LIDB_], 0, 1) * lk, tT = e[TILTT_] * lk, tB = e[TILTB_] * lk;
  float yl = y0 + lidT * hi;
  int xi = side < 0 ? x0 + wi : x0 - 1, xo = side < 0 ? x0 - 1 : x0 + wi;
  if (lidT > .001f) g.fillRect(x0 - 1, y0 - 2, wi + 2, lroundf(lidT * hi) + 2, 0);
  if (tT > .5f) g.fillTriangle(xi, lroundf(yl - 1), xo, lroundf(yl - 1), xi, lroundf(yl + tT), 0);
  else if (tT < -.5f) g.fillTriangle(xo, lroundf(yl - 1), xi, lroundf(yl - 1), xo, lroundf(yl - tT), 0);
  float yb = y0 + hi - lidB * hi;
  if (lidB > .001f) { float rb = wi * fmaxf(.62f, 1.25f - lidB * 1.15f); g.fillCircle(lroundf(x0 + wi / 2.f), lroundf(yb + rb), lroundf(rb), 0); }
  if (tB > .5f) g.fillTriangle(xi, lroundf(yb + 1), xo, lroundf(yb + 1), xi, lroundf(yb - tB), 0);
  else if (tB < -.5f) g.fillTriangle(xo, lroundf(yb + 1), xi, lroundf(yb + 1), xo, lroundf(yb + tB), 0);
  EyeBox b = {x0 + wi / 2.f, (yl + yb) / 2, (float)wi}; return b;   // y = centro de la parte visible
}

inline float noise(uint32_t n) { float x = sinf(n * 127.1f + 311.7f) * 43758.5453f; return x - floorf(x); }

template <class G> void drawZ(G& g, int x, int y, int s) { g.drawFastHLine(x, y, s, 1); g.drawLine(x + s - 1, y, x, y + s - 1, 1); g.drawFastHLine(x, y + s - 1, s, 1); }

#ifdef EYES_WITH_MATRIZ   // LOCAL: requiere Adafruit_GFX
// Matriz: se dibuja como Bloque en un lienzo auxiliar y se cuantiza (GFXcanvas1 de Adafruit_GFX, 1 KB)
static GFXcanvas1 matrixCanvas(128, 64);
#endif

template <class G> void render(G& g, const float* P, uint8_t style, uint32_t clock) {
  g.fillRect(0, 0, 128, 64, 0);
  uint32_t fr = clock / TICK_MS;
  int sx = 0, sy = 0;
  if (P[F_SHAKE] > .05f) { sx = lroundf((noise(fr) * 2 - 1) * P[F_SHAKE]); sy = lroundf((noise(fr + 99) * 2 - 1) * P[F_SHAKE] * .6f); }
#ifdef EYES_WITH_MATRIZ
  if (style == ST_MATRIZ) {
    matrixCanvas.fillScreen(0);
    drawEye(matrixCanvas, P, &P[L_W], -1, ST_BLOQUE, clock, sx, sy);
    drawEye(matrixCanvas, P, &P[R_W], 1, ST_BLOQUE, clock, sx, sy);
    for (int y = 0; y < 64; y += 4) for (int x = 0; x < 128; x += 4) if (matrixCanvas.getPixel(x + 1, y + 1)) g.fillRect(x, y, 3, 3, 1);
  } else
#endif
  {
    EyeBox eL = drawEye(g, P, &P[L_W], -1, style, clock, sx, sy);
    EyeBox eR = drawEye(g, P, &P[R_W], 1, style, clock, sx, sy);
    if (style == ST_MANCUERNA && eR.x - eL.x > 4) thickLine(g, eL.x + eL.w / 2 - 1, eL.y, eR.x - eR.w / 2 + 1, eR.y, 3);
  }
  if (style == ST_CASCO) { g.drawRoundRect(4, 5, 120, 54, 10, 1); g.drawRoundRect(5, 6, 118, 52, 9, 1); g.fillRect(60, 3, 8, 2, 1); }
  if (P[F_SWEAT] > .5f) {               // gota de refrigerante, ciclo 1400 ms
    float ph = (clock % 1400) / 1400.f;
    if (ph < .85f) { float u = ph / .85f; int y = lroundf(5 + u * u * 12);
      g.fillCircle(119, y, 3, 1); g.fillTriangle(116, y - 1, 122, y - 1, 119, y - 7, 1); g.drawPixel(118, y - 1, 0); }
  }
  if (P[F_ZZZ] > .5f) for (int i = 0; i < 2; i++) {
    float pz = fmodf(clock / 2600.f + i / 2.f, 1);
    if (pz < .9f) drawZ(g, lroundf(100 + pz * 18), lroundf(24 - pz * 23), 4 + lroundf(pz * 5));
  }
  if (P[F_Q] > .5f) { int x = 113, y = 2 + lroundf(sinf(clock / 900.f * 2 * M_PI));
    g.fillRect(x + 2, y, 5, 2, 1); g.fillRect(x, y + 2, 2, 2, 1); g.fillRect(x + 7, y + 2, 2, 3, 1);
    g.fillRect(x + 5, y + 5, 3, 2, 1); g.fillRect(x + 4, y + 7, 2, 2, 1); g.fillRect(x + 4, y + 11, 2, 2, 1); }
  if (P[F_BAT] > .5f && (clock % 1000) < 620) { g.drawRect(55, 1, 17, 9, 1); g.fillRect(72, 3, 2, 5, 1); g.fillRect(57, 3, 2, 5, 1); }
  if (P[F_SPIN] > .5f) {
    int head = (clock / 90) % 8; float scy = CY + P[F_GY] + (style == ST_MANCUERNA ? 15 : 0);
    for (int j = 0; j < 8; j++) {
      float a = j / 8.f * 2 * M_PI - M_PI / 2; int x = lroundf(64 + cosf(a) * 6), y = lroundf(scy + sinf(a) * 6), age = (head - j + 8) % 8;
      if (age == 0) g.fillCircle(x, y, 2, 1); else if (age == 1) g.fillCircle(x, y, 1, 1); else g.drawPixel(x, y, 1);
    }
  }
  if (P[F_SPARK] > .5f) {
    static const uint8_t sp[6][2] = {{10, 8}, {118, 10}, {64, 6}, {8, 54}, {120, 56}, {64, 58}};
    for (int k = 0; k < 6; k++) { float tw = fmodf(clock / 600.f + k * .37f, 1);
      if (tw < .5f) { int s = tw < .25f ? 2 : 1; g.drawFastHLine(sp[k][0] - s, sp[k][1], 2 * s + 1, 1); g.drawFastVLine(sp[k][0], sp[k][1] - s, 2 * s + 1, 1); } }
  }
}

}  // namespace eyes
