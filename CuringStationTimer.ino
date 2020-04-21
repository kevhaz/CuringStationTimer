#include <LedControl.h>
#include <EEPROM.h>


#define PIN_BUTTON 2
#define PIN_RELAY  13
#define PIN_BUZZER 12
#define PIN_JOYX A1
#define PIN_JOYY A0

#define NUMBUZZERSECS 1
#define NUMNOPSECS 20

#define FLASHMILLIS 200
#define FLASHSLOWCHANGE 1
#define FLASHFASTCHANGE 10

#define MAX_SECS (60*60*24)

typedef enum  { 
  NOP = 0, 
  RELAYON = 1, 
  BUZZERON = 2,
  SLEEP = 3,
  SETUP = 4
} STATUS;

STATUS g_iOp = NOP;

typedef enum {
  CENTRE = 0,
  UP,
  HARDUP,
  DOWN, 
  
  HARDDOWN,
  LEFT,
  RIGHT
} DIRECTION;

uint32_t g_nopTimer = 0;
uint32_t g_relayTimer = 0;
uint32_t g_buzzerTimer = 0;
uint32_t g_flashTimer = 0;
uint32_t g_millTO;
uint32_t g_relayTimerSeccondCounter;
uint32_t g_nopTimerSeccondCounter;
uint32_t g_flashTimerCounter;
uint32_t g_boolScreenState;
uint32_t g_numTimerSecs;

LedControl g_lc=LedControl(9,8,7,1);




void setup() {
  Serial.begin(115200);
  
  // put your setup code here, to run once:
  pinMode( PIN_BUTTON, INPUT_PULLUP );
  
  pinMode( PIN_RELAY,  OUTPUT );
  digitalWrite( PIN_RELAY, LOW );
  
  pinMode( PIN_BUZZER,  OUTPUT );
  digitalWrite( PIN_BUZZER, LOW );  
  /*
   The MAX72XX is in power-saving mode on startup,
   we have to do a wakeup call
   */
  g_lc.shutdown(0,false);
  /* Set the brightness to a medium values */
  g_lc.setIntensity(0,8);
  /* and clear the display */
  g_lc.clearDisplay(0);

  // TODO. Get last numTimerSecs out of EEPROM.
  EEPROM.get( 0, g_numTimerSecs );
  if ( g_numTimerSecs <= 0 ) {
    g_numTimerSecs = 30;
  }
  
  printDuration( g_numTimerSecs ); 

  STATUS g_iOp = NOP;
  g_nopTimer = millis();

  Serial.println ( "Ready!" );

}

void loop() {
  // When the button is pressed, the relay goes on for numSecs. On completion the relay switches off,
  // and the buzzer sounds for numBuzzerSecs
  int pinval;
  DIRECTION joydir = CENTRE;

  // we need to know what the joystick is doing
  pinval=analogRead(PIN_JOYX);
  if(pinval<256){joydir=LEFT;}
  if(pinval>768){joydir=RIGHT;}
  pinval=analogRead(PIN_JOYY);
  if(pinval<256){joydir=UP;}
  if(pinval>768){joydir=DOWN;}
  if(pinval<10){joydir=HARDUP;}
  if(pinval>980){joydir=HARDDOWN;}

  //Serial.print( "Y pinval = " );
  //Serial.print ( pinval );
  //Serial.print ( ". decoded direction = " );
  //Serial.println ( joydir );
    
  switch( g_iOp ) {
    case NOP:
        {
          g_millTO = NUMNOPSECS * 1000; // convert timeout time to milliseconds
          if ( (millis() - g_nopTimer) > g_millTO ) 
          {
            g_iOp = SLEEP;
            g_lc.shutdown(0,true);
            Serial.println( "Going to sleep" );
          } else {
            // in this state, we are looking for a button press. If the button goes LOW it was pressed. On a press we start the timer.
            int butState = digitalRead( PIN_BUTTON );
              
            if ( LOW == butState && CENTRE == joydir ) 
            {
 
               // Save this time for next time.
               EEPROM.put( 0, g_numTimerSecs );

               // lets start the timer and switch on the relay              
               g_iOp = RELAYON;
               digitalWrite( PIN_RELAY, HIGH );
               g_relayTimer = millis();
               delay(250); // delay 1/4second to avoid and debounce. This also sets the minumum relay on time.
               g_relayTimerSeccondCounter = 0;
               Serial.print( "Timer Started " );
               Serial.print( g_numTimerSecs );         
               Serial.println( "s remaining" );
            }
            if ( LOW == butState && RIGHT == joydir ) 
            {
               // lets start the timer and switch on the relay
               g_iOp = SETUP;
               delay(250); // delay 1/4second to avoid and debounce. This also sets the minumum relay on time.
               g_flashTimer = millis();
               g_boolScreenState = true;
               g_flashTimerCounter = 0;
               Serial.println( "Entering Setup Mode" );
            }
          }
          break;
        }
    case RELAYON:
      {
        // in this state we turn the relay off, only if it has been on long enough
        g_millTO = g_numTimerSecs * 1000; // convert timeout time to milliseconds
        if ( (millis() - g_relayTimer) > g_millTO ) 
        {
          // got timeout
          g_iOp = BUZZERON;
          digitalWrite( PIN_RELAY, LOW );
          digitalWrite( PIN_BUZZER, HIGH );  
          
          g_buzzerTimer = millis();
          
          g_relayTimerSeccondCounter = 0;
          Serial.println( "Timer Timed out" );
          
        } else {
          // If the button is pressed, then we cancel the timer
  
          int butState = digitalRead( PIN_BUTTON );
          if ( LOW == butState ) 
          {
             // lets start the timer and switch on the relay
             g_iOp = NOP;
             g_nopTimer = millis();
             g_relayTimerSeccondCounter = 0;
       
             digitalWrite( PIN_RELAY, HIGH );
             digitalWrite( PIN_BUZZER, HIGH );              
             delay(100); // delay 1/4second to avoid and debounce.
             digitalWrite( PIN_BUZZER, LOW );  
             delay(400);           
             Serial.println( "Timer Aborted" );

             printDuration( g_numTimerSecs );
          }
        }
        break;
      }
    case BUZZERON:
      {
           // in this state we turn the buzzery off, only if it has been on long enough
          g_millTO = NUMBUZZERSECS * 1000; // conver timeout time to milliseconds
          if ( (millis() - g_buzzerTimer) > g_millTO )
          {            
            digitalWrite( PIN_BUZZER, LOW ); 
            printDuration( g_numTimerSecs ); 

             // got timeout
            g_iOp = NOP;
            g_nopTimer = millis();
                    
            Serial.println( "Buzz Completed" ); 
          }
          break; 
      }
    case SLEEP:
        {
          // in this state, we are looking for a button press. If the button goes LOW it was pressed. we wake up the LED.
          int butState = digitalRead( PIN_BUTTON );
          if ( LOW == butState ) 
          {
            g_iOp = NOP;
            g_nopTimer = millis();
            g_nopTimerSeccondCounter = 0;
            g_lc.shutdown(0,false);
            Serial.println( "Waking up from sleep" );
            delay(250);            
          }
          break;
        }
    case SETUP:
        {
          // Step one, we need to flash the display  
          // I want to turn the LED on and off every 200ms.
          uint32_t flashduration = ( millis() - g_flashTimer ) / FLASHMILLIS;
          if ( flashduration > g_flashTimerCounter ) {
             g_boolScreenState = !g_boolScreenState;

             // on each flash state when we turn on the screen, we need to decide if we need to 
             // increment ir decrement the counter

             if ( g_boolScreenState ) {
              
               switch( joydir ) {
                  case UP: {
                      Serial.println( "Incrementing Time");
                      g_numTimerSecs+= FLASHSLOWCHANGE;
                    }
                    break;
                  case DOWN: {
                      Serial.println( "Decrementing Time");
                      g_numTimerSecs-= FLASHSLOWCHANGE;
                    }
                    break;
                  case HARDUP: {
                      Serial.println( "Incrementing Time");
                      g_numTimerSecs+= FLASHFASTCHANGE;
                    }
                    break;
                  case HARDDOWN: {
                      Serial.println( "Decrementing Time");
                      g_numTimerSecs-= FLASHFASTCHANGE;
                    }
                    break;                 
                 }

                 if (g_numTimerSecs < 1 ) g_numTimerSecs = 1;
                 if (g_numTimerSecs >= MAX_SECS ) g_numTimerSecs = g_numTimerSecs-1;
                 
                 printDuration( g_numTimerSecs );
             }

             g_lc.shutdown(0,g_boolScreenState);
             g_flashTimerCounter = flashduration;             
          }
          int butState = digitalRead( PIN_BUTTON );
          if ( LOW == butState && CENTRE == joydir ) 
          {
            g_iOp = NOP;
            g_nopTimer = millis();
            g_lc.shutdown(0,false);
            Serial.println( "Exiting Setup" );
            delay(250);            
          }
          break;
        }
    default:
      Serial.println ("Error State");
  }  // switch


  if ( NOP == g_iOp ) {
    // how long has we been idling so far in seconds
    uint32_t duration = ( millis() - g_nopTimer ) / 1000;

    if (duration > g_nopTimerSeccondCounter )
    { 
       // another second has passed by
        g_nopTimerSeccondCounter = duration;
        Serial.print( "Go to sleep in ");     
        Serial.print( int(NUMNOPSECS - g_nopTimerSeccondCounter) );         
        Serial.println( "s" );   

    }
  }

  
  if ( RELAYON == g_iOp ) {
    // how long has the relay been on so far in seconds
    uint32_t duration = ( millis() - g_relayTimer ) / 1000;

    if (duration > g_relayTimerSeccondCounter )
    { 
       // another second has passed by
        g_relayTimerSeccondCounter = duration;
           
        Serial.print( long(g_numTimerSecs - g_relayTimerSeccondCounter) );         
        Serial.println( "s remaining" );   
        printDuration( g_numTimerSecs - g_relayTimerSeccondCounter );     

    }
  }
      
}
