#include <SPI.h>
#include "mcp_can.h"

const int SPI_CS_PIN = 5; // Pin CS del MCP2515
MCP_CAN CAN(SPI_CS_PIN);

// ID del mensaje CAN para controlar la velocidad del motor
const unsigned long SPEED_COMMAND_ID = 0x141;
const unsigned long SPEED_COMMAND_ID2 = 0x142;
const unsigned long SPEED_COMMAND_ID3 = 0x143;
const unsigned long CONFIGURE_ID = 0x300;
unsigned long ID_command = 0x141;


// ID del mensaje CAN que el motor utiliza para responder con los datos del estado
const unsigned long RESPONSE_STATUS_ID = 0x241;
const unsigned long RESPONSE_STATUS_ID2 = 0x242;
const unsigned long RESPONSE_STATUS_ID3 = 0x243;

int t = 0;
int ID = 1;
int speed = 0;
char receivedChar;
boolean newData = false;
bool no_iniciado = true;
bool modo1 = false;
bool modo2 = false;
bool modo3 = false;


void setup() {
    Serial.begin(115200);
    if (CAN.begin(MCP_ANY, CAN_1000KBPS, MCP_8MHZ) == CAN_OK) { // Set baud rate to 1 Mbps
        Serial.println("MCP2515 Initialized Successfully!");
    } else {
        Serial.println("Error Initializing MCP2515...");
        while (1); // Detiene el programa si hay un error en la inicialización
    }
    Serial.println("Iniciando Robot . . .");
    CAN.setMode(MCP_NORMAL);
    //ReferenceZeroMotor();
    delay(5000);
    Serial.println("Robot listo!");
    while (no_iniciado){
      recvOneChar();
      showNewData();
    }
    delay(1000);
}

void loop() {

    // Wait for motor's response
    /*
    Check_Status(1);
    delay(50);
    Check_Status(2);
    delay(50);
    Check_Status(3);
    delay(50);
    */
    recvOneChar();
    showNewData();

    if (modo1){
      SendOneSpeed(400, 10000000, 1);
      StopMotor(400, 1);
      SendOneSpeed(400, 10000000, 2);
      StopMotor(400, 2);
      //SendOneSpeed(300, 10000, 3);
      //StopMotor(400, 3);
      SendOneSpeed(400, -10000000, 1);
      StopMotor(400, 1);
      SendOneSpeed(400, -1000000, 2);
      StopMotor(400, 2);
      //SendOneSpeed(300, -10000, 3);
      //StopMotor(400, 3);
    } else if (modo2){
      SendOneSpeed(400, 1000000, 1);
      StopMotor(400, 1);
      SendOneSpeed(400, -100000, 2);
      StopMotor(400, 2);
      SendOneSpeed(400, -1000000, 1);
      StopMotor(400, 1);
      //SendOneSpeed(300, 10000, 3);
      //StopMotor(400, 3);
      SendOneSpeed(400, 1000000, 1);
      StopMotor(400, 1);
      SendOneSpeed(400, 100000, 2);
      StopMotor(400, 2);
      SendOneSpeed(400, -1000000, 1);
      StopMotor(400, 1);
      //SendOneSpeed(300, -10000, 3);
      //StopMotor(400, 3);
    } else if (modo3){
      StopMotor(400, 1);
      StopMotor(400, 2);
      //StopMotor(400, 3);
    }
}

void SendOneSpeed(int t, int speed, int ID) {
    // Command to set motor speed
    // Máximo es 4.294.967.295
    int32_t speedSetpoint = speed; // Example speed value, change as needed
    byte commandData[8] = {0xA2, 0x00, 0x00, 0x00,
                           (byte)(speedSetpoint & 0xFF), 
                           (byte)((speedSetpoint >> 8) & 0xFF), 
                           (byte)((speedSetpoint >> 16) & 0xFF), 
                           (byte)((speedSetpoint >> 24) & 0xFF)};
    ID_command = SPEED_COMMAND_ID;
    if (ID == 1){
      ID_command = SPEED_COMMAND_ID;
    } else if (ID == 2){
      ID_command = SPEED_COMMAND_ID2;
    } else if (ID == 3){
      ID_command = SPEED_COMMAND_ID3;
    }
    //Serial.println(ID_command, HEX);
    if (CAN.sendMsgBuf(ID_command, 0, 8, commandData) == CAN_OK) {
        Serial.println("Speed Command Sent Successfully!");
    } else {
        Serial.println("Error Sending Speed Command...");
    }
    delay(t);
}

void StopMotor(int t, int ID) {
    // Command to set motor speed to 0
    byte commandData[8] = {0x81, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    ID_command = SPEED_COMMAND_ID;
    if (ID == 1){
      ID_command = SPEED_COMMAND_ID;
    } else if (ID == 2){
      ID_command = SPEED_COMMAND_ID2;
    } else if (ID == 3){
      ID_command = SPEED_COMMAND_ID3;
    }
    //Serial.println(ID_command, HEX);
    if (CAN.sendMsgBuf(ID_command, 0, 8, commandData) == CAN_OK) {
        Serial.println("Stop Command Sent Successfully!");
    } else {
        Serial.println("Error Sending Stop Command...");
    }
    delay(t);
}

void ReferenceZeroMotor(){
    // Command to initiate the postion zero
    byte commandData[8] = {0x64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    
    if (CAN.sendMsgBuf(SPEED_COMMAND_ID, 0, 8, commandData) == CAN_OK) {
        Serial.println("ReferenceZeroMotor Command Sent Successfully!");
    } else {
        Serial.println("Error Sending ReferenceZeroMotor Command...");
    }
    delay(10);
}

void recvOneChar() {
    if (Serial.available() > 0) {
        receivedChar = Serial.read();
        newData = true;
    }
}

void showNewData() {
    if (newData == true) {
        Serial.println(receivedChar);
        newData = false;
         if (receivedChar == '1'){
          Serial.println("Iniciando modo 1 ...");
          modo1 = true;
          modo2 = false;
          modo3 = false;
          StopMotor(400, 1);
          StopMotor(400, 2);
          //StopMotor(400, 3);
          no_iniciado = false;
        } else if (receivedChar == '2'){
          Serial.println("Iniciando modo 2 ...");
          modo1 = false;
          modo2 = true;
          modo3 = false;
          StopMotor(400, 1);
          StopMotor(400, 2);
          //StopMotor(400, 3);
          no_iniciado = false;
        } else if (receivedChar == '3'){
          Serial.println("Iniciando modo 3 ...");
          modo1 = false;
          modo2 = false;
          modo3 = true;
          StopMotor(400, 1);
          StopMotor(400, 2);
          //StopMotor(400, 3);
          no_iniciado = false;
        }
    }
}

void Check_Status(int ID) {
  if (CAN_MSGAVAIL == CAN.checkReceive()) {
        long unsigned int rxId;
        unsigned char len = 0;
        unsigned char rxBuf[8];

        CAN.readMsgBuf(&rxId, &len, rxBuf);
        if (ID == 1){
          if (rxId == RESPONSE_STATUS_ID) {
            // Process the response
            int8_t temperature = rxBuf[1];
            int16_t torqueCurrent = (rxBuf[3] << 8) | rxBuf[2];
            int16_t speed = (rxBuf[5] << 8) | rxBuf[4];
            int16_t angle = (rxBuf[7] << 8) | rxBuf[6];
            
            Serial.println("Motor 1 - Status");
            Serial.print("Temperature: ");
            Serial.print(temperature);
            Serial.print(" °C");
            Serial.print(" |Torque Current: ");
            Serial.print(torqueCurrent * 0.01);
            Serial.print(" A");
            Serial.print(" |Speed: ");
            Serial.print(speed);
            Serial.print(" dps");
            Serial.print(" |Angle: ");
            Serial.print(angle);
            Serial.println(" degrees");
            } else {
                  Serial.println("No Message Available");
              }
        } else if(ID == 2){
          if (rxId == RESPONSE_STATUS_ID2) {
            // Process the response
            int8_t temperature = rxBuf[1];
            int16_t torqueCurrent = (rxBuf[3] << 8) | rxBuf[2];
            int16_t speed = (rxBuf[5] << 8) | rxBuf[4];
            int16_t angle = (rxBuf[7] << 8) | rxBuf[6];
            
            Serial.println("Motor 2 - Status");
            Serial.print("Temperature: ");
            Serial.print(temperature);
            Serial.print(" °C");
            Serial.print(" |Torque Current: ");
            Serial.print(torqueCurrent * 0.01);
            Serial.print(" A");
            Serial.print(" |Speed: ");
            Serial.print(speed);
            Serial.print(" dps");
            Serial.print(" |Angle: ");
            Serial.print(angle);
            Serial.println(" degrees");
        } else {
        Serial.println("No Message Available");
              }
        } else if (ID == 3){
          if (rxId == RESPONSE_STATUS_ID3) {
            // Process the response
            int8_t temperature = rxBuf[1];
            int16_t torqueCurrent = (rxBuf[3] << 8) | rxBuf[2];
            int16_t speed = (rxBuf[5] << 8) | rxBuf[4];
            int16_t angle = (rxBuf[7] << 8) | rxBuf[6];
            
            Serial.println("Motor 3 - Status");
            Serial.print("Temperature: ");
            Serial.print(temperature);
            Serial.print(" °C");
            Serial.print(" |Torque Current: ");
            Serial.print(torqueCurrent * 0.01);
            Serial.print(" A");
            Serial.print(" |Speed: ");
            Serial.print(speed);
            Serial.print(" dps");
            Serial.print(" |Angle: ");
            Serial.print(angle);
            Serial.println(" degrees");
        } else {
        Serial.println("No Message Available");
              } 
        }
    } 
}