/*
 * This class does not itself manage the the WiFi on the ESP-01. Instead, it handles
 * the management of network connectivity, specifically, it simplifies the process of
 * connecting to available networks without having to hardcode credentials into the
 * ESP-01.
 * 
 * While the class itself is new, much of the functionality and large bits of code have
 * been drawn from https://how2electronics.com/esp8266-manual-wifi-configuration-without-hard-code-with-eeprom/#ESP8266_Manual_Wifi_Configuration_with_EEPROM
 */

#include "WiFiManager.h"

int statusCode;
String st;
String content;
String esid;
int led;
int ON_SIGNAL;
int OFF_SIGNAL;
ESP8266WebServer server(80);

// try connecting to WiFi for 15 seconds
bool WiFiManager::testWiFi() {
  // set some timer variables
  const int timeout = 15000;            // ms to wait for connection to AP
  const int onTime = 250;               // LED blink on duration
  const int offTime = 1000;             // LED blink off duration
  const unsigned long timer = millis(); // timer for connection timeout
  unsigned long ledTimer = millis();    // timer for LED blinks
  
  int ledTimeout = 0;                   // placeholder for ms wait time for blinks
  bool ledOn = false;
  // run a loop for the duration of the connection wait time
  while (millis() - timer < timeout) {
    if (WiFi.status() == WL_CONNECTED) {
      // looks like we connected. Let's get out of here
      digitalWrite(led, OFF_SIGNAL);  // turn the LED off
      return true;              // return from function with connection success
    }
    
    // while we are attepting to connect, blink LED
    if (millis() - ledTimer >= ledTimeout) {
      digitalWrite(led, (ledOn ? OFF_SIGNAL : ON_SIGNAL));
      ledTimeout = (ledOn ? offTime : onTime);
      ledOn = !ledOn;
      ledTimer = millis();
    }
    delay(10);
  }
  
  // Connection to AP timed out
  digitalWrite(led, OFF_SIGNAL);      // turn the LED off
  return false;  
}

void WiFiManager::initialize() {
  WiFi.disconnect();

  EEPROM.begin(512); //Initialasing EEPROM
  delay(10);

  // read stored SSID from EEPROM
  for (int i = 0; i < 32; ++i) {
    esid += char(EEPROM.read(i));
  }

  // read stored network password from EEPROM
  String epass = "";
  for (int i = 32; i < 96; ++i) {
    epass += char(EEPROM.read(i));
  }

  // Attempt a connection to the stored AP
  WiFi.begin(esid.c_str(), epass.c_str());
  if (testWiFi()) {
    // shut down the ESP AP if we don't need it
    const bool wifioff = true;
    WiFi.softAPdisconnect(wifioff);
    return;
  } else {
    // Turning the HotSpot On
    launchWeb();
    setupAP();// Setup HotSpot
  }
 
  // While WiFi is not connected, handle the web configuration page
  unsigned long timer = millis();   // blink timer
  const int timeout = 1000;         // ms on/off blink duration
  bool ledOn = false;
  while ((WiFi.status() != WL_CONNECTED))
  {
    
    // while in AP mode, do slow 1s blink (1s on / 1s off)
    if (millis() - timer >= timeout) {  
      digitalWrite(led, (ledOn ? OFF_SIGNAL : ON_SIGNAL));
      ledOn = !ledOn;
      timer = millis();
    }
    delay(10);
    // process any web requests
    server.handleClient();
  }

}

void WiFiManager::launchWeb() {
  createWebServer();
  // Start the web server for router setup
  server.begin();
}

void WiFiManager::rescan() {
  // scan available AP's
  int n = WiFi.scanNetworks();

  // Create <select> options of each SSID and RSSI found during scan
  st = "";
  for (int i = 0; i < n; ++i) {
    st += "<option value='"+WiFi.SSID(i)+"'>" + WiFi.SSID(i) + 
    " (" + WiFi.RSSI(i) + ")" + ((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*") + 
    "</option>";
  }
}

// Set the ESP in AP mode
void WiFiManager::setupAP() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  rescan();
  delay(100);
  WiFi.softAP(AP_SSID, AP_PWD);
  launchWeb();
}

// Create a web server to serve pages for network configuration
void WiFiManager::createWebServer() {
  {
    // page to server when root is requested
    server.on("/", [this]() {
      IPAddress ip = WiFi.softAPIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
      content = "<!DOCTYPE html>\r\n<html><head>"
        "<script>function onSelectNetwork() {"
        "const network=document.getElementById('ssid').value;"
        "const pass = document.getElementById('pass');"
        "const join = document.getElementById('join');"
        "if(network===''){"
        "pass.setAttribute('disabled','disabled');"
        "join.setAttribute('disabled','disabled');"
        "}else{"
        "join.removeAttribute('disabled');"
        "pass.removeAttribute('disabled');"
        "pass.focus();"
        "}}"
        "function rescan(){"
        "document.getElementById('overlay').classList.remove('hidden');"
        "}</script>"
        "<style>"
          "h1{"
          "font-size:4rem !important;"
          "background:cornflowerblue;"
          "color:gold;"
          "padding:2rem;"
          "margin:0;"
          "}"
          "body{"
          "margin:0;"
          "background:#ededed;"
          "display:flex;"
          "flex-direction:column;"
          "}"
          "body *{"
          "font-size:3rem;"
          "font-family:sans-serif;"
          "}"
          "input,select,label{"
          "margin:2rem;"
          "margin-bottom:0;"
          "flex-grow:1;"
          "height:8rem;"
          "border-radius:10px;"
          "padding:10px;"
          "box-sizing:border-box;"
          "}"
          "label{height:auto;}"
          ".content{"
          "display:flex;"
          "flex-direction:column;"
          "}"
          "form{"
          "display:flex;"
          "flex-direction:column;"
          "}"
          ".btn.success{"
          "background:#28a745;"
          "color:white;"
          "}"
          ".btn{"
          "font-weight:bold;"
          "cursor:pointer;"
          "background:#6c757d;"
          "color:white;"
          "}"
          "select {"
          "background:#fff;"
          "}"
          ".success.btn[disabled]{"
          "background:#b2b9b3;"
          "color:gray;"
          "}"
          "#overlay{"
          "background:#000a;"
          "position:fixed;"
          "width:100%;"
          "height:100vh;"
          "display:flex;"
          "align-items:center;"
          "color:#eee;"
          "font-weight:bold;"
          "}"
          "#overlay>div{"
          "margin:auto;"
          "font-size:5rem;"
          "text-shadow:0 0 10px #000;"
          "}"
          ".hidden{"
          "display:none;"
          "visibility:hidden;"
          "}"
        "</style></head><body>"
        "<h1>Join a network</h1>"
        "<label>Device: "+hostname+"</label>"
        "<form method='get' action='setting'>"
        "<select id='ssid' name='ssid' onchange='onSelectNetwork()' required>"
        "<option value=''>SELECT A NETWORK</option>" + st + "</select><br>"
        "<input disabled='disabled' id='pass' name='pass' minlength=8 maxlength=64 type='password' placeholder='Password' required><br>"
        "<input id='join' disabled='disabled' class='success btn' type='submit' value='Join Network'></form>"
        "<form action=\"/rescan\" method=\"POST\" onclick='rescan()'><input class='btn' type=\"submit\" value=\"Rescan\"></form>"
        "<form action=\"/reboot\" method=\"POST\"><input class='btn' type=\"submit\" value=\"Reboot\"></form>"
        "<div class='hidden' id='overlay'><div>Please wait ...</div></div>"
        "</body></html>";
      server.send(200, "text/html", content);
    });
    
    // page to serve when /reboot is requested and reboot ESP-01
    server.on("/reboot", [this]() {
      String content = "<!DOCTYPE html>"
        "<html>"
        "<head><style>"
          "body{"
          "margin:0;"
          "background:#ededed;"
          "display:flex;"
          "flex-direction:column;"
          "}"
          "body *{"
          "font-family:sans-serif;"
          "}"
          "#overlay{"
          "background:#000a;"
          "position:fixed;"
          "width:100%;"
          "height:100vh;"
          "display:flex;"
          "align-items:center;"
          "color:#eee;"
          "font-weight:bold;"
          "}"
          "#overlay>div{"
          "margin:auto;"
          "font-size:5rem;"
          "text-shadow:0 0 10px #000;"
          "text-align:center;"
          "}"
        "</style></head>"
        "<body>"
        "<div id='overlay'><div>Rebooting and will attempt to connect to "+esid+".<br><br>This device is no longer connected to "+AP_SSID+"</div></div>"
        "</body>"
        "</html>";
      server.send(200, "text/html", content);
      delay(1000);
      ESP.reset();
    });
    
    // rescan available networks and refresh the page
    server.on("/rescan", [this]() {
      rescan();
      IPAddress ip = WiFi.softAPIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
 
      content = "<!DOCTYPE HTML>\r\n<html><body><script>window.location='http://"+ipStr+"'</script><h1>Loading...</h1></body></html>";
      server.send(200, "text/html", content);
    });
 
    // Save the settings entered (if valid) for selected network
    server.on("/setting", [this]() {
      String qsid = server.arg("ssid");
      String qpass = server.arg("pass");
      if (qsid.length() > 0 && qpass.length() > 7) {
        // clearing eeprom
        for (int i = 0; i < 96; ++i) {
          EEPROM.write(i, 0);
        }

        // writing eeprom ssid
        for (int i = 0; i < qsid.length(); ++i) {
          EEPROM.write(i, qsid[i]);
        }
        
        // writing eeprom pass
        for (int i = 0; i < qpass.length(); ++i)
        {
          EEPROM.write(32 + i, qpass[i]);
        }
        EEPROM.commit();
 
        String content = "<!DOCTYPE html>"
          "<html>"
          "<head><style>"
              "body{"
              "margin:0;"
              "background:#ededed;"
              "display:flex;"
              "flex-direction:column;"
              "}"
              "body *{"
              "font-family:sans-serif;"
              "}"
              "#overlay{"
              "background:#000a;"
              "position:fixed;"
              "width:100%;"
              "height:100vh;"
              "display:flex;"
              "align-items:center;"
              "color:#eee;"
              "font-weight:bold;"
              "}"
              "#overlay>div{"
              "margin:auto;"
              "font-size:5rem;"
              "text-shadow:0 0 10px #000;"
              "text-align:center;"
              "}"
          "</style></head>"
          "<body>"
          "<div id='overlay'><div>Credentials saved!<br>Now attempting to join "+qsid+"<br><br>This device is no longer connected to "+AP_SSID+"</div></div>"
          "</body>"
          "</html>";
        server.send(200, "text/html", content);
        delay(1000);
        ESP.reset();
      } else {
        content = "{\"Error\":\"Password too short!\"}";
        statusCode = 404;
        server.sendHeader("Access-Control-Allow-Origin", "*");
        server.send(statusCode, "application/json", content);
      }
    });
  }  
}

WiFiManager::WiFiManager() {
  // set some default values
  setAPCredentials("ESP-01", "");
  const IPAddress IP(192,168,4,1);
  setHostname("unnamed");
  setAPIP(IP);
  led = 1;
  ON_SIGNAL = LOW;
  OFF_SIGNAL = HIGH;
}

WiFiManager::~WiFiManager() {
  
}

// start the ESP's wifi
void WiFiManager::start() {
  pinMode(led, OUTPUT);
  digitalWrite(led, OFF_SIGNAL);

  initialize();
}

// Set the credentials that should be used to connect to the ESP when
// it is in AP mode.
void WiFiManager::setAPCredentials(String _AP_SSID, String _AP_PWD){
  AP_SSID = _AP_SSID;
  AP_PWD = _AP_PWD;  
}

// Set the hostname of ESP that will show when it is running in client mode
void WiFiManager::setHostname(String _hostname) {
  WiFi.hostname(_hostname);
  hostname = _hostname;
}

// Set the IP address of the ESP when it is running in AP mode
void WiFiManager::setAPIP(IPAddress IP) {
  const IPAddress subnet(255,255,255,0);
  WiFi.softAPConfig(IP, IP, subnet);  
}

// Query to find out if the WiFi is connected or not
int WiFiManager::state() {
  return WiFi.status() == WL_CONNECTED ? WIFI_CONNECTED : WIFI_DISCONNECTED;
}

// Set the pin that will blink state patterns
// Arg1: the pin number of the LED
// Arg2: the signal that indicates the LED on state (HIGH/LOW)
int WiFiManager::setLed(int _led, int _ON_SIGNAL) {
  led = _led;
  ON_SIGNAL = _ON_SIGNAL;
  OFF_SIGNAL = (ON_SIGNAL + 1) % 2;
}
