#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include <WiFi.h>

// Replace with your network credentials
const char* ssid = "caravello";
const char* password = "v3r1z0n1";

// Set web server port number to 80
WiFiServer server(80);

// Create the motor shield object with the default I2C address
Adafruit_MotorShield AFMS = Adafruit_MotorShield(); 
 
// Select which 'port' M1, M2, M3 or M4. In this case, M1
Adafruit_DCMotor *leftTrack = AFMS.getMotor(1);

// You can also make another motor on port M2
Adafruit_DCMotor *rightTrack = AFMS.getMotor(2);

int maxspeed = 50;

// Variable to store the HTTP request
String header;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

int leftState = 0;
int rightState = 0;

void setup() {
	Serial.begin(115200);           // set up Serial library at 9600 bps
	Serial.println("Motor test!");

	bool wireStatus = Wire1.begin(21,22);
	AFMS.begin(0x60,&Wire1);

	leftTrack->setSpeed(0);
	leftTrack->run(FORWARD);
	rightTrack->setSpeed(0);
	rightTrack->run(FORWARD);

	Serial.print("Connecting to ");
	Serial.println(ssid);
	WiFi.begin(ssid, password);
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}
	Serial.println("Connected");
	Serial.println("IP address: ");
	Serial.println(WiFi.localIP());

	server.begin();

	Serial.println("RoboTank Ready");
}

void loop() {
	WiFiClient client = server.available();   // Listen for incoming clients

	if (client) {                             // If a new client connects,
		currentTime = millis();
		previousTime = currentTime;
		Serial.println("New Client.");          // print a message out in the serial port
		String currentLine = "";                // make a String to hold incoming data from the client
		while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
			currentTime = millis();
			if (client.available()) {             // if there's bytes to read from the client,
				char c = client.read();             // read a byte, then
				Serial.write(c);                    // print it out the serial monitor
				header += c;
				if (c == '\n') {                    // if the byte is a newline character

				// if the current line is blank, you got two newline characters in a row.
				// that's the end of the client HTTP request, so send a response:
				if (currentLine.length() == 0) {
					// HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
					// and a content-type so the client knows what's coming, then a blank line:
					client.println("HTTP/1.1 200 OK");
					client.println("Content-type:text/html");
					client.println("Connection: close");
					client.println();

					// turns the GPIOs on and off
					if (header.indexOf("GET /left/forward") >= 0) {
						Serial.println("Left Forward");
						leftState = 1;
						leftTrack->run(FORWARD);
						leftTrack->setSpeed(maxspeed);
					} else if (header.indexOf("GET /left/reverse") >= 0) {
						Serial.println("Left Reverse");
						leftState = -1;
						leftTrack->run(BACKWARD);
						leftTrack->setSpeed(maxspeed);
					} else if (header.indexOf("GET /left/stop") >= 0) {
						Serial.println("Left Stop");
						leftState = 0;
						leftTrack->run(RELEASE);
						leftTrack->setSpeed(maxspeed);
					} else if (header.indexOf("GET /right/forward") >= 0) {
						Serial.println("Right Forward");
						rightState = 1;
						rightTrack->run(FORWARD);
						rightTrack->setSpeed(maxspeed);
					} else if (header.indexOf("GET /right/reverse") >= 0) {
						Serial.println("Right Reverse");
						rightState = -1;
						rightTrack->run(BACKWARD);
						rightTrack->setSpeed(maxspeed);
					} else if (header.indexOf("GET /right/stop") >= 0) {
						Serial.println("Right Stop");
						rightState = 0;
						rightTrack->run(RELEASE);
						rightTrack->setSpeed(maxspeed);
					}

					// Display the HTML web page
					client.println("<!DOCTYPE html><html>");
					client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
					client.println("<link rel=\"icon\" href=\"data:,\">");

					// CSS to style the on/off buttons 
					// Feel free to change the background-color and font-size attributes to fit your preferences
					client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
					client.println(".button { display: inline-block; float: left; background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
					client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
					client.println(".button2 { display: inline-block; float: left; background-color: #555555;}</style></head>");
					client.println("</p>");

					// Web Page Heading
					client.println("<body><h1>RoboTank!!!</h1>");

					// Display current state and control buttons for Left Tread
					client.println("<p>Left State</p><p style=\"display: block;\">");
					if (leftState == 1) {
						client.println("<p><a href=\"/left/forward\"><button class=\"button\">FORWARD</button></a></p>");
						client.println("<p><a href=\"/left/stop\"><button class=\"button button2\">STOP</button></a></p>");
						client.println("<p><a href=\"/left/reverse\"><button class=\"button button2\">REVERSE</button></a></p>");
					}
					else if (leftState == -1) {
						client.println("<p><a href=\"/left/forward\"><button class=\"button button2\">FORWARD</button></a></p>");
						client.println("<p><a href=\"/left/stop\"><button class=\"button button2\">STOP</button></a></p>");
						client.println("<p><a href=\"/left/reverse\"><button class=\"button\">REVERSE</button></a></p>");
					}
					else {
						client.println("<p><a href=\"/left/forward\"><button class=\"button button2\">FORWARD</button></a></p>");
						client.println("<p><a href=\"/left/stop\"><button class=\"button\">STOP</button></a></p>");
						client.println("<p><a href=\"/left/reverse\"><button class=\"button button2\">REVERSE</button></a></p>");
					}
					client.println("</p>");

					client.println("<hr style=\"clear: both;\">");

					// Display current state and control buttons for Right Tread
					client.println("<p>Right State</p><p style=\"display: block;\">");
					if (rightState == 1) {
						client.println("<p><a href=\"/right/forward\"><button class=\"button\">FORWARD</button></a></p>");
						client.println("<p><a href=\"/right/stop\"><button class=\"button button2\">STOP</button></a></p>");
						client.println("<p><a href=\"/right/reverse\"><button class=\"button button2\">REVERSE</button></a></p>");
					}
					else if (rightState == -1) {
						client.println("<p><a href=\"/right/forward\"><button class=\"button button2\">FORWARD</button></a></p>");
						client.println("<p><a href=\"/right/stop\"><button class=\"button button2\">STOP</button></a></p>");
						client.println("<p><a href=\"/right/reverse\"><button class=\"button\">REVERSE</button></a></p>");
					}
					else {
						client.println("<p><a href=\"/right/forward\"><button class=\"button button2\">FORWARD</button></a></p>");
						client.println("<p><a href=\"/right/stop\"><button class=\"button\">STOP</button></a></p>");
						client.println("<p><a href=\"/right/reverse\"><button class=\"button button2\">REVERSE</button></a></p>");
					}
					client.println("</p>");

					client.println("</body></html>");

					// The HTTP response ends with another blank line
					client.println();

					// Break out of the while loop
					break;
				}
				else { // if you got a newline, then clear currentLine
					currentLine = "";
				}
				}
				else if (c != '\r') {  // if you got anything else but a carriage return character,
					currentLine += c;      // add it to the end of the currentLine
				}
			}
		}
		// Clear the header variable
		header = "";
		// Close the connection
		client.stop();
		Serial.println("Client disconnected.");
		Serial.println("");
	}
}