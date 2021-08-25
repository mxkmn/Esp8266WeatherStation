int8_t cTemp, cFeelingTemp, hTemp[MAX_HOURS], hFeelingTemp[MAX_HOURS], dMinTemp[MAX_DAYS], dMaxTemp[MAX_DAYS], weatherTypeCounter = 0, weatherIcon = 0;
uint16_t cHumidity, cWindSpeed, cPressure, hStatus[MAX_HOURS], dStatus[MAX_DAYS];
int32_t timezoneOffset = 0;
time_t sunriseTime, sunsetTime;
float trueLatitude, trueLongitude;
OWM_current *current = new OWM_current; OWM_hourly *hourly = new OWM_hourly; OWM_daily *daily = new OWM_daily; // формируем структуры данных

time_t lastFullWeatherUpdate, lastWeatherUpdate = 0;

void tryUpdateWeather() {
  if ((now()-lastWeatherUpdate > CHECK_WEATHER_DELAY or lastWeatherUpdate == 0) and WiFi.status() == WL_CONNECTED) {
    printUpdatingIcon(true); // установка иконки часов

    // получение погоды
    if (useMainLogging) Serial.print("\nПолучение погоды... ");

    bool success = owm.getWeather(current, hourly, daily, getLat(weatherTypeCounter), getLon(weatherTypeCounter), (weatherTypeCounter == 0 ? FULL : CURRENT));

    if (success) { // если данные с сервера успешно получены
      if (useMainLogging) Serial.println("Погода получена!");

      if (timezoneOffset != current->timezoneOffset) { // если часовой пояс изменился
        if (now() + (current->timezoneOffset - timezoneOffset) >= 0) {
          setTime(now() + (current->timezoneOffset - timezoneOffset)); // изменяем время на разницу
        }
        timezoneOffset = current->timezoneOffset; // устанавливаем часовой пояс

        if (useMainLogging) {
          Serial.print("Часовой пояс изменён: теперь это " + String( (timezoneOffset >= 0) ? "+":"" ) + String(timezoneOffset/3600.0) + " часов.");
          Serial.print(" Сейчас "); printTime(now()); Serial.println(".\n");
        }
      }

      // добавляем данные в переменные
      cTemp = round(current->mainTemp); cFeelingTemp = round(current->feelsLikeTemp); // температура настоящая и ощущаемая
      cHumidity = current->humidity; cWindSpeed = round(current->windSpeed); // влажность и скорость ветра
      if (units == U_RUS) cPressure = round((current->pressureSeaLevel)*0.75); // давление в мм рт. ст. (над уровнем моря, поэтому показания отличаются от давления из многих сервисов с прогнозами)
      else cPressure = current->pressureSeaLevel; // давление в hPa
      sunriseTime = getSecondsInDay(current->sunriseTime+timezoneOffset); sunsetTime = getSecondsInDay(current->sunsetTime+timezoneOffset); // время восхода и захода
      if (weatherTypeCounter == 0) {
        lastFullWeatherUpdate = current->serverUpdateTime+timezoneOffset; // время последнего обновления погоды на сервере (она обновляется не каждую секунду, минимум раз в 120 секунд)
        for (int i = 0; i < MAX_HOURS; i++) { hTemp[i] = round(hourly->mainTemp[i]); hFeelingTemp[i] = round(hourly->feelsLikeTemp[i]); hStatus[i] = hourly->weatherCondId[i]; } // почасовой прогноз
        for (int i = 0; i < MAX_DAYS; i++) { dMinTemp[i] = round(daily->minTemp[i]); dMaxTemp[i] = round(daily->maxTemp[i]); dStatus[i] = daily->weatherCondId[i]; } // дневной прогноз
      }
      hStatus[0] = current->weatherCondId; // устанавливаем данные о текущем статусе в нулевой час
      updateIcon(); // определяет будут ли осадки в следующие часы для отрисовки иконки дождя/снега

      if (useMainLogging) printCurrentWeather();
      lastWeatherUpdate = now(), weatherTypeCounter = (weatherTypeCounter+1)%5;
    }
    else if (lastWeatherUpdate-now() > 60*10) weatherTypeCounter = 0; // устанавливаем следующую проверку погоды полной, если погода не обновлялась более 10 минут
    printUpdatingIcon(false); // установка прошлой (или обновлённой) иконки
  }
}

void printCurrentWeather() {
  Serial.print("Погода обновлена на сервере в "); printTime(current->serverUpdateTime+timezoneOffset); Serial.println();

  Serial.println("Сейчас " + String(cTemp) + "° (ощущается " + String(cFeelingTemp) + "°), влажность " + String(cHumidity) + "%, скорость ветра "
                 + String(cWindSpeed) + (units == U_IMPERIAL ? " миль/ч" : " м/с") + ", давление " + String(cPressure) + (units == U_RUS ? " мм рт. ст." : " гПа"));
  
  Serial.print("Восход и заход Солнца: "); printTime(sunriseTime); Serial.print(", "); printTime(sunsetTime); Serial.println(".\n");

  if (weatherTypeCounter == 0) {
    for (int i = 0; i < MAX_HOURS; i++) { 
      Serial.println("Через " + String(i) + " часов: температура " + String(hTemp[i]) + "° (ощущается " + String(hFeelingTemp[i]) + "°), "
                     + owm.getStatus16(hStatus[i]) + " (код " + String(hStatus[i]) + ").");
    }
    Serial.println();
  
    for (int i = 0; i < MAX_DAYS; i++) {
      Serial.println("Через " + String(i) + " суток: минимальная температура " + String(dMinTemp[i]) + "°, максимальная " + String(dMaxTemp[i]) + "°, "
                     + owm.getStatus16(dStatus[i]) + " (код " + String(dStatus[i]) + ").");
    }
    Serial.println();
  }
}

void updateIcon() {
  if (weatherIconsCheckingCounters == 100) return; // исключение при отключенном показе иконки
  int tmp = getActualHour();
  for (int i = tmp; i <= (weatherIconsCheckingCounters+tmp <= MAX_HOURS-1 ? weatherIconsCheckingCounters+tmp : MAX_HOURS-1); i++) {
    switch (hStatus[i]) {
      case 200: case 201: case 202: case 230: case 231: case 232: case 300: case 301: case 302: case 310: case 311: case 312: case 313:
      case 314: case 321: case 500: case 501: case 502: case 503: case 504: case 511: case 520: case 521: case 522: case 531:
        { weatherIcon = 1; return; } // если дождь
      case 600: case 601: case 602: case 611: case 612: case 613: case 615: case 616: case 620: case 621: case 622:
        { weatherIcon = 2; return; } // если снег
    }
  }
  weatherIcon = 0;
}

bool isWeatherOutdated() { // проверка, устарела ли погода
  return (now()-lastWeatherUpdate > CHECK_WEATHER_DELAY*2.5);
}

String getLat(uint8_t i) { // получение широты в зависимости от итерации запросов
  if (i == 0) return(String(trueLatitude)); // для full запроса просто выдаём настоящие координаты

  // для current запроса, которому необходимы различные координаты, начинаем их изменять в зависимости от итерации i, ибо иначе данные будут актуализироваться лишь раз в 10 минут
  int out = floor(trueLatitude*100); // приводим широту вида 55.758 к числу 5575
  if (i == 1 or i == 2) return(String(out*1.0/100)); // в случае итераций 1 и 2 выдаём число 55.75
  else return(String(out*1.0/100+0.01)); // в случае итераций 3 и 4 выдаём число 55.76 (немного "смещаемся" - и это новое место, которое обработает OWM!)
}

String getLon(uint8_t i) { // получение долготы в зависимости от итерации запросов
  if (i == 0) return(String(trueLongitude)); // для full запроса просто выдаём настоящие координаты

  // для current запроса, которому необходимы различные координаты, начинаем их изменять в зависимости от итерации i, ибо иначе данные будут актуализироваться лишь раз в 10 минут
  int out = floor(trueLongitude*100); // приводим долготу вида 135.373 к числу 13537
  if (i == 1 or i == 4) return(String(out*1.0/100)); // в случае итераций 1 и 4 выдаём число 135.37
  else return(String(out*1.0/100+0.01)); // в случае итераций 2 и 3 выдаём число 135.38 (немного "смещаемся" - и это новое место, которое обработает OWM!)
}
