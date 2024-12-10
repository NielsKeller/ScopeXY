#include <Wire.h>


// Comment from sparkfun -----------------
//This is the I2C Address of the MCP4725, by default (A0 pulled to GND).
//For devices with A0 pulled HIGH, use 0x61
// Comment from sparkfun -----------------

#define MCP4725_ADDR 0x60  
#define MCP4725_ADDR2 0x61   

//////////////////////////////////////////////
//        RemoteXY include library          //
//////////////////////////////////////////////

// you can enable debug logging to Serial at 115200
//#define REMOTEXY__DEBUGLOG    

// RemoteXY select connection mode and include library 
#define REMOTEXY_MODE__SOFTSERIAL

#include <SoftwareSerial.h>

// RemoteXY connection settings 
#define REMOTEXY_SERIAL_RX 2
#define REMOTEXY_SERIAL_TX 3
#define REMOTEXY_SERIAL_SPEED 9600


#include <RemoteXY.h>

// RemoteXY GUI configuration  
#pragma pack(push, 1)  
uint8_t RemoteXY_CONF[] =   // 93 bytes
  { 255,3,0,0,0,86,0,19,0,0,0,0,31,1,106,200,1,1,6,0,
  1,28,14,50,50,0,136,31,0,1,28,74,50,50,0,136,31,0,1,28,
  139,50,50,0,135,31,0,129,32,34,41,12,64,24,83,116,97,116,105,99,
  32,49,0,129,32,93,41,12,64,24,83,116,97,116,105,99,32,50,0,129,
  33,158,40,12,64,24,77,111,118,105,110,103,0 };
  
// this structure defines all the variables and events of your control interface 
struct {

    // input variables
  uint8_t Static_1; // =1 if button pressed, else =0
  uint8_t Static_2; // =1 if button pressed, else =0
  uint8_t Moving; // =1 if button pressed, else =0

    // other variable
  uint8_t connect_flag;  // =1 if wire connected, else =0

} RemoteXY;   
#pragma pack(pop)
 
/////////////////////////////////////////////
//           END RemoteXY include          //
/////////////////////////////////////////////

//Send value to DAC constrained from 0 to 4095 (12-bits)
void dacSend(int address, int value){
  value = constrain(value, 0, 4095);

  Wire.beginTransmission(address);
  Wire.write(64);                     // cmd to update the DAC
  Wire.write(value >> 4);        // the 8 most significant bits...
  Wire.write((value & 15) << 4); // the 4 least significant bits...
  Wire.endTransmission();
}

// Transmit a sin wave to the DAC, call this handeler in loop.
// The phase is in terms of pi
// The index is an incrementing variable
// F is for phase ratio and is multiplied by fdiv
void dacSin(int address, int index, float phase, float F, float fdiv){
  Wire.beginTransmission(address);
  Wire.write(64);                     // cmd to update the DAC

  int sinVal = (sin(index*2*PI*F*fdiv + phase*PI)+1)*4095/2;
  sinVal = constrain(sinVal,0,4095);


  Wire.write(sinVal >> 4);        // the 8 most significant bits...
  Wire.write((sinVal & 15) << 4); // the 4 least significant bits...
  Wire.endTransmission();
}


//Makes Lissajous figure with frequency ratio and phase shift (phase shift is multiplied by 1/(2pi) rads) call in loop with incrementing i
void LJFigure(float ratio, float phase, int index, float fdiv){
  
  dacSin(MCP4725_ADDR, index, phase, ratio, fdiv);
  dacSin(MCP4725_ADDR2, index,0,1, fdiv);
}

void LJFigureMoving(float ratio, float period, int index, float fdiv){
  LJFigure(ratio, millis()*2/(period*1000), index, fdiv);
}

void setup()
{
  Serial.begin(9600);
  RemoteXY_Init ();
  Wire.begin();
}

#define fdiv1 0.03

void loop() {
  static float ratio = 0;
  static float phase = 0;
  
  RemoteXY_Handler ();

  static int i = 0;

  if (RemoteXY.Static_1 == 1){
    LJFigure(6.0/5, 1.0/4, i, 0.01);
    i++;
  }
  if (RemoteXY.Static_2 == 1){
    LJFigure(3.0/2, 7.0/8, i, 0.0051);
    i++;
  }
  if (RemoteXY.Moving == 1){
    LJFigureMoving(2/1, 3, i, fdiv1);
    i++;
  }
}