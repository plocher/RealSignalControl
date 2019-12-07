// Drive a LED signal head using relays or a Seeed Studio high current RGB strip driver with an external power supply.
// (https://products.currentbyge.com/transportation-lighting/rail-signals/wayside-signals)
//
// In mid-2015, I helped demonstrate that early versions of a real railroad signal head had an unexpected
// failure mode:  In certain conditions, they would blink instead of showing a steady color.
//
// If a signal's power supply / battery had an internal resistance that lowered its output voltage slightly
// under load, and it's output voltage was just above that needed to turn on the LED lamp, the following
// behaviors would happen:
//
//  1) OK - the steady state current capacity of the PS was sufficient to the load:
//
//     The LED lamp would turn on bright and stay on.
//
//  2) Marginal - the current sourcing was less than that consumed by the LED lamp, but the regulation was sufficient
//     to keep the voltage from dropping:
//
//     The LED lamp would light brightly, quickly dim and stay on at a dim level.  This can often be seen in the orange
//     "Don't Walk" pedestrian traffic crosswalk signals if you know what to look for...
//
//  3) Failure - the regulation can't keep the voltage high enough, such as in a discharging battery:
//
//     The lamp would light bright, quickly dim and then go out, and when the power supply recovered,
//     the lamp would turn back on and repeat the cycle, producing a blinking light pattern at 1/2 to 2 Hz
//
//  This is a concern because railroad signals are supposed to be part of a FAIL SAFE system.
//  DARK signals (burnt out lamps) are more restrictive than steady-on ones, and
//  steady-on ones are more restrictive than blinking ones:
//      DARK signals mean STOP and contact dispatcher for instructions
//      STEADY RED means stop and wait,
//      BLINKING READ means STOP and proceed slowly
//  OOPS!  A malfunctioning LED signal like this will actually fail /LESS/ safely
//
//  Cause: LEDs are voltage-activated, current driven devices.  They stay off (don't conduct) until a
//  threshold voltage is reached, at which time they conduct, with the current flow proportional to their brightness.
//  This inrush current is typically supplied from the capacitors found in the power source, with the source's
//  linear or switching regulators tasked with keeping up with the demand.
//  If the continuous power delivery of the supply is less than that of its peak, but still sufficient,
//  the LEDs will start bright and quickly dim, giving a strobe-like visual appearance.
//  If their current draw exceeds the continuous capacity, the supply's voltage typically drops, and at some point in
//  time, will drop below the LED's turn-on voltage threshold - which will cause the LED to turn off and stop drawing
//  current.  This allows the supply time to recover, recharge itself, and raise its voltage back above the turn-on
//  threshold...
//
//  To reproduce:
//     0) start with an early model railroad signal lamp LED head (current ones don't have this bug!)
//     1) set up a bench supply with V=LED turn on voltage (say 10v)
//     2) set up current limiting on the supply such that it is greater than needed
//     3) connect the LED lamp to the power supply.
//     4) slowly lower the current limit until the LEDs start to dim
//     5) slowly lower the voltage limit until the lamp turns off
//     6) very slightly RAISE the voltage - if your bench supply isn't perfect, the LED lamp should start blinking
//
//  Result of testing:  The signals that displayed this behavior were immediately taken out of service and replaced
//  by an updated version.


#include <Arduino.h>

#define USE_STRIP_DRIVER 1  // = 0 if using relays instead

#if USE_STRIP_DRIVER == 1
#include "RGBdriver.h"
#define CLK 7    //pins definitions for the driver        
#define DIO 6
RGBdriver Driver(CLK,DIO);
#else // use relays instead of strip driver:
#define redPin 13
#define yellowPin 12
#define greenPin 11
#endif

#define BLINKTIME (1200)        // mS
#define SECOND    (1000)        // mS
void setRed   (void);
void setYellow(void);
void setGreen (void);
void setBlack (void);


enum Speed { OFF, SLOW, FAST, ERROR } 
    myspeed = OFF;
enum State { INIT,  WAIT, WAITING, STOP, RESTRICTING, APPROACH, ADVANCEDAPPROACH, CLEAR } 
    state = INIT, nextstate = INIT, laststate = INIT;
enum Blink { DISABLED, RED, YELLOW, GREEN } 
    blinking = DISABLED;
    
static unsigned long lWaitMillis, lBlinkMillis;
static long when;
static boolean blinkOn = LOW;


void setup() {
  myspeed = FAST;
#if USE_STRIP_DRIVER == 0
  pinMode(redPin,    OUTPUT);
  pinMode(yellowPin, OUTPUT);
  pinMode(greenPin,  OUTPUT);
#endif
}

void loop() {
    switch (state) {
        default:
        case INIT:   
                    setBlack();
                    state = STOP;
                    laststate = INIT;
                    lWaitMillis = millis();
                    lBlinkMillis = millis();
                    blinking = DISABLED;
                    when =0;
                    break;
        case STOP:
                    setRed();
                    state = WAIT;
                    nextstate = RESTRICTING;
                    break;
        case RESTRICTING:
                    setRed(); 
                    blinking = RED;
                    state = WAIT;
                    nextstate = APPROACH;
                    break;
        case APPROACH:
                    setYellow(); 
                    state = WAIT;
                    nextstate = ADVANCEDAPPROACH;
                    break;
        case ADVANCEDAPPROACH:
                    setYellow();
                    blinking = YELLOW;
                    state = WAIT;
                    nextstate = CLEAR;
                    break;
        case CLEAR:
                    setGreen();
                    state = WAIT;
                    nextstate = STOP;
                    break;
        case WAIT:
                    if (laststate == INIT) {
                        when = 0;
                    } else {
                        if (myspeed == SLOW) when = random(10,100);
                        if (myspeed == FAST) when = random(200,1000);
                    }
                    lWaitMillis = millis() + (1 * SECOND);
                    lBlinkMillis = millis() + BLINKTIME;
                    blinkOn = HIGH;
                    laststate = state;
                    state = WAITING;
                    break;
        case WAITING:
                    if (blinking != DISABLED) {
                        if( (long)( millis() - lBlinkMillis ) >= 0) {
                            lBlinkMillis = millis() + BLINKTIME;
                            blinkOn = !blinkOn;
                            if (blinkOn) switch (blinking) {
                                case DISABLED:
                                default:                     break;
                                case RED:    setRed();       break;
                                case YELLOW: setYellow();    break;
                                case GREEN:  setGreen();     break;
                            } else { setBlack(); }
                        }
                    }
                    if( (long)( millis() - lWaitMillis ) >= 0) {
                        when--;
                        if (when <= 0) {
                            state = nextstate;
                            blinking = DISABLED;
                        } else {
                            lWaitMillis = millis() + (1 * SECOND);
                        }
                    }
                    break;
    }
    delay(100);
}

// change these routines if you are using a set of 
// relays instead of the strip driver
#if USE_STRIP_DRIVER == 1
// Direct drive Grove LED Strip Driver
void display(int r, int g, int b) {
    Driver.begin(); // begin
    Driver.SetColor(r,g,b); 
    Driver.SetColor(r,g,b); 
    Driver.end();
}
void setRed   (void) { display(255, 0,   0); }
void setYellow(void) { display(0, 255,   0); }
void setGreen (void) { display(0,   0, 255); }
void setBlack (void) { display(0,   0,   0); }

#else
/* for relay use: */
void setRed   (void) { setBlack(); digitalWrite(redPin,    HIGH); }
void setYellow(void) { setBlack(); digitalWrite(yellowPin, HIGH); }
void setGreen (void) { setBlack(); digitalWrite(greenPin,  HIGH); }
void setBlack (void) { digitalWrite(redPin, LOW);digitalWrite(yellowPin, LOW);digitalWrite(greenPin, LOW); }
#endif





