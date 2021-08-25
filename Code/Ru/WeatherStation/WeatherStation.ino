// Используйте последнюю версию ESP8266 core. Код полностью отлажен на версии 3.0.2, при несовместимостя установите эту версию.
// Использование версий ESP8266 core ниже 3.0.0 гарантированно приведёт к проблемам яркости дисплея в связи с изменениями в реализации функций.
// Подробнее здесь: https://arduino-esp8266.readthedocs.io/en/latest/reference.html#analog-output

// Настройте "User_Setup.h" (libraries/OWM_for_ESP/src/User_Setup.h): отключите ENABLE_STRINGS (необязательно) и установите для LANGUAGE значение RU

// ======== Конфигурация ========
const uint8_t LCD_MAX_BRIGHTNESS = 255; // яркость подсветки (число от 0 до 255, можно узнать идеал, активировав функцию setupBrightness() в
                                        // void setup(). Рекомендуется устанавливать яркость 255 для устранения мерцания во время получения данных)
const uint16_t LCD_DELAY_BRIGHTNESS = 5000; // время подсветки ночью после нажатия кнопки (в миллисекундах)
const uint16_t FLIP_TO_MAIN_PAGE_DELAY = 15000; // время ожидания до перехода на главную вкладку при бездействии (в миллисекундах)
const uint16_t LCD_SPEED_BRIGHTNESS = 650; // скорость включения и выключения подсветки (в микросекундах, чем меньше значение, тем быстрее изменение яркости на 1)

const uint16_t CHECK_WEATHER_DELAY = 122, CHECK_TIME_DELAY = 8640; // погода обновляется 664 раза в сутки (86400/130), время 10 раз в сутки (86400/8640)

const char TIME_SERVER[] = "time.nist.gov"; // NTP сервер времени
const bool USE_ACCURATE_TIME = true; // наиболее точное время с увеличением времени получения (не более 1 секунды в случае отсутствия ошибок соединения)

const int LEFT_BUTTON_PIN = D2, RIGHT_BUTTON_PIN = A0, ONE_WIRE_BUS = D1; // пины для кнопок и датчиков на шине 1-Wire (DS18B20)
const int LCD_PIN4 = D4, LCD_PIN6 = D3, LCD_PIN11 = D8, LCD_PIN12 = D7, LCD_PIN13 = D6, LCD_PIN14 = D5, LCD_BACKLIGHT_PIN = D0; // пины для дисплея
// ===== Конец конфигурации =====



const uint8_t FIRMWARE_VERSION = 2; // версия прошивки
const char* AP_NAME = "Weather station by mxkmn"; // название точки доступа (если Вы измените название, отображение на дисплее не изменится)

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
#include <OWM_for_ESP.h> // https://github.com/mxkmn/OWM_for_ESP v2
OWM_Weather owm;

#include <TimeLib.h> // https://github.com/PaulStoffregen/Time

#include <OneWire.h> // https://github.com/PaulStoffregen/OneWire
#include <DallasTemperature.h> // https://github.com/milesburton/Arduino-Temperature-Control-Library
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

#include <LiquidCrystalRus.h> // https://github.com/mxkmn/LiquidCrystalRus
LiquidCrystalRus lcd(LCD_PIN4, LCD_PIN6, LCD_PIN11, LCD_PIN12, LCD_PIN13, LCD_PIN14);

#include "FS.h"

// задаём константы для удобства
enum UnitsConsts {
  U_METRIC,
  U_RUS,
  U_IMPERIAL
};
enum DirectionsConsts {
  D_LEFT,
  D_RIGHT
};

// установка глобальных переменных, которые используются в функциях, созданных до "тематической" вкладки
bool isDebugPageActivated, isTempSensorsReversed, isSingleSensorOutdoor, useMainLogging, useWifiLogging, canSetupAtPowerOn, lButtonPressed, rButtonPressed;
int8_t units, weatherIconsCheckingCounters, pageType = -10, pageNum = 0;

void printPage() { // вывод номера вкладки
  if (useMainLogging) Serial.println("Произошёл переход на вкладку " + String(pageType) + " - " + String(pageNum));
}
