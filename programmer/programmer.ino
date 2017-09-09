#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#define ssid  ""
#define password ""

// convert HTML in https://www.freeformatter.com/java-dotnet-escape.html#ad-output
String localIP = "";

// WebServer to serve the HTML !
ESP8266WebServer server;

// not a good solution - but part mash up ... 
const String HOME_PAGE_HEADER = "<!DOCTYPE html>\r\n<html>\r\n<head> \r\n    <link rel=\"stylesheet\" href=\"http://codemirror.net/lib/codemirror.css\">\r\n    <script src=\"http://codemirror.net/lib/codemirror.js\"></script>\r\n    <script src=\"http://codemirror.net/addon/mode/simple.js\"></script>\r\n    <script src=\"http://codemirror.net/addon/edit/matchbrackets.js\"></script>\r\n    <script src=\"http://codemirror.net/mode/javascript/javascript.js\"></script>  \r\n\t<title>Programmer</title>\r\n</head>\r\n<style> \r\n\t@namespace url(http://www.w3.org/1999/xhtml);\r\n\t.CodeMirror {\r\n\t    font-family:\"Ubuntu Mono\", monospace !important;\r\n\t    font-size:18pt !important;\r\n\t}\r\n\t.CodeMirror { height: 100%; }\r\n\t.button {\r\n\t  display: inline-block;\r\n\t  padding: 15px 25px;\r\n\t  font-size: 24px;\r\n\t  cursor: pointer;\r\n\t  text-align: center;\r\n\t  text-decoration: none;\r\n\t  outline: none;\r\n\t  color: #fff;\r\n\t  background-color: #4CAF50;\r\n\t  border: none;\r\n\t  border-radius: 15px;\r\n\t  box-shadow: 0 9px #999;\r\n\t}\t\r\n</style>\r\n<script type=\"text/javascript\">\r\n\twindow.onload = function () {\r\n\tCodeMirror.defineSimpleMode(\"iotcode\", {\r\n\t  start: [\r\n\t    {regex: /\"(?:[^\\\\]|\\\\.)*?(?:\"|$)/, token: \"string\"},\r\n\t    {regex: /(function)(\\s+)([a-z$][\\w$]*)/,\r\n\t     token: [\"keyword\", null, \"variable-2\"]},\r\n\t    {regex: /(?:STOP|START|LED|MOTOR|WAIT|GOTO|SOUND)\\b/,\r\n\t     token: \"keyword\"},\r\n\t    {regex: /true|false|null|LEFT|RIGHT|ON|OFF|undefined/, token: \"atom\"},\r\n\t    {regex: /0x[a-f\\d]+|[-+]?(?:\\.\\d+|\\d+\\.?\\d*)(?:e[-+]?\\d+)?/i,\r\n\t     token: \"number\"},\r\n\t    {regex: /\\/\\/.*/, token: \"comment\"},\r\n\t    {regex: /\\/(?:[^\\\\]|\\\\.)*?\\//, token: \"variable-3\"},\r\n\t    {regex: /\\/\\*/, token: \"comment\", next: \"comment\"},\r\n\t    {regex: /[-+\\/*=<>!]+/, token: \"operator\"},\r\n\t    {regex: /[\\{\\[\\(]/, indent: true},\r\n\t    {regex: /[\\}\\]\\)]/, dedent: true},\r\n\t    {regex: /[a-z$][\\w$]*/, token: \"variable\"},\r\n\t    {regex: /<</, token: \"meta\", mode: {spec: \"xml\", end: />>/}}\r\n\t  ],\r\n\t  comment: [\r\n\t    {regex: /.*?\\*\\//, token: \"comment\", next: \"start\"},\r\n\t    {regex: /.*/, token: \"comment\"}\r\n\t  ],\r\n\t  meta: {\r\n\t    dontIndentStates: [\"comment\"],\r\n\t    lineComment: \"//\"\r\n\t  }\r\n\t});    \r\n\tvar editableCodeMirror = CodeMirror.fromTextArea(document.getElementById('codesnippet_editable'), {\r\n\t        mode: \"iotcode\",\r\n\t        theme: \"default\",\r\n\t        lineNumbers: true\r\n\t    });\r\n\t};\r\n</script>\r\n<body>\r\n<h1 align=\"center\" style=\"font-family: Consolas, monaco, monospace;\"> Welcome to IoT Programmer !</h1>\r\n\r\n<form action=\"/\" method=\"post\">\r\n  <br>\r\n  <div style=\"border: 2px;border-style: solid;\">";  
const String HOME_PAGE_BUTTON = "\t</div>\r\n\t<br>\r\n  <button align=\"center\" class=\"button\" type=\"submit\" value=\"Submit!\"> Run my code !</button> <br>\t\r\n  <br>";
const String HOME_PAGE_END = "</form> \r\n</body>\r\n</html>";

const String SAMPLE_CODE = "START\r\nON LED 1\r\nWAIT 5\r\nON LED 2\r\nWAIT 5\r\nON LED 3\r\nWAIT 5\r\nON LED 4\r\nMOTOR LEFT\r\nWAIT 5\r\nMOTOR RIGHT \r\nWAIT 5\r\nSTOP";

String generateCodeAreaHTML(String code) {
	String startTextArea = "<textarea rows=\"20\" cols=\"50\" name=\"codesnippet_editable\" id=\"codesnippet_editable\"/>";
	String endTextArea = "</textarea>";

	return startTextArea + code + endTextArea;
}

String generateErrorAreaHTML(String errorMsg) {
	String startTextArea = "<textarea readonly rows=\"10\" style=\"font-family: Consolas, monaco, monospace;font-size:14pt;width:50%;border:2px;border-style:dashed;display: block;margin-left: auto;margin-right: auto;\">";
	String endTextArea = "</textarea>";

	return startTextArea + errorMsg + endTextArea;
}

String generateHTMLPage(String code, String errorMsg)
{
	return 
		HOME_PAGE_HEADER +
		generateCodeAreaHTML (code) +
		HOME_PAGE_BUTTON + 
		generateErrorAreaHTML(errorMsg) +
		HOME_PAGE_END;
}

void setup() {
	Serial.begin(115200);

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
    server.send(200, "text/html", generateHTMLPage(SAMPLE_CODE, "No Error!"));
  }
}

void compile()
{
  String code , message = "";

  int startIndex = 0, readIndex = 0;

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

  		message = message + "DONE : " + line;
  	}
  }

  server.send(200, "text/html", generateHTMLPage(code, "Compiled the following code \n" + message));
}

