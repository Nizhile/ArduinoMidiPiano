//#define DEBUG

#define NUM_ROWS 6
#define NUM_COLS 9

#define NOTE_ON_CMD 0x90
#define NOTE_OFF_CMD 0x80
#define NOTE_VELOCITY 127

// C2
#define FIRST_NOTE 36
// C6
#define LAST_NOTE 84

#define NOTES_RANGE (LAST_NOTE - FIRST_NOTE + 1)

//MIDI baud rate replaced by serial/USB
#define SERIAL_RATE 115200

// Pin Definitions

// Row input pins
const int row1Pin = 2;
const int row2Pin = 3;
const int row3Pin = 4;
const int row4Pin = 5;
const int row5Pin = 6;
const int row6Pin = 7;

// 74HC595 pins
//const int dataPin = 8;
//const int latchPin = 9;
//const int clockPin = 10;

int columns[NUM_COLS] = { 8, 9, 10, 11, 12, A3, A2, A1, A0};

boolean keyPressed[NUM_ROWS][NUM_COLS];
uint8_t keyToMidiMap[NUM_ROWS][NUM_COLS];

// bitmasks for scanning columns
int bits[] =
{ 
  B00000001,
  B00000010,
  B00000100,
  B00001000,
  B00010000,
  B00100000,
  B01000000,
  B10000000
};

void setup()
{
  int note = FIRST_NOTE;

  for(int colCtr = 0; colCtr < NUM_COLS; ++colCtr)
  {
    for(int rowCtr = 0; rowCtr < NUM_ROWS; ++rowCtr)
    {
      keyPressed[rowCtr][colCtr] = false;
      keyToMidiMap[rowCtr][colCtr] = note;
      note++;
    }
  }

  // setup pins output/input mode
//  pinMode(dataPin, OUTPUT);
//  pinMode(clockPin, OUTPUT);
//  pinMode(latchPin, OUTPUT);
  for (int i = 0; i < NUM_COLS; i++) {
    pinMode(columns[i], OUTPUT);
    digitalWrite(columns[i], LOW);
  }

  pinMode(row1Pin, INPUT);
  pinMode(row2Pin, INPUT);
  pinMode(row3Pin, INPUT);
  pinMode(row4Pin, INPUT);
  pinMode(row5Pin, INPUT);
  pinMode(row6Pin, INPUT);

  Serial.begin(SERIAL_RATE);

#ifdef DEBUG
  // checking pulldown
  int rowValue[NUM_ROWS];
  rowValue[0] = digitalRead(row1Pin);
  rowValue[1] = digitalRead(row2Pin);
  rowValue[2] = digitalRead(row3Pin);
  rowValue[3] = digitalRead(row4Pin);
  rowValue[4] = digitalRead(row5Pin);
  rowValue[5] = digitalRead(row6Pin);
  Serial.print("Pull down: ");
  for (int k = NUM_ROWS -1 ; k >= 0; k--) {
    Serial.print(rowValue[k]);
  }
  Serial.println("");
  // checking mapping
  for(int colCtr = 0; colCtr < NUM_COLS; ++colCtr)
  {
    for(int rowCtr = 0; rowCtr < NUM_ROWS; ++rowCtr)
    {
    Serial.print(" ");
    Serial.print(colCtr);
    Serial.print(".");
    Serial.print(rowCtr);
    Serial.print(" n");
    Serial.print(keyToMidiMap[rowCtr][colCtr]);
    }
  }
  Serial.println("");
#endif
}

void loop()
{
  for (int colCtr = 0; colCtr < NUM_COLS; ++colCtr)
  {
    //scan next column
    scanColumn(colCtr);

    //get row values at this column
    int rowValue[NUM_ROWS];
    rowValue[0] = digitalRead(row1Pin);
    rowValue[1] = digitalRead(row2Pin);
    rowValue[2] = digitalRead(row3Pin);
    rowValue[3] = digitalRead(row4Pin);
    rowValue[4] = digitalRead(row5Pin);
    rowValue[5] = digitalRead(row6Pin);

    // process keys pressed
    for(int rowCtr=0; rowCtr<NUM_ROWS; ++rowCtr)
    {
      if(rowValue[rowCtr] != 0 && !keyPressed[rowCtr][colCtr])
      {
        keyPressed[rowCtr][colCtr] = true;
        noteOn(rowCtr,colCtr);
      }
    }

    // process keys released
    for(int rowCtr=0; rowCtr<NUM_ROWS; ++rowCtr)
    {
      if(rowValue[rowCtr] == 0 && keyPressed[rowCtr][colCtr])
      {
        keyPressed[rowCtr][colCtr] = false;
        noteOff(rowCtr,colCtr);
      }
    }
  }
}

#ifdef USE_74HC595
void scanColumn(int colNum)
{
  digitalWrite(latchPin, LOW);

  if(0 <= colNum && colNum <= 7)
  {
    shiftOut(dataPin, clockPin, MSBFIRST, B00000000); //right sr
    shiftOut(dataPin, clockPin, MSBFIRST, bits[colNum]); //left sr
  }
  else
  {
    shiftOut(dataPin, clockPin, MSBFIRST, bits[colNum-8]); //right sr
    shiftOut(dataPin, clockPin, MSBFIRST, B00000000); //left sr
  }
  digitalWrite(latchPin, HIGH);
}
#else
void scanColumn(int colNum)
{
  int previousCol = colNum - 1;
  if (previousCol < 0) previousCol = NUM_COLS - 1;
  digitalWrite(columns[previousCol], LOW);
  digitalWrite(columns[colNum], HIGH);
}
#endif
#ifndef DEBUG
void noteOn(int row, int col)
{
  Serial.write(NOTE_ON_CMD);
  Serial.write(keyToMidiMap[row][col]);
  Serial.write(NOTE_VELOCITY);
}

void noteOff(int row, int col)
{
  Serial.write(NOTE_OFF_CMD);
  Serial.write(keyToMidiMap[row][col]);
  Serial.write(NOTE_VELOCITY);
}
#else
void noteOn(int row, int col)
{
  Serial.print("0x");
  Serial.print(NOTE_ON_CMD, HEX);
  Serial.print(" n");
  Serial.print(keyToMidiMap[row][col]);
  Serial.print(" v");
  Serial.print(NOTE_VELOCITY);
  Serial.println("");
}

void noteOff(int row, int col)
{
  Serial.print("0x");
  Serial.print(NOTE_OFF_CMD, HEX);
  Serial.print(" n");
  Serial.print(keyToMidiMap[row][col]);
  Serial.print(" v");
  Serial.print(NOTE_VELOCITY);
  Serial.println("");
}
#endif
