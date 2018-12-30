
void setup() {
  // initialize both serial ports:
  Serial.begin(9600);
  Serial1.begin(9600);
}

void loop() {
   // read from port 0, send to port 1:
  int ch = 0;
  if (Serial.available()) {
    ch = Serial.read();
    if (ch == 'g') {       
      Serial.write("Sending...\n");
      sendButton();
    } else if (ch == 'd') {       
      Serial.write("Sending disconnect \n");
      disconnect();
    } else if (ch == 'r') {       
      Serial.write("Report \n");
      sendReport();
    } else {
      Serial1.write(ch);
    }
  }
  
  // read from port 1, send to port 0:
  if (Serial1.available()) {
    int inByte = Serial1.read();
    Serial.write(inByte); 
  }
}

void sendButton() {
  sendHeader();
  //uint8_t btnState1 = btnState & 0xFF;
  //uint8_t btnState2 = (btnState >> 8) & 0xFF;
  Serial1.write((uint8_t)0x00); // axis_x
  Serial1.write((uint8_t)0x00); // axis_y
  Serial1.write((uint8_t)0x00); // axis_z
  Serial1.write((uint8_t)0x00); // axis_?
  Serial1.write((uint8_t)0x00); // buttons
  Serial1.write((uint8_t)0x20); // buttons
  //Serial1.write((uint8_t)0x00);
}

void disconnect() {
  Serial1.write((uint8_t) 0x00);
}

void sendReport() {
  Serial1.write((uint8_t) 0xFF);
}

void sendHeader() {
  Serial1.write((uint8_t) 0xFD);
  Serial1.write((uint8_t) 0x06);
}
