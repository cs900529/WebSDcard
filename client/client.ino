/*
  Modbus-Arduino Example - Master Modbus IP Client (ESP8266/ESP32)
  Read Holding Register from Server device

  (c)2018 Alexander Emelianov (a.m.emelianov@gmail.com)
  https://github.com/emelianov/modbus-esp8266
*/

#ifdef ESP8266
 #include <ESP8266WiFi.h>
#else
 #include <WiFi.h>
#endif
#include <ModbusIP_ESP8266.h>

const int PV_REG = 9;               // Modbus Hreg Offset
const int LOAD_REG = 2181;
const int FIR_ORDER = 100;

IPAddress remote(140, 115, 65, 193);  // Address of Modbus Slave device
const int LOOP_COUNT = 10;

ModbusIP mb;  //ModbusIP object
int fir_filter[FIR_ORDER]={0};

float solar_flaten(float input_mppt)
{
  int i;
  float temp=0;
  for(i=FIR_ORDER-1;i>0;i--)
  {
      fir_filter[i]=fir_filter[i-1];
  }
      
  fir_filter[0]=input_mppt;

  for(i =0;i<FIR_ORDER;i++)
    temp+= fir_filter[i]*(1.0/FIR_ORDER);
    /*
  Serial.print(" temp:");
  Serial.print(temp);*/
  return temp;
}

void setup() {
  Serial.begin(115200);
 
  WiFi.begin("lab428", "3454834548");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
 
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  mb.client();
}

uint16_t pv_power = 0;
uint8_t show = LOOP_COUNT;
uint16_t load_power =0;
uint16_t flatten_power = 0;
void loop() {
  if (mb.isConnected(remote)) {   // Check if connection to Modbus Slave is established
    mb.readHreg(remote, PV_REG, &pv_power);  // Initiate Read Coil from Modbus Slave
    mb.readHreg(remote, LOAD_REG, &load_power);
  } else {
    mb.connect(remote);           // Try to connect if no connection
  }
  mb.task();                      // Common local Modbus task
  flatten_power = solar_flaten(pv_power*0.1);
  delay(200);                     // Pulling interval
  if (!show--) {                   // Display Slave register value one time per second (with default settings)
    Serial.print("PV_power:");
    Serial.print(pv_power*0.1);
    Serial.print(" Load_power");
    Serial.print(load_power);
    Serial.print(" Flatten_power");
    /*
    for(int i = 0 ; i < 25 ; i++)
    {
        Serial.print("filter:");
        Serial.println(fir_filter[i]);
    }*/
      
    Serial.println(flatten_power);
    show = LOOP_COUNT;
  }
}
