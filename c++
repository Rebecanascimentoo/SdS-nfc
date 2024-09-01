CODIGO C++


#include <SPI.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <MFRC522.h>
#include <SoftwareSerial.h>

// Definições dos pinos para o módulo NFC
#define RST_PIN 9          // Pino de reset do módulo NFC
#define SS_PIN 10          // Pino de seleção do módulo NFC

// Inicialização dos objetos
LiquidCrystal_I2C lcd(0x27, 16, 2); // Endereço do LCD (0x27), 16 colunas e 2 linhas
MFRC522 nfc(SS_PIN, RST_PIN);       // Configuração do módulo NFC
SoftwareSerial mySerial(2, 3);      // RX=2, TX=3 - comunicação com o banco de dados

void setup() {
    // Inicialização dos componentes
    lcd.begin();
    lcd.backlight();          // Liga a luz de fundo do LCD
    SPI.begin();             // Inicializa o SPI para comunicação com o módulo NFC
    nfc.PCD_Init();         // Inicializa o módulo NFC
    mySerial.begin(9600);   // Inicializa a comunicação serial com o banco de dados

    lcd.setCursor(0, 0);
    lcd.print("Aproxime o NFC");
}

void loop() {
    // Verifica se há um cartão NFC próximo
    if (!nfc.PICC_IsNewCardPresent() || !nfc.PICC_ReadCardSerial()) {
        return;  // Retorna se nenhum cartão for detectado
    }

    // Exibe o UID do cartão na tela LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Cartao lido:");

    String uid = "";
    for (byte i = 0; i < nfc.uid.size; i++) {
        uid += String(nfc.uid.uidByte[i], HEX);  // Concatena cada byte do UID em hexadecimal
    }
    uid.toUpperCase();  // Converte o UID para maiúsculas para garantir a correspondência no banco de dados

    lcd.setCursor(0, 1);
    lcd.print(uid);

    // Envia o UID para o banco de dados via serial
    mySerial.print("UID:");
    mySerial.println(uid);

    // Espera 2 segundos para permitir que o usuário retire o cartão
    delay(2000);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Aproxime o NFC");
}
