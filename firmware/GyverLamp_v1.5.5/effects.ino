// ================================= ЭФФЕКТЫ ====================================

#define EFF_SNOWSTORM 4
#define EFF_STARFALL 9
#define EFF_CUBE 10

// --------------------------------- конфетти ------------------------------------
void sparklesRoutine() {
  for (byte i = 0; i < modes[0].scale; i++) {
    byte x = random(0, WIDTH);
    byte y = random(0, HEIGHT);
    if (getPixColorXY(x, y) == 0)
      leds[getPixelNumber(x, y)] = CHSV(random(0, 255), 255, 255);
  }
  fader(70);
}

// функция плавного угасания цвета для всех пикселей
void fader(byte step) {
  for (byte i = 0; i < WIDTH; i++) {
    for (byte j = 0; j < HEIGHT; j++) {
      fadePixel(i, j, step);
    }
  }
}
void fadePixel(byte i, byte j, byte step) {     // новый фейдер
  int pixelNum = getPixelNumber(i, j);
  if (getPixColor(pixelNum) == 0) return;

  if (leds[pixelNum].r >= 30 ||
      leds[pixelNum].g >= 30 ||
      leds[pixelNum].b >= 30) {
    leds[pixelNum].fadeToBlackBy(step);
  } else {
    leds[pixelNum] = 0;
  }
}

// -------------------------------------- огонь ---------------------------------------------
// эффект "огонь"
#define SPARKLES 1        // вылетающие угольки вкл выкл
unsigned char line[WIDTH];
int pcnt = 0;

//these values are substracetd from the generated values to give a shape to the animation
const unsigned char valueMask[8][16] PROGMEM = {
  {32 , 0  , 0  , 0  , 0  , 0  , 0  , 32 , 32 , 0  , 0  , 0  , 0  , 0  , 0  , 32 },
  {64 , 0  , 0  , 0  , 0  , 0  , 0  , 64 , 64 , 0  , 0  , 0  , 0  , 0  , 0  , 64 },
  {96 , 32 , 0  , 0  , 0  , 0  , 32 , 96 , 96 , 32 , 0  , 0  , 0  , 0  , 32 , 96 },
  {128, 64 , 32 , 0  , 0  , 32 , 64 , 128, 128, 64 , 32 , 0  , 0  , 32 , 64 , 128},
  {160, 96 , 64 , 32 , 32 , 64 , 96 , 160, 160, 96 , 64 , 32 , 32 , 64 , 96 , 160},
  {192, 128, 96 , 64 , 64 , 96 , 128, 192, 192, 128, 96 , 64 , 64 , 96 , 128, 192},
  {255, 160, 128, 96 , 96 , 128, 160, 255, 255, 160, 128, 96 , 96 , 128, 160, 255},
  {255, 192, 160, 128, 128, 160, 192, 255, 255, 192, 160, 128, 128, 160, 192, 255}
};

//these are the hues for the fire,
//should be between 0 (red) to about 25 (yellow)
const unsigned char hueMask[8][16] PROGMEM = {
  {1 , 11, 19, 25, 25, 22, 11, 1 , 1 , 11, 19, 25, 25, 22, 11, 1 },
  {1 , 8 , 13, 19, 25, 19, 8 , 1 , 1 , 8 , 13, 19, 25, 19, 8 , 1 },
  {1 , 8 , 13, 16, 19, 16, 8 , 1 , 1 , 8 , 13, 16, 19, 16, 8 , 1 },
  {1 , 5 , 11, 13, 13, 13, 5 , 1 , 1 , 5 , 11, 13, 13, 13, 5 , 1 },
  {1 , 5 , 11, 11, 11, 11, 5 , 1 , 1 , 5 , 11, 11, 11, 11, 5 , 1 },
  {0 , 1 , 5 , 8 , 8 , 5 , 1 , 0 , 0 , 1 , 5 , 8 , 8 , 5 , 1 , 0 },
  {0 , 0 , 1 , 5 , 5 , 1 , 0 , 0 , 0 , 0 , 1 , 5 , 5 , 1 , 0 , 0 },
  {0 , 0 , 0 , 1 , 1 , 0 , 0 , 0 , 0 , 0 , 0 , 1 , 1 , 0 , 0 , 0 }
};

void fireRoutine() {
  if (loadingFlag) {
    loadingFlag = false;
    //FastLED.clear();
    generateLine();
  }
  if (pcnt >= 100) {
    shiftUp();
    generateLine();
    pcnt = 0;
  }
  drawFrame(pcnt);
  pcnt += 30;
}

// Randomly generate the next line (matrix row)

void generateLine() {
  for (uint8_t x = 0; x < WIDTH; x++) {
    line[x] = random(64, 255);
  }
}

void shiftUp() {
  for (uint8_t y = HEIGHT - 1; y > 0; y--) {
    for (uint8_t x = 0; x < WIDTH; x++) {
      uint8_t newX = x;
      if (x > 15) newX = x - 15;
      if (y > 7) continue;
      matrixValue[y][newX] = matrixValue[y - 1][newX];
    }
  }

  for (uint8_t x = 0; x < WIDTH; x++) {
    uint8_t newX = x;
    if (x > 15) newX = x - 15;
    matrixValue[0][newX] = line[newX];
  }
}

// draw a frame, interpolating between 2 "key frames"
// @param pcnt percentage of interpolation

void drawFrame(int pcnt) {
  int nextv;

  //each row interpolates with the one before it
  for (unsigned char y = HEIGHT - 1; y > 0; y--) {
    for (unsigned char x = 0; x < WIDTH; x++) {
      uint8_t newX = x;
      if (x > 15) newX = x - 15;
      if (y < 8) {
        nextv =
          (((100.0 - pcnt) * matrixValue[y][newX]
            + pcnt * matrixValue[y - 1][newX]) / 100.0)
          - pgm_read_byte(&(valueMask[y][newX]));

        CRGB color = CHSV(
                       modes[1].scale * 2.5 + pgm_read_byte(&(hueMask[y][newX])), // H
                       255, // S
                       (uint8_t)max(0, nextv) // V
                     );

        leds[getPixelNumber(x, y)] = color;
      } else if (y == 8 && SPARKLES) {
        if (random(0, 20) == 0 && getPixColorXY(x, y - 1) != 0) drawPixelXY(x, y, getPixColorXY(x, y - 1));
        else drawPixelXY(x, y, 0);
      } else if (SPARKLES) {

        // старая версия для яркости
        if (getPixColorXY(x, y - 1) > 0)
          drawPixelXY(x, y, getPixColorXY(x, y - 1));
        else drawPixelXY(x, y, 0);

      }
    }
  }

  //first row interpolates with the "next" line
  for (unsigned char x = 0; x < WIDTH; x++) {
    uint8_t newX = x;
    if (x > 15) newX = x - 15;
    CRGB color = CHSV(
                   modes[1].scale * 2.5 + pgm_read_byte(&(hueMask[0][newX])), // H
                   255,           // S
                   (uint8_t)(((100.0 - pcnt) * matrixValue[0][newX] + pcnt * line[newX]) / 100.0) // V
                 );
    leds[getPixelNumber(newX, 0)] = color;
  }
}

byte hue;
// ---------------------------------------- радуга ------------------------------------------
void rainbowVertical() {
  hue += 2;
  for (byte j = 0; j < HEIGHT; j++) {
    CHSV thisColor = CHSV((byte)(hue + j * modes[2].scale), 255, 255);
    for (byte i = 0; i < WIDTH; i++)
      drawPixelXY(i, j, thisColor);
  }
}
void rainbowHorizontal() {
  hue += 2;
  for (byte i = 0; i < WIDTH; i++) {
    CHSV thisColor = CHSV((byte)(hue + i * modes[3].scale), 255, 255);
    for (byte j = 0; j < HEIGHT; j++)
      drawPixelXY(i, j, thisColor);   //leds[getPixelNumber(i, j)] = thisColor;
  }
}

// ---------------------------------------- ЦВЕТА ------------------------------------------
void colorsRoutine() {
  hue += modes[4].scale;
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV(hue, 255, 255);
  }
}

// --------------------------------- ЦВЕТ ------------------------------------
void colorRoutine() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV(modes[14].scale * 2.5, 255, 255);
  }
}

// ------------------------------ снегопад 2.0 --------------------------------
void snowRoutine() {
  // сдвигаем всё вниз
  for (byte x = 0; x < WIDTH; x++) {
    for (byte y = 0; y < HEIGHT - 1; y++) {
      drawPixelXY(x, y, getPixColorXY(x, y + 1));
    }
  }

  for (byte x = 0; x < WIDTH; x++) {
    // заполняем случайно верхнюю строку
    // а также не даём двум блокам по вертикали вместе быть
    if (getPixColorXY(x, HEIGHT - 2) == 0 && (random(0, modes[15].scale) == 0))
      drawPixelXY(x, HEIGHT - 1, 0xE0FFFF - 0x101010 * random(0, 4));
    else
      drawPixelXY(x, HEIGHT - 1, 0x000000);
  }
}

// ------------------------------ МАТРИЦА ------------------------------
void matrixRoutine() {
  for (byte x = 0; x < WIDTH; x++) {
    // заполняем случайно верхнюю строку
    uint32_t thisColor = getPixColorXY(x, HEIGHT - 1);
    if (thisColor == 0)
      drawPixelXY(x, HEIGHT - 1, 0x00FF00 * (random(0, modes[16].scale) == 0));
    else if (thisColor < 0x002000)
      drawPixelXY(x, HEIGHT - 1, 0);
    else
      drawPixelXY(x, HEIGHT - 1, thisColor - 0x002000);
  }

  // сдвигаем всё вниз
  for (byte x = 0; x < WIDTH; x++) {
    for (byte y = 0; y < HEIGHT - 1; y++) {
      drawPixelXY(x, y, getPixColorXY(x, y + 1));
    }
  }
}

// ----------------------------- СВЕТЛЯКИ ------------------------------
#define LIGHTERS_AM 100
int lightersPos[2][LIGHTERS_AM];
int8_t lightersSpeed[2][LIGHTERS_AM];
CHSV lightersColor[LIGHTERS_AM];
byte loopCounter;

int angle[LIGHTERS_AM];
int speedV[LIGHTERS_AM];
int8_t angleSpeed[LIGHTERS_AM];

void lightersRoutine() {
  if (loadingFlag) {
    loadingFlag = false;
    randomSeed(millis());
    for (byte i = 0; i < LIGHTERS_AM; i++) {
      lightersPos[0][i] = random(0, WIDTH * 10);
      lightersPos[1][i] = random(0, HEIGHT * 10);
      lightersSpeed[0][i] = random(-10, 10);
      lightersSpeed[1][i] = random(-10, 10);
      lightersColor[i] = CHSV(random(0, 255), 255, 255);
    }
  }
  FastLED.clear();
  if (++loopCounter > 20) loopCounter = 0;
  for (byte i = 0; i < modes[17].scale; i++) {
    if (loopCounter == 0) {     // меняем скорость каждые 255 отрисовок
      lightersSpeed[0][i] += random(-3, 4);
      lightersSpeed[1][i] += random(-3, 4);
      lightersSpeed[0][i] = constrain(lightersSpeed[0][i], -20, 20);
      lightersSpeed[1][i] = constrain(lightersSpeed[1][i], -20, 20);
    }

    lightersPos[0][i] += lightersSpeed[0][i];
    lightersPos[1][i] += lightersSpeed[1][i];

    if (lightersPos[0][i] < 0) lightersPos[0][i] = (WIDTH - 1) * 10;
    if (lightersPos[0][i] >= WIDTH * 10) lightersPos[0][i] = 0;

    if (lightersPos[1][i] < 0) {
      lightersPos[1][i] = 0;
      lightersSpeed[1][i] = -lightersSpeed[1][i];
    }
    if (lightersPos[1][i] >= (HEIGHT - 1) * 10) {
      lightersPos[1][i] = (HEIGHT - 1) * 10;
      lightersSpeed[1][i] = -lightersSpeed[1][i];
    }
    drawPixelXY(lightersPos[0][i] / 10, lightersPos[1][i] / 10, lightersColor[i]);
  }
}

// ------------- метель -------------
#define SNOW_DENSE            (60U)                         // плотность снега
#define SNOW_TAIL_STEP        (100U)                        // длина хвоста
#define SNOW_SATURATION       (0U)                          // насыщенность (от 0 до 255)

void snowStormRoutine()
{
  if (loadingFlag)
  {
    loadingFlag = false;
    FastLED.clear();
  }
  
  // заполняем головами комет левую и верхнюю линию
  for (uint8_t i = HEIGHT / 2U; i < HEIGHT; i++)
  {
    if (getPixColorXY(0U, i) == 0U &&
       (random(0, SNOW_DENSE) == 0) &&
        getPixColorXY(0U, i + 1U) == 0U &&
        getPixColorXY(0U, i - 1U) == 0U)
    {
      leds[getPixelNumber(0U, i)] = CHSV(random(0, 200), SNOW_SATURATION, 255U);
    }
  }
  
  for (uint8_t i = 0U; i < WIDTH / 2U; i++)
  {
    if (getPixColorXY(i, HEIGHT - 1U) == 0U &&
       (random(0, map(modes[EFF_SNOWSTORM].scale, 0U, 255U, 10U, 120U)) == 0U) &&
        getPixColorXY(i + 1U, HEIGHT - 1U) == 0U &&
        getPixColorXY(i - 1U, HEIGHT - 1U) == 0U)
    {
      leds[getPixelNumber(i, HEIGHT - 1U)] = CHSV(random(0, 200), SNOW_SATURATION, 255U);
    }
  }

  // сдвигаем по диагонали
  for (uint8_t y = 0U; y < HEIGHT - 1U; y++)
  {
    for (uint8_t x = WIDTH - 1U; x > 0U; x--)
    {
      drawPixelXY(x, y, getPixColorXY(x - 1U, y + 1U));
    }
  }

  // уменьшаем яркость левой и верхней линии, формируем "хвосты"
  for (uint8_t i = HEIGHT / 2U; i < HEIGHT; i++)
  {
    fadePixel(0U, i, SNOW_TAIL_STEP);
  }
  for (uint8_t i = 0U; i < WIDTH / 2U; i++)
  {
    fadePixel(i, HEIGHT - 1U, SNOW_TAIL_STEP);
  }
}

// ------------- звездопад -------------
#define STAR_DENSE            (60U)                         // плотность комет
#define STAR_TAIL_STEP        (100U)                        // длина хвоста кометы
#define STAR_SATURATION       (150U)                        // насыщенность кометы (от 0 до 255)

void starfallRoutine()
{
  if (loadingFlag)
  {
    loadingFlag = false;
    FastLED.clear();
  }
  
  // заполняем головами комет левую и верхнюю линию
  for (uint8_t i = HEIGHT / 2U; i < HEIGHT; i++)
  {
    if (getPixColorXY(0U, i) == 0U &&
       (random(0, STAR_DENSE) == 0) &&
        getPixColorXY(0U, i + 1U) == 0U &&
        getPixColorXY(0U, i - 1U) == 0U)
    {
      leds[getPixelNumber(0U, i)] = CHSV(random(0, 200), STAR_SATURATION, 255U);
    }
  }
  
  for (uint8_t i = 0U; i < WIDTH / 2U; i++)
  {
    if (getPixColorXY(i, HEIGHT - 1U) == 0U &&
       (random(0, map(modes[EFF_STARFALL].scale, 0U, 255U, 10U, 120U)) == 0U) &&
        getPixColorXY(i + 1U, HEIGHT - 1U) == 0U &&
        getPixColorXY(i - 1U, HEIGHT - 1U) == 0U)
    {
      leds[getPixelNumber(i, HEIGHT - 1U)] = CHSV(random(0, 200), STAR_SATURATION, 255U);
    }
  }

  // сдвигаем по диагонали
  for (uint8_t y = 0U; y < HEIGHT - 1U; y++)
  {
    for (uint8_t x = WIDTH - 1U; x > 0U; x--)
    {
      drawPixelXY(x, y, getPixColorXY(x - 1U, y + 1U));
    }
  }

  // уменьшаем яркость левой и верхней линии, формируем "хвосты"
  for (uint8_t i = HEIGHT / 2U; i < HEIGHT; i++)
  {
    fadePixel(0U, i, STAR_TAIL_STEP);
  }
  for (uint8_t i = 0U; i < WIDTH / 2U; i++)
  {
    fadePixel(i, HEIGHT - 1U, STAR_TAIL_STEP);
  }
}

// ------------- светлячки со шлейфом -------------
#define BALLS_AMOUNT          (3U)                          // количество "шариков"
#define CLEAR_PATH            (1U)                          // очищать путь
#define BALL_TRACK            (1U)                          // (0 / 1) - вкл/выкл следы шариков
#define TRACK_STEP            (70U)                         // длина хвоста шарика (чем больше цифра, тем хвост короче)
int16_t coord[BALLS_AMOUNT][2U];
int8_t vector[BALLS_AMOUNT][2U];
CRGB ballColors[BALLS_AMOUNT];

void ballsRoutine()
{
  if (loadingFlag)
  {
    loadingFlag = false;

    for (uint8_t j = 0U; j < BALLS_AMOUNT; j++)
    {
      int8_t sign;
      // забиваем случайными данными
      coord[j][0U] = WIDTH / 2 * 10;
      random(0, 2) ? sign = 1 : sign = -1;
      vector[j][0U] = random(4, 15) * sign;
      coord[j][1U] = HEIGHT / 2 * 10;
      random(0, 2) ? sign = 1 : sign = -1;
      vector[j][1U] = random(4, 15) * sign;
      ballColors[j] = CHSV(random(0, 9) * 28, 255U, 255U);
    }
  }

  if (!BALL_TRACK)                                          // режим без следов шариков
  {
    FastLED.clear();
  }
  else                                                      // режим со следами
  {
    fader(TRACK_STEP);
  }

  // движение шариков
  for (uint8_t j = 0U; j < BALLS_AMOUNT; j++)
  {
    // движение шариков
    for (uint8_t i = 0U; i < 2U; i++)
    {
      coord[j][i] += vector[j][i];
      if (coord[j][i] < 0)
      {
        coord[j][i] = 0;
        vector[j][i] = -vector[j][i];
      }
    }

    if (coord[j][0U] > (int16_t)((WIDTH - 1) * 10))
    {
      coord[j][0U] = (WIDTH - 1) * 10;
      vector[j][0U] = -vector[j][0U];
    }
    if (coord[j][1U] > (int16_t)((HEIGHT - 1) * 10))
    {
      coord[j][1U] = (HEIGHT - 1) * 10;
      vector[j][1U] = -vector[j][1U];
    }
    leds[getPixelNumber(coord[j][0U] / 10, coord[j][1U] / 10)] =  ballColors[j];
  }
}

// ------------- пейнтбол -------------
const uint8_t BorderWidth = 2U;

void lightBallsRoutine()
{
  // Apply some blurring to whatever's already on the matrix
  // Note that we never actually clear the matrix, we just constantly
  // blur it repeatedly.  Since the blurring is 'lossy', there's
  // an automatic trend toward black -- by design.
  uint8_t blurAmount = dim8_raw(beatsin8(3,64,100));
  blur2d(leds, WIDTH, HEIGHT, blurAmount);

  // Use two out-of-sync sine waves
  uint8_t  i = beatsin8(  91, BorderWidth, WIDTH-BorderWidth);
  uint8_t  j = beatsin8( 109, BorderWidth, WIDTH-BorderWidth);
  uint8_t  k = beatsin8(  73, BorderWidth, WIDTH-BorderWidth);
  uint8_t  m = beatsin8( 123, BorderWidth, WIDTH-BorderWidth);

  // The color of each point shifts over time, each at a different speed.
  uint16_t ms = millis();
  leds[XY( i, j)] += CHSV( ms / 29, 200U, 255U);
  leds[XY( j, k)] += CHSV( ms / 41, 200U, 255U);
  leds[XY( k, m)] += CHSV( ms / 73, 200U, 255U);
  leds[XY( m, i)] += CHSV( ms / 97, 200U, 255U);

}
// Trivial XY function for the SmartMatrix; use a different XY
// function for different matrix grids. See XYMatrix example for code.
uint16_t XY(uint8_t x, uint8_t y)
{
  uint16_t i;
  if (y & 0x01)
  {
    // Odd rows run backwards
    uint8_t reverseX = (WIDTH - 1) - x;
    i = (y * WIDTH) + reverseX;
  }
  else
  {
    // Even rows run forwards
    i = (y * WIDTH) + x;
  }
  return i;
}

// ------------- блуждающий кубик -------------
#define RANDOM_COLOR          (1U)                          // случайный цвет при отскоке
int16_t coordB[2U];
int8_t vectorB[2U];
CRGB ballColor;
int8_t ballSize;

void ballRoutine()
{
  if (loadingFlag)
  {
    loadingFlag = false;
    //FastLED.clear();

    for (uint8_t i = 0U; i < 2U; i++)
    {
      coordB[i] = WIDTH / 2 * 10;
      vectorB[i] = random(8, 20);
      ballColor = CHSV(random(0, 9) * 28, 255U, 255U);
    }
  }

  ballSize = map(modes[EFF_CUBE].scale, 0U, 255U, 2U, max((uint8_t)min(WIDTH,HEIGHT) / 3, 2));
  for (uint8_t i = 0U; i < 2U; i++)
  {
    coordB[i] += vectorB[i];
    if (coordB[i] < 0)
    {
      coordB[i] = 0;
      vectorB[i] = -vectorB[i];
      if (RANDOM_COLOR) ballColor = CHSV(random(0, 9) * 28, 255U, 255U);
      //vectorB[i] += random(0, 6) - 3;
    }
  }
  if (coordB[0U] > (int16_t)((WIDTH - ballSize) * 10))
  {
    coordB[0U] = (WIDTH - ballSize) * 10;
    vectorB[0U] = -vectorB[0U];
    if (RANDOM_COLOR) ballColor = CHSV(random(0, 9) * 28, 255U, 255U);
    //vectorB[0] += random(0, 6) - 3;
  }
  if (coordB[1U] > (int16_t)((HEIGHT - ballSize) * 10))
  {
    coordB[1U] = (HEIGHT - ballSize) * 10;
    vectorB[1U] = -vectorB[1U];
    if (RANDOM_COLOR)
    {
      ballColor = CHSV(random(0, 9) * 28, 255U, 255U);
    }
    //vectorB[1] += random(0, 6) - 3;
  }
  FastLED.clear();
  for (uint8_t i = 0U; i < ballSize; i++)
  {
    for (uint8_t j = 0U; j < ballSize; j++)
    {
      leds[getPixelNumber(coordB[0U] / 10 + i, coordB[1U] / 10 + j)] = ballColor;
    }
  }
}

/*
  void lightersRoutine() {
  if (loadingFlag) {
    loadingFlag = false;
    randomSeed(millis());
    for (byte i = 0; i < LIGHTERS_AM; i++) {
      lightersPos[0][i] = random(0, WIDTH * 10);
      lightersPos[1][i] = random(0, HEIGHT * 10);

      lightersColor[i] = CHSV(random(0, 255), 255, 255);

      speedV[i] = random(5, 10);
      angle[i] = random(0, 360);
      angleSpeed[i] = random(-10, 10);
    }
  }
  FastLED.clear();
  if (++loopCounter > 20) loopCounter = 0;

  for (byte i = 0; i < modes[17].scale; i++) {
    if (loopCounter == 0) {     // меняем скорость каждые 255 отрисовок
      angleSpeed[i] += random(-3, 4);
      angleSpeed[i] = constrain(angleSpeed[i], -15, 15);
    }

    lightersPos[0][i] += speedV[i] * cos(radians(angle[i]));
    lightersPos[1][i] += speedV[i] * sin(radians(angle[i]));

    if (lightersPos[0][i] < 0) lightersPos[0][i] = (WIDTH - 1) * 10;
    if (lightersPos[0][i] >= WIDTH * 10) lightersPos[0][i] = 0;

    if (lightersPos[1][i] < 0) {
      lightersPos[1][i] = 0;
      angle[i] = 360 - angle[i];
    } else {
      angle[i] += angleSpeed[i];
    }

    if (lightersPos[1][i] >= (HEIGHT - 1) * 10) {
      lightersPos[1][i] = (HEIGHT - 1) * 10;
      angle[i] = 360 - angle[i];
    } else {
      angle[i] += angleSpeed[i];
    }

    if (angle[i] > 360) angle[i] = 360 - angle[i];
    if (angle[i] < 0) angle[i] = 360 + angle[i];

    drawPixelXY(lightersPos[0][i] / 10, lightersPos[1][i] / 10, lightersColor[i]);
  }
  }
*/
