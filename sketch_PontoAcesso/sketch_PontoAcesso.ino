//Bibliotecas
#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal.h>
#include <DS1307.h>

//Pinos
#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);
LiquidCrystal lcd(6, 7, 5, 4, 3, 2);
const int buzzer_led = 8;
DS1307 rtc(A4, A5);

//Variáveis
char st[20];
MFRC522::MIFARE_Key key;

int codMessage;

void setup() {
  //Seta led e buzzer
  pinMode(buzzer_led,OUTPUT);
  digitalWrite (8, LOW);
  
  // Inicia a serial
  Serial.begin(9600);   

  // Inicia  SPI bus
  SPI.begin();      

  // Inicia MFRC522
  mfrc522.PCD_Init();

  //Aciona o relogio
  rtc.halt(false);
    
  //Define o número de colunas e linhas do LCD:
  lcd.begin(16, 2);

  //Prepara chave - padrao de fabrica = FFFFFFFFFFFFh
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

   //As linhas abaixo setam a data e hora do modulo
  //e podem ser comentada apos a primeira utilizacao
  //rtc.setTime(12, 12, 0);    //Define o horario
  //rtc.setDate(8, 6, 2016);  //Define o dia, mes e ano
   
  //Definicoes do pino SQW/Out
  rtc.setSQWRate(SQW_RATE_1);
  rtc.enableSQW(true);
  
}

void loop() { 
  mostrarHora();
  
  leitura();
  delay(1000);   
  mensagem();
}

void mensagem() {
  codMessage = Serial.parseInt();
  switch(codMessage) {
    case 1:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Ponto Registrado");    
      delay(3000);                         
      lcd.clear();
    break;
    case 2:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Acesso Permitido");                             
      delay(3000);
      lcd.clear();
    break;
    case 3:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Acesso Negado");  
      delay(3000);                           
      lcd.clear();
    break;
    case 4:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Funcionario");     
      lcd.setCursor(0, 1);
      lcd.print("Suspenso");     
      delay(3000);                        
      lcd.clear();
    break;
    case 5:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Erro"); 
      lcd.setCursor(0, 1);
      lcd.print("contate o suporte");     
      delay(3000);                            
      lcd.clear();
    break;
  }
}

void leitura() {
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }
  
  String conteudo = "";
  byte letra;
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    conteudo.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    conteudo.concat(String(mfrc522.uid.uidByte[i], HEX));
  }  

  //Printa na serial o número do cartão
  Serial.println(conteudo);

  delay(2000);
  
  digitalWrite(buzzer_led, HIGH);
  delay(500);
   
  //Desligando o buzzer e led
  digitalWrite (buzzer_led, LOW);
}


void mostrarHora(){
  lcd.setCursor(0,0);
  lcd.print(rtc.getTimeStr());
  
  lcd.setCursor(0,1);
  lcd.print(rtc.getDateStr(FORMAT_SHORT));
  lcd.print(" ");
  }

void escrita() {
    lcd.clear();
    lcd.setCursor(2, 0);
    lcd.print("Modo gravacao");
    lcd.setCursor(3, 1);
    lcd.print("selecionado");
    delay(3000);
    modo_gravacao();
}

void modo_gravacao() {
  //Aguarda cartao
  while ( ! mfrc522.PICC_IsNewCardPresent()) {
    delay(100);
  }
  if ( ! mfrc522.PICC_ReadCardSerial())    return;

  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  
  byte buffer[34];
  byte block;
  MFRC522::StatusCode status;
  byte len;

  
  //Grava no bloco 1
  block = 1;
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A,
                                    block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    return;
  }
  status = mfrc522.MIFARE_Write(block, buffer, 16);
  if (status != MFRC522::STATUS_OK) {
    return;
  }

  //Grava no bloco 2
  block = 2;
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A,
                                    block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    return;
  }
  status = mfrc522.MIFARE_Write(block, &buffer[16], 16);
  if (status != MFRC522::STATUS_OK) {
    return;
  }

  //Grava no bloco 4
  block = 4;
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A,
                                    block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    return;
  }
  status = mfrc522.MIFARE_Write(block, buffer, 16);
  if (status != MFRC522::STATUS_OK) {
    return;
  }

  //Grava no bloco 5
  block = 5;
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A,
                                    block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    return;
  }
  status = mfrc522.MIFARE_Write(block, &buffer[16], 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    //return;
  } else {
    lcd.clear();
    lcd.print("Gravacao OK!");
  }

  mfrc522.PICC_HaltA(); // Halt PICC
  mfrc522.PCD_StopCrypto1();  // Stop encryption on PCD
  delay(1000);
}
