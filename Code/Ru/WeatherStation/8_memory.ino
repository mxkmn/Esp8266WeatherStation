String readFile(String file) {
  File f = SPIFFS.open(file, "r");
  if (!f) { // если при открытии/создании файла возникла ошибка
    if (mainLogging) Serial.println("Ошибка: " + file + " не был прочитан.");
    f.close();
    return("error");
  }
  String out = f.readStringUntil('\n');
  f.close();
  return out.substring(0, out.length()-1);
}
void writeFile(String file, String data) {
  File f = SPIFFS.open(file, "w");
  if (!f) { // если при открытии/создании файла возникла ошибка
    if (mainLogging) Serial.println("Ошибка: в " + file + " не была записана информация.");
  }
  else f.println(data);
  f.close();
}

void checkMemory() {
  SPIFFS.begin();
  if (!SPIFFS.exists("/firm_vers")) { // если первый запуск (файла не существует)
    drawClearingPage();
    SPIFFS.format(); // форматирование хранилища

    writeFile("/firm_vers", String(FIRMWARE_VERSION));
    writeFile("/WiFi_SSID", "");
    setupAP();
  }
  else {
    if (readFile("/WiFi_SSID") == "") setupAP(); // переходим к настройке, если они не заданы
    String ver = readFile("/firm_vers");
    if (ver != String(FIRMWARE_VERSION)) {
      delay(1000); // задержка для того, чтобы вывод не потерялся
      Serial.println("Прошлая версия прошивки: " + ver);

      // Возможно тут когда-нибудь появится добавление новых данных

      writeFile("/firm_vers", String(FIRMWARE_VERSION));
      Serial.println("Новая версия прошивки: " + readFile("/firm_vers"));
    }
  }
  apiKey = readFile("/API_key");
  
  lat = readFile("/latitude").toFloat();
  lon = readFile("/longitude").toFloat();
  
  units = readFile("/units").toInt();
  activateDebugPage = readFile("/act_debug_p").toInt();
  tempSensorsReversed = readFile("/temp_sens_rev").toInt();
  oneSensorIsOutdoor = readFile("/one_sensor_is_outdoor").toInt();
  weatherIconsCheckingCounters = readFile("/wea_icon_counts").toInt();
  mainLogging = readFile("/main_logging").toInt();
  wifiLogging = readFile("/wifi_logging").toInt();
  setupAtPowerOn = readFile("/setup_at_power_on").toInt();
}

String getValue(String data, char separator, int index) { // функция отсюда: https://arduino.stackexchange.com/a/1237
    int found = 0;
    int strIndex[] = {0, -1};
    int maxIndex = data.length()-1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
      if (data.charAt(i) == separator || i == maxIndex) {
        found++;
        strIndex[0] = strIndex[1] + 1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
      }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}
