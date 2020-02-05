boolean brightDirection;
boolean colorDirection;
boolean scaleDirection;

void buttonTick() {
  touch.tick();
  if (touch.isSingle()) {
      ONflag = !ONflag;
      changePower();
      sendSettings_flag = true;
  }

  if (!ONflag) return;
  
  if (touch.isDouble()) {
    if (++currentMode >= MODE_AMOUNT) currentMode = 0;
    loadingFlag = true;
    settChanged = true;
    eepromTimer = millis();
    FastLED.clear();
    delay(1);
    sendSettings_flag = true;
    return;
  }
  
  if (touch.isTriple()) {
    if (--currentMode < 0) currentMode = MODE_AMOUNT - 1;
    loadingFlag = true;
    settChanged = true;
    eepromTimer = millis();
    FastLED.clear();
    delay(1);
    sendSettings_flag = true;
    return;
  }

  // вывод IP на лампу
  if (touch.hasClicks()) {
    if (touch.getClicks() == 5) {
      resetString();
      while (!fillString(lampIP, CRGB::Green, true)) {
        delay(1);
        ESP.wdtFeed();   // пнуть собаку
        yield();  // ещё раз пнуть собаку
      }
      return;
    }
  }

  // если меняем яркость
  if (touch.isStep()) {
    if (touch.isHolded()) {
      brightDirection = !brightDirection;
    }
    if (brightDirection) {
      if (currentBrightness < 10) currentBrightness += 1;
      else if (currentBrightness < 250) currentBrightness += 5;
      else currentBrightness = 255;
    } else {
      if (currentBrightness > 15) currentBrightness -= 5;
      else if (currentBrightness > 1) currentBrightness -= 1;
      else currentBrightness = 1;
    }
    FastLED.setBrightness(currentBrightness);
    settChanged = true;
    eepromTimer = millis();
    sendSettings_flag = true;
    return;
  }

  // если меняем цвет
  if (touch.isStep(1)) {
    if (touch.isHolded()) {
      colorDirection = !colorDirection;
    }
    if (colorDirection) {
      if (modes[currentMode].scale < 100) modes[currentMode].scale += 1;
      else modes[currentMode].scale = 100;
    } else {
      if (modes[currentMode].scale > 1) modes[currentMode].scale -= 1;
      else modes[currentMode].scale = 1;
    }
    Serial.println(String(modes[currentMode].scale));
    effectsTick();
    FastLED.show();
    settChanged = true;
    eepromTimer = millis();
    sendSettings_flag = true;
    return;
  }

  // если меняем скорость
  if (touch.isStep(2)) {
    if (touch.isHolded()) {
      scaleDirection = !scaleDirection;
    }
    if (scaleDirection) {
      if (modes[currentMode].speed < 40) modes[currentMode].speed += 1;
      else if (modes[currentMode].speed < 250) modes[currentMode].speed += 5;
      else modes[currentMode].speed = 255;
    } else {
      if (modes[currentMode].speed > 40) modes[currentMode].speed -= 5;
      else if (modes[currentMode].speed > 1) modes[currentMode].speed -= 1;
      else modes[currentMode].speed = 1;
    }
    Serial.println(String(modes[currentMode].speed));
    effectsTick();
    settChanged = true;
    eepromTimer = millis();
    sendSettings_flag = true;
    return;
  }
}
