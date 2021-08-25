enum PagesConsts {
  mainP,
  addP,
  hourP,
  dayP,
  debugP
};

bool useBacklight = true, isTempOutdated, isFeelingTempOutdated, isTimePrinted;
int8_t lastCTemp, lastOutdoorTemp, lastCFeelingTemp, lastMinute, lastWeatherIcon;
int16_t lcdBrightness = 0;
uint32_t keysPressedTime = 0;

void tryChangeBrightness() { // смена яркости при необходимости
  if (isNowNight() and millis()-keysPressedTime > LCD_DELAY_BRIGHTNESS) useBacklight = false;
  else if (!isNowNight() or millis()-keysPressedTime <= LCD_DELAY_BRIGHTNESS) useBacklight = true;  

  if (useBacklight and lcdBrightness < LCD_MAX_BRIGHTNESS) { // повышение яркости при необходимости
    while (lcdBrightness < LCD_MAX_BRIGHTNESS) {
      if (++lcdBrightness == 255) digitalWrite(LCD_BACKLIGHT_PIN, HIGH);
      else analogWrite(LCD_BACKLIGHT_PIN, lcdBrightness);
      delayMicroseconds(LCD_SPEED_BRIGHTNESS);
    }
  }
  else if (!useBacklight and 0 < lcdBrightness) { // понижение яркости при необходимости с перехватом нажатий кнопок
    while (0 < lcdBrightness) {
      if (--lcdBrightness == 0) digitalWrite(LCD_BACKLIGHT_PIN, LOW);
      else analogWrite(LCD_BACKLIGHT_PIN, lcdBrightness);

      updateButtonVars(false);
      delayMicroseconds(LCD_SPEED_BRIGHTNESS);
      if (lButtonPressed or rButtonPressed) { // если кнопка нажата - просто передаем управление функции работы с кнопками
        checkButtons();
        break;
      }
    }
  }
}

void flip(int8_t to, int8_t fun, int8_t pageNumber = 0) { // изменение вкладки с анимацией смещения
  for (int8_t i = -2; i <= 0; i++) {
    int8_t modifiedI = (to == D_LEFT) ? i : -i;
    switch(fun) {
      case mainP: { drawMainPage(modifiedI); break; }
      case addP: { drawAdditionalPage(modifiedI); break; }
      case debugP: { drawDebugPage(modifiedI); break; }
      case hourP: { drawHourPage(modifiedI, pageNumber); break; }
      case dayP: { drawDayPage(modifiedI, pageNumber); break; }
    }
    delayMicroseconds((75 + i*15) * 1000);
  }
  switch(fun) {
    case mainP: { pageType = 0; pageNum = 0; break; }
    case addP: { pageType = 0; pageNum = 1; break; }
    case debugP: { pageType = 0; pageNum = 2; break; }
  }
}

void drawMainPage(int8_t posX) { // отрисовка главной вкладки
  // очистка места под печать
  clearArea(posX);

  // отрисовка текущей температуры (с датчика или с сервера)
  lcd.setCursor(posX, 0);
  if (isOutdoorSensorConnected()) printOutdoorTemp();
  else printCityTemp();
  lastCTemp = cTemp;
  lastOutdoorTemp = outdoorTemp;
  isTempOutdated = isWeatherOutdated();

  // отрисовка иконки дождя или снега
  lcd.setCursor(posX+15, 0);
  printPrecipitationIcon();

  // отрисовка ощущаемой температуры
  lcd.setCursor(posX, 1);
  printFeelingTemp();
  lastCFeelingTemp = cFeelingTemp;
  isFeelingTempOutdated = isTempOutdated;

  // пишем время или дату
  lcd.setCursor(posX+11, 1);
  printDataOrTime();
  isTimePrinted = isTimeShouldBeOnDisplay();
  lastMinute = minute();
}

void tryUpdateMainPage() { // проверка необходимости вывести изменение на главной вкладке
  if (outdoorTemp != lastOutdoorTemp) { // если произошло изменение температуры на датчике
    if (!isOutdoorSensorConnected()) { // если датчик был отключен
      isTempOutdated = isWeatherOutdated(); // проверяем актуальность имеющейся температуры
      if (isTempOutdated) updateMainTemp(2); // если неактуальна, пишем В городе + ??
      else updateMainTemp(3); // если актуальна, пишем В городе + температуру
    }
    else if (lastOutdoorTemp == -127) updateMainTemp(0); // если датчик был подключен - пишем За окном + температуру с датчика
    else updateMainTemp(1); // если нужно просто поменять значение температуры - обновляем температуру с датчика
  }
  else if (!isOutdoorSensorConnected()) { // если датчик отключен уже какое-то время
    if (isTempOutdated != isWeatherOutdated()) { // если теперь погода неактуальна или снова актуальна
      isTempOutdated = isWeatherOutdated();
      if (isTempOutdated) updateMainTemp(4); // если неактуальна, пишем ??
      else updateMainTemp(5); // если актуальна, пишем температуру вместо ??
    }
    else if (cTemp != lastCTemp) { // если изменилась (и актуальна)
      updateMainTemp(5); // обновляем температуру
    }
  }
  lastOutdoorTemp = outdoorTemp, lastCTemp = cTemp;

  if (isFeelingTempOutdated != isWeatherOutdated() or cFeelingTemp != lastCFeelingTemp) { // если актуальность ощущаемой температуры изменилась или изменилась сама температура
    isFeelingTempOutdated = isWeatherOutdated();
    if (isFeelingTempOutdated) updateFeelsTemp(1); // если неактуальна пишем ??
    else updateFeelsTemp(0); // если актуальна пишем температуру вместо ??
    lastCFeelingTemp = cFeelingTemp;
  }

  if (weatherIcon != lastWeatherIcon) { // отрисовка иконки дождя или снега (или их удаление)
    lcd.setCursor(15, 0);
    printPrecipitationIcon();
  }

  /*lcd.setCursor(13, 0); lcd.print(second()); // написание текущей секунды (только для дебага!!)
  if ((second()) < 10)  lcd.print(" ");*/
  
  if (isTimePrinted != isTimeShouldBeOnDisplay()) { // если нужно сменить дату на время (или наоборот)
    isTimePrinted = isTimeShouldBeOnDisplay();
    lcd.setCursor(11, 1);
    if (isTimePrinted) {
      updateClockAndDate(1); // пишем время
    }
    else {
      updateClockAndDate(2); // пишем дату
      if (millis()-keysPressedTime > LCD_DELAY_BRIGHTNESS) tryUpdateWeather(); // получение погоды, если необходимо
    }
    updateTemperature(); // получение температуры с датчиков
  }
  else if (isTimePrinted and lastMinute != minute()) { // если произошла смена минуты и написано неактуальное время
    lcd.setCursor(11, 1);
    updateClockAndDate(1);
    lastMinute = minute();
    if (millis()-keysPressedTime > LCD_DELAY_BRIGHTNESS) tryUpdateTime(); // получение времени, если необходимо
  }
}

void drawAdditionalPage(int8_t posX) { // отрисовка дополнительной вкладки
  clearArea(posX);

  // скорость ветра
  lcd.setCursor(posX, 0);
  printWindSpeed();

  // влажность
  int freeSymbols = 8 - getCharCount(cWindSpeed) - getCharCount(cHumidity); // вычисление пробелов для влажности
  if (units == U_RUS or cPressure > 999) freeSymbols--; // если давление в мм рт. ст. или давление в hPa и занимает 4 символа, то отнимаем свободную ячейку
  for (int i = 0; i < ceil(freeSymbols/2.0); i++) lcd.print(" "); // перемещение для влажности
  printHumidity(); // само написание влажности

  // давление
  if (units == U_RUS or cPressure > 1000) lcd.setCursor(posX+11, 0);
  else lcd.setCursor(posX+12, 0);
  printPressure();

  // написание погоды в помещении или в городе
  lcd.setCursor(posX, 1);
  if (isOutdoorSensorConnected() and !isIndoorSensorConnected()) printCityTemp();
  else printIndoorTemp();
}

void drawDebugPage(int8_t posX) { // отрисовка вкладки для отладки
  clearArea(posX);

  // верхняя строка:
  lcd.setCursor(posX, 0);
  if (isOutdoorSensorConnected() and isIndoorSensorConnected()) printCityTemp(); // пишем погоду в городе, если в других вкладках выводится погода с различных датчиков
  else printInitials(); // пишем инициалы разработчика, если все полученные данные итак есть в других вкладках

  // пишем версию прошивки
  lcd.setCursor(posX + 15 - getCharCount(FIRMWARE_VERSION), 0);
  printVer();

  // пишем время работы после включения
  lcd.setCursor(posX, 1);
  printUptime();
}

void drawHourPage(int8_t posX, int8_t h) { // отрисовка вкладки с информацией о часе
  clearArea(posX);

  // пишем час и день
  lcd.setCursor(posX, 0);
  printDayAndHour(lastFullWeatherUpdate+h*3600);

  // температура справа (реал/ощущ)
  int freeSymbols = 11 - getCharCount(hour(lastFullWeatherUpdate+h*3600)) - getCharCount(day(lastFullWeatherUpdate+h*3600)) - getCharCount(hTemp[h]) - getCharCount(hFeelingTemp[h]); // вычисление количества пробелов

  if (freeSymbols == 1) { // если занято почти максимальное количество клеток (возможен лишь один пробел): пишем обе температуры вплотную
    lcd.print(" "); // пишем пробел
    printTemp(hTemp[h]); // и реальную температуру
  }
  else { // если возможно несколько пробелов: пишем обе температуры через пробел
    for (int i = 0; i < freeSymbols-1; i++) lcd.print(" "); // перемещение на freeSymbols-1 символов
    printTemp(hTemp[h]); // пишем реальную температуру
    lcd.print(" "); // втыкаем 1 пробел
  }
  lcd.write("\x1F"); // пишем символ "примерно"
  printTemp(hFeelingTemp[h]); // пишем ощущаемую температуру

  // статус погоды
  lcd.setCursor(posX, 1); printStatus(hStatus[h]);
}

void drawDayPage(int8_t posX, int8_t d) { // отрисовка вкладки с информацией о дне
  clearArea(posX);

  // пишем день и месяц
  lcd.setCursor(posX, 0);
  printDayAndMonth(lastFullWeatherUpdate+d*86400);

  // температура справа (мин/макс)
  int8_t freeSymbols = posX + 13 - getCharCount(dMinTemp[d]) - getCharCount(dMaxTemp[d]); // вычисление символов от края экрана
  lcd.setCursor(freeSymbols, 0); printTemp(dMinTemp[d]); lcd.print("/"); printTemp(dMaxTemp[d]); // пишем температуру

  // статус погоды
  lcd.setCursor(posX, 1); printStatus(dStatus[d]);
}

void drawSettingPage(int8_t n) { // отрисовка информации для настройки
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(getSettingLine(n-1));
  lcd.setCursor(0, 1); lcd.print(getSettingLine(n));
}

void setupBrightness() {
  int br = 200;
  while(true) {
    lcd.clear();

    updateButtonVars(true);
    if (lButtonPressed) br -= 5;
    if (rButtonPressed) br += 5;

    if (br < 0) br = 0;
    if (br > 255) br = 255;

    analogWrite(LCD_BACKLIGHT_PIN, br);
    lcd.print(br);
    delay(150);
  }
}
