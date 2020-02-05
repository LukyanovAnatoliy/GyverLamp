
/*
 * Начало   Длина   Описание
 *   0       1      признак первого запуска
 *   1       1      последний режим
 *   2       1      состояние лампы вкл/выкл
 *   3       1      яркость
 * 4-20      17     резерв
 * 21...            запись состояния режимов
 */

#define EEPROM_MODE_STRUCT_SIZE    2
#define EEPROM_TOTAL_BYTES_USED    (1U + 1U + 1U + 1U + 17U + MODE_AMOUNT * EEPROM_MODE_STRUCT_SIZE + 1U)

#define EEPROM_FIRST_START_ADDRESS         0U
#define EEPROM_LAST_EFFECT_MODE_ADDRESS    1U
#define EEPROM_LAMP_STATE_ADDRESS          2U
#define EEPROM_BRIGHTNESS_ADDRESS          3U
#define EEPROM_EFFECT_SETUP_START_ADDRESS  21U

#define EEPROM_FIRST_START_MASK 24U

void saveEEPROM() {
  EEPROM.put(EEPROM_MODE_STRUCT_SIZE * currentMode + EEPROM_EFFECT_SETUP_START_ADDRESS, modes[currentMode]);
  EEPROM.commit();
}

void eepromTick() {
  if (settChanged && millis() - eepromTimer > 30000) {
    settChanged = false;
    eepromTimer = millis();
    saveEEPROM();
    if (EEPROM.read(EEPROM_LAST_EFFECT_MODE_ADDRESS) != currentMode) {
      Serial.println("Save currentMode");
      EEPROM.write(EEPROM_LAST_EFFECT_MODE_ADDRESS, currentMode);
    }

    if (EEPROM.read(EEPROM_BRIGHTNESS_ADDRESS) != currentBrightness) {
      Serial.println("Save currentBrightness");
      EEPROM.write(EEPROM_BRIGHTNESS_ADDRESS, currentBrightness);
    }

    if (EEPROM.read(EEPROM_LAMP_STATE_ADDRESS) != ONflag) {
      Serial.println("Save lamp state");
      EEPROM.write(EEPROM_LAMP_STATE_ADDRESS, ONflag);
    }
    
    EEPROM.commit();
  }
}

void initEEPROM() {
  EEPROM.begin(EEPROM_TOTAL_BYTES_USED);
  delay(50);
  if (EEPROM.read(EEPROM_FIRST_START_ADDRESS) != EEPROM_FIRST_START_MASK) {   // первый запуск
    Serial.println("First start setup");
    EEPROM.write(EEPROM_FIRST_START_ADDRESS, EEPROM_FIRST_START_MASK);
    Serial.println(EEPROM.commit());
    EEPROM.commit();
    Serial.println(String(EEPROM.read(EEPROM_FIRST_START_ADDRESS)));

    for (byte i = 0; i < MODE_AMOUNT; i++) {
      EEPROM.put(EEPROM_MODE_STRUCT_SIZE * i + EEPROM_EFFECT_SETUP_START_ADDRESS, modes[i]);
      EEPROM.commit();
    }
    EEPROM.write(EEPROM_LAST_EFFECT_MODE_ADDRESS, 0);   // режим
    EEPROM.commit();
    
    EEPROM.write(EEPROM_BRIGHTNESS_ADDRESS, 40);
    EEPROM.commit();
  }
  for (byte i = 0; i < MODE_AMOUNT; i++) {
    EEPROM.get(EEPROM_MODE_STRUCT_SIZE * i + EEPROM_EFFECT_SETUP_START_ADDRESS, modes[i]);
  }
  currentMode = (int8_t)EEPROM.read(EEPROM_LAST_EFFECT_MODE_ADDRESS);
  currentBrightness = (int8_t)EEPROM.read(EEPROM_BRIGHTNESS_ADDRESS);
  ONflag = (boolean)EEPROM.read(EEPROM_LAMP_STATE_ADDRESS);
}
