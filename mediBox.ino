#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);

extern volatile unsigned long timer0_millis;
unsigned long t;

int setBtnPin = 12; //설정 진입, 변경할 선택
int howLong = 0; //얼마나 꾹 누르고 있었는지
int setting = 0; //설정 진입여부

int changeBtnPin = 13; //시간 값 변경
int changeThing = 0; //date[changeThing][0]

int date[6][2] = {{ 2022, 0 }, //year (시간, 위치)
                  { 01, 5 }, //month
                  { 01, 8 }, //day
                  { 00, 11 }, //hour
                  { 00, 14 }, //minute
                  { 00, 17 } }; //second
                  
int sec = date[5][0]; //초기 sec
int memory[3][6];

int memoBtnPin = 11; //누르면 시간 기록!
int memoBtnLev;
            
void setup(){
  lcd.init();
  lcd.backlight();

  Serial.begin(9600);
  
  pinMode(setBtnPin, INPUT_PULLUP);
  pinMode(changeBtnPin, INPUT_PULLUP);
  pinMode(memoBtnPin, INPUT_PULLUP);

  memoBtnLev = digitalRead(memoBtnPin);
}

void loop(){
     
  t = sec + millis()/1000;
  
  if (setting) {
    changeDate();
    outSetting();
  } else {
    updateDate();
    enterSetting(); 
    memo();
  }

  if (t >= 60) timer0_millis = 0;

}

//시간 기록
void memo() {
  int memoBtn = digitalRead(memoBtnPin);
  if ((memoBtnLev != memoBtn) && memoBtn) {
    int memory0Size = sizeof(memory[0]) / sizeof(int);
    int memorySize = sizeof(memory) / sizeof(int) / memory0Size;

    //이전의 기록들 한 칸씩 미루기
    for (int i = memorySize - 1; i > 0; i --) {
      for (int k = 0; k < memory0Size; k ++) {
        memory[i][k] = memory[i - 1][k];
      }
    }

    //새 기록
    for (int k = 0; k < memory0Size; k ++) {
      memory[0][k] = date[k][0];
    }

    //LCD에 표시
    for (int i = 0; i < memorySize; i ++) {
      for (int k = 0; k < memory0Size; k ++) {
        if(memory[i][0]) lcdPrint(String(memory[i][k]), date[k][1], i + 1);
      }
    }
    delay(300);
  }

  memoBtnLev = memoBtn;
}

//설정 들어가기
void enterSetting() {
  int setBtn = digitalRead(setBtnPin);

  if (!setBtn) {
    howLong += 1;
    if (howLong >= 20) { //꾹 눌렀으면 세팅 진입
      howLong = 0;
      setting = 1;
    }
  } else {
    howLong = 0;
  }
}

//설정 나가기
void outSetting() {
  int setBtn = digitalRead(setBtnPin);
  
  if (!setBtn) {
    howLong += 1;
    if (howLong >= 10) { //꾹 눌렀으면 세팅 나가기
      howLong = 0;
      setting = 0;
    }
  } else {
    howLong = 0;
  }
}

//설정에서 시간 변경
void changeDate() {
  lcd.cursor();
  lcd.blink();
  
  lcd.setCursor(date[changeThing][1], 0);

  int setBtn = digitalRead(setBtnPin);
  if (!setBtn) {
    changeThing += 1;
    if (changeThing == 6) changeThing = 0;
    delay(200);
  }

  int changeBtn = digitalRead(changeBtnPin);
  if (!changeBtn) {
    date[changeThing][0] += 1;
    dateLimit();
    delay(100);
    showDate();
  }

  timer0_millis = 0;
  sec = date[5][0];
}

//시간의 흐름에 따라 현재 시간 갱신
void updateDate() {
  lcd.noCursor();
  lcd.noBlink();
  
  date[5][0] = t; //t가 증가함에 따라 시간 갱신
  dateLimit();//각 시간 값 제한
  
  showDate();//갱신된 시간 보여주기
}

//각 시간 값 한계치 조정
void dateLimit() {
  //year = date[0], month = date[1], day = date[2]
  //hour = date[3], minute = date[4], second = date[5]

  if (date[5][0] >= 60) { //second
    if (!setting) date[4][0] += 1; //min
    date[5][0] = 0;
  }

  if (date[4][0] >= 60) { //minute
    if (!setting) date[3][0] += 1; //hour
    date[4][0] = 0;
  }

  if (date[3][0] >= 24) { //hour
    if (!setting) date[2][0] += 1; //day
    date[3][0] = 0;
  }

  //day
  if ((date[2][0] > 30 && (date[1][0] == 4 || date[1][0] == 6 || date[1][0] == 9 || date[1][0] == 11))
  || (date[2][0] > 31  && (date[1][0] == 1 || date[1][0] == 3 || date[1][0] == 5 || date[1][0] == 7 || date[1][0] == 8 || date[1][0] == 10 || date[1][0] == 12))
  || (date[0][0] % 4 && date[2][0] > 28 && date[1][0] == 2)
  || (!(date[0][0] % 4) && date[2][0] > 29 && date[1][0] == 2)) {
    if (!setting) date[1][0] += 1; //month
    date[2][0] = 1;
  }

  if (date[1][0] > 12) { //month
    if (!setting) date[0][0] += 1; //year
    date[1][0] = 1;
  }

  if (date[0][0] > 2100) { //year
    date[0][0] = 2022;
    date[1][0] = 1;
    date[2][0] = 1;
    date[3][0] = 0;
    date[4][0] = 0;
    date[5][0] = 0;
  }
  
}

//LCD에 시간 보여주기
void showDate() {
  
  //시간들
  for (int i = 0; i < 6; i ++) {
    lcdPrint(String(date[i][0]), date[i][1], 0);
  }
  
  //특수문자들
  lcdPrint("-", 4, 0);
  lcdPrint("-", 7, 0);
  lcdPrint(":", 13, 0);
  lcdPrint(":", 16, 0);
  
}


void lcdPrint(String str, int x, int y) { //LCD에 글자 쓰기

  if (str != ":" && str != "-" && str.length() < 2) str = "0" + str;

  lcd.setCursor(x, y);
  lcd.print(str);
  
}
