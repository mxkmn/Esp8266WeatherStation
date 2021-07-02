void testServer() { // эта штука замеряет скорость получения данных с сервера, подключается в void setup(). По моим тестам самым быстрым сервером оказался time.nist.gov
  uint32_t testingTimer = millis(); // сохранение времени начала замеров
  if (getNtpTime()>0) { // подключение к серверу и проверка полученных данных
    Serial.print("\nСервер времени "); Serial.print(TIME_SERVER);
    Serial.print(" имеет задержку "); Serial.print(millis()-testingTimer); Serial.println("мс\n");
  }
}

bool isNowNight() { // сейчас ночь? (для включения/выключения подсветки)
  uint32_t nowSecs = getSecondsInDay(now());
  return (nowSecs<sunrise or sunset<nowSecs);
}

int getActualHour() { return (now()/3600 - weatherChecked/3600); }
int getActualDay() { return (now()/86400 - weatherChecked/86400); }


time_t timeUpdated = 0;
void tryUpdateTime() {
  if (now()-timeUpdated>DELAY_CHECK_TIME or timeUpdated==0) {
    if (pageType == 0) { lcd.setCursor(15, 0); lcd.write(byte(3)); }
    time_t tempTime = getNtpTime();
    if (tempTime>0) { // если время получено успешно
      if (USE_ACCURATE_TIME) { // если мы хотим получить время максимально точно
        time_t tempTime2 = tempTime; // создаём вторую переменную
        while (tempTime >= tempTime2) { // пока вторая переменная равна или даже меньше первой
          tempTime2 = getNtpTime(); // ждем изменения времени на секунду (для уменьшения задержки)
          if (tempTime2 == 0) { // если произошла ошибка получения времени
            while (tempTime2 == 0) tempTime2 = getNtpTime(); // ждём успешного получения времени
            tempTime = tempTime2; // и делаем tempTime равным tempTime2 на случай, если в момент ошибок прошла секунда или больше
          }
        }
        tempTime = tempTime2; // при изменении времени на секунду делаем tempTime актуальным
      }
      setTime(tempTime + timeZone); // устанавливаем время
      if (timeUpdated == 0 and weatherUpdated != 0) weatherUpdated = now(); // убираем попытку повторного получения погоды в первый раз
      timeUpdated = now();
    }
    if (mainLogging) { Serial.print("Сейчас "); printTime(now()); Serial.print(".\n\n"); }
    if (pageType == 0) { updateWeatherIcon(); } // изменение иконки часов
    // Serial.print("Free RAM on time: "); Serial.println(ESP.getFreeHeap());
  }
}

void printTime(time_t t) { printTwoDigits(hour(t)); Serial.print(":"); printTwoDigits(minute(t)); Serial.print(":"); printTwoDigits(second(t)); }
void printTwoDigits(int digits) { if(digits < 10) Serial.print('0'); Serial.print(digits); } // вывод числа с нулём, если число не двузначное

const uint8_t NTP_PACKET_SIZE = 48; // NTP-время содержится в первых 48 байтах сообщения
byte packetBuffer[NTP_PACKET_SIZE]; // буфер для хранения входящих и исходящих пакетов 
time_t getNtpTime() {
  IPAddress ntpServerIP; // создание переменной для хранения IP-адреса NTP-сервера
 
  while (Udp.parsePacket() > 0) ; // отбраковываем все пакеты, полученные ранее 
  if (mainLogging) { Serial.print("\nПередача NTP-запроса на "); Serial.print(TIME_SERVER); Serial.print("... "); }
  WiFi.hostByName(TIME_SERVER, ntpServerIP); // подключаемся к случайному серверу из списка
  sendNtpPacket(ntpServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // считываем пакет в буфер
      unsigned long secsSince1900;
      // конвертируем 4 байта (начиная с позиции 40) в длинное целое число: 
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      if (mainLogging) Serial.print("Время получено! ");
      return secsSince1900 - 2208988800UL;
    }
  }
  if (mainLogging) Serial.print("Нет NTP-ответа, время не получено. ");
  return 0;
}
void sendNtpPacket(IPAddress &address) { // отправляем NTP-запрос серверу времени по указанному адресу: 
  memset(packetBuffer, 0, NTP_PACKET_SIZE); // задаем все байты в буфере на «0»
  // инициализируем значения для создания NTP-запроса
  packetBuffer[0] = 0b11100011;   // LI (от «leap indicator», т.е. «индикатор перехода»), версия, режим работы 
  packetBuffer[1] = 0;     // слой (или тип часов) 
  packetBuffer[2] = 6;     // интервал запросов 
  packetBuffer[3] = 0xEC;  // точность 
  // 8 байтов с нулями, обозначающие базовую задержку и базовую дисперсию: 
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  // После заполнения всех указанных полей вы сможете отправлять пакет с запросом о временной метке:      
  Udp.beginPacket(address, 123); // NTP-запросы к порту 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}
