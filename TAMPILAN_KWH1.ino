#include <UTFTGLUE.h>              //use GLUE class and constructor
UTFTGLUE myGLCD(0, A2, A1, A3, A4, A0); //all dummy args

String dataIn;
String dt[10];
int i;
boolean parsing = false;

char teg[10];
char arus[10];
char pwr[10];
char enrgy[10];
char freq[10];
char pf[10];

int tloop = 0; 

void setup() {
  Serial.begin(115200);
  dataIn = "";
  randomSeed(analogRead(0));

  // Setup the LCD
  myGLCD.InitLCD();
  tampilan();
}

void loop() {
  if (Serial.available() > 0) {
    char inChar = (char)Serial.read();
    dataIn += inChar;
    if (inChar == '\n') {
      parsing = true;
    }
  }

  if (parsing) {
    parsingData();
    parsing = false;
    dataIn = "";
  }
}

void tampilan() {
  myGLCD.setFont(BigFont);
  myGLCD.fillScr(43, 90, 105);
  myGLCD.setColor(255, 255, 255);
  myGLCD.setBackColor(43, 90, 105);
  myGLCD.print("SMART KWH METER", CENTER, 30);

  //1
  myGLCD.setColor(76, 201, 240);
  myGLCD.fillRoundRect(10, 70, 158, 169);//kiri,atas,kanan,bawah
  myGLCD.setColor(7, 59, 76);
  myGLCD.fillRect(10, 70, 158, 90);//kiri,atas,kanan,bawah

  //2
  myGLCD.setColor(76, 201, 240);
  myGLCD.fillRoundRect(166, 70, 314, 169);//kiri,atas,kanan,bawah
  myGLCD.setColor(7, 59, 76);
  myGLCD.fillRect(166, 70, 314, 90);//kiri,atas,kanan,bawah

  //3
  myGLCD.setColor(76, 201, 240);
  myGLCD.fillRoundRect(322, 70, 470, 169);//kiri,atas,kanan,bawah
  myGLCD.setColor(7, 59, 76);
  myGLCD.fillRect(322, 70, 470, 90);//kiri,atas,kanan,bawah

  //4
  myGLCD.setColor(76, 201, 240);
  myGLCD.fillRoundRect(10, 180, 158, 279);
  myGLCD.setColor(7, 59, 76);
  myGLCD.fillRect(10, 180, 158, 200);

  //5
  myGLCD.setColor(76, 201, 240);
  myGLCD.fillRoundRect(166, 180, 314, 279);
  myGLCD.setColor(7, 59, 76);
  myGLCD.fillRect(166, 180, 314, 200);

  //6
  myGLCD.setColor(76, 201, 240);
  myGLCD.fillRoundRect(322, 180, 470, 279);
  myGLCD.setColor(7, 59, 76);
  myGLCD.fillRect(322, 180, 470, 200);

  //text
  myGLCD.setFont(SmallFont);
  myGLCD.setColor(255, 255, 255);
  myGLCD.setBackColor(7, 59, 76);

  myGLCD.print("Current", CENTER, 75);
  myGLCD.print("Voltage", LEFT_C, 75);
  myGLCD.print("Power", RIGHT_C, 75);
  myGLCD.print("Energy", LEFT_C, 185);
  myGLCD.print("Frequency", CENTER, 185);
  myGLCD.print("Power Factor", RIGHT_C, 185);
}

void parsingData() {
  int j = 0;

  dt[j] = "";
  //proses parsing data
  for (i = 1; i < dataIn.length(); i++) {
    //pengecekan tiap karakter dengan karakter (#) dan (,)
    if ((dataIn[i] == '#') || (dataIn[i] == ','))
    {
      //increment variabel j, digunakan untuk merubah index array penampung
      j++;
      dt[j] = "";     //inisialisasi variabel array dt[j]
    }
    else
    {
      //proses tampung data saat pengecekan karakter selesai.
      dt[j] = dt[j] + dataIn[i];
    }
  }

  dt[0].toCharArray(teg, 15);
  dt[1].toCharArray(arus, 15);
  dt[2].toCharArray(pwr, 15);
  dt[3].toCharArray(enrgy, 15);
  dt[4].toCharArray(freq, 15);
  dt[5].toCharArray(pf, 15);

  //kirim data hasil parsing
  Serial.print("Voltage : ");
  Serial.println(teg);
  Serial.print("Current : ");
  Serial.println(arus);
  Serial.print("Power : ");
  Serial.println(pwr);
  Serial.print("Energy : ");
  Serial.println(enrgy);
  Serial.print("Frequency : ");
  Serial.println(freq);
  Serial.print("Power Factor : ");
  Serial.println(pf);
  Serial.print("\n");

  // clearscreen
  myGLCD.setColor(76, 201, 240);
  myGLCD.fillRoundRect(10, 91, 158, 169);//kiri,atas,kanan,bawah
  myGLCD.fillRoundRect(166, 91, 314, 169);
  myGLCD.fillRoundRect(322, 91, 470, 169);
  myGLCD.fillRoundRect(10, 201, 158, 279);
  myGLCD.fillRoundRect(166, 201, 314, 279);
  myGLCD.fillRoundRect(322, 201, 470, 279);

  myGLCD.setFont(BigFont);
  myGLCD.setColor(255, 255, 255);
  myGLCD.setBackColor(76, 201, 240);

  myGLCD.print(teg, LEFT_C, 120);
  myGLCD.print(arus, CENTER, 120);
  myGLCD.print(pwr, RIGHT_C, 120);
  myGLCD.print(enrgy, LEFT_C, 230);
  myGLCD.print(freq, CENTER, 230);
  myGLCD.print(pf, RIGHT_C, 230);
}

