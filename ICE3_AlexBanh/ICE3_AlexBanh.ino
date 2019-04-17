// Alex Banh, ajb9702 HCDE 440 sp19 ICE #3
// adafruitIO url: https://io.adafruit.com/wow1881/dashboards/hcde-440-ice3

//DHT required libraries
#include "DHT.h"
//MPL115A2 required libraries
#include <Wire.h>
#include <Adafruit_MPL115A2.h>
//OLED SSD1306 128x32 i2c required libraries
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "config.h"

#define DHTPIN 12     // Digital pin connected to the DHT sensor

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

#define OLED_RESET     13 // Reset pin # (or -1 if sharing Arduino reset pin)


#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

// Initialize DHT sensor
DHT dht(DHTPIN, DHTTYPE);

// Initialize MPL115A2 sensor
Adafruit_MPL115A2 mpl115a2;

// Initialize the OLED display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Initialize the adafruitIO feed for temperature
AdafruitIO_Feed *temperature = io.feed("temperature");

// Initialize the adafruitIO feed for pressure
AdafruitIO_Feed *pressure = io.feed("pressure");


void setup() {
  // put your setup code here, to run once:
    Serial.begin(115200);
    dht.begin();
    mpl115a2.begin();

    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
      Serial.println(F("SSD1306 allocation failed"));
      for(;;); // Don't proceed, loop forever
    }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000); // Pause for 2 seconds

  // Clear the buffer
  display.clearDisplay();

    // connect to io.adafruit.com
  Serial.print("Connecting to Adafruit IO");
  io.connect();

  // wait for a connection
  while(io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  // we are connected
  Serial.println();
  Serial.println(io.statusText());

}

void loop() {

  // io.run() keeps the client connected to
  // io.adafruit.com and processes any incoming data.
  io.run();

  // ======== START DHT =========
  
  // Wait a few seconds between measurements.
  delay(2000);

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  
  // Print out all the information collected from the DHT sensor
  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C "));
  Serial.print(f);
  Serial.print(F("°F  Heat index: "));
  Serial.print(hic);
  Serial.print(F("°C "));
  Serial.print(hif);
  Serial.println(F("°F"));

  // Sends the current farenheit value of temperature to the "temperature" feed on adafruitIO
  temperature->save(f);


  // ======== END DHT =========

  // ======== START MPL115A2 =========

  // Variables initalized to store the current pressure and temperature
  float pressureKPA = 0, temperatureC = 0;    

  // Gets and sets both the pressure and temperature variables with data from the MPL115A2 sensor
  mpl115a2.getPT(&pressureKPA,&temperatureC);

  // Prints the currently stored values for pressure and temperature
  Serial.print("Pressure (kPa): "); Serial.print(pressureKPA, 4); Serial.print(" kPa  ");
  Serial.print("Temp (*C): "); Serial.print(temperatureC, 1); Serial.println(" *C both measured together");

  // Sends the current pressure value to the "pressure" feed on adafruitIO
  pressure->save(pressureKPA);
  
  // ======== END MPL115A2 =========

  // ======== START OLED =========

  // Calls a method to print the current temperature value from the DHT and the current pressure value
  // from the MPL115A2 to our OLED display.
  displayData((String)t, (String)pressureKPA);

  // ======== END OLED ISH =========

}

// DisplayData takes in a string representing the current temperature and a string representing the current pressure
// and then prints them out to the OLED display
void displayData(String temp, String pres) {
  // Clears the current display
  display.clearDisplay();

  // Sets some display options
  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(WHITE); // Draw white text
  display.setCursor(0, 0);     // Start at top-left corner
  display.cp437(true);         // Use full 256 char 'Code Page 437' font

  // Prints the label "Current temperature: " with the current temperature reading from the DHT sensor
  display.print("Current temperature: ");
  display.print(temp + "\n");

  // Prints the label "Current temperature: " with the current temperature reading from the DHT sensor
  display.print("Current pressure: \n");
  display.print(pres);

  // Tells the display to display what is buffered 
  display.display();
}
