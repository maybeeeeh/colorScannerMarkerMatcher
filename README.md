**DISCLAIMER** This code was done pretty quick and dirty so it isn't very pretty but oh well I tried
# colorScannerMarkerMatcher
### Overview
This was a project for part 2 of Mark Rober's Creative Engineering course. It is an RGB scanner that will match the color of the scanned object to one of the many markers that I own. It uses an Adafruit TCS34725 RGB sensor to measure the RGB values of the objects. Every time the limit switch trigger is hit the sensor reads five R, G, and B values then averages them to get the best approximation of the scanned color. It then compares the scanned objects RGB values to the saved RGB values of the markers and finds the one with the shortest Euclidean color distance. The name of the closest marker is then displayed on the LCD screen. The markers I used for this project are the Blick Studio pro alcohol markers and a few Copic Sketch markers. Each marker had to be scanned and input manually into the Arduino code. This process was pretty tedious so I did not include every single marker. I omitted black and all of the gray shades. 

#### BOM
For this project you will need:
- Arduino UNO or equivalent
- Adafruit TCS34725
- 16 pin LCD screen
- limit switch
- 220 ohm resitor
- 10k ohm potentiometer
- breadboard
- USB for microcontroller
- various jumper wires

### Future Plans/TODO
If I have the time/motivation/money in the future, I hope to:
1. Be able to include all of the markers I have 
2. Make the process of inputing the RGB values for the markers easier. It would probably be best if there was a mode for the scanner that automatically saved the RGB values to the marker database. Either that, or a function to parse a CSV file and import the data that way. 
3. Create a handheld enclosure for the device so I can scan things more easily
4. Do a better job calibrating the colors and fine tuning the database values
5. Clean up the program to make it more compartmentalized with custom functions/classes
