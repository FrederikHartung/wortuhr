#include "led_tests.h"
#include "clock_core.h"

// Global position variables for running lights
int posi1 = 0;
int posi2 = 1;
int posi3 = 2;

void showEverySecondLed(){
  for (int i = 0; i < NUM_LEDS; i++) {
    if(i % 2 == 0){
      setLed(i, 100, 0, 0);
    }
    else{
      setLed(i, 0, 0, 0);
    }
  }

  pixels.show();
  delay(500);

  for (int i = 0; i < NUM_LEDS; i++) {
   if(i % 2 == 0){
      setLed(i, 0, 0, 0);
    }
   else{
      setLed(i, 100, 0, 0);
    }
  }

  pixels.show();
  delay(500);
}

void showSpecificLed(){
  setLed(0, 100, 0, 0);
  setLed(1, 0, 100, 0);
  setLed(2, 0, 0, 100);
  setLed(3, 100, 100, 100);
  pixels.show();
  delay(2000);
  pixels.clear();

  setLed(0, 0, 0, 0);
  setLed(2, 0, 0, 0);
  setLed(2, 0, 0, 0);
  setLed(3, 0, 0, 0);
  pixels.show();
  delay(500);
}

void showRunningLights(){
  setLed(posi1, 50, 0, 0);
  setLed(posi2, 0, 50, 0);
  setLed(posi3, 0, 0, 50);
  if(posi1 == 0){
   setLed(NUM_LEDS - 1, 0, 0, 0);  
  }
  else{
    setLed(posi1 - 1, 0, 0, 0); 
  }

  pixels.show();

  posi1++;
  if(posi1 == NUM_LEDS){
    posi1 = 0;
  }

  posi2++;
  if(posi2 == NUM_LEDS){
    posi2 = 0;
  }

  posi3++;
  if(posi3 == NUM_LEDS){
    posi3 = 0;
  }
}

void letAllShine(){
  for(int i = 0; i < NUM_LEDS; i++){
      setLed(i, 10, 10, 10);
  }

  pixels.show();
}

void testCornerLeds(){
  setLed(113, 10, 0, 0); 
  setLed(112, 10, 0, 0); 
  setLed(111, 10, 0, 0); 
  setLed(110, 10, 0, 0); 
  pixels.show();
  delay(5000); //sleep 5 sec
  setLed(113, 0, 10, 0); 
  setLed(112, 0, 10, 0); 
  setLed(111, 0, 10, 0); 
  setLed(110, 0, 10, 0); 
  pixels.show();
  delay(5000); //sleep 5 sec
  setLed(113, 0, 0, 10); 
  setLed(112, 0, 0, 10); 
  setLed(111, 0, 0, 10); 
  setLed(110, 0, 0, 10); 
  pixels.show();
  delay(5000); //sleep 5 sec
}