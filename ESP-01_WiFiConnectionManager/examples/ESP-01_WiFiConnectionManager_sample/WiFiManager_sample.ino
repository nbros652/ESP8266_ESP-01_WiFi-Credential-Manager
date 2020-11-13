#include "WiFiManager.h"

WiFiManager wifi; // create the object we'll use to handle the wifi

void setup() {
  // There are some settings you can configure on the wifi object
  wifi.setAPIP(IPAddress(10,0,0,1));  // When the ESP is in AP mode use IP: 10.0.0.1
                                      // If unset, the IP defaults to 192.168.4.1

  wifi.setAPCredentials("Configure ESP", "Pass123!"); // When the ESP is in AP mode, have it
                                                        // create a network named "Configure ESP"
                                                        // and set the network password to
                                                        // "Pass123!"
                                                        // If unset, the device will use ESP-01.
  wifi.setHostname("iot-device"); // When the ESP is in client mode and connected to an AP, use
                                  // this name on the network. Make sure this string has valid
                                  // hostname syntax: https://en.wikipedia.org/wiki/Hostname#Syntax
                                  // If unset, hostname defaults to "unnamed"
  wifi.setLed(1, LOW);  // This setting will use the LED on pin 1 (the TX LED) for blinking
                        // the ESP-01 status. LOW indicates the state of pin 1 when the LED is
                        // lit.
                        // If unset, this defaults to (1, LOW). If an LED were connected to pin
                        // 13 and the on-state for the LED was HIGH, you would use (13, HIGH)
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

                // When the start method is in running, there are two blink patterns that will 
                // be used:
                // 1) If the ESP is attempting to connect to a network, it will have a short
                //    blink pattern (1/4-second-blink).
                // 2) If the ESP timed out while attempting a connection, it will drop into the
                //    network configuration mode (AP mode) where it waits for a user to connect
                //    and access the network configuration page. In this mode, there is a 
                //    long-blink pattern (1-second-blink).
  
}

bool startedSerial = false;
void loop() {
  // delay starting Serial until after wifi.start() has completed in case the Serial pin (pin 1)
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
