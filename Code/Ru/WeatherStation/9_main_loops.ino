void setup() {
  Serial.begin(74880);

  pinMode(LCD_BACKLIGHT_PIN, OUTPUT);
  lcd.begin(16, 2);

  checkMemory();

  lcd.createChar(0, degSymbol);
  lcd.createChar(1, rainSymbol);
  lcd.createChar(2, snowSymbol);
  lcd.createChar(3, clockSymbol);
  lcd.createChar(4, daySymbol);
  lcd.createChar(5, threeDotsSymbol);
  if (units == RUS) {
    lcd.createChar(6, mmrtst1Symbol);
    lcd.createChar(7, mmrtst2Symbol);
  }
  else {
    lcd.createChar(6, barSymbol);
  }

  setTime(10000000); // ставим рандомную дату на случай, если получения погоды не произойдет (чтобы везде писалось ??)

  connectToWifi();
  SPIFFS.end(); // все данные из ФС получены, теперь ее можно отключать
  if (wifiLogging) checkWiFiChanges();

  sensors.begin(); sensors.setResolution(12); // подключаем работу с датчиками температуры с максимальной точностью

  Udp.begin(8888); // включаем UDP для получения времени на порте 8888 (нужно для получения времени)

  drawWifiStatus(5); // уведомляем на дисплее о начале получения данных
  do { // получение времени
    tryUpdateTime();
  } while (now()<1000000000); // предотвращаем неудачное получение времени, если //проблем с получением погоды не возникло//, а время почему-то застряло в 1970-ых
  tryUpdateWeather(); // получение погоды

  if (mainLogging) testServer(); // вывод скорости получения данных с сервера времени

  requestTemperature(); updateTemperature(); // получение температуры с датчиков
  sensors.setWaitForConversion(false); // убираем задержку получения температуры
}

void loop() {
  if (pageType == 0 and pageNum == 0) { // если открыта основная вкладка
    tryUpdateMainPage(); // изменение главного экрана при необходимости + обновление данных
    if ((now()+7)/10 != lastTemperatureRequest) requestTemperature(); // получение температуры с датчиков в фоне
  }
  else if (now()-keysPressed>LCD_DELAY_BRIGHTNESS+10) {  // если вкладка не основная и кнопка не нажималась LCD_DELAY_BRIGHTNESS+10 секунд
    flipLeft(mainP); pageType = 0; pageNum = 0;
    if (mainLogging) { Serial.print("pageType = "); Serial.print(pageType); Serial.print(", pageNum = "); Serial.print(pageNum); Serial.println("."); }
  }
  tryChangeBrightness(); // смена яркости при необходимости
  checkButtons(); // проверка нажатия кнопок

  if (wifiLogging) checkWiFiChanges(); // проверка изменения статуса WiFi
}
