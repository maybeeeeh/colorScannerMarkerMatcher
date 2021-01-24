#include <Wire.h>
#include "Adafruit_TCS34725.h"
#include <LiquidCrystal.h>

/* For RGB Sensor:
    Connect SCL to SCL
    Connect SDA to SDA
    Connect VIN to 5V DC
    Connect GND to common ground 
    Connect LED to Digital pin 7 */
    
/* For Trigger switch:
    Connect C to Digital pin 8
    Connect NC to common ground
    Connect NO to 5V DC */
    
// Initialise with specific int time and gain values
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

int reading;           // the current reading from the trigger input pin
int previous = LOW;    // the previous reading from the trigger input pin

byte count;             // index for number of color readings

long time = 0;         // the last time the output pin was toggled
long debounce = 200;   // the debounce time

uint16_t rval[5];           // R values for sample
uint16_t gval[5];           // G values for sample
uint16_t bval[5];           // B values for sample

uint16_t ravg;      // the average R value for the sample
uint16_t bavg;      // the average B value for the sample
uint16_t gavg;      // the average G value for the sample

unsigned long rsum;     // the sum of all the R values for the sample
unsigned long gsum;     // the sum of all the G values for the sample
unsigned long bsum;     // the sum of all the B values for the sample

bool trigger;     // flag for when the trigger is activated
bool startup;     // flag for when the lcd needs to be reset
bool first;       // flag for when the 1st time lcd screen needs to be displayed

// store the RGB values for all the markers in a really big array
const uint8_t MARKERS[][3] = {
  {159, 48, 45},
  {150, 54, 38},
  {106, 92, 38},
  {60, 101, 75},
  {36, 78, 125},
  {73, 60, 113},
  {119, 53, 71},
  {124, 45, 81},
  {111, 84, 41},
  {69, 112, 52},
  {56, 81, 102},
  {88, 82, 69},
  {92, 83, 64},
  {108, 70, 65},
  {159, 46, 41},
  {92, 98, 45},
  {32, 86, 121},
  {42, 68, 129},
  {134, 65, 42},
  {54, 93, 90},
  {44, 104, 89},
  {106, 81, 53},
  {127, 72, 41},
  {68, 84, 86},
  {100, 88, 48},
  {65, 80, 92},
  {54, 100, 82},
  {100, 74, 67},
  {96, 79, 64},
  {89, 81, 68},
  {87, 92, 59},
  {79, 98, 58},
  {55, 97, 85},
  {129, 61, 49},
  {105, 74, 59},
  {128, 57, 56},
  {121, 54, 73},
  {74, 76, 89},
  {59, 81, 98},
  {63, 89, 86},
  {37, 81, 122},
  {99, 84, 56},
  {121, 69, 49},
  {93, 81, 64},
  {122, 67, 54},
  {109, 74, 58},
  {115, 74, 54},
  {98, 84, 55},
  {83, 85, 68},
  {69, 85, 85},
  {78, 93, 67},
  {64, 92, 82},
  {76, 100, 59},
  {76, 96, 63},
  {52, 111, 74},
  {78, 109, 48},
  {146, 53, 44},
  {96, 80, 61},
  {143, 57, 44},
  {141, 59, 47},
  {93, 77, 66},
  {88, 76, 74},
  {59, 62, 122},
  {79, 78, 82},
  {127, 62, 59},
  {92, 89, 56},
  {117, 82, 39},
  {119, 81, 37},
};

// don't store all the marker color names bc caps out memory
/* const char *COLOR[] = {
  "red",
  "orange",
  "canary",
  "dk green",
  "bright blue",
  "purple",
  "pink",
  "magenta",
  "honey",
  "apple green",
  "periwinkle",
  "putty",
  "sepia",
  "wine",
  "poppy",
  "chartreuse",
  "true blue",
  "ultramarine",
  "burnt orange",
  "horizon blue",
  "aqua",
  "camel",
  "caramel",
  "lt periwinkle",
  "lt maize",
  "navy",
  "coral sea",
  "dk umber",
  "latte",
  "taupe",
  "olive",
  "leaf green",
  "teal",
  "salmon",
  "peach blush",
  "coral",
  "very berry",
  "lilac",
  "ink blue",
  "lt cerulean blue",
  "cerulean blue",
  "antique",
  "tan",
  "dk brown",
  "sienna",
  "walnut",
  "lt umber",
  "sand",
  "beach",
  "steel blue",
  "sage",
  "celedon green",
  "green tomato",
  "dk olive green",
  "bright green"
  "spring green",
  "peach",
  "lt peach",
  "spice",
  "terra cotta",
  "shell",
  "rose petal",
  "violet",
  "gray lavender",
  "brick",
  "buttercream",
  "yellow ochre",
  "saffron",
}; */

// find and define the number of marker RGB values
const byte markerCount = sizeof(MARKERS) / sizeof(MARKERS[0]);

double distance;      // the distance between scanned color and marker colors
double minDist;       // the minimum color distance found
byte minIndex;        // the array index of the marker with the smallest distance


// init stuff
void setup(void) {
  
  Serial.begin(9600);

  // initialize startup routine
  startup = true;
  first = true;
  
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);

  // set the trigger input pin
  pinMode(8, INPUT);
  // lower the trigger flag
  trigger = false;

  // set the LED output pin 
  pinMode(7, OUTPUT);
  // turn the LED off
  digitalWrite(7,LOW);

  // initialize the number of readings
  count = 0;

  // make sure RGB sensor is connected
  if (tcs.begin()) {
    Serial.println("Found sensor");
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    while (1); // halt
  }
}

// main loop
void loop(void) {
  float r, g, b;  // the RGB values read from the sensor

  // check the current state of the trigger flag by reading the input pin 
  reading = digitalRead(8);
  
  // startup routine for lcd screen
  if (startup == true) {
    if (first == true) {
      // print init msg to lcd
      lcd.print(" color  scanner");
      delay(2000);    // disp msg for 2 sec
      first = false;  // lower the first time startup flag
    }
    lcd.clear();
    // display waiting screen
    lcd.print("  scan a color");
    lcd.setCursor(0,1);
    lcd.print("     =^..^=");
    startup = false;    // lower the startup flag
  }

  // check to see if trigger has been activated
  if (reading == HIGH && previous == LOW && millis() - time > debounce){
    trigger = true;   // raise the trigger flag
    minDist = 4000000000;   // initialize the minimum distance to a random large number 
                            // (should be larger than max possible color dist just to be safe)
  }

  // check the trigger flag and read data if needed
  if (trigger == true) {
    digitalWrite(7, HIGH);    // turn on LED
    rsum = 0;   // clear RGB sums
    gsum = 0;
    bsum = 0;
    delay(200);   // wait 200ms before taking 1st reading to ensure accuracy
    
    // take 5 color readings
    while (count < 5) {
      tcs.getRGB(&r, &g, &b);
      // save each new reading to appropriate array
      rval[count] = r;
      gval[count] = g;
      bval[count] = b;

      // debugging print statements to check RGB values
      Serial.print("R: "); Serial.print(int(r)); Serial.print(" ");
      Serial.print("G: "); Serial.print(int(g)); Serial.print(" ");
      Serial.print("B: "); Serial.print(int(b)); Serial.print(" ");
      Serial.println(" ");  

      count += 1;   // increment count
    }

    // clear count after taking 5 readings
    count = 0;
    // turn off LED
    digitalWrite(7,LOW);

    // find average RGB values
    for (int i = 0; i < 5; i++) {
      rsum += rval[i];
      gsum += gval[i];
      bsum += bval[i];  
    }
    ravg = rsum/5;
    gavg = gsum/5;
    bavg = bsum/5;

    // debugging print statements to check avg RGB values
    Serial.print("R avg: ");Serial.println(ravg);
    Serial.print("G avg: ");Serial.println(gavg);
    Serial.print("B avg: ");Serial.println(bavg);

    // check Euclidean color dist for each marker
    for (int n = 0; n < markerCount; n++) {
      distance = sqrt(sq(ravg-MARKERS[n][0])+sq(gavg-MARKERS[n][1])+sq(bavg-MARKERS[n][2]));
      // save the minimum distance and index
      if (distance < minDist) {
        minDist = distance;
        minIndex = n;
      }
    }

    // print the closest color name to the lcd
    lcd.clear();
    lcd.setCursor(0,0);

    Serial.print("Closest marker color: ");
    switch (minIndex) {
      case 0:
        Serial.println("red");
        lcd.print("red");
        break;
      case 1:
        Serial.println("orange");
        lcd.print("orange");
        break;
      case 2:
        Serial.println("canary yellow");
        lcd.print("canary yellow");
        break;
      case 3:
        Serial.println("dark green");
        lcd.print("dark green");
        break;
      case 4:
        Serial.println("bright blue");
        lcd.print("bright blue");
        break;
      case 5:
        Serial.println("purple");
        lcd.print("purple");
        break;
      case 6:
        Serial.println("pink");
        lcd.print("pink");
        break;
      case 7:
        Serial.println("magenta");
        lcd.print("magenta");
        break;
      case 8:
        Serial.println("honey yellow");
        lcd.print("honey yellow");
        break;
      case 9:
        Serial.println("apple green");
        lcd.print("apple green");
        break;
      case 10:
        Serial.println("periwinkle");
        lcd.print("periwinkle");
        break;
      case 11:
        Serial.println("putty");
        lcd.print("putty");
        break;
      case 12:
        Serial.println("sepia");
        lcd.print("sepia");
        break;
      case 13:
        Serial.println("wine");
        lcd.print("wine");
        break;
      case 14:
        Serial.println("poppy red");
        lcd.print("poppy red");
        break;
      case 15:
        Serial.println("chartreuse");
        lcd.print("chartreuse");
        break;
      case 16:
        Serial.println("true blue");
        lcd.print("true blue");
        break;
      case 17:
        Serial.println("ultramarine");
        lcd.print("burnt orange");
        break;
      case 18:
        Serial.println("burnt orange");
        lcd.print("burnt orange");
        break;
      case 19:
        Serial.println("horizon blue");
        lcd.print("horizon blue");
        break;
      case 20:
        Serial.println("aqua");
        lcd.print("aqua");
        break;
      case 21:
        Serial.println("camel");
        lcd.print("camel");
        break;
      case 22:
        Serial.println("caramel");
        lcd.print("caramel");
        break;
      case 23:
        Serial.println("light periwinkle");
        lcd.print("light periwinkle");
        break;
      case 24:
        Serial.println("light maize");
        lcd.print("light maize");
        break;
      case 25:
        Serial.println("navy blue");
        lcd.print("navy blue");
        break;
      case 26:
        Serial.println("coral sea");
        lcd.print("coral sea");
        break;
      case 27:
        Serial.println("dark umber");
        lcd.print("dark umber");
        break;
      case 28:
        Serial.println("latte");
        lcd.print("latte");
        break;
      case 29:
        Serial.println("taupe");
        lcd.print("taupe");
        break;
      case 30:
        Serial.println("olive");
        lcd.print("olive");
        break;
      case 31:
        Serial.println("leaf green");
        lcd.print("leaf green");
        break;
      case 32:
        Serial.println("teal green");
        lcd.print("teal green");
        break;
      case 33:
        Serial.println("salmon pink");
        lcd.print("salmon pink");
        break;
      case 34:
        Serial.println("peach blush");
        lcd.print("peach blush");
        break;
      case 35:
        Serial.println("coral pink");
        lcd.print("coral pink");
        break;
      case 36:
        Serial.println("very berry");
        lcd.print("very berry");
        break;
      case 37:
        Serial.println("lilac");
        lcd.print("lilac");
        break;
      case 38:
        Serial.println("ink blue");
        lcd.print("ink blue");
        break;
      case 39:
        Serial.println("light cerulean blue");
        lcd.print("light cerulean blue");
        break;
      case 40:
        Serial.println("cerulean blue");
        lcd.print("cerulean blue");
        break;
      case 41:
        Serial.println("antique");
        lcd.print("antique");
        break;
      case 42:
        Serial.println("tan");
        lcd.print("tan");
        break;
      case 43:
        Serial.println("dark brown");
        lcd.print("dark brown");
        break;
      case 44:
        Serial.println("sienna");
        lcd.print("sienna");
        break;
      case 45:
        Serial.println("walnut");
        lcd.print("walnut");
        break;
      case 46:
        Serial.println("light umber");
        lcd.print("light umber");
        break;
      case 47:
        Serial.println("sand");
        lcd.print("sand");
        break;
      case 48:
        Serial.println("beach");
        lcd.print("beach");
        break;
      case 49:
        Serial.println("steel blue");
        lcd.print("steel blue");
        break;
      case 50:
        Serial.println("sage");
        lcd.print("sage");
        break;
      case 51:
        Serial.println("celedon green");
        lcd.print("celedon green");
        break;
      case 52:
        Serial.println("green tomato");
        lcd.print("green tomato");
        break;
      case 53:
        Serial.println("dark olive green");
        lcd.print("dark olive green");
        break;
      case 54:
        Serial.println("bright green");
        lcd.print("bright green");
        break;
      case 55:
        Serial.println("spring green");
        lcd.print("spring green");
        break;
      case 56:
        Serial.println("peach");
        lcd.print("peach");
        break;
      case 57:
        Serial.println("light peach");
        lcd.print("light peach");
        break;
      case 58:
        Serial.println("spice");
        lcd.print("spice");
        break;
      case 59:
        Serial.println("terra cotta");
        lcd.print("terra cotta");
        break;
      case 60:
        Serial.println("shell");
        lcd.print("shell");
        break;
      case 61:
        Serial.println("rose petal");
        lcd.print("rose petal");
        break;
      case 62:
        Serial.println("violet");
        lcd.print("violet");
        break;
      case 63:
        Serial.println("gray lavender");
        lcd.print("gray lavender");
        break;
      case 64:
        Serial.println("brick red");
        lcd.print("brick red");
        break;
      case 65:
        Serial.println("buttercream");
        lcd.print("buttercream");
        break;
      case 66:
        Serial.println("yellow ochre");
        lcd.print("yellow ochre");
        break;
      case 67:
        Serial.println("saffron");
        lcd.print("saffron");
        break;
    }

    Serial.print("Euclidean color distance: "); Serial.println(minDist);
    
    delay(5000);    // keep color name displayed for 5 seconds

    startup = true;   // raise the startup flag
    trigger = false;  // lower the trigger flag
  }
}
