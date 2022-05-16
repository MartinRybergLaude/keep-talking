// 2A: Shared drawing canvas (Server)
import processing.serial.*;
import java.lang.Math;
import processing.net.*;

Client c;
Serial port;

GameState gameState = GameState.INIT;
int serialValue = 0;

void setup() {
  String portName = Serial.list()[2];
  port = new Serial(this, portName, 9600);
  
  size(450, 255);
  background(204);
  stroke(0);
  frameRate(5);
  
 // c = new Client(this, "130.229.186.236", 12345);

}
void draw() {
  //if (!c.active()) return;
  
  //int retrievedGameState = 0;
  //gameState = GameState.values()[retrievedGameState];
  //port.write(retrievedGameState);
  setSerialValue();
  
  switch(gameState) {
    case INIT:
      handleInit();
      break;
    case PLAY_1:
      break;
    case PLAY_2:
      break;
    case PLAY_3:
      break;
    case END:
      break;
  }
  delay(100);
}

void handleInit() {
   if (serialValue == 1) {
     println("1");
   } else {
     println("0");
   }
}

void setSerialValue() {
  String val = "";
  if (port.available() > 0) {
      val = port.readStringUntil('\n').replaceAll("\\s+",""); 
  }
  if (val == null || val.length() == 0) {
    return;
  }
  try {
    serialValue = Integer.parseInt(val);
  } catch(Exception e) {
    return;
  } //<>//
}

enum GameState {
  INIT,
  PLAY_1,
  PLAY_2,
  PLAY_3,
  END
}

class ProtocolUp {
   public int PotentioMeter;
   public boolean HasCompleted;
   
   public ProtocolUp(int potentioMeter, boolean hasCompleted) {
      this.PotentioMeter = potentioMeter;
      this.HasCompleted = hasCompleted;
   }
}
