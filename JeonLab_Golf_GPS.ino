/*  Golf GPS
  - GPS: PA6C
  - microcontroller: ATmega328p with Arduino bootloader, 8MHz internal oscillator
  - LCD: OLED 128x64 i2c 
    - library: http://www.rinkydinkelectronics.com/library.php?id=79
    - fonts: modified fonts (original from rinkydinkelectronics)
  - distance calculation: my formula

  Programmed by: Jinseok Jeon (JeonLab.wordpress.com)
  Created: Nov. 2017
  Revised: Mar. 2018
*/

// ----- course data ----- //
int num_courses = 4; //actual number of courses - 1

const String courseName[] PROGMEM = {
  "course name 0", // 0
  "course name 1", //1
  "course name 2", //2
  "course name 3", //3
  "course name 4", //4
};

const String selectMode[] PROGMEM = {
  "Shot -1", // 0
  "Play", // 1
  "Manual", //2
  "Course", // 3
  "Drive" // 4
};

const PROGMEM float coord[][2][19] = { //coord[course index][0:lat, 1:long][hole #, 0:office]
                                       //first coordinate is not used in order to match the index to hole number
  // latitude for course 0: 
  { { 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 
      00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000
    }, //longitude for course 0:
    { 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 
      00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000
    }
  },
  // latitude for course 1:
  { { 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 
      00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000
    }, //longitude for course 1:
    { 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 
      00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000
    }
  },
  // latitude for course 2: 
  { { 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 
      00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000
    }, //longitude for course 2:
    { 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 
      00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000
    }
  },
  // latitude for course 3: 
  { { 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 
      00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000
    }, //longitude for course 3:
    { 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 
      00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000
    }
  },
  // latitude for course 4: 
  { { 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 
      00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000
    }, //longitude for course 4:
    { 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 
      00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000, 00.000000
    }
  }
};
// ----- end of course data -----//

#include <OLED_I2C.h>
OLED  myOLED(SDA, SCL);
extern uint8_t franklingothic_12x16[];
extern uint8_t Ubuntu_Num_24x32[];

/* You need to modify SoftwareSerial.h to use abs
   - file location: arduino-1.8.5/hardware/arduino/avr/librariesSoftwareSerial/src
   - comment #undef abs
*/
#include <SoftwareSerial.h>
SoftwareSerial gps(7, -1); // RX, TX
const int MSG_LENGTH = 57;
const int TimeZone = -5; //EST, change this for your timezone (UTC +/- xx)
int DSTbegin[] = { //DST 2013 - 2025 in Canada and US
  310, 309, 308, 313, 312, 311, 310, 308, 314, 313, 312, 310, 309
};
int DSTend[] = { //DST 2013 - 2025 in Canada and US
  1103, 1102, 1101, 1106, 1105, 1104, 1103, 1101, 1107, 1106, 1105, 1103, 1102
};
int DaysAMonth[] = { //number of days a month
  31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

int gpsYear;
int gpsMonth;
int gpsDay;
int gpsHour;
byte gpsMin;
byte gpsSec;

float gpsLat;
float gpsLong;
float gpsSpeed; //km/h
float gpsBearing; //deg
float gpsAlt; //altitude [m]
float shotLat;
float shotLong;

const byte GPS_fix_pin = 8;
const byte LED_pin = 6;
const byte switch1_pin = 3;
const byte switch2_pin = 4;
const byte buzz_pin = 2;

char dataValid;
int courseIndex = 0;
int currentHole;
int mode = 4;
int totalShot = 0;
int currentShot = 0;
int distanceToGreen;
boolean courseSelected = false;
boolean lowBat = false;
boolean onGreen = false;
boolean startBack = false;
const int inGreenRadius = 20;
const int outGreenRadius = 30;

void setup()
{
  pinMode(GPS_fix_pin, INPUT); //check FIX
  pinMode(switch1_pin, INPUT_PULLUP); // switch 1
  pinMode(switch2_pin, INPUT_PULLUP); // switch 2
  pinMode(LED_pin, OUTPUT); //LED
  pinMode(buzz_pin, OUTPUT); //buzzer
  digitalWrite(LED_pin, LOW);
  digitalWrite(buzz_pin, LOW);
  gps.begin(9600);
  myOLED.begin();
  myOLED.setBrightness(0);
  myOLED.setFont(franklingothic_12x16);
  myOLED.print("Golf GPS", LEFT, 0);
  myOLED.print("JeonLab", RIGHT, 48);
  myOLED.update();
  delay(5000);
  myOLED.print("Starting..", CENTER, 24);
  while (!gpsFixChk())
  {
    digitalWrite(LED_pin, digitalRead(GPS_fix_pin));
  }
  myOLED.clrScr();
  myOLED.update();
}

void loop()
{
  switch (mode)
  {
    case 0: // shot-1
      if (!courseSelected)
      {
        mode = 3;
      }
      else
      {
        totalShot--;
        if (totalShot < 0) totalShot = 0;
        currentShot--;
        if (currentShot < 0) currentShot = 0;
        mode = 1;
      }
      break;
    case 1: //play golf
      if (!courseSelected)
      {
        mode = 3;
      }
      else
        golfPlay();
      break;
    case 2: //Manual hole change
      if (!courseSelected)
      {
        mode = 3;
      }
      else
        manualHole();
      currentShot = 0;
      mode = 1;
      break;
    case 3: //select golf course and start hole
      golfSelect();
      courseSelected = true;
      mode = 1;
      break;
    case 4: //drive or walk
      showGPSdata();
      break;
  }
  if (!digitalRead(switch2_pin))
  {
    myOLED.invert(true);
    delay(100);
    myOLED.invert(false);
    delay(500);
    if (mode == 1) mode = 0;
    if (mode == 4) mode = 3;
    do
    {
      myOLED.clrScr();
      myOLED.print("SELECT", LEFT, 0);
      myOLED.print(selectMode[mode], CENTER, 24);
      myOLED.print("Yes", LEFT, 48);
      myOLED.print("Next", RIGHT, 48);
      myOLED.update();
      while (digitalRead(switch1_pin) && digitalRead(switch2_pin));
      if (!digitalRead(switch2_pin))
      {
        mode++;
        if (mode > 4) mode = 0;
        delay(500);
      }
      if (!digitalRead(switch1_pin)) break;
    } while (1);
    delay(500);
  }
  if (!digitalRead(switch1_pin) && mode == 1)
  {
    myOLED.invert(true);
    delay(100);
    myOLED.invert(false);
    delay(500);
    totalShot++;
    currentShot++;
    shotLat = gpsLat;
    shotLong = gpsLong;
  }
}

void golfSelect()
{
  do
  {
    myOLED.clrScr();
    myOLED.print("SELECT", LEFT, 0);
    myOLED.print(courseName[courseIndex], LEFT, 24);
    myOLED.print("Yes", LEFT, 48);
    myOLED.print("Next", RIGHT, 48);
    myOLED.update();
    while (digitalRead(switch1_pin) && digitalRead(switch2_pin));
    if (!digitalRead(switch2_pin))
    {
      courseIndex++;
      if (courseIndex >= (num_courses + 1)) courseIndex = 0;
      delay(500);
    }
    if (!digitalRead(switch1_pin)) break;
  } while (1);
  delay(500);
  if (courseIndex == (num_courses + 1))
  {
    mode = 4;
    courseIndex = 0;
  }
  else
  {
    myOLED.clrScr();
    myOLED.print("Front", CENTER, 0);
    myOLED.print("or Back", CENTER, 16);
    myOLED.print("Front", LEFT, 48);
    myOLED.print("Back", RIGHT, 48);
    myOLED.update();
    while (digitalRead(switch1_pin) && digitalRead(switch2_pin));
    if (!digitalRead(switch1_pin))
    {
      delay(20);
      currentHole = 1;
    }
    if (!digitalRead(switch2_pin))
    {
      delay(20);
      currentHole = 10;
      startBack = true;
    }
  }
  totalShot = 0;
  currentShot = 0;
  delay(500);
}

void golfPlay()
{
  while (!gpsRead());
  myOLED.clrScr();
  if (lowBat)
  {
    myOLED.invertText(true);
    myOLED.print("LOW", RIGHT, 0);
    myOLED.invertText(false);
  }
  else
  {
    myOLED.printNumI(gpsHour, 68 + 12 * (gpsHour < 10), 0);
    myOLED.print(":", 92, 0);
    myOLED.printNumI(gpsMin, 104, 0);
  }
  distanceToGreen = distanceCalc(pgm_read_float(&(coord[courseIndex][0][currentHole])),
                                 pgm_read_float(&(coord[courseIndex][1][currentHole])),
                                 gpsLat, gpsLong);
  if (distanceToGreen < inGreenRadius) onGreen = true;
  if (onGreen && distanceToGreen > outGreenRadius)
  {
    currentShot = 0;
    currentHole++;
    if (startBack && currentHole == 19) currentHole = 1;
    onGreen = false;
  }
  if ((!startBack && currentHole == 19) || ((startBack && currentHole == 10) && totalShot > 60))
  {
    myOLED.print("Total Shot:", CENTER, 16);
    myOLED.setFont(Ubuntu_Num_24x32);
    myOLED.printNumI(totalShot, CENTER, 32);
    myOLED.setFont(franklingothic_12x16);
  }
  else
  {
    if (onGreen) myOLED.invertText(true);
    myOLED.print("# ", LEFT, 0);
    myOLED.printNumI(currentHole, 24, 0);
    if (onGreen) myOLED.invertText(false);
    myOLED.setFont(Ubuntu_Num_24x32);
    myOLED.printNumI(distanceToGreen, RIGHT, 16);
    myOLED.setFont(franklingothic_12x16);
    myOLED.printNumI(totalShot, LEFT, 48);
    myOLED.printNumI(currentShot, CENTER, 48);
    if (shotLat != 0.0 && shotLong != 0.0)
      myOLED.printNumI(distanceCalc(shotLat, shotLong, gpsLat, gpsLong), RIGHT, 48);
  }
  myOLED.update();
}

void manualHole()
{
  do
  {
    myOLED.clrScr();
    myOLED.print("Select", LEFT, 0);
    myOLED.setFont(Ubuntu_Num_24x32);
    myOLED.printNumI(currentHole, CENTER, 16);
    myOLED.setFont(franklingothic_12x16);
    myOLED.print("Done", LEFT, 48);
    myOLED.print("Next", RIGHT, 48);
    myOLED.update();
    while (digitalRead(switch1_pin) && digitalRead(switch2_pin));
    if (!digitalRead(switch2_pin))
    {
      currentHole++;
      if (currentHole == 19) currentHole = 1;
      delay(300);
    }
    if (!digitalRead(switch1_pin)) break;
  } while (1);
  delay(500);
}

void showGPSdata()
{
  while (!gpsRead());
  myOLED.clrScr();
  if (dataValid == 'A')
  {
    myOLED.printNumI(gpsHour, 12 * (gpsHour < 10), 0);
    myOLED.print(":", 24, 0);
    myOLED.printNumI(gpsMin, 36, 0);
    if (lowBat)
    {
      myOLED.invertText(true);
      myOLED.print("LOW", RIGHT, 0);
      myOLED.invertText(false);
    }
    else
    {
      myOLED.printNumI(gpsMonth, 12 * (gpsMonth < 10) + 67, 0);
      myOLED.print("/", 24 + 67, 0);
      myOLED.printNumI(gpsDay, 36 + 67, 0);
    }
    //  myOLED.printNumF(gpsAlt, 0, LEFT, 32);
    if (gpsSpeed > 3)
    {
      myOLED.printNumF(gpsSpeed, 0, LEFT, 32);
      myOLED.setFont(Ubuntu_Num_24x32);
      myOLED.printNumF(gpsBearing, 0, RIGHT, 16);
     
      myOLED.setFont(franklingothic_12x16);
    }
  }
  else
  {
    myOLED.print("No signal", CENTER, 16);
  }
  myOLED.update();
}

float distanceCalc(float lat1, float long1, float lat2, float long2)
{
  float distLat;
  float distLong;
  float distance;
  distLat = (lat1 - lat2) * 111194.9;
  distLong = 111194.9 * (long1 - long2) * cos(radians((lat1 + lat2) / 2));
  distance = sqrt(pow(distLat, 2) + pow(distLong, 2));
  return distance / 0.9144; //in yard
}

boolean gpsFixChk() //  $GPGSA,A,3,
{
  int fixChk = 0;
  if (gps.available() > MSG_LENGTH)
  {
    if (char(gps.read()) == 'G' && char(gps.read()) == 'S' && char(gps.read()) == 'A' && char(gps.read()) == ',')
    {
      fixChk = gps.parseInt();
      if (fixChk == 3) return 1;
      else
      {
        myOLED.print("   ", 0, 48);
        myOLED.printNumI(fixChk, 0, 48);
        myOLED.update();
        return 0;
      }
    }
  }
  return 0;
}

boolean gpsRead()
{
  int la1, la2, lo1, lo2;
  if (gps.available() > MSG_LENGTH)
  {
    if (char(gps.read()) == 'R' && char(gps.read()) == 'M' && char(gps.read()) == 'C')
    {
      gpsTime(gps.parseInt());
      gps.parseFloat(); //discard unnecessary part
      gps.read();
      dataValid = char(gps.read());
      la1 = gps.parseInt();
      la2 = gps.parseInt();
      lo1 = gps.parseInt();
      lo2 = gps.parseInt();

      gpsSpeed = gps.parseFloat() * 1.852; //knot to km/h
      gpsBearing = gps.parseFloat();

      gpsDate(gps.parseInt());
      if (gpsYear != 0 && gpsYear % 4 == 0) DaysAMonth[1] = 29; //leap year check
      else DaysAMonth[1] = 28;
      //Time zone adjustment
      gpsHour += TimeZone;
      //DST adjustment
      if (gpsMonth * 100 + gpsDay >= DSTbegin[gpsYear - 13] &&
          gpsMonth * 100 + gpsDay < DSTend[gpsYear - 13]) gpsHour += 1;
      if (gpsHour < 0)
      {
        gpsHour += 24;
        gpsDay -= 1;
        if (gpsDay < 1)
        {
          if (gpsMonth == 1)
          {
            gpsMonth = 12;
            gpsYear -= 1;
          }
          else
          {
            gpsMonth -= 1;
          }
          gpsDay = DaysAMonth[gpsMonth - 1];
        }
      }
      if (gpsHour >= 24)
      {
        gpsHour -= 24;
        gpsDay += 1;
        if (gpsDay > DaysAMonth[gpsMonth - 1])
        {
          gpsDay = 1;
          gpsMonth += 1;
          if (gpsMonth > 12) gpsYear += 1;
        }
      }
      gpsLatLong(la1, la2, lo1, lo2);
      if (dataValid == 'A')
      {
        digitalWrite(LED_pin, HIGH);
        delay(10);
        digitalWrite(LED_pin, LOW);
      }
      if (gpsMin % 10 == 0)
      {
        if (readVcc() < 3500) lowBat = true;
      }
      return 1;
    }
  }
  return 0;
}

void gpsTime(long UTC)
{
  gpsHour = int(UTC / 10000);
  gpsMin = int(UTC % 10000 / 100);
  gpsSec = UTC % 100;
}

void gpsLatLong(int lat1, int lat2, int long1, int long2)
{
  gpsLat = int(lat1 / 100) + (lat1 % 100) / 60.0 + float(lat2) / 10000.0 / 60.0;
  gpsLong = int(long1 / 100) + (long1 % 100) / 60.0 + float(long2) / 10000.0 / 60.0;
}

void gpsDate(long dateRead)
{
  gpsDay = int(dateRead / 10000);
  gpsMonth = int(dateRead % 10000 / 100);
  gpsYear = dateRead % 100; //last 2 digits, e.g. 2013-> 13
}

long readVcc() {
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1); //for ATmega328p

  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA, ADSC)); // measuring

  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH
  uint8_t high = ADCH; // unlocks both

  long result = (high << 8) | low;

  result = 1125300L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  return result; // Vcc in millivolts
}
