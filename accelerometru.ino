/* 
 *  Program varianta cu accelerometru Adafruit LSM303 si comunicatie bluetooth pentru transmiterea wireless in calculator a valorilor 
 *  acceleratiei, in vederea analizei valorilor inregistrare si stabilirea pragului de detectie a unui pas
 */
#include <Wire.h> // biblioteca de functii pentru comunicatie I2C intre accelerometru si Arduino 
#include <Adafruit_Sensor.h>    // biblioteca de functii unified pentru dispozitive Adafruit
#include <Adafruit_LSM303_U.h>  // biblioteca de functii pentru accelerometru; accelerometrul vine preprogramat cu 2 adrese I2C
                                // 0x19 pentru functia de accelerometru si 0x1E pentru functia de magnetometru. Aceste adrese
                                // sunt predefinite in biblioteca
#include <SoftwareSerial.h> // biblioteca de functii pentru comunicatie Arduino - bluetooth


/* Atribuim un ID unic accelerometrului */
Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(54321);

// Definim un obiect de tip SoftwareSerial si specificam pinii Arduino cu rol RX | TX pentru comunicatia cu modulul bluetooth
SoftwareSerial ModulBT(0, 1); // RX | TX

//variabile pentru prelucrarea componentelor acceleratiei masurate de accelerometru
float accX; //acceleratia raportata de senzor pentru axa X
float accY; //acceleratia raportata de senzor pentru axa Y
float accZ; //acceleratia raportata de senzor pentru axa Z
float accRezultanta; // variabila in care se va memora valoarea calculata a rezultantei acceleratiilor

bool debug = false; // utilizam o variabila "debug" pentru a activa sau dezactiva liniile de cod utilizate in debugging

void DetaliiAccelerometru(void){
  sensor_t sensor;
  accel.getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.print  ("Senzor:       "); Serial.println(sensor.name);
  Serial.print  ("Versiune senzor:   "); Serial.println(sensor.version);
  Serial.print  ("ID senzor:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Valoare maxima:    "); Serial.print(sensor.max_value); Serial.println(" m/s^2");
  Serial.print  ("Valoare minima:    "); Serial.print(sensor.min_value); Serial.println(" m/s^2");
  Serial.print  ("Rezolutia senzorului:   "); Serial.print(sensor.resolution); Serial.println(" m/s^2");
  Serial.println("------------------------------------");
  Serial.println("");
  delay(500);
}

void setup(void) {
  Serial.begin(38400); //se defineste rata de modulatie (baudrate) pentru comunicatia seriala prin USB (pentru debugging)
  ModulBT.begin(38400); //se defineste rata de modulatie (baudrate) pentru comunicatia seriala prin bluetooth
   
  //Rutina de initializare a accelerometrului
  Serial.println("Test accelerometru"); Serial.println("");
  //initializarea senzorului
  if(!accel.begin())
  {
    //daca senzorul nu a fost detectat, se transmite un mesaj 
    Serial.println("LSM303 nu a fost detectat ... ");
    while(1);
  }

 if (debug) {
   DetaliiAccelerometru();
 }

  
}

void loop(void)
{
  /* Exragem un nou event de la senzor*/
  sensors_event_t event;
  accel.getEvent(&event);

/* variabilele sunt instantiate cu valorile masurate de senzor (masurate in m/s^2)*/
  accX = event.acceleration.x;
  accY = event.acceleration.y;
  accZ = event.acceleration.z;

/* se calculeaza acceleratia rezultanta si se transmite catre calculator*/
   accRezultanta = sqrt (accX*accX+accY*accY+accZ*accZ);
   Serial.print("Rezultanta: "); Serial.println(accRezultanta);
 
 if (debug) {  // in modul debug se transmit valorile obtinute
  Serial.print("X: "); Serial.print(event.acceleration.x); Serial.print("  ");
  Serial.print("Y: "); Serial.print(event.acceleration.y); Serial.print("  ");
  Serial.print("Z: "); Serial.print(event.acceleration.z); Serial.print("  ");Serial.println("m/s^2 ");
 }
   

}
