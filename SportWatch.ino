#include <Wire.h> 
#include <Adafruit_Sensor.h>    // biblioteca de functii unified pentru dispozitive Adafruit
#include <Adafruit_LSM303_U.h>  // biblioteca de functii pentru accelerometru; accelerometrul vine preprogramat cu 2 adrese I2C
                                // 0x19 pentru functia de accelerometru si 0x1E pentru functia de magnetometru. Aceste adrese
                                // sunt predefinite in biblioteca
#include <LiquidCrystal_I2C.h>  // biblioteca de functii pentru afisarea pe display                              

/* Atribuim un ID unic accelerometrului */
Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(54321);

/* Se defineste un obiect "lcd" pentru adresa I2C a displayului (0x27), si se specifica numarul de caractere pe care  
displayul il poate afisa pe pe un rand (16), respectiv numarul de randuri (2) */
LiquidCrystal_I2C lcd(0x27, 16, 2);

//variabile pentru prelucrarea componentelor acceleratiei masurate de accelerometru
float accX = 0; //acceleratia raportata de senzor pentru axa X
float accY = 0; //acceleratia raportata de senzor pentru axa Y
float accZ = 0; //acceleratia raportata de senzor pentru axa Z
float accRezultanta = 0; // variabila in care se va memora valoarea calculata a rezultantei acceleratiilor

int NumarPasi = 0; // variabila pentru a memora numarul de pasi
float PragDetectiePasi = 11.53; //pragul de detectie pentru a incrementa numaratorul de pasi- determinat in urma analizei 
                                //inregistrarilor. Acest prag depinde de la persoana la persoana, si este determinat pentru un anumit tip de mers.
bool PasActiv = false; // atunci cand se face un pas,valoarea acceleratiei va avea o serie de valori deasupra valorii prag
                       //aceasta variabila va primi valoarea "true" atunci cand se observa prima valoare a acceleratiei care se afla deasupra 
                       //pragului, si va primi valoarea "false" cand se va observa prima valoare sub prag

//variabile utilizate la implementarea unei alternative pentru functia delay()
unsigned long TimpN1 = 0; 
unsigned long TimpN2 = 0; 
const unsigned long secunda = 1000;

//declararea pinilor pe care se conecteaza butoanele
int butonS1 = 47;   
int butonS2 = 49;
int butonS3 = 51;
int butonS4 = 53;

bool arataCronometru = false; //activeaza functia de cronometru
bool cronometruPornit = false; //asigura functionarea in fundal a cronometrului
bool cronometruPauza = false;
bool cronometruStop = false;
bool meniuCronometruPornit = false; // asigura contextualitatea meniului Cronometru, in functie daca cronometrul este pornit sau nu
unsigned long cronometruInit = 0; //memoreaza semnatura temporala a momentului cand este pornit cronometrul
unsigned long valCronometru = 0; //determina timpul scurs de la pornirea cronometrului
unsigned long momentPauza = 0; //determina momentul cand cronometrul a fost pus pe pauza

int milisecunde = 0;
int secunde = 0;
int minute = 0;
int ore = 0;

void setup(void) {
  
  pinMode(butonS1, INPUT_PULLUP); 
  pinMode(butonS2, INPUT_PULLUP); 
  pinMode(butonS3, INPUT_PULLUP); 
  pinMode(butonS4, INPUT_PULLUP);
  
  Serial.begin(38400); //pentru debugging
  
  lcd.init(); 
  lcd.backlight();
  
  //mesaj afisat initial, la pornirea dispozitivului
  lcd.print("SportWatch");
  delay(3000); 
  
  //Rutina de initializare a accelerometrului
  Serial.println("Test accelerometru"); Serial.println("");
  
  //initializarea senzorului
  if(!accel.begin())
  {
    //daca senzorul nu a fost detectat, se transmite un mesaj 
    Serial.println("LSM303 nu a fost detectat ... ");
    while(1);
  }
}

void numarPasi() { 
    /* Extragem un nou event de la senzor*/
    sensors_event_t event;
    accel.getEvent(&event);
  
    /* variabilele sunt instantiate cu valorile masurate de senzor (masurate in m/s^2)*/
    accX = event.acceleration.x;
    accY = event.acceleration.y;
    accZ = event.acceleration.z;
  
    /* se calculeaza acceleratia rezultanta si se transmite catre calculator*/
    accRezultanta = sqrt (accX*accX+accY*accY+accZ*accZ);
    //Serial.println(accRezultanta); 
  
    //atunci cand acceleratia rezultanta este mai mare decat valoarea prag si variabila PasActiv este falsa
    if((accRezultanta > PragDetectiePasi) && (PasActiv == false)){ 
      NumarPasi++; //se incrementeaza cu 1 numarul de pasi
      PasActiv = true; //si se marcheaza ca s-a numarat acest pas
    }
   
    //atunci cand acceleratia rezultanta devine mai mica decat valoarea prag si variabila PasActiv este adevarata
    if((accRezultanta < PragDetectiePasi) && (PasActiv == true)){
      PasActiv = false; //se marcheaza finalul procesului de numarare a pasului
    }  
}

void afiseazaPedometru(){
    while (digitalRead(butonS4) != LOW){ //intoarcerea in meniul principal se face apasand butonul S4 
      numarPasi();
    
      //numarul de pasi se va actualiza pe display o data la o secunda
      TimpN2 = millis(); //se citeste timpul curent
      if (TimpN2 - TimpN1 >= secunda){   //daca diferenta dintre timpul curent si timpul salvat anterior este mai mare de o secunda
        TimpN1 = TimpN2;                 //timpul curent devine timpul anterior
        //se afiseaza numarul de pasi
        lcd.clear();
        lcd.print("Nr. de pasi: "); 
        lcd.setCursor(0,1);
        lcd.print(NumarPasi); 
      }

      //atunci cand se doreste resetarea pedometrului, se apasa butonul S3
      if (digitalRead(butonS3) == LOW) 
        NumarPasi = 0;
   }
}

void afiseazaCronometru(){
  arataCronometru = true;
  unsigned long activeMenu = millis(); //meniul afiseazaCronometru se acceseaza prin apasarea butonului S2. Dar acelasi buton activeaza pauza.
                                       //variabila pentru a evita intrarea automata in pauza la revenirea in meniu (butonul S2 va fi ignorat un interval de timp)
  
  //intoarcerea in meniul principal se face apasand butonul S4 
  while(arataCronometru == true){ 
    numarPasi();
    
    if(meniuCronometruPornit == false){//daca cronometrul nu este pornit in fundal, se afiseaza meniul principal
        TimpN2 = millis(); //emularea functiei delay(), fara a suspenda activitatea procesorului
        if (TimpN2 - TimpN1 >= secunda){
          TimpN1 = TimpN2;
          lcd.clear();
          lcd.print("1.Start  2.Pauza");
          lcd.setCursor(0,1);
          lcd.print("3.Stop 4.Inapoi"); 
        }
    }
    else {
          if (cronometruPauza == false && cronometruStop == false){
              TimpN2 = millis(); //emularea functiei delay(), fara a suspenda activitatea procesorului
              if (TimpN2 - TimpN1 >= secunda/10){
                TimpN1 = TimpN2;
                valCronometru = millis() - cronometruInit;
                milisecunde = valCronometru%1000;
                secunde = (valCronometru/1000)%60;
                minute = (valCronometru/60000)%60;
                ore = valCronometru/3600000;
                lcd.clear();
                lcd.print(ore);lcd.print(":");lcd.print(minute);lcd.print(":");lcd.print(secunde);lcd.print(".");lcd.print(milisecunde);
              }
          }
          else{
              TimpN2 = millis(); //emularea functiei delay(), fara a suspenda activitatea procesorului
              if (TimpN2 - TimpN1 >= secunda){
                TimpN1 = TimpN2;
                lcd.clear();
                lcd.print(ore);lcd.print(":");lcd.print(minute);lcd.print(":");lcd.print(secunde);lcd.print(".");lcd.print(milisecunde);
              }
          }
      }
      
      //daca se apasa butonul S1 si cronometrul este oprit, se porneste cronometrul
      if(digitalRead(butonS1) == LOW && cronometruPornit == false && arataCronometru == true){ 
        cronometruPornit = true;
        meniuCronometruPornit = true;
        cronometruStop = false;
        cronometruInit = millis();
      }
      
      //daca se apasa butonul S1 si cronometrul este pe pauza, se reporneste cronometrul
      if(digitalRead(butonS1) == LOW && cronometruPornit == true && cronometruPauza == true && arataCronometru == true){  
        cronometruPauza = false;
        unsigned long durataPauza = millis() - momentPauza; //se determina durata pauzei
        cronometruInit = cronometruInit + durataPauza; //momentul de start a cronomentrului se ajusteaza cu durata pauzei
      }
  
      //daca cronometrul este pornit si se apasa pauza
      if(digitalRead(butonS2) == LOW && cronometruPornit == true && cronometruPauza == false && arataCronometru == true && (millis() - activeMenu > 500)) {
        cronometruPauza = true;
        momentPauza = millis(); //se memoreaza momentul cand s-a pus pauza
      }
      
      if(digitalRead(butonS3) == LOW && cronometruPornit == true && arataCronometru == true){
        cronometruPauza = false;
        cronometruPornit = false;
        cronometruStop = true;
      }
  
      if(digitalRead(butonS4) == LOW){
        arataCronometru = false;
        if(cronometruStop == true){
            cronometruStop = false;
            meniuCronometruPornit = false;
        }
      }
  }
}
  
void loop(void) {
  //------------------------------meniul principal afisat pe ecranul LCD --------  
  TimpN2 = millis(); //emularea functiei delay(), fara a suspenda activitatea procesorului
  if (TimpN2 - TimpN1 >= secunda){
    TimpN1 = TimpN2;
    lcd.clear();
    lcd.print("1. Pedometru "); 
    lcd.setCursor(0,1);
    lcd.print("2. Cronometru"); 
  }

  numarPasi();

  if (digitalRead(butonS1) == LOW){ // Pedometrul se afiseaza apasand butonul S1
    afiseazaPedometru();
  }
  
  if (digitalRead(butonS2) == LOW){ // Cronometrul se afiseaza apasand butonul S2
    afiseazaCronometru();
  }
}
