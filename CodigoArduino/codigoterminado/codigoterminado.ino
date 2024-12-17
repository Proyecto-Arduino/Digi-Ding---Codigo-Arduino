#include <SoftwareSerial.h>
#include <RTClib.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <LiquidCrystal_I2C.h>

RTC_DS3231 rtc;
SoftwareSerial ble(10, 11);
const int chipSelect = 4; // Pines de la SD (puede variar según tu shield)
const int pinRele = 7; // Pin donde se conecta el rele
LiquidCrystal_I2C lcd(0x27, 16, 2);

struct Horario {
  int hora;
  int minuto;
  int duracion;
};

Horario horarios[] = {
  {8, 5, 2},
  {8, 45, 4},
  {9, 40, 2},
  {10, 20, 4},
  {11, 15, 2},
  {11, 55, 2},
  {12, 35, 4},
  {14, 25, 2},
  {15, 5, 4},
  {15, 55, 2},
  {16, 35, 4},
  {17, 25, 2},
  {18, 5, 4}
};

void setup() {
  Serial.begin(9600);
  pinMode(pinRele, OUTPUT);
  digitalWrite(pinRele, LOW);

  lcd.init();
  lcd.backlight();

  if (!SD.begin(chipSelect)) {
    Serial.println("Error iniciando la tarjeta SD");
    return;
  }

  if (!rtc.begin()) {
    Serial.println("Modulo RTC no encontrado !");
    while (1);
  }
  DateTime fechaRTC = rtc.now(); 

  // Intentar leer la hora de la SD
  if (SD.exists("fecha.txt")) {
    File archivo = SD.open("fecha.txt");
    if (archivo) {
      String linea = archivo.readStringUntil('\n');
      archivo.close();
      int dia, mes, ano, hora, minuto, segundo;
      sscanf(linea.c_str(), "%d/%d/%d %d:%d:%d", &dia, &mes, &ano, &hora, &minuto, &segundo);
      DateTime fechaSD(ano, mes, dia, hora, minuto, segundo);

      // Comparar las fechas: si la de la SD es más reciente, ajustar el RTC
      if (fechaSD > fechaRTC) {
        rtc.adjust(fechaSD);
        Serial.println("Hora actualizada desde la SD:");
        Serial.println(linea);
      } else {
        Serial.println("La hora del RTC es más reciente o igual a la de la SD.");
      }
    }
  } else {
    // Ajustar al momento de la compilación si no hay datos en la SD
    rtc.adjust(DateTime(__DATE__, __TIME__));
    Serial.println("No se encontró fecha en la SD, ajustando a la hora de compilación.");
  }
}

void activarRele(int duracion) {
  digitalWrite(pinRele, HIGH);
  delay(duracion * 1000);
  digitalWrite(pinRele, LOW);
}

void loop() {
  DateTime fecha = rtc.now();
  Serial.print(fecha.day());
  Serial.print("/");
  Serial.print(fecha.month());
  Serial.print("/");
  Serial.print(fecha.year());
  Serial.print(" ");
  Serial.print(fecha.hour());
  Serial.print(":");
  Serial.print(fecha.minute());
  Serial.print(":");
  Serial.println(fecha.second());

  lcd.clear();
  lcd.setCursor(6, 0);
  lcd.print("Hora");
  lcd.setCursor(4, 1);
  lcd.print(fecha.hour());
  lcd.print(":");
  lcd.print(fecha.minute());
  lcd.print(":");
  lcd.print(fecha.second());

  for (int i = 0; i < sizeof(horarios) / sizeof(horarios[0]); i++) {
    if (fecha.hour() == horarios[i].hora && fecha.minute() == horarios[i].minuto && fecha.second() == 0) {
      activarRele(horarios[i].duracion);
    }
  }

  delay(1000);

  // Guardar la hora actual en la SD
  File archivo = SD.open("fecha.txt", FILE_WRITE);
  if (archivo) {
    archivo.print(fecha.day());
    archivo.print("/");
    archivo.print(fecha.month());
    archivo.print("/");
    archivo.print(fecha.year());
    archivo.print(" ");
    archivo.print(fecha.hour());
    archivo.print(":");
    archivo.print(fecha.minute());
    archivo.print(":");
    archivo.println(fecha.second());
    archivo.close();
  } else {
    Serial.println("Error al abrir fecha.txt");
  }
}
