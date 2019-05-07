#include <LiquidCrystal.h>
#include <SoftwareSerial.h>
#include <BlynkSimpleStream.h>
LiquidCrystal lcd(12, 11, 5, 4, 3 ,2 );           
SoftwareSerial DebugSerial(2, 3); // RX, TX
#define BLYNK_PRINT DebugSerial
#define trigPin1 6 
#define echoPin1 7
#include <dht11.h>
#define DHT11PIN 9  

const int calibrationLed = 13;                      
const int MQ_PIN=A0; 
int buzzer = 8 ;                               
int RL_VALUE=5;                                     
float RO_CLEAN_AIR_FACTOR=9.83;
int CALIBARAION_SAMPLE_TIMES=50;                    
int CALIBRATION_SAMPLE_INTERVAL=500;                                                                    
int READ_SAMPLE_INTERVAL=50;                        
int READ_SAMPLE_TIMES=5;                                                                            
#define         GAS_LPG             0   
#define         GAS_CO              1   
#define         GAS_SMOKE           2    
float           LPGCurve[3]  =  {2.3,0.21,-0.47};                                                      
float           COCurve[3]  =  {2.3,0.72,-0.34};                                                       
float           SmokeCurve[3] ={2.3,0.53,-0.44};                                                                                                         
float           Ro           =  10; 
dht11 DHT11;

int visitor = 0; // de base il y a personne dans notre magasin et visitor ne peut prendre une valeur negative


char auth[] = "7846605ad72f4f55a69ccb16093086ce";


//***********************************************************************


BlynkTimer timer;

        void myTimerEvent()
        {     
              long iPPM_LPG = 0;
              long iPPM_CO = 0;
              long iPPM_Smoke = 0;
              
              Serial.println();              
              Serial.println();
              Serial.println();
              Serial.println();
              Serial.println("****************myTimerEvent****************");
              Serial.println();
              iPPM_LPG = MQGetGasPercentage(MQRead(MQ_PIN)/Ro,GAS_LPG);
              iPPM_CO = MQGetGasPercentage(MQRead(MQ_PIN)/Ro,GAS_CO);
              iPPM_Smoke = MQGetGasPercentage(MQRead(MQ_PIN)/Ro,GAS_SMOKE);


               //AFFICHAGE SUR L'APPLICATION
               //Blynk.virtualWrite(V2, iPPM_CO);
               //Blynk.virtualWrite(V3, iPPM_LPG);
               //Blynk.virtualWrite(V5, iPPM_Smoke);
               
              
              if (iPPM_LPG > 10 or iPPM_CO > 10 or iPPM_Smoke > 10 )
              { Serial.println("  A L E R T ! ! ! ! ! ! ");
                digitalWrite  (buzzer, HIGH);

                //    Email Et NOTIFICATION
                 Blynk.email("hattabimahmoud@gmail.com", "Alerte Feu", "appel d'urgence Feu detecté chez vous !!");
                 Blynk.notify("Alerte Feu");
                 
                 // Affichage sur l'app
                 //Blynk.virtualWrite(V0, "Fumée detectée");
                 //Blynk.virtualWrite(V1, "Alarme");
                }
              else
              { digitalWrite(buzzer, LOW);
                Serial.println("Tout est NORMALE :) ");
              }
              
              // PARTIE DE COMPTAGE
              long duration1, distance1; // ici on dÃ©fini deux variables qui correspondent a la distance de l'obstacle
              digitalWrite(trigPin1, HIGH);
              delayMicroseconds(2); // test
              digitalWrite(trigPin1, HIGH);
              delayMicroseconds(10);
              digitalWrite(trigPin1, LOW);
              delayMicroseconds(10); // test
              digitalWrite(trigPin1, LOW);
              duration1 = pulseIn(echoPin1, HIGH);
              distance1 = duration1 / 58; 
              // Serial.print("Capteur 1: ");
              // Serial.println(distance1); // si besoin pour deboguer
              if(distance1 < 1000 ) { 
              visitor = visitor+1; // ALORS on ajoute +1 a la variable visitor
              }
              else { 
              visitor = visitor-1; // ALORS on enleve -1 a la variable visitor
              }
              Serial.print("Nombre de personnes : ");
              Serial.println(visitor); 
              //Serial.println(distance1);      

              //  Partie AFFICHAGE sur la LCD
              lcd.clear();   
              lcd.setCursor( 0 , 0 );
              lcd.print("LPG: ");
              lcd.print(iPPM_LPG); 
              Serial.println("**************** LPG ****************");
              Serial.println(iPPM_LPG);
              lcd.print(" CO: ");
              lcd.print(iPPM_CO);
              Serial.println("**************** CO ****************");
              Serial.println(iPPM_CO);
              lcd.setCursor( 0,1 );
              lcd.print("FUMEE: ");
              lcd.print(iPPM_Smoke);
              //lcd.print(" ppm");
              Serial.println("**************** SMOKE ****************");
              Serial.println(iPPM_Smoke);
              lcd.print(" P: ");
              lcd.print(visitor);

              // Partie capteur humidité et temperature
              DHT11.read(DHT11PIN);     
              Serial.print("Humidité (%):");
              Serial.print((float)DHT11.humidity, 2);
              Serial.print("\t");
              Serial.print("Température (°C): ");
              Serial.println((float)DHT11.temperature, 2);
               
         }



void setup()
{
        /*******************************/
      
        // Debug console
        DebugSerial.begin(9600);
      
        // Blynk will work through Serial
        // Do not read or write this serial manually in your sketch
        Serial.begin(9600);
        Blynk.begin(Serial, auth);

        //  COMPTEUR DE PERSONNE
        pinMode(trigPin1, OUTPUT); 
        pinMode(echoPin1, INPUT);

        //  L'affichage LCD
        lcd.begin(16,2);
        pinMode(calibrationLed,OUTPUT);
        pinMode(buzzer, OUTPUT) ;
        digitalWrite(calibrationLed,HIGH);
        lcd.print("Calibration...");                       
        Ro = MQCalibration(MQ_PIN);                               
        digitalWrite(calibrationLed,LOW);
        lcd.clear();     
        lcd.print("FAIT!");                               
        lcd.setCursor(0,1);
        lcd.print("Ro= ");
        lcd.print(Ro);
        lcd.print("kohm");
        Serial.println("calibrage fait");
        timer.setInterval(1000L, myTimerEvent);
        delay(6000);

}


void loop()
{
    //j'execute le blynk et le timer 
    Blynk.run();
    timer.run();

}

float MQResistanceCalculation(int raw_adc)
{  return ( ((float)RL_VALUE*(1023-raw_adc)/raw_adc));}



 
long MQGetGasPercentage(float rs_ro_ratio, int gas_id)
{
  if ( gas_id == GAS_LPG ) {
     return MQGetPercentage(rs_ro_ratio,LPGCurve);
  } else if ( gas_id == GAS_CO ) {
     return MQGetPercentage(rs_ro_ratio,COCurve);
  } else if ( gas_id == GAS_SMOKE ) {
     return MQGetPercentage(rs_ro_ratio,SmokeCurve);
  }    
 
  return 0;
}

float MQCalibration(int mq_pin)
{
  int i;
  float val=0;

  for (i=0;i<CALIBARAION_SAMPLE_TIMES;i++) {            //prendre plusieurs échantillons
    val += MQResistanceCalculation(analogRead(mq_pin));
    delay(CALIBRATION_SAMPLE_INTERVAL);
  }
  val = val/CALIBARAION_SAMPLE_TIMES;                   //calculer la valeur moyenne
  val = val/RO_CLEAN_AIR_FACTOR;                        //divisé par RO_CLEAN_AIR_FACTOR donne le Ro                                  
  return val;                                           //selon le tableau dans la fiche technique 

}


float MQRead(int mq_pin)
{
  int i;
  float rs=0;
 
  for (i=0;i<READ_SAMPLE_TIMES;i++) {
    rs += MQResistanceCalculation(analogRead(mq_pin));
    delay(READ_SAMPLE_INTERVAL);
  }
 
  rs = rs/READ_SAMPLE_TIMES;
 
  return rs;  
}
 
long  MQGetPercentage(float rs_ro_ratio, float *pcurve)
{
  return (pow(10,( ((log(rs_ro_ratio)-pcurve[1])/pcurve[2]) + pcurve[0])));
}
 
