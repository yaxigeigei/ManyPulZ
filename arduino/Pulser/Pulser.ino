#include <String.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>


// Device Status
volatile boolean standby = true;


// Channels
byte chanPins[] = { 12, 11, 2 };
volatile boolean chanFiring[] = { false, false, false };
long pulDur[] = { 0, 0, 0 };
long cycDur[] = { 0, 0, 0 };
volatile unsigned int pulComplete[] = { 0, 0, 0 };
volatile unsigned int cycComplete[] = { 0, 0, 0 };
volatile long timeCount = 0;
volatile long nextHigh[] = { 0, 0, 0 };
volatile long nextLow[] = { 0, 0, 0 };


// LCD Uses Pin 4, 5, 6, 7, 8(rs), 9(enable), 10(backlight), 13(rw)
LiquidCrystal lcd(8, 13, 9, 4, 5, 6, 7);
byte backLightPin = 10;
byte itemSelect = 0;
unsigned int numChangeFactor = 1;


// Information
char msgs[10][17] = { "ManyPulZ 2.0    ",    // 0
                      "Channel to Set  ",    // 1
                      "Channel to Use  ",    // 2
                      "Pulse Interval  ",    // 3
                      "Pulse Number    ",    // 4
                      "Pulse Duration  ",    // 5
                      "Cycle Number    ",    // 6
                      "Cycle Interval  ",    // 7
                      "Initial Delay   ",    // 8
                      "Save & Load     " };  // 9

unsigned int paraMatrix[10][3] = { 0, 0, 0,    // 0
                                   0, 0, 0,    // 1[0] Channel to Set
                                   0, 0, 0,    // 2[0] Channel to Use
                                   99, 99, 99, // 3 Pulse Interval
                                   1, 1, 1,    // 4 Pulse Number
                                   1, 1, 1,    // 5 Pulse Duration
                                   1, 1, 1,    // 6 Cycle Number
                                   0, 0, 0,    // 7 Cycle Interval
                                   0, 0, 0,    // 8 Initial Delay
                                   0, 0, 0 };  // 9[0] Save or Load

unsigned int* chanSet = &paraMatrix[1][0];
unsigned int* comboSelect = &paraMatrix[2][0];
unsigned int* saveLoadSelect = &paraMatrix[9][0];


char units[10][3] = { "  ",    // 0
                      "  ",    // 1
                      "  ",    // 2
                      "ms",    // 3
                      "  ",    // 4
                      "ms",    // 5
                      "  ",    // 6
                      "ms",    // 7
                      "ms",    // 8
                      "  " };  // 9

char chanCombo[8][10] = { "         ",    // 0 => B000
                          "0        ",    // 1 => B001
                          "1        ",    // 2 => B010
                          "0 & 1    ",    // 3 => B011
                          "2        ",    // 4 => B100
                          "0 & 2    ",    // 5 => B101
                          "1 & 2    ",    // 6 => B110
                          "0 & 1 & 2" };  // 7 => B111

char saveLoad[2][17] = { "Save Settings   ",    // 0
                         "Load Settings   " };  // 1


void setup()
{
  Serial.begin(115200);
  
  for (int i = 0; i < 3; i++)
    pinMode(chanPins[i], OUTPUT);
  pinMode(backLightPin, OUTPUT);
  
  digitalWrite(backLightPin, HIGH);
  lcd.clear();
  lcd.begin(16, 2);
  lcd.setCursor(0,0);
  lcd.print("ManyPulZ 2.0    ");
  lcd.setCursor(0,1);
  lcd.print("Ready           ");
  
  // Configure Pulser Timer (Every 1ms)
  // Clear all existing settings
  cli();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  // Turn on CTC mode with OCR1A set as TOP value
  TCCR1B |= (1 << WGM12);
  // Set prescaler to 1
  TCCR1B |= (1 << CS10);
  // Interrupt Point
  OCR1A = 16000;
//  // Enable OCR1A CTC Interrupt
  sei();
  
  byte* pmPointer;
  pmPointer = (byte*)paraMatrix;
  
  for (int i = 0; i < sizeof(paraMatrix); i++)
  {
    *pmPointer = EEPROM.read(i);
    pmPointer++;
  }
}


void loop()
{
  ReadSerialCommand();
  if (standby)
    ReadButton();
}


void Fire()
{
  if (standby)
  {
    standby = false;
    
    byte chanIndex[] = { B001, B010, B100 };
    
    for (int i = 0; i < 3; i++)
    {
      if (*comboSelect & chanIndex[i])
        chanFiring[i] = true;
      
      nextHigh[i] = (long)paraMatrix[8][i];
      nextLow[i] = nextHigh[i] + (long)paraMatrix[3][i];
      pulComplete[i] = 0;
      cycComplete[i] = 0;
      pulDur[i] = (long)paraMatrix[3][i] + (long)paraMatrix[5][i];
      cycDur[i] = pulDur[i] * (long)paraMatrix[4][i] + (long)paraMatrix[7][i];
    }
    
    cli();
    timeCount = 0;
    TCNT1 = 0;
    TIMSK1 |= (1 << OCIE1A);
    sei();
  }
}



ISR(TIMER1_COMPA_vect)
{
  for (int i = 0; i <3; i++)
  {
    if (chanFiring[i])
    {
      if (nextHigh[i] == timeCount)
      {
        digitalWrite(chanPins[i], HIGH);
//        Serial.println(timeCount);
      }
      if (nextLow[i] == timeCount)
      {
        digitalWrite(chanPins[i], LOW);
        pulComplete[i]++;
//        Serial.println(timeCount);
      }
      
      if (cycComplete[i] < paraMatrix[6][i]) // Check Completed Cycle Number
      {
        if (pulComplete[i] < paraMatrix[4][i]) // Check Completed Pulse Number
        {
          nextHigh[i] = (long)paraMatrix[8][i] + (long)cycComplete[i] * cycDur[i] + (long)pulComplete[i] * pulDur[i];
          nextLow[i] = nextHigh[i] + (long)paraMatrix[5][i];
        }
        else
        {
          pulComplete[i] = 0;
          cycComplete[i]++;
//          Serial.println(cycComplete[i]);
        }
      }
      if (nextLow[i] < timeCount)
        chanFiring[i] = false;
    }
  }
  
  if (!(chanFiring[0] | chanFiring[1] | chanFiring[2]))
  {
    TIMSK1 = 0;
    standby = true;
    lcd.setCursor(0, 1);
    lcd.print("Ready           ");
  }
  
  timeCount++;
}




void SaveOrLoad()
{
  byte* pmPointer;
  pmPointer = (byte*)paraMatrix;
  
  if (*saveLoadSelect == 0)
  {
    for (int i = 0; i < sizeof(paraMatrix); i++)
    {
      EEPROM.write(i, *pmPointer);
      pmPointer++;
    }
    lcd.setCursor(0, 1);
    lcd.print("All Saved        ");
  }
  else
  {
    for (int i = 0; i < sizeof(paraMatrix); i++)
    {
      *pmPointer = EEPROM.read(i);
      pmPointer++;
    }
    lcd.setCursor(0, 1);
    lcd.print("All Loaded       ");
  }
  
  delay(1000);
  
}
