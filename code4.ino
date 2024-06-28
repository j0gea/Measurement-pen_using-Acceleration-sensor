#include <LiquidCrystal_I2C.h> //LCD I2C(어뎁터) 라이브러리 
LiquidCrystal_I2C lcd(0x27, 16, 2); // 모듈 인스턴스 선언







float X = 0;  
float Y = 0; 




float t = 100;
const int xInput = A0;
const int yInput = A1;
//const int zInput = A2;

// initialize minimum and maximum Raw Ranges for each axis
int RawMin = 0;
int RawMax = 1023;

// Take multiple samples to reduce noise
const int sampleSize = 10;

void setup()
{
  // LCD 초기화
  lcd.init();
  // 백라이트 켜기
  lcd.backlight();

  analogReference(EXTERNAL);
  Serial.begin(9600);

  pinMode(7, INPUT); // 단위변환 cm
  pinMode(6, INPUT); // 단위변환 in
  pinMode(5, INPUT); // 단위변환 ft
}



float sum = 0; // 길이의 합
float len = 0; // 한 번(두 점을 지날 때)의 길이
float RX = 0; float RY = 0; // X / Y의 '지난값'

float AX = 0; float AY = 0; // 처음 '가속도 값'
float VX = 0; float VY = 0; // 처음 '속도 값'

int mode = 0; // 단위변환 모드
float p_sum = 0; // 합을 단위변환시킨 것

float LX = 0; float LY = 0;



int flag = 0;
int drawMode = 0; // 0: total, 1: line

int buttonState; int state = 0;

void loop()
{

  if (!digitalRead(7)) {
    mode = mode + 1; // 0: cm, 1: in, 2:ft
    if (mode >= 3){
      mode = 0;
    }
  }

  if (!digitalRead(6)) {
    drawMode = drawMode + 1; // 0: cm, 1: in, 2:ft
    lcd.setCursor(0, 1);
    lcd.print("M1");
    if (drawMode >= 2){
      drawMode = 0;
      lcd.setCursor(0, 1);
      lcd.print("M0");
    }
    delay(10);
  }

  
  // if (!digitalRead(5)) mode = 2; // ft


  //Read raw values
  int xRaw = ReadAxis(xInput);
  int yRaw = ReadAxis(yInput);
  // int zRaw = ReadAxis(zInput);



  //int X = xRaw;
  //int Y = yRaw;

  

  // Convert raw values to 'milli-Gs"
  long xScaled = map(xRaw, RawMin, RawMax, -3000, 3000);
  long yScaled = map(yRaw, RawMin, RawMax, -3000, 3000);
  // long zScaled = map(zRaw, RawMin, RawMax, -3000, 3000);

  // re-scale to fractional Gs
  float xAccel = xScaled / 1000.0;
  float yAccel = yScaled / 1000.0;
  // float zAccel = zScaled / 1000.0;

  float xcha = (abs(xAccel - LX));
  float ycha = (abs(yAccel - LY));

  // 사다리꼴 방법          //  t -> 설정 시간간격

  if(xcha > 0.010 || ycha > 0.010)
  {
    if(drawMode == 1 && flag == 1){
      sum = 0;
      flag = 0;
    }

    //Serial.println(xcha,5);
    //Serial.println(ycha,5);
  X = 0.5*t*(VX+0.5*t*(AX+xcha));   //  R(X) -> 센서로 받은 X축 가속도
  Y = 0.5*t*(VY+0.5*t*(AY+ycha));     //  R(Y) -> 센서로 받은 Y축 가속도
  
  VX = 0.5*t*(AX+xcha);
  VY = 0.5*t*(AY+ycha);
  AX = AX + xcha;
  AY = AY + ycha;
  }
  
  LX = xAccel; LY = yAccel;

  if(xcha < 0.006 && ycha < 0.006){
    if(flag == 0){
      flag = 1;
    }
  }
  Serial.print("flag: ");
  Serial.println(flag);
  Serial.print("drawMode: ");
  Serial.println(drawMode);


    if (RX == 0 && RY == 0) {
      // 이전 값이 존재하지 않으면 현재 값을 이전 값으로 설정
      // 이것은 좌표가 처음부터 0이 아니라면(절대좌표) 움직이지
      // 않았음에도 길이가 재질 수 있기에
      RX = X;
      RY = Y;
    }

    else { // 처음이 아닐 시
      len = sqrt(pow(RX - X, 2) + pow(RY - Y, 2));
      // 현재의 길이
      sum = sum + (len * 0.002);
      // 총합은 총합 + 현재 길이


      // 출력할 모드 정하기
      if (mode == 0) { // cm변환
        p_sum = sum;
        lcd.setCursor(14, 1);
        lcd.print("cm");
      }

      if (mode == 1) { // 인치변환
        p_sum = sum / 2.54;
        lcd.setCursor(14, 1);
        lcd.print("in");
      }

      if (mode == 2) { // 피트변환
        p_sum = sum / 30.48;
        lcd.setCursor(14, 1);
        lcd.print("ft");
      }
    }

    // 지금 좌표
    RX = X;
    RY = Y;

    // LCD 첫줄
    lcd.setCursor(2, 0);
    lcd.print("total length:");
    // LCD 둘째줄
    lcd.setCursor(5, 1);
    lcd.print(p_sum);

    


    if (!digitalRead(5)) { // 리셋버튼(5번 == 3번째)
      lcd.setCursor(0, 1);
      lcd.print("------reset-----");
      delay(100);
      sum = 0; // 총합 0으로 바꾸기
      lcd.init();
    }

 




  delay(t);
}

// Take samples and return the average
int ReadAxis(int axisPin)
{
  long reading = 0;
  analogRead(axisPin);
  delay(1);
  for (int i = 0; i < sampleSize; i++)
  {
    reading += analogRead(axisPin);
  }
  return reading / sampleSize;
}
