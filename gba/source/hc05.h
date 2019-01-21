//
// Created by Shyri on 2019-01-05.
//

#ifndef GBA_BT_HID_HC05_H
#define GBA_BT_HID_HC05_H
bool startCommandMode();
bool connectLast();
void sendGamepad(int xAxis, int yAxis, int zAxis, int rAxis, int buttons1, int buttons2);
bool checkPaired();
void sendDisconnect();
#endif //GBA_BT_HID_HC05_H
