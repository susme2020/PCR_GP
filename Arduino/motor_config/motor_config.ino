  #include <SPI.h>
#include "mcp_can.h"

const int SPI_CS_PIN = 5; // Pin CS del MCP2515
MCP_CAN CAN(SPI_CS_PIN);

// ID del mensaje CAN para controlar la velocidad del motor
const unsigned long SPEED_COMMAND_ID = 0x141;
const unsigned long SPEED_COMMAND_ID2 = 0x142;
const unsigned long SPEED_COMMAND_ID3 = 0x143;
const unsigned long CONFIGURE_ID = 0x300;
// ID del mensaje CAN que el motor utiliza para responder con los datos del estado
const unsigned long RESPONSE_STATUS_ID = 0x241;

unsigned long ID_command = 0x141;
int ID = 0;
char receivedChar;
boolean newData = false;
bool no_iniciado = true;

void setup() {
    Serial.begin(115200);
    
    if (CAN.begin(MCP_ANY, CAN_1000KBPS, MCP_8MHZ) == CAN_OK) { // Set baud rate to 1 Mbps
        Serial.println("MCP2515 Initialized Successfully!");
    } else {
        Serial.println("Error Initializing MCP2515...");
        while (1); // Detiene el programa si hay un error en la inicialización
    }

    CAN.setMode(MCP_NORMAL);
    delay(2000);
    Serial.println("Robot listo!");
    while (no_iniciado){
      recvOneChar();
      showNewData();
    }
}

void loop() {
  SendOneSpeed(400, 10000, ID);
  StopMotor(400, ID);
  SendOneSpeed(400, -10000, ID);
  StopMotor(400, ID);
  readMotorID();
  if (CAN_MSGAVAIL == CAN.checkReceive()) {
      long unsigned int rxID;
      unsigned char len;
      unsigned char buf[8];
      
      CAN.readMsgBuf(&rxID, &len, buf);
      Serial.print("Mensaje CAN recibido con ID: ");
      Serial.println(rxID, HEX);
      for (int i = 0; i < len; i++) {
          Serial.print(buf[i], HEX);
          Serial.print(" ");
      }
      Serial.println();
  }
  delay(100);
}

void readMotorID() {
    // Comando para leer el CANID actual
    byte commandData[8] = {0x79, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00};

    // Enviar el mensaje con ID del motor
    if (CAN.sendMsgBuf(CONFIGURE_ID, 0, 8, commandData) == CAN_OK) {
        Serial.print("Comando de lectura de CANID enviado correctamente al Motor con ID: ");
    } else {
        Serial.println("Error al enviar comando de lectura de CANID.");
    }
}
void SetCAN_ID(int ID) {
    // Command to configure CAN ID
    byte commandData[8] = {0x79, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
    if (ID == 1){
      byte commandData[8] = {0x79, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
    } else if (ID == 2){
      byte commandData[8] = {0x79, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02};
    } else if (ID == 3){
      byte commandData[8] = {0x79, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03};
    }

    if (CAN.sendMsgBuf(CONFIGURE_ID, 0, 8, commandData) == CAN_OK) {
        Serial.println("SetCan ID Command Sent Successfully!");
    } else {
        Serial.println("Error Sending SetCan ID Command...");
    }
}
void SystemReset(int ID){
    // Command to initiate the postion zero
    byte commandData[8] = {0x76, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    ID_command = SPEED_COMMAND_ID;
    if (ID == 1){
      ID_command = SPEED_COMMAND_ID;
    } else if (ID == 2){
      ID_command = SPEED_COMMAND_ID2;
    } else if (ID == 3){
      ID_command = SPEED_COMMAND_ID3;
    }
    if (CAN.sendMsgBuf(ID_command, 0, 8, commandData) == CAN_OK) {
        Serial.println("SystemReset Command Sent Successfully!");
    } else {
        Serial.println("Error Sending SystemReset Command...");
    }
    delay(10);
}
void Shutdown(){
    byte commandData[8] = {0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    if (CAN.sendMsgBuf(CONFIGURE_ID, 0, 8, commandData) == CAN_OK) {
        Serial.println("Shutdown Command Sent Successfully!");
    } else {
        Serial.println("Error Sending Shutdown Command...");
    }
    delay(1000);
}
void ErrorFlag(){
    byte commandData[8] = {0x9A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    if (CAN.sendMsgBuf(SPEED_COMMAND_ID, 0, 8, commandData) == CAN_OK) {
        Serial.println("Error Flag Command Sent Successfully!");
    } else {
        Serial.println("Error Sending Error Flag Command...");
    }
    delay(1000);
}
void EN_CANID_FILTER(int ID) {
    // Command to configure CAN ID
    byte commandData[8] = {0x20, 0x02, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00};
    ID_command = SPEED_COMMAND_ID;
    if (ID == 1){
      ID_command = SPEED_COMMAND_ID;
    } else if (ID == 2){
      ID_command = SPEED_COMMAND_ID2;
    } else if (ID == 3){
      ID_command = SPEED_COMMAND_ID3;
    }
    if (CAN.sendMsgBuf(ID_command, 0, 8, commandData) == CAN_OK) {
        Serial.println("Enable CanID Filter Command Sent Successfully!");
    } else {
        Serial.println("Error Sending Enable CanID Filter Command...");
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
          ID = 1;
          ConfigurarMotor(ID);
          no_iniciado = false;
        } else if (receivedChar == '2'){
          ID = 2;
          ConfigurarMotor(ID);
          no_iniciado = false;
        } else if (receivedChar == '3'){
          ID = 3;
          ConfigurarMotor(ID);
          no_iniciado = false;
        }
        if (CAN_MSGAVAIL == CAN.checkReceive()) {
        long unsigned int rxID;
        unsigned char len;
        unsigned char buf[8];
        
        CAN.readMsgBuf(&rxID, &len, buf);
        Serial.print("Mensaje CAN recibido con ID: ");
        Serial.println(rxID, HEX);
        for (int i = 0; i < len; i++) {
            Serial.print(buf[i], HEX);
            Serial.print(" ");
        }
        Serial.println();
      }
    }
}
void ConfigurarMotor(int ID) {
  Serial.print("Configurando Motor ");
  Serial.print(ID);
  Serial.println("  ...");
  SystemReset(ID);
  delay(2000);
  SetCAN_ID(ID);
  delay(2000);
  EN_CANID_FILTER(ID);
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