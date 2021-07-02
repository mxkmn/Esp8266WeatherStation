int8_t outdoorTemp = -127, indoorTemp = -127;
uint32_t lastTemperatureRequest = 0;

void requestTemperature() { // опрос дачиков
  sensors.requestTemperatures();
  lastTemperatureRequest = (now()+7)/10;
}
void updateTemperature() { // обновление температуры
  int8_t sensor1, sensor2;
  if (units == IMPERIAL) { // получение температуры в градусах Фаренгейта
    sensor1 = round(sensors.getTempFByIndex(0));
    sensor2 = round(sensors.getTempFByIndex(1));
  }
  else { // получение температуры в градусах Цельсия
    sensor1 = round(sensors.getTempCByIndex(0));
    sensor2 = round(sensors.getTempCByIndex(1));
  }
  if (sensor2 != -127) { // если подключено два датчика
    if (!tempSensorsReversed) { outdoorTemp = sensor1; indoorTemp = sensor2; }
    else                    { outdoorTemp = sensor2; indoorTemp = sensor1; }
  }
  else { // если подключён один датчик или ни одного
    if (oneSensorIsOutdoor) { outdoorTemp = sensor1; indoorTemp = -127; } // если настроено, что единственный датчик - внешний
    else                    { indoorTemp = sensor1; outdoorTemp = -127; } // если настроено, что единственный датчик - внутренний
  }
  if (mainLogging) { Serial.print("Текущая температура: "); Serial.print(outdoorTemp); Serial.print(" за окном, "); Serial.print(indoorTemp); Serial.println(" в помещении."); } // вывод температуры
}
