// ===== eyes_machine.h — máquina de estados de los ojos (port del Player de bundle.js) =====
// Sin hardware ni Arduino: recibe porcentaje, estado de sistema y taps; cada tick() deja en pose()
// los 47 parámetros listos para eyes::render(). Se puede compilar en el PC para probarla.
//
// Capas por tick, en este orden (igual que el motor del Design System):
//   1 base (humor o sistema, con transición)  2 animación activa  3 parpadeo  4 sacadas
//   5 (seguimiento: no implementado)  6 idles aleatorios  7 comportamiento de sistema
#pragma once
#include <stdint.h>
#include <math.h>
#include "oled_eyes.h"

namespace eyes {

enum Sys : uint8_t { SYS_ARRANCANDO, SYS_CONECTANDO, SYS_SIN_WIFI, SYS_SIN_DATOS, SYS_NORMAL };
enum Mood : uint8_t { MOOD_TRANQUILO, MOOD_ATENTO, MOOD_PREOCUPADO, MOOD_ESTRESADO, MOOD_AGOTADO, MOOD_COUNT };
enum SacMode : uint8_t { SAC_NONE, SAC_SOFT, SAC_MICRO, SAC_RESTLESS, SAC_PINGPONG };

// ---------- tablas por humor (estados.md · Humores por consumo de cuota) ----------
struct MoodSpec {
  const float* pose;
  uint8_t loPct;                                              // umbral inferior del humor
  uint16_t blinkMin, blinkMax; float blinkScale, blinkDbl;    // scale: >1 = parpadeo más lento
  uint16_t sacMin, sacMax; SacMode sacMode; float sacAmp, sacBig; uint16_t sacDur;
  uint16_t idleMin, idleMax; uint8_t nIdle; const Anim* idle[3];
};

static const MoodSpec MOODS[MOOD_COUNT] = {
  { POSE_TRANQUILO,  0,  3500, 6500, 1.35f, .10f,  2500, 5000, SAC_SOFT,     .35f, 0,    170,  16000, 28000, 3, { &ANIM_BOSTEZO, &ANIM_ABURRIDO, &ANIM_MIRAR_ALREDEDOR } },
  { POSE_ATENTO,     50, 2500, 5000, 1.00f, .15f,  700,  1800, SAC_MICRO,    .20f, .25f, 90,   14000, 24000, 3, { &ANIM_MIRAR_ALREDEDOR, &ANIM_SOSPECHOSA, &ANIM_ESTORNUDO } },
  { POSE_PREOCUPADO, 75, 1800, 3500, 0.90f, .30f,  600,  1200, SAC_RESTLESS, .55f, 0,    80,   12000, 20000, 2, { &ANIM_SOSPECHOSA, &ANIM_MIRAR_ALREDEDOR, nullptr } },
  { POSE_ESTRESADO,  90, 900,  2000, 0.70f, .40f,  250,  450,  SAC_PINGPONG, .85f, 0,    60,   0,     0,     0, { nullptr, nullptr, nullptr } },
  { POSE_AGOTADO,    100, 6000, 9000, 2.20f, 0,    0,    0,    SAC_NONE,     0,    0,    0,    9000,  15000, 2, { &ANIM_MAREO_IDLE, &ANIM_BOSTEZO, nullptr } },
};

// ---------- transiciones entre humores (estados.md · Transiciones) ----------
struct TransSpec { uint16_t dur; uint8_t ease; bool blinkAfter; };
static const TransSpec TRANS_UP[MOOD_COUNT] = {
  { 0, E_LINEAR, false },                       // (TRANQUILO no se alcanza subiendo)
  { 380, E_ANTICIPATEOVERSHOOT, false },
  { 420, E_ANTICIPATEOVERSHOOT, false },
  { 300, E_OUTBACK, false },
  { 900, E_OUTBOUNCE, false },
};
static const TransSpec TRANS_DOWN   = { 650, E_INOUTSINE, true };    // + parpadeo de alivio al final
static const TransSpec TRANS_SYSTEM = { 450, E_INOUTCUBIC, false };

class Machine {
 public:
  // Arranca con la animación ARRANQUE. El humor inicial sale de pct (0..100).
  void begin(uint32_t seed, int pct) {
    rng_ = seed ? seed : 1;
    clock_ = 0;
    pct_ = clampPct(pct);
    mood_ = moodFor(pct_, -1);
    sys_ = SYS_ARRANCANDO; wanted_ = SYS_NORMAL;
    trans_.on = false; anim_ = nullptr; blink_ = false; nTaps_ = 0;
    sac_ = Sac();
    copyPose(lastBase_, target());
    copyPose(out_, lastBase_);
    schedule();
    play(&ANIM_ARRANQUE, true);
  }

  // % de cuota consumido. Humor nuevo → transición animada (sólo si el sistema está en NORMAL).
  void setPercent(int p) {
    pct_ = clampPct(p);
    Mood nm = moodFor(pct_, mood_);
    if (nm == mood_) return;
    bool up = nm > mood_;
    mood_ = nm;
    schedule();
    if (sys_ == SYS_NORMAL) goTo(up ? TRANS_UP[nm] : TRANS_DOWN);
  }

  // Estado de sistema que pide el firmware. Durante ARRANCANDO se guarda y se aplica al terminar.
  void setSystem(Sys s) {
    if (s == SYS_ARRANCANDO) return;
    wanted_ = s;
    if (sys_ == SYS_ARRANCANDO || s == sys_) return;
    sys_ = s; anim_ = nullptr; animBoot_ = false;
    goTo(TRANS_SYSTEM);
    schedule();
  }

  // 1 tap = COSQUILLAS; 3 taps en menos de 1,2 s = TROMPO. Ignorado durante ARRANCANDO.
  void tap() {
    if (sys_ == SYS_ARRANCANDO) return;
    uint8_t n = 0;
    for (uint8_t i = 0; i < nTaps_; i++) if (clock_ - taps_[i] < 1200) taps_[n++] = taps_[i];
    nTaps_ = n;
    if (nTaps_ < 3) taps_[nTaps_++] = clock_;
    if (nTaps_ >= 3) { nTaps_ = 0; play(&ANIM_TROMPO); }
    else if (!anim_ || anim_ == &ANIM_COSQUILLAS) play(&ANIM_COSQUILLAS);
  }

  // POST /egg
  void egg() { if (sys_ != SYS_ARRANCANDO) play(&ANIM_TROMPO); }

  void tick() {
    clock_ += TICK_MS;
    const uint32_t c = clock_;
    const MoodSpec& m = MOODS[mood_];
    Pose base, pose;

    // 1) base: humor/sistema, o transición en curso
    if (trans_.on) {
      float k = float(c - trans_.t0) / float(trans_.spec.dur);
      if (k >= 1) {
        copyPose(base, trans_.to);
        if (trans_.spec.blinkAfter) startBlink(&ANIM_PARPADEO, 1.2f);
        trans_.on = false;
      } else evalTrans(trans_.from, trans_.to, k, trans_.spec.ease, base);
    } else copyPose(base, target());
    copyPose(lastBase_, base);
    copyPose(pose, base);

    // 2) animación activa
    if (anim_) {
      uint32_t t = c - animT0_;
      if (t >= anim_->dur) {
        bool boot = animBoot_;
        anim_ = nullptr; animBoot_ = false;
        if (boot) {                                   // ARRANCANDO terminó: aplicar lo que pidió el firmware
          sys_ = SYS_NORMAL;
          if (wanted_ != SYS_NORMAL) { sys_ = wanted_; goTo(TRANS_SYSTEM); }
          schedule();
        }
        nextIdle_ = c + between(m.idleMin, m.idleMax);
      } else evalAnim(*anim_, base, (uint16_t)t, pose);
    }
    const bool free = !anim_ && sys_ == SYS_NORMAL;

    // 3) parpadeo programado (multiplica open: funciona sobre cualquier pose)
    if (c >= nextBlink_ && sys_ != SYS_SIN_DATOS) {
      if (!anim_) startBlink(rand01() < m.blinkDbl ? &ANIM_DOBLE_PARPADEO : &ANIM_PARPADEO, sys_ == SYS_NORMAL ? m.blinkScale : 1.f);
      nextBlink_ = c + (sys_ == SYS_NORMAL ? between(m.blinkMin, m.blinkMax) : between(2500, 5000));
    }
    if (blink_) {
      float bt = float(c - blinkT0_) / blinkScale_;
      if (bt >= blinkDef_->dur || anim_) blink_ = false;
      else {
        Pose unit, b;
        copyPose(unit, POSE_ATENTO); unit[L_OPEN] = unit[R_OPEN] = 1;
        evalAnim(*blinkDef_, unit, (uint16_t)bt, b);
        pose[L_OPEN] *= b[L_OPEN]; pose[R_OPEN] *= b[L_OPEN];
      }
    }

    // 4) sacadas por humor: aditivas sobre gx, gy, px, py (y lidT al mirar abajo), con outBack
    if (free && m.sacMode != SAC_NONE && c >= nextSac_) {
      sac_.fx = sac_.x; sac_.fy = sac_.y; sac_.t0 = c; sac_.dur = m.sacDur;
      const float a = m.sacAmp;
      switch (m.sacMode) {
        case SAC_PINGPONG: sac_.side = -sac_.side; sac_.tx = sac_.side * a; sac_.ty = (rand01() - .5f) * .3f; break;
        case SAC_MICRO: {
          bool big = rand01() < m.sacBig;
          float amp = big ? .65f : a;
          sac_.tx = (rand01() * 2 - 1) * amp; sac_.ty = (rand01() * 2 - 1) * amp * .6f;
          if (!big && rand01() < .4f) { sac_.tx *= .3f; sac_.ty *= .3f; }
        } break;
        case SAC_SOFT: sac_.tx = rand01() < .4f ? 0 : (rand01() * 2 - 1) * a; sac_.ty = (rand01() * 2 - 1) * a * .5f; break;
        default: sac_.tx = (rand01() * 2 - 1) * a; sac_.ty = (rand01() * 2 - 1) * a * .8f; break;   // restless
      }
      nextSac_ = c + between(m.sacMin, m.sacMax);
    }
    if (free) {
      float u = float(c - sac_.t0) / float(imax(1, sac_.dur));
      float ee = ease(E_OUTBACK, u > 1 ? 1 : u);
      sac_.x = sac_.fx + (sac_.tx - sac_.fx) * ee;
      sac_.y = sac_.fy + (sac_.ty - sac_.fy) * ee;
      pose[F_GX] += sac_.x * 8; pose[F_GY] += sac_.y * 4;
      pose[L_PX] = clampf(pose[L_PX] + sac_.x * .7f, -1, 1); pose[R_PX] = clampf(pose[R_PX] + sac_.x * .7f, -1, 1);
      pose[L_PY] = clampf(pose[L_PY] + sac_.y * .7f, -1, 1); pose[R_PY] = clampf(pose[R_PY] + sac_.y * .7f, -1, 1);
      if (sac_.y > 0) { pose[L_LIDT] += sac_.y * .15f; pose[R_LIDT] += sac_.y * .15f; }   // al mirar abajo baja el párpado
    } else {
      // LOCAL (difiere de bundle.js): el motor JS sólo decae sac.x/y y al volver a "free" reaparece
      // el último destino de golpe (hasta 4 px). Acá se sincroniza origen y destino para que no haya corte.
      sac_.x *= .8f; sac_.y *= .8f;
      sac_.fx = sac_.tx = sac_.x; sac_.fy = sac_.ty = sac_.y;
    }

    // 6) idle aleatorio: sólo si no pasa nada más
    if (free && m.nIdle && c >= nextIdle_ && !trans_.on) {
      play(m.idle[(uint32_t)(rand01() * m.nIdle) % m.nIdle]);
      nextIdle_ = c + between(m.idleMin, m.idleMax);
    }

    // 7) comportamiento procedural de sistema (periodos exactos con %: sin deriva de float)
    if (!trans_.on) {
      const float TWO_PI_F = 6.2831853f;
      if (sys_ == SYS_CONECTANDO) {            // orbit: 1200 ms/vuelta, el derecho con 80 ms de retraso
        float th = (c % 1200) / 1200.f * TWO_PI_F, th2 = ((c + 1200 - 80) % 1200) / 1200.f * TWO_PI_F;
        pose[L_PX] = cosf(th) * .8f;  pose[L_PY] = sinf(th) * .8f;
        pose[R_PX] = cosf(th2) * .8f; pose[R_PY] = sinf(th2) * .8f;
      } else if (sys_ == SYS_SIN_DATOS) {      // breathe: ±1 px cada 3 s
        pose[F_GY] += sinf((c % 3000) / 3000.f * TWO_PI_F);
      } else if (sys_ == SYS_SIN_WIFI) {       // bob: deriva de mirada ±0,1 cada 1800 ms
        float b = sinf((c % 1800) / 1800.f * TWO_PI_F) * .1f;
        pose[L_PX] += b; pose[R_PX] += b;
      }
    }
    copyPose(out_, pose);
  }

  const float* pose() const { return out_; }
  uint32_t clock() const { return clock_; }
  Sys sys() const { return sys_; }
  Mood mood() const { return mood_; }
  int percent() const { return pct_; }
  const Anim* anim() const { return anim_; }

 private:
  struct Sac { float x = 0, y = 0, fx = 0, fy = 0, tx = 0, ty = 0; uint32_t t0 = 0; uint16_t dur = 1; int8_t side = 1; };
  struct Trans { bool on = false; Pose from; const float* to = nullptr; uint32_t t0 = 0; TransSpec spec = {0, 0, false}; };

  uint32_t rng_ = 1, clock_ = 0;
  int pct_ = 0;
  Mood mood_ = MOOD_TRANQUILO;
  Sys sys_ = SYS_ARRANCANDO, wanted_ = SYS_NORMAL;
  Pose lastBase_, out_;
  Trans trans_;
  const Anim* anim_ = nullptr; uint32_t animT0_ = 0; bool animBoot_ = false;
  bool blink_ = false; uint32_t blinkT0_ = 0; const Anim* blinkDef_ = nullptr; float blinkScale_ = 1;
  uint32_t nextBlink_ = 0, nextSac_ = 0, nextIdle_ = 0;
  Sac sac_;
  uint32_t taps_[3]; uint8_t nTaps_ = 0;

  static int clampPct(int p) { return p < 0 ? 0 : p > 100 ? 100 : p; }

  // humor por %, con histéresis de 2 puntos al bajar (prev < 0 = sin humor previo)
  static Mood moodFor(int pct, int prev) {
    Mood raw = pct >= 100 ? MOOD_AGOTADO : pct >= 90 ? MOOD_ESTRESADO : pct >= 75 ? MOOD_PREOCUPADO : pct >= 50 ? MOOD_ATENTO : MOOD_TRANQUILO;
    if (prev >= 0 && raw < prev && pct > MOODS[prev].loPct - 3) return (Mood)prev;
    return raw;
  }

  const float* target() const {
    switch (sys_) {
      case SYS_CONECTANDO: return POSE_CONECTANDO;
      case SYS_SIN_WIFI: return POSE_SIN_WIFI;
      case SYS_SIN_DATOS: return POSE_SIN_DATOS;
      default: return MOODS[mood_].pose;     // NORMAL y ARRANCANDO
    }
  }

  void goTo(const TransSpec& spec) {
    copyPose(trans_.from, lastBase_);
    trans_.to = target(); trans_.t0 = clock_; trans_.spec = spec; trans_.on = true;
  }

  void play(const Anim* a, bool boot = false) { anim_ = a; animT0_ = clock_; animBoot_ = boot; }

  void startBlink(const Anim* def, float scale) { blink_ = true; blinkT0_ = clock_; blinkDef_ = def; blinkScale_ = scale; }

  void schedule() {
    const MoodSpec& m = MOODS[mood_];
    nextBlink_ = clock_ + between(m.blinkMin, m.blinkMax);
    nextSac_ = clock_ + between(m.sacMin, m.sacMax);
    nextIdle_ = clock_ + between(m.idleMin, m.idleMax);
  }

  float rand01() {                       // xorshift32, mismo reparto que el motor JS
    rng_ ^= rng_ << 13; rng_ ^= rng_ >> 17; rng_ ^= rng_ << 5;
    return (rng_ % 100000) / 100000.f;
  }
  uint32_t between(uint32_t a, uint32_t b) { return a + (uint32_t)((b - a) * rand01()); }
};

}  // namespace eyes
