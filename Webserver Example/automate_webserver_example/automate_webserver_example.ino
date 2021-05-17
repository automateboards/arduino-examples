/*
 Created by AutoMATE board author
 Credits to https://randomnerdtutorials.com
 */

#include <WiFi.h>
#include <Wire.h>
#include <WebServer.h>
#include "RTClib.h"
#include <MCP23017.h>

/* Web page layout in separate header file */
#include "webpage.h" 

/* WiFi settings */
const char *ssid = "YOURWIFISSID";
const char *password = "YOURWIFIPASSWORD";

/* STM32 I2C slave address */
const int I2C_SLAVE_ADDRESS = 0x20;

/* Web page update interval in ms */
const long interval = 1000;

const int relay1_pin = 25; // ESP32 GPIO
const int relay2_pin = 26; // ESP32 GPIO

const bool DMX512_ENABLE = false;

/* Global variables */
unsigned long previousMillis = 0; 
uint16_t temp_value = 0;
uint16_t bus3V3_value = 0;
uint16_t bus5V0_value = 0;
String   rtc_value = "";
uint8_t GPA_val = 0;
uint8_t GPB_val = 0;

/* Other objects */
WebServer server(80);
MCP23017 mcp = MCP23017(0x22);
RTC_DS1307 RTC;

/* Load webpage */
void handle_root() {
 String root_code = html_page;
 server.send(200, "text/html", root_code);
}

void handle_temp() {
  server.send(200, "text/plane", String(temp_value));
}

void handle_voltage3V3() {
  server.send(200, "text/plane", String(bus3V3_value));
}

void handle_voltage5V0() {
  server.send(200, "text/plane", String(bus5V0_value));
}

void handle_rtc() {
  server.send(200, "text/plane", rtc_value);
}

/* Web page button value updated callback */
void handle_button() {

 String button_state[4];
 
 /* Check value of each button */
 for (int i = 0; i < 4; i++) {
  button_state[i] = server.arg("button_state" + String(i + 1));

  if (button_state[i] == "1" || button_state[i] == "0") {
    Serial.print("button val" + String(i + 1) + ": ");
    Serial.println(button_state[i]);

    /* The used Raspberry Pi Relay HAT has 2 directly accessible Relays (by ESP32 GPIOs) 
    /* and 2 relays that have to be accesed via the STM32 (as IO expander) 
    /* Check out the HAT socket pin mapping on the website */
    if (i == 0)
      digitalWrite(relay1_pin, button_state[i] == "1" ? LOW : HIGH);
    else if (i == 3)
      digitalWrite(relay2_pin, button_state[i] == "1" ? LOW : HIGH);
    else if (i == 1) {
      if (button_state[i] == "1")
        bitClear(GPA_val, 7);
      else
        bitSet(GPA_val, 7);

      mcp.writeRegister(MCP23017Register::GPIO_A, GPA_val);
    }
    else if (i == 2) {
      if (button_state[i] == "1")
        bitClear(GPB_val, 5);
      else
        bitSet(GPB_val, 5);

      mcp.writeRegister(MCP23017Register::GPIO_B, GPB_val);
    }
  }
 }
 //server.send(200, "text/plane", curr_state); //Send state back to page
}

/* Web page slider value updated callback */
void handle_slider() {
  
  String slider_state[5];
  
  /* Check value of each slider */
  for (int i = 0; i < 5; i++) {
    slider_state[i] = server.arg("slider_val" + String(i + 1));
    
    Serial.print("slider val" + String(i + 1) + ": ");
    Serial.println(slider_state[i]);

    if ( i == 0 && !slider_state[i].isEmpty()) {
      
      /* Set IO1 duty cycle (range = 0 .. 1000 for 0 .. 100%) */
      stm32_set_register(0x5A, slider_state[i].toInt());
      
      /* Set DMX512 dim value, channel 2 = red, channel 3 = green and channels 4 = blue for the used DMX512 ficture */
      if (DMX512_ENABLE) {
        /* Get DMX512 dim value (range = 0 .. 255 for 0 .. 100%) -> factor 3.92 */ 
        uint8_t dmx512_chan_val = slider_state[i].toInt() / 3.92;

        Serial.print("dmx512 val");
        Serial.println(dmx512_chan_val);
      
        stm32_set_dmx512_channel(2, dmx512_chan_val);
        stm32_set_dmx512_channel(4, dmx512_chan_val);
        // stm32_set_dmx512_channel(3, 255-dmx512_chan_val);
      }
    }
    else if ( i == 1 && !slider_state[i].isEmpty()) {
      /* Set IO2 voltage (range = 0 .. 10000mV) */
      stm32_set_register(0x57, slider_state[i].toInt());
    }
    else if ( i == 2 && !slider_state[i].isEmpty()) {
      /* Set IO3 voltage (range = 0 .. 24000mV) */
      stm32_set_register(0x58, slider_state[i].toInt());
    }
    else if ( i == 3 && !slider_state[i].isEmpty()) {
      /* Set IO3 frequency (range = 20 .. 5000Hz) */
      stm32_set_register(0x6A, slider_state[i].toInt());
    }
    else if ( i == 4 && !slider_state[i].isEmpty()) {
      /* Set IO3 duty cycle (range = 0 .. 1000 for 0 .. 100%) */
      stm32_set_register(0x5C, slider_state[i].toInt());
    }
  }
  
 server.send(200,"text/plane","0");
}

/* Helper function to set 16-bit register value on the STM32 */
void stm32_set_register(uint8_t register_address, uint16_t register_value)
{
  Wire.beginTransmission(I2C_SLAVE_ADDRESS);
  Wire.write(register_address);
  Wire.write((register_value & 0xFF00) >> 8);
  Wire.write((register_value & 0x00FF) >> 0);
  Wire.endTransmission(true);
}

/* Helper function to set 8-bit DMX512 channel value on the STM32 */
void stm32_set_dmx512_channel(uint16_t channel, uint8_t value)
{
  Wire.beginTransmission(I2C_SLAVE_ADDRESS);

  /* Write 16-bit channel pointer */
  Wire.write(0x90);
  Wire.write((channel & 0xFF00) >> 8);
  Wire.write((channel & 0x00FF) >> 0);

  /* Address 0x91 (auto increment): Set number of DMX512 bytes to send (= 1 to set 1 channel) */
  Wire.write(0);
  Wire.write(1);

  /* Address 0x92 (auto increment): Write DMX512 channel value */
  Wire.write(value);

  /* Address 0x93 (auto increment): Resume transmission */
  Wire.write(0);
  Wire.write(0);
  
  Wire.endTransmission(true);
}

void setup(void){
  
  /* Init UART */
  Serial.begin(115200);
  
  /* Init I2C */
  Wire.begin();
  
  /* Init WiFi */
  WiFi.begin(ssid, password);
  
  Serial.println("");
  
  if (!RTC.begin()) 
    Serial.print("Couldn't find RTC");
    
  if (!RTC.isrunning()) 
    Serial.print("RTC is NOT running!");
    
  /* The used Raspberry Pi Relay HAT has 2 directly accessible Relays (by ESP32 GPIOs) */
  /* and 2 relays that have to be accesed via the STM32 (as IO expander) */
  /* Check out the HAT socket pin mapping on the website */

  /* Init IO expander (MCP23017 emulater inside STM32) for relay 3 and 4 */
  mcp.init();
  mcp.portMode(MCP23017Port::A, 0b01111111); /* GPA7 = BCM26 = output, 0 = output, 1 = input */
  mcp.portMode(MCP23017Port::B, 0b11011111); /* GPB5 = BCM20 = output, 0 = output, 1 = input */

  /* Init GPIO for relay 1 and relay 2 */
  pinMode(relay1_pin, OUTPUT); 
  pinMode(relay2_pin, OUTPUT); 

  /* switch relays off by default (are active low) */
  digitalWrite(relay1_pin, HIGH);
  digitalWrite(relay2_pin, HIGH);
  
  bitSet(GPB_val, 5);
  mcp.writeRegister(MCP23017Register::GPIO_B, GPB_val);

  bitSet(GPA_val, 7);
  mcp.writeRegister(MCP23017Register::GPIO_A, GPA_val);

  /* Configure external IO1 */
  stm32_set_register(0x37, 0xA400); // Open drain output, active low
  stm32_set_register(0x40, 0x0002); // PWM LED mode

  /* Configure external IO2 */
  stm32_set_register(0x38, 0x4000); // Analog output
  stm32_set_register(0x41, 0x0009); // Analog output 0-10V mode

  /* Configure external IO3 */
  stm32_set_register(0x39, 0x8400); // Configure as push-pull output
  stm32_set_register(0x42, 0x0001); // Set as PWM mode

  /* Configure RS485 port for DMX512 */
  if (DMX512_ENABLE) {
    stm32_set_register(0x16, 0x0001); // Set UART switch to RS485 driver (this disables the serial monitor)
    stm32_set_register(0x80, 0x0280); // Put UART in DMX512 mode
    stm32_set_dmx512_channel(1, 255); // Channel 1 = global dim level for the used fixture
  }
  else {
    stm32_set_register(0x16, 0x0003); // Set UART switch to STM32 (this enables the serial monitor)
  }

  // Connection wait
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  /* Print network info */
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
 
  /* Webserver callbacks */
  server.on("/",              handle_root);
  server.on("/setSlider",     handle_slider);
  server.on("/setButton",     handle_button);
  server.on("/setTemp",       handle_temp);
  server.on("/setVoltage3V3", handle_voltage3V3);
  server.on("/setVoltage5V0", handle_voltage5V0);
  server.on("/setRTC",        handle_rtc);

  server.begin();
  Serial.println("HTTP server started");
}

/* Update webserver values loop */
void loop(void){
  
  /* Handle user input on web page */
  server.handleClient();

  if (millis() - previousMillis >= interval) {
    previousMillis = millis();

    /* Read 3V3, 5V rail voltage and temperature from STM32 */
    Wire.beginTransmission(I2C_SLAVE_ADDRESS);
    Wire.write(0x02);
    Wire.endTransmission(false);
    Wire.requestFrom(I2C_SLAVE_ADDRESS, byte(6));
    
    /* Decode 3V3, 5V rail voltage and temperature (register is 16-bit wide)*/
    bus3V3_value = (Wire.read() << 8) | Wire.read();
    bus5V0_value = (Wire.read() << 8) | Wire.read();
    temp_value = (Wire.read() << 8) | Wire.read();

    /* Read RTC time from STM32 using the DS1307 library 
     * Note: It is also possible to do this directly with I2C as above. */
    DateTime now = RTC.now();
    rtc_value = (String(now.year(), DEC) + "/" + 
      String(now.month(), DEC) + "/" + 
      String(now.day(), DEC) + " " +
      String(now.hour(), DEC) + ":" +
      String(now.minute(), DEC) + ":" +
      String(now.second(), DEC)
    );
  }
}
