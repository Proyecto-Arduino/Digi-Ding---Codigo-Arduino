#include <SoftwareSerial.h>
#include <RTClib.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <LiquidCrystal_I2C.h>

RTC_DS3231 rtc;
SoftwareSerial ble(10, 11);
int verde = 7;
int rojo = 8;
char cadena[255] = {0};
int i = 0;
const int chipSelect = 4; // Pines de la SD (puede variar según tu shield)
LiquidCrystal_I2C lcd(0x27, 16, 2);
void setup() {
  Serial.begin(9600);
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
  lcd.init();
  lcd.backlight();
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
  // Actualizar la pantalla LCD con la hora actual
  lcd.clear();
  lcd.setCursor(6, 0);
  lcd.print("Hora");
  lcd.setCursor(4, 1);
  lcd.print(fecha.hour());
  lcd.print(":");
  lcd.print(fecha.minute());
  lcd.print(":");
  lcd.print(fecha.second());

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