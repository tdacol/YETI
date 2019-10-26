//Arduino mega 2560
//Pin: 12 digitali, 7 PWM, 3 SPI
//Non usare pin 50,51,52,53 (Comunicazione seriale)

//Modificare colori fili
/*Tempo: (tTot=500 (alternanza servo1 in joy), le configurazioni durano 3s (Creare una variabile di durata delle configurazioni?))
  1.Il tempo a disposizione per giocare prima che YETI assuma la sua configurazione sadness è modificabile in YETI dispositivo
  2.Assegnare Beg prima del ciclo, End all'interno del ciclo (t... nel loop, t...Fun nelle funzioni, tBegSonar prima di un ciclo contente ruotaSonar)
  3.Non sono strettamente necessarie sia la variabile t..., sia t...Fun che t...Sonar ma vengono mantenute tutte e tre per chiarezza di lettura del codice)
*/
//Comunicazione: (Sostituire la stringa con un singolo bit (0 o 1)?)
/*
 * 
 Driver (backward lento, left e right semi lento)
 1.Slittamento:  (curve)
   1.Aumento velocità graduale?
   2.Nel passaggio da uno stato all'altro chiamre stay?
 2.Chiamare solo left se ostacolo=1? Chiamare prima backward?
 3.Passare un parametro di velocità alle funzioni?
 */

//Slittamento ruote?
//joy: servo e driver non funzionano contemporaneamente, potenza batteria? stay() se ostacolo=1?

//Pin mode radio, sonar? Input/Output default o libreria?

//Inclusione libreria servo
#include<Servo.h>

//Inclusione libreria radio
#include<SPI.h>
#include<nRF24L01.h>
#include<RF24.h>

//Inclusione libreria sonar
#include <NewPing.h>

//Inclusione librerie generali
#include<string.h>

//Prototipo funzione sonar (acquisizione alternata dai diversi sonar e eliminazione del fenomeno del crosstalk)
void ruotaSonar();

//Prototipi funzioni driver
void stay();
void forward();
void backward();
void left();
void right();

//Prototipi funzioni configurazioni
void joy();
void sadness();

//Pin led gioia
const int ledRed=2; //PWM //Rosso
const int ledGreen=3; //PWM //Verde

//Pin led tristezza
const int ledBlue=4; //PWM //Blu

//Pin servo
const int pinServo1=6; //PWM //Giallo
const int pinServo2=7; //PWM //Arancione

//Pin radio
const int CE=22; //Giallo 
const int CSN=23; //Arancione
//MISO Bianco, MOSI Verde, SCK Blu

//Pin Sonar   
const int Trig1=26; //Giallo
const int Echo1=27; //Arancione
const int Trig2=28; //Giallo
const int Echo2=29; //Arancione
const int Trig3=30; //Giallo
const int Echo3=31; //Arancione

//Pin Driver
//Motore sinistro
const int I1=34;  //Giallo  
const int I2=35;  //Arancione  
const int EA=9;  //PWM  //Viola 
//Motore destro
const int I3=36;  //Verde  
const int I4=37;  //Blu   
const int EB=10;  //PWM  //Grigio

//Variabili motore
int angle;  //Necessaria per il movimento alternato del servo nella configurazione joy

//Variabili radio
const byte add[]="00001";  //Indirizzo di ricezione del canale di comunicazione seriale
char text[10];  //Dimensione massima della stringa passata per comunicazione seriale

//Variabili sonar
const int dMax=300;  //Massima distanza rilevata dal sonar (in cm) (dMax<400) 
float d;  //Distanza rilevata dal sonar (se non viene rilevato niente tra 0 e dMax il sonar ritorna 0)
int sonar;  //Necessaria per l'acquisizione alternata dai diversi sonar
int controllo;  //Necessaria per evitare il fenomeno del crosstalk
int ostacolo;  //Necessaria per evitare il fenomeno del crosstalk
float tBegSonar,tEndSonar;  //Necessarie a evitare il fenomeno del crosstalk (importante: assegnare millis() a tBeg prima del ciclo contenente ruotaSonar)

//Variabili generali 
const int tTot=500; //E' il minore intervallo temporale presente all'interno dello sketch (senza contare quello interno a ruotaSonar), è riferito all'alternanza della rotazione di servo1 all'interno di joy
float tBeg,tEnd,tBegFun,tEndFun;  //Tempi
int i,iFun;  //Contatori: Il primo serve a rendere alternato il movimento del "driver", il secondo quello di servo1

//Creazione oggetto servo
Servo servo1,servo2; 

//Creazione oggetto radio
RF24 radio(CE,CSN);

//Creazione oggetto sonar
NewPing sonar1(Trig1,Echo1,dMax);
NewPing sonar2(Trig2,Echo2,dMax);
NewPing sonar3(Trig3,Echo3,dMax);

void setup() 
{
  //Pin mode led
  pinMode(ledRed,OUTPUT);
  pinMode(ledGreen,OUTPUT);
  pinMode(ledBlue,OUTPUT);
  
  //Assegnamento pin servo
  servo1.attach(pinServo1); 
  servo2.attach(pinServo2); 

  //Pin mode radio
  
  //Pin mode sonar

  //Pin mode driver
  pinMode(EA,OUTPUT);
  pinMode(I1,OUTPUT);
  pinMode(I2,OUTPUT);
  pinMode(EB,OUTPUT);
  pinMode(I3,OUTPUT);
  pinMode(I4,OUTPUT);

  //Setup led 
  analogWrite(ledRed,0);
  analogWrite(ledGreen,0);
  analogWrite(ledBlue,0);

  //Setup servo
  servo1.write(90); //Il setup dell'angolo di servo1 deve essere di 90, il motore deve potersi muovere simmetricamente in entrambe le direzioni (per permettere il movimento alternato)
  servo2.write(90); //Non è possibile modificare la velocità di rotazione del servo, per rendere più lento il movimento avvicinare il cappio della fune collegata al motore all'asse di rotazione del motore
  
  //Setup radio
  radio.begin();
  radio.openReadingPipe(0,add); 
  radio.startListening();
  text[10]={0};

  //Setup sonar
  d=0;
  sonar=1;  //Importante: Assegnamento necessario al corretto funzionamento di ruotaSonar
  controllo=0;
  ostacolo=0;
  tBegSonar=0;
  tEndSonar=0;

  //Setup driver
  stay();
  
  //Setup generale
  i=0;
  iFun=0;
  tBeg=0;
  tEnd=0;
  tBegFun=0;
  tEndFun=0;
}

void loop() 
{
  //Led (colore giallo)
  analogWrite(ledRed,155);
  analogWrite(ledGreen,255);
  analogWrite(ledBlue,0);
  
  //Servo (nessun movimento)
  servo1.write(90); 
  servo2.write(90); 

  i+=1;  

  //Movimento, sonar e radio
  if(i%2)  //Pattern untilizzato per alternare due stati
  {
    //Movimento
    forward();
    
    tBeg=millis();  
    tBegSonar=millis();
    do
    {
      tEnd=millis();
      
      //Sonar
      ruotaSonar();  
      if(ostacolo)  
      {
        stay();
        i=1; //Assegnamento necessario per entrare all'interno dell'else (left) alla successiva iterazione del loop
        break;
      }

      //radio (analisi della stringa ricevuta per comunicazione seriale)
      if(radio.available())
      {   
        radio.read(&text,sizeof(text));  
        if(!strcmp(text,"Gioia")) 
        {
          joy();
          break;
        }  
        else if(!strcmp(text,"Tristezza"))
        {
          sadness();
          break;
        }
      }
      
    }
    while(tEnd-tBeg<6*tTot);
  } 
  else
  {
    //Movimento
    left();
    
    tBeg=millis();
    do
    {
      tEnd=millis();
      
      //Radio
      if(radio.available())
      {   
        radio.read(&text,sizeof(text));  
        if(!strcmp(text,"Gioia")) 
        {
          joy();
          break;
        }  
        else if(!strcmp(text,"Tristezza"))
        {
          sadness();
          break;
        }
      }
      
    }
    while(tEnd-tBeg<6*tTot);
  } 
}

void ruotaSonar()
{
  tEndSonar=millis(); 
  controllo=0;
  ostacolo=0;
  if(tEndSonar-tBegSonar>60)
  {
    tBegSonar=tEndSonar;
    if(sonar==1&&controllo==0)
    {
      d=sonar1.ping_cm();
      if(d<50&&d>3)
      {
        ostacolo=1;
      }
      controllo=1;
      sonar=2;
    }
    if(sonar==2&&controllo==0)
    {
      d=sonar2.ping_cm();
      if(d<50&&d>3)
      {
        ostacolo=1;
      }
      controllo=1;
      sonar=3;
    }
    if(sonar==3&&controllo==0)
    {
      d=sonar3.ping_cm();
      if(d<50&&d>3)
      {
        ostacolo=1;
      }
      controllo=1;
      sonar=1;
    }
  }
}

void stay()
{
  digitalWrite(EA,0);
  digitalWrite(EB,0);
}

void forward()  
{
  digitalWrite(EA,1);
  digitalWrite(EB,1);
  digitalWrite(I1,0);
  digitalWrite(I2,1);
  digitalWrite(I3,1);
  digitalWrite(I4,0);
}

void backward()
{
  analogWrite(EA,125);
  analogWrite(EB,125);
  digitalWrite(I1,1);
  digitalWrite(I2,0);
  digitalWrite(I3,0);
  digitalWrite(I4,1);
}

void left()
{
  analogWrite(EA,200);
  analogWrite(EB,200);
  digitalWrite(I1,1);
  digitalWrite(I2,0);
  digitalWrite(I3,1);
  digitalWrite(I4,0);
}

void right()
{
  analogWrite(EA,200);
  analogWrite(EB,200);
  digitalWrite(I1,0);
  digitalWrite(I2,1);
  digitalWrite(I3,0);
  digitalWrite(I4,1);
}

void joy()
{
    //Led (giallo)
    analogWrite(ledRed,155);
    analogWrite(ledGreen,255);
    analogWrite(ledBlue,0);

    //Driver (avanti)
    forward();
    
    for(iFun=0;iFun<6;iFun++)
    {
      //Servo (servo1 alternato)
      if(iFun%2)
        angle=30;
      else
        angle=120;
      servo1.write(angle);
      
      tBegFun=millis();
      tBegSonar=millis();  
      do
      { 
        tEndFun=millis();
        
        //Sonar
        ruotaSonar();
        if(ostacolo)
        {
          stay();
        }
        
      }
      while(tEndFun-tBegFun<tTot);     
    } 
}

void sadness()
{
  //Led (blu)
  analogWrite(ledRed,0);
  analogWrite(ledGreen,0);
  analogWrite(ledBlue,255);
  
  //Servo (abbassamento struttura)
  servo2.write(0);
  
  //Driver (indietro)
  backward(); 
  delay(6*tTot);
}


