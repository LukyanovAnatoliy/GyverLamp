uint32_t effTimer;

void effectsTick() {
  if (ONflag && millis() - effTimer >= ((currentMode < 5 || currentMode > 13) ? modes[currentMode].speed : 50) ) {
    effTimer = millis();
    switch (currentMode) {
      case 0: sparklesRoutine();
        break;
      case 1: fireRoutine();
        break;
      case 2: rainbowVertical();
        break;
      case 3: rainbowHorizontal();
        break;
      case 4: snowStormRoutine();
        break;
      case 5: madnessNoise();
        break;
      case 6: cloudNoise();
        break;
      case 7: lavaNoise();
        break;
      case 8: plasmaNoise();
        break;
//      case 9: rainbowNoise();
      case 9: starfallRoutine();
        break;
//      case 10: rainbowStripeNoise();
      case 10: ballsRoutine();
        break;
//      case 11: zebraNoise();
      case 11: ballRoutine();
        break;
      case 12: forestNoise();
        break;
      case 13: oceanNoise();
        break;
//      case 14: colorRoutine();
      case 14: lightBallsRoutine();
        break;
      case 15: snowRoutine();
        break;
      case 16: matrixRoutine();
        break;
      case 17: lightersRoutine();
        break;
    }
    FastLED.show();
  }
}

void changePower() {
  if (ONflag) {
    effectsTick();
    for (int i = 0; i < currentBrightness; i += 8) {
      FastLED.setBrightness(i);
      delay(1);
      FastLED.show();
    }
    FastLED.setBrightness(currentBrightness);
    delay(2);
    FastLED.show();
  } else {
    effectsTick();
    for (int i = currentBrightness; i > 8; i -= 8) {
      FastLED.setBrightness(i);
      delay(1);
      FastLED.show();
    }
    FastLED.clear();
    delay(2);
    FastLED.show();
  }
}
