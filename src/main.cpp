//--------------------------------------------------
//              Réveil Avec un Arduino
//          Fait en 2017 et mis à jours en 2021
//                      V1.4.3 By Xmow
//                   GammaTroniques.fr
//--------------------------------------------------

#include <Arduino.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal.h>
#include <RTClib.h>
#include <pins.h>

SoftwareSerial HC06(HC06_TX, HC06_RX);                            //On définit le module BlueTooth
LiquidCrystal lcd(LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7); //On définit l'écran
RTC_DS1307 RTC;                                                   //On définit le module RTC

//Quelques variables:
int alarmH = 8;
int alarmM = 30;

bool screenON = true;
bool alarm = false;

int nowH;
int nowM;

String msg;

String getArgs(String data, char separator, int index)
{
  //Function for separate String with special characters
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++)
  {
    if (data.charAt(i) == separator || i == maxIndex)
    {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }

  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void setup()
{
  pinMode(START_BUTTON, INPUT);   //Le bouton power est une entrée
  pinMode(ALARM_BUTTON, INPUT);   //Le bouton réveil est une entrée
  pinMode(RELAY_PIN, OUTPUT);     //Le relay est une sortie
  pinMode(LCD_BACKLIGHT, OUTPUT); //L'éclairage de l'écran est une sortie

  lcd.begin(LCD_COLUMN, LCD_ROW); //Démarrage de l'écran
  Wire.begin();
  RTC.begin(); //Démarrage du module RTC
  Serial.begin(9600);
  HC06.begin(9600); //Démarrage du module HC06 (Bluetooth)
}

void printHello() //Affiche Bonjour sur l'écran avec une animation
{
  lcd.clear();
  digitalWrite(RELAY_PIN, HIGH);
  digitalWrite(LCD_BACKLIGHT, HIGH);
  lcd.setCursor(4, 0);
  lcd.print("B");
  delay(200);
  lcd.setCursor(5, 0);
  lcd.print("o");
  delay(200);
  lcd.setCursor(6, 0);
  lcd.print("n");
  delay(200);
  lcd.setCursor(7, 0);
  lcd.print("j");
  delay(200);
  lcd.setCursor(8, 0);
  lcd.print("o");
  delay(200);
  lcd.setCursor(9, 0);
  lcd.print("u");
  delay(200);
  lcd.setCursor(10, 0);
  lcd.print("r");

  lcd.setCursor(4, 1);
  lcd.print("Il est");
}

void printAlarmON()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Alarme activee a:");
  lcd.setCursor(0, 1);
  if (alarmH < 10)
  {
    lcd.print("0");
  }
  lcd.print(alarmH);
  lcd.setCursor(2, 1);
  lcd.print(":");
  lcd.setCursor(3, 1);
  if (alarmM < 10)
  {
    lcd.print("0");
  }
  lcd.print(alarmM);
  delay(3000);
  lcd.clear();
}

void sendStateToApp()
{
  //on prépare la réponse pour l'application
  String State = "Light:";
  State += screenON;
  State += ",Timer:";
  State += alarm;

  Serial.println(State);
  HC06.println(State); //On l'envoie à l'app
}

void loop()
{
  DateTime now = RTC.now(); //récupère la date et l'heure depuis le module RTC
  nowH = now.hour();        //Récupère l'heure
  nowM = now.minute();      //Récupère minute

  if (nowH == alarmH && nowM == alarmM && alarm == true && now.second() == 0) //Si c'est l'heure du réveil et que le réveil est activé, ça alume l'écran et dit bonjour
  {
    screenON = true;
    printHello();
    delay(2000);
    lcd.clear();
  }

  if (digitalRead(START_BUTTON) == 1 && screenON == false) //Si on appuit sur le bouton power et que l'écran est éteint, l'écran s'allume
  {
    while (digitalRead(START_BUTTON) == 1)
    {
      //Pour évité de déclancher plusieurs appuis, on stop tous le code si le bouton reste appuyer
    }
    printHello();
    delay(2000);
    lcd.clear();
    screenON = true; //variable de l'état de l'écran
    sendStateToApp();
  }

  if (digitalRead(START_BUTTON) == 1 && screenON == true) //Si on appuit sur le bouton power et que l'écran est allumé, l'écran s'éteint
  {
    while (digitalRead(START_BUTTON) == 1)
    {
      //Pour évité de déclancher plusieurs appuis, on stop tous le code si le bouton reste appuyer
    }
    screenON = false;         //variable de l'état de l'écran
    DateTime now = RTC.now(); //récupère la date et l'heure depuis le module RTC

    if (now.hour() >= 20) //Affiche Bonne nuit si il est plus tard que 20h
    {
      lcd.clear();
      lcd.setCursor(3, 0);
      lcd.print("Bonne nuit");
      lcd.setCursor(7, 1);
      lcd.print("zZ");
      delay(2000);
      lcd.clear();
    }
    else //sinon on dit au revoir
    {
      lcd.clear();
      lcd.setCursor(4, 0);
      lcd.print("Au Revoir");
      delay(2000);
      lcd.clear();
    }

    digitalWrite(RELAY_PIN, LOW);     //on éteint le relay
    digitalWrite(LCD_BACKLIGHT, LOW); //on éteint l'écairage de l'écran
    lcd.clear();                      //on efface l'écran
    sendStateToApp();
  }

  if (digitalRead(ALARM_BUTTON) == 1 && alarm == false) //Si on appuit sur le bouton d'activation du réveil et que le réveil est désactivé, le réveil s'active
  {
    while (digitalRead(ALARM_BUTTON) == 1)
    {
      //Pour évité de déclancher plusieurs appuis, on stop tous le code si le bouton reste appuyer
    }
    alarm = true; //active le réveil

    //affichage sur l'écran:
    printAlarmON();
    sendStateToApp();
  }

  if (digitalRead(ALARM_BUTTON) == 1 && alarm == true) //Si on appuit sur le bouton d'activation du réveil et que le réveil est activé, le réveil se désactive
  {
    while (digitalRead(ALARM_BUTTON) == 1)
    {
      //Pour évité de déclancher plusieurs appuis, on stop tous le code si le bouton reste appuyer
    }
    alarm = false; //désactivation du réveil
    sendStateToApp();
  }

  if (screenON == true) //Si l'écran est allumé:
  {

    //Affichage de l'état du réveil
    if (alarm == true)
    {
      lcd.setCursor(14, 0);
      lcd.print("ON");
    }
    else
    {
      lcd.setCursor(13, 0);
      lcd.print("OFF");
    }

    //Affichage de l'heure
    lcd.setCursor(0, 0);
    lcd.print(now.hour());
    lcd.print(":");
    if (now.minute() < 10)
    {
      lcd.print("0");
    }
    lcd.print(now.minute());
    lcd.print(":");
    if (now.second() < 10)
    {
      lcd.print("0");
    }
    lcd.print(now.second());

    //Affichage de la date
    lcd.setCursor(0, 1);
    lcd.print(now.day());
    lcd.print(" ");

    //Affichage du mois
    switch (now.month())
    {
    case 1:
      lcd.print("janvier");
      break;
    case 2:
      lcd.print("fevrier");
      break;
    case 3:
      lcd.print("mars");
      break;
    case 4:
      lcd.print("avril");
      break;
    case 5:
      lcd.print("mai");
      break;
    case 6:
      lcd.print("juin");
      break;
    case 7:
      lcd.print("juillet");
      break;
    case 8:
      lcd.print("aout");
      break;
    case 9:
      lcd.print("septembre");
      break;
    case 10:
      lcd.print("octobre");
      break;
    case 11:
      lcd.print("novembre");
      break;
    case 12:
      lcd.print("decembre");
      break;
    }
    //Affichage de l'année
    lcd.print(" ");
    lcd.print(now.year());
    delay(50);
  }

  //Si on reçoit un message de la part du bluetooth:
  while (HC06.available())
  {
    delay(3);
    char c = HC06.read();
    msg += c;
  }
  //si le messahe n'est pas vide:
  if (msg.length() > 0)
  {
    msg.trim();          //on enlève tous les espace,entré ... du message pour facilité sont traitement
    Serial.println(msg); //on l'affiche dans le moniteur serie (optionel)

    //Si on reçoit Light:1
    if (msg == "Light:1")
    {
      if (screenON) //Si la lumière est déja allumé, on ne fait rien
      {
      }
      else
      {
        //on allume l'écran et le relay
        printHello();
        delay(2000);
        lcd.clear();
        screenON = true; //variable de l'état de l'écran
      }
    }
    //Si on reçoit Light:0
    else if (msg == "Light:0")
    {
      if (screenON == 0) //Si la lumière est déja éteinte, on ne fait rien
      {
      }
      else
      {
        //on éteint tous
        screenON = false;         //variable de l'état de l'écran
        DateTime now = RTC.now(); //récupère la date et l'heure depuis le module RTC

        if (now.hour() >= 20) //Affiche Bonne nuit si il est plus tard que 20h
        {
          lcd.clear();
          lcd.setCursor(3, 0);
          lcd.print("Bonne nuit");
          lcd.setCursor(7, 1);
          lcd.print("zZ");
          delay(2000);
          lcd.clear();
        }
        else //sinon on dit au revoir
        {
          lcd.clear();
          lcd.setCursor(4, 0);
          lcd.print("Au Revoir");
          delay(2000);
          lcd.clear();
        }

        digitalWrite(RELAY_PIN, LOW);     //on éteint le relay
        digitalWrite(LCD_BACKLIGHT, LOW); //on éteint l'écairage de l'écran
        lcd.clear();
      }
    }

    //Si on reçoit Timer:1
    if (msg == "Timer:1")
    {
      if (alarm == 1) //Si le réveil est déja allumé, on ne fait rien
      {
      }
      else
      {
        //on active le réveil et on affiche sur l'écran
        alarm = true;
        printAlarmON();
      }
    }

    //Si on reçoit Timer:0
    else if (msg == "Timer:0")
    {
      if (alarm == 0) //Si le réveil est déja éteint, on ne fait rien
      {
      }
      else
      {
        //on désactive le réveil
        alarm = false;
      }
    }

    //Si on reçoit un message qui commence par TimerH:   C'est que on va régler l'heure du réveil
    else if (msg.startsWith("TimerH:"))
    {
      //Le message resemble à ça: "TimerH:10,TimerM:58"
      //On sépare le message en 2 parties avec la virgule
      String msgH = getArgs(msg, ',', 0); //donc ici on a "TimerH:10"
      String msgM = getArgs(msg, ',', 1); //et ici on a "TimerM:58"

      msgH.remove(0, 7);     //on supprime les 7 premier caractères donc il nous reste : 10
      alarmH = msgH.toInt(); // et on le converti en nombre (entier), et on l'enregistre dans la variable

      msgM.remove(0, 7);     //on supprime les 7 premier caractères donc il nous reste : 58
      alarmM = msgM.toInt(); //et on le converti en nombre (entier), et on l'enregistre dans la variable
    }
    //Si on reçoit un message qui commence par NowH:   C'est que on va régler l'heure actuelle dans le module RTC
    else if (msg.startsWith("NowH:"))
    {
      //Le message resemble à ça: "NowH:11,NowM:1"
      //On sépare le message en 2 parties avec la virgule
      String msgH = getArgs(msg, ',', 0); //donc ici on a "NowH:11"
      String msgM = getArgs(msg, ',', 1); //et ici on a "NowM:1"

      msgH.remove(0, 5);   //on supprime les 5 premier caractères donc il nous reste : 11
      nowH = msgH.toInt(); //et on le converti en nombre (entier), et on l'enregistre dans la variable

      msgM.remove(0, 5);   //on supprime les 5 premier caractères donc il nous reste : 1
      nowM = msgM.toInt(); //et on le converti en nombre (entier), et on l'enregistre dans la variable

      RTC.adjust(DateTime(now.year(), now.month(), now.day(), nowH, nowM, now.second())); //et on l'enregistre dans le module RTC pour garder l'heure même si il ny a plus de courrent dans l'arduino.
    }

    //Si on reçoit un message qui commence par NowD:   C'est que on va régler la date actuelle dans le module RTC
    else if (msg.startsWith("NowD:"))
    {
      //Le message resemble à ça: "NowD:25,Nowm:2,NowY:2021"
      //On sépare le message en 3 parties avec la virgule
      String msgD = getArgs(msg, ',', 0);  //donc ici on a "NowD:25"
      String msgMo = getArgs(msg, ',', 1); // ici on a "Nowm:2"
      String msgY = getArgs(msg, ',', 2);  //et ici on a "NowY:2021"

      msgD.remove(0, 5);       //on supprime les 5 premier caractères donc il nous reste : 25
      int nowD = msgD.toInt(); //et on le converti en nombre (entier), et on l'enregistre dans la variable

      msgMo.remove(0, 5);        //on supprime les 5 premier caractères donc il nous reste : 2
      int nowMo = msgMo.toInt(); //et on le converti en nombre (entier), et on l'enregistre dans la variable

      msgY.remove(0, 5);       //on supprime les 5 premier caractères donc il nous reste : 2021
      int nowY = msgY.toInt(); //et on le converti en nombre (entier), et on l'enregistre dans la variable

      RTC.adjust(DateTime(nowY, nowMo, nowD, now.hour(), now.minute(), now.second())); //et on l'enregistre dans le module RTC pour garder l'heure même si il ny a plus de courrent dans l'arduino.
    }
    //Si on reçoit state?  c'est que l'application demande l'état de l'écran et du réveil
    else if (msg.startsWith("state?"))
    {
      sendStateToApp();
    }

    msg = ""; //on efface le message car on en a plus besoin
  }
}