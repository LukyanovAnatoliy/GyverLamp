void parseUDP() {
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    int n = Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
    packetBuffer[n] = 0;
    inputBuffer = packetBuffer;

    if (inputBuffer.startsWith("DEB")) {
      if (sendSettings_flag) sendCurrent();
      else inputBuffer = "OK ";
      sendSettings_flag = false;
    } else if (inputBuffer.startsWith("GET")) {
      sendCurrent();
    } else if (inputBuffer.startsWith("EFF")) {
      settChanged = true;
      currentMode = (byte)inputBuffer.substring(3).toInt();
      loadingFlag = true;
      FastLED.clear();
      delay(1);
      sendCurrent();
    } else if (inputBuffer.startsWith("BRI")) {
      currentBrightness = inputBuffer.substring(3).toInt();
      FastLED.setBrightness(currentBrightness);
      settChanged = true;
      eepromTimer = millis();
    } else if (inputBuffer.startsWith("SPD")) {
      modes[currentMode].speed = inputBuffer.substring(3).toInt();
      loadingFlag = true;
      settChanged = true;
      eepromTimer = millis();
    } else if (inputBuffer.startsWith("SCA")) {
      modes[currentMode].scale = inputBuffer.substring(3).toInt();
      Serial.println(String(modes[currentMode].scale));
      loadingFlag = true;
      settChanged = true;
      eepromTimer = millis();
    } else if (inputBuffer.startsWith("P_ON")) {
      ONflag = true;
      changePower();
      sendCurrent();
    } else if (inputBuffer.startsWith("P_OFF")) {
      ONflag = false;
      changePower();
      sendCurrent();
    }

    char reply[inputBuffer.length() + 1];
    inputBuffer.toCharArray(reply, inputBuffer.length() + 1);
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Udp.write(reply);
    Udp.endPacket();
  }
}

void sendCurrent() {
  inputBuffer = "CURR";
  inputBuffer += " ";
  inputBuffer += String(currentMode);
  inputBuffer += " ";
  inputBuffer += String(currentBrightness);
  inputBuffer += " ";
  inputBuffer += String(modes[currentMode].speed);
  inputBuffer += " ";
  inputBuffer += String(modes[currentMode].scale);
  inputBuffer += " ";
  inputBuffer += String(ONflag);
}

void sendSettings() {
  sendCurrent();
  char reply[inputBuffer.length() + 1];
  inputBuffer.toCharArray(reply, inputBuffer.length() + 1);
  Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
  Udp.write(reply);
  Udp.endPacket();
}
