#include <Arduino.h>
#include <M5EPD.h>
#include <set>
#include <vector>

// Geometry constants
#define screenWidth 540
#define screenHeight 960
#define cellWidth 102
#define cellHeight 100
#define keyWidth 51
#define keyHeight 60
#define buttonHeight 72
#define margin 15

// Font constants
#define fontName "/font.ttf"
#define cellFontSize_TTF 54
#define keyFontSize_TTF 26

// Color constants
#define blackColor 15
#define grayColor 2
#define whiteColor 0

// Font size will be changed if external font loaded
int cellFontSize = 5;
int keyFontSize = 3;

// Canvas
M5EPD_Canvas screenCanvas(&M5.EPD);
M5EPD_Canvas buttonCanvas(&M5.EPD);
M5EPD_Canvas lineCanvas(&M5.EPD);
// M5EPD_Canvas keyCanvas(&M5.EPD);
M5EPD_Canvas keyboardCanvas(&M5.EPD);
tp_finger_t lastFingerItem;

// Valid word set will be loaded from SD card
std::set<String> wordSet;

// Valiables for game state
String answer = "PAPER";
String inputLine = "";
String hit = "";
String contained = "";
String notContained = "";

int lineIndex = -1;
char table[6][5];
char state[6][5];
boolean gameFinished = false;

// Keyboard tops
char keyLine1[] = "QWERTYUIOP";
char keyLine2[] = "ASDFGHJKL";
char keyLine3[] = "=ZXCVBNM <";

// Font geometry caches
std::map<String, int> widthForStringMap;
M5EPD_Canvas widthCanvas(&M5.EPD);
int _cellFontHeight;
int _keyFontHeight;

// Functions
// Key and Input
void keyPushed(int keyboardX, int keyboardY, char key);
void checkWordOnInputLine();
void addWordToTable(String line);
void updateInputLineArea();

// Drawing
void updateAllScreen();
void drawKeyboard();
void drawKey(char key, int x, int y);

// Utilities
int stringWidth(String string, int fontSize);
int cellFontHeight();
int keyFontHeight();
int batteryPercent();
void savePGM(M5EPD_Canvas targetCanvas);

// Load and save state
void loadState();
void saveState();
void loadWordList();
void startNewGame();

// Setup function to initialize
void setup()
{
  // Initialize M5Paper
  M5.begin();
  M5.EPD.SetRotation(90);
  M5.TP.SetRotation(90);
  M5.EPD.Clear(true);

  lastFingerItem.x = 0;
  lastFingerItem.y = 0;

  // Create canvases
  // canvas: main canvas for whole screen
  // lineCanvas: display input line
  // keyboardCanvas: display keyboard
  screenCanvas.createCanvas(screenWidth, screenHeight);
  buttonCanvas.createCanvas(cellWidth, buttonHeight);
  buttonCanvas.fillCanvas(blackColor);
  lineCanvas.createCanvas(cellWidth * 5, cellHeight);
  // keyCanvas.createCanvas(keyWidth, keyHeight);
  // keyCanvas.fillCanvas(blackColor);
  widthCanvas.createCanvas(cellWidth, cellHeight);

  // If font file exists in SD card, load the font
  if (SD.exists(fontName))
  {
    uint16_t fontCount = screenCanvas.fontsLoaded();
    screenCanvas.loadFont(fontName, SD);
    cellFontSize = cellFontSize_TTF;
    keyFontSize = keyFontSize_TTF;
    screenCanvas.createRender(cellFontSize, 32);
    screenCanvas.createRender(keyFontSize, 32);
  }

  screenCanvas.setTextColor(blackColor);
  lineCanvas.setTextColor(blackColor);
  lineCanvas.setTextSize(cellFontSize);

  keyboardCanvas.createCanvas(keyWidth * 10 + 1, keyHeight * 3 + 1);
  keyboardCanvas.setTextColor(blackColor);
  keyboardCanvas.setTextSize(keyFontSize);

  // Load valid words file fron words.txt in SD card
  loadWordList();

  // Load current game state from state.txt in SD card
  loadState();

  // Draw keyboard on keyboardCanvas
  drawKeyboard();

  // Draw all screen and display it
  updateAllScreen();
}

void loop()
{
  // Button detection
  M5.update();
  if (M5.BtnP.wasPressed())
  {
    savePGM(screenCanvas);
    delay(300);

    return;
  }

  // Touch detection
  if (M5.TP.avaliable())
  {
    if (!M5.TP.isFingerUp())
    {
      M5.TP.update();
      tp_finger_t fingerItem = M5.TP.readFinger(0);
      if (lastFingerItem.x == fingerItem.x && lastFingerItem.y == fingerItem.y)
      { // if touched positiion is same with last position, discard
        return;
      }
      lastFingerItem = fingerItem;

      // Button-touch detection
      if (fingerItem.y < margin + buttonHeight)
      {
        if (fingerItem.x < margin + cellWidth)
        {
          buttonCanvas.pushCanvas(margin, margin, UPDATE_MODE_DU);
          startNewGame();
          drawKeyboard();
          updateAllScreen();
        }
        else if (fingerItem.x > margin + cellWidth * 4)
        {
          buttonCanvas.pushCanvas(margin + cellWidth * 4, margin, UPDATE_MODE_DU);
          delay(500);
          M5.shutdown();
        }
      }

      // keyboard-touch detection
      if (gameFinished || lineIndex > 5)
        return;
      int keyboardY = margin + buttonHeight + margin + cellHeight * 6;
      if (fingerItem.y > keyboardY && fingerItem.y < (keyboardY + keyHeight))
      { // line 1
        for (int i = 0; i < 10; i++)
        {
          int keyboardX = i * keyWidth + margin;
          if (fingerItem.x > keyboardX && fingerItem.x < (keyboardX + keyWidth))
          {
            keyPushed(keyboardX, keyboardY, keyLine1[i]);
          }
        }
        return;
      }
      else if (fingerItem.y > (keyboardY + keyHeight) && fingerItem.y < (keyboardY + keyHeight * 2))
      { // line 2
        for (int i = 0; i < 9; i++)
        {
          int keyboardX = i * keyWidth + keyWidth / 2 + margin;
          if (fingerItem.x > keyboardX && fingerItem.x < (keyboardX + keyWidth))
          {
            keyPushed(keyboardX, keyboardY + keyHeight, keyLine2[i]);
          }
        }
        return;
      }
      else if (fingerItem.y > (keyboardY + keyHeight * 2) && fingerItem.y < (keyboardY + keyHeight * 3))
      { // line 3
        for (int i = 0; i < 10; i++)
        {
          int keyboardX = i * keyWidth + margin;
          if (fingerItem.x > keyboardX && fingerItem.x < (keyboardX + keyWidth))
          {
            keyPushed(keyboardX, keyboardY + keyHeight * 2, keyLine3[i]);
          }
        }
        return;
      }
    }
  }
}

// called when key-touch detected
void keyPushed(int keyboardX, int keyboardY, char key)
{
  Serial.println(key);
  //keyCanvas.pushCanvas(keyboardX, keyboardY, UPDATE_MODE_DU);
  keyboardCanvas.ReversePartColor(keyboardX - margin + 1, keyboardY - (margin + buttonHeight + margin + cellHeight * 6) + 1, keyWidth - 2, keyHeight - 2);
  keyboardCanvas.pushCanvas(margin, margin + buttonHeight + margin + cellHeight * 6, UPDATE_MODE_NONE);
  M5.EPD.UpdateArea(keyboardX + 1, keyboardY + 1, keyWidth - 2, keyHeight - 2, UPDATE_MODE_DU);


  delay(200);
  keyboardCanvas.ReversePartColor(keyboardX - margin + 1, keyboardY - (margin + buttonHeight + margin + cellHeight * 6) + 1, keyWidth - 2, keyHeight - 2);
  keyboardCanvas.pushCanvas(margin, margin + buttonHeight + margin + cellHeight * 6, UPDATE_MODE_NONE);
  M5.EPD.UpdateArea(keyboardX + 1, keyboardY + 1, keyWidth - 2, keyHeight - 2, UPDATE_MODE_DU4);

  if (key == '=')
  { // if 5 letters in input line, check if the word exists
    if (inputLine.length() >= 5)
    {
      checkWordOnInputLine();
    }
  }
  else if (key == '<')
  { // Backspace
    if (inputLine.length() >= 1)
    {
      if (inputLine.length() == 1)
      {
        inputLine = "";
      }
      else if (inputLine.length() > 1)
      {
        inputLine = inputLine.substring(0, inputLine.length() - 1);
      }
      updateInputLineArea();
    }
  }
  else if (key != ' ' && inputLine.length() < 5)
  { // Other alphabet
    inputLine += String(key);
    updateInputLineArea();
  }
}

// check if the word exists and put it into last line
void checkWordOnInputLine()
{
  if (lineIndex > 5)
    return;
  if (inputLine.length() == 5)
  {
    if (wordSet.count(inputLine) > 0)
    { // word exists in word list. valid input
      Serial.println("found in word list");
      addWordToTable(inputLine);
      saveState();
      drawKeyboard();
      updateAllScreen();
      inputLine = "";
    }
    else
    { // word not found in word list. clear input
      inputLine = "";
      updateInputLineArea();
    }
  }
}

// check if the word contains answer and update status
void addWordToTable(String line)
{
  if (lineIndex > 5)
  {
    // Already last line. can't add word
    return;
  }
  if (line.length() == 5)
  {
    for (int i = 0; i < 5; i++)
    {
      table[lineIndex][i] = line[i];
      if (line[i] == answer[i])
      { // Hit: correct char and correct position
        state[lineIndex][i] = 3;
        hit += String(line[i]);
      }
      else if (answer.indexOf(line[i]) > -1)
      { // Contained: the answer contains char but not correct position
        state[lineIndex][i] = 2;
        contained += String(line[i]);
      }
      else
      { // Not Contained: the answer doesn't contain char
        state[lineIndex][i] = 1;
        notContained += String(line[i]);
      }
    }
    lineIndex++;
  }
}

// Update input line on screen when keys are typed
void updateInputLineArea()
{
  lineCanvas.fillCanvas(0);
  int y = 0;
  for (int i = 0; i < 5; i++)
  {
    int x = i * cellWidth;
    lineCanvas.drawRect(x, y, cellWidth + 1, cellHeight + 1, blackColor);
    if (i < inputLine.length())
    {
      String oneChar = inputLine.substring(i, i + 1);
      lineCanvas.drawString(oneChar, x + (cellWidth - stringWidth(oneChar, cellFontSize)) / 2, y + (cellHeight - cellFontHeight()) / 2);
    }
  }
  lineCanvas.pushCanvas(margin, margin + buttonHeight + margin + cellHeight * lineIndex, UPDATE_MODE_DU4);
}

// Update all screen with current status
void updateAllScreen()
{
  // prevent ghost of e-ink
  screenCanvas.fillCanvas(whiteColor);
  screenCanvas.pushCanvas(0, 0, UPDATE_MODE_A2);

  // draw top buttons
  // NEW button
  screenCanvas.setTextSize(keyFontSize);
  screenCanvas.drawRect(margin, margin, cellWidth + 1, buttonHeight, blackColor);
  screenCanvas.drawString("NEW", margin + (cellWidth - stringWidth("NEW", keyFontSize)) / 2, margin + (buttonHeight - keyFontHeight()) / 2);

  // Counter area
  String statusString = String(lineIndex) + "/6";
  screenCanvas.drawString(statusString, margin + cellWidth * 2 + (cellWidth - stringWidth(statusString, keyFontSize)) / 2, margin + (buttonHeight - keyFontHeight()) / 2);

  // Battery area
  String batteryString = String(batteryPercent()) + "%";
  screenCanvas.drawString(batteryString, margin + cellWidth * 3 + (cellWidth - stringWidth(batteryString, keyFontSize)) / 2, margin + (buttonHeight - keyFontHeight()) / 2);

  // OFF button
  screenCanvas.drawRect(margin + cellWidth * 4, margin, cellWidth, buttonHeight, blackColor);
  screenCanvas.drawString("OFF", margin + cellWidth * 4 + (cellWidth - stringWidth("OFF", keyFontSize)) / 2, margin + (buttonHeight - keyFontHeight()) / 2);

  int hitCount = 0; // counter for hit charactors

  // draw main table lines
  screenCanvas.drawFastHLine(margin, margin * 2 + buttonHeight, cellWidth * 5, blackColor);
  screenCanvas.drawFastHLine(margin, margin * 2 + buttonHeight + cellHeight, cellWidth * 5, blackColor);
  screenCanvas.drawFastHLine(margin, margin * 2 + buttonHeight + cellHeight * 2, cellWidth * 5, blackColor);
  screenCanvas.drawFastHLine(margin, margin * 2 + buttonHeight + cellHeight * 3, cellWidth * 5, blackColor);
  screenCanvas.drawFastHLine(margin, margin * 2 + buttonHeight + cellHeight * 4, cellWidth * 5, blackColor);
  screenCanvas.drawFastHLine(margin, margin * 2 + buttonHeight + cellHeight * 5, cellWidth * 5, blackColor);
  screenCanvas.drawFastHLine(margin, margin * 2 + buttonHeight + cellHeight * 6, cellWidth * 5, blackColor);

  screenCanvas.drawFastVLine(margin, margin * 2 + buttonHeight, cellHeight * 6, blackColor);
  screenCanvas.drawFastVLine(margin + cellWidth, margin * 2 + buttonHeight, cellHeight * 6, blackColor);
  screenCanvas.drawFastVLine(margin + cellWidth * 2, margin * 2 + buttonHeight, cellHeight * 6, blackColor);
  screenCanvas.drawFastVLine(margin + cellWidth * 3, margin * 2 + buttonHeight, cellHeight * 6, blackColor);
  screenCanvas.drawFastVLine(margin + cellWidth * 4, margin * 2 + buttonHeight, cellHeight * 6, blackColor);
  screenCanvas.drawFastVLine(margin + cellWidth * 5, margin * 2 + buttonHeight, cellHeight * 6, blackColor);

  screenCanvas.setTextSize(cellFontSize);
  for (int row = 0; row < lineIndex; row++)
  {
    hitCount = 0;
    for (int i = 0; i < 5; i++)
    {
      int x = margin + i * cellWidth;
      int y = margin + buttonHeight + margin + cellHeight * row;

      //screenCanvas.drawRect(x, y, cellWidth + 1, cellHeight + 1, blackColor);

      // if (row < lineIndex)
      // {
        // draw state marker
        switch (state[row][i])
        {
        case 3: // Circle = Hit: correct char and correct position
          hitCount += 1;
          screenCanvas.fillCircle(x + cellWidth / 2, y + cellHeight / 2, cellHeight / 2 - 10, grayColor);
          break;

        case 2: // Triangle = Contained: the answer contains the char
          screenCanvas.fillTriangle(x + cellWidth / 2, y + 8, x + 8, y + cellHeight - 16, x + cellWidth - 8, y + cellHeight - 16, grayColor);
          break;

        case 1: // None = Not Contained: the answer doesn't contain the char
          break;

        default:
          break;
        }

        // Draw single char
        char oneChar[2] = "\0";
        oneChar[0] = table[row][i];
        screenCanvas.drawString(oneChar, x + (cellWidth - stringWidth(oneChar, cellFontSize)) / 2, y + (cellHeight - cellFontHeight()) / 2);
      // }
    }
  }

  // Draw keyboard
  int y = margin + buttonHeight + margin + cellHeight * 6;
  keyboardCanvas.pushToCanvas(margin, y, &screenCanvas);

  // Draw message
  y += keyHeight * 3 + margin;
  screenCanvas.setTextSize(keyFontSize);
  if (hitCount == 5)
  { // Correct answer
    gameFinished = true;
    screenCanvas.drawString("Correct!", margin, y);
  }
  else if (lineIndex > 5)
  { // Failed 6 times
    gameFinished = true;
    screenCanvas.drawString("Failed! It was " + answer, margin, y);
  }

  // Push canvas to update all screen
  screenCanvas.pushCanvas(0, 0, UPDATE_MODE_GL16);
}


// Draw keyboard on keyboardCanvas without update all screen (Fast and low quality)
void drawKeyboard()
{
  keyboardCanvas.fillCanvas(whiteColor);

  int y = 0;
  // Keyboard line 1
  for (int i = 0; i < 10; i++)
  {
    int x = i * keyWidth;
    drawKey(keyLine1[i], x, y);
  }

  // Keyboard line 2
  y += keyHeight;
  for (int i = 0; i < 9; i++)
  {
    int x = keyWidth / 2 + i * keyWidth;
    drawKey(keyLine2[i], x, y);
  }

  // Keyboard line 3
  y += keyHeight;
  for (int i = 0; i < 10; i++)
  {
    int x = i * keyWidth;
    drawKey(keyLine3[i], x, y);
  }
}

// Draw single key with char and state marker
void drawKey(char key, int x, int y)
{
  keyboardCanvas.drawRect(x, y, keyWidth + 1, keyHeight + 1, blackColor);
  if (hit.indexOf(key) > -1)
  { // Circle: this key hit the answer word. it was correct char and correct position
    keyboardCanvas.drawCircle(x + keyWidth / 2, y + keyHeight / 2, keyWidth / 2 - 8, blackColor);
  }
  else if (contained.indexOf(key) > -1)
  { // Triangle: this key is contained in the answer word
    keyboardCanvas.drawTriangle(x + keyWidth / 2, y + 8, x + 8, y + keyWidth - 8, x + keyWidth - 8, y + keyWidth - 8, blackColor);
  }
  else if (notContained.indexOf(key) > -1)
  { // Strikethrough: this key is used but not contained
    keyboardCanvas.drawFastHLine(x + 8, y + keyHeight / 2, keyWidth - 16, blackColor);
  }
  keyboardCanvas.drawString(String(key), x + (keyWidth - stringWidth(String(key), keyFontSize)) / 2, y + (keyHeight - keyFontHeight()) / 2);
}

// Return width of string using font size
int stringWidth(String string, int fontSize)
{
  String key = String(fontSize) + string;
  if (widthForStringMap.count(key) > 0)
  { // Width value found in cache map
    return widthForStringMap.at(key);
  }
  else
  { // Not in cache map. Print char to test width
    widthCanvas.setCursor(0, 0);
    widthCanvas.setTextSize(fontSize);
    widthCanvas.print(string);
    int width = widthCanvas.getCursorX();
    widthForStringMap.insert(std::make_pair(key, width));
    return width;
  }
  return fontSize;
}

// Return font height using font size
int cellFontHeight()
{
  if (_cellFontHeight > 0) 
  { // Height is in cache
    return _cellFontHeight;
  }
  // Print to estimate height
  widthCanvas.setCursor(0, 0);
  widthCanvas.setTextSize(cellFontSize);
  widthCanvas.print("A\n");
  _cellFontHeight = widthCanvas.getCursorY() * 2 / 3;
  return _cellFontHeight;
}
int keyFontHeight()
{
  if (_keyFontHeight > 0) 
  { // Height is in cache
    return _keyFontHeight;
  }
  // Print to estimate height
  widthCanvas.setCursor(0, 0);
  widthCanvas.setTextSize(keyFontSize);
  widthCanvas.print("A\n");
  _keyFontHeight = widthCanvas.getCursorY() * 2 / 3;
  return _keyFontHeight;
}

// Return battery 0 - 100 (3.2 V - 4.25 V)
int batteryPercent()
{
  float voltage = M5.getBatteryVoltage() / 1000.0;
  return (int)(((voltage - 3.2) / (4.25 - 3.2)) * 100.0);
}

// Save canvas content as PGM file
void savePGM(M5EPD_Canvas canvas)
{
  uint32_t bufferSize = canvas.getBufferSize();
  uint8_t *buffer = (uint8_t *)(canvas.frameBuffer(1));
  int width = canvas.width();
  int height = canvas.height();
  Serial.println(String(width) + " x " + String(height) + " = " + String(width * height) + ", bufferSize = " + String(bufferSize));

  // Open file
  int fileIndex = 1;
  while (SD.exists("/ss" + String(fileIndex) + ".pgm"))
  {
    fileIndex++;
  }
  String fileName = "/ss" + String(fileIndex) + ".pgm";
  Serial.println("File name = " + fileName);
  File pgmFile = SD.open(fileName, FILE_WRITE);
  if (!pgmFile)
    return;

  // Write PGM header
  pgmFile.print("P5 ");
  pgmFile.print(String(width) + " ");
  pgmFile.print(String(height) + " ");
  pgmFile.print("255 ");

  // Write PGM bytes
  for (uint32_t i = 0; i < (bufferSize); i++)
  {
    // Convert 4 bit gray to 8 bit gray
    uint8_t byte = buffer[i];
    uint8_t high4bit = 17 * (15 - (byte >> 4));
    uint8_t low4bit = 17 * (15 - ((B00001111 & byte)));
    pgmFile.write(high4bit);
    pgmFile.write(low4bit);
  }
  pgmFile.close();
  Serial.println("File wrote: " + fileName);
}

// Load current answer, previous inputs from state.txt in SD card
void loadState()
{
  if (SD.exists("/state.txt"))
  {
    File stateFile = SD.open("/state.txt");
    if (stateFile)
    {
      lineIndex = -1;
      while (stateFile.available() > 0)
      {
        String line = stateFile.readStringUntil('\n');
        if (lineIndex == -1)
        { // First line is current answer
          answer = line;
          lineIndex = 0;
        }
        else if (lineIndex < 6)
        { // Put other 6 lines into table
          if (line.length() >= 5)
          {
            addWordToTable(line);
          }
          else
          {
            break;
          }
        }
      }
    }
    stateFile.close();
  }
  else
  { // if state.txt not found, start new game
    startNewGame();
  }
}

// Save current answer, current inputs as state.txt in SD card
void saveState()
{
  // Clear old state.txt
  SD.remove("/state.txt");
  File stateFile = SD.open("/state.txt", FILE_WRITE);

  if (!stateFile)
    return;

  // First line is current answer
  stateFile.print(answer);
  stateFile.print('\n');

  // Other lines are table content
  for (int row = 0; row < 6; row++)
  {
    if (table[row][0] == 0)
      break;
    for (int i = 0; i < 5; i++)
    {
      if (table[row][i] > 0)
        stateFile.print(table[row][i]);
      else
        break;
    }
    stateFile.print('\n');
  }
  stateFile.close();
}

// Load word list from "words.txt" in SD card
void loadWordList()
{
  File wordFile = SD.open("/words.txt");
  if (wordFile)
  {
    while (wordFile.available() > 0)
    {
      String line = wordFile.readStringUntil('\n');
      if (line.length() == 5)
      {
        line.toUpperCase();
        wordSet.insert(line);
      }
    }
  }
  wordFile.close();
}

// Start new game
void startNewGame()
{
  // Clear variables
  memset(table, 0, sizeof(table));
  memset(state, 0, sizeof(state));
  hit = "";
  contained = "";
  notContained = "";
  inputLine = "";
  lineIndex = 0;
  gameFinished = false;

  //load all words from "words.txt" in SD card
  if (SD.exists("/words.txt"))
  { 
    File wordFile = SD.open("/words.txt");
    if (wordFile)
    {
      std::vector<String> allWords;

      while (wordFile.available() > 0)
      {
        String line = wordFile.readStringUntil('\n');
        if (line.length() == 5)
        {
          line.toUpperCase();
          allWords.push_back(line);
        }
      }

      //Set answer randomly
      srand(millis());
      answer = allWords[rand() % allWords.size()];
      Serial.println(answer);
    }
    wordFile.close();
  }
  else
  {
    // No words.txt in SD card
    answer = ".....";

    addWordToTable("PLACE");
    addWordToTable("WORDS");
    addWordToTable("FILE ");
    addWordToTable("ON SD");
  }
}