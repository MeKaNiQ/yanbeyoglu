#include <Servo.h>

// Tanımlamalar ve pin atamaları
int sensorIRQ = 0;  // Dijital pin 2'ye bağlı kesme/interrupt pini
int sensorPin = 2;  // Akış sensörünün bağlı olduğu pin
int dip = 4;        // Dip sensörünün bağlı olduğu pin
int sarj = 3;       // Şarj motorunun bağlı olduğu pin
int top = 7;        // Top sensörünün bağlı olduğu pin
int desarj = 8;
int temp = 11;   // Deşarj motorunun bağlı olduğu pin
int pompa = 10;  // Pompa motorunun bağlı olduğu pin
int besleme = 12;
Servo sarjServo;    // Şarj servo motoru
Servo desarjServo;  // Deşarj servo motoru

int maksMiktar = 90;
float kalibrasyonFaktoru = 50;  // Akış sensörü kalibrasyon faktörü
float flowRate = 0.0;           // Akış hızı
float flowMiliLitre = 0;        // Hesaplanan akış miktarı
unsigned int toplamGecen = 0;   // Toplam akış miktarı
unsigned long sifirAni = 0;     // Geçen süreyi hesaplamak için kullanılan başlama zamanı
volatile int palsSay = 0;       // Akış sensöründen gelen pals sayısı
bool desarjReset = false;       // Sadece ilk kez 50'ye ulaşıldığında sıfırlamak için kullanılır
bool pompaflag = false;

void setup() {
  Serial.begin(9600);  // Seri haberleşmeyi başlat
  pinMode(pompa, OUTPUT);
  pinMode(desarj, OUTPUT);
  pinMode(sarj, OUTPUT);
  pinMode(besleme, OUTPUT);
  pinMode(sensorPin, INPUT);
  pinMode(dip, INPUT);
  pinMode(top, INPUT);
  pinMode(temp, INPUT);

  // Çıkışların, Projedeki varsayılan başlangıç durumlarını ayarla
  digitalWrite(pompa, HIGH);
  digitalWrite(sensorPin, HIGH);
  digitalWrite(sarj, HIGH);
  digitalWrite(desarj, HIGH);
  digitalWrite(besleme, HIGH);

  sarjServo.attach(6);    // Servo motoru 'sarj' pini ile bağla
  desarjServo.attach(5);  // Servo motoru 'desarj' pini ile bağla

  // Kesme isteğini başlat, (RISING modu da vardır!)
  attachInterrupt(sensorIRQ, palsSayac, FALLING);
}

void loop() {
  // Top sensörü çalışıyorsa, şarj motorunu çalıştır
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

  // 1000 ms geçtikten sonra ölçümleri yap
  if ((millis() - sifirAni) > 1000) {
    // Kesme/interrupt kaldır
    detachInterrupt(sensorIRQ);

    // Akış hızını hesapla
    flowRate = ((1000.0 / (millis() - sifirAni)) * palsSay) / kalibrasyonFaktoru;
    sifirAni = millis();                     // Zamanı güncelle
    flowMiliLitre = (flowRate / 60) * 1000;  // Akış miktarını hesapla
    toplamGecen += flowMiliLitre;            // Toplam akış miktarını güncelle

    Serial.print("DipSensor durum= ");
    Serial.print(digitalRead(dip));

    Serial.print("Desarj Motor durum= ");
    Serial.print(digitalRead(desarj));

    Serial.print("TOP Sensor durum= ");
    Serial.print(digitalRead(top));

    Serial.print("Sarj Motor durum= ");
    Serial.print(digitalRead(sarj));

    // Akış hızını ve toplam akış miktarını ekrana yazdır
    Serial.print("Flow rate: ");
    Serial.print(flowMiliLitre, DEC);
    Serial.print("mL/Second\t");

    Serial.print("Output Liquid Quantity: ");
    Serial.print(toplamGecen, DEC);
    Serial.println("mL\t");

    // temp Sensör aktif ise ve toplamGecen 50ml'ye ulaşana kadar desarj yap,
    /////// Maksimum miktara ulaşınca desarj motorunu kapat, Maksimum seviyeye gelmeden temp sensör kapalı konuma geçer ise desarj motorunu kapat
    if (digitalRead(temp) == LOW) {
      if (toplamGecen < maksMiktar) {
        digitalWrite(desarj, LOW);
      } else {
        if (!desarjReset) {
          digitalWrite(desarj, HIGH);
          desarjReset = true;  // Sıfırlama işlemi bir kez gerçekleşti
          pompaflag = true;
          // Sarj servo motorunu 80 dereceye getir
          slowMoveServo(sarjServo, 80, 5);
          slowMoveServo(desarjServo, 0, 5);
        }
      }
    } else {
      digitalWrite(desarj, HIGH);
      desarjReset = false;  // temp sensor kapalı olduğunda resetle
      toplamGecen = 0;      // temp sensor kapalı olduğunda toplamGecen'ı sıfırla
      if (pompaflag == true) {
        delay(1500);
        digitalWrite(pompa, LOW);
        delay(4000);
        digitalWrite(pompa, HIGH);
        pompaflag = false;
        // Sarj servo motorunu 0 dereceye getir
        slowMoveServo(sarjServo, 80, 5);
        slowMoveServo(desarjServo, 0, 5);
        pompaflag = false;
      }
    }

    // Pals sayacını sıfırla ve kesme/interrupt tekrar başlat
    palsSay = 0;
    attachInterrupt(sensorIRQ, palsSayac, FALLING);
  }
}
//  Servoların başlangıç ve hedef pozisyona geçişteki hızlarını ayarla
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
// Kesme/interrupt fonksiyonu: pals sayacını artır
void palsSayac() {
  palsSay++;
}
