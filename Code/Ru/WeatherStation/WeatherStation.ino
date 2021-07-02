// ======== Конфигурация ========
const uint16_t LCD_MAX_BRIGHTNESS = 800; // яркость подсветки (от 0 до 1023 единиц)
const uint8_t LCD_DELAY_BRIGHTNESS = 5; // время подсветки ночью после нажатия кнопки (в секундах)
const uint16_t LCD_SPEED_BRIGHTNESS = 650; // скорость включения и выключения подсветки (в микросекундах, чем меньше значение, тем быстрее изменение яркости на 1)

const uint16_t DELAY_CHECK_WEATHER = 122, DELAY_CHECK_TIME = 8640; // погода обновляется 664 раза в сутки (86400/130), время 10 раз в сутки (86400/8640)

const char TIME_SERVER[] = "time.nist.gov"; // NTP сервер времени
const bool USE_ACCURATE_TIME = true; // наиболее точное время с увеличением времени получения (не более 1 секунды в случае отсутствия ошибок соединения)

const int LEFT_BUTTON_PIN = D2, RIGHT_BUTTON_PIN = A0, ONE_WIRE_BUS = D1; // пины для кнопок и датчиков на шине 1-Wire (DS18B20)
const int LCD_LOGICPIN4_PIN = D4, LCD_LOGICPIN6_PIN = D3, LCD_LOGICPIN11_PIN = D8, LCD_LOGICPIN12_PIN = D7, LCD_LOGICPIN13_PIN = D6, LCD_LOGICPIN14_PIN = D5, LCD_BACKLIGHT_PIN = D0; // пины для дисплея
// ===== Конец конфигурации =====



const uint8_t FIRMWARE_VERSION = 1; // версия прошивки
const char* AP_NAME = "Weather station by mxkmn"; // название точки доступа (если вы измените его - отображение на дисплее не изменится)

// кастомные символы для дисплея. Рисовать можно тут: https://maxpromer.github.io/LCD-Character-Creator
byte degSymbol[8] = { B01000, B10100, B01000 };
byte rainSymbol[8] = { B00010, B00110, B01110, B11011, B10001, B11011, B01110 };
byte snowSymbol[8] = { B10101, B01110, B11011, B01110, B10101 };
byte clockSymbol[8] = { B01110, B10101, B10111, B10001, B01110 };
byte daySymbol[8] = { B11111, B10001, B10111, B10110, B11100 }; // страница календаря
byte threeDotsSymbol[8] = { B00000, B00011, B00011, B00000, B00011, B00011, B10000 }; // символ после "Ощущ"
byte mmrtst1Symbol[8] = { B01010, B10101, B10101, B00000, B11000, B10100, B11000, B10010 }; // м|р.
byte mmrtst2Symbol[8] = { B01010, B10101, B10101, B00000, B01100, B10000, B10000, B01101 }; // м|с.
byte barSymbol[8] = { B01110, B10011, B10101, B10001, B01010 }; // кружок барометра



#include <ESP8266WiFi.h>

#include <ESP8266WebServer.h>
ESP8266WebServer server;

#include <WiFiUdp.h>
WiFiUDP Udp;

#include <JSON_Decoder.h> // https://github.com/Bodmer/JSON_Decoder
#include <OWM_for_ESP.h> // https://github.com/mxkmn/OWM_for_ESP
OWM_Weather owm;

#include <TimeLib.h> // https://github.com/PaulStoffregen/Time

#include <OneWire.h> // https://github.com/PaulStoffregen/OneWire
#include <DallasTemperature.h> // https://github.com/milesburton/Arduino-Temperature-Control-Library
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

#include <LiquidCrystalRus.h> // https://github.com/mxkmn/LiquidCrystalRus
LiquidCrystalRus lcd(LCD_LOGICPIN4_PIN, LCD_LOGICPIN6_PIN, LCD_LOGICPIN11_PIN, LCD_LOGICPIN12_PIN, LCD_LOGICPIN13_PIN, LCD_LOGICPIN14_PIN);

#include "FS.h"

String apiKey;
bool activateDebugPage, tempSensorsReversed, oneSensorIsOutdoor, mainLogging, wifiLogging, setupAtPowerOn;
int8_t units, weatherIconsCheckingCounters;
float lat, lon;

bool lButtonPressed, rButtonPressed;

const int8_t METRIC = 0, RUS = 1, IMPERIAL = 2;
int8_t pageType = -10, pageNum = 0;

uint32_t getSecondsInDay(time_t d) { return (hour(d)*3600+minute(d)*60+second(d)); }
