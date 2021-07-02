int lastWiFiSt = -1;
void checkWiFiChanges() {
  if (WiFi.status()!=lastWiFiSt) {
    lastWiFiSt = WiFi.status();
    Serial.print("Новый статус WiFI: ");
    switch (lastWiFiSt) {
      case WL_CONNECTED: { Serial.println("WL_CONNECTED"); return; }
      case WL_IDLE_STATUS: { Serial.println("WL_IDLE_STATUS"); return; }
      case WL_DISCONNECTED: { Serial.println("WL_DISCONNECTED"); return; }
      case WL_CONNECTION_LOST: { Serial.println("WL_CONNECTION_LOST"); return; }
      case WL_CONNECT_FAILED: { Serial.println("WL_CONNECT_FAILED"); return; }
      case WL_NO_SSID_AVAIL: { Serial.println("WL_NO_SSID_AVAIL"); return; }
      case WL_SCAN_COMPLETED: { Serial.println("WL_SCAN_COMPLETED"); return; }
      case WL_NO_SHIELD: { Serial.println("WL_NO_SHIELD"); return; }
      default: { Serial.println("Unknown"); return; }
    }
  }
}

void connectToWifi() {
  if (setupAtPowerOn) {
    lcd.setCursor(0, 0); lcd.print("Подкл. через  с.");
    for (int i = 3; i > 0; i--) {
      lcd.setCursor(13, 0); lcd.print(i); // пишем секунду
      for (int j = 0; j < 100; j++) {
        if (j == 0) drawWifiStatus(3);
        else if (j == 50) drawWifiStatus(4);

        // проверка нажатия кнопок:
        lButtonPressed = digitalRead(LEFT_BUTTON_PIN); rButtonPressed = analogRead(RIGHT_BUTTON_PIN)>512;
        yield(); delayMicroseconds(10000);
        if (lButtonPressed and rButtonPressed) setupAP(); // переходим к настройкам если кнопки нажаты
      }
    }
  }
  
  
  // преобразование String в char
  String ssid = readFile("/WiFi_SSID"), pass = readFile("/WiFi_pass");
  uint8_t wifiSSIDLength = ssid.length()+1, wifiPassLength = pass.length()+1;
  char wifiSSID[wifiSSIDLength], wifiPass[wifiPassLength];
  ssid.toCharArray(wifiSSID, wifiSSIDLength);
  pass.toCharArray(wifiPass, wifiPassLength);
  
  Serial.printf("\nПодключение к сети %s... ", wifiSSID); drawWifiStatus(0); // уведомляем о начале подключения
  WiFi.mode(WIFI_STA); WiFi.begin(wifiSSID, wifiPass); // инициируем подключение
  WiFi.waitForConnectResult(); // ожидаем подключения к сети
  
  
  if (WiFi.status()!=WL_CONNECTED) { // уведомляем о безуспешном подключении
    Serial.println("WiFi недоступен!"); drawWifiStatus(2);

    uint8_t timeCounter = 0;
    while (WiFi.status()!=WL_CONNECTED) {
      // назойливая надпись "настройка" внизу экрана:
      if (timeCounter == 0) drawWifiStatus(3);
      else if (timeCounter == 75) drawWifiStatus(4);

      // проверка нажатия кнопок:
      lButtonPressed = digitalRead(LEFT_BUTTON_PIN); rButtonPressed = analogRead(RIGHT_BUTTON_PIN)>512;
      yield(); delayMicroseconds(10000);
      if (lButtonPressed and rButtonPressed) setupAP(); // переходим к настройкам если кнопки нажаты
      
      timeCounter = (timeCounter+1)%150; // сбрасываем счетчик каждые 1.5 секунды
    }
  }
  if (WiFi.status()==WL_CONNECTED) { Serial.println("Подключено!"); drawWifiStatus(1); } // при подключенном WiFi уведомляем об успешном подключении
}

void setupAP() {
  analogWrite(LCD_BACKLIGHT_PIN, LCD_MAX_BRIGHTNESS); // включаем подсветку дисплея
  
  WiFi.mode(WIFI_AP); WiFi.softAP(AP_NAME); // создание точки доступа

  if (readFile("/WiFi_SSID") == "") Serial.print("Необходимо пройти первую настройку. Она");
  else Serial.print("Настройка");
  Serial.print(" производится через подключение к точке доступа "); Serial.print(AP_NAME);
  Serial.print(", после подключения введите в браузере "); Serial.println(WiFi.softAPIP());

  server.on("/", sendHtml);
  server.on("/senddata", getData);
  server.begin();

  uint16_t timeCounter = 0;
  while (true) {
    if (timeCounter%250 == 0) drawSettingPage(timeCounter/250 + 1);
    server.handleClient();
    delayMicroseconds(10000);
    timeCounter = (timeCounter+1)%1750;
  }
}

void sendHtml() {
  String out = FPSTR(settingsPage1);
  out += "wifi = '";
  out += readFile("/WiFi_SSID");
  if (readFile("/WiFi_SSID") == "") out += "';";
  else {
    out += "', lati = "; out += readFile("/latitude");
    out += ", long = "; out += readFile("/longitude");
    out += ", uni = "; out += readFile("/units");
    out += ", deb = "; out += readFile("/act_debug_p");
    out += ", tem = "; out += readFile("/temp_sens_rev");
    out += ", one = "; out += readFile("/one_sensor_is_outdoor");
    out += ", cou = "; out += readFile("/wea_icon_counts");
    out += ", mLog = "; out += readFile("/main_logging");
    out += ", wLog = "; out += readFile("/wifi_logging");
    out += ", set = "; out += readFile("/setup_at_power_on"); out += ";";
  }
  out += FPSTR(settingsPage2);
  out += readFile("/firm_vers");
  out += FPSTR(settingsPage3);
  server.send(200, "text/html", out);
}

void getData() {
  String data = server.arg("plain"); // получаем POST со смарт-устройства

  String val;
  for (int i = 0; i <= 12; i++) {
    val = getValue(getValue(data, '&', i), '=', 1);
    if (i == 0)                    writeFile("/WiFi_SSID", val);
    else if (i == 1 and val != "") writeFile("/WiFi_pass", val);
    else if (i == 2 and val != "") writeFile("/API_key", val);
    else if (i == 3 and val != "") writeFile("/latitude", val);
    else if (i == 4 and val != "") writeFile("/longitude", val);
    
    else if (i == 5) writeFile("/units", val);
    else if (i == 6) writeFile("/act_debug_p", val);
    else if (i == 7) writeFile("/temp_sens_rev", val);
    else if (i == 8) writeFile("/one_sensor_is_outdoor", val);
    else if (i == 9) writeFile("/wea_icon_counts", val);
    else if (i == 10) writeFile("/main_logging", val);
    else if (i == 11) writeFile("/wifi_logging", val);
    else if (i == 12) writeFile("/setup_at_power_on", val);
  }

  server.send(204, "");
  ESP.reset();
}
