/*
  Скетч к проекту "Многофункциональный RGB светильник"
  Страница проекта (схемы, описания): https://alexgyver.ru/GyverLamp/
  Исходники на GitHub: https://github.com/AlexGyver/GyverLamp/
  Нравится, как написан код? Поддержи автора! https://alexgyver.ru/support_alex/
  Автор: AlexGyver, AlexGyver Technologies, 2019
  https://AlexGyver.ru/
*/

// ============= НАСТРОЙКИ =============
// -------- КНОПКА -------
#define USE_BUTTON 1    // 1 - использовать кнопку, 0 - нет

// ---------- МАТРИЦА ---------
#define BRIGHTNESS 40         // стандартная маскимальная яркость (0-255)
#define CURRENT_LIMIT 2000    // лимит по току в миллиамперах, автоматически управляет яркостью (пожалей свой блок питания!) 0 - выключить лимит

#define WIDTH 16              // ширина матрицы
#define HEIGHT 16             // высота матрицы

#define COLOR_ORDER GRB       // порядок цветов на ленте. Если цвет отображается некорректно - меняйте. Начать можно с RGB

#define MATRIX_TYPE 0         // тип матрицы: 0 - зигзаг, 1 - параллельная
#define CONNECTION_ANGLE 0    // угол подключения: 0 - левый нижний, 1 - левый верхний, 2 - правый верхний, 3 - правый нижний
#define STRIP_DIRECTION 1     // направление ленты из угла: 0 - вправо, 1 - вверх, 2 - влево, 3 - вниз
// при неправильной настройке матрицы вы получите предупреждение "Wrong matrix parameters! Set to default"
// шпаргалка по настройке матрицы здесь! https://alexgyver.ru/matrix_guide/

// --------- ESP --------
#define ESP_MODE 1
// 0 - точка доступа
// 1 - локальный
byte IP_AP[] = {192, 168, 4, 66};   // статический IP точки доступа (менять только последнюю цифру)

// ----- AP (точка доступа) -------
#define AP_SSID "GyverLamp"
#define AP_PASS "12345678"
#define AP_PORT 8888

// -------- Менеджер WiFi ---------
#define AC_SSID "AutoConnectAP"
#define AC_PASS "12345678"

// ============= ДЛЯ РАЗРАБОТЧИКОВ =============
#define LED_PIN 14             // пин ленты
#define BTN_PIN 16
#define MODE_AMOUNT 18

#define NUM_LEDS WIDTH * HEIGHT
#define SEGMENTS 1            // диодов в одном "пикселе" (для создания матрицы из кусков ленты)
// ---------------- БИБЛИОТЕКИ -----------------
#define FASTLED_INTERRUPT_RETRY_COUNT 0
#define FASTLED_ALLOW_INTERRUPTS 0
#define FASTLED_ESP8266_RAW_PIN_ORDER

#include <FastLED.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <WiFiUdp.h>
#include <EEPROM.h>
#include <GyverButton.h>
#include "fonts.h"
#include <ArduinoOTA.h>

// ------------------- ТИПЫ --------------------
CRGB leds[NUM_LEDS];
WiFiServer server(80);
WiFiUDP Udp;
GButton touch(BTN_PIN, LOW_PULL, NORM_OPEN);

// ----------------- ПЕРЕМЕННЫЕ ------------------
const char* autoConnectSSID = AC_SSID;
const char* autoConnectPass = AC_PASS;
const char AP_NameChar[] = AP_SSID;
const char WiFiPassword[] = AP_PASS;
unsigned int localPort = AP_PORT;
char packetBuffer[UDP_TX_PACKET_MAX_SIZE + 1]; //buffer to hold incoming packet
String inputBuffer;
static const byte maxDim = max(WIDTH, HEIGHT);
struct {
  byte speed = 30;
  byte scale = 40;
} modes[MODE_AMOUNT];

boolean sendSettings_flag = false;

int8_t currentMode = 0;
int8_t currentBrightness = BRIGHTNESS;

boolean loadingFlag = true;
boolean ONflag = true;
uint32_t eepromTimer;
boolean settChanged = false;
// Конфетти, Огонь, Радуга верт., Радуга гориз., Смена цвета,
// Безумие 3D, Облака 3D, Лава 3D, Плазма 3D, Радуга 3D,
// Павлин 3D, Зебра 3D, Лес 3D, Океан 3D,

unsigned char matrixValue[8][16];
String lampIP = "";

void setup() {
  ESP.wdtDisable();
  //ESP.wdtEnable(WDTO_8S);

  // ЛЕНТА
  FastLED.addLeds<WS2812B, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS)/*.setCorrection( TypicalLEDStrip )*/;
  FastLED.setBrightness(BRIGHTNESS);
  if (CURRENT_LIMIT > 0) FastLED.setMaxPowerInVoltsAndMilliamps(5, CURRENT_LIMIT);
  FastLED.show();

  touch.setStepTimeout(100);
  touch.setClickTimeout(500);

  Serial.begin(115200);
  Serial.println();
  delay(1000);

  // WI-FI
  if (ESP_MODE == 0) {    // режим точки доступа
    WiFi.softAPConfig(IPAddress(IP_AP[0], IP_AP[1], IP_AP[2], IP_AP[3]),
                      IPAddress(192, 168, 4, 1),
                      IPAddress(255, 255, 255, 0));

    WiFi.softAP(AP_NameChar, WiFiPassword);
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("Access point Mode");
    Serial.print("AP IP address: ");
    Serial.println(myIP);

    server.begin();
  } else {                // подключаемся к роутеру
    Serial.print("WiFi manager");
    WiFiManager wifiManager;
    wifiManager.setDebugOutput(false);

#if (USE_BUTTON == 1)
    if (digitalRead(BTN_PIN)) wifiManager.resetSettings();
#endif

    wifiManager.autoConnect(autoConnectSSID, autoConnectPass);
    /*WiFi.config(IPAddress(IP_STA[0], IP_STA[1], IP_STA[2], IP_STA[3]),
                IPAddress(192, 168, 1, 1),
                IPAddress(255, 255, 255, 0));*/
    Serial.print("\nConnected! IP address: ");
    Serial.println(WiFi.localIP());
    lampIP = WiFi.localIP().toString();
  }
  Serial.printf("UDP server on port %d\n", localPort);
  Udp.begin(localPort);

  // EEPROM
  initEEPROM();

  FastLED.setBrightness(currentBrightness);

  Serial.print("currentMode = ");
  Serial.println(String(currentMode));

  // отправляем настройки
  sendSettings();

  memset(matrixValue, 0, sizeof(matrixValue));

  randomSeed(micros());

  // настройка ота обновления
  ArduinoOTA.setHostname("ESPLamp");
  ArduinoOTA.setRebootOnSuccess(true);
  
  ArduinoOTA.onStart([]() {
    Serial.println("Start updating ");
    currentMode = 16;
    effectsTick();
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    effectsTick();
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
}

void loop() {
  ArduinoOTA.handle();
  parseUDP();
  effectsTick();
  eepromTick();
#if (USE_BUTTON == 1)
  buttonTick();
#endif
  ESP.wdtFeed();   // пнуть собаку
  yield();  // ещё раз пнуть собаку
}
