#include "Wire.h"
int Vin_xz = 0.0;
int Vin_yz = 0.0;
int Vin_xy = 0.0;
int gyro_nu = 0.0;
int Vin_acc_x = 0.0;
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
int Test_x = 0.0;
int Test_y = 0.0;
const int MaxDistance_trigPin = 2;
const int MaxDistance_echoPin = 3;

#include <LiquidCrystal.h>
const int rs = 12, en = 11, d4 = 10, d5 = 9, d6 = 8, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void setup() {
  Serial.begin(9600);
  // put your setup code here, to run once:
  Wire.begin();
  Wire.beginTransmission(0x68);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission(true);

  // Acceleraometeret skal køre en gang, og logge værdier, for at kunne finde orinteringen:
  Wire.beginTransmission(0x68);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(0x68, 2 * 3, true); //en pakke er 8 bits. 3 sensorer, af datapakker 2.

  int16_t accelerometer_x = Wire.read() << 8 | Wire.read(); //16 bit, med 8 int af gangen. read 8 og skub 8 tal til venstre
  int16_t accelerometer_y = Wire.read() << 8 | Wire.read();
  int16_t accelerometer_z = (Wire.read() << 8 | Wire.read());

  Vin_y_gyro = -((atan2(accelerometer_x, accelerometer_z) * 180) / 3.1415);
  Vin_x_gyro = ((atan2(accelerometer_y, accelerometer_z) * 180) / 3.1415);


  //Alt det der skal printes til excel dokumentet:

  Serial.print("Vin_x ");
  Serial.print(",");
  Serial.print("Vin_y");
  Serial.print(",");
  Serial.print("Vin_Z");
  Serial.print(",");
  Serial.print("Temperatur");
  Serial.print(",");
  Serial.println("Afstand");

  pinMode(MaxDistance_trigPin, OUTPUT);
  pinMode(MaxDistance_echoPin, INPUT);

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);

}
void loop() {
  // put your main code here, to run repeatedly:
  gyro_x_f = gyro_x;
  gyro_y_f = gyro_y;
  gyro_z_f = gyro_z;
  tid_f = tid_n;
  tid_n = millis();

  Wire.beginTransmission(0x68);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(0x68, 2 * 7, true); //en pakke er 8 bits. 3 sensorer, af datapakker 2.

  // Læser de forskellige værdier.
  int16_t accelerometer_x = Wire.read() << 8 | Wire.read(); //16 bit, med 8 int af gangen. read 8 og skub 8 tal til venstre
  int16_t accelerometer_y = Wire.read() << 8 | Wire.read();
  int16_t accelerometer_z = (Wire.read() << 8 | Wire.read());
  int16_t temp = Wire.read() << 8 | Wire.read();
  int16_t gyro_x = Wire.read() << 8 | Wire.read();
  int16_t gyro_y = Wire.read() << 8 | Wire.read();
  int16_t gyro_z = Wire.read() << 8 | Wire.read();



  // Vinkel med gyro, hvor accelerationscalibreringen benyttes som startværdi.
  Vin_x_gyro = Vin_x_gyro + (((tid_n - tid_f) / 1000) * (((gyro_x + gyro_x_f) + 390) / 131));
  Vin_y_gyro = Vin_y_gyro + (((tid_n - tid_f) / 1000) * (((gyro_y + gyro_y_f) - 125) / 131));
  Vin_z_gyro = Vin_z_gyro + (((tid_n - tid_f) / 1000) * (((gyro_z + gyro_z_f) + 0) / 131));

  //Sikrer at der kun kan stå 0-360 grader på displayet.
  if (Vin_x_gyro < 0) {
    Vin_x_gyro = Vin_x_gyro + 359 ;
  }
  if (Vin_x_gyro > 359) {
    Vin_x_gyro = Vin_x_gyro - 359;
  }

 
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

  //Printer display
  // Print GYRO til LCD
  lcd.print("Pitch");
  lcd.setCursor(0, 1);
  lcd.print((round(Vin_x_gyro) / 100U) % 10);
  lcd.setCursor(1, 1);
  lcd.print((round(Vin_x_gyro) / 10U % 10));
  lcd.setCursor(2, 1);
  lcd.print((round(Vin_x_gyro) / 1U % 10));
  lcd.setCursor(6, 0);
  lcd.print("Roll");
  lcd.setCursor(6, 1);
  lcd.print((round(Vin_y_gyro) / 100U) % 10);
  lcd.setCursor(7, 1);
  lcd.print((round(Vin_y_gyro) / 10U % 10));
  lcd.setCursor(8, 1);
  lcd.print((round(Vin_y_gyro) / 1U % 10));
  lcd.setCursor(11, 0);
  lcd.print("Yaw");
  lcd.setCursor(11, 1);
  lcd.print((round(Vin_z_gyro) / 100U) % 10);
  lcd.setCursor(12, 1);
  lcd.print((round(Vin_z_gyro) / 10U % 10));
  lcd.setCursor(13, 1);
  lcd.print((round(Vin_z_gyro) / 1U % 10));
  lcd.setCursor(0, 0);


  digitalWrite(MaxDistance_trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(MaxDistance_trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(MaxDistance_trigPin, LOW);
  // wait for signal to return
  // convert duration to distance in cm
  int MaxDistance_distance = 0.01715 * pulseIn(MaxDistance_echoPin, HIGH);


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
  Serial.println(MaxDistance_distance);








}



// This is a test to see GitHub update rate
// This line is sent from a Live Share Participant, and uploaded to GitHub!

