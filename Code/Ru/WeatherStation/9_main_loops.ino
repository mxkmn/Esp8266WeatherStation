void setup() {
  //setupBrightness(); // активируйте для подстройки яркости с помощью кнопок (узнайте с помощью этой функции лучшую яркость для Вашего дисплея)

  Serial.begin(74880);

  pinMode(LCD_BACKLIGHT_PIN, OUTPUT);
  lcd.begin(16, 2);

  SPIFFS.begin();
  checkMemory();

  lcd.createChar(0, degSymbol);
  lcd.createChar(1, rainSymbol);
  lcd.createChar(2, snowSymbol);
  lcd.createChar(3, clockSymbol);
  lcd.createChar(4, daySymbol);
  lcd.createChar(5, threeDotsSymbol);
  if (units == U_RUS) {
    lcd.createChar(6, mmrtst1Symbol);
    lcd.createChar(7, mmrtst2Symbol);
  }
  else {
    lcd.createChar(6, barSymbol);
  }

  setTime(86400); // ставим рандомную дату на случай, если получения погоды не произойдет (чтобы везде писалось ??)

  connectToWifi();
  SPIFFS.end(); // все данные из ФС получены, теперь ее можно отключать
  checkWiFiChanges();

  sensors.begin(); sensors.setResolution(12); // подключаем работу с датчиками температуры с максимальной точностью

  Udp.begin(8888); // включаем UDP для получения времени на порте 8888 (нужно для получения времени)

  drawWifiStatus(5); // уведомляем на дисплее о начале получения данных
  do { // получение времени
    tryUpdateTime();
  } while (now() < 1000000000); // предотвращаем неудачное получение времени

  if (useMainLogging) testServer(); // вывод скорости получения данных с сервера времени

  tryUpdateWeather(); // получение погоды

  requestTemperature(); updateTemperature(); // получение температуры с датчиков
  sensors.setWaitForConversion(false); // убираем задержку получения температуры
}

void loop() {
  if (pageType == 0 and pageNum == 0) { // если открыта основная вкладка
    tryUpdateMainPage(); // изменение главного экрана при необходимости + обновление данных
    if ((now()+7)/10 != lastTemperatureUpdate) requestTemperature(); // получение температуры с датчиков в фоне
  }
  else if (millis()-keysPressedTime > FLIP_TO_MAIN_PAGE_DELAY) {  // если вкладка не основная и кнопка не нажималась FLIP_TO_MAIN_PAGE_DELAY секунд
    flip(D_LEFT, mainP); // переход на главную вкладку
    printPage(); // логирование номера вкладки в консоль
  }
  tryChangeBrightness(); // смена яркости при необходимости
  checkButtons(); // проверка нажатия кнопок

  checkWiFiChanges(); // проверка изменения статуса WiFi
}
