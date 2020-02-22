uint32_t readROMLength() {
  digitalWrite(SS_PIN, LOW);
  
  SPI.transfer(0x03);
  
  SPI.transfer(0x00);
  SPI.transfer(0x00);
  SPI.transfer(0x00);

  uint32_t size1 = SPI.transfer(0x00);
  uint32_t size2 = SPI.transfer(0x00);
  uint32_t size3 = SPI.transfer(0x00);
  uint32_t size4 = SPI.transfer(0x00);

  uint32_t readRomSize = ((size1 << 24) | (size2 << 16) | (size3 << 8) | size4);

  digitalWrite(SS_PIN, HIGH);
  
  return readRomSize;
}

uint32_t read4ROMBytes(int startAddress) {
  digitalWrite(MUX_PIN, HIGH);
  delayMicroseconds(10);
  SPI.end();
  
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);
  SPI.begin();
  startAddress = startAddress + 0x04;
  
  digitalWrite(SS_PIN, LOW);
  
  uint32_t page = startAddress / 0x100;
  uint32_t startPoint = startAddress % 0x100;
  
  SPI.transfer(0x03);
  SPI.transfer((page >> 8) & 0xFF);
  SPI.transfer(page & 0xFF);
  SPI.transfer(startPoint);

  uint32_t byte1 = 0x00;
  uint32_t byte2 = 0x00;
  uint32_t byte3 = 0x00;
  uint32_t byte4 = 0x00;
  
  if(startAddress <= romLength) {
    byte1 = SPI.transfer(0x00);
  }

  if(startAddress + 1  <= romLength) {
    byte2 = SPI.transfer(0x00);
  }

  if(startAddress + 2  <= romLength) {
    byte3 = SPI.transfer(0x00);
  }

  if(startAddress + 3  <= romLength) {
    byte4 = SPI.transfer(0x00);
  }

  //w = rom[i] | (rom[i + 1] << 8) | (rom[i + 2] << 16) | (rom[i + 3] << 24) ;
  uint32_t fullByte = ((byte4 << 24) | (byte3 << 16) | (byte2 << 8) | byte1);
  digitalWrite(SS_PIN, HIGH);
  
  return fullByte;
}

void waitWhileBusy() {
  int rv;
  do {
    delay(10);
    
    digitalWrite(SS_PIN, LOW);
    SPI.transfer(0x05);
    rv = SPI.transfer(0x00);
    digitalWrite(SS_PIN, HIGH);
  } while(rv != 0);
}

bool checkWEL() {
  digitalWrite(SS_PIN, LOW);
  SPI.transfer(0x05);
  int rv = SPI.transfer(0x00);
  digitalWrite(SS_PIN, HIGH);

  return rv & 0x02;
}

bool chipErase() {
  waitWhileBusy();
  
  digitalWrite(SS_PIN, LOW);
  SPI.transfer(0x06);
  digitalWrite(SS_PIN, HIGH);
  
  if(checkWEL()) {
    digitalWrite(SS_PIN, LOW);
    SPI.transfer(0x60);
    digitalWrite(SS_PIN, HIGH);
  
    waitWhileBusy();
    return true;
  } else {
    return false;
  }
}

boolean enableROMWrite(uint32_t address) {
  waitWhileBusy();

  int page = (address >> 8) & 0xFFFF;
  int startByte = address & 0xFF;
  
  digitalWrite(SS_PIN, LOW);
  SPI.transfer(0x06);
  digitalWrite(SS_PIN, HIGH);

  if(checkWEL()) {
    digitalWrite(SS_PIN, LOW);
    SPI.transfer(0x02);
    
    SPI.transfer((page >> 8) & 0xFF);
    SPI.transfer(page & 0xFF);
    SPI.transfer(startByte);
    
    return true;
  } else {
    return false;
  }
}

void commitROMWrite() {
   digitalWrite(SS_PIN, HIGH);
    
   waitWhileBusy();
}

void writeROMByte(int address, int byte) {
  waitWhileBusy();
  
  int page = (address >> 8) & 0xFFFF;
  int startByte = address & 0xFF;
  
  digitalWrite(SS_PIN, LOW);
  SPI.transfer(0x06);
  digitalWrite(SS_PIN, HIGH);
  
  if(checkWEL()) {
    digitalWrite(SS_PIN, LOW);
    SPI.transfer(0x02);
    
    SPI.transfer((page >> 8) & 0xFF);
    SPI.transfer(page & 0xFF);
    SPI.transfer(startByte);

    SPI.transfer(byte);
    
    digitalWrite(SS_PIN, HIGH);
    
    waitWhileBusy();
  }
}
