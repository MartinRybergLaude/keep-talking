// 2A: Shared drawing canvas (Server)
import processing.serial.*;
import java.lang.Math;
import processing.net.*;

Client c;
Serial myPort;

int serialValue = 0;
int data = 0;
boolean firstContact = false;
int gameState = 0;

void setup() {
  String portName = Serial.list()[2];
  myPort = new Serial(this, portName, 9600);
  c = new Client(this, "130.229.1550.24", 12345);
}
void draw() {
  serialEvent();
  
  // Receive data from client
  if (!c.active()) {
    String input = c.readString();
    input = input.substring(0, input.indexOf("\n")); 
    gameState = Integer.parseInt(input);
    myPort.write(serialValue);
  } 
}

void serialEvent() {
  String myString = myPort.readStringUntil('\n');
  // Ignore other bytes than linefeed
  if (myString != null) {
    myString = trim(myString);
 
    // Listen for initial contact
    if (firstContact == false) {
      if (myString.equals("hello")) {
        myPort.clear();   
        firstContact = true;
        println("HANDSHAKE");
        myPort.write(gameState);       
      }
    }
    // Successful handshake
    else {
      serialValue = int(myString);
      println(serialValue);
    }
    // Ask for additional data
    myPort.write(gameState);
  }
}
