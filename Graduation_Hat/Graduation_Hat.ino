#include <Arduino.h>
#include <FastLED.h>  //Libraries
#include "Freenove_IR_Lib_for_ESP32.h"
#include "text.h"
Freenove_ESP32_IR_Recv ir_recv(15);  //Change it according to the pin number you are using for IR receiver

#define NUM_LEDS 49    //Number of LEDs
#define DATA_PIN 18    //Data Pin
#define BACKUP_PIN 19  //Backup Data Pin
CRGB leds[NUM_LEDS];
int brightness = 125;  //Initial Brightness

// RGB Setting
//The arrays are in following colors: {Red, Red to Orange, Orange, Orange to Yellow, Yellow, Green, Green to Cyan, Cyan, Cyan to Teal, Teal, Blue, Blue to Purple, Purple to Pink, Pink}
int red[] = { 255, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 40, 130, 180, 255 };
int green[] = { 0, 10, 30, 150, 255, 255, 255, 255, 127, 63, 0, 0, 0, 0, 0 };
int blue[] = { 0, 0, 0, 0, 0, 0, 127, 255, 255, 255, 255, 255, 255, 255, 255 };
int code[] = { 16195807, 16191727, 16199887, 16189687, 16197847, 16228447, 16224367, 16232527, 16222327, 16230487, 16212127, 16208047, 16216207, 16206007, 16214167 };  //These are the corresponding codes on the IR remote
int color;
bool ledMatrix[7][12];
bool useRGB;
long blinkClock;
bool blink;
bool blinkMode = false;

long previousMillis;
int brightLedCount;
int blackLedCount;
int characterCount;
int frame;
int hue;
int jokes;

uint32_t data;  //This is where the code sent from the IR receiver will be stored
int mode = 1;

void LED_Control(void* parameter) {
  while (1) {                       //Since this function is running outside the loop(), there is while(1) to make it act similar to loop().
    if (blinkMode) {
      if (millis() - blinkClock > 1000) {
        if (blink) {
          FastLED.clear();
          blink = false;
        } else {
          blink = true;
        }
        blinkClock = millis();
      }
    }

    if (ir_recv.nec_available()) {  //ir_recv.nec_available() triggers when a new code is received.
      Serial.print("IR Protocol: ");
      Serial.print(ir_recv.protocol());
      Serial.print("IR Code: ");
      Serial.println(ir_recv.data());      //Serial print the protocol and the code
      if (ir_recv.data() != 4294967295) {  //My IR remote sends an invalid code when I hold on to the button, so this prevents the invaild code from storing in "data".
        data = ir_recv.data();
        brightLedCount = 0;
        blackLedCount = 0;

        if (data == 16187647 || data == 16220287) {  //Brightness Control
          if (data == 16187647) {
            brightness += 25;
          } else {
            brightness -= 25;
          }
          brightness = constrain(brightness, 0, 255);  //Constrain to 0-255
          FastLED.setBrightness(brightness);
        }

        if (data == 16240687) {  //Change to Different Mode
          mode++;
          if (mode > 7) {  //Change it to the number of modes you would like to have
            mode = 1;
          } else if (mode >= 4 and mode <= 5) {
            for (int i = 0; i < 7; i++) {
              for (int j = 0; j < 12; j++) {
                ledMatrix[i][j] = 0;
              }
            }
            frame = 6;
            characterCount = 0;
            jokes = 0;
          }
        }

        for (int i = 0; i < 15; i++) {  //Color Change //Change "15" to the number of buttons you are using for different colors
          if (data == code[i]) {
            color = i;
            break;
          }
        }

        if (data == 16203967) {  //Turn Off
          mode = 0;
        }

        if (data == 16236607) {  //Turn On
          mode = 1;
        }

        if (data == 16248847) {
          if (useRGB) {
            useRGB = false;
          } else {
            useRGB = true;
          }
        }

        if (data == 16238647) {
          if (blinkMode) {
            blinkMode = false;
          } else {
            blinkMode = true;
          }
        }

        FastLED.clear();  //Clear the LED Matrix
      }
    }
    if (mode == 1) {                          //This mode allows the LEDs to turn on one by one and turn off individually again after all have been lit up
      if (millis() - previousMillis > 500) {  //0.5s delay until the next LED turns on
        previousMillis = millis();
        if (brightLedCount < 49) {
          leds[brightLedCount] = CRGB(green[color], red[color], blue[color]); //Somehow the parameters for RGB are actually GRB
          brightLedCount++;
        } else if (blackLedCount < 49) {
          leds[blackLedCount] = CRGB::Black;
          blackLedCount++;
        } else {
          brightLedCount = 0;
          blackLedCount = 0;
        }
      }
    } else if (mode == 2) {  //This mode turns all LEDs
      for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CRGB(green[color], red[color], blue[color]);
      }
    } else if (mode == 3) {  //Rainbow Effect
      for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CHSV(6 * i + hue, 255, 255);
      }
      hue += 1;                                                                           //Adjust this to change the speed
      if (hue > 255) {
        hue = 0;
      }
    } else if (mode == 4) {                                                               //This mode allows for text to print
      int text[] = {6, 17, 0, 3, 20, 0, 19, 4, 42, 14, 5, 42, 28, 26, 28, 32, 37, 42, 42, 42};
      printText(text, 18);                                                                //Adjust the second parameter to the number of characters in your message, including space(' ')
    } else if (mode == 5) {
      if (jokes == 0) {
        characterCount = 0;
        frame = 6;
        jokes = random(1, 22);
        Serial.println(jokes);
      }
      printText(jokesText[jokes], jokesLength[jokes]);
    } else if (mode == 6) {
      bool heart[49] = {0,1,1,0,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,0,0,0,1,1,1,0,0,0,0,0,1,0,0,0};
      if (blinkMode == false || blink) {
        for (int i = 0; i < NUM_LEDS; i++) {
          if (heart[i]) {
            if (useRGB) {
              leds[i] = CHSV(6 * i + hue, 255, 255);
            } else {
              leds[i] = CRGB(green[color], red[color], blue[color]);
            }
          }
        }
      }

    } else if (mode == 7) {
      bool note[49] = {0,0,0,1,0,0,0,0,0,1,1,0,0,0,0,0,0,1,0,1,0,0,0,0,1,0,0,0,0,1,1,0,0,0,0,0,0,0,1,1,1,1,0,1,1,0,0,0,0};
      if (blinkMode == false || blink) {
        for (int i = 0; i < NUM_LEDS; i++) {
          if (note[i]) {
            if (useRGB) {
              leds[i] = CHSV(6 * i + hue, 255, 255);
            } else {
              leds[i] = CRGB(green[color], red[color], blue[color]);
            }
          }
        }
      }
    }
    hue += 1;
    if (hue > 255) {
      hue = 0;
    }
    FastLED.show();  //Turn on the LEDs according to how we mapped it on leds[]
    vTaskDelay(1);   //Having a delay is crucial in the code because without it, loop() does not get to run as LED_Control() has more priority
  }
}

void printText(int text[], int length) {
  if (frame < 7) {
    for (int i = 0; i < 7; i++) {
      for (int j = 0; j < 6; j++) {
        ledMatrix[i][j + 6] = alphabet[text[characterCount]][(6 * i) + j];
      }
    }

    characterCount ++;
    if (characterCount == length) {
      characterCount = 0;
      if (mode == 5) {
        jokes = 0;
      }
    }
    frame += 6;
  } 
  if (millis() - previousMillis > 125) {
    FastLED.clear();
    for (int i = 0; i < 7; i++) {
      if (i % 2 == 0) {
        for (int j = 0; j < 7; j++) {
          if (ledMatrix[i][j]) {
            leds[(7 * i) + j] = CRGB(green[color], red[color], blue[color]);
          }
        }
      } else {
        for (int j = 6; j > -1; j--) {
          if (ledMatrix[i][j]) {
            leds[(7 * i) + 6 - j] = CRGB(green[color], red[color], blue[color]);
          }
        }
      }
    }
    /*for (int i = 0; i < 7; i++) {
      for (int j = 0; j < 12; j++) {
        Serial.print(ledMatrix[i][j]);
      }
      Serial.println();
    }
    Serial.println();*/
    for (int i = 0; i < 7; i++) {
      for (int j = 0; j < 11; j++) {
        ledMatrix[i][j] = ledMatrix[i][j + 1];
      }
    }

    frame --;
    previousMillis = millis();
  }
}

void setup() {
  Serial.begin(115200);
  FastLED.addLeds<WS2813, DATA_PIN, RGB>(leds, NUM_LEDS);                        //Switch "WS2813" to the type of LED strip you are using
  xTaskCreatePinnedToCore(LED_Control, "LED_Control", 2048, NULL, 10, NULL, 1);  //We are creating a task using FreeRTOS to run LED_Control() similar to loop() becasue ir_recv.task() is a delayed function
}

void loop() {
  ir_recv.task();  //This function receives the incoming code from IR remote //The reason why LED_Control() is not placed inside loop() is that ir_recv.task() waits infinitely until it receives a code, which makes it impossible to display patterns, unless the multitasking feature in ESP32 is enabled like here
}