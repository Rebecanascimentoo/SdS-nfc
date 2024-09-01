#include <SPI.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <MFRC522.h>
#include <SoftwareSerial.h>
//SPI.h para comunicação SPI 
//Wire.h para comunicação I2C.
//LiquidCrystal_I2C.h para controle da tela LCD.
//MFRC522.h para controle do módulo NFC.
//SoftwareSerial.h se for usar uma comunicação serial alternativa para conectar com o banco de dados.

#define RST_PIN 9          // Pino reset do módulo NFC
#define SS_PIN 10          // Pino de seleção do módulo NFC

LiquidCrystal_I2C lcd(0x27, 16, 2); // Endereço do LCD, com 16 colunas e 2 linhas
MFRC522 nfc(SS_PIN, RST_PIN);       // Configuração do módulo NFC
SoftwareSerial mySerial(2, 3);      // RX, TX - comunicação com banco de dados

void setup() {
    // Inicialização das bibliotecas
    lcd.begin();
    lcd.backlight();
    SPI.begin();
    nfc.PCD_Init();
    mySerial.begin(9600);

    lcd.setCursor(0, 0);
    lcd.print("Aproxime o NFC");
}

void loop() {
    // Verificar se há um cartão NFC próximo
    if (!nfc.PICC_IsNewCardPresent() || !nfc.PICC_ReadCardSerial()) {
        return;
    }

    // Exibir o UID do cartão na tela LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Cartao lido:");

    String uid = "";
    for (byte i = 0; i < nfc.uid.size; i++) {
        uid += String(nfc.uid.uidByte[i], HEX);
    }

    lcd.setCursor(0, 1);
    lcd.print(uid);

    // Enviar o UID para o banco de dados via serial
    mySerial.print("UID:");
    mySerial.println(uid);

    // Espera 2 segundos para permitir que o usuário retire o cartão
    delay(2000);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Aproxime o NFC");
}
