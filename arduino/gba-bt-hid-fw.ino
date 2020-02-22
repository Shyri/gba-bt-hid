#include<SPI.h>

#define SS_PIN 10
#define MUX_PIN 5

#define ROM_WRITE_PIN 2
#define MB_PIN 4
#define UART_PIN 3

#define COMMAND_DELETE_REQUEST 0xA0
#define COMMAND_DELETE_REQUEST_ACK 0xA1
#define COMMAND_DELETED 0xA2
#define COMMAND_ERROR 0xA5
#define COMMAND_SUCCESS 0xA6


void dumpRom(uint32_t romLength);
uint32_t readROMLength();
uint32_t romLength;
long fcnt = 0;
int writeMode = 0;

void setup() {
  Serial.begin(115200);
  
  pinMode(SS_PIN, OUTPUT);
  digitalWrite(SS_PIN, HIGH);
  
  pinMode(MUX_PIN, OUTPUT);
  digitalWrite(MUX_PIN, HIGH);
  
  pinMode(MB_PIN, OUTPUT);
  digitalWrite(MB_PIN, HIGH);
  
  pinMode(UART_PIN, OUTPUT);
  digitalWrite(UART_PIN, LOW);

  pinMode(ROM_WRITE_PIN, INPUT);
  writeMode = digitalRead(ROM_WRITE_PIN); 
}

void loop() {
  if(writeMode) {
     SPI.begin();
     writeROMRoutine();
     while (1) {}
  } else {
    Serial.println("GBA Bluetoot HID");
    SPI.begin();
    Serial.print("ROM Size: ");
    romLength = readROMLength();
    Serial.println(romLength, HEX);
    SPI.end();
    multiboot();
    while (1) {}
  }
}

int readCommand() {
  while (Serial.available() == 0);
  return Serial.read();
}

void sendCommand(int command) {
  Serial.write(COMMAND_DELETED);
}

void writeROMRoutine() {
  int incomingByte = readCommand();
  
  if(incomingByte == COMMAND_DELETE_REQUEST) {
    Serial.write(COMMAND_DELETE_REQUEST_ACK);
    
    if(chipErase()) {
      Serial.write(COMMAND_DELETED); 
    } else {
      Serial.write(COMMAND_ERROR); 
    }
  }

  int size1 = readCommand();
  int size2 = readCommand();
  int size3 = readCommand();
  int size4 = readCommand();

  uint32_t romSize = ((size1 << 24) | (size2 << 16) | (size3 << 8) | size4) & 0xFFFF;
  
  uint32_t address = 0;
  
  writeROMByte(address, size1);
  address++;
  writeROMByte(address, size2);
  address++;
  writeROMByte(address, size3);
  address++;
  writeROMByte(address, size4);
  address++;

  enableROMWrite(address);

  int bytesWritten = 0x04;
  while(address < romSize + 4) {
    if(bytesWritten == 0x100) {
      commitROMWrite();
      bytesWritten = 0x00;
      while(!enableROMWrite(address));
    }
    int romByte = readCommand();
    SPI.transfer(romByte);
    //writeROMByte(address, romByte);
    Serial.write(romByte);
    bytesWritten++;
    address++;
  }

  commitROMWrite();

  Serial.write(COMMAND_SUCCESS);
}

void dumpRom(uint32_t page) {
  digitalWrite(SS_PIN, LOW);
  
  SPI.transfer(0x03);
  
  SPI.transfer((page >> 8) & 0xFF);
  SPI.transfer(page & 0xFF);
  SPI.transfer(0x00);

  int j = 0;
  for(uint32_t i = 0; i < 0x100; i++) {
    int rv = SPI.transfer(0x00);
    Serial.print("0x");
    Serial.print(rv, HEX);
    Serial.print(" ");
    j++;
    if(j == 16) {
      Serial.println(" ");
      j = 0;
    }
  }

  digitalWrite(SS_PIN, HIGH);
}
