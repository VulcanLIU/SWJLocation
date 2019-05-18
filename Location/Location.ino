#include "ComwithGY25.h"
#include "Display.h"

//#define SERIAL_DEBUG 1
//#define POS_DEBUG 1
#define DATA_OUTPUT 1

double x = 0;//中点位置
double y = 0;
double p = 0;

long int x_step = 0;
long int y_step = 0;

long int x_step_total = 0;
long int y_step_total = 0;

//接线信息
static const int INT0A = 3;//INT0
static const int INT0B = 8;
static const int INT1A = 2;//INT1
static const int INT1B = 7;

//物理信息
const double wheel_d = 38;//轮直径38mm
const int x_line = 1000;
const int y_line = 1000;

//用于串口输出
double _x = 0;
double _y = 0;

double x2 = 0;
double y2 = 0;

void blinkX();
void blinkY();

void POS_begin();
void POS_clear();
void POS_refresh();

long int _millis = 0;

// The setup() function runs once each time the micro-controller starts
void setup()
{
  GY25.begin();
  
  dp.begin();
  
  POS_begin();

  Serial.begin(115200);
}
// Add the main program code into the continuous loop() function
void loop()
{
  
  GY25.refresh();

  if (millis() - _millis >= 50)
  {
    POS_refresh();

    dp.displayXYP(x,y,p);

#ifdef DATA_OUTPUT
    Serial.print("x:");
    Serial.print(x);
    Serial.print("y:");
    Serial.print(y);
    Serial.print("p:");
    Serial.print(p);
#endif

#ifdef SERIAL_DEBUG
    Serial.print("X_step:");
    Serial.print(x_step_total);
    Serial.print("Y_step:");
    Serial.print(y_step_total);
#endif

    Serial.println();

    _millis = millis();
  }
}

void blinkX()
{
  int xstate = digitalRead(INT0B);
  if (xstate == HIGH)
  {
    x_step++;
    x_step_total ++;
  }
  else
  {
    x_step--;
    x_step_total --;
  }
}

void blinkY()
{
  int ystate = digitalRead(INT1B);
  if (ystate == HIGH)
  {
    y_step++;
    y_step_total ++;
  }
  else
  {
    y_step--;
    y_step_total --;
  }
}

void POS_begin()
{
  pinMode(INT0A, INPUT_PULLUP);
  pinMode(INT1A, INPUT);

  pinMode(INT0B, INPUT_PULLUP);
  pinMode(INT1B, INPUT);

  attachInterrupt(digitalPinToInterrupt(INT0A), blinkX, FALLING);
  attachInterrupt(digitalPinToInterrupt(INT1A), blinkY, FALLING);
}

void POS_clear()
{
  x_step = 0;
  y_step = 0;

  x = 0;
  y = 0;
  p = 0;
}

void POS_refresh()
{

  //统计刷新时间内 编码器位移值
  _x = double(x_step)/ x_line * PI * wheel_d;
  //x_step_total += x_step;
  x_step = 0;//行走的距离 单位mm；
  _y = double(y_step)/ y_line * PI * wheel_d; //行走的距离 单位mm；
  //y_step_total += y_step;
  y_step = 0;
  p = GY25.YPR[0] / 180.00 * PI;
  
  //刘展鹏的算法
  x2 += (-_x) * cos(p-45) + (_y) * sin(p-45);
  y2 += (_y) * cos(p-45) - (-_x) * sin(p-45);

  x = x2;
  y = y2;
  p = p / PI * 180;

#ifdef POS_DEBUG
  Serial.print("_x:");
  Serial.print(_x);
  Serial.print("_y:");
  Serial.print(_y);

  Serial.println();
#endif
}

void serialEvent()
{
  if(Serial.available()>0)
  {
    //x2 = 0;
    String str = Serial.readStringUntil('\n');
    str.toLowerCase();
    str = str.substring(0,str.length()-1);
    Serial.println(str);
    Serial.println(str.length());
    
    if(str == "clear"){GY25.correctYaw();delay(3000);GY25.autoMode();}
    else
    {
      Serial.println("1");
    }
    if(str == "clearall"){GY25.correctYaw();delay(3000);GY25.autoMode();x2 = 0;y2 = 0;}
    else
    {
      Serial.println("2");
    }
  }
}
