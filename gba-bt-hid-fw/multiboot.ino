uint8_t header[] = {
0x2E, 0x00, 0x00, 0xEA, 0x24, 0xFF, 0xAE, 0x51, 0x69, 0x9A, 0xA2, 0x21, 0x3D, 0x84, 0x82, 0x0A, 
0x84, 0xE4, 0x09, 0xAD, 0x11, 0x24, 0x8B, 0x98, 0xC0, 0x81, 0x7F, 0x21, 0xA3, 0x52, 0xBE, 0x19, 
0x93, 0x09, 0xCE, 0x20, 0x10, 0x46, 0x4A, 0x4A, 0xF8, 0x27, 0x31, 0xEC, 0x58, 0xC7, 0xE8, 0x33, 
0x82, 0xE3, 0xCE, 0xBF, 0x85, 0xF4, 0xDF, 0x94, 0xCE, 0x4B, 0x09, 0xC1, 0x94, 0x56, 0x8A, 0xC0, 
0x13, 0x72, 0xA7, 0xFC, 0x9F, 0x84, 0x4D, 0x73, 0xA3, 0xCA, 0x9A, 0x61, 0x58, 0x97, 0xA3, 0x27, 
0xFC, 0x03, 0x98, 0x76, 0x23, 0x1D, 0xC7, 0x61, 0x03, 0x04, 0xAE, 0x56, 0xBF, 0x38, 0x84, 0x00, 
0x40, 0xA7, 0x0E, 0xFD, 0xFF, 0x52, 0xFE, 0x03, 0x6F, 0x95, 0x30, 0xF1, 0x97, 0xFB, 0xC0, 0x85, 
0x60, 0xD6, 0x80, 0x25, 0xA9, 0x63, 0xBE, 0x03, 0x01, 0x4E, 0x38, 0xE2, 0xF9, 0xA2, 0x34, 0xFF, 
0xBB, 0x3E, 0x03, 0x44, 0x78, 0x00, 0x90, 0xCB, 0x88, 0x11, 0x3A, 0x94, 0x65, 0xC0, 0x7C, 0x63, 
0x87, 0xF0, 0x3C, 0xAF, 0xD6, 0x25, 0xE4, 0x8B, 0x38, 0x0A, 0xAC, 0x72, 0x21, 0xD4, 0xF8, 0x07, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x30, 0x31, 0x96, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x00, 0x00};

uint32_t send(uint32_t command) {
  uint32_t rv[4];
  
  rv[0] = SPI.transfer((command >> 24) & 0xFF);
  rv[1] = SPI.transfer((command >> 16) & 0xFF);
  rv[2] = SPI.transfer((command >> 8) & 0xFF);
  rv[3] = SPI.transfer(command & 0xFF);

  delayMicroseconds(36);
  return rv[3] | (rv[2] << 8) | (rv[1] << 16) | (rv[0] << 24);
}

void initMultibootSPI() {
  SPI.end();
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE3);
  SPI.setClockDivider(SPI_CLOCK_DIV32);
  SPI.begin();
  delayMicroseconds(10);
  digitalWrite(MUX_PIN, LOW);
}

void sendHandshake(uint32_t rv) {
  // handshake_data 11h+client_data[1]+client_data[2]+client_data[3]
  uint32_t handshake_data = (((rv >> 16) + 0xf) & 0xff) | 0x00006400;
  Serial.print("Sending Handshake: "); Serial.println(handshake_data, HEX);
  rv = send(handshake_data);
  Serial.println(rv, HEX);
}

void sendROMHeader() {
  Serial.println("Sending ROM Header");
  uint32_t block;

  // Send Header in blocks of two bytes
  for (uint32_t i = 0; i <= 0x5f; i++) {
    block = header[2 * i];
    block = header[2 * i + 1] << 8 | block;
    fcnt += 2;

    send(block);
    Serial.print(block, HEX);
  }
  Serial.println("Header Transfered");
  uint32_t rv = send(0x00006200); // Transfer of header data completed
  Serial.println(rv, HEX);
}

void multiboot() {
  initMultibootSPI();
  
  uint32_t rom_size = sizeof(header) + romLength;
  rom_size = (rom_size + 0xf) & 0xFFFFFFF0; // Align rom length to 16
  Serial.print("ROM Size:");
  Serial.println(rom_size, HEX);

  uint32_t rv;

  Serial.println("Waiting for GBA");
  while (rv != 0x72026202) {
    rv = send(0x00006202);
  }
  Serial.print("GBA Found: "); Serial.println(rv, HEX);
  rv = 0;

  send(0x00006202); // Found GBA
  send(0x00006102); // Recognition OK

  sendROMHeader();  // Transfer C0h bytes header data in units of 16bits with no encrpytion

  rv = send(0x00006202); // Exchange master/slave info again
  Serial.println(rv, HEX);

  Serial.println("Sending Palette");
  // palette_data as "81h+color*10h+direction*8+speed*2", or as "0f1h+color*2" for fixed palette, whereas color=0..6, speed=0..3, direction=0..1.
  // Then wait until 0x73hh**** is received. hh represents client_data
  while (((rv >> 24) & 0xFF) != 0x73) {
    rv = send(0x000063D1);
    Serial.println(rv, HEX);
  }

  uint32_t client_data = ((rv >> 16) & 0xFF); // Random client generated data used for later handshake
  Serial.print("Client Data:");
  Serial.println(client_data, HEX);

  uint32_t m = ((rv & 0x00ff0000) >>  8) + 0xffff00d1;
  uint32_t h = ((rv & 0x00ff0000) >> 16) + 0xf;

  sendHandshake(rv);

  Serial.print("Sending length information: "); Serial.println((rom_size - 0x190) / 4, HEX);
  rv = send((rom_size - 0x190) / 4); // Send length information and receive random data[1-3] (seed)
  Serial.println(rv, HEX);

  uint32_t f = (((rv & 0x00ff0000) >> 8) + h) | 0xffff0000;
  uint32_t c = 0x0000c387;

  uint32_t bytes_sent = 0;
  uint32_t w, w2, bitt;
  int i = 0;

  Serial.println("Sending ROM");
  while (fcnt < rom_size) {
    if (bytes_sent == 32) {
      bytes_sent = 0;
    }

    //w = rom[i] | (rom[i + 1] << 8) | (rom[i + 2] << 16) | (rom[i + 3] << 24) ;
    
    w = read4ROMBytes(i);
    
    i = i + 4;
    bytes_sent += 4;

    if (fcnt % 0x80 == 0 || fcnt > 63488 || fcnt == rom_size) {
      Serial.print(fcnt, HEX); Serial.print("/"); Serial.println(rom_size, HEX);
    }


    w2 = w;

    for (bitt = 0; bitt < 32; bitt++) {
      if ((c ^ w) & 0x01) {
        c = (c >> 1) ^ 0x0000c37b;
      }
      else {
        c = c >> 1;
      }
      w = w >> 1;
    }


    m = (0x6f646573 * m) + 1;
    initMultibootSPI();
    rv = send(w2 ^ ((~(0x02000000 + fcnt)) + 1) ^m ^ 0x43202f2f);

    fcnt = fcnt + 4;
  }
  Serial.println(rv, HEX);
  Serial.println("ROM sent! Doing checksum now...");


  for (bitt = 0; bitt < 32; bitt++) {
    if ((c ^ f) & 0x01) {
      c = ( c >> 1) ^ 0x0000c37b;
    }
    else {
      c = c >> 1;
    }

    f = f >> 1;
  }
  Serial.print("CRC: "); Serial.println(c, HEX);

  Serial.println("Waiting for CRC");
  while (rv != 0x00750065) {
    rv = send(0x00000065);
  }


  rv = send(0x00000066);
  Serial.println(rv, HEX);

  Serial.println("Exchanging CRC");
  rv = send(c);
  Serial.println(rv, HEX);

  Serial.println("Done!");

  
  digitalWrite(MB_PIN, LOW);
  digitalWrite(UART_PIN, HIGH);
}
