uint8_t getCharCount(int num) { // вывод количества символов, из которых состоит число
  if (0 <= num and num <= 9) return 1;
  else if ((10 <= num and num <= 99) or (-9 <= num and num <= -1)) return 2;
  else if ((100 <= num and num <= 999) or (-99 <= num and num <= -10)) return 3;
  else if ((1000 <= num and num <= 9999) or (-999 <= num and num <= -100)) return 4;
  else return 0;
}

bool isTimeShouldBeOnDisplay() {
  return (((second()+7)/5)%2);
}

void clearArea(int8_t posX) { // очистка участка под смещаемые символы
  lcd.setCursor(posX, 0); lcd.print("                ");
  lcd.setCursor(posX, 1); lcd.print("                ");
}

void printTemp(int8_t tem) {
  lcd.print(tem);
  lcd.write(byte(0));
}

void printOutdoorTemp() {
  lcd.print("За окном ");
  printTemp(outdoorTemp);
}

void printCityTemp() {
  lcd.print("В городе ");
  if (isWeatherOutdated()) lcd.print("??");
  else printTemp(cTemp);
}

void printFeelingTemp() {
  lcd.print("Ощущ");
  lcd.write(byte(5));
  lcd.print(" ");
  if (isWeatherOutdated()) lcd.print("??");
  else printTemp(cFeelingTemp);
}

void printIndoorTemp() {
  lcd.print("В помещении ");
  if (!isIndoorSensorConnected()) lcd.print("??");
  else printTemp(indoorTemp);
}

void printPrecipitationIcon() {
  if (weatherIcon != 0) lcd.write(byte(weatherIcon));
  else lcd.print(" ");

  lastWeatherIcon = weatherIcon;
}

void printDataOrTime() {
  if (isTimeShouldBeOnDisplay()) updateClockAndDate(1); // пишем время
  else updateClockAndDate(2); // пишем дату
}

void printWindSpeed() {
  lcd.print(cWindSpeed); 
  if (units == U_IMPERIAL) lcd.print("м/ч");
  else lcd.print("м/с");
}

void printHumidity() {
  lcd.print(cHumidity);
  lcd.print("%");
}

void printPressure() {
  lcd.print(cPressure); 
  lcd.write(byte(6));
  if (units == U_RUS) lcd.write(byte(7));
}

void printInitials() {
  lcd.print("W.s. by mxkmn");
}

void printVer() {
  lcd.print("v");
  lcd.print(FIRMWARE_VERSION);
}

void printUptime() {
  lcd.print("Uptime ");
  lcd.print(millis()/86400000); lcd.print("d");
  lcd.print(millis()%86400000/3600000); lcd.print("h");
  lcd.print(millis()%3600000/60000); lcd.print("m");
}

void printDayAndHour(uint32_t t) {
  lcd.print(hour(t)); lcd.write(byte(3));
  lcd.print(day(t)); lcd.write(byte(4));
}

void printDayAndMonth(uint32_t t) {
  lcd.print(day(t)); lcd.print(".");
  lcd.print(month(t));
}

void printStatus(uint16_t st) {
  lcd.print(owm.getStatus16(st));
}

String getSettingLine(int8_t i) {
  switch (i) {
    // отрисовка информации для настройки
    case 0: return("Для настройки");
    case 1: return("подключитесь к");
    case 2: return("точке доступа");
    case 3: return("\"Weather stati-");
    case 4: return("on by mxkmn\"");
    case 5: return("После введите");
    case 6: return(WiFi.softAPIP().toString());
    case 7: return("в браузере.");

    // оповещение об очистке пользовательского ROM
    case 8: return("Очистка");
    case 9: return("хранилища");
    default: return("");
  }
}





void drawWifiStatus(int8_t n) { // отрисовка данных о WiFi
  if (n == 0) {
    lcd.clear();
    lcd.setCursor(0, 0); lcd.print("Подключение WiFi");
  }
  else if (n == 1) {    
    lcd.clear();
    lcd.setCursor(0, 0); lcd.print("Подключен к WiFi");
  }
  else if (n == 2) {
    lcd.clear();
    lcd.setCursor(0, 0); lcd.print("WiFi недоступен!");
  }
  else if (n == 3) {    
    lcd.setCursor(0, 1); lcd.write(218); lcd.print("заж. для настр"); lcd.write(218);
  }
  else if (n == 4) {
    lcd.setCursor(0, 1); lcd.print(" заж. для настр.");
  }
  else if (n == 5) {
    lcd.setCursor(0, 1); lcd.print("Получение данных");
  }
}

void updateMainTemp(uint8_t state) { // отрисовка основной температуры при изменении
  if (state == 0) { // если нужно написать За окном
    lcd.setCursor(0, 0); lcd.print("За окном"); // пишем За окном
  }
  if (state == 0 or state == 1) { // если нужно написать температуру с датчика
    lcd.setCursor(9, 0); lcd.print("    "); // стираем старое значение
    lcd.setCursor(9, 0); printTemp(outdoorTemp);
  }
  if (state == 2 or state == 3) { // если нужно написать В городе
    lcd.setCursor(0, 0); lcd.print("В городе"); // пишем В городе
  }
  if (state == 2 or state == 4) { // если нужно написать ??
    lcd.setCursor(9, 0); lcd.print("    "); // стираем старое значение
    lcd.setCursor(9, 0); lcd.print("??");
  }
  if (state == 3 or state == 5) { // если нужно написать температуру из интернета
    lcd.setCursor(9, 0); lcd.print("    "); // стираем старое значение
    lcd.setCursor(9, 0); printTemp(cTemp);
  }
}

void updateClockAndDate(uint8_t state) { // если на дисплее изменилась ощущаемая температура
  if (state == 1) { // если нужно написать время
    if (hour() < 10) lcd.print("0"); // если час занимает 1 символ - добавляем 0
    lcd.print(hour());
    lcd.print(":");
    if (minute() < 10) lcd.print("0"); // если минута занимает 1 символ - добавляем 0
    lcd.print(minute()); 
  }
  if (state == 2) { // если нужно написать дату
    if (day() < 10) lcd.print("0"); // если день занимает 1 символ - добавляем еще один пробел
    lcd.print(day());
    lcd.print(".");
    if (month() < 10) lcd.print("0"); // если месяц занимает 1 символ - пишем 0
    lcd.print(month());
  }
}

void updateFeelsTemp(uint8_t state) { // отрисовка ощущаемой температуры при изменении
  lcd.setCursor(6, 1); lcd.print("    "); // стираем старое значение
  lcd.setCursor(6, 1);
  if (state == 0) printTemp(cFeelingTemp); // если нужно написать температуру
  else if (state == 1) lcd.print("??"); // если нужно написать ??
}

void printUpdatingIcon(bool state) {
  if (pageType == 0) {
    lcd.setCursor(15, 0);

    if (state) lcd.write(byte(3)); // если передано true - установка иконки часов
    else printPrecipitationIcon(); // если передано false - установка прошлой иконки
  }
}
