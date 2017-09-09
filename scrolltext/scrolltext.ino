#include <SPI.h>
#include "LedMatrix.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#define ssid  ""
#define password "$!23"

#define NUMBER_OF_DEVICES 5
#define CS_PIN D4

#define MAX_FEEDS 5

unsigned long int newsCounter = 0;

// convert HTML in https://www.freeformatter.com/java-dotnet-escape.html#ad-output

bool resetFromWeb = false;
String localIP = "";


ESP8266WebServer server;

LedMatrix ledMatrix = LedMatrix(NUMBER_OF_DEVICES, CS_PIN);

unsigned long tRefreshRSSFeed = 0, tLEDMatrixInterval = 0;

const String INDEX_HTML = "<!DOCTYPE html>\r\n<html>\r\n<head> \r\n<title>ServiceNow Message Post Demo </title>\r\n</head>\r\n<style> \r\ntextarea {\r\n    width: 100%;\r\n    height: 150px;\r\n    padding: 12px 20px;\r\n    box-sizing: border-box;\r\n    border: 2px solid #ccc;\r\n    border-radius: 4px;\r\n    background-color: #f8f8f8;\r\n    font-size: 45px;\r\n    resize: none;\r\n    font-family: Consolas, monaco, monospace;\r\n}\r\n.button {\r\n  display: inline-block;\r\n  padding: 15px 25px;\r\n  font-size: 24px;\r\n  cursor: pointer;\r\n  text-align: center;\r\n  text-decoration: none;\r\n  outline: none;\r\n  color: #fff;\r\n  background-color: #4CAF50;\r\n  border: none;\r\n  border-radius: 15px;\r\n  box-shadow: 0 9px #999;\r\n}\r\n\r\n.button:hover {background-color: #3e8e41}\r\n\r\n.button:active {\r\n  background-color: #3e8e41;\r\n  box-shadow: 0 5px #666;\r\n  transform: translateY(4px);\r\n\r\nh1 { \r\n\tfont-family: Arial, Helvetica, sans-serif;\r\n    display: block;\r\n    font-size: 2em;\r\n    margin-top: 0.67em;\r\n    margin-bottom: 0.67em;\r\n    margin-left: 0;\r\n    margin-right: 0;\r\n    font-weight: bold;\r\n}\r\n\r\n}\r\n</style>\r\n<body>\r\n<h1 align=\"center\" style=\"font-family: Consolas, monaco, monospace;\"> Welcome to Internet Scroll ! </h1>\r\n\r\n<form action=\"/\" method=\"post\">\r\n  <p style=\"font-family: Consolas, monaco, monospace;\"> Your message : </p>\r\n  <button align=\"center\" class=\"button\" type=\"submit\" value=\"Submit!\"> Submit Now! </button> <br>\r\n  <br>\r\n  <textarea type=\"textarea\" name=\"message\"></textarea>\r\n  <br>\r\n</form> \r\n\r\n</body>\r\n</html>\r\n\r\n\r\n";
  
void setup() {
	Serial.begin(115200);

	resetFromWeb = false;

	ledMatrix.init();
	ledMatrix.setText("Welome to Family Day !");

	if(connectToWiFi()) {
		Serial.println("Connected and set for requests ...");
	}
}

void loop() {
	server.handleClient();

	if(millis() - tLEDMatrixInterval > 10) { 
		ledMatrix.clear();
		ledMatrix.scrollTextLeft();
		ledMatrix.drawText();
		ledMatrix.commit();

		tLEDMatrixInterval = millis();
	}
}

// connects to Wifi and sets up the WebServer !
bool connectToWiFi() {
	Serial.println();
	Serial.println();
	Serial.print("Connecting to ");
	Serial.println(ssid);

	int count = 0;

	WiFi.begin(ssid, password);

	while (WiFi.status() != WL_CONNECTED) {
		if(count++ > 30) // 30 attempts
			return false; 
		delay(500);
		Serial.print(".");
	}

	Serial.println("");
	Serial.println("WiFi connected");
	Serial.println("IP address: ");
	
	localIP = WiFi.localIP().toString();

	Serial.println(localIP);

	// run the server now.
	server.on("/", handleRoot);
	server.on("/reset", []() {
		ledMatrix.setNextText("Welome to Family Day !");
		handleRoot();
	});  

	// start the server
	Serial.println("Starting the server ... ");
	server.begin(); 

	return true;
}


void handleRoot()
{
  if (server.hasArg("message")) {
    handleSubmit();
  }
  else {
    server.send(200, "text/html", INDEX_HTML);
  }
}

void handleSubmit()
{
  String message;

  message = server.arg("message");

  if(message != "") {
  	Serial.print("Got message : ");
  	Serial.println(message);
  	message.replace("\n", " ");
  	ledMatrix.setNextText(message);
  }

  server.send(200, "text/html", INDEX_HTML);
}

