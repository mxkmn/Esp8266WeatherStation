uint32_t timeWhenLAndRButtonPressed = 0; uint8_t rButtonPressCounter = 0;

void checkButtons() {
  lButtonPressed = digitalRead(LEFT_BUTTON_PIN); rButtonPressed = analogRead(RIGHT_BUTTON_PIN)>512;
  yield(); delayMicroseconds(10000); // на 1000 умирает
  if (lButtonPressed or rButtonPressed) { // если кнопка нажата
    keysPressed = now(); // записываем когда нажата кнопка
    
    int tmp = nowBrightness;
    tryChangeBrightness();
    if (nowBrightness != tmp) {
      return; // если яркость изменилась - выходим, ведь нам не нужно изменение вкладки
    }
    else if (lButtonPressed) rButtonPressCounter = 0;
  }
  else {
    rButtonPressCounter = 0;
    return;
  }
  if (lButtonPressed and rButtonPressed) { // при нажатии обеих кнопок возвращаемся в начало
    if (pageType!=0 or pageNum!=0) { 
      flipLeft(mainP); pageType = 0; pageNum = 0;
    }
    timeWhenLAndRButtonPressed = millis();
  }
  else if (timeWhenLAndRButtonPressed+100>millis()) {
    if (timeWhenLAndRButtonPressed>millis()+1) timeWhenLAndRButtonPressed = 0; // если произошло обнуление millis - обнуляем переменную
    else return; // а это нужно для того, чтобы после перехода на основную вкладку и убирания пальцев не было ненужного перехода влево/вправо
  }
  else if (lButtonPressed) { // при нажатии левой кнопки
    if (pageNum == 0 or (pageType == 1 and pageNum <= getActualHour()) or (pageType == 2 and pageNum <= getActualDay())) { // если нужно перейти в другой тип вкладки
      if (pageType == 0) { // если сейчас открыта основная вкладка
        pageNum = getActualHour();
        if (!(pageNum < MAX_HOURS)) pageNum = MAX_HOURS-1;
        flipLeft(hourP, pageNum);
      }
      else if (pageType == 1) { // если сейчас открыта вкладка с почасовым прогнозом
        pageNum = getActualDay();
        if (!(pageNum < MAX_DAYS)) pageNum = MAX_DAYS-1;
        flipLeft(dayP, pageNum);
      }
      else { pageNum = 0; flipLeft(mainP); } // если сейчас открыта вкладка с дневным прогнозом
      pageType = (pageType+1)%3;
    }
    else { // если нужно перейти в прошлую вкладку того же типа
      if (pageType == 0 and pageNum == 1) { flipLeft(mainP); pageNum--; }
      else if (pageType == 0 and pageNum == 2) { flipLeft(addP); pageNum--; }
      else if (pageType == 1) flipLeft(hourP, --pageNum);
      else flipLeft(dayP, --pageNum);
    }
  }
  else if (rButtonPressed) { // при нажатии правой кнопки
    if (pageType == 0 and pageNum == 0) { flipRight(addP); pageNum = 1; }
    else if (activateDebugPage and pageType == 0 and pageNum == 1) { flipRight(debugP); pageNum = 2; }
    else if (pageType == 1 and pageNum < MAX_HOURS-1) {
      pageNum += 1 + rButtonPressCounter/3;
      if (pageNum > MAX_HOURS-1) pageNum = MAX_HOURS-1;
      flipRight(hourP, pageNum);
    }
    else if (pageType == 2 and pageNum < MAX_DAYS-1) {
      pageNum += 1 + rButtonPressCounter/3;
      if (pageNum > MAX_DAYS-1) pageNum = MAX_DAYS-1;
      flipRight(dayP, pageNum);
    }
    rButtonPressCounter++;
  }
  if (mainLogging) { Serial.print("Page: "); Serial.print(pageType); Serial.print(", additional: "); Serial.print(pageNum); Serial.println("."); } // вывод номера вкладки
}
