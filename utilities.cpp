#include "utilities.h"
#include "clock_core.h"

void checkForTimeUpdate(){
  Serial.println("waiting for time input...");
  if (Serial.available()) { // if there is data coming
    String input = Serial.readStringUntil('\n'); // read string until newline character
    //Dummy utc in epoch seconds time: 1672670580

    if(input.equals("clear")){
        clearAllLEDs();
    }
    else if(input.length() == 10){
      Serial.println("trying to adjust the rtc time to new value");
      rtc.adjust(DateTime(input.toInt()));
      clearAllLEDs();
    }
    else{ //set input as epoch seconds string
    Serial.println("unknown what to do with input: " + input);
    }
  }
}