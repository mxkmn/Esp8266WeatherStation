int8_t outdoorTemp = -127, indoorTemp = -127, lastOutdoorTempLogging = 127, lastIndoorTempLogging = 127;
time_t lastTemperatureUpdate = 0;

void requestTemperature() { // опрос дачиков
  sensors.requestTemperatures();
  lastTemperatureUpdate = (now()+7)/10;
}

void updateTemperature() { // обновление температуры
  int8_t sensor1, sensor2;
  if (units == U_IMPERIAL) { // получение температуры в градусах Фаренгейта
    sensor1 = round(sensors.getTempFByIndex(0));
    sensor2 = round(sensors.getTempFByIndex(1));
  }
  else { // получение температуры в градусах Цельсия
    sensor1 = round(sensors.getTempCByIndex(0));
    sensor2 = round(sensors.getTempCByIndex(1));
  }
  if (sensor2 != -127) { // если подключено два датчика
    if (!isTempSensorsReversed) { outdoorTemp = sensor1; indoorTemp = sensor2; }
    else                        { outdoorTemp = sensor2; indoorTemp = sensor1; }
  }
  else { // если подключён один датчик или ни одного
    if (isSingleSensorOutdoor) { outdoorTemp = sensor1; indoorTemp = -127; } // если настроено, что единственный датчик - внешний
    else                       { indoorTemp = sensor1; outdoorTemp = -127; } // если настроено, что единственный датчик - внутренний
  }
  if (useMainLogging and (outdoorTemp != lastOutdoorTempLogging or indoorTemp != lastIndoorTempLogging)) { // вывод температуры при useMainLogging и её изменении
    Serial.println("Температура изменилась: за окном " + String(outdoorTemp) + "°, в помещении " + String(indoorTemp) + "°.");
    lastOutdoorTempLogging = outdoorTemp, lastIndoorTempLogging = indoorTemp;
  }
}

bool isOutdoorSensorConnected() {
  return (outdoorTemp != -127);
}

bool isIndoorSensorConnected() {
  return (indoorTemp != -127);
}
