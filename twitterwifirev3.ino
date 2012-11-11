#include <LiquidCrystal.h>
#include <WiFi.h>
#include <WiFiServer.h>
#include <SPI.h>

char ssid[] = "Mehfil Guest"; //  your network SSID (name) 
int keyIndex = 0;            // your network key Index number (needed only for WEP)
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

int status = WL_IDLE_STATUS; // status of the wifi connection
int bytei; // used in writing serial data to string
int col; // used to print a row's worth of string to lcd

// initialize the library instance:
WiFiClient client;

const unsigned long requestInterval = 30000;    // delay between requests; 30 seconds

// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
//IPAddress server(199,59,149,200);    // numeric IP for api.twitter.com
char server[] = "api.twitter.com";     // name address for twitter API

boolean requested;                     // whether you've made a request since connecting
unsigned long lastAttemptTime = 0;     // last time you connected to the server, in milliseconds

String currentLine = "";               // string to hold the text from server
String tweet = "";                     // string to hold the tweet
boolean readingTweet = false;          // if you're currently reading the tweet

void setup() {
  // set up the LCD's number of columns and rows: 
  lcd.begin(20, 4);
  lcd.noCursor();
  // Print a message to the LCD.
  lcd.clear();
  lcd.noAutoscroll();  
  lcd.setCursor(0,0);
  lcd.print("Tomas' Arduino CNN");
  delay(3000);
  
  // reserve space for the strings:
  currentLine.reserve(256);
  tweet.reserve(150);
 
  // start serial port:
  Serial.begin(9600);

  // attempt to connect to Wifi network:
  while ( status != WL_CONNECTED) { 
    lcd.clear(); 
    lcd.setCursor(0, 0);
    lcd.print("Attempting to");
    lcd.setCursor(0, 1);
    lcd.print("connect to SSID: ");
    lcd.setCursor(0, 2);
    lcd.println(ssid);
    status = WiFi.begin(ssid);
    // wait 10 seconds for connection:
    delay(10000);
  } 
  // you're connected now, so print out the status:
  printWifiStatus();
  connectToServer();
}

void stringToLCD()
{
    int lineCount = 0;
    int lineNumber = 0;
    byte stillProcessing = 1;
    byte charCount = 1;
    lcd.clear();
    lcd.setCursor(0,0);

    while(stillProcessing) {
         if (lineNumber > 3) {    // if the line number goes over 4 lines
              delay(5000);
              lcd.clear(); 
              lcd.setCursor(0, 0);
              lineNumber = 0;
         }
         if (++lineCount > 20) {    // have we printed 20 characters yet (+1 for the logic)
              lineNumber += 1;
              lcd.setCursor(0,lineNumber);   // move cursor down
              lineCount = 1;
         }

         lcd.print(tweet[charCount - 1]);

         if (!tweet[charCount]) {   // no more chars to process?
              stillProcessing = 0;
         }
         charCount += 1;
    }
}

void loop()
{
  if (client.connected()) {
    if (client.available()) {
      // read incoming bytes:
      char inChar = client.read();

      // add incoming byte to end of line:
      currentLine += inChar; 

      // if you get a newline, clear the line:
      if (inChar == '\n') {
        currentLine = "";
      } 
      // if the current line ends with <text>, it will
      // be followed by the tweet:
      if ( currentLine.endsWith("<text>")) {
        // tweet is beginning. Clear the tweet string:
        readingTweet = true; 
        tweet = "";
        // break out of the loop so this character isn't added to the tweet:
        return;
      }
      // if you're currently reading the bytes of a tweet,
      // add them to the tweet String:
      if (readingTweet) {
        if (inChar != '<') {
          tweet += inChar;
        } 
        else {
          // if you got a "<" character,
          // you've reached the end of the tweet:
          readingTweet = false;
          lcd.clear(); 
          lcd.setCursor(0, 0);
          stringToLCD();   
          // close the connection to the server:
          client.stop(); 
        }
      }
    }   
  }
  else if (millis() - lastAttemptTime > requestInterval) {
    // if you're not connected, and two minutes have passed since
    // your last connection, then attempt to connect again:
    connectToServer();
  }
}

void connectToServer() {
  // attempt to connect, and wait a millisecond:
  lcd.clear(); 
  lcd.setCursor(0, 0);
  lcd.println("connecting...");
  if (client.connect(server, 80)) {
    lcd.clear(); 
    lcd.setCursor(0, 0);
    lcd.println("HTTP request...");
    // make HTTP GET request to twitter:
    client.println("GET /1/statuses/user_timeline.xml?screen_name=cnnbrk HTTP/1.1");
    client.println("Host:api.twitter.com");
    client.println("Connection:close");
    client.println();
  }
  // note the time of this connect attempt:
  lastAttemptTime = millis();
}   


void printWifiStatus() {
  // print the SSID of the network you're attached to:
  lcd.clear(); 
  lcd.setCursor(0, 0);
  lcd.print("SSID: ");
  lcd.setCursor(0, 1);
  lcd.println(WiFi.SSID());
  delay(3000);

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  lcd.clear(); 
  lcd.setCursor(0, 0);
  lcd.print("IP Address: ");
  lcd.setCursor(0, 1);
  lcd.println(ip);
  delay(3000);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  lcd.clear(); 
  lcd.setCursor(0, 0);
  lcd.print("signal strength");
  lcd.setCursor(0, 1);
  lcd.print("(RSSI):");
  lcd.print(rssi);
  lcd.println(" dBm");
  delay(3000);
}


