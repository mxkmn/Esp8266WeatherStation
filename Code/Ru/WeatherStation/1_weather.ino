int8_t nowTemperature, nowFeels, hourlyTemperature[MAX_HOURS], hourlyFeels[MAX_HOURS], dailyTemperatureMin[MAX_DAYS], dailyTemperatureMax[MAX_DAYS], weatherTypeCounter = 0, weatherIcon = 0;
uint16_t nowHumidity, nowWindSpeed, nowPressure, hourlyStatus[MAX_HOURS], dailyStatus[MAX_DAYS];
uint32_t timeZone, sunrise, sunset;

time_t weatherChecked, buttonPressed = 0, weatherUpdated = 0;
void tryUpdateWeather() {
  if (now()-weatherUpdated>DELAY_CHECK_WEATHER or weatherUpdated==0) {
    if (pageType == 0) { lcd.setCursor(15, 0); lcd.write(byte(3)); } // вывод иконки часов

    if (mainLogging) Serial.print("\nПолучение погоды... ");
    OWM_current *current = new OWM_current; OWM_hourly *hourly = new OWM_hourly; OWM_daily *daily = new OWM_daily; // формируем массивы данных

    bool success = owm.getWeather(current, hourly, daily, apiKey, getLat(weatherTypeCounter), getLon(weatherTypeCounter), (units == IMPERIAL ? "imperial" : "metric"), "en", (weatherTypeCounter == 0 ? "full" : "current")); // получение погоды

    if (success) { // если данные с сервера успешно получены
      if (timeZone != current->timezone_offset and (now() - timeZone)>0) { // если часовой пояс изменился
        setTime(now() + (current->timezone_offset - timeZone)); // изменяем время на разницу
        timeZone = current->timezone_offset;
      }

      // добавляем данные в переменные
      nowTemperature = round(current->temp); nowFeels = round(current->feels_like); // температура правдивая и чувствуемая
      nowHumidity = current->humidity; nowWindSpeed = round(current->wind_speed); // влажность, скорость ветра
      if (units == RUS) nowPressure = round((current->pressure)*0.75); // давление в мм рт. ст.
      else nowPressure = current->pressure; // давление в hPa
      sunrise = getSecondsInDay(current->sunrise+timeZone); sunset = getSecondsInDay(current->sunset+timeZone); // время восхода и захода
      if (weatherTypeCounter == 0) {
        weatherChecked = current->dt+timeZone; // время последнего обновления погоды на сервере (она обновляется не каждую секунду, минимум раз в 120 секунд)
        for (int i = 0; i<MAX_HOURS; i++) { hourlyTemperature[i] = round(hourly->temp[i]); hourlyFeels[i] = round(hourly->feels_like[i]); hourlyStatus[i] = hourly->id[i]; } // часовые данные
        for (int i = 0; i<MAX_DAYS; i++) { dailyTemperatureMin[i] = round(daily->temp_min[i]); dailyTemperatureMax[i] = round(daily->temp_max[i]); dailyStatus[i] = daily->id[i]; } // дневные данные
      }
      hourlyStatus[0] = current->id; // устанавливаем данные о текущем статусе в нулевой час
      updateIcon(); // определяет будет ли дождь или снег в следующие weatherIconsCheckingCounters часов
      // Serial.print("Погода обновлена на сервере в "); printTime(current->dt+timeZone); Serial.println();
      if (mainLogging) printCurrentWeather();
      weatherUpdated = now(); weatherTypeCounter = (weatherTypeCounter+1)%5;
    }
    else if (weatherUpdated - now() > 60 * 10) weatherTypeCounter = 0; // устанавливаем следующую проверку погоды полной, если погода не обновлялась более 10 минут
    delete current; delete hourly; delete daily; // удаляем для освобождения пространства и последующей правильной работы с этими переменными
    if (pageType == 0) { updateWeatherIcon(); } // изменение иконки часов
    // Serial.print("Free RAM on weather: "); Serial.println(ESP.getFreeHeap());
  }
}

void printCurrentWeather() {
  Serial.print("Погода получена! Часовой пояс: "); Serial.println(timeZone/3600);

  Serial.print("Текущая температура: "); Serial.print(nowTemperature); Serial.print(", ощущается: "); Serial.print(nowFeels); Serial.print(", влажность: "); Serial.print(nowHumidity);
  Serial.print("%, скорость ветра: "); Serial.print(nowWindSpeed); Serial.print(" м/с, давление: "); Serial.print(nowPressure); Serial.println(" мм рт. ст.");
  Serial.print("Восход и заход Солнца: "); printTime(sunrise); Serial.print(", "); printTime(sunset); Serial.println("\n");

  if (weatherTypeCounter == 0) {
    for (int i = 0; i<MAX_HOURS; i++) { 
      Serial.print("Температура через "); Serial.print(i); Serial.print(" часов: "); Serial.print(hourlyTemperature[i]); Serial.print(", ощущается: ");
      Serial.print(hourlyFeels[i]); Serial.print(", статус: "); Serial.print(hourlyStatus[i]); Serial.print(" ("); Serial.print(getStatus(hourlyStatus[i])); Serial.println(").");
    } Serial.println();
  
    for (int i = 0; i<MAX_DAYS; i++) {
      Serial.print("Прогноз спустя "); Serial.print(i); Serial.print(" суток: ");
      Serial.print("минимальная температура: "); Serial.print(dailyTemperatureMin[i]); Serial.print(", максимальная: "); Serial.print(dailyTemperatureMax[i]); Serial.println(".");
    } Serial.println();
  }
}
void updateIcon() {
  if (weatherIconsCheckingCounters == 100) return; // исключение при отключенном показе иконки
  int tmp = getActualHour();
  for (int i = tmp; i<=(weatherIconsCheckingCounters+tmp<=MAX_HOURS-1 ? weatherIconsCheckingCounters+tmp : MAX_HOURS-1); i++) {
    uint16_t st = hourlyStatus[i];
    if ( // если дождь
      st==232 or st==201 or st==202 or st==230 or st==200 or st==231 or st==300 or st==301 or st==302 or st==321 or st==310 or 
      st==311 or st==312 or st==313 or st==314 or st==500 or st==501 or st==502 or st==520 or st==503 or st==521 or st==504 or 
      st==522 or st==511 or st==531 or st==612 or st==613 or st==615 or st==616 or st==620 or st==621 or st==622
    ) { weatherIcon = 1; return; }
    else if ( // если снег
      st==600 or st==601 or st==602
    ) { weatherIcon = 2; return; }
  }
  weatherIcon = 0;
}

String getLat(uint8_t st) {
  if (st == 0) return(String(lat));
  int out = floor(lat*100);
  if (st == 1 or st == 2) return(String(out*1.0/100));
  else return(String(out*1.0/100+0.01));
}
String getLon(uint8_t st) {
  if (st == 0) return(String(lon));
  int out = floor(lon*100);
  if (st == 1 or st == 4) return(String(out*1.0/100));
  else return(String(out*1.0/100+0.01));
}

String getStatus(uint16_t st) {
  switch (st) {
    case 800: return("Ясно");
    case 801: return("Малооблачно");
    case 802: return("Перем.облачность");
    case 803: return("Облачно");
    case 804: return("Пасмурно");

    case 211: return("Гроза");
    case 210: return("Слабая гроза");
    case 212: return("Сильная гроза");
    case 221: return("Местами гроза");
    case 200:
    case 230:
    case 231: return("Гроза с моросью");
    case 201:
    case 232: return("Гроза с дождём");
    case 202: return("Гроза и ливень");
    case 300: return("Небольшая морось");
    case 301: return("Морось");
    case 302:
    case 321: return("Сильная морось");
    case 310:
    case 311:
    case 312: return("Дождь и морось");
    case 313:
    case 314: return("Ливень и морось");
    case 500: return("Лёгкий дождь");
    case 501: return("Умеренный дождь");
    case 502:
    case 520: return("Сильный дождь");
    case 531: return("Местами ливень");
    case 503:
    case 521: return("Ливень");
    case 504:
    case 522: return("Сильный ливень");
    case 511: return("Ледяной дождь");
    case 600: return("Небольшой снег");
    case 601: return("Снегопад");
    case 602: return("Сильный снегопад");
    case 611: return("Слякоть");
    case 612: return("Дождь и слякоть");
    case 613: return("Ливень и слякоть");
    case 615:
    case 616:
    case 620:
    case 621: return("Дождь со снегом"); // Я выглянул в окно узнать, какая погода. На улице воздух был серым. С неба летели либо снежинки, либо дождевые капли. Я подумал: «Снегодождь. Опять снегодождь».
    case 622: return("Ливень со снегом");
    case 701: return("Туман");
    case 711: return("Дым");
    case 721: return("Лёгкий туман");
    case 731: return("Вихри пыли");
    case 741: return("Мгла");
    case 751: return("Дует песком");
    case 761: return("Пыльно");
    case 771: return("Сильный ветер");
    case 762: return("Вулканич. пепел");
    case 781: return("Торнадо");
    default: {
      String ret = "Неизвестно ";
      if (st > 999) ret += st;
      else { ret += "("; ret += st; ret += ")"; }
      return(ret);
    }
  }
}
