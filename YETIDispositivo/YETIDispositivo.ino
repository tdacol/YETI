//Arduino UNO 
//Pin: 6 digitali, 4 PWM, 3 SPI 
//Non usare pin 10?,11,12,13 (Utilizzo pin anlogici come digitali)

//Tempi: tTot=30s (tempo di gioco) (Portare a 60s, attenzione overflow); tSenOff (Tempo di non spegnimento sensore di colore) tLedOff=8s (tempo di spegnimento led) dopo aver inviato la stringa per comunicazione seriale (è stato aumentato (viceversa inserire un delay all'inizio del loop) per evitare l'acquisizione dal dispositvo quando è ancora appoggiato alla base precedente (oppure non chiamare sadness quando il dispositvo rimane o viene appoggiato sulla base del colore precedente?)
//Utilizzare randomseed()

//Inclusione librerie radio
#include<SPI.h>   
#include<nRF24L01.h>  
#include<RF24.h>

//Pin led RGB
const int ledRed=3;  //PWM  //Rosso
const int ledGreen=5;  //PWM  //Verde
const int ledBlu=6;  //PWM  //Blu

//Pin sensore
const int s0=2;  //Verde
const int s1=4;  //Blu
const int s2=7;  //Giallo
const int s3=8;  //Arancione
const int out=9;  //PWM  //Bianco

//Pin radio
const int CE=14;  //Pin analogico come pin digitale?  //Giallo
const int CSN=15;  //Arancione
//MISO Bianco, MOSI Verde, SCK Blu

//Creazione oggetto radio
RF24 radio(CE,CSN);

//Variabili sensore
int redSen=0;
int greenSen=0;
int bluSen=0;

//Variabili radio
const byte add[]="00001";

//Variabili generali
const int sec=1000;
const int tTot=30*sec;  
const int tSenOff=3*sec;
const int tLedOff=8*sec;
float tBeg,tEnd;
int i,iTem;  //iTem variabile temporanea necessaria a verificare che i sia diverso a ogni iterazione
const char joy[]="Gioia";
const char sadness[]="Tristezza";

void setup() 
{
  //Pin mode led RGB
  pinMode(ledRed,OUTPUT);
  pinMode(ledGreen,OUTPUT);
  pinMode(ledBlu,OUTPUT);

  //Pin mode sensore
  pinMode(s0,OUTPUT);
  pinMode(s1,OUTPUT);
  pinMode(s2,OUTPUT);
  pinMode(s3,OUTPUT);
  pinMode(out,INPUT);

  //Setup led RGB
  analogWrite(ledRed,0);   
  analogWrite(ledGreen,0);
  analogWrite(ledBlu,0);

  //Setup sensore
  digitalWrite(s0,1);
  digitalWrite(s1,1);

  //Setup radio
  radio.begin();  //Attivazione modem
  radio.setRetries(15,15);  //Argomento 1: 15*250us, indica ogni quanto tempo il modem cerca di inviare il dato, argomento 2: indica quante volte il modem cerca di inviare il dato 
  radio.openWritingPipe(add);
  radio.stopListening();

  //Setup generale
  i=0;
  iTem=0;
}

void loop() 
{
  
  //Accensione led (pattern RGB)
  if(i==0)
  {
    analogWrite(ledRed,255);   
    analogWrite(ledGreen,0);
    analogWrite(ledBlu,0);
  }
  else if(i==1)
  {
    analogWrite(ledRed,0);   
    analogWrite(ledGreen,255);
    analogWrite(ledBlu,0);
  }
  else if(i==2)
  {
    analogWrite(ledRed,0);   
    analogWrite(ledGreen,0);
    analogWrite(ledBlu,255);
  }

  delay(tSenOff);

  tBeg=millis();   //Assegnamento tempo di inzio
  tEnd=millis();   //Inizializzazine tempo di fine
  
  //Condizione di controllo tempo
  while(tEnd-tBeg<tTot)
  {
    tEnd=millis();  //Aggiornamento timeEnd
    
    //Acquisizione da sensore
    //Acquisizione rosso
    digitalWrite(s2,LOW);
    digitalWrite(s3,LOW);   
    redSen=pulseIn(out,HIGH);   
    //Acquisizione blu
    digitalWrite(s3,HIGH);   
    bluSen=pulseIn(out,HIGH);
    //Acquisizione verde
    digitalWrite(s2,HIGH);   
    greenSen=pulseIn(out,HIGH);
        
    //Verifica condizioni e attuazione di controllo task
    if(redSen<greenSen&&redSen<bluSen&&redSen<100)
    {
      if(i==0)
        radio.write(&joy,sizeof(joy));
      else
        radio.write(&sadness,sizeof(sadness));
      break;
    }
    else if(greenSen<redSen&&greenSen<bluSen&&greenSen<100)
    {
      if(i==1)
        radio.write(&joy,sizeof(joy));
      else
        radio.write(&sadness,sizeof(sadness));
      break;
    }
    else if(bluSen<redSen&&bluSen<greenSen&&bluSen<150)
    {
      if(i==2)
        radio.write(&joy,sizeof(joy));
      else
        radio.write(&sadness,sizeof(sadness));
      break;
    }
    
  }
  if(tEnd-tBeg>tTot)
    radio.write(&sadness,sizeof(sadness));

  //Spegnimento led
  analogWrite(ledRed,0);   
  analogWrite(ledGreen,0);
  analogWrite(ledBlu,0);
  delay(tLedOff);
  
  //Aggiornamento index
  do
    iTem=random(3);
  while(i==iTem);
  i=iTem;
  
}
