/*
Title:Final Project
Written By Keith Lin
This is a code about the EELab
 sem 2 final project exp 4 arduino
Date: 22/5/2022
*/

//IMPORT:
#include <HCSR04.h>
#include <AdvancedSevenSegment.h>
#include <LM35.h>
#include <TimerOne.h>

//SETUP: the DistanceSensor 
#define tri A1
#define echo A2 
//setup the button using interrupt 
#define button 2 
// display 
#define n_SEG 0, 0, 1, 0, 1, 0, 1
#define o_SEG 1, 1, 0, 0, 0, 1, 1

//sensor
UltraSonicDistanceSensor distanceSensor(tri, echo);
//display 
AdvanceSevenSegment seg(12, 13, 8, 7, 6, 11, 10, 9); // GLOBAL: seven segment //11, 10, 8, 7, 6, 12, 13, 9
const int pin_dis[] = { 5,4 }; // display change pin 
char display_chr[6];
//GLOBAL: temperature sensor 
LM35 temp(A0);
//mode 
volatile boolean MODE = true;
volatile uint16_t mainStep = 0; // only 6 step 
#define step_plus(s) ((s + 1) == 6 ? 0: s + 1) // step de-overflow func 
#define to_INT(num) num - '0'
// 
// display function according the step 
void display_function(uint16_t step);

/**
 * @brief Make number to Char array
 *
 * @param number want number to display array
 */
void to_display_chr_custom(float number) {
    String num_Str;
    Serial.print("Mode: "); // Serial output the mode 

    if (MODE) {// distance mode 
        Serial.print("distance mode, ");

        num_Str = String(static_cast<int>(number));
        if (number < 100) {
            // add zero in front 
            num_Str = String("000");
            if (number > 0) {
                num_Str = String('0') + String(static_cast<int>(number));
            }
        }

        Serial.print((number < 0 ? 0 : number));
        Serial.println(" cm");
    }
    else { //temperature mode  
        Serial.print("temperature mode, ");

        num_Str = String(static_cast<int>((number < 100 ? number * 10 : number)));

        Serial.print(number);
        Serial.println(" C");

    }

    num_Str.toCharArray(display_chr, 6);
}
/**
 * @brief Change mode from button
 *
 */
void change_mode() {
    static uint32_t last_interrupt_time = 0;
    uint32_t interrupt_time = millis();

    if (interrupt_time - last_interrupt_time > 200) { //debouncing function 
        MODE = !MODE; //change mode 
        mainStep = 0; //init step 

        to_display_chr_custom((MODE == true ? distanceSensor.measureDistanceCm(temp.cel()) : temp.cel()));

    }
    last_interrupt_time = interrupt_time;

}

/**
 * @brief function for display
 *
 */
void move_step() {
    mainStep++;
    if (mainStep == 6) {
        mainStep = 0;

    }
    else if (mainStep == 5) {
        to_display_chr_custom((MODE == true ? distanceSensor.measureDistanceCm(temp.cel()) : temp.cel()));
    }

}

void passive_pin(boolean f_s) {
    digitalWrite(pin_dis[0], f_s);
    digitalWrite(pin_dis[1], !f_s);
}

void clr_dis() {
    digitalWrite(pin_dis[0], LOW);
    digitalWrite(pin_dis[1], LOW);
}

/**
 * @brief Display function
 *
 */
void dis_ss() {
    auto Main_step_tmp = mainStep;
    int display_step[] = { Main_step_tmp , step_plus(Main_step_tmp) };
    boolean state = LOW;

    for (auto& e : display_step) {
        clr_dis();
        display_function(e);
        state = !state;
        passive_pin(state);
        delay(10);

    }
}

void setup() { // SETUP:
    //for debug 
    Serial.begin(9600);

    //display setup 
    pinMode(pin_dis[0], OUTPUT);
    pinMode(pin_dis[1], OUTPUT);

    // Timer init 
    Timer1.initialize(750000); //0.75 second //750000 us //like thead 
    Timer1.attachInterrupt(move_step);

    //button setup 
    pinMode(button, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(button), change_mode, FALLING);

    //init function 
    MODE = 0;
    to_display_chr_custom(distanceSensor.measureDistanceCm(temp.cel()));
    seg.setDot(0);

}

/**
 * @brief Only display
 *
 */
void loop() {
    dis_ss();
}

/**
 * @brief display_function
 *
 * @param step For display
 */
void display_function(uint16_t step) {
    seg.setDot(0);
    if (MODE) { // distance mode 
        switch (step) {
        case 0:
            seg.setNumber(to_INT(display_chr[0]));
            break;
        case 1:
            seg.setNumber(to_INT(display_chr[1]));
            break;
        case 2:
            seg.setNumber(to_INT(display_chr[2]));
            break;
        case 3:
            seg.setCharacter('c');
            break;
        case 4:
            seg.print(n_SEG);
            break;
        case 5: // space
            seg.clean();
            break;

        default: // 
            seg.clean();
            break;
        }
    }
    else { //temperature mode  
        switch (step) {
        case 0:
            seg.setNumber(to_INT(display_chr[0]));
            break;
        case 1:
            seg.setNumber(to_INT(display_chr[1]));
            seg.setDot(1);
            break;
        case 2:
            seg.setNumber(to_INT(display_chr[2]));
            break;
        case 3:
            seg.print(o_SEG);
            break;
        case 4:
            seg.setCharacter('c');
            break;
        case 5: // space
            seg.clean();
            break;

        default: // 
            seg.clean();
            break;
        }
    }
}