// Specifikke pakker initialiseres.

#include "Wire.h"
#include <LiquidCrystal.h>


// Diverse relevante beregningsnavne defineres tideligt i scopet.
long gyro_x_f = 0.0;
long gyro_x = 0.0;
long gyro_y_f = 0.0;
long gyro_y = 0.0;
long gyro_z_f = 0.0;
long gyro_z = 0.0;
float tid_f = 0.0;
float tid_n = 0.0;
float Vin_x_gyro = 0.0;
float Vin_y_gyro = 0.0;
float Vin_z_gyro = 0.0;
int Cal_x = 0;
int Cal_y = 0;
int Cal_z = 0;

// Ultrasonic Sensor konfiguereres til at lytte til Pin 2 og 3.
const int MaxDistance_trigPin = 2;
const int MaxDistance_echoPin = 3;

// LCD-screenkonfiguereres til at lytte til Pin 7 gennem 12. Dokumentation om set-up som er fuldt findes på Arduinos egen hjemmeside, minus potentiometeret. // https://docs.arduino.cc/learn/electronics/lcd-displays //
const int rs = 12, en = 11, d4 = 10, d5 = 9, d6 = 8, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void setup() {
  // put your setup code here, to run once:

  // Serial monitoren sættes igang, og lytter til baud 9600
  Serial.begin(9600);
  
  // Wire (MPU6050 + GY-521) Begynder transmission, og hukommelse indlæses
  // Er dette ikke potentielt overflødigt med orienteringsprceduren lige under? (redundancy?)
  Wire.begin();
  Wire.beginTransmission(0x68);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission(true);


  // Accelerometerets værdier i henholdsvis x,y,z retningen hentes fra de 3 første bytes, og anvendes til at bestemme gyroskopets potision fra 'origo' ved startup
  // 'origo' defineres således, at det er helt plant - Vandret i x og y.
  Wire.beginTransmission(0x68);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(0x68, 2 * 7, true); //en pakke er 8 bits. 3 sensorer, af datapakker 2.

  int16_t accelerometer_x = Wire.read() << 8 | Wire.read(); //16 bit, med 8 int af gangen. read 8 og skub 8 tal til venstre
  int16_t accelerometer_y = Wire.read() << 8 | Wire.read();
  int16_t accelerometer_z = (Wire.read() << 8 | Wire.read());
  int16_t temp = Wire.read() << 8 | Wire.read();
  int16_t gyro_x = Wire.read() << 8 | Wire.read();
  int16_t gyro_y = Wire.read() << 8 | Wire.read();
  int16_t gyro_z = Wire.read() << 8 | Wire.read();

  for (int i = 0; i < 10; i++) {
    Cal_x = Cal_x + gyro_x;
    Cal_y = Cal_y + gyro_y;
    Cal_z = Cal_z + gyro_z;
  }
  Cal_x = Cal_x / 10;
  Cal_y = Cal_y / 10;
  Cal_z = Cal_z / 10;

  // Ved hjælp at tyngdeaccelerationens bidrag til 'tyngden' i henholdsvis x og y retningen, bestemmes hvilken retning som er 'ned'.
  Vin_y_gyro = -((atan2(accelerometer_x, accelerometer_z) * 180) / 3.1415);
  Vin_x_gyro = ((atan2(accelerometer_y, accelerometer_z) * 180) / 3.1415);


  // Alt det der skal printes til excel dokumentet:
  // CSV filen opbygges ved brug af det externe plugin 'ArduSpreadsheet' som tillader udskrivning til konventionel .csv, ved brug af f.eks kommaer.
  // https://circuitjournal.com/arduino-serial-to-spreadsheet //

  Serial.print("Vin_x ");
  Serial.print(",");
  Serial.print("Vin_y");
  Serial.print(",");
  Serial.print("Vin_Z");
  Serial.print(",");
  Serial.print("Temperatur");
  Serial.print(",");
  Serial.println("Afstand");


  // Ultrasonic Sensor's pintyper defineres.
  pinMode(MaxDistance_trigPin, OUTPUT);
  pinMode(MaxDistance_echoPin, INPUT);

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);

}
void loop() {
  // put your main code here, to run repeatedly:
  // Et meget lille tids-gap stilles op, og anvendes til at beregne gyroskopets relative vinkelændring fra foregående position.
  gyro_x_f = gyro_x;
  gyro_y_f = gyro_y;
  gyro_z_f = gyro_z;
  tid_f = tid_n;
  tid_n = millis();

  // Klargøring til at indlæse alle sensordataer fra MPU6050 + GY-521 modulerne
  Wire.beginTransmission(0x68);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(0x68, 2 * 7, true); // 

  // Læser de forskellige værdier.
  int16_t accelerometer_x = Wire.read() << 8 | Wire.read(); //16 bit, med 8 int af gangen. read 8 og skub 8 tal til venstre
  int16_t accelerometer_y = Wire.read() << 8 | Wire.read();
  int16_t accelerometer_z = (Wire.read() << 8 | Wire.read());
  int16_t temp = Wire.read() << 8 | Wire.read();
  int16_t gyro_x = Wire.read() << 8 | Wire.read();
  int16_t gyro_y = Wire.read() << 8 | Wire.read();
  int16_t gyro_z = Wire.read() << 8 | Wire.read();



  // Vinkel med gyro, hvor accelerationscalibreringen benyttes som startværdi.
  Vin_x_gyro = Vin_x_gyro + (((tid_n - tid_f) / 1000) * (((gyro_x + gyro_x_f) + Cal_x) / 131));
  Vin_y_gyro = Vin_y_gyro + (((tid_n - tid_f) / 1000) * (((gyro_y + gyro_y_f) - Cal_y) / 131));
  Vin_z_gyro = Vin_z_gyro + (((tid_n - tid_f) / 1000) * (((gyro_z + gyro_z_f) + Cal_z) / 131));
  // OBS! værdierne (390, -125 og 0) er hårde kalibreringer, som varierer fra system til system. Hver gang at et nyt system initialiseres (eller en wire skiftes) skal disse tal genkalibreres, ved at kigge på den konstante værdi som (Vin_?_gyro outputter)..


  // Til LCD skærmen låses gyro-værdierne til hele tal imellem 0 og 360, for at simplificere implementeringen, og gøre plads til alle 3 tal påå skærmen samtidigt.
  //Sikrer at der kun kan stå 0-360 grader på displayet.
  
  // Hvis Gyro-værdien falder under 0, tilføj 359 grader
  if (Vin_x_gyro < 0) {
    Vin_x_gyro = Vin_x_gyro + 359 ;
  }

  //  Hvis Gyro-værdien overstiger under 359, fjern 359 grader
  // 359, og ikke 360, da 360 = 0, Ve dat tillade både 360 og 00, ville vi tillade 361 værdier af græder
  if (Vin_x_gyro > 359) {
    Vin_x_gyro = Vin_x_gyro - 359;
  }

 // Gentag i y og z retningerne.
  if (Vin_y_gyro < 0) {
    Vin_y_gyro = Vin_y_gyro + 359 ;
  }
  if (Vin_y_gyro > 359) {
    Vin_y_gyro = Vin_y_gyro - 359;
  }

  if (Vin_z_gyro < 0) {
    Vin_z_gyro = Vin_z_gyro + 359 ;
  }
  if (Vin_z_gyro > 359) {
    Vin_z_gyro = Vin_z_gyro - 359;
  }

  // Printer display
  // Print GYRO til LCD

  // per default, 'peger' LCD skærmen på celle 1 i kolonne 1
  lcd.print("Pitch");
  // peg på celle 1 i kolonne 2 (Pitch-værdiens hundrede-position)
  lcd.setCursor(0, 1);
  // Isoler henholdsvis hundredende, tierne og et'erne.
  lcd.print((round(Vin_x_gyro) / 100U) % 10);
  // Peg på celle 2, kolonne 2, print tierne.
  lcd.setCursor(1, 1);
  lcd.print((round(Vin_x_gyro) / 10U % 10));
  // Peg på celle 3, kolonne 2, print en'erne.
  lcd.setCursor(2, 1);
  lcd.print((round(Vin_x_gyro) / 1U % 10));

  // Peg på celle 7, kolonne 1
  lcd.setCursor(6, 0);
  lcd.print("Roll");
  // peg på celle 7 i kolonne 2 (Roll-værdiens hundrede-position)
  lcd.setCursor(6, 1);
  lcd.print((round(Vin_y_gyro) / 100U) % 10);
  // Peg på celle 8, kolonne 2, print tierne.
  lcd.setCursor(7, 1);
  lcd.print((round(Vin_y_gyro) / 10U % 10));
   // Peg på celle 9, kolonne 2, print en'erne
  lcd.setCursor(8, 1);
  lcd.print((round(Vin_y_gyro) / 1U % 10));

   // Peg på celle 12, kolonne 1
  lcd.setCursor(11, 0);
  lcd.print("Yaw");
  // peg på celle 12 i kolonne 2 (Yaw-værdiens hundrede-position)
  lcd.setCursor(11, 1);
  lcd.print((round(Vin_z_gyro) / 100U) % 10);
  // Peg på celle 13, kolonne 2, print tierne.
  lcd.setCursor(12, 1);
  lcd.print((round(Vin_z_gyro) / 10U % 10));
  // Peg på celle 14, kolonne 2, print en'erne
  lcd.setCursor(13, 1);
  lcd.print((round(Vin_z_gyro) / 1U % 10));

  // Peg tilbage på første celle i første kolonne, således at når loopet gentages, overskrives de gamle værdier, uden dette, ville Pitch forsøges skrevet i celle 14, kolonne 2.
  lcd.setCursor(0, 0);


  // Kør Ultrasound Sensor (Fuckoff-Jacket kode)
  digitalWrite(MaxDistance_trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(MaxDistance_trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(MaxDistance_trigPin, LOW);
  // wait for signal to return
  // convert duration to distance in cm
  int MaxDistance_distance = 0.01715 * pulseIn(MaxDistance_echoPin, HIGH);

  // Ultrasonic 'Out of Range' statement
    if (MaxDistance_distance > 1000) {
      Serial.print("Distance = ");
      Serial.println("Out of Range");
      Serial.println("-----");

    } else {
      Serial.print("Distance = ");
      Serial.print(MaxDistance_distance);
      Serial.println("cm");
      Serial.println("-----");
    }


  // Vinkler, temperatur og Ultrasound distancemåling printes til .csv filen.
  //Serial.print("Vin_x ");
  Serial.print(Vin_x_gyro);
  Serial.print(",");
  //Serial.print("Vin_y");
  Serial.print(Vin_y_gyro);
  //Serial.print("Vin_z");
  Serial.print(",");
  Serial.print(Vin_z_gyro);
  //Serial.print("Temp = ");
  Serial.print(",");
  Serial.print(temp / 340.00 + 36.53);
  Serial.print(",");
  Serial.print(",");
  // Ultrasonic 'Out of Range' statement
  if (MaxDistance_distance > 1000) {
    Serial.println("Out of Range");

  } else {
    Serial.println(MaxDistance_distance);
  }

}

}

