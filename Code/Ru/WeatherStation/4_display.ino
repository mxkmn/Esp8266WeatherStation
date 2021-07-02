const int8_t mainP = 0, addP = 1, hourP = 2, dayP = 3, debugP = 4;

bool backlightTurnedOn = true; time_t keysPressed = 0; int nowBrightness = 0;

void tryChangeBrightness() {
  if (isNowNight() and now()-keysPressed>LCD_DELAY_BRIGHTNESS) backlightTurnedOn = false;
  else if (!isNowNight() or now()-keysPressed<=LCD_DELAY_BRIGHTNESS) backlightTurnedOn = true;  

  if (backlightTurnedOn and nowBrightness<LCD_MAX_BRIGHTNESS) {
    while (nowBrightness<LCD_MAX_BRIGHTNESS) {
      analogWrite(LCD_BACKLIGHT_PIN, ++nowBrightness);
      delayMicroseconds(LCD_SPEED_BRIGHTNESS);
    }
  }
  else if (!backlightTurnedOn and 0<nowBrightness) {
    while (0<nowBrightness) {
      analogWrite(LCD_BACKLIGHT_PIN, --nowBrightness);
      lButtonPressed = digitalRead(LEFT_BUTTON_PIN); rButtonPressed = analogRead(RIGHT_BUTTON_PIN)>512;
      delayMicroseconds(LCD_SPEED_BRIGHTNESS);
      if (lButtonPressed or rButtonPressed) { checkButtons(); break; } // если кнопка нажата - просто передаем управление функции работы с кнопками
    }
  }
}

void clearArea(int8_t posX) { // очистка участка под смещаемые символы
  lcd.setCursor(posX, 0); lcd.print("                ");
  lcd.setCursor(posX, 1); lcd.print("                ");
}
int nowTemperatureW, outdoorTempW, nowFeelsW, weatherIconW, lastMinute;
bool tempOutdated, tempFeelsOutdated, timePrinted;

void drawMainPage(int8_t posX) {
  clearArea(posX);
  if (outdoorTemp == -127) {
    lcd.setCursor(posX, 0); lcd.print("В городе ");
    if (now()-weatherUpdated>DELAY_CHECK_WEATHER*2) { lcd.print("??"); }
    else { lcd.print(nowTemperature); lcd.write(byte(0)); }
  }
  else {
    lcd.setCursor(posX, 0); lcd.print("За окном "); lcd.print(outdoorTemp); lcd.write(byte(0));
  }

  if (weatherIcon != 0) { lcd.setCursor(posX+15, 0); lcd.write(byte(weatherIcon)); } // отрисовка иконки дождя или снега

  lcd.setCursor(posX, 1); lcd.print("Ощущ"); lcd.write(byte(5)); lcd.print(" ");
  if (now()-weatherUpdated>DELAY_CHECK_WEATHER*2) { lcd.print("??"); }
  else { lcd.print(nowFeels); lcd.write(byte(0)); }

  timePrinted = ((second()+7)/5)%2;
  lcd.setCursor(posX+11, 1);
  if (timePrinted) updateClockAndDate(1); // пишем время
  else updateClockAndDate(2); // пишем дату

  nowTemperatureW = nowTemperature; outdoorTempW = outdoorTemp; nowFeelsW = nowFeels; weatherIconW = weatherIcon;
  tempOutdated = now()-weatherUpdated>DELAY_CHECK_WEATHER*2; tempFeelsOutdated = tempOutdated;
  lastMinute = minute();
}

void tryUpdateMainPage() { // код проверки необходимости изменений на дисплее
    if (outdoorTemp != outdoorTempW) { // если произошло изменение температуры на датчике
      if (outdoorTemp == -127) { // если датчик был отключен
        tempOutdated = now()-weatherUpdated>DELAY_CHECK_WEATHER*2; // проверяем актуальность имеющейся температуры
        if (tempOutdated) { // если неактуальна
          updateMainTemp(2); // пишем В городе + ??
        }
        else{ // если актуальна
          updateMainTemp(3); // пишем В городе + температуру
        }
      }
      else{ // если нужно просто поменять значение температуры
        if (outdoorTempW == -127) updateMainTemp(0); // пишем За окном + температуру с датчика
        else updateMainTemp(1); // обновляем температуру с датчика
      }
    }
    else if (outdoorTemp == -127) { // если датчик отключен уже какое-то время
      if (tempOutdated != now()-weatherUpdated>DELAY_CHECK_WEATHER*2) { // если теперь погода неактуальна или снова актуальна
        tempOutdated = now()-weatherUpdated>DELAY_CHECK_WEATHER*2;
        if (tempOutdated) { // если неактуальна
          updateMainTemp(4); // пишем ??
        }
        else{ // если актуальна
          updateMainTemp(5); // пишем температуру вместо ??
        }
      }
      else if (nowTemperature != nowTemperatureW) { // если актуальна и изменилась
        updateMainTemp(5); // обновляем температуру
      }
    }
    outdoorTempW = outdoorTemp;
    nowTemperatureW = nowTemperature;

    if (tempFeelsOutdated != now()-weatherUpdated>DELAY_CHECK_WEATHER*2 or nowFeels != nowFeelsW) { // если актуальность ощущаемой температуры изменилась или изменилась сама температура
      tempFeelsOutdated = now()-weatherUpdated>DELAY_CHECK_WEATHER*2;
      if (tempFeelsOutdated) updateFeelsTemp(1); // если неактуальна пишем ??
      else updateFeelsTemp(0); // если актуальна пишем температуру вместо ??
      nowFeelsW = nowFeels;
    }

    if (weatherIcon != weatherIconW) { // отрисовка иконки дождя или снега (или их удаление)
      updateWeatherIcon();
    }

    /*lcd.setCursor(13, 0); lcd.print(second()); // написание текущей секунды (только для дебага!!)
    if ((second()) < 10)  lcd.print(" ");*/
    
    if (timePrinted != ((second()+7)/5)%2) { // если нужно сменить дату на время (или наоборот)
      timePrinted = ((second()+7)/5)%2;
      lcd.setCursor(11, 1);
      if (timePrinted) {
        updateClockAndDate(1); // пишем время
      }
      else {
        updateClockAndDate(2); // пишем дату
        if (now()-keysPressed>LCD_DELAY_BRIGHTNESS) tryUpdateWeather(); // получение погоды
      }
      updateTemperature(); // получение температуры с датчиков
    }
    else if (timePrinted and lastMinute != minute()) { // если сейчас написано неактуальное время (произошла смена минуты)
      lcd.setCursor(11, 1);
      updateClockAndDate(1);
      lastMinute = minute();
      if (now()-keysPressed>LCD_DELAY_BRIGHTNESS) tryUpdateTime(); // получение времени
    }
}

void updateMainTemp(int state) { // если на дисплее изменилась основная температура
  if (state == 0) { // если нужно написать За окном
    lcd.setCursor(0, 0); lcd.print("За окном"); // пишем За окном
  }
  if (state == 0 or state == 1) { // если нужно написать температуру с датчика
    lcd.setCursor(9, 0); lcd.print("    "); // стираем старое значение
    lcd.setCursor(9, 0); lcd.print(outdoorTemp); lcd.write(byte(0));
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
    lcd.setCursor(9, 0); lcd.print(nowTemperature); lcd.write(byte(0));
  }
}
void updateFeelsTemp(int state) { // если на дисплее изменилась ощущаемая температура
  lcd.setCursor(6, 1); lcd.print("    "); // стираем старое значение
  lcd.setCursor(6, 1);
  if (state == 0) { lcd.print(nowFeels); lcd.write(byte(0)); } // если нужно написать температуру
  else if (state == 1) lcd.print("??"); // если нужно написать ??
}
void updateClockAndDate(int state) { // если на дисплее изменилась ощущаемая температура
  if (state == 1) { // если нужно написать время
    if (hour()<10) lcd.print("0"); // если час занимает 1 символ - добавляем 0
    lcd.print(hour());
    lcd.print(":");
    if (minute()<10) lcd.print("0"); // если минута занимает 1 символ - добавляем 0
    lcd.print(minute()); 
  }
  if (state == 2) { // если нужно написать дату
    if (day()<10) lcd.print("0"); // если день занимает 1 символ - добавляем еще один пробел
    lcd.print(day());
    lcd.print(".");
    if (month()<10) lcd.print("0"); // если месяц занимает 1 символ - пишем 0
    lcd.print(month());
  }
}
void updateWeatherIcon() {
  lcd.setCursor(15, 0);
  if (weatherIcon == 0) lcd.print(" ");
  else lcd.write(byte(weatherIcon));
  weatherIconW = weatherIcon;
}

void drawAdditionalPage(int8_t posX) {
  clearArea(posX);
  
  // скорость ветра
  lcd.setCursor(posX, 0); lcd.print(nowWindSpeed); 
  if (units == IMPERIAL) lcd.print("м/ч");
  else lcd.print("м/с");

  // вычисление пробелов для влажности
  int freeSymbols = 6;
  if (nowWindSpeed>9) freeSymbols--; // если скорость занимает 2 символа, то отнимаем свободную ячейку
  if (nowWindSpeed>99) freeSymbols--; // если скорость занимает 3 символа, то отнимаем свободную ячейку
  if (nowHumidity>9) freeSymbols--; // если влажность занимает 2 символа, то отнимаем свободную ячейку
  if (nowHumidity==100) freeSymbols--; // если влажность занимает 3 символа, то отнимаем свободную ячейку
  if (units==RUS or nowPressure>999) freeSymbols--; // если давление в мм рт. ст. или давление в hPa и занимает 4 символа, то отнимаем свободную ячейку

  for (int i = 0; i<ceil(freeSymbols/2.0); i++) lcd.print(" "); // перемещение
  lcd.print(nowHumidity); lcd.print("%"); // само написание влажности

  // давление
  if (units==RUS or nowPressure>1000) lcd.setCursor(posX+11, 0);
  else lcd.setCursor(posX+12, 0);
  lcd.print(nowPressure); 
  lcd.write(byte(6));
  if (units==RUS) lcd.write(byte(7));

  lcd.setCursor(posX, 1);
  if (indoorTemp == -127) {
    if (outdoorTemp == -127) {
      lcd.print("В помещении ??");
    }
    else { // если на главной выводится температура с внешнего датчика - пишем температуру с OWM
      lcd.print("В городе ");
      if (now()-weatherUpdated>DELAY_CHECK_WEATHER*2) { lcd.print("??"); }
      else { lcd.print(nowTemperature); lcd.write(byte(0)); }
    }
  }
  else {
    lcd.print("В помещении "); lcd.print(indoorTemp); lcd.write(byte(0));
  }
}
void drawDebugPage(int8_t posX) {
  clearArea(posX);

  lcd.setCursor(posX, 0); lcd.print("В городе "); lcd.print(nowTemperature); lcd.write(byte(0));

  lcd.setCursor(posX, 1); lcd.print("Uptime ");
  lcd.print(millis()/86400000); lcd.print("d");
  lcd.print(millis()%86400000/3600000); lcd.print("h");
  lcd.print(millis()%3600000/60000); lcd.print("m");
}
void drawHourPage(int8_t posX, int8_t h) {
  clearArea(posX);

  lcd.setCursor(posX, 0); lcd.print(hour(weatherChecked+h*3600)); lcd.write(byte(3)); lcd.print(day(weatherChecked+h*3600)); lcd.write(byte(4)); // пишем час и день

  // вычисление количества пробелов
  int freeSymbols = 7;
  if (hour(weatherChecked+h*3600)>9) freeSymbols--; // если час занимает 2 символа, то отнимаем свободную ячейку
  if (day(weatherChecked+h*3600)>9) freeSymbols--; // если день занимает 2 символа, то отнимаем свободную ячейку
  if (hourlyTemperature[h]>9 or -10<hourlyTemperature[h] and hourlyTemperature[h]<0) freeSymbols--; // если реальная температура занимает 2 символа, то отнимаем свободную ячейку
  else if (hourlyTemperature[h]<-9 or 99<hourlyTemperature[h]) freeSymbols-=2; // или если реальная температура занимает 3 символа, то отнимаем 2 свободных ячейки
  if (hourlyFeels[h]>9 or -10<hourlyFeels[h] and hourlyFeels[h]<0) freeSymbols--; // если чувствуемая температура занимает 2 символа, то отнимаем свободную ячейку
  else if (hourlyFeels[h]<-9 or 99<hourlyFeels[h]) freeSymbols-=2; // или если чувствуемая температура занимает 3 символа, то отнимаем 2 свободных ячейки
    
  if(freeSymbols==1) { // если занято почти максимальное количество клеток (возможен лишь один пробел)
    lcd.print(" "); lcd.print(hourlyTemperature[h]); lcd.write(byte(0)); // пишем пробел и реальную температуру
    lcd.write("\x1F"/*byte(4)*/); lcd.print(hourlyFeels[h]); lcd.write(byte(0)); // без пробела пишем ощущаемую температуру
  }
  else{ // если доступно более 1 символа
    for (int i = 0; i<freeSymbols-1; i++) lcd.print(" "); // перемещение на freeSymbols-1 символов
  
    lcd.print(hourlyTemperature[h]); lcd.write(byte(0)); // пишем реальную температуру
    lcd.print(" "); // втыкаем 1 пробел
    lcd.write("\x1F"/*byte(4)*/); lcd.print(hourlyFeels[h]); lcd.write(byte(0)); // пишем ощущаемую температуру
  }
  
  lcd.setCursor(posX, 1); lcd.print(getStatus(hourlyStatus[h]));
}
void drawDayPage(int8_t posX, int8_t d) {
  clearArea(posX);
  
  lcd.setCursor(posX, 0); lcd.print(day(weatherChecked+d*86400)); lcd.print("."); lcd.print(month(weatherChecked+d*86400)); // пишем день и месяц

  // вычисление символов от края экрана
  int someSymbols = 11;
  if (dailyTemperatureMin[d]>9 or -10<dailyTemperatureMin[d] and dailyTemperatureMin[d]<0) someSymbols--; // если минимальная температура занимает 2 символа, то отнимаем свободную ячейку
  else if (dailyTemperatureMin[d]<-9 or 99<dailyTemperatureMin[d]) someSymbols-=2; // или если минимальная температура занимает 3 символа, то отнимаем 2 свободных ячейки
  if (dailyTemperatureMax[d]>9 or -10<dailyTemperatureMax[d] and dailyTemperatureMax[d]<0) someSymbols--; // если максимальная температура занимает 2 символа, то отнимаем свободную ячейку
  else if (dailyTemperatureMax[d]<-9 or 99<dailyTemperatureMax[d]) someSymbols-=2; // или если максимальная температура занимает 3 символа, то отнимаем 2 свободных ячейки

  lcd.setCursor(posX+someSymbols, 0); lcd.print(dailyTemperatureMin[d]); lcd.write(byte(0)); lcd.print("/"); lcd.print(dailyTemperatureMax[d]); lcd.write(byte(0)); // пишем температуру справа

  lcd.setCursor(posX, 1); lcd.print(getStatus(dailyStatus[d]));
}

void flipLeft(int8_t fun, int8_t pageNumber = 0) {
  for (int i = -2; i<1; i++) {
    switch(fun) {
      case mainP: {drawMainPage(i); break;}
      case addP: {drawAdditionalPage(i); break;}
      case debugP: {drawDebugPage(i); break;}
      case hourP: {drawHourPage(i, pageNumber); break;}
      case dayP: {drawDayPage(i, pageNumber); break;}
    }
    delayMicroseconds(80*1000);
  }
}

void flipRight(int8_t fun, int8_t pageNumber = 0) {
  for (int i = 2; i>-1; i--) {
    switch(fun) {
      case mainP: {drawMainPage(i); break;}
      case addP: {drawAdditionalPage(i); break;}
      case debugP: {drawDebugPage(i); break;}
      case hourP: {drawHourPage(i, pageNumber); break;}
      case dayP: {drawDayPage(i, pageNumber); break;}
    }
    delayMicroseconds(80*1000);
  }
}

void drawSettingPage(int8_t n) {
  lcd.clear();
  if (n == 1) {
    lcd.setCursor(0, 0); lcd.print("Для настройки");
    lcd.setCursor(0, 1); lcd.print("подключитесь к");
  }
  else if (n == 2) {
    lcd.setCursor(0, 0); lcd.print("подключитесь к");
    lcd.setCursor(0, 1); lcd.print("точке доступа");
  }
  else if (n == 3) {
    lcd.setCursor(0, 0); lcd.print("точке доступа");
    lcd.setCursor(0, 1); lcd.print("\"Weather stati-");
  }
  else if (n == 4) {
    lcd.setCursor(0, 0); lcd.print("\"Weather stati-");
    lcd.setCursor(0, 1); lcd.print("on by mxkmn\"");
  }
  else if (n == 5) {
    lcd.setCursor(0, 0); lcd.print("on by mxkmn\"");
    lcd.setCursor(0, 1); lcd.print("После введите");
  }
  else if (n == 6) {
    lcd.setCursor(0, 0); lcd.print("После введите");
    lcd.setCursor(0, 1); lcd.print(WiFi.softAPIP());
  }
  else if (n == 7) {
    lcd.setCursor(0, 0); lcd.print(WiFi.softAPIP());
    lcd.setCursor(0, 1); lcd.print("в браузере.");
  }
}

void drawWifiStatus(int8_t n) {
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

void drawClearingPage() {
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("Очистка");
  lcd.setCursor(0, 1); lcd.print("хранилища");
}
