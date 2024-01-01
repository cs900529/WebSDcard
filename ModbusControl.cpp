#include "ModbusControl.h"

const int FIR_ORDER = 100;

// solar_flaten
int fir_filter[FIR_ORDER]={0};

// demand_response
int demandArray[900]={0};
int count = 0;

void reset() {
  // 相關資料 reset
  memset(demandArray, 0, sizeof(demandArray));
  memset(fir_filter, 0, sizeof(fir_filter));
  count = 0;
}

float solar_flaten(float input_mppt) {
  int i;
  float temp=0;
  for(i=FIR_ORDER-1;i>0;i--) {
    fir_filter[i]=fir_filter[i-1];
  }
      
  fir_filter[0]=input_mppt;

  for(i =0;i<FIR_ORDER;i++) {
    temp+= fir_filter[i]*(1.0/FIR_ORDER);
  }
  return temp;
}

int demand_response(int load_power) {
  int goal = 1000; // not used
  for(int i=30;i>0;i--){
    demandArray[i] = demandArray[i-1];
  }
  demandArray[0] = load_power;

  // Calculate average
  int sum = 0;
  float avg = 0;
  for(int i=0;i<30;i++) {
    sum += demandArray[i]; 
  }
  count = count + 1;
  Serial.println(count);

  if(count == 30) {
    avg = sum / 30.0;
    count = 0;
    if(avg >950) {
      Serial.println("Closed");
      return 1;
    }
    else {
      Serial.println("Keep Load");
      return 0;
    }
  }
  return 0;
}
