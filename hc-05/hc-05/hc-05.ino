
void setup() {
  // initialize both serial ports:
  Serial.begin(9600);
  Serial1.begin(9600);
  Serial2.begin(9600);
}

void loop() {
   // read from port 0, send to port 1:
  ///sendButton();
  //Serial.write((uint8_t) 0xFD);
  //Serial.write((uint8_t) 0x06);
  int ch = 0;
  if (Serial. available()) {
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
    Serial.println("HC-05"); 
    Serial.println(inByte); 
    //Serial.println(" "); 
  }
  
  // read from port 2, send to port 0:
  if (Serial2.available()) {
    int inByte = Serial2.read();
    Serial.println("GBA"); 
    Serial.write(inByte); 
    Serial.println(" "); 
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
  Serial1.write((uint8_t)0x01); // buttons

  Serial.write((uint8_t)0x00); // axis_x
  Serial.write((uint8_t)0x00); // axis_y
  Serial.write((uint8_t)0x00); // axis_z
  Serial.write((uint8_t)0x00); // axis_?
  Serial.write((uint8_t)0x00); // buttons
  Serial.write((uint8_t)0x01); // buttons
  //Serial1.write((uint8_t)0x00);
  // 0x0001 -> A
  // 0x0002 -> B
  // 0x0008 -> X
  
  // 0x0010 -> Y
  // 0x0040 -> L
  // 0x0080 -> R
  
  // 0x2000 -> SELECT
  // 0x4000 -> START

  // Axises X,Y:
  // 0x7f -> RIGHT
  // 0x80 -> LEFT
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
