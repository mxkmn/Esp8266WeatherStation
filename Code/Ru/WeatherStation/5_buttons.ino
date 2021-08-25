uint8_t rButtonPressCounter = 0;
time_t timeWhenLAndRButtonPressed = 0;

void updateButtonVars(bool useDelay) {
  lButtonPressed = digitalRead(LEFT_BUTTON_PIN);
  rButtonPressed = analogRead(RIGHT_BUTTON_PIN) > 512;

  if (useDelay) {
    yield();
    delayMicroseconds(10000); // на 1000 умирает
  }
}

void checkButtons() { // проверка нажатия на кнопки, предпринятие действий при нажатии
  updateButtonVars(true);
  if (lButtonPressed or rButtonPressed) { // если кнопка нажата
    keysPressedTime = millis(); // записываем когда нажата кнопка

    int tmpLcdBrightness = lcdBrightness;
    tryChangeBrightness();
    if (lcdBrightness != tmpLcdBrightness) return; // если яркость изменилась - выходим, ведь нам не нужно изменение вкладки
    else if (lButtonPressed) rButtonPressCounter = 0; // мы должны обнулять переменную для ускорения перехода вправо если нажаты обе кнопки
  }
  else {
    rButtonPressCounter = 0; // мы должны обнулять переменную для ускорения перехода вправо в случае, если кнопки не нажаты
    return;
  }

  if (lButtonPressed and rButtonPressed) { // при нажатии обеих кнопок возвращаемся в начало
    if (pageType != 0 or pageNum != 0) flip(D_LEFT, mainP);
    timeWhenLAndRButtonPressed = millis(); // переменная для того, чтобы какое-то время после включения подсветки была задержка, во время которого не будет происходить перелистывания вкладок
  }
  else if (timeWhenLAndRButtonPressed+100 > millis()) {
    if (timeWhenLAndRButtonPressed > millis()+1) timeWhenLAndRButtonPressed = 0; // если произошло обнуление millis - обнуляем переменную
    else return; // а это нужно для того, чтобы после перехода на основную вкладку и убирания пальцев не было ненужного перехода влево/вправо
  }
  else if (lButtonPressed) { // при нажатии левой кнопки
    if (pageNum == 0 or (pageType == 1 and pageNum <= getActualHour()) or (pageType == 2 and pageNum <= getActualDay())) { // если нужно перейти в другой тип вкладки
      if (pageType == 0) { // если сейчас открыта основная вкладка
        pageNum = getActualHour();
        if (!(pageNum < MAX_HOURS)) pageNum = MAX_HOURS-1;
        pageType = 1;

        flip(D_LEFT, hourP, pageNum);
      }
      else if (pageType == 1) { // если сейчас открыта вкладка с почасовым прогнозом
        pageNum = getActualDay();
        if (!(pageNum < MAX_DAYS)) pageNum = MAX_DAYS-1;
        pageType = 2;

        flip(D_LEFT, dayP, pageNum);
      }
      else { // если сейчас открыта вкладка с дневным прогнозом
        flip(D_LEFT, mainP);
      }
    }
    else { // если нужно перейти в прошлую вкладку того же типа
      if (pageType == 0 and pageNum == 1) flip(D_LEFT, mainP);
      else if (pageType == 0 and pageNum == 2) flip(D_LEFT, addP);
      else if (pageType == 1) flip(D_LEFT, hourP, --pageNum);
      else flip(D_LEFT, dayP, --pageNum);
    }
  }
  else if (rButtonPressed) { // при нажатии правой кнопки
    if (pageType == 0 and pageNum == 0) flip(D_RIGHT, addP);
    else if (isDebugPageActivated and pageType == 0 and pageNum == 1) flip(D_RIGHT, debugP);
    else if (pageType == 1 and pageNum < MAX_HOURS-1) {
      pageNum += 1 + rButtonPressCounter/3;
      if (pageNum > MAX_HOURS-1) pageNum = MAX_HOURS-1;

      flip(D_RIGHT, hourP, pageNum);
    }
    else if (pageType == 2 and pageNum < MAX_DAYS-1) {
      pageNum += 1 + rButtonPressCounter/3;
      if (pageNum > MAX_DAYS-1) pageNum = MAX_DAYS-1;

      flip(D_RIGHT, dayP, pageNum);
    }
    rButtonPressCounter++; // ускорение при зажатии правой кнопки
  }
  printPage();
}
