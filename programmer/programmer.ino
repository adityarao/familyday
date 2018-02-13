#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Servo.h>

#define ssid  ""
#define password "$23"

#define MAX_LEDS 6
#define MAX_WAIT 20

const int LED_PIN[MAX_LEDS] = {D0, D1, D2, D3, D4, D5};
const int SOUND_PIN = D6;
const int SERVO1 = D7;
const int SERVO2 = D8;


// Servos 

Servo servo1, servo2;


// convert HTML in https://www.freeformatter.com/java-dotnet-escape.html#ad-output
String localIP = "";

// WebServer to serve the HTML !
ESP8266WebServer server;

// not a good solution - but part mash up ... 
const String HOME_PAGE_HEADER = "<!DOCTYPE html>\r\n<html>\r\n<head> \r\n    <link rel=\"stylesheet\" href=\"http://codemirror.net/lib/codemirror.css\">\r\n    <script src=\"http://codemirror.net/lib/codemirror.js\"></script>\r\n    <script src=\"http://codemirror.net/addon/mode/simple.js\"></script>\r\n    <script src=\"http://codemirror.net/addon/edit/matchbrackets.js\"></script>\r\n    <script src=\"http://codemirror.net/mode/javascript/javascript.js\"></script>  \r\n\t<title>Programmer</title>\r\n</head>\r\n<style> \r\n\t@namespace url(http://www.w3.org/1999/xhtml);\r\n\t.CodeMirror {\r\n\t    font-family:\"Ubuntu Mono\", monospace !important;\r\n\t    font-size:18pt !important;\r\n\t}\r\n\t.CodeMirror { height: 100%; }\r\n\t.button {\r\n\t  display: inline-block;\r\n\t  padding: 15px 25px;\r\n\t  font-size: 24px;\r\n\t  cursor: pointer;\r\n\t  text-align: center;\r\n\t  text-decoration: none;\r\n\t  outline: none;\r\n\t  color: #fff;\r\n\t  background-color: #4CAF50;\r\n\t  border: none;\r\n\t  border-radius: 15px;\r\n\t  box-shadow: 0 9px #999;\r\n\t}\t\r\n</style>\r\n<script type=\"text/javascript\">\r\n\twindow.onload = function () {\r\n\tCodeMirror.defineSimpleMode(\"iotcode\", {\r\n\t  start: [\r\n\t    {regex: /\"(?:[^\\\\]|\\\\.)*?(?:\"|$)/, token: \"string\"},\r\n\t    {regex: /(function)(\\s+)([a-z$][\\w$]*)/,\r\n\t     token: [\"keyword\", null, \"variable-2\"]},\r\n\t    {regex: /(?:STOP|START|LED|MOTOR|WAIT|GOTO|SOUND)\\b/,\r\n\t     token: \"keyword\"},\r\n\t    {regex: /true|false|null|LEFT|RIGHT|ON|OFF|undefined/, token: \"atom\"},\r\n\t    {regex: /0x[a-f\\d]+|[-+]?(?:\\.\\d+|\\d+\\.?\\d*)(?:e[-+]?\\d+)?/i,\r\n\t     token: \"number\"},\r\n\t    {regex: /\\/\\/.*/, token: \"comment\"},\r\n\t    {regex: /\\/(?:[^\\\\]|\\\\.)*?\\//, token: \"variable-3\"},\r\n\t    {regex: /\\/\\*/, token: \"comment\", next: \"comment\"},\r\n\t    {regex: /[-+\\/*=<>!]+/, token: \"operator\"},\r\n\t    {regex: /[\\{\\[\\(]/, indent: true},\r\n\t    {regex: /[\\}\\]\\)]/, dedent: true},\r\n\t    {regex: /[a-z$][\\w$]*/, token: \"variable\"},\r\n\t    {regex: /<</, token: \"meta\", mode: {spec: \"xml\", end: />>/}}\r\n\t  ],\r\n\t  comment: [\r\n\t    {regex: /.*?\\*\\//, token: \"comment\", next: \"start\"},\r\n\t    {regex: /.*/, token: \"comment\"}\r\n\t  ],\r\n\t  meta: {\r\n\t    dontIndentStates: [\"comment\"],\r\n\t    lineComment: \"//\"\r\n\t  }\r\n\t});    \r\n\tvar editableCodeMirror = CodeMirror.fromTextArea(document.getElementById('codesnippet_editable'), {\r\n\t        mode: \"iotcode\",\r\n\t        theme: \"default\",\r\n\t        lineNumbers: true\r\n\t    });\r\n\t};\r\n</script>\r\n<body>\r\n<h1 align=\"center\" style=\"font-family: Consolas, monaco, monospace;\"> IoT CODE CHALLANGE - Do you have the CodeMojo?</h1>\r\n\r\n<form action=\"/\" method=\"post\">\r\n  <br>\r\n  <div style=\"border: 2px;border-style: solid;\">";  
const String HOME_PAGE_BUTTON = "\t</div>\r\n\t<br>\r\n  <button align=\"center\" class=\"button\" type=\"submit\" value=\"Submit!\"> Run my code !</button> <br>\t\r\n  <br>";
const String HOME_PAGE_END = "</form> \r\n</body>\r\n</html>";

const String SAMPLE_CODE = "START\r\nON LED 1\r\nWAIT 5\r\nON LED 2\r\nWAIT 5\r\nON LED 3\r\nWAIT 5\r\nON LED 4\r\nMOTOR LEFT\r\nWAIT 5\r\nMOTOR RIGHT \r\nWAIT 5\r\nSTOP";

const String CORRECT_CHAR_HTML = "&#10004;";
const String INCORRECT_CHAR_HTML = "&#10008;";


String generateCodeAreaHTML(String code) {
	String startTextArea = "<textarea rows=\"20\" cols=\"50\" name=\"codesnippet_editable\" id=\"codesnippet_editable\"/>";
	String endTextArea = "</textarea>";

	return startTextArea + code + endTextArea;
}

String generateErrorAreaHTML(String errorMsg, bool hasError) {
	String errorborder = hasError ? "border-color:RED;" : "border-color:GREEN;" ;
	String startTextArea = "<textarea readonly rows=\"30\" style=\"font-family: Consolas, monaco, monospace;font-size:14pt;width:50%;border:5px;border-style:dashed;display: block;margin-left: auto;margin-right: auto;"+ errorborder +"\">";
	String endTextArea = "</textarea>";

	return startTextArea + errorMsg + endTextArea;
}

String generateHTMLPage(String code, String errorMsg, bool hasError)
{
	return 
		HOME_PAGE_HEADER +
		generateCodeAreaHTML (code) +
		HOME_PAGE_BUTTON + 
		generateErrorAreaHTML(errorMsg, hasError) +
		HOME_PAGE_END;
}

void initState() {
	for(int i = 0; i < MAX_LEDS; i++) {
		digitalWrite(LED_PIN[i], LOW);
	}
	digitalWrite(SOUND_PIN, LOW);
	servo1.write(90);
}

void setup() {
	Serial.begin(115200);

	for(int i = 0; i < MAX_LEDS; i++) {
		pinMode(LED_PIN[i], OUTPUT);
	}
	pinMode(SOUND_PIN, OUTPUT);

	servo1.attach(SERVO1);
	servo2.attach(SERVO2);

	if(connectToWiFi()) {
		Serial.println("Connected and set for requests ...");
	}
}

void loop() {
	server.handleClient();
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
	server.on("/", compileAndRun);

	// start the server
	Serial.println("Starting the server ... ");
	server.begin(); 

	return true;
}


void compileAndRun()
{
  if (server.hasArg("codesnippet_editable")) {
    compile();
  }
  else {
    server.send(200, "text/html", generateHTMLPage(SAMPLE_CODE, "No Error!", false));
  }
}

void compile()
{
  String code , message = "";

  int startIndex = 0, readIndex = 0;
  int lineNumber = 0;

  bool hasError = false;

  code = server.arg("codesnippet_editable");

  if(code != "") {
  	Serial.println("Got message : ");
  	Serial.println(code);

  	// read lines
  	while(readIndex >= 0) {
  		readIndex = code.indexOf('\n', startIndex);
  		String line = code.substring(startIndex, readIndex);
  		startIndex = readIndex+1;

  		Serial.print("Read line: ");
  		Serial.println(line);

  		lineNumber++; 

  		line.trim(); // trim any white spaces

  		if(line == "")
  			continue; // just continue on whitespace lines

  		if(line.startsWith("//") || line.startsWith("START") || line.startsWith("STOP")) {// ignore the whole line
  			Serial.print("Found comment .. ignoring : ");
  			Serial.println(line);
  			message += getLogMessageLine (lineNumber, CORRECT_CHAR_HTML, line, "Comment found -- No problem !");
  			continue;
  		}

  		// change the line to upper case
  		String originalLine = line; // copy of the code line
  		line.toUpperCase();

  		if(line.startsWith("ON")) { // turning on the LED!
  			line = line.substring(2);
  			line.trim(); // trim any white spaces;
  			if(line.startsWith("LED")) {
  				line = line.substring(3);
  				line.trim();
  				int led = line.toInt();
  				if(led < 1 || led > MAX_LEDS) {
  					message += getLogMessageLine (lineNumber, INCORRECT_CHAR_HTML, originalLine, "LED number is incorrect or not provided !");
  					hasError = true;
  					break;
  				} else {
	  				Serial.print("Turning ON the LED : ");
  					Serial.println(led);
  					message += getLogMessageLine (lineNumber, CORRECT_CHAR_HTML, originalLine, "DONE !");
  					digitalWrite(LED_PIN[led-1], HIGH);
  				}
  			} else {
  				message += getLogMessageLine (lineNumber, INCORRECT_CHAR_HTML, originalLine, "You can use only LED after ON !! - try ON LED 1");
  				hasError = true;
  				break;  				
  			}
  		} else if(line.startsWith("OFF")) { // turning OFF the LED!
  			line = line.substring(3);
  			line.trim(); // trim any white spaces;
  			if(line.startsWith("LED")) {
  				line = line.substring(3);
  				line.trim();
  				int led = line.toInt();
  				if(led < 1 || led > MAX_LEDS) {
  					message += getLogMessageLine (lineNumber, INCORRECT_CHAR_HTML, originalLine, "LED number is incorrect or not provided !");
   					hasError = true;
  					break;
  				} else {
  					// turn off the right LED
	  				Serial.print("Turning off the LED : ");
  					Serial.println(led);
  					message += getLogMessageLine (lineNumber, CORRECT_CHAR_HTML, originalLine, "DONE !");
  					digitalWrite(LED_PIN[led-1], LOW);
  				}
  			} else {
  				message += getLogMessageLine (lineNumber, INCORRECT_CHAR_HTML, originalLine, "You can use only LED after OFF !! - try OFF LED 1");
  				hasError = true;
  				break;  				
  			}
  		} else if(line.startsWith("WAIT")) { // turning OFF the LED!
  			line = line.substring(4);
  			line.trim(); // trim any white spaces;
  			int time = line.toInt(); // assume wait time is provided
  			if(time < 0 || time > MAX_WAIT) {
  				message += getLogMessageLine (lineNumber, INCORRECT_CHAR_HTML, originalLine, "You must provide a number after WAIT or it must be between 1 and 20 !");
  				hasError = true;
  				break;
  			} else {
  				Serial.print("Wait time provided : ");
  				Serial.println(time);
  				message += getLogMessageLine (lineNumber, CORRECT_CHAR_HTML, originalLine, "DONE !");
  				delay(time*1000);
  				// process a delay
  			}
  		} else if(line.startsWith("MOTOR")) { // turing the motor left and right!
  			line = line.substring(5);
  			line.trim(); // trim any white spaces;
  			if(line.startsWith("LEFT")) {
  				servo1.write(180);
  				Serial.print("Turing the motor LEFT ");
  				message += getLogMessageLine (lineNumber, CORRECT_CHAR_HTML, originalLine, "DONE !");
  			} else if (line.startsWith("RIGHT")){
  				Serial.print("Turing the motor RIGHT ");
  				servo1.write(0);
  				message += getLogMessageLine (lineNumber, CORRECT_CHAR_HTML, originalLine, "DONE !");
  			} else {
  				message += getLogMessageLine (lineNumber, INCORRECT_CHAR_HTML, originalLine, "You can turn the motor either LEFT or RIGHT only !");
  				hasError = true;
  				break;
  			}
  		} else if(line.startsWith("SOUND")) { // turing the motor left and right!
  			line = line.substring(5);
  			line.trim(); // trim any white spaces;
  			if(line.startsWith("ON")) {
  				digitalWrite(SOUND_PIN, HIGH);
  				Serial.print("Turing the SOUND ON ");
  				message += getLogMessageLine (lineNumber, CORRECT_CHAR_HTML, originalLine, "DONE !");
  			} else if (line.startsWith("OFF")){
  				Serial.print("Turing the SOUND OFF ");
  				digitalWrite(SOUND_PIN, LOW);
  				message += getLogMessageLine (lineNumber, CORRECT_CHAR_HTML, originalLine, "DONE !");
  			} else {
  				message += getLogMessageLine (lineNumber, INCORRECT_CHAR_HTML, originalLine, "You can turn the motor either LEFT or RIGHT only !");
  				hasError = true;
  				break;
  			}
  		} else {
  			message += getLogMessageLine (lineNumber, INCORRECT_CHAR_HTML, originalLine, "Hmmm .. I dont understand this line :( !");
  			hasError = true;
  			break;
  		}				
  	}
  }

  Serial.println("Log: ");
  Serial.println(message);
  String yahMsg = hasError ? "Ooooops! something is wrong! please see below: \n" : "Good job! All good ...\n" ;
  server.send(200, "text/html", generateHTMLPage(code, yahMsg + message, hasError));
  initState();
}

String getLogMessageLine(int lineNumber, String isCorrect, String codeLine, String message) {
	String logMsg =  "Processing Line : " + String(lineNumber);
	logMsg = logMsg + ":" + isCorrect + ":" + codeLine;
	logMsg = logMsg + " <<< " + message + "\n";

	Serial.println("getLogMessageLine");
	Serial.println(logMsg);

	return logMsg;
}

