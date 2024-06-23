  /* Biblioteci Arduion. */
#include <LiquidCrystal.h> 
#include <Servo.h>  

/* Declararea Pinilor Analogici. */
int senzorTemp = A8;
int PinSenzorGaz = A12;
int PinSenzorUmiditate = A10;
/* Declararea Pinilor Digitali. */
int PinSenzorMiscare = 46;
int PinSenzorVibratii = 48;
int PinBuzzer = 31;
int BUTTON_PIN_1 =  53;
int BUTTON_PIN_2 =  51;
int BUTTON_PIN_3 =  52;
int LED_PIN_1 = 47;
int LED_PIN_2 = 45;
int LED_PIN_3 = 50;
int LED_Senzor_Gaz = 41;
int LED_Senzor_Miscare = 39;
int ARM_BUTTON_PIN = 37;
LiquidCrystal lcd( 8,  9,  4,  5,  6,  7);

/* Initializarea Variabilelor ce citesc date de la pini digitali. */
int SenzorVibratii = LOW;
int ValoareSenzorMiscare = LOW;
int lastButtonState_1 = LOW;
int ledState_1 = LOW;
int lastButtonState_2 = LOW;
int ledState_2 = LOW;
int lastButtonState_3 = LOW;
int ledState_3 = LOW;
int ledState_miscare = LOW;
int ledState_gaz = LOW;
int lastButtonState_4 = LOW;
int TransfCels = 0;
int VarUmid = 0;
/* Define-uri din cadrul codului. */
#define GazDetectat 500
/* Flag-uri din cadrul codului. */
int flag_activare_securitate = 0;
int flag_intrus = 0;
int flag_emisie_gaz = 0;
void setup() {
  
  pinMode(PinBuzzer, OUTPUT);
  pinMode(PinSenzorVibratii, INPUT);
  pinMode(PinSenzorMiscare, INPUT);
  pinMode(LED_PIN_1, OUTPUT);
  pinMode(LED_PIN_2, OUTPUT);
  pinMode(LED_PIN_3, OUTPUT);
  pinMode(LED_Senzor_Miscare, OUTPUT);
  pinMode(LED_Senzor_Gaz, OUTPUT);
  pinMode(BUTTON_PIN_1, INPUT_PULLUP);
  pinMode(BUTTON_PIN_2, INPUT_PULLUP);
  pinMode(BUTTON_PIN_3, INPUT_PULLUP);
  pinMode(ARM_BUTTON_PIN, INPUT_PULLUP);
  Serial1.begin(9600);
  lcd.begin(16, 2);
  lcd.setCursor(0,0);
  lcd.print("SMART HOME");
}

void loop() {
  
  DetectieGaz();
   DetectareVibratii();
  ControlLCD();
   LuminaCamera1();
   LuminaCamera2();
   LuminaCamera3();
   ActivareSecuritate();
   AfisareTemperaturaUmiditateAplicatie();
   ControlAplicatie();
  
}

void AfisareTemperaturaUmiditateAplicatie()
{
  CalculTemperatura();
  CitireUmiditate();
   delay(300);
  String data = "Temperatura:" + String(TransfCels) +" degC"+ " " +"Umiditate:" + String(VarUmid) + "%"; 
  Serial1.println(String(data));

}

void ActivareSecuritate()
{
  byte buttonState = digitalRead(ARM_BUTTON_PIN);
  if(buttonState != lastButtonState_4) {
    lastButtonState_4 = buttonState;
    if (buttonState == LOW) {
      if(flag_activare_securitate == 1)
      {
        flag_activare_securitate = 0;
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print(" Securitate ");
          lcd.setCursor(0,1);
          lcd.print(" Dezactivata ");
          delay(2000);
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("SMART HOME");
      }
      else if(flag_activare_securitate == 0)
      {
        flag_activare_securitate = 1;
          lcd.clear();
          lcd.setCursor(0,0);
          for(int i = 9 ; i > 0 ; i--)
          {
            lcd.setCursor(0,0);
            lcd.print("Securitate ");
            lcd.setCursor(0,1);
            lcd.print("Activa in:");
            lcd.setCursor(10,1);
            lcd.print(i);
            lcd.setCursor(12,1);
            lcd.print("sec");
            delay(1000);
            
          }
          lcd.clear();
          lcd.print(" Securitate ");
          lcd.setCursor(0,1);
          lcd.print(" Activata ");
          delay(2000);
          lcd.clear();
      }
    }
  }

  DetectieMiscare(flag_activare_securitate);

}
void ControlAplicatie()
{
  if (Serial1.available() > 0) {
    char command = Serial1.read();
    switch (command) {
      case '1':
        digitalWrite(LED_PIN_1, HIGH);
        ledState_1 = HIGH;
        break;
      case '2':
        digitalWrite(LED_PIN_1, LOW);
        ledState_1 = LOW;
        break;
      case '3':
        digitalWrite(LED_PIN_2, HIGH);
        ledState_2 = HIGH;
        break;
      case '4':
        digitalWrite(LED_PIN_2, LOW);
        ledState_2 = LOW;
        break;
      case '5':
        digitalWrite(LED_PIN_3, HIGH);
        ledState_3 = HIGH;
        break;
      case '6':
        digitalWrite(LED_PIN_3, LOW);
        ledState_3 = LOW;
        break;
    }
  }
}
void ControlLCD()
{
  int x;
 x = analogRead (0);
 lcd.setCursor(10,1);
 
 if (x < 60) {
  if(ledState_miscare == HIGH || ledState_gaz == HIGH)
  {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("LED RESETAT");
      digitalWrite(LED_Senzor_Miscare, LOW);
      digitalWrite(LED_Senzor_Gaz, LOW);
      ledState_miscare = LOW;
      ledState_gaz = LOW;
      delay(3000);
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("SMART HOME");
      

  }

 }
 else if (x < 200) {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Temp:");
    lcd.print(TransfCels);
    lcd.print(" degC");
 }
 else if (x < 400){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Umiditate:");
    lcd.print(VarUmid);
    lcd.print("%");
 }
  else if (x < 600){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("SMART HOME");
 }

 
}

void DetectieGaz()
{
  int SenzorGAZ = analogRead(PinSenzorGaz);
  if(SenzorGAZ > GazDetectat)
  { 
    /* Au fost detectate emisii de gaz. */
    SunetAlarma(LED_Senzor_Gaz,2);
    
  }
  else
  {
    /* Nu au fost detectate emisii de gaz. */
    StopAlarma();
  }

}
void DetectareVibratii()
{
  SenzorVibratii = digitalRead(PinSenzorVibratii);
  if(SenzorVibratii == HIGH)
  {
    /* Au fost detectate vibratii. */
    AlarmaCutremur();
  }
  else
  {
    /* Nu au fost detectate vibratii. */
    StopAlarma();
  }
}

void DetectieMiscare(int flag_activare)
{
  if(flag_activare == 1)
  {
    ValoareSenzorMiscare = digitalRead(PinSenzorMiscare);
       if(ValoareSenzorMiscare == HIGH)
          {
            /* A fost detectata miscare. */
            SunetAlarma(LED_Senzor_Miscare,1);

          }
          else
          {
            /* NU a fost detectata miscare. */
            StopAlarma();
          }
  }
}

void CalculTemperatura()
{
  int SenzorTemperatura = analogRead(senzorTemp);
  float TransfmV = SenzorTemperatura/1023.0 * 5.0 * 1000;
  TransfCels = (int((TransfmV - 500) / 10)/10); 
}

void CitireUmiditate()
{
  int SenzorUmiditate = analogRead(PinSenzorUmiditate);
  VarUmid = (SenzorUmiditate/10);

}

void SunetAlarma(int PinLED, int caz)
{
  for(int i = 0; i < 5; i++)
  {
    tone(PinBuzzer, 1000); 
    digitalWrite(PinLED, HIGH); 
    delay(1000);
    noTone(PinBuzzer); 
    digitalWrite(PinLED, LOW);  
    delay(500);
    if(caz == 1)
    {
       lcd.clear();
       lcd.setCursor(0,0);
       lcd.print("INTRUS");
       flag_intrus = 1;
       ledState_miscare = HIGH;
    }
    else
    {
       lcd.clear();
       lcd.setCursor(0,0);
       lcd.print("Emisie GAZ");
       flag_emisie_gaz = 1;
       ledState_gaz = HIGH;
    }
  }
  digitalWrite(PinLED, HIGH);
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("SMART HOME");
}
void AlarmaCutremur()
{
  for(int i = 0 ; i < 5 ;i++)
  {
    tone(PinBuzzer, 1000); 
    delay(1000);
    noTone(PinBuzzer);
    delay(500);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(" Cutremur ");
  }
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("SMART HOME");
}
void StopAlarma()
{
   noTone(PinBuzzer); 
}
void LuminaCamera1()
{
  int buttonState = digitalRead(BUTTON_PIN_1);
  if (buttonState != lastButtonState_1) {
    lastButtonState_1 = buttonState;
    if (buttonState == LOW) {
      ledState_1 = (ledState_1 == HIGH) ? LOW: HIGH;
      digitalWrite(LED_PIN_1, ledState_1);
    }
  }
}
void LuminaCamera2()
{
  int buttonState = digitalRead(BUTTON_PIN_2);
  if (buttonState != lastButtonState_2) {
    lastButtonState_2 = buttonState;
    if (buttonState == LOW) {
      ledState_2 = (ledState_2 == HIGH) ? LOW: HIGH;
      digitalWrite(LED_PIN_2, ledState_2);
    }
  }
}
void LuminaCamera3()
{
  int buttonState = digitalRead(BUTTON_PIN_3);
  if (buttonState != lastButtonState_3) {
    lastButtonState_3 = buttonState;
    if (buttonState == LOW) {
      ledState_3 = (ledState_3 == HIGH) ? LOW: HIGH;
      digitalWrite(LED_PIN_3, ledState_3);
    }
  }
}
