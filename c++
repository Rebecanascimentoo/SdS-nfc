//CODIGO CONTORLE DE ACESSO
#include <EEPROM.h>     // We are going to read and write PICC's UIDs from/to EEPROM
#include <SPI.h>        // RC522 Module uses SPI protocol
#include <MFRC522.h>  // Library for Mifare RC522 Devices
//#include <Wire.h>
//#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

#define COMMON_ANODE
#define wipeB 3     // Button pin for WipeMode
#define relay 8     // Set Relé no Pin 8
#define VRMled 7    // Set Led Vermelho no Pino 7
#define VRDled 6  // Set Led Verde no Pino 6

//LiquidCrystal_I2C lcd(0x27, 16, 2); // Endereço do LCD (0x27), 16 colunas e 2 linhas
//MFRC522 nfc(SS_PIN, RST_PIN);       // Configuração do módulo NFC
//SoftwareSerial mySerial(0, 1);      // RX=0, TX=1 - comunicação com o banco de dados

// Definições dos pinos para o módulo RFID
#define RST_PIN 9          // Pino de reset do módulo NFC
#define SS_PIN 10          // Pino de seleção do módulo NFC
MFRC522 mfrc522(SS_PIN, RST_PIN);  // Creação de instancia do MFRC522

bool programMode = false;  // initialize programming mode to false
uint8_t successRead;    // Variable integer to keep if we have Successful Read from Reader
byte storedCard[4];   // Stores an ID read from EEPROM
byte readCard[4];   // Stores scanned ID read from RFID Module
byte masterCard[4];   // Stores master card's ID read from EEPROM





/////////////////////////////////////////  Access Granted    ///////////////////////////////////
void granted ( uint16_t setDelay) {
//  digitalWrite(blueLed, LED_OFF);   // Turn off blue LED
  digitalWrite(VRMled, LOW);  // Turn off red LED
  digitalWrite(VRDled, HIGH);   // Turn on green LED
  digitalWrite(relay, HIGH);     // Unlock door!
  delay(1500);          // Aguarda 1,5s
  digitalWrite(relay, LOW);    // Volta a travar porta
  delay(1000);            // Hold green LED on for a second
}

///////////////////////////////////////// Access Denied  ///////////////////////////////////
void denied() {
  digitalWrite(VRDled, LOW);  // Make sure green LED is off
//  digitalWrite(blueLed, LED_OFF);   // Make sure blue LED is off
  digitalWrite(VRMled, HIGH);   // Turn on red LED
  digitalWrite(relay, LOW);    //Mantém Acesso fechado
  delay(1000);
}

///////////////////////////////////////// Get PICC's UID  //////////////////////////////////
uint8_t getID() {
  // Getting ready for Reading PICCs
  if ( ! mfrc522.PICC_IsNewCardPresent()) { //If a new PICC placed to RFID reader continue
    return 0;
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) {   //Since a PICC placed get Serial and continue
    return 0;
  }
  // There are Mifare PICCs which have 4 byte or 7 byte UID care if you use 7 byte PICC
  // I think we should assume every PICC as they have 4 byte UID
  // Until we support 7 byte PICCs
  Serial.println(F("Scanned PICC's UID:"));
  for ( uint8_t i = 0; i < 4; i++) {  //
    readCard[i] = mfrc522.uid.uidByte[i];
    Serial.print(readCard[i], HEX);
  }
  Serial.println("");
  mfrc522.PICC_HaltA(); // Stop reading
  return 1;
}


void ShowReaderDetails() {
  // Get the MFRC522 software version
  byte v = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
  Serial.print(F("MFRC522 Software Version: 0x"));
  Serial.print(v, HEX);
  if (v == 0x91)
    Serial.print(F(" = v1.0"));
  else if (v == 0x92)
    Serial.print(F(" = v2.0"));
  else
    Serial.print(F(" (unknown),probably a chinese clone?"));
  Serial.println("");
  // When 0x00 or 0xFF is returned, communication probably failed
  if ((v == 0x00) || (v == 0xFF)) {
    Serial.println(F("WARNING: Communication failure, is the MFRC522 properly connected?"));
    Serial.println(F("SYSTEM HALTED: Check connections."));
    // Visualize system is halted
    digitalWrite(VRDled, LOW);  // Make sure green LED is off
//    digitalWrite(blueLed, LED_OFF);   // Make sure blue LED is off
    digitalWrite(VRMled, HIGH);   // Turn on red LED
    while (true); // do not go further
  }
}
////////////////////////////////////////// Normal Mode Led  ///////////////////////////////////
void normalModeOn () {
//  digitalWrite(blueLed, LED_ON);  // Blue LED ON and ready to read card
  digitalWrite(VRMled, LOW);  // Make sure Red LED is off
  digitalWrite(VRDled, HIGH);  // Make sure Green LED is off
  digitalWrite(relay, HIGH);    // Make sure Door is Locked
}

//////////////////////////////////////// Read an ID from EEPROM //////////////////////////////
void readID( uint8_t number ) {
  uint8_t start = (number * 4 ) + 2;    // Figure out starting position
  for ( uint8_t i = 0; i < 4; i++ ) {     // Loop 4 times to get the 4 Bytes
    storedCard[i] = EEPROM.read(start + i);   // Assign values read from EEPROM to array
  }
}

///////////////////////////////////////// Add ID to EEPROM   ///////////////////////////////////
void writeID( byte a[] ) {
  if ( !findID( a ) ) {     // Before we write to the EEPROM, check to see if we have seen this card before!
    uint8_t num = EEPROM.read(0);     // Get the numer of used spaces, position 0 stores the number of ID cards
    uint8_t start = ( num * 4 ) + 6;  // Figure out where the next slot starts
    num++;                // Increment the counter by one
    EEPROM.write( 0, num );     // Write the new count to the counter
    for ( uint8_t j = 0; j < 4; j++ ) {   // Loop 4 times
      EEPROM.write( start + j, a[j] );  // Write the array values to EEPROM in the right position
    }
    successWrite();
    Serial.println(F("Succesfully added ID record to EEPROM"));
  }
  else {
    failedWrite();
    Serial.println(F("Failed! There is something wrong with ID or bad EEPROM"));
  }
}

///////////////////////////////////////// Remove ID from EEPROM   ///////////////////////////////////
void deleteID( byte a[] ) {
  if ( !findID( a ) ) {     // Before we delete from the EEPROM, check to see if we have this card!
    failedWrite();      // If not
    Serial.println(F("Failed! There is something wrong with ID or bad EEPROM"));
  }
  else {
    uint8_t num = EEPROM.read(0);   // Get the numer of used spaces, position 0 stores the number of ID cards
    uint8_t slot;       // Figure out the slot number of the card
    uint8_t start;      // = ( num * 4 ) + 6; // Figure out where the next slot starts
    uint8_t looping;    // The number of times the loop repeats
    uint8_t j;
    uint8_t count = EEPROM.read(0); // Read the first Byte of EEPROM that stores number of cards
    slot = findIDSLOT( a );   // Figure out the slot number of the card to delete
    start = (slot * 4) + 2;
    looping = ((num - slot) * 4);
    num--;      // Decrement the counter by one
    EEPROM.write( 0, num );   // Write the new count to the counter
    for ( j = 0; j < looping; j++ ) {         // Loop the card shift times
      EEPROM.write( start + j, EEPROM.read(start + 4 + j));   // Shift the array values to 4 places earlier in the EEPROM
    }
    for ( uint8_t k = 0; k < 4; k++ ) {         // Shifting loop
      EEPROM.write( start + j + k, 0);
    }
    successDelete();
    Serial.println(F("Succesfully removed ID record from EEPROM"));
  }
}

///////////////////////////////////////// Check Bytes   ///////////////////////////////////
bool checkTwo ( byte a[], byte b[] ) {   
  for ( uint8_t k = 0; k < 4; k++ ) {   // Loop 4 times
    if ( a[k] != b[k] ) {     // IF a != b then false, because: one fails, all fail
       return false;
    }
  }
  return true;  
}

///////////////////////////////////////// Find Slot   ///////////////////////////////////
uint8_t findIDSLOT( byte find[] ) {
  uint8_t count = EEPROM.read(0);       // Read the first Byte of EEPROM that
  for ( uint8_t i = 1; i <= count; i++ ) {    // Loop once for each EEPROM entry
    readID(i);                // Read an ID from EEPROM, it is stored in storedCard[4]
    if ( checkTwo( find, storedCard ) ) {   // Check to see if the storedCard read from EEPROM
      // is the same as the find[] ID card passed
      return i;         // The slot number of the card
    }
  }
}

///////////////////////////////////////// Find ID From EEPROM   ///////////////////////////////////
bool findID( byte find[] ) {
  uint8_t count = EEPROM.read(0);     // Read the first Byte of EEPROM that
  for ( uint8_t i = 1; i < count; i++ ) {    // Loop once for each EEPROM entry
    readID(i);          // Read an ID from EEPROM, it is stored in storedCard[4]
    if ( checkTwo( find, storedCard ) ) {   // Check to see if the storedCard read from EEPROM
      return true;
    }
    else {    // If not, return false
    }
  }
  return false;
}

///////////////////////////////////////// Write Success to EEPROM   ///////////////////////////////////
// Flashes the green LED 3 times to indicate a successful write to EEPROM
void successWrite() {
//  digitalWrite(blueLed, LED_OFF);   // Make sure blue LED is off
  digitalWrite(VRMled, LOW);  // Make sure red LED is off
  digitalWrite(VRDled, LOW);  // Make sure green LED is on
  delay(200);
  digitalWrite(VRDled, HIGH);   // Make sure green LED is on
  delay(200);
  digitalWrite(VRDled, LOW);  // Make sure green LED is off
  delay(200);
  digitalWrite(VRDled, HIGH);   // Make sure green LED is on
  delay(200);
  digitalWrite(VRDled, LOW);  // Make sure green LED is off
  delay(200);
  digitalWrite(VRDled, HIGH);   // Make sure green LED is on
  delay(200);
}

///////////////////////////////////////// Write Failed to EEPROM   ///////////////////////////////////
// Flashes the red LED 3 times to indicate a failed write to EEPROM
void failedWrite() {
//  digitalWrite(blueLed, LED_OFF);   // Make sure blue LED is off
  digitalWrite(VRMled, LOW);  // Make sure red LED is off
  digitalWrite(VRDled, LOW);  // Make sure green LED is off
  delay(200);
  digitalWrite(VRMled, HIGH);   // Make sure red LED is on
  delay(200);
  digitalWrite(VRMled, LOW);  // Make sure red LED is off
  delay(200);
  digitalWrite(VRMled, HIGH);   // Make sure red LED is on
  delay(200);
  digitalWrite(VRMled, LOW);  // Make sure red LED is off
  delay(200);
  digitalWrite(VRMled, HIGH);   // Make sure red LED is on
  delay(200);
}

///////////////////////////////////////// Success Remove UID From EEPROM  ///////////////////////////////////
// Flashes the blue LED 3 times to indicate a success delete to EEPROM
void successDelete() {
//  digitalWrite(blueLed, LED_OFF);   // Make sure blue LED is off
  digitalWrite(VRMled, LOW);  // Make sure red LED is off
  digitalWrite(VRDled, LOW);  // Make sure green LED is off
  delay(200);
  digitalWrite(VRMled, HIGH);  // Make sure blue LED is on
  digitalWrite(VRDled, HIGH);  // Make sure blue LED is on
  delay(200);
  digitalWrite(VRMled, LOW);   // Make sure blue LED is off
  digitalWrite(VRDled, LOW);  // Make sure blue LED is on
  delay(200);
  digitalWrite(VRMled, HIGH);  // Make sure blue LED is on
  digitalWrite(VRDled, HIGH);  // Make sure blue LED is on
  delay(200);
  digitalWrite(VRMled, LOW);  // Make sure blue LED is on
  digitalWrite(VRDled, LOW);  // Make sure blue LED is on
  delay(200);
}

////////////////////// Check readCard IF is masterCard   ///////////////////////////////////
// Check to see if the ID passed is the master programing card
bool isMaster( byte test[] ) {
  return checkTwo(test, masterCard);
}

bool monitorWipeButton(uint32_t interval) {
  uint32_t now = (uint32_t)millis();
  while ((uint32_t)millis() - now < interval)  {
    // check on every half a second
    if (((uint32_t)millis() % 500) == 0) {
      if (digitalRead(wipeB) != LOW)
        return false;
    }
  }
  return true;
}







void setup() {
    // Configuração dos Pinos:
    pinMode(wipeB, INPUT);   // Enable pin resistor
    pinMode(VRMled, OUTPUT);
    pinMode(VRDled, OUTPUT);
    pinMode(relay, OUTPUT);

    //Configuração de Protocolos
    //nfc.PCD_Init();         // Inicializa o módulo NFC
    Serial.begin(9600);   // Inicializa a comunicação serial com o banco de dados
    SPI.begin();      // Init SPI bus
    Serial.println("Iniciando...");
    mfrc522.PCD_Init();   // Init MFRC522
    
    digitalWrite(relay, LOW);    // Make sure door is locked
    digitalWrite(VRMled, HIGH);  // Make sure led is on
    digitalWrite(VRDled, HIGH);  // Make sure led is on
    delay(400);       // Optional delay. Some board do need more time after init to be ready, see Readme
    //mfrc522.PCD_DumpVersionToSerial();  // Show details of PCD - MFRC522 Card Reader details
    
    Serial.println("Aproxime o cartão");
    ShowReaderDetails();  // Show details of PCD - MFRC522 Card Reader details

/*    //Wipe Code - If the Button (wipeB) Pressed while setup run (powered on) it wipes EEPROM
  if (digitalRead(wipeB) == LOW) {  // when button pressed pin should get low, button connected to ground
    digitalWrite(VRMled, HIGH); // Red Led stays on to inform user we are going to wipe
    Serial.println(F("Wipe Button Pressed"));
    Serial.println(F("Você tem 10s para Cancelar"));
    Serial.println(F("Esta ação apagará todos os registros e não poderá ser desfeita")); //This will be remove all records and cannot be undone
    bool buttonState = monitorWipeButton(10000); // Give user enough time to cancel operation
    if (buttonState == true && digitalRead(wipeB) == LOW) {    // If button still be pressed, wipe EEPROM
      Serial.println(F("Starting Wiping EEPROM"));
      for (uint16_t x = 0; x < EEPROM.length(); x = x + 1) {    //Loop end of EEPROM address
        if (EEPROM.read(x) == 0) {              //If EEPROM address 0
          // do nothing, already clear, go to the next address in order to save time and reduce writes to EEPROM
        }
        else {
          EEPROM.write(x, 0);       // if not write 0 to clear, it takes 3.3mS
        }
      }
      Serial.println(F("EEPROM Successfully Wiped"));
      digitalWrite(VRLled, LOW);  // visualize a successful wipe
      delay(200);
      digitalWrite(VRLled, HIGH);
      delay(200);
      digitalWrite(VRLled, LOW);
      delay(200);
      digitalWrite(VRLled, HIGH);
      delay(200);
      digitalWrite(VRLled, LOW);
    }
    else {
      Serial.println(F("Wiping Cancelled")); // Show some feedback that the wipe button did not pressed for 15 seconds
      digitalWrite(redLed, LED_OFF);
    }
  }*/

  // Verificar se há um Cartão Mestre cadastrado, se não houver, escolher um
  // Esta função deverá ser usada apenas para redefinir o Cartão Mestre
  // You can keep other EEPROM records just write other than 143 to EEPROM address 1
  // EEPROM address 1 should hold magical number which is '143'
/*  if (EEPROM.read(1) != 143) {
    Serial.println(F("Não há Cartão Maestre Definido!"));
    Serial.println(F("Scan A PICC para definir como Master Card"));
    do {
      successRead = getID();            // sets successRead to 1 when we get read from reader otherwise 0
      digitalWrite(VRDled, HIGH);    // Visualize Master Card need to be defined
      delay(200);
      digitalWrite(VRDled, LOW);
      delay(200);
    }
    while (!successRead);                  // Program will not go further while you not get a successful read
    for ( uint8_t j = 0; j < 4; j++ ) {        // Loop 4 times
      EEPROM.write( 2 + j, readCard[j] );  // Write scanned PICC's UID to EEPROM, start from address 3
    }
    EEPROM.write(1, 143);                  // Write to EEPROM we defined Master Card.
    Serial.println(F("Master Card Definido"));
  }
  Serial.println(F("-------------------"));
  Serial.println(F("Master Card's UID"));
  for ( uint8_t i = 0; i < 4; i++ ) {          // Read Master Card's UID from EEPROM
    masterCard[i] = EEPROM.read(2 + i);    // Write it to masterCard
    Serial.print(masterCard[i], HEX);
  }
  Serial.println("");
  Serial.println(F("-------------------"));
  Serial.println(F("Everything is ready"));
  Serial.println(F("Waiting PICCs to be scanned"));
//  cycleLeds();    // Everything ready lets give user some feedback by cycling leds
*/
}

void loop() {
/*  // Verifica se há um cartão NFC próximo
    if (!nfc.PICC_IsNewCardPresent() || !nfc.PICC_ReadCardSerial()) {
        return;  // Retorna se nenhum cartão for detectado
    }
    // Exibe o UID do cartão na tela LCD
    Serial.println("Cartao lido:");
    String uid = "";
    for (byte i = 0; i < nfc.uid.size; i++) {
        uid += String(nfc.uid.uidByte[i], HEX);  // Concatena cada byte do UID em hexadecimal
    }
    uid.toUpperCase();  // Converte o UID para maiúsculas para garantir a correspondência no banco de dados
    // Envia o UID para o banco de dados via serial
    Serial.print("UID:");
    Serial.println(uid);
    mySerial.println(uid);
    // Espera 1 segundo para permitir que o usuário retire o cartão
    delay(1000);
    Serial.println("Aproxime um Cartão");*/

    do {
    successRead = getID();  // sets successRead to 1 when we get read from reader otherwise 0
    // When device is in use if wipe button pressed for 10 seconds initialize Master Card wiping
    if (digitalRead(wipeB) == HIGH) { // Check if button is pressed
      // Visualize normal operation is iterrupted by pressing wipe button Red is like more Warning to user
      digitalWrite(VRMled, HIGH);  // Make sure led is off
      digitalWrite(VRDled, LOW);  // Make sure led is off
      
      // Give some feedback
      Serial.println(F("Wipe Button Pressed"));
      Serial.println(F("Master Card will be Erased! in 10 seconds"));
/*      bool buttonState = monitorWipeButton(10000); // Give user enough time to cancel operation
      if (buttonState == true && digitalRead(wipeB) == LOW) {    // If button still be pressed, wipe EEPROM
        EEPROM.write(1, 0);                  // Reset Magic Number.
        Serial.println(F("Master Card Erased from device"));
        Serial.println(F("Please reset to re-program Master Card"));
        while (1);
      }
      Serial.println(F("Master Card Erase Cancelled"));*/
    }
    if (programMode) {
//      cycleLeds();       // Program Mode cycles through Red Green Blue waiting to read a new card
    }
    else {
      normalModeOn();     // Normal mode, blue Power LED is on, all others are off
    }
  }
  while (!successRead);   //the program will not go further while you are not getting a successful read
  if (programMode) {
    if ( isMaster(readCard) ) { //When in program mode check First If master card scanned again to exit program mode
      Serial.println(F("Master Card Scanned"));
      Serial.println(F("Exiting Program Mode"));
      Serial.println(F("-----------------------------"));
      programMode = false;
      return;
    }
    else {
      if ( findID(readCard) ) { // If scanned card is known delete it
        Serial.println(F("I know this PICC, removing..."));
        deleteID(readCard);
        Serial.println("-----------------------------");
        Serial.println(F("Scan a PICC to ADD or REMOVE to EEPROM"));
      }
      else {                    // If scanned card is not known add it
        Serial.println(F("I do not know this PICC, adding..."));
        writeID(readCard);
        Serial.println(F("-----------------------------"));
        Serial.println(F("Scan a PICC to ADD or REMOVE to EEPROM"));
      }
    }
  }
  else {
    if ( isMaster(readCard)) {    // If scanned card's ID matches Master Card's ID - enter program mode
      programMode = true;
      Serial.println(F("Hello Master - Entered Program Mode"));
      uint8_t count = EEPROM.read(0);   // Read the first Byte of EEPROM that
      Serial.print(F("I have "));     // stores the number of ID's in EEPROM
      Serial.print(count);
      Serial.print(F(" record(s) on EEPROM"));
      Serial.println("");
      Serial.println(F("Scan a PICC to ADD or REMOVE to EEPROM"));
      Serial.println(F("Scan Master Card again to Exit Program Mode"));
      Serial.println(F("-----------------------------"));
    }
    else {
      if ( findID(readCard) ) { // If not, see if the card is in the EEPROM
        Serial.println(F("Seja bem-vindo, Cartão Autorizado!"));
        granted(300);         // Open the door lock for 300 ms
      }
      else {      // If not, show that the ID was not valid
        Serial.println(F("Cartão NÃO autorizado!"));
        denied();
      }
    }
  }
}
