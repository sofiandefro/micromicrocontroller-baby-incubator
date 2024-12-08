/* arduino UNO rev3
maquette ISIFC 
ex d'utilisation des CAN
*/


#include "ISIFC.h"
#include <stdio.h>


int t=0;
int temperature_piece = 20;
int consigne = 250;
bool ON = true;
bool bouton_chauffage_max = false;
bool changement_temperature=false;


void setup() {

  // definition du mode de fonctionnement des broches

  pinMode(Thermo, INPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2,OUTPUT);
  pinMode(txLCD, OUTPUT);
  pinMode(Bouton1, INPUT_PULLUP);
  pinMode(Bouton2, INPUT_PULLUP);
  pinMode(SW1, INPUT_PULLUP);
  pinMode(SW2, INPUT_PULLUP);

  // configuration des vitesses (en bauds)

  LCD.begin(19200);
  Serial.begin(9600);
  
  /*
  configuration des boutons d'interruption 
  bouton qui permet de changer la temperature
  */

  attachInterrupt(digitalPinToInterrupt(Bouton1),Changement_consigne, FALLING);  

  /* 
  bouton ON/OFF, declenche la fonction arret_prog() 
  (1ere pression : allume_led_temperature = False -> True / 2eme pression : allume_led_temperature = True -> False) 
  */

  attachInterrupt(digitalPinToInterrupt(Bouton2), allume_led_temperature, FALLING); 
  
  /* 
  bouton pour forcer le chauffage Ã  fond, declenche la fonctoin allume_led_temperature() 
  (1ere pression : allume_led_temperature = False -> True / 2eme pression : allume_led_temperature = True -> False) 
  */
}



void Changement_consigne() 
{
  char message2[32];
  if(digitalRead(SW2))
  {
    sprintf(message2, "Tc=%d,%dC + 0.1C",(int) consigne/10, (int) consigne%10);
    consigne+=1;
  }
  else
  {
    consigne-=1;
    sprintf(message2, "Tc=%d,%dC - 0.1C",(int) consigne/10, (int) consigne%10);
  }
  send_char(0xA3) ; 
  send_char(1) ; // on efface le LCD

  send_string("bouton1",message2);
  delay(2000);

}


void allume_led_temperature() {

  bouton_chauffage_max =! bouton_chauffage_max;
  send_char(0xA3) ; 
  send_char(1) ; // on efface le LCD

  send_string("bouton2","bouton2");
  delay(2000);

}


long int conversion(int T) {

  long int calcul= (((long int)T*55*10)/1023) + temperature_piece*10;
  return(calcul);
}

// fonction d'affichage des strings sur le LCD

void send_string(char tableau_1[],char tableau_2[])
{
  char i;
  send_char(0xA3) ; 
  send_char(1) ; // on efface le LCD

  send_char(0xA1) ;
  send_char(0) ;
  send_char(0) ;

  for(i = 0; i <= 32; i++) 
  {
    if(i<16) {

      // on positionne le curseur
      send_char(0xA1);
      send_char(i);
      send_char(0);

      // on ecrie la lettre(i)
      send_char(0xA2);
      if(tableau_1[i]=='\0'){i=15;}
      else{send_char(tableau_1[i]);}
    }
    else {

      // on positionne le curseur
      send_char(0xA1);
      send_char(i%16);
      send_char(1);

      // on ecrie la lettre (i) (iÃ¨me lettre)
      send_char(0xA2);
      if ( tableau_2[i%16] == '\0' ) { 
        i=33; 
      }
      else {
        send_char(tableau_2[i-16]); 
      }
    }
    send_char(0); // sortir du mode affichage
  }
}


void loop() {
  if(digitalRead(SW1)) 
  {

    digitalWrite(LED2,0); // allume la led qui signifie que le systÃ¨me fonctionne



    //On calcul la moyenne des temperatures avec un rafraÃ®chissement de 1Hz

    int somme=0;

    for (int i=0;i<10;i+=1) {

      somme+=conversion(analogRead(Thermo)); 
    }

    long int temperature = somme/10;


      
    // calcul d'une intensitÃ© de chauffage proportionnelle Ã  la diffÃ©rence entre une tempÃ©rature de consigne et la tempÃ©rature mesurÃ©e 

    int rapport_cyclique = 0;

    if(bouton_chauffage_max==true) {

      rapport_cyclique=0;
    }
    else {

      rapport_cyclique = map(constrain(temperature,consigne-50,consigne),consigne-50,consigne,255,0);
    }

    analogWrite(LED1, rapport_cyclique);



    // affichage des resultat sur un graphique 

    Serial.print("Temperature:"); 
    Serial.println(temperature);  

    Serial.print("rapport_cyclique:");
    Serial.println(rapport_cyclique);


      
    // affichage des resultat sur l'ecran lcd

    send_char(0xA3) ; 
    send_char(1) ; // on efface le LCD

    char message1[32];
    char message2[32];

    sprintf(message1, "T=%d,%dC", (int) temperature/10, (int) temperature%10);
    sprintf(message2, "R=%d; Tc=%d,%dC", rapport_cyclique,(int) consigne/10, (int) consigne%10);

      
    send_string(message1, message2);



    // rajout d'un delay pour les 1Hz

    t = 1000-LCDdelay*10; // temporisation de 1Hz
    delay(t);
    
  }
  else {

    digitalWrite(LED2,1); // eteint la led qui signifie que le systeme ne fonctionne pas / plus
    send_char(0xA3) ; 
    send_char(1) ;
    send_string("programme arrete","");
  
    delay(1000);



  }
}
