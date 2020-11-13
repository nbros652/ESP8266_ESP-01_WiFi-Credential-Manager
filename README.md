# ESP8266_ESP-01_WiFi-Credential-Manager
*An abstraction layer for managing WiFi network connectivity on an ESP-01*

![screenshots](https://i.postimg.cc/59LGxTHh/screenshots.png)

This is an Arduino library for the ESP8266 ESP-01. It has only been tested on the ESP-01. Milage may vary on other ESP8266 devices.
This library depends on other libraries that are included when you configure everything for the ESP8266 in the board manager. For specific instructions on getting this set up, visit https://arduino-esp8266.readthedocs.io/en/latest/installing.html.

The included sample sketch shows how this library should be used, but here is a quick reference for using this library:

## Quick overview 
This library provides a quick and easy interface for managing ESP-01 network connectivity. When used, it attempts to connect to the last saved network. If it is unable, it blocks further code execution in your sketch and drops into a network configuration mode, putting the ESP-01 in AP mode. This happens when the start() method is called. There are two blink patters to look for while this class blocks code execution. The first is a short-blink pattern (1/4s blinks), and the second is a long-blink pattern (1s blinks). The short-blink pattern indicates that the ESP is attempting to connect to the last saved network (this will timeout after 15 seconds if does not connect). The long-blink pattern indicates that the ESP is in AP mode and awaiting a device to connect and input some WiFi credentials for a nearby wireless network. Once credentials are successfully saved, the ESP will then attempt to connect to the specified network on successive boots, falling back to the network configuration mode if it was unable to connect to the saved network. Upon successful connection, this class unblocks and the sketch impementing it is allowed to run.

## Setting it up 
Download the `ESP-01_WiFiConnectionManager` folder and place in the Arduino libraries folder.

## Using it in Arduino sketches 
At the top of your sketch, include the lines 

    #include "WiFiManager.h"
    WiFiManager wifi; // create the object we'll use to handle the wifi

Now, you have a couple of functions available to you that will most likely come in handy withing the `setup()` method of your sketch:
 * `wifi.setAPCredentials(String AP_SSID, String AP_PWD)`: This function allows you to specify the network name and password that should be used when the ESP is in AP mode and awaiting the configuration of network credentials. If not called, the default network name will be ESP-01 and there will be no password (it will be an open network with no security);
 * `wifi.setAPIP(IPAddress ip)`: This function allows you to specify the IP address that you want the ESP-01 to use when it is in AP mode. When connecting to the ESP-01 for network configuration, this is the address that will be used in the web browser (eg. http://192.168.4.1 or http://10.0.0.1). If this function is not called, the ESP-01 will default to 192.168.4.1.
 * `wifi.setHostname(String hostname)`: This function allows you to specify the hostname that the ESP-01 will use when it is in client mode and connected to a network. If not configured, this will default to "unnamed"
 * `wifi.setLed(int pin, int on_state)`: This function is used to specify a pin that should be used for status blink patterns while this library is running (described above). The second parameter specifies what state should be used to designate a lit LED. If not called, this defaults to pin 1 (the TX LED on the ESP-01) and LOW for the on_state because, the TX LED is lit when pin 1 is set LOW and it is off when pin 1 is set HIGH.
 * `wifi.start()`: This function should be called only after the previously mentioned functions have been called. This function is a blocking function. All code execution in your sketch will stop here until the ESP-01 successfully connects to a network. If your ESP-01 does not connect to a WiFi network, the code in your sketch will not exectute beyond this point!
 * `wifi.state()`: This function is the only that should be called after the `wifi.start()` method. It simply returns the state of the WiFi. Is it connected (WIFI_CONNECTED) or not (WIFI_DISCONNECTED). That way if an ESP-01 did successfully connect to a WiFi network and then dropped off, you can query for this information.
 
## Example Sketch 

    // Sample sketch demonstrating use of the WiFiManager class in an Arduino sketch
    #include "WiFiManager.h"

    WiFiManager wifi; // create the object we'll use to handle the wifi this could be named anything 
                      // you like 

    void setup() {
      wifi.setAPIP(IPAddress(10,0,0,1));  // When the ESP is in AP mode use IP: 10.0.0.1
      wifi.setAPCredentials("Configure ESP", "Pass123!"); // When the ESP is in AP mode, have it
                                                            // create a network named "Configure ESP"
                                                            // and set the network password to
                                                            // "Pass123!"
      wifi.setHostname("iot-device"); // When the ESP is in client mode and connected to an AP, use
                                      // this name on the network. Make sure this string has valid
                                      // hostname syntax: https://en.wikipedia.org/wiki/Hostname#Syntax
      wifi.setLed(1, LOW);  // This setting will use the LED on pin 1 (the TX LED) for blinking
                            // the ESP-01 status. LOW indicates the state of pin 1 when the LED is
                            // lit. This is the default setting, so ommitting this line would change
                            // in the case of this example.
      wifi.start(); // Once the wifi is configured, the start() method must be called to bring the 
                    // device online. This is a blocking method! Until this method completes, code
                    // execution stops here. No commands past this point will be run until this 
                    // method returns. That means nothing in the loop function will run until this
                    // method returns.
                    // If the ESP has not been configured to connect to a network, or the network
                    // for which it was configred is not in range, the ESP will go into AP mode so
                    // that a network can be selected for client mode and configured (at the IP
                    // specified above or 192.168.4.1 if not specified). Until a network has been
                    // selected and set up correctly, this function will not return, and the ESP
                    // will be stuck in AP mode awaiting credentials for a network.
                    // Once a network has been configured, the ESP will automatically reboot and
                    // attempt to connect to that network. If it is able to successfully connect,
                    // this method will return and code execution will resume. 
                    // While we are trapped in this method, the blink patterns described in the 
                    // documentation will flash to indicate the state of the ESP-01
      
    }

    bool startedSerial = false;
    void loop() {
      // delay starting Serial until after wifi.start() has completed in case the Serial TX pin (pin 1)
      // was used for blinking the onboard LED.
      if(!startedSerial) {
        Serial.begin(115200);
        startedSerial = true;
        delay(100);
      }
      String msg;
      if (wifi.state() == WIFI_CONNECTED) {
        msg = "The ESP is connected to WiFi!";
      } else if (wifi.state() == WIFI_DISCONNECTED) {
        msg = "The ESP is NOT connected to WiFi!";
      }
      Serial.println(msg);

      delay(1000);

    }
 
## Attribution 
As I sat down to work on this code, I looked around to see if anyone else had put something like it together. I came across [this post](https://how2electronics.com/esp8266-manual-wifi-configuration-without-hard-code-with-eeprom/#ESP8266_Manual_Wifi_Configuration_with_EEPROM) on the how2electronics.com website. I took the code, did a bit of tweaking, broke it out into this class and got their concent to post my derivative work on GitHub.
