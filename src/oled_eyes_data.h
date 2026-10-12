// ===== oled_eyes_data.h — generado desde el design system OledEyes. No editar a mano. =====
#pragma once
#include <stdint.h>

enum Param : uint8_t {
  L_W, L_H, L_R, L_OPEN, L_LIDT, L_TILTT, L_LIDB, L_TILTB, L_PX, L_PY, L_PR, L_HL, L_TH, L_DX, L_DY, L_S, L_ROT, L_SHAPE, R_W, R_H, R_R, R_OPEN, R_LIDT, R_TILTT, R_LIDB, R_TILTB, R_PX, R_PY, R_PR, R_HL, R_TH, R_DX, R_DY, R_S, R_ROT, R_SHAPE, F_GX, F_GY, F_SHAKE, F_MERGE, F_SWEAT, F_ZZZ, F_Q, F_BAT, F_SPIN, F_SPARK, F_FLY,
  NP  // 47 parámetros
};
enum Shape : uint8_t { SH_OJO = 0, SH_CORAZON = 1, SH_ESTRELLA = 2, SH_X = 3, SH_ESPIRAL = 4, SH_CHEVRON = 5, SH_ARCO = 6 };
enum Ease : uint8_t { E_LINEAR, E_INQUAD, E_OUTQUAD, E_INOUTQUAD, E_INCUBIC, E_OUTCUBIC, E_INOUTCUBIC, E_INOUTSINE, E_OUTBACK, E_OUTELASTIC, E_OUTBOUNCE, E_ANTICIPATEOVERSHOOT };
enum Mode : uint8_t { M_SET, M_ADD, M_MUL, M_BASE };

// true = discreto (cambia de golpe al llegar a la clave; no se interpola)
const bool DISCRETE[NP] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,1,1,1,1,1,1 };

typedef float Pose[NP];
struct Patch { uint8_t p; uint8_t mode; float v; };
struct Key { uint16_t t; uint8_t ease; bool toBase; const Patch* patch; uint8_t n; };
struct Anim { const Key* keys; uint8_t n; uint16_t dur; };

// ---- Poses base: humores y estados de sistema (orden = enum Param) ----
const Pose POSE_TRANQUILO = { 34.0f, 34.0f, 4.0f, 1.0f, 0.2f, -2.0f, 0.0f, 0.0f, 0.0f, 0.15f, 7.0f, 0.0f, 3.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 34.0f, 34.0f, 4.0f, 1.0f, 0.2f, -2.0f, 0.0f, 0.0f, 0.0f, 0.15f, 7.0f, 0.0f, 3.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };  // 0-49%
const Pose POSE_ATENTO = { 34.0f, 42.0f, 4.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -0.05f, 8.0f, 0.0f, 3.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 34.0f, 42.0f, 4.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -0.05f, 8.0f, 0.0f, 3.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };  // 50-74%
const Pose POSE_PREOCUPADO = { 32.0f, 38.0f, 4.0f, 1.0f, 0.08f, -9.0f, 0.0f, 0.0f, 0.0f, -0.25f, 6.0f, 0.0f, 3.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 32.0f, 38.0f, 4.0f, 1.0f, 0.08f, -9.0f, 0.0f, 0.0f, 0.0f, -0.25f, 6.0f, 0.0f, 3.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };  // 75-89%
const Pose POSE_ESTRESADO = { 38.0f, 46.0f, 5.0f, 1.0f, 0.0f, -5.0f, 0.0f, 0.0f, 0.0f, 0.0f, 4.0f, 0.0f, 3.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 38.0f, 46.0f, 5.0f, 1.0f, 0.0f, -5.0f, 0.0f, 0.0f, 0.0f, 0.0f, 4.0f, 0.0f, 3.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, -2.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };  // 90-99%
const Pose POSE_AGOTADO = { 36.0f, 38.0f, 4.0f, 1.0f, 0.6f, -6.0f, 0.0f, 0.0f, 0.0f, 0.75f, 6.0f, 0.0f, 3.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 36.0f, 38.0f, 4.0f, 1.0f, 0.6f, -6.0f, 0.0f, 0.0f, 0.0f, 0.75f, 6.0f, 0.0f, 3.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 5.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f };  // 100-100%
const Pose POSE_CONECTANDO = { 24.0f, 24.0f, 3.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 5.0f, 0.0f, 3.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 24.0f, 24.0f, 3.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 5.0f, 0.0f, 3.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f };
const Pose POSE_SIN_WIFI = { 34.0f, 40.0f, 4.0f, 1.0f, 0.0f, -5.0f, 0.0f, 0.0f, 0.45f, -0.5f, 7.0f, 0.0f, 3.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 30.0f, 30.0f, 4.0f, 1.0f, 0.3f, 4.0f, 0.0f, 0.0f, 0.45f, -0.2f, 7.0f, 0.0f, 3.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, -3.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f };
const Pose POSE_SIN_DATOS = { 30.0f, 40.0f, 4.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 7.0f, 0.0f, 3.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 30.0f, 40.0f, 4.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 7.0f, 0.0f, 3.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 4.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

// ---- Parches de expresión (sobre la base del humor actual) ----
const Patch P_CERRADO[] = { {L_OPEN,M_SET,0.0f}, {R_OPEN,M_SET,0.0f} };
const Patch P_MIRA_IZQ[] = { {L_PX,M_SET,-0.65f}, {R_PX,M_SET,-0.65f}, {F_GX,M_SET,-9.0f} };
const Patch P_MIRA_DER[] = { {L_PX,M_SET,0.65f}, {R_PX,M_SET,0.65f}, {F_GX,M_SET,9.0f} };
const Patch P_MIRA_ARR[] = { {L_PY,M_SET,-0.7f}, {R_PY,M_SET,-0.7f}, {L_LIDT,M_SET,0.0f}, {R_LIDT,M_SET,0.0f}, {L_TILTT,M_SET,0.0f}, {R_TILTT,M_SET,0.0f}, {F_GY,M_ADD,-6.0f} };
const Patch P_MIRA_ABA[] = { {L_PY,M_SET,0.7f}, {R_PY,M_SET,0.7f}, {L_LIDT,M_SET,0.28f}, {R_LIDT,M_SET,0.28f}, {F_GY,M_ADD,5.0f} };
const Patch P_GUINO[] = { {L_LIDB,M_SET,0.32f}, {R_OPEN,M_SET,0.0f}, {F_GX,M_SET,1.0f} };
const Patch P_FELIZ[] = { {L_LIDB,M_SET,0.58f}, {R_LIDB,M_SET,0.58f}, {L_LIDT,M_SET,0.0f}, {R_LIDT,M_SET,0.0f}, {L_TILTT,M_SET,0.0f}, {R_TILTT,M_SET,0.0f}, {L_PR,M_SET,0.0f}, {R_PR,M_SET,0.0f}, {L_HL,M_SET,0.0f}, {R_HL,M_SET,0.0f}, {F_GY,M_ADD,-2.0f} };
const Patch P_CORAZON[] = { {L_SHAPE,M_SET,1.0f}, {R_SHAPE,M_SET,1.0f}, {L_S,M_SET,1.0f}, {R_S,M_SET,1.0f} };
const Patch P_ESTRELLA[] = { {L_SHAPE,M_SET,2.0f}, {R_SHAPE,M_SET,2.0f}, {L_S,M_SET,1.0f}, {R_S,M_SET,1.0f}, {L_ROT,M_SET,0.0f}, {R_ROT,M_SET,0.0f}, {F_SPARK,M_SET,1.0f} };
const Patch P_SOSPECHOSA[] = { {L_LIDT,M_SET,0.52f}, {R_LIDT,M_SET,0.52f}, {L_TILTT,M_SET,2.0f}, {R_TILTT,M_SET,2.0f}, {L_LIDB,M_SET,0.0f}, {R_LIDB,M_SET,0.0f}, {L_PR,M_SET,6.0f}, {R_PR,M_SET,6.0f}, {L_PX,M_SET,-0.7f}, {R_PX,M_SET,-0.7f} };
const Patch P_ANTICIPA[] = { {L_H,M_MUL,0.82f}, {R_H,M_MUL,0.82f}, {L_W,M_MUL,1.06f}, {R_W,M_MUL,1.06f}, {F_GY,M_ADD,2.0f} };
const Patch P_SORPRESA[] = { {L_W,M_SET,38.0f}, {R_W,M_SET,38.0f}, {L_H,M_SET,52.0f}, {R_H,M_SET,52.0f}, {L_R,M_SET,5.0f}, {R_R,M_SET,5.0f}, {L_LIDT,M_SET,0.0f}, {R_LIDT,M_SET,0.0f}, {L_TILTT,M_SET,0.0f}, {R_TILTT,M_SET,0.0f}, {L_LIDB,M_SET,0.0f}, {R_LIDB,M_SET,0.0f}, {L_PR,M_SET,4.0f}, {R_PR,M_SET,4.0f}, {L_HL,M_SET,0.0f}, {R_HL,M_SET,0.0f}, {F_GY,M_ADD,-3.0f} };
const Patch P_BOSTEZO[] = { {L_OPEN,M_SET,0.14f}, {R_OPEN,M_SET,0.14f}, {L_LIDB,M_SET,0.3f}, {R_LIDB,M_SET,0.3f}, {L_TILTT,M_SET,-6.0f}, {R_TILTT,M_SET,-6.0f}, {F_GY,M_ADD,-3.0f} };
const Patch P_ABURRIDO[] = { {L_LIDT,M_SET,0.45f}, {R_LIDT,M_SET,0.45f}, {L_TILTT,M_SET,0.0f}, {R_TILTT,M_SET,0.0f}, {L_LIDB,M_SET,0.0f}, {R_LIDB,M_SET,0.0f}, {L_PY,M_SET,0.45f}, {R_PY,M_SET,0.45f}, {L_PX,M_SET,-0.6f}, {R_PX,M_SET,-0.6f} };
const Patch P_ESTORNUDO_A[] = { {L_LIDT,M_SET,0.45f}, {R_LIDT,M_SET,0.45f}, {L_TILTT,M_SET,-8.0f}, {R_TILTT,M_SET,-8.0f}, {L_PY,M_SET,-0.5f}, {R_PY,M_SET,-0.5f}, {F_GY,M_ADD,-4.0f} };
const Patch P_ESTORNUDO[] = { {L_SHAPE,M_SET,5.0f}, {R_SHAPE,M_SET,5.0f}, {L_S,M_SET,1.0f}, {R_S,M_SET,1.0f}, {F_GY,M_ADD,4.0f}, {F_SHAKE,M_SET,3.0f} };
const Patch P_MAREO[] = { {L_SHAPE,M_SET,4.0f}, {R_SHAPE,M_SET,4.0f}, {L_S,M_SET,1.0f}, {R_S,M_SET,1.0f}, {F_SHAKE,M_SET,1.0f} };
const Patch P_KO[] = { {L_SHAPE,M_SET,3.0f}, {R_SHAPE,M_SET,3.0f}, {L_S,M_SET,1.0f}, {R_S,M_SET,1.0f} };
const Patch P_LINEA[] = { {L_OPEN,M_SET,0.0f}, {R_OPEN,M_SET,0.0f}, {L_W,M_SET,80.0f}, {R_W,M_SET,80.0f}, {L_PR,M_SET,0.0f}, {R_PR,M_SET,0.0f}, {L_HL,M_SET,0.0f}, {R_HL,M_SET,0.0f}, {F_MERGE,M_SET,1.0f} };

// ---- Animaciones: claves con parche ya fusionado (pose + set) ----
const Patch A_PARPADEO_1[] = { {L_OPEN,M_SET,0.0f}, {R_OPEN,M_SET,0.0f} };
const Key K_PARPADEO[] = {
  { 0, E_LINEAR, false, nullptr, 0 },
  { 60, E_INQUAD, false, A_PARPADEO_1, 2 },  // cierre rápido
  { 90, E_LINEAR, false, nullptr, 0 },  // hold
  { 190, E_OUTBACK, true, nullptr, 0 },  // abre con estirón
};
const Anim ANIM_PARPADEO = { K_PARPADEO, 4, 190 };
const Patch A_DOBLE_PARPADEO_1[] = { {L_OPEN,M_SET,0.0f}, {R_OPEN,M_SET,0.0f} };
const Patch A_DOBLE_PARPADEO_5[] = { {L_OPEN,M_SET,0.0f}, {R_OPEN,M_SET,0.0f} };
const Key K_DOBLE_PARPADEO[] = {
  { 0, E_LINEAR, false, nullptr, 0 },
  { 55, E_INQUAD, false, A_DOBLE_PARPADEO_1, 2 },
  { 75, E_LINEAR, false, nullptr, 0 },
  { 150, E_OUTQUAD, true, nullptr, 0 },  // sin rebote
  { 230, E_LINEAR, false, nullptr, 0 },
  { 285, E_INQUAD, false, A_DOBLE_PARPADEO_5, 2 },
  { 310, E_LINEAR, false, nullptr, 0 },
  { 420, E_OUTBACK, true, nullptr, 0 },  // rebote final
};
const Anim ANIM_DOBLE_PARPADEO = { K_DOBLE_PARPADEO, 8, 420 };
const Patch A_MIRAR_IZQ_1[] = { {F_GX,M_SET,1.0f} };
const Patch A_MIRAR_IZQ_2[] = { {L_PX,M_SET,-0.65f}, {R_PX,M_SET,-0.65f}, {F_GX,M_SET,-9.0f} };
const Key K_MIRAR_IZQ[] = {
  { 0, E_LINEAR, false, nullptr, 0 },
  { 40, E_INQUAD, false, A_MIRAR_IZQ_1, 1 },  // anticipa 1 px
  { 130, E_OUTBACK, false, A_MIRAR_IZQ_2, 3 },
  { 700, E_LINEAR, false, nullptr, 0 },
  { 900, E_INOUTCUBIC, true, nullptr, 0 },
};
const Anim ANIM_MIRAR_IZQ = { K_MIRAR_IZQ, 5, 900 };
const Patch A_MIRAR_DER_1[] = { {F_GX,M_SET,-1.0f} };
const Patch A_MIRAR_DER_2[] = { {L_PX,M_SET,0.65f}, {R_PX,M_SET,0.65f}, {F_GX,M_SET,9.0f} };
const Key K_MIRAR_DER[] = {
  { 0, E_LINEAR, false, nullptr, 0 },
  { 40, E_INQUAD, false, A_MIRAR_DER_1, 1 },  // anticipa 1 px
  { 130, E_OUTBACK, false, A_MIRAR_DER_2, 3 },
  { 700, E_LINEAR, false, nullptr, 0 },
  { 900, E_INOUTCUBIC, true, nullptr, 0 },
};
const Anim ANIM_MIRAR_DER = { K_MIRAR_DER, 5, 900 };
const Patch A_MIRAR_ARR_1[] = { {F_GY,M_ADD,1.0f} };
const Patch A_MIRAR_ARR_2[] = { {L_PY,M_SET,-0.7f}, {R_PY,M_SET,-0.7f}, {L_LIDT,M_SET,0.0f}, {R_LIDT,M_SET,0.0f}, {L_TILTT,M_SET,0.0f}, {R_TILTT,M_SET,0.0f}, {F_GY,M_ADD,-6.0f} };
const Key K_MIRAR_ARR[] = {
  { 0, E_LINEAR, false, nullptr, 0 },
  { 40, E_INQUAD, false, A_MIRAR_ARR_1, 1 },
  { 140, E_OUTBACK, false, A_MIRAR_ARR_2, 7 },
  { 700, E_LINEAR, false, nullptr, 0 },
  { 920, E_INOUTCUBIC, true, nullptr, 0 },
};
const Anim ANIM_MIRAR_ARR = { K_MIRAR_ARR, 5, 920 };
const Patch A_MIRAR_ABA_1[] = { {F_GY,M_ADD,-1.0f} };
const Patch A_MIRAR_ABA_2[] = { {L_PY,M_SET,0.7f}, {R_PY,M_SET,0.7f}, {L_LIDT,M_SET,0.28f}, {R_LIDT,M_SET,0.28f}, {F_GY,M_ADD,5.0f} };
const Key K_MIRAR_ABA[] = {
  { 0, E_LINEAR, false, nullptr, 0 },
  { 40, E_INQUAD, false, A_MIRAR_ABA_1, 1 },
  { 140, E_OUTBACK, false, A_MIRAR_ABA_2, 5 },  // el párpado acompaña
  { 700, E_LINEAR, false, nullptr, 0 },
  { 920, E_INOUTCUBIC, true, nullptr, 0 },
};
const Anim ANIM_MIRAR_ABA = { K_MIRAR_ABA, 5, 920 };
const Key K_SEGUIMIENTO[] = {
  { 0, E_LINEAR, false, nullptr, 0 },
  { 2400, E_LINEAR, false, nullptr, 0 },  // procedural: resorte por tick, ojo derecho 2 frames atrás
};
const Anim ANIM_SEGUIMIENTO = { K_SEGUIMIENTO, 2, 2400 };  // procedural: ver Seguimiento
const Patch A_GUINO_1[] = { {L_LIDB,M_SET,0.32f}, {R_OPEN,M_SET,0.0f}, {F_GX,M_SET,1.0f} };
const Key K_GUINO[] = {
  { 0, E_LINEAR, false, nullptr, 0 },
  { 80, E_INQUAD, false, A_GUINO_1, 3 },  // el otro ojo sonríe
  { 330, E_LINEAR, false, nullptr, 0 },
  { 470, E_OUTBACK, true, nullptr, 0 },
};
const Anim ANIM_GUINO = { K_GUINO, 4, 470 };
const Patch A_CORAZON_1[] = { {L_OPEN,M_SET,0.55f}, {R_OPEN,M_SET,0.55f} };
const Patch A_CORAZON_2[] = { {L_SHAPE,M_SET,1.0f}, {R_SHAPE,M_SET,1.0f}, {L_S,M_SET,0.2f}, {R_S,M_SET,0.2f}, {L_OPEN,M_SET,1.0f}, {R_OPEN,M_SET,1.0f} };
const Patch A_CORAZON_3[] = { {L_S,M_SET,1.12f}, {R_S,M_SET,1.12f} };
const Patch A_CORAZON_4[] = { {L_S,M_SET,0.94f}, {R_S,M_SET,0.94f} };
const Patch A_CORAZON_5[] = { {L_S,M_SET,1.16f}, {R_S,M_SET,1.16f} };
const Patch A_CORAZON_6[] = { {L_S,M_SET,1.0f}, {R_S,M_SET,1.0f} };
const Patch A_CORAZON_8[] = { {L_S,M_SET,0.2f}, {R_S,M_SET,0.2f} };
const Patch A_CORAZON_9[] = { {L_SHAPE,M_SET,0.0f}, {R_SHAPE,M_SET,0.0f}, {L_OPEN,M_SET,0.0f}, {R_OPEN,M_SET,0.0f}, {L_S,M_SET,1.0f}, {R_S,M_SET,1.0f} };
const Key K_CORAZON[] = {
  { 0, E_LINEAR, false, nullptr, 0 },
  { 90, E_INQUAD, false, A_CORAZON_1, 2 },  // anticipa (squash)
  { 100, E_LINEAR, false, A_CORAZON_2, 6 },  // cambio de forma
  { 360, E_OUTBACK, false, A_CORAZON_3, 2 },  // pop
  { 480, E_INOUTSINE, false, A_CORAZON_4, 2 },  // lub
  { 600, E_OUTQUAD, false, A_CORAZON_5, 2 },  // dub
  { 800, E_INOUTSINE, false, A_CORAZON_6, 2 },
  { 1200, E_LINEAR, false, nullptr, 0 },
  { 1290, E_INQUAD, false, A_CORAZON_8, 2 },
  { 1300, E_LINEAR, false, A_CORAZON_9, 6 },  // vuelve tras un parpadeo
  { 1500, E_OUTBACK, true, nullptr, 0 },
};
const Anim ANIM_CORAZON = { K_CORAZON, 11, 1500 };
const Patch A_ESTRELLA_1[] = { {L_OPEN,M_SET,0.55f}, {R_OPEN,M_SET,0.55f} };
const Patch A_ESTRELLA_2[] = { {L_SHAPE,M_SET,2.0f}, {R_SHAPE,M_SET,2.0f}, {L_S,M_SET,0.2f}, {R_S,M_SET,0.2f}, {L_ROT,M_SET,-40.0f}, {R_ROT,M_SET,-40.0f}, {F_SPARK,M_SET,1.0f}, {L_OPEN,M_SET,1.0f}, {R_OPEN,M_SET,1.0f} };
const Patch A_ESTRELLA_3[] = { {L_S,M_SET,1.1f}, {R_S,M_SET,1.1f}, {L_ROT,M_SET,8.0f}, {R_ROT,M_SET,8.0f} };
const Patch A_ESTRELLA_4[] = { {L_S,M_SET,1.0f}, {R_S,M_SET,1.0f}, {L_ROT,M_SET,0.0f}, {R_ROT,M_SET,0.0f} };
const Patch A_ESTRELLA_5[] = { {L_S,M_SET,1.08f}, {R_S,M_SET,1.08f}, {L_ROT,M_SET,6.0f}, {R_ROT,M_SET,6.0f} };
const Patch A_ESTRELLA_6[] = { {L_S,M_SET,1.0f}, {R_S,M_SET,1.0f}, {L_ROT,M_SET,0.0f}, {R_ROT,M_SET,0.0f} };
const Patch A_ESTRELLA_7[] = { {L_S,M_SET,0.2f}, {R_S,M_SET,0.2f} };
const Patch A_ESTRELLA_8[] = { {L_SHAPE,M_SET,0.0f}, {R_SHAPE,M_SET,0.0f}, {L_OPEN,M_SET,0.0f}, {R_OPEN,M_SET,0.0f}, {L_S,M_SET,1.0f}, {R_S,M_SET,1.0f}, {L_ROT,M_SET,0.0f}, {R_ROT,M_SET,0.0f}, {F_SPARK,M_SET,0.0f} };
const Key K_ESTRELLA[] = {
  { 0, E_LINEAR, false, nullptr, 0 },
  { 90, E_INQUAD, false, A_ESTRELLA_1, 2 },
  { 100, E_LINEAR, false, A_ESTRELLA_2, 9 },
  { 380, E_OUTBACK, false, A_ESTRELLA_3, 4 },  // pop + giro
  { 560, E_INOUTSINE, false, A_ESTRELLA_4, 4 },
  { 900, E_INOUTSINE, false, A_ESTRELLA_5, 4 },  // titila
  { 1150, E_INOUTSINE, false, A_ESTRELLA_6, 4 },
  { 1290, E_INQUAD, false, A_ESTRELLA_7, 2 },
  { 1300, E_LINEAR, false, A_ESTRELLA_8, 9 },
  { 1500, E_OUTBACK, true, nullptr, 0 },
};
const Anim ANIM_ESTRELLA = { K_ESTRELLA, 10, 1500 };
const Patch A_SOSPECHOSA_1[] = { {L_LIDT,M_SET,0.52f}, {R_LIDT,M_SET,0.52f}, {L_TILTT,M_SET,2.0f}, {R_TILTT,M_SET,2.0f}, {L_LIDB,M_SET,0.0f}, {R_LIDB,M_SET,0.0f}, {L_PR,M_SET,6.0f}, {R_PR,M_SET,6.0f}, {L_PX,M_SET,0.0f}, {R_PX,M_SET,0.0f} };
const Patch A_SOSPECHOSA_2[] = { {L_PX,M_SET,-0.7f}, {R_PX,M_SET,-0.7f} };
const Patch A_SOSPECHOSA_4[] = { {L_PX,M_SET,0.7f}, {R_PX,M_SET,0.7f} };
const Key K_SOSPECHOSA[] = {
  { 0, E_LINEAR, false, nullptr, 0 },
  { 260, E_INOUTCUBIC, false, A_SOSPECHOSA_1, 10 },  // párpados a media altura
  { 900, E_INOUTSINE, false, A_SOSPECHOSA_2, 2 },
  { 1300, E_LINEAR, false, nullptr, 0 },
  { 1900, E_INOUTSINE, false, A_SOSPECHOSA_4, 2 },
  { 2300, E_LINEAR, false, nullptr, 0 },
  { 2700, E_OUTBACK, true, nullptr, 0 },
};
const Anim ANIM_SOSPECHOSA = { K_SOSPECHOSA, 7, 2700 };
const Patch A_SORPRESA_1[] = { {L_H,M_MUL,0.82f}, {R_H,M_MUL,0.82f}, {L_W,M_MUL,1.06f}, {R_W,M_MUL,1.06f}, {F_GY,M_ADD,2.0f} };
const Patch A_SORPRESA_2[] = { {L_W,M_SET,38.0f}, {R_W,M_SET,38.0f}, {L_H,M_SET,52.0f}, {R_H,M_SET,52.0f}, {L_R,M_SET,5.0f}, {R_R,M_SET,5.0f}, {L_LIDT,M_SET,0.0f}, {R_LIDT,M_SET,0.0f}, {L_TILTT,M_SET,0.0f}, {R_TILTT,M_SET,0.0f}, {L_LIDB,M_SET,0.0f}, {R_LIDB,M_SET,0.0f}, {L_PR,M_SET,4.0f}, {R_PR,M_SET,4.0f}, {L_HL,M_SET,0.0f}, {R_HL,M_SET,0.0f}, {F_GY,M_ADD,-3.0f} };
const Patch A_SORPRESA_3[] = { {L_H,M_SET,48.0f}, {R_H,M_SET,48.0f} };
const Key K_SORPRESA[] = {
  { 0, E_LINEAR, false, nullptr, 0 },
  { 90, E_INQUAD, false, A_SORPRESA_1, 5 },  // se agacha
  { 230, E_OUTBACK, false, A_SORPRESA_2, 17 },  // ¡pop!
  { 520, E_INOUTSINE, false, A_SORPRESA_3, 2 },  // asienta
  { 1000, E_LINEAR, false, nullptr, 0 },
  { 1300, E_INOUTCUBIC, true, nullptr, 0 },
};
const Anim ANIM_SORPRESA = { K_SORPRESA, 6, 1300 };
const Patch A_BOSTEZO_1[] = { {L_LIDT,M_SET,0.35f}, {R_LIDT,M_SET,0.35f}, {L_TILTT,M_SET,-6.0f}, {R_TILTT,M_SET,-6.0f}, {F_GY,M_ADD,-2.0f} };
const Patch A_BOSTEZO_2[] = { {L_OPEN,M_SET,0.14f}, {R_OPEN,M_SET,0.14f}, {L_LIDB,M_SET,0.3f}, {R_LIDB,M_SET,0.3f}, {L_TILTT,M_SET,-6.0f}, {R_TILTT,M_SET,-6.0f}, {F_GY,M_ADD,-3.0f} };
const Patch A_BOSTEZO_5[] = { {L_OPEN,M_SET,0.0f}, {R_OPEN,M_SET,0.0f} };
const Key K_BOSTEZO[] = {
  { 0, E_LINEAR, false, nullptr, 0 },
  { 500, E_INOUTSINE, false, A_BOSTEZO_1, 5 },
  { 950, E_INQUAD, false, A_BOSTEZO_2, 7 },  // aprieta
  { 1750, E_LINEAR, false, nullptr, 0 },
  { 2150, E_OUTQUAD, true, nullptr, 0 },
  { 2260, E_INQUAD, false, A_BOSTEZO_5, 2 },
  { 2500, E_OUTBACK, true, nullptr, 0 },
};
const Anim ANIM_BOSTEZO = { K_BOSTEZO, 7, 2500 };
const Patch A_ESTORNUDO_1[] = { {L_LIDT,M_SET,0.25f}, {R_LIDT,M_SET,0.25f}, {L_TILTT,M_SET,-5.0f}, {R_TILTT,M_SET,-5.0f}, {F_GY,M_ADD,-2.0f} };
const Patch A_ESTORNUDO_3[] = { {L_LIDT,M_SET,0.45f}, {R_LIDT,M_SET,0.45f}, {L_TILTT,M_SET,-8.0f}, {R_TILTT,M_SET,-8.0f}, {L_PY,M_SET,-0.5f}, {R_PY,M_SET,-0.5f}, {F_GY,M_ADD,-4.0f} };
const Patch A_ESTORNUDO_4[] = { {L_SHAPE,M_SET,5.0f}, {R_SHAPE,M_SET,5.0f}, {L_S,M_SET,1.0f}, {R_S,M_SET,1.0f}, {F_GY,M_ADD,4.0f}, {F_SHAKE,M_SET,3.0f} };
const Patch A_ESTORNUDO_5[] = { {F_SHAKE,M_SET,0.0f} };
const Patch A_ESTORNUDO_6[] = { {L_SHAPE,M_SET,0.0f}, {R_SHAPE,M_SET,0.0f}, {L_OPEN,M_SET,0.0f}, {R_OPEN,M_SET,0.0f}, {F_SHAKE,M_SET,0.0f} };
const Key K_ESTORNUDO[] = {
  { 0, E_LINEAR, false, nullptr, 0 },
  { 350, E_INOUTSINE, false, A_ESTORNUDO_1, 5 },  // a…
  { 550, E_LINEAR, false, nullptr, 0 },
  { 880, E_INQUAD, false, A_ESTORNUDO_3, 7 },  // a… a…
  { 920, E_LINEAR, false, A_ESTORNUDO_4, 6 },  // ¡chú! (> <)
  { 1150, E_OUTQUAD, false, A_ESTORNUDO_5, 1 },
  { 1200, E_LINEAR, false, A_ESTORNUDO_6, 5 },
  { 1500, E_OUTBACK, true, nullptr, 0 },
};
const Anim ANIM_ESTORNUDO = { K_ESTORNUDO, 8, 1500 };
const Patch A_MIRAR_ALREDEDOR_1[] = { {L_PX,M_SET,-0.65f}, {R_PX,M_SET,-0.65f}, {F_GX,M_SET,-9.0f} };
const Patch A_MIRAR_ALREDEDOR_3[] = { {L_PX,M_SET,0.65f}, {R_PX,M_SET,0.65f}, {F_GX,M_SET,9.0f} };
const Patch A_MIRAR_ALREDEDOR_5[] = { {L_PX,M_SET,-0.4f}, {R_PX,M_SET,-0.4f}, {L_PY,M_SET,-0.65f}, {R_PY,M_SET,-0.65f}, {L_LIDT,M_SET,0.0f}, {R_LIDT,M_SET,0.0f}, {L_TILTT,M_SET,0.0f}, {R_TILTT,M_SET,0.0f}, {F_GX,M_SET,-5.0f}, {F_GY,M_ADD,-4.0f} };
const Key K_MIRAR_ALREDEDOR[] = {
  { 0, E_LINEAR, false, nullptr, 0 },
  { 130, E_OUTBACK, false, A_MIRAR_ALREDEDOR_1, 3 },
  { 700, E_LINEAR, false, nullptr, 0 },
  { 830, E_OUTBACK, false, A_MIRAR_ALREDEDOR_3, 3 },
  { 1400, E_LINEAR, false, nullptr, 0 },
  { 1530, E_OUTBACK, false, A_MIRAR_ALREDEDOR_5, 10 },
  { 2050, E_LINEAR, false, nullptr, 0 },
  { 2300, E_INOUTCUBIC, true, nullptr, 0 },
};
const Anim ANIM_MIRAR_ALREDEDOR = { K_MIRAR_ALREDEDOR, 8, 2300 };
const Patch A_ABURRIDO_1[] = { {L_LIDT,M_SET,0.45f}, {R_LIDT,M_SET,0.45f}, {L_TILTT,M_SET,0.0f}, {R_TILTT,M_SET,0.0f}, {L_LIDB,M_SET,0.0f}, {R_LIDB,M_SET,0.0f}, {L_PY,M_SET,0.45f}, {R_PY,M_SET,0.45f}, {L_PX,M_SET,-0.6f}, {R_PX,M_SET,-0.6f} };
const Patch A_ABURRIDO_2[] = { {L_PX,M_SET,0.6f}, {R_PX,M_SET,0.6f} };
const Patch A_ABURRIDO_3[] = { {L_OPEN,M_SET,0.35f}, {R_OPEN,M_SET,0.35f} };
const Patch A_ABURRIDO_4[] = { {L_OPEN,M_SET,1.0f}, {R_OPEN,M_SET,1.0f} };
const Key K_ABURRIDO[] = {
  { 0, E_LINEAR, false, nullptr, 0 },
  { 450, E_INOUTSINE, false, A_ABURRIDO_1, 10 },
  { 2050, E_LINEAR, false, A_ABURRIDO_2, 2 },  // deriva
  { 2350, E_INOUTSINE, false, A_ABURRIDO_3, 2 },  // suspiro
  { 2650, E_INOUTSINE, false, A_ABURRIDO_4, 2 },
  { 3300, E_INOUTCUBIC, true, nullptr, 0 },
};
const Anim ANIM_ABURRIDO = { K_ABURRIDO, 6, 3300 };
const Patch A_MAREO_IDLE_1[] = { {L_OPEN,M_SET,0.0f}, {R_OPEN,M_SET,0.0f} };
const Patch A_MAREO_IDLE_2[] = { {L_SHAPE,M_SET,4.0f}, {R_SHAPE,M_SET,4.0f}, {L_S,M_SET,0.3f}, {R_S,M_SET,0.3f}, {F_SHAKE,M_SET,1.0f}, {L_OPEN,M_SET,1.0f}, {R_OPEN,M_SET,1.0f} };
const Patch A_MAREO_IDLE_3[] = { {L_S,M_SET,1.0f}, {R_S,M_SET,1.0f} };
const Patch A_MAREO_IDLE_5[] = { {L_S,M_SET,0.3f}, {R_S,M_SET,0.3f} };
const Patch A_MAREO_IDLE_6[] = { {L_SHAPE,M_SET,0.0f}, {R_SHAPE,M_SET,0.0f}, {L_OPEN,M_SET,0.0f}, {R_OPEN,M_SET,0.0f}, {L_S,M_SET,1.0f}, {R_S,M_SET,1.0f}, {F_SHAKE,M_SET,0.0f} };
const Key K_MAREO_IDLE[] = {
  { 0, E_LINEAR, false, nullptr, 0 },
  { 120, E_INQUAD, false, A_MAREO_IDLE_1, 2 },
  { 130, E_LINEAR, false, A_MAREO_IDLE_2, 7 },
  { 380, E_OUTBACK, false, A_MAREO_IDLE_3, 2 },
  { 1800, E_LINEAR, false, nullptr, 0 },
  { 1900, E_INQUAD, false, A_MAREO_IDLE_5, 2 },
  { 1910, E_LINEAR, false, A_MAREO_IDLE_6, 7 },
  { 2200, E_OUTQUAD, true, nullptr, 0 },
};
const Anim ANIM_MAREO_IDLE = { K_MAREO_IDLE, 8, 2200 };
const Patch A_COSQUILLAS_1[] = { {L_LIDB,M_SET,0.58f}, {R_LIDB,M_SET,0.58f}, {L_LIDT,M_SET,0.0f}, {R_LIDT,M_SET,0.0f}, {L_TILTT,M_SET,0.0f}, {R_TILTT,M_SET,0.0f}, {L_PR,M_SET,0.0f}, {R_PR,M_SET,0.0f}, {L_HL,M_SET,0.0f}, {R_HL,M_SET,0.0f}, {F_GY,M_ADD,-2.0f} };
const Patch A_COSQUILLAS_2[] = { {F_GX,M_SET,-2.0f} };
const Patch A_COSQUILLAS_3[] = { {F_GX,M_SET,2.0f} };
const Patch A_COSQUILLAS_4[] = { {F_GX,M_SET,-2.0f} };
const Patch A_COSQUILLAS_5[] = { {F_GX,M_SET,1.0f} };
const Patch A_COSQUILLAS_6[] = { {F_GX,M_SET,0.0f} };
const Key K_COSQUILLAS[] = {
  { 0, E_LINEAR, false, nullptr, 0 },
  { 90, E_OUTQUAD, false, A_COSQUILLAS_1, 11 },
  { 170, E_INOUTSINE, false, A_COSQUILLAS_2, 1 },
  { 250, E_INOUTSINE, false, A_COSQUILLAS_3, 1 },
  { 330, E_INOUTSINE, false, A_COSQUILLAS_4, 1 },
  { 410, E_INOUTSINE, false, A_COSQUILLAS_5, 1 },
  { 490, E_INOUTSINE, false, A_COSQUILLAS_6, 1 },
  { 700, E_LINEAR, false, nullptr, 0 },
  { 900, E_INOUTCUBIC, true, nullptr, 0 },
};
const Anim ANIM_COSQUILLAS = { K_COSQUILLAS, 9, 900 };
const Patch A_TROMPO_1[] = { {L_OPEN,M_SET,0.7f}, {R_OPEN,M_SET,0.7f}, {F_GY,M_ADD,2.0f} };
const Patch A_TROMPO_2[] = { {L_OPEN,M_SET,1.0f}, {R_OPEN,M_SET,1.0f}, {F_GX,M_SET,1.2f}, {F_GY,M_ADD,0.0f} };
const Patch A_TROMPO_3[] = { {L_OPEN,M_SET,1.0f}, {R_OPEN,M_SET,1.0f}, {F_GX,M_SET,4.6f}, {F_GY,M_ADD,0.7f} };
const Patch A_TROMPO_4[] = { {L_OPEN,M_SET,1.0f}, {R_OPEN,M_SET,1.0f}, {F_GX,M_SET,8.7f}, {F_GY,M_ADD,3.1f} };
const Patch A_TROMPO_5[] = { {L_OPEN,M_SET,1.0f}, {R_OPEN,M_SET,1.0f}, {F_GX,M_SET,9.6f}, {F_GY,M_ADD,7.6f} };
const Patch A_TROMPO_6[] = { {L_OPEN,M_SET,1.0f}, {R_OPEN,M_SET,1.0f}, {F_GX,M_SET,3.4f}, {F_GY,M_ADD,11.6f} };
const Patch A_TROMPO_7[] = { {L_OPEN,M_SET,1.0f}, {R_OPEN,M_SET,1.0f}, {F_GX,M_SET,-6.7f}, {F_GY,M_ADD,10.4f} };
const Patch A_TROMPO_8[] = { {L_OPEN,M_SET,1.0f}, {R_OPEN,M_SET,1.0f}, {F_GX,M_SET,-9.4f}, {F_GY,M_ADD,4.0f} };
const Patch A_TROMPO_9[] = { {L_OPEN,M_SET,1.0f}, {R_OPEN,M_SET,1.0f}, {F_GX,M_SET,0.0f}, {F_GY,M_ADD,0.0f} };
const Patch A_TROMPO_10[] = { {L_OPEN,M_SET,1.0f}, {R_OPEN,M_SET,1.0f}, {F_GX,M_SET,9.4f}, {F_GY,M_ADD,4.0f} };
const Patch A_TROMPO_11[] = { {L_OPEN,M_SET,1.0f}, {R_OPEN,M_SET,1.0f}, {F_GX,M_SET,6.7f}, {F_GY,M_ADD,10.4f} };
const Patch A_TROMPO_12[] = { {L_OPEN,M_SET,1.0f}, {R_OPEN,M_SET,1.0f}, {F_GX,M_SET,-3.4f}, {F_GY,M_ADD,11.6f} };
const Patch A_TROMPO_13[] = { {L_OPEN,M_SET,1.0f}, {R_OPEN,M_SET,1.0f}, {F_GX,M_SET,-9.6f}, {F_GY,M_ADD,7.6f} };
const Patch A_TROMPO_14[] = { {L_OPEN,M_SET,1.0f}, {R_OPEN,M_SET,1.0f}, {F_GX,M_SET,-8.7f}, {F_GY,M_ADD,3.1f} };
const Patch A_TROMPO_15[] = { {L_OPEN,M_SET,1.0f}, {R_OPEN,M_SET,1.0f}, {F_GX,M_SET,-4.6f}, {F_GY,M_ADD,0.7f} };
const Patch A_TROMPO_16[] = { {L_OPEN,M_SET,1.0f}, {R_OPEN,M_SET,1.0f}, {F_GX,M_SET,-1.2f}, {F_GY,M_ADD,0.0f} };
const Patch A_TROMPO_17[] = { {L_OPEN,M_SET,1.0f}, {R_OPEN,M_SET,1.0f}, {F_GX,M_SET,0.0f}, {F_GY,M_ADD,0.0f} };
const Patch A_TROMPO_18[] = { {L_SHAPE,M_SET,4.0f}, {R_SHAPE,M_SET,4.0f}, {L_S,M_SET,1.0f}, {R_S,M_SET,1.0f}, {F_SHAKE,M_SET,1.0f}, {F_GX,M_SET,0.0f}, {F_GY,M_BASE,0.0f} };
const Patch A_TROMPO_19[] = { {F_GX,M_SET,-3.0f} };
const Patch A_TROMPO_20[] = { {F_GX,M_SET,3.0f} };
const Patch A_TROMPO_21[] = { {F_GX,M_SET,-3.0f} };
const Patch A_TROMPO_22[] = { {F_GX,M_SET,2.0f} };
const Patch A_TROMPO_23[] = { {F_GX,M_SET,0.0f} };
const Patch A_TROMPO_24[] = { {L_SHAPE,M_SET,0.0f}, {R_SHAPE,M_SET,0.0f}, {L_OPEN,M_SET,0.0f}, {R_OPEN,M_SET,0.0f}, {F_SHAKE,M_SET,0.0f} };
const Patch A_TROMPO_26[] = { {F_GX,M_SET,-4.0f} };
const Patch A_TROMPO_27[] = { {F_GX,M_SET,3.0f} };
const Patch A_TROMPO_28[] = { {F_GX,M_SET,-2.0f} };
const Patch A_TROMPO_29[] = { {F_GX,M_SET,1.0f} };
const Patch A_TROMPO_30[] = { {F_GX,M_SET,0.0f} };
const Patch A_TROMPO_31[] = { {L_LIDB,M_SET,0.58f}, {R_LIDB,M_SET,0.58f}, {L_LIDT,M_SET,0.0f}, {R_LIDT,M_SET,0.0f}, {L_TILTT,M_SET,0.0f}, {R_TILTT,M_SET,0.0f}, {L_PR,M_SET,0.0f}, {R_PR,M_SET,0.0f}, {L_HL,M_SET,0.0f}, {R_HL,M_SET,0.0f}, {F_GY,M_ADD,-2.0f} };
const Key K_TROMPO[] = {
  { 0, E_LINEAR, false, nullptr, 0 },
  { 140, E_INQUAD, false, A_TROMPO_1, 3 },  // anticipa
  { 280, E_LINEAR, false, A_TROMPO_2, 4 },
  { 340, E_LINEAR, false, A_TROMPO_3, 4 },
  { 400, E_LINEAR, false, A_TROMPO_4, 4 },
  { 460, E_LINEAR, false, A_TROMPO_5, 4 },
  { 520, E_LINEAR, false, A_TROMPO_6, 4 },
  { 580, E_LINEAR, false, A_TROMPO_7, 4 },
  { 640, E_LINEAR, false, A_TROMPO_8, 4 },
  { 700, E_LINEAR, false, A_TROMPO_9, 4 },
  { 760, E_LINEAR, false, A_TROMPO_10, 4 },
  { 820, E_LINEAR, false, A_TROMPO_11, 4 },
  { 880, E_LINEAR, false, A_TROMPO_12, 4 },
  { 940, E_LINEAR, false, A_TROMPO_13, 4 },
  { 1000, E_LINEAR, false, A_TROMPO_14, 4 },
  { 1060, E_LINEAR, false, A_TROMPO_15, 4 },
  { 1120, E_LINEAR, false, A_TROMPO_16, 4 },
  { 1180, E_LINEAR, false, A_TROMPO_17, 4 },
  { 1240, E_LINEAR, false, A_TROMPO_18, 7 },  // espirales
  { 1440, E_INOUTSINE, false, A_TROMPO_19, 1 },
  { 1640, E_INOUTSINE, false, A_TROMPO_20, 1 },
  { 1840, E_INOUTSINE, false, A_TROMPO_21, 1 },
  { 2040, E_INOUTSINE, false, A_TROMPO_22, 1 },
  { 2240, E_INOUTSINE, false, A_TROMPO_23, 1 },
  { 2400, E_LINEAR, false, A_TROMPO_24, 5 },
  { 2560, E_OUTBACK, true, nullptr, 0 },  // se sacude
  { 2640, E_OUTQUAD, false, A_TROMPO_26, 1 },
  { 2720, E_INOUTSINE, false, A_TROMPO_27, 1 },
  { 2800, E_INOUTSINE, false, A_TROMPO_28, 1 },
  { 2880, E_INOUTSINE, false, A_TROMPO_29, 1 },
  { 2980, E_INOUTSINE, false, A_TROMPO_30, 1 },
  { 3100, E_OUTBACK, false, A_TROMPO_31, 11 },  // ^^
  { 3700, E_LINEAR, false, nullptr, 0 },
  { 4000, E_INOUTCUBIC, true, nullptr, 0 },
};
const Anim ANIM_TROMPO = { K_TROMPO, 34, 4000 };
const Patch A_ARRANQUE_0[] = { {L_OPEN,M_SET,0.0f}, {R_OPEN,M_SET,0.0f}, {L_W,M_SET,0.0f}, {R_W,M_SET,0.0f}, {L_PR,M_SET,0.0f}, {R_PR,M_SET,0.0f}, {L_HL,M_SET,0.0f}, {R_HL,M_SET,0.0f}, {F_MERGE,M_SET,1.0f} };
const Patch A_ARRANQUE_1[] = { {L_OPEN,M_SET,0.0f}, {R_OPEN,M_SET,0.0f}, {L_W,M_SET,80.0f}, {R_W,M_SET,80.0f}, {L_PR,M_SET,0.0f}, {R_PR,M_SET,0.0f}, {L_HL,M_SET,0.0f}, {R_HL,M_SET,0.0f}, {F_MERGE,M_SET,1.0f} };
const Patch A_ARRANQUE_3[] = { {L_W,M_BASE,0.0f}, {R_W,M_BASE,0.0f}, {F_MERGE,M_SET,0.0f} };
const Patch A_ARRANQUE_7[] = { {L_PX,M_SET,-0.65f}, {R_PX,M_SET,-0.65f}, {F_GX,M_SET,-9.0f} };
const Patch A_ARRANQUE_9[] = { {L_PX,M_SET,0.65f}, {R_PX,M_SET,0.65f}, {F_GX,M_SET,9.0f} };
const Patch A_ARRANQUE_12[] = { {L_OPEN,M_SET,0.0f}, {R_OPEN,M_SET,0.0f} };
const Key K_ARRANQUE[] = {
  { 0, E_LINEAR, false, A_ARRANQUE_0, 9 },
  { 320, E_OUTCUBIC, false, A_ARRANQUE_1, 9 },  // nace la línea
  { 420, E_LINEAR, false, nullptr, 0 },
  { 680, E_INOUTCUBIC, false, A_ARRANQUE_3, 3 },  // se parte en dos
  { 760, E_LINEAR, false, nullptr, 0 },
  { 960, E_OUTBACK, true, nullptr, 0 },  // abre
  { 1150, E_LINEAR, false, nullptr, 0 },
  { 1300, E_OUTBACK, false, A_ARRANQUE_7, 3 },
  { 1650, E_LINEAR, false, nullptr, 0 },
  { 1800, E_OUTBACK, false, A_ARRANQUE_9, 3 },
  { 2150, E_LINEAR, false, nullptr, 0 },
  { 2300, E_INOUTCUBIC, true, nullptr, 0 },
  { 2380, E_INQUAD, false, A_ARRANQUE_12, 2 },
  { 2500, E_OUTBACK, true, nullptr, 0 },
};
const Anim ANIM_ARRANQUE = { K_ARRANQUE, 14, 2500 };