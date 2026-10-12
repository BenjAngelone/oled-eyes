#include <Arduino.h>
/*
  ============================================================================
  OLED 1.3" (SSD1306, I2C) - "Ojos OLED" que reaccionan al consumo
  de cuota de una sesion de Claude Code.

  Arquitectura:
    PC (bridge.py lee ccusage) --POST JSON--> placa (/update) --> ojos en OLED

  Dibujo y animacion: Design System "Ojos OLED" (estilo A - Bloque)
    oled_eyes_data.h  poses, parches y animaciones (GENERADO, no editar a mano)
    oled_eyes.h       runtime: easing, evalAnim, evalTrans, render
    oled_buf.h        primitivas tipo Adafruit_GFX sobre el buffer de U8g2
    eyes_machine.h    maquina de estados (humores, parpadeo, sacadas, idles, sistema)

  Placas (mismo codigo, ramas #ifdef ESP8266; un env de PlatformIO por placa):
    ESP32-C3 (SuperMini / DevKitM-1), core ESP32 (Espressif) 3.x
    NodeMCU v3 (ESP8266, ESP-12E)
  ============================================================================
*/

// ============================================================================
// LIBRERIAS
// ============================================================================
#include <U8g2lib.h>
#include <Wire.h>
#ifdef ESP8266
  #include <ESP8266WiFi.h>
  #include <ESP8266WebServer.h>
  #include <ESP8266mDNS.h>
#else
  #include <WiFi.h>
  #include <WebServer.h>
  #include <ESPmDNS.h>
#endif
#include <ArduinoJson.h>
#include <time.h>

#include "oled_buf.h"
#include "eyes_machine.h"

// ============================================================================
// CONFIGURACION DE PINES (facil de cambiar)
// ============================================================================
#ifdef ESP8266
  #define SDA_PIN 4   // D2 en NodeMCU
  #define SCL_PIN 5   // D1 en NodeMCU
  // Boton del easter egg: el pulsador FLASH de la placa (GPIO0, activo en bajo): no hace falta cablear nada.
  #define BUTTON_PIN 0
  #define ISR_ATTR ICACHE_RAM_ATTR
#else
  #define SDA_PIN 8
  #define SCL_PIN 9
  // Boton del easter egg: un lado al pin, el otro a GND (pull-up interno).
  // OJO: en el SuperMini el GPIO9 es el boton BOOT y es el SCL del OLED, por eso no se usa.
  #define BUTTON_PIN 4
  #define ISR_ATTR IRAM_ATTR
#endif
#define BUTTON_DEBOUNCE_MS 40

// ============================================================================
// CONFIGURACION DE PANTALLA
// ============================================================================
// Constructor full-buffer: el render escribe directo en el buffer de U8g2 (oled_buf.h)
// y sendBuffer() lo manda entero. OJO: este modulo de 1.3" resulto ser SSD1306, no SH1106
// (probado a mano). Si alguna vez la pantalla queda negra con I2C detectado en 0x3C, probar
// el otro constructor.
// U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// Rotacion: U8G2_R0 normal, U8G2_R2 = 180 grados. NO usar U8G2_MIRROR:
// deja la pantalla en negro en este modulo.
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

#define OLED_I2C_ADDR 0x3C
#define I2C_CLOCK_HZ 400000

// Estilo de ojos: ST_MANCUERNA (panel blanco, dos puntos negros unidos por una barra; el panel se
// invierte por hardware) o ST_BLOQUE (ojos blancos sobre negro, sin invertir).
// La inversion se manda una sola vez al iniciar (nunca por frame): ver applyStyleInvert().
#ifndef STYLE
#define STYLE eyes::ST_MANCUERNA
#endif

// ---- Cuidado del panel (OBLIGATORIO con Mancuerna: ~95% de los pixeles encendidos) ----
// Atenuado: sin cambios de cuota, de estado ni toques durante DIM_AFTER_MS baja el contraste.
// Cualquier cambio de %, de estado o un tap lo restaura.
#ifndef DIM_AFTER_MS
#define DIM_AFTER_MS (10UL * 60UL * 1000UL)
#endif
// Cada PANEL_REFRESH_MS se reenvia la config del panel (charge pump, display on, inversion, contraste):
// si el SSD1306 se reseteo por una caida de tension, se recupera solo. 0 = desactivado.
#ifndef PANEL_REFRESH_MS
#define PANEL_REFRESH_MS 3000UL
#endif
// Con ~95% del panel encendido las filas con menos pixeles prendidos (donde estan los puntos y la barra)
// se ven con mas brillo: caida de tension (IR drop). Se baja el contraste y se ajustan precarga y VCOMH.
// Se pueden probar en vivo con GET /tune?contrast=..&precharge=..&vcomh=.. (ver handleTune).
#ifndef CONTRAST_NORMAL
#define CONTRAST_NORMAL 0x7F  // el valor por defecto del controlador es 0xCF (207)
#endif
#ifndef OLED_PRECHARGE
#define OLED_PRECHARGE 0xF1   // comando 0xD9; por defecto 0x22
#endif
#ifndef OLED_VCOMH
#define OLED_VCOMH 0x20       // comando 0xDB; por defecto 0x20
#endif
#define CONTRAST_DIM 0x10
// Horario nocturno (hora local por NTP): panel apagado (DISPLAYOFF). Un tap lo prende NIGHT_WAKE_MS.
// Hoy desactivado (START == END).
// Si START == END queda desactivado. Admite ventanas que cruzan la medianoche (ej. 22 -> 6).
#ifndef NIGHT_START_HOUR
#define NIGHT_START_HOUR 0
#endif
#ifndef NIGHT_END_HOUR
#define NIGHT_END_HOUR 0      // desactivado a pedido del dueno (START == END). Para 00-08 h: 8
#endif
#ifndef NIGHT_WAKE_MS
#define NIGHT_WAKE_MS 30000UL
#endif

// Log de rendimiento por Serial (render / display / fps) cada PERF_LOG_MS. 0 = apagado.
#define PERF_LOG_MS 5000

// ============================================================================
// CONFIGURACION WIFI Y RED
// ============================================================================
#if __has_include("secrets.h")
#include "secrets.h"  // WIFI_SSID / WIFI_PASS (no se versiona; ver include/secrets.example.h)
#else
#error "Copiar include/secrets.example.h a include/secrets.h y cargar la red WiFi"
#endif
const char* MDNS_HOST = "esp32-eyes"; // -> http://esp32-eyes.local

// NTP - Argentina (UTC-3, sin horario de verano)
const char* NTP_SERVER = "pool.ntp.org";
const char* TZ_ARGENTINA = "<-03>3";

// ============================================================================
// CONSTANTES DE TUNING
// ============================================================================
#define NO_DATA_TIMEOUT_MS (10UL * 60UL * 1000UL) // 10 minutos = "sin datos"

#ifdef ESP8266
  // Medido: el ESP8266 tarda ~10,5 s en asociarse. Con reintentos cada 5 s (disconnect + begin) cada
  // intento se cortaba a la mitad: nunca conectaba y cada begin() disparaba un escaneo de ~0,9 s
  // que congelaba la animacion.
  #define WIFI_RETRY_INTERVAL_MS 20000
  #define WIFI_FIRST_CONNECT_MS 20000             // dentro de esta ventana es "conectando", despues "sin wifi"
#else
  #define WIFI_RETRY_INTERVAL_MS 5000
  #define WIFI_FIRST_CONNECT_MS 15000
#endif

// ============================================================================
// VARIABLES GLOBALES DE ESTADO
// ============================================================================
#ifdef ESP8266
ESP8266WebServer server(80);
#else
WebServer server(80);
#endif
OledBuf gfx;
eyes::Machine machine;

// Datos recibidos por POST /update
float lastPercent = -1;          // -1 = nunca llego dato valido
uint32_t lastResetEpoch = 0;
long lastTokensUsed = -1;
long lastTokensLimit = -1;
unsigned long lastUpdateMillis = 0; // millis() local del ultimo POST valido
bool everGotData = false;

// WiFi - maquina de estados no bloqueante
unsigned long lastWifiAttemptMillis = 0;
unsigned long wifiStartMillis = 0;
bool wifiWasConnected = false;
bool mdnsStarted = false;
bool serverStarted = false;

// Ojos: ultimo estado de sistema pedido a la maquina (ARRANCANDO = centinela "ninguno todavia")
eyes::Sys wantedSys = eyes::SYS_ARRANCANDO;

// Reloj de ticks: acumulador de millis(), nunca delay()
uint32_t lastLoopMs = 0;
uint32_t tickAcc = 0;

// Boton (ISR + antirrebote por tiempo)
volatile bool buttonPressed = false;
volatile uint32_t buttonIsrMs = 0;

// Cuidado del panel
unsigned long lastActivityMillis = 0;
bool dimmed = false;
uint8_t contrastNormal = CONTRAST_NORMAL;
uint8_t oledPrecharge = OLED_PRECHARGE;
uint8_t oledVcomh = OLED_VCOMH;
bool panelOn = true;               // false = DISPLAYOFF por horario nocturno
bool nightNow = false;             // ultimo resultado de isNightHour()
bool wakeActive = false;           // tap en horario nocturno: panel prendido hasta wakeUntilMs
unsigned long wakeUntilMs = 0;
unsigned long lastPanelCheckMs = 0;
unsigned long lastRefreshMs = 0;
uint16_t badSends = 0;
unsigned long lastRecoverMs = 0;
int localHour = -1;                // -1 = hora aun no sincronizada por NTP

// Rendimiento
struct Perf {
  uint32_t frames = 0, windowStart = 0;
  uint64_t renderUs = 0, sendUs = 0;
  uint32_t maxRenderUs = 0, maxSendUs = 0;
  float lastRenderMs = 0, lastSendMs = 0, lastFps = 0;
} perf;

// ============================================================================
// FORWARD DECLARATIONS
// ============================================================================
void connectWifiNonBlocking();
void setupWebServer();
void handleUpdate();
void handleEgg();
void handleRoot();
void handleStatus();
void handleTune();
void handlePanelReset();
void initPanel();
int i2cBusClear();
void refreshPanelRegs(bool full);
void applyPanelTuning();
void scanI2C();
void updateSystemState();
void pollButton();
void stepFrame(uint32_t now);
void updateDim();
void noteActivity();
void applyStyleInvert();
void updatePanelPower(bool force);
void wakePanel();
void onButton();

// ============================================================================
// SETUP
// ============================================================================
void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println(F("=== Ojos OLED - Iniciando ==="));

  // ---- Pantalla ----
  initPanel();

  Serial.println(F("[I2C] Escaneando bus..."));
  scanI2C();

  // ---- Boton ----
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), onButton, FALLING);

  // ---- Ojos: arranca con la animacion ARRANQUE ----
#ifdef ESP8266
  machine.begin(RANDOM_REG32 ^ micros(), 0);
#else
  machine.begin(esp_random() ^ micros(), 0);
#endif
  lastLoopMs = millis();
  lastActivityMillis = millis();

  // ---- WiFi ----
#ifdef ESP8266
  // Sin esto cada WiFi.begin() escribe SSID/clave en flash: bloquea cientos de ms (se vio como
  // caidas a ~11 fps en cada reintento) y desgasta la flash.
  WiFi.persistent(false);
#endif
  WiFi.mode(WIFI_STA);
  // Baja la potencia de transmision: el pico de corriente al asociarse es la
  // causa tipica de que la pantalla se apague a los segundos de bootear
  // (brownout con USB flojo). 8.5 dBm alcanza de sobra en red domestica.
#ifdef ESP8266
  WiFi.setOutputPower(8.5f);
#else
  WiFi.setTxPower(WIFI_POWER_8_5dBm);
#endif
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  lastWifiAttemptMillis = millis();
  wifiStartMillis = millis();

  Serial.print(F("[WiFi] Conectando a "));
  Serial.println(WIFI_SSID);
}

// ============================================================================
// LOOP PRINCIPAL - sin delay(), todo con millis()
// ============================================================================
void loop() {
  uint32_t now = millis();

  connectWifiNonBlocking();
  updateSystemState();
  if (serverStarted) {
    server.handleClient();
#ifdef ESP8266
    MDNS.update(); // requerido en ESP8266
#endif
  }
  pollButton();
  updateDim();
  updatePanelPower(false);
  if (panelOn && now - lastRefreshMs >= PANEL_REFRESH_MS) { lastRefreshMs = now; refreshPanelRegs(false); }
  stepFrame(now);
}

// ============================================================================
// FRAME: ticks fijos de TICK_MS con acumulador. Si algo tarda, se recuperan
// ticks de logica (hasta 6) pero se dibuja una sola vez.
// ============================================================================
void stepFrame(uint32_t now) {
  tickAcc += now - lastLoopMs;
  lastLoopMs = now;
  if (tickAcc > 200) tickAcc = 200;
  if (tickAcc < eyes::TICK_MS) return;

  uint32_t t0 = micros();
  while (tickAcc >= eyes::TICK_MS) {
    tickAcc -= eyes::TICK_MS;
    machine.tick();                 // la logica sigue corriendo aunque el panel este apagado
  }
  if (!panelOn) return;             // horario nocturno: ni render ni I2C
  eyes::render(gfx, machine.pose(), STYLE, machine.clock());
  uint32_t t1 = micros();
  u8g2.sendBuffer();
  uint32_t t2 = micros();
  // Un sendBuffer() real tarda ~25-35 ms; si dura <8 ms el OLED no esta ACKeando (bus colgado):
  // tras ~2 s seguidos se recupera el bus y se reinicializa el panel (como mucho cada 5 s).
  if (t2 - t1 < 8000) {
    if (++badSends >= (2000 / eyes::TICK_MS) && now - lastRecoverMs > 5000) {
      lastRecoverMs = now; badSends = 0;
      Wire.beginTransmission(OLED_I2C_ADDR);
      int err = Wire.endTransmission();
      Serial.printf("[Panel] el OLED no responde (t=%lus, SDA=%d, err=%d): reinicializando panel\n",
                    now / 1000, digitalRead(SDA_PIN), err);
      initPanel();
      return;
    }
  } else badSends = 0;

  perf.frames++;
  perf.renderUs += t1 - t0;
  perf.sendUs += t2 - t1;
  if (t1 - t0 > perf.maxRenderUs) perf.maxRenderUs = t1 - t0;
  if (t2 - t1 > perf.maxSendUs) perf.maxSendUs = t2 - t1;

  uint32_t elapsed = now - perf.windowStart;
  if (perf.windowStart == 0) {
    perf.windowStart = now;
  } else if (elapsed >= 2000) {
    perf.lastFps = perf.frames * 1000.0f / elapsed;
    perf.lastRenderMs = perf.renderUs / 1000.0f / perf.frames;
    perf.lastSendMs = perf.sendUs / 1000.0f / perf.frames;
#if PERF_LOG_MS > 0
    static uint32_t lastLog = 0;
    if (now - lastLog >= PERF_LOG_MS) {
      lastLog = now;
      Serial.printf("[Perf] tick+render avg=%.2f ms max=%.2f ms | display() avg=%.2f ms max=%.2f ms | %.1f fps (tick %u ms) | heap libre %u\n",
                    perf.lastRenderMs, perf.maxRenderUs / 1000.0f, perf.lastSendMs, perf.maxSendUs / 1000.0f,
                    perf.lastFps, (unsigned)eyes::TICK_MS, (unsigned)ESP.getFreeHeap());
    }
#endif
    perf.frames = 0; perf.renderUs = 0; perf.sendUs = 0; perf.windowStart = now;
    perf.maxRenderUs = 0; perf.maxSendUs = 0;
  }
}

// ============================================================================
// BOTON: ISR marca el flanco (con antirrebote por tiempo), el loop reacciona
// ============================================================================
void ISR_ATTR onButton() {
  uint32_t t = millis();
  if (t - buttonIsrMs > BUTTON_DEBOUNCE_MS) {
    buttonIsrMs = t;
    buttonPressed = true;
  }
}

void pollButton() {
  if (!buttonPressed) return;
  buttonPressed = false;
  Serial.println(F("[Boton] tap"));
  machine.tap();
  noteActivity();
  wakePanel();
}

// De noche, una interaccion (tap o POST /egg) prende el panel NIGHT_WAKE_MS.
void wakePanel() {
  if (!nightNow) return;
  wakeActive = true;
  wakeUntilMs = millis() + NIGHT_WAKE_MS;
  updatePanelPower(true);
}

// ============================================================================
// CUIDADO DEL PANEL (burn-in): inversion por hardware, atenuado y horario nocturno
// ============================================================================
// Mancuerna se dibuja como siempre (encendido sobre negro) y el SSD1306 invierte el panel:
// 0xA7 = invertido, 0xA6 = normal. Se manda una sola vez, no por frame.
void applyStyleInvert() {
  // OJO: u8g2.sendF("c", ...) manda el comando sin abrir/cerrar la transferencia I2C y deja el bus
  // colgado (el escaneo posterior no ve el OLED). Hay que hacerlo con Start/End explicitos.
  u8x8_t* u = u8g2.getU8x8();
  u8x8_cad_StartTransfer(u);
  u8x8_cad_SendCmd(u, (STYLE == eyes::ST_MANCUERNA) ? 0xA7 : 0xA6);
  u8x8_cad_EndTransfer(u);
}

// Destraba el bus I2C. Si el ESP se reinicia mientras el OLED manda un ACK, el OLED queda sosteniendo
// SDA en 0 hasta recibir mas pulsos de SCL: Wire.begin() no lo arregla y el panel no responde nunca mas
// (endTransmission = 4, "linea ocupada"). Medido: 1 de 25 reinicios con frames en curso; 1 pulso lo libera.
// OJO: solo drenaje abierto (bajar = salida en 0, soltar = entrada). NUNCA digitalWrite(HIGH): en ESP8266
// deja GPO=1 en el pin y el driver I2C (que baja las lineas habilitando la salida, asumiendo GPO=0)
// ya no puede bajar nada: todas las sondas dan NACK (err=2) aunque el OLED este bien. Asi fallaba el
// recover anterior. En ESP32 el driver de IDF ya limpia el bus en Wire.begin().
int i2cBusClear() {
#ifdef ESP8266
  digitalWrite(SDA_PIN, LOW);          // GPO=0 en ambos pines (tambien lo que necesita el driver I2C)
  digitalWrite(SCL_PIN, LOW);
  pinMode(SDA_PIN, INPUT_PULLUP);
  pinMode(SCL_PIN, INPUT_PULLUP);
  delayMicroseconds(10);
  int n = 0;
  for (; n < 9 && !digitalRead(SDA_PIN); n++) {
    pinMode(SCL_PIN, OUTPUT); delayMicroseconds(10);        // SCL a 0
    pinMode(SCL_PIN, INPUT_PULLUP); delayMicroseconds(10);  // SCL suelto (sube por pull-up)
  }
  pinMode(SDA_PIN, OUTPUT); delayMicroseconds(10);          // STOP: SDA sube mientras SCL esta alto
  pinMode(SDA_PIN, INPUT_PULLUP); delayMicroseconds(10);
  return n;
#else
  return 0;
#endif
}

// Inicializa bus + U8g2 + estilo. Se usa al arrancar y para recuperar el panel en marcha.
void initPanel() {
  int sda = digitalRead(SDA_PIN);
  int pulses = i2cBusClear();
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.beginTransmission(OLED_I2C_ADDR);
  int err = Wire.endTransmission();
  // err: 0 OK, 2 NACK (no responde), 4 linea ocupada (SDA trabado)
  Serial.printf("[I2C] init: SDA en reposo=%d, pulsos de destrabe=%d, sonda 0x%02X err=%d\n", sda, pulses, OLED_I2C_ADDR, err);
  u8g2.begin();
  u8g2.setBusClock(I2C_CLOCK_HZ);
  u8g2.setI2CAddress(OLED_I2C_ADDR << 1);
  u8g2.clearBuffer();
  u8g2.sendBuffer();
  gfx.attach(u8g2.getBufferPtr());
  applyStyleInvert();
  applyPanelTuning();
}

// Reenvia la configuracion del panel. Si el SSD1306 se reinicia por una caida de tension (con ~95% del
// panel encendido la corriente es alta), queda con la pantalla apagada y la config perdida, pero sigue
// ACKeando el I2C: desde el ESP no se nota. Todos estos comandos son idempotentes (no parpadea).
// full = re-inicializacion completa de U8g2 (puede causar un parpadeo breve).
void refreshPanelRegs(bool full) {
  if (!panelOn) return;               // si esta apagado por horario nocturno, no se toca
  if (full) u8g2.initDisplay();
  u8x8_t* u = u8g2.getU8x8();
  u8x8_cad_StartTransfer(u);
  u8x8_cad_SendCmd(u, 0x8D); u8x8_cad_SendArg(u, 0x14);   // charge pump ON
  u8x8_cad_EndTransfer(u);
  u8g2.setPowerSave(0);                                   // DISPLAYON
  applyStyleInvert();
  applyPanelTuning();
}

// Contraste normal + precarga (0xD9) y VCOMH (0xDB): ajustes de la etapa de manejo del panel.
void applyPanelTuning() {
  u8x8_t* u = u8g2.getU8x8();
  u8x8_cad_StartTransfer(u);
  u8x8_cad_SendCmd(u, 0xD9); u8x8_cad_SendArg(u, oledPrecharge);
  u8x8_cad_SendCmd(u, 0xDB); u8x8_cad_SendArg(u, oledVcomh);
  u8x8_cad_EndTransfer(u);
  u8g2.setContrast(dimmed ? CONTRAST_DIM : contrastNormal);
}

bool isNightHour() {
  if (NIGHT_START_HOUR == NIGHT_END_HOUR) return false;
  time_t t = time(nullptr);
  if (t < 1700000000) { localHour = -1; return false; }   // NTP todavia no sincronizo
  struct tm tmv;
  localtime_r(&t, &tmv);
  localHour = tmv.tm_hour;
  int h = tmv.tm_hour;
  return NIGHT_START_HOUR < NIGHT_END_HOUR ? (h >= NIGHT_START_HOUR && h < NIGHT_END_HOUR)
                                           : (h >= NIGHT_START_HOUR || h < NIGHT_END_HOUR);
}

// Revisa la hora cada 1 s (o ya, con force) y prende/apaga el panel (DISPLAYOFF / DISPLAYON).
void updatePanelPower(bool force) {
  unsigned long now = millis();
  if (!force && now - lastPanelCheckMs < 1000) return;
  lastPanelCheckMs = now;
  nightNow = isNightHour();
  if (wakeActive && (long)(now - wakeUntilMs) >= 0) wakeActive = false;
  bool want = !nightNow || wakeActive;
  if (want != panelOn) {
    u8g2.setPowerSave(want ? 0 : 1);
    panelOn = want;
    if (want) {
      tickAcc = eyes::TICK_MS;      // dibujar de inmediato al volver
    }
    Serial.println(want ? F("[Panel] encendido") : F("[Panel] apagado (horario nocturno)"));
  }
}
void noteActivity() {
  lastActivityMillis = millis();
  if (dimmed) {
    u8g2.setContrast(contrastNormal);
    dimmed = false;
    Serial.println(F("[Panel] contraste normal"));
  }
}

void updateDim() {
  if (!dimmed && millis() - lastActivityMillis > DIM_AFTER_MS) {
    u8g2.setContrast(CONTRAST_DIM);
    dimmed = true;
    Serial.println(F("[Panel] sin cambios de cuota: contraste bajo"));
  }
}

// ============================================================================
// CONEXION WIFI - maquina de estados no bloqueante
// ============================================================================
void connectWifiNonBlocking() {
  unsigned long now = millis();

  if (WiFi.status() == WL_CONNECTED) {
    if (!wifiWasConnected) {
      // Recien nos conectamos (primera vez o reconexion)
      wifiWasConnected = true;
      Serial.print(F("[WiFi] Conectado. IP: "));
      Serial.println(WiFi.localIP());

      if (!mdnsStarted) {
        if (MDNS.begin(MDNS_HOST)) {
          MDNS.addService("http", "tcp", 80);
          mdnsStarted = true;
          Serial.print(F("[mDNS] Registrado como http://"));
          Serial.print(MDNS_HOST);
          Serial.println(F(".local"));
        } else {
          Serial.println(F("[mDNS] Error al iniciar mDNS"));
        }
        setupWebServer();
        server.begin();
        serverStarted = true;
        Serial.println(F("[HTTP] Servidor iniciado en puerto 80"));
      }

      // NTP
      configTime(0, 0, NTP_SERVER);
      setenv("TZ", TZ_ARGENTINA, 1);
      tzset();
      Serial.println(F("[NTP] Sincronizando hora..."));
    }
  } else {
    if (wifiWasConnected) {
      // Se cayo la conexion
      wifiWasConnected = false;
      Serial.println(F("[WiFi] Conexion perdida"));
    }
    // Reintentar cada WIFI_RETRY_INTERVAL_MS sin bloquear
    if (now - lastWifiAttemptMillis >= WIFI_RETRY_INTERVAL_MS) {
      lastWifiAttemptMillis = now;
      Serial.printf("[WiFi] Reintentando conexion (status=%d)...\n", (int)WiFi.status());
      WiFi.disconnect();
      WiFi.begin(WIFI_SSID, WIFI_PASS);
    }
  }
}

// ============================================================================
// ESTADO DEL SISTEMA -> maquina de ojos
// Prioridad (la maquina aplica ARRANCANDO por su cuenta, primero):
//   SIN_WIFI > CONECTANDO > SIN_DATOS > humor por % de cuota
// ============================================================================
void updateSystemState() {
  eyes::Sys s;
  if (WiFi.status() != WL_CONNECTED) {
    // Primera conexion (ventana de WIFI_FIRST_CONNECT_MS) vs caido despues de haber andado
    bool firstTry = !wifiWasConnected && !everGotData && millis() - wifiStartMillis < WIFI_FIRST_CONNECT_MS;
    s = firstTry ? eyes::SYS_CONECTANDO : eyes::SYS_SIN_WIFI;
  } else if (!everGotData || (millis() - lastUpdateMillis > NO_DATA_TIMEOUT_MS)) {
    s = eyes::SYS_SIN_DATOS;
  } else {
    s = eyes::SYS_NORMAL;
  }

  if (s != wantedSys) {
    wantedSys = s;
    Serial.print(F("[Estado] "));
    Serial.println((int)s);
    machine.setSystem(s);
    noteActivity();                 // un cambio de estado tambien restaura el contraste
  }
}

// ============================================================================
// SERVIDOR WEB
// ============================================================================
void setupWebServer() {
  server.on("/update", HTTP_POST, handleUpdate);
  server.on("/egg", HTTP_POST, handleEgg);
  server.on("/", HTTP_GET, handleRoot);
  server.on("/status", HTTP_GET, handleStatus);
  server.on("/tune", HTTP_GET, handleTune);
  server.on("/panelreset", HTTP_GET, handlePanelReset);
}

void handleUpdate() {
  if (!server.hasArg("plain")) {
    server.send(400, "application/json", "{\"ok\":false,\"error\":\"body vacio\"}");
    return;
  }

  String body = server.arg("plain");
  JsonDocument doc; // ArduinoJson v7
  DeserializationError err = deserializeJson(doc, body);

  if (err) {
    Serial.print(F("[HTTP] JSON invalido: "));
    Serial.println(err.c_str());
    server.send(400, "application/json", "{\"ok\":false,\"error\":\"json invalido\"}");
    return;
  }

  if (!doc["percent"].is<float>() || !doc["reset_epoch"].is<unsigned long>()) {
    server.send(400, "application/json", "{\"ok\":false,\"error\":\"faltan campos obligatorios (percent, reset_epoch)\"}");
    return;
  }

  float prevPercent = lastPercent;
  lastPercent = doc["percent"].as<float>();
  lastResetEpoch = doc["reset_epoch"].as<uint32_t>();

  if (doc["tokens_used"].is<long>()) lastTokensUsed = doc["tokens_used"].as<long>();
  if (doc["tokens_limit"].is<long>()) lastTokensLimit = doc["tokens_limit"].as<long>();

  lastUpdateMillis = millis();
  everGotData = true;

  // % entero hacia abajo: 100 solo si de verdad llego a 100
  machine.setPercent((int)lastPercent);
  if ((int)prevPercent != (int)lastPercent) noteActivity();

  Serial.print(F("[HTTP] POST /update -> percent="));
  Serial.print(lastPercent);
  Serial.print(F(" reset_epoch="));
  Serial.println(lastResetEpoch);

  server.send(200, "application/json", "{\"ok\":true}");
}

// POST /egg -> easter egg ANIM_TROMPO
void handleEgg() {
  machine.egg();
  noteActivity();
  wakePanel();
  Serial.println(F("[HTTP] POST /egg -> TROMPO"));
  server.send(200, "application/json", "{\"ok\":true}");
}

// GET /tune?contrast=0x7F&precharge=0xF1&vcomh=0x20  (decimal o hex; cualquiera es opcional)
// Sirve para probar valores en vivo. No se guardan: al reiniciar vuelven a los #define.
void handleTune() {
  if (server.hasArg("contrast")) contrastNormal = (uint8_t)strtol(server.arg("contrast").c_str(), nullptr, 0);
  if (server.hasArg("precharge")) oledPrecharge = (uint8_t)strtol(server.arg("precharge").c_str(), nullptr, 0);
  if (server.hasArg("vcomh")) oledVcomh = (uint8_t)strtol(server.arg("vcomh").c_str(), nullptr, 0);
  applyPanelTuning();
  char buf[96];
  snprintf(buf, sizeof buf, "{\"contrast\":%u,\"precharge\":%u,\"vcomh\":%u}", contrastNormal, oledPrecharge, oledVcomh);
  server.send(200, "application/json", buf);
}

// GET /panelreset: re-inicializacion completa del OLED (por si quedo apagado/corrupto).
void handlePanelReset() {
  refreshPanelRegs(true);
  server.send(200, "application/json", "{\"ok\":true}");
}

void handleRoot() {
  String html = "<html><head><meta charset='utf-8'><title>ESP32 Eyes</title></head><body>";
  html += "<h2>ESP32-C3 Eyes - Diagnostico</h2>";
  html += "<p>WiFi: " + String(WiFi.status() == WL_CONNECTED ? "conectado" : "desconectado") + "</p>";
  html += "<p>RSSI: " + String(WiFi.RSSI()) + " dBm</p>";
  html += "<p>Uptime: " + String(millis() / 1000) + " s</p>";
  html += "<p>Ultimo percent: " + (everGotData ? String(lastPercent) : String("--")) + "</p>";
  html += "<p>Ultimo update hace: " + (everGotData ? String((millis() - lastUpdateMillis) / 1000) + " s" : String("nunca")) + "</p>";
  html += "<p>Estado ojos: " + String((int)machine.sys()) + " / humor: " + String((int)machine.mood()) + "</p>";
  html += "<p>Render: " + String(perf.lastRenderMs, 2) + " ms, display(): " + String(perf.lastSendMs, 2) + " ms, " + String(perf.lastFps, 1) + " fps</p>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void handleStatus() {
  JsonDocument doc;
  doc["wifi_connected"] = (WiFi.status() == WL_CONNECTED);
  doc["rssi"] = WiFi.RSSI();
  doc["uptime_s"] = millis() / 1000;
  doc["ever_got_data"] = everGotData;
  doc["last_percent"] = everGotData ? lastPercent : -1;
  doc["seconds_since_update"] = everGotData ? (millis() - lastUpdateMillis) / 1000 : -1;
  doc["system_state"] = (int)machine.sys();
  doc["mood"] = (int)machine.mood();
  doc["tick_ms"] = (int)eyes::TICK_MS;
  doc["render_ms"] = perf.lastRenderMs;
  doc["display_ms"] = perf.lastSendMs;
  doc["fps"] = perf.lastFps;
  doc["dimmed"] = dimmed;
  doc["panel_on"] = panelOn;
  doc["night"] = nightNow;
  doc["local_hour"] = localHour;
  doc["style"] = (int)STYLE;
  doc["free_heap"] = ESP.getFreeHeap();

  String out;
  serializeJson(doc, out);
  server.send(200, "application/json", out);
}

// ============================================================================
// SCANNER I2C OPCIONAL (informativo, corre una vez en setup)
// ============================================================================
void scanI2C() {
  byte count = 0;
  byte lastErr = 0;
  for (byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    lastErr = Wire.endTransmission();
    if (lastErr == 0) {
      Serial.print(F("[I2C] Dispositivo encontrado en 0x"));
      if (addr < 16) Serial.print("0");
      Serial.println(addr, HEX);
      count++;
    }
  }
  if (count == 0) {
    Serial.printf("[I2C] No se encontraron dispositivos (ultimo err=%u: 2 = nadie responde, 4 = linea ocupada)\n", lastErr);
  }
}
