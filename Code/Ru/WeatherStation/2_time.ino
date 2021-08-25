time_t lastTimeUpdate = 0;

void testServer() { // замер скорости NTP-сервера. По моим тестам самым быстрым оказался time.nist.gov
  uint32_t testingTimer = millis(); // сохранение времени начала замеров
  Serial.print("\nТестирование скорости NTP-сервера " + String(TIME_SERVER) + ":");
  if (getNtpTime() > 0) Serial.println("\nСервер имеет задержку " + String(millis()-testingTimer) + "мс.\n");
  else Serial.println("\nСервер не отвечает.\n");
}

bool isNowNight() { // сейчас ночь? (для включения/выключения подсветки)
  uint32_t nowSecs = getSecondsInDay(now());
  return (nowSecs < sunriseTime or sunsetTime < nowSecs);
}

uint32_t getSecondsInDay(time_t d) { // возврат секунд с начала дня
  return (d%86400);
}

// возврат смещения актуального часа/дня на основе сохранённых данных в массивы в данный момент
int getActualHour() {
  return (now()/3600 - lastFullWeatherUpdate/3600);
}
int getActualDay() {
  return (now()/86400 - lastFullWeatherUpdate/86400);
}

void tryUpdateTime() { // обновление времени, если его нужно обновить
  if ((now()-lastTimeUpdate > CHECK_TIME_DELAY or lastTimeUpdate == 0) and WiFi.status() == WL_CONNECTED) {
    printUpdatingIcon(true); // установка иконки часов

    time_t tempTime = getNtpTime();
    if (tempTime > 0) { // если время получено успешно
      if (USE_ACCURATE_TIME) { // если мы хотим получить время максимально точно
        time_t tempTime2 = tempTime; // создаём вторую переменную
        while (tempTime >= tempTime2) { // пока вторая переменная равна или даже меньше первой
          if (useMainLogging) Serial.print("Производим повторное получение времени для максимальной точности...");
          tempTime2 = getNtpTime(); // ждем изменения времени на секунду (для уменьшения задержки)
          if (tempTime2 == 0) { // если произошла ошибка получения времени
            while (tempTime2 == 0) tempTime2 = getNtpTime(); // ждём успешного получения времени
            tempTime = tempTime2; // и делаем tempTime равным tempTime2 на случай, если в момент ошибок прошла секунда или больше
          }
        }
        tempTime = tempTime2; // при изменении времени на секунду делаем tempTime актуальным
      }
      setTime(tempTime + timezoneOffset); // устанавливаем время
      lastTimeUpdate = now();
      if (useMainLogging) { Serial.print("Сейчас "); printTime(now()); Serial.println("."); }
    }

    printUpdatingIcon(false); // установка прошлой иконки
  }
}

void printTime(time_t t) { // вывод времени в формате 23:59:59 в консоль
  printTwoDigits(hour(t)); Serial.print(":"); printTwoDigits(minute(t)); Serial.print(":"); printTwoDigits(second(t));
}
void printTwoDigits(int digits) { // вывод числа с нулём, если число не двузначное
  if (digits < 10) Serial.print('0');
  Serial.print(digits);
}

// код получения времени с помощью NTP из Интернета (лежит на многих сайтах, поэтому ссылка на первоисточник мне неизвестна)
const uint8_t NTP_PACKET_SIZE = 48; // NTP-время содержится в первых 48 байтах сообщения
byte packetBuffer[NTP_PACKET_SIZE]; // буфер для хранения входящих и исходящих пакетов 
time_t getNtpTime() {
  IPAddress ntpServerIP; // создание переменной для хранения IP-адреса NTP-сервера
 
  while (Udp.parsePacket() > 0) ; // отбраковываем все пакеты, полученные ранее 
  if (useMainLogging) { Serial.print("\nПередача NTP-запроса на "); Serial.print(TIME_SERVER); Serial.print("... "); }
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
      if (useMainLogging) Serial.print("Время получено! ");
      return secsSince1900 - 2208988800UL;
    }
  }
  if (useMainLogging) Serial.print("Нет NTP-ответа, время не получено. ");
  return 0;
}

void sendNtpPacket(IPAddress &address) { // отправляем NTP-запрос серверу времени по указанному адресу
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
