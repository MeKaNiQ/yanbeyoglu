#include <Servo.h>

int sensorIRQ = 0;  
int sensorPin = 2;  
int dip = 4;       
int sarj = 3;    
int top = 7;       
int desarj = 8;
int temp = 11;   
int pompa = 10;  
int besleme = 12;
Servo sarjServo;  
Servo desarjServo;  

int maksMiktar = 90;
float kalibrasyonFaktoru = 50;  
float flowRate = 0.0;          
float flowMiliLitre = 0;       
unsigned int toplamGecen = 0;   
unsigned long sifirAni = 0;     
volatile int palsSay = 0;       
bool desarjReset = false;       
bool pompaflag = false;

void setup() {
  Serial.begin(9600);  
  pinMode(pompa, OUTPUT);
  pinMode(desarj, OUTPUT);
  pinMode(sarj, OUTPUT);
  pinMode(besleme, OUTPUT);
  pinMode(sensorPin, INPUT);
  pinMode(dip, INPUT);
  pinMode(top, INPUT);
  pinMode(temp, INPUT);


  digitalWrite(pompa, HIGH);
  digitalWrite(sensorPin, HIGH);
  digitalWrite(sarj, HIGH);
  digitalWrite(desarj, HIGH);
  digitalWrite(besleme, HIGH);

  sarjServo.attach(6);    
  desarjServo.attach(5);  

  
  attachInterrupt(sensorIRQ, palsSayac, FALLING);
}

void loop() {
 
  if (digitalRead(top) == LOW) {
    digitalWrite(sarj, LOW);
  } else {
    digitalWrite(sarj, HIGH);
  }

  if (digitalRead(dip) == HIGH) {
    digitalWrite(besleme, LOW);
  } else {
    digitalWrite(besleme, HIGH);
  }


  if ((millis() - sifirAni) > 1000) {
   
    detachInterrupt(sensorIRQ);

    // Akış hızını hesapla
    flowRate = ((1000.0 / (millis() - sifirAni)) * palsSay) / kalibrasyonFaktoru;
    sifirAni = millis();                     
    flowMiliLitre = (flowRate / 60) * 1000;  
    toplamGecen += flowMiliLitre;           

    Serial.print("DipSensor durum= ");
    Serial.print(digitalRead(dip));

    Serial.print("Desarj Motor durum= ");
    Serial.print(digitalRead(desarj));

    Serial.print("TOP Sensor durum= ");
    Serial.print(digitalRead(top));

    Serial.print("Sarj Motor durum= ");
    Serial.print(digitalRead(sarj));

    
    Serial.print("Flow rate: ");
    Serial.print(flowMiliLitre, DEC);
    Serial.print("mL/Second\t");

    Serial.print("Output Liquid Quantity: ");
    Serial.print(toplamGecen, DEC);
    Serial.println("mL\t");

    
    if (digitalRead(temp) == LOW) {
      if (toplamGecen < maksMiktar) {
        digitalWrite(desarj, LOW);
      } else {
        if (!desarjReset) {
          digitalWrite(desarj, HIGH);
          desarjReset = true;  
          pompaflag = true;
        
          slowMoveServo(sarjServo, 80, 5);
          slowMoveServo(desarjServo, 0, 5);
        }
      }
    } else {
      digitalWrite(desarj, HIGH);
      desarjReset = false; 
      toplamGecen = 0;     
      if (pompaflag == true) {
        delay(1500);
        digitalWrite(pompa, LOW);
        delay(4000);
        digitalWrite(pompa, HIGH);
        pompaflag = false;
      
        slowMoveServo(sarjServo, 80, 5);
        slowMoveServo(desarjServo, 0, 5);
        pompaflag = false;
      }
    }

   
    palsSay = 0;
    attachInterrupt(sensorIRQ, palsSayac, FALLING);
  }
}

void slowMoveServo(Servo servo, int hedefPozisyon, int motorhizi) {
  int baslamaPozisyonu = servo.read();
  if (baslamaPozisyonu < hedefPozisyon) {
    for (int pozisyon = baslamaPozisyonu; pozisyon <= hedefPozisyon; pozisyon += 1) {
      servo.write(pozisyon);
      delay(motorhizi);
    }
  } else {
    for (int pozisyon = baslamaPozisyonu; pozisyon >= hedefPozisyon; pozisyon -= 1) {
      servo.write(pozisyon);
      delay(motorhizi);
    }
  }
}

void palsSayac() {
  palsSay++;
}
