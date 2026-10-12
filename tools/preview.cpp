// Render de escritorio: corre la maquina de estados y el render reales (los mismos .h del firmware)
// y escribe los frames 128x64 crudos (1 byte por pixel) para armar GIFs de vista previa.
// Uso: preview <estilo> <escenario> <ms> <salida.raw>
//   escenarios: tranquilo atento preocupado estresado agotado cosquillas trompo sin_wifi conectando sin_datos tour
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/oled_buf.h"
#include "../src/eyes_machine.h"

int main(int argc, char** argv) {
  if (argc < 5) { fprintf(stderr, "uso: preview <estilo> <escenario> <ms> <salida.raw>\n"); return 1; }
  int style = atoi(argv[1]);
  const char* sc = argv[2];
  int ms = atoi(argv[3]);
  FILE* f = fopen(argv[4], "wb");
  if (!f) return 1;

  static uint8_t buf[128 * 64 / 8];
  OledBuf g; g.attach(buf);
  eyes::Machine m;
  int pct = !strcmp(sc, "atento") ? 60 : !strcmp(sc, "preocupado") ? 80 : !strcmp(sc, "estresado") ? 95
          : !strcmp(sc, "agotado") ? 100 : 20;
  m.begin(12345, pct);
  bool tour = !strcmp(sc, "tour");
  // salta la animacion de arranque salvo en el tour
  if (!tour) { m.setSystem(eyes::SYS_NORMAL); for (int t = 0; t < 2500; t += eyes::TICK_MS) m.tick(); }
  else m.setSystem(eyes::SYS_NORMAL);
  if (!strcmp(sc, "sin_wifi")) m.setSystem(eyes::SYS_SIN_WIFI);
  if (!strcmp(sc, "conectando")) m.setSystem(eyes::SYS_CONECTANDO);
  if (!strcmp(sc, "sin_datos")) m.setSystem(eyes::SYS_SIN_DATOS);
  if (!strcmp(sc, "trompo")) m.egg();

  static uint8_t px[128 * 64];
  for (int t = 0; t < ms; t += eyes::TICK_MS) {
    if (!strcmp(sc, "cosquillas") && (t == 300 || t == 1500)) m.tap();
    if (tour) {  // recorre los humores subiendo la cuota
      static const int steps[][2] = {{3000, 60}, {5500, 80}, {8000, 95}, {10500, 100}, {13500, 20}};
      for (auto& s : steps) if (t == s[0] - s[0] % eyes::TICK_MS) m.setPercent(s[1]);
    }
    m.tick();
    eyes::render(g, m.pose(), style, m.clock());
    for (int y = 0; y < 64; y++) for (int x = 0; x < 128; x++) px[y * 128 + x] = g.getPixel(x, y) ? 1 : 0;
    fwrite(px, 1, sizeof(px), f);
  }
  fclose(f);
  return 0;
}
