#include <lightbar.h>
#include <serial.h>
#include <TimedAction.h>
#include <SoftwareSerial.h>

// Setup the amber light bar connection.
lightBar amberLightBar;

// Variables for input pins.
int alternatingHeadlightsButton[] = {3,40};    // The button that activates or deactivates the alternating headlamps.
int sirenButton[]                 = {4,41};    // The button that primes and un-primes the siren.
int arrivalButton[]               = {5,42};    // This button will deactivate everything.
int tripleNineButton[]            = {6,43};    // Turns the system to 999 mode (everything on)
int grillStrobeButton[]           = {7,44};
int ambersButton[]                = {9,46};

int hornButtonPin               = 2;      // Connects to vehicle horn and controls changing of the siren tones when the siren is primed.
//int hornButtonPin                 = 9;      // Connects to vehicle horn and controls changing of the siren tones when the siren is primed.
int sirenOutputPin                = 39;    // The pin which outputs the note for the siren.

// Arrival function variables
unsigned long arrivalButtonLightLastFlashed = 0;
bool arrivalButtonLightState = false;
bool forceArrival = false;

// Rear reds function variables
int           rearRedsButton[]          = {8, 45};
bool          rearRedsActive            = false;
int           rearRedsOutput            = 48;
unsigned long rearRedsLastStateChange   = 0;
bool          rearRedsPatternChangeLive = false;
bool          rearRedsButtonState       = true;

// Variables for output pins.
int leftHeadlight  = 12;
int rightHeadlight = 13;

// Time variables.
unsigned long lastUserAction = 0;

// Alternating headlight variables.
bool alternatingHeadlightsEnabled = false;
int alternatingHeadlightsLamp = 0; // 0 = left; 1 = right;
unsigned long alternatingHeadlightsLightLastFlash = 0;
bool alternatingHeadlightsLight = false;

// Siren variables.
bool sirenIsPrimed = false;
int sirenTone = 0;          // 0 = off; 1 = Siren; 2 =hi-lo; 3 = wail-1 up; 4 = wail-1 down; 5= wail-2 up; 6=wail-2 down;
//bool sirenIsPrimed = true;
//int sirenTone = 3;
int currentSirenNote = 500; // The note that the siren is currently playing.
int sirenSwingDirection = 1; // 1 = up, 2 = dn
bool sirenLight = false;
unsigned long sirenButtonLastFlash = 0;
unsigned long sirenHoldStarted = 0;

// 999 Mode Variables.
bool tripleNineLight = false;
unsigned long tripleNineButtonLastFlash = 0;

// These are the variables that are used for the siren sounds.
// They are in format [ lower tone limit, upper tone limit, tone step, delay between each tone ]
int sirenToneOneConfiguration[4] = {500, 3000, 5, 4};
int sirenToneTwoConfiguration[4] = {500, 3000, 50, 4};
int sirenToneThreeConfiguration[4] = {250, 750, 50, 1000};
int sirenToneFourConfiguration [4] = {240, 750, 15, 4};
int sirenToneFiveConfiguration [4] = {135, 660, 15, 2};

// grill strobe outputs
int grillStrobeA       = 18;
int grillStrobeB       = 19;
int grillStrobeC       = 20;
int grillStrobeD       = 21;
bool grillStrobeOn     = false;
int grillStrobeTimer   = 0;
unsigned long lastStrobedAt = 0;
bool grillStrobeStatus[4]   = { false, false, false, false };
bool grillStrobeAllowed[4]  = { false, false, false, false };
unsigned long lastSequenced = 0;
unsigned long lastPatternChange = 0;
int grillStrobePattern = 1;
bool forceGrillLightsChange = false;
int numberOfGrillStrobePatterns = 4;
unsigned long grillStrobeButtonLightLastFlashed = 0;
bool grillStrobeButtonLightState = false;

// 999 Mode
bool tripleNineModeActive = false;

// Roof bar data
bool roofBar = false;
bool changeRoofBarState = false;

void setup() {
  pinMode( 45, OUTPUT );
  digitalWrite( 45, HIGH );

  // Initialise the serial connection ( for monitoring );
  Serial.begin( 38400 );
  
  // Configure the input pins.
  pinMode( tripleNineButton[0], INPUT );
  pinMode( alternatingHeadlightsButton[0], INPUT );
  pinMode( sirenButton[0],   INPUT );
  pinMode( arrivalButton[0], INPUT );
  pinMode( rearRedsButton[0], INPUT );
  pinMode( ambersButton[0], INPUT );
  pinMode( hornButtonPin, INPUT );

  // Configure the output pins.
  pinMode( sirenButton[1],       OUTPUT );
  pinMode( grillStrobeButton[1], OUTPUT );
  pinMode( arrivalButton[1], OUTPUT );
  pinMode( tripleNineButton[1], OUTPUT );
  pinMode( alternatingHeadlightsButton[1], OUTPUT );
  pinMode( ambersButton[1], OUTPUT );
  
  pinMode( leftHeadlight,  OUTPUT );
  pinMode( rightHeadlight, OUTPUT );
  pinMode( sirenOutputPin, OUTPUT );
  pinMode( grillStrobeA, OUTPUT );
  pinMode( grillStrobeB, OUTPUT );
  pinMode( grillStrobeC, OUTPUT );
  pinMode( grillStrobeD, OUTPUT );
  pinMode( rearRedsOutput, OUTPUT );
  pinMode( rearRedsButton[1], OUTPUT );

  // Let the grill strobes LED light up solidly
  digitalWrite( grillStrobeButton[1], HIGH );
  grillStrobeButtonLightState = true;

  // Setup siren.
  digitalWrite(sirenOutputPin, HIGH);

  // Light up the arrival button light
  digitalWrite( arrivalButton[1], HIGH );
  arrivalButtonLightState = true;
/**  
int arrivalButtonLight = 43;
unsigned long arrivalButtonLightLastFlashed = 0;
bool arrivalButtonLightState = false;
**/
  // Ensure that the roof light bar starts in a switched off state.
  amberLightBar.flashFunction_02();
  delay(3000);
  amberLightBar.flashFunction_00();

  // Log a message to console to say ready.
  Serial.println("System set up complete. Ready to go.");
}

void ambersControlFunction(){
    if(!digitalRead( ambersButton[0] ) )return;
    
    if( millis() - lastUserAction >= 250 ){      
      if( roofBar ){
        amberLightBar.flashFunction_00();
      } else {
        amberLightBar.flashFunction_01();
      }

      roofBar = !roofBar;
    }

    lastUserAction = millis();
}

void ambersButtonFunction(){
  digitalWrite( ambersButton[1], roofBar ? HIGH : LOW );
}

// Function which turns the rear reds on or off
void rearRedsControlFunction(){  
  // If the button is not pushed, don't need to check any further.
  if( !digitalRead( rearRedsButton[0] ) )return;

  if( millis() - rearRedsLastStateChange >= 250 ){
    digitalWrite( rearRedsOutput, !rearRedsActive );
    rearRedsActive = !rearRedsActive;
  
    Serial.println("rear reds state change");
  
    // If this is the second rapid push, turn off the rear reds.
  }
  
  digitalWrite( rearRedsButton[1], HIGH );
  
  // Record this as a user action, and also update the rear reds last state change counter.
  lastUserAction = rearRedsLastStateChange = millis();
}

void rearRedsButtonFlashFunction(){
  if( !rearRedsActive ) return;
  digitalWrite( rearRedsButton[1], rearRedsButtonState ? HIGH : LOW );
  rearRedsButtonState = !rearRedsButtonState;
}

// Function which detects input on button for alternating headlights and turns them on or off.
void alternatingHeadlightsControlFunction(){
    // If the button is pressed, and it's been more than a second since the last user action, change the state.
    if( digitalRead( alternatingHeadlightsButton[0] ) == HIGH && ( ( millis() - lastUserAction ) >= 1000 ) ){
       // The button is pressed and we need to change alternating headlamps state to either on or off (opposite what they are now)
       alternatingHeadlightsEnabled = !alternatingHeadlightsEnabled;

       // Record this as a user action.
       lastUserAction = millis();

       // Print a line to the serial connection to show that the state has changed.
       if( alternatingHeadlightsEnabled ){ Serial.println( "Alternating Headlights Switched On." ); } else { Serial.println( "Alternating Headlights Switched Off." ); }
    }

    // indicator LED
    alternatingHeadlightsFlashLED();
}

void alternatingHeadlightsFunction(){
    // If alternating headlights are not enabled, make sure both relays are off.
    if( !alternatingHeadlightsEnabled ){
        digitalWrite( leftHeadlight, LOW );
        digitalWrite( rightHeadlight, LOW );
        return;
    }

    // Switch the headlights between left and right.
    switch( alternatingHeadlightsLamp ){
      case 0:
        // Left lamp.
        digitalWrite( leftHeadlight, HIGH );
        digitalWrite( rightHeadlight, LOW );
        alternatingHeadlightsLamp = 1;
        break;
      case 1:
        // Right lamp.
        digitalWrite( leftHeadlight,  LOW );
        digitalWrite( rightHeadlight, HIGH );
        alternatingHeadlightsLamp = 0;
        break;
    }
}

void primeSirenFunction(){
  // If the user pressed the prime siren button and it's been more than a second since the last user interaction.
  if( digitalRead( sirenButton[0] ) == HIGH && ( ( millis() - lastUserAction ) >= 1000 ) ){
      // Flip the siren primed flag
      sirenIsPrimed = !sirenIsPrimed;

      // Record this as a user action.
      lastUserAction = millis();

      // Log in the serial box for debugging reasons.
      if( sirenIsPrimed ){ Serial.println( "Siren is primed." ); } else { Serial.println( "Siren is not primed." ); }
  }

  if( sirenTone == 0 && sirenIsPrimed ){
    digitalWrite( sirenButton[1], HIGH);
    sirenLight = true;
  } else {
    if( sirenIsPrimed && ( millis() - sirenButtonLastFlash >= 1000 ) ){
      if( sirenLight ){
        digitalWrite( sirenButton[1], LOW );
        sirenLight = false;
      } else {
        digitalWrite( sirenButton[1], HIGH);
        sirenLight = true;
      }

      sirenButtonLastFlash = millis();
    }

    if( sirenLight && !sirenIsPrimed ){
      digitalWrite( sirenButton[1], LOW );
      sirenLight = false;
    }
  }
}

void sirenToneOneFunction(){
  if( sirenTone == 1 ){
    // Play the current siren tone note.
    tone( sirenOutputPin, currentSirenNote );

    // If the siren tone is at its lowest level, switch so that it will start to head to the upper level.
    if( currentSirenNote == sirenToneOneConfiguration[0] ){
      sirenSwingDirection = 1;

      // If not currently holding the siren at its lowest point, set the timer for that here.
      if( sirenHoldStarted == 0 )sirenHoldStarted = millis();
    }

    // If the siren tone is at its highest level, switch so that it will start to head to the lower level.
    if( currentSirenNote == sirenToneOneConfiguration[1] ){
      sirenSwingDirection = 2;

      // If not currently holding the siren at its highest point, set the timer for that here.
      if( sirenHoldStarted == 0 )sirenHoldStarted = millis();
    }

    // Increase or decrease the current siren tone note.
    if(sirenSwingDirection == 1){
      // If the sound is at its lowest note, just hold it there for 100ms.
      if( ( currentSirenNote == sirenToneOneConfiguration[0] ) && ( ( millis() - sirenHoldStarted ) < 100 ) )return;
      
      currentSirenNote += sirenToneOneConfiguration[2];
    } else {
      // If the sound is at its highest note, just hold it there for 100ms.
      if( ( currentSirenNote == sirenToneOneConfiguration[1] ) && ( ( millis() - sirenHoldStarted ) < 100 ) )return;
      
      currentSirenNote -= sirenToneOneConfiguration[2];
    }

    // Reset the siren hold timer.
    sirenHoldStarted = 0;
  }  
}

void sirenToneTwoFunction(){
  if( sirenTone == 2 ){
    // Play the current siren tone note.
    tone( sirenOutputPin, currentSirenNote);

    // If the siren tone is at its limit, flip the direction it is travelling.
    if( currentSirenNote == sirenToneTwoConfiguration[0] )sirenSwingDirection = 1;
    if( currentSirenNote == sirenToneTwoConfiguration[1] )sirenSwingDirection = 2;

    // Increase or decrease the current siren tone note.
    if(sirenSwingDirection == 1){
      currentSirenNote += sirenToneTwoConfiguration[2];
    } else {
      currentSirenNote -= sirenToneTwoConfiguration[2];
    }
  }  
}

void sirenToneThreeFunction(){
  if( sirenTone == 3 ){
    // Play the current siren tone note.
    tone( sirenOutputPin, currentSirenNote);

    // If the siren tone is at its limit, flip the direction it is travelling.
    if( currentSirenNote == sirenToneThreeConfiguration[0] )sirenSwingDirection = 1;
    if( currentSirenNote == sirenToneThreeConfiguration[1] )sirenSwingDirection = 2;

    // Increase or decrease the current siren tone note.
    if(sirenSwingDirection == 1){
      currentSirenNote = sirenToneThreeConfiguration[1];
    } else {
      currentSirenNote = sirenToneThreeConfiguration[0];
    }
  }  
}

void sirenToneFourFunction(){
  if( sirenTone == 4 ){
    // Siren always swings downwards.
    sirenSwingDirection = 2;
    
    // Play the current siren tone note
    tone( sirenOutputPin, currentSirenNote);

    // If at the min note for this sound, go back to the start.
    if( currentSirenNote == sirenToneFourConfiguration[0] ){
      currentSirenNote = sirenToneFourConfiguration[1];
    } else {
      // Decrease by 15 for the next note
      currentSirenNote -= sirenToneFourConfiguration[2];
    }
  }
}

void sirenToneFiveFunction(){
  if( sirenTone == 5 ){
    // Siren always swings downwards.
    sirenSwingDirection = 2;
    
    // Play the current siren tone note
    tone( sirenOutputPin, currentSirenNote);

    // If at the min note for this sound, go back to the start.
    if( currentSirenNote == sirenToneFiveConfiguration[0] ){
      currentSirenNote = sirenToneFiveConfiguration[1];
    } else {
      // Decrease by 15 for the next note
      currentSirenNote -= sirenToneFiveConfiguration[2];
    }
  }
}

void sirenChangeToneFunction(){      
  // If the horn is pressed and it has been more than a second since last user action, change siren tone
  if( ( digitalRead( hornButtonPin ) == HIGH && ( ( millis() - lastUserAction ) >= 1000 ) ) ){
      // If the siren is not primed, do nothing.
      if( !sirenIsPrimed ){
        Serial.println("Siren was not activated as you have not pressed the prime siren button.");
         bool overrideButtonActionStart = false ;
         
         // Return. Do not change the siren tone.
        return;
      }

    // If sirenTone is already on the max, reset it to 0 so it becomes 1.
    if( sirenTone == 6 )sirenTone = 0;

    // Clear any pending siren holds.
    sirenHoldStarted = 0;
    
    // Change the tone of the siren.
    sirenTone++;

    // Reset the siren note to the default, which is 500.
    switch(sirenTone){
      case 1:
        currentSirenNote=sirenToneOneConfiguration[0];
        break;
      case 2:
        currentSirenNote=sirenToneTwoConfiguration[0];
        break;
      case 3:
        currentSirenNote=sirenToneThreeConfiguration[1];
        // Start playing the siren tone, because it could be a second until the next loop for siren patterns.
        sirenToneThreeFunction();
        break;
      case 4:
        currentSirenNote=sirenToneFourConfiguration[1];
        break;
      case 5:
        currentSirenNote=sirenToneFiveConfiguration[1];
        break;
      default:
        // Go back to position 1.
        sirenTone = 0;
        currentSirenNote=sirenToneOneConfiguration[0];
        sirenChangeToneFunction();
        return;
    }

    // Turn on the indicator light.
    digitalWrite( sirenButton[1], HIGH);

    // Siren always swings up first.
    sirenSwingDirection = 1;

    // Print the new siren tone (no siren connected yet -- DEBUG ONLY )
    String prelude    = "Siren tone changed. Tone is now tone: " ;
    String theMessage = prelude + sirenTone;
    Serial.println( theMessage );

    // Record this as a user action
    lastUserAction = millis();
  }

 // if it's been more than 500ms since last user action, but less than 750ms, shut off the siren.
 // also automatically shut off the siren if it is no longer primed.
 if( ( digitalRead( hornButtonPin ) == HIGH && ( ( millis() - lastUserAction ) >= 250 ) && ( millis() - lastUserAction ) <= 750  )|| ( !sirenIsPrimed && sirenTone != 0  )){
   muteSiren();
 }
}

void muteSiren(){
  // Change the tone of the siren.
   sirenTone = 0;

   // Make sure the siren pin is not doing a tone.
   noTone(sirenOutputPin);
  
   // Print the new siren tone (no siren connected yet -- DEBUG ONLY )
   Serial.println( "Siren is now switched off." );

   // Turn off the indicator light
   digitalWrite( sirenButton[1], LOW);

   // Record this as a user action
    lastUserAction = millis();
}

void arrivalFunction(){
  // If the arrival button is pressed and it's more than a second since last user action
  if( digitalRead( arrivalButton[0] ) == HIGH && ( millis() - lastUserAction >= 1000 ) || forceArrival ){
    // Record this as a user interaction
    lastUserAction = millis();

    // Mute the siren.
    muteSiren();

    // Turn off the alternating headlights.
    alternatingHeadlightsEnabled = false;

    // If it is on, turn off the grill strobes;
    if( grillStrobeOn )forceGrillLightsChange = true;

    // Ensure that the 999 mode flag is de-activated so the button can be used again.
    tripleNineModeActive = false;

    roofBar = false;
    changeRoofBarState = true;

    // Change forceArrival to false to avoid re-triggering this if it was a forced trigger
    forceArrival = false;

    // Print log message.
    Serial.println("Arrival activated. Everything switched off.");
  }
}

void tripleNineFunction(){
  // If user has pressed the 999 mode button and it's been more than a second since the last user interaction
  if( digitalRead( tripleNineButton[0] ) == HIGH && ( millis() - lastUserAction >= 1000 ) ){
    // Record this as a user interaction
    lastUserAction = millis();
    
    if( tripleNineModeActive ){
      Serial.println( "999 Mode is already activated.");
      return;
    }

    // Log to say 999 mode activated.
    Serial.println("999 Mode activated.");

    // Prime the siren and set tone to 1 so that it starts straight away.
    if( !sirenIsPrimed )sirenTone = 0;
    sirenIsPrimed = true;

    // Force the grill lights to come on if they are not already on, also force them to pattern 0 so that they use pattern 1 when they come on.
    if( !grillStrobeOn ){
      forceGrillLightsChange = true;
      grillStrobePattern     = 0;
    }
    
    // Enable alternating headlights.
    alternatingHeadlightsEnabled = true;

    // Set 999 mode flag
    tripleNineModeActive = true;

    roofBar = true;
    changeRoofBarState = true;
  }

  tripleNineFlashLED();
}

void checkFlash(){
  if( roofBar && changeRoofBarState ){
    changeRoofBarState = false;
    
    // Turn on the amber roof lights
    amberLightBar.flashFunction_13();
  } else {
    if( changeRoofBarState ){
      changeRoofBarState = false;
      
      // Turn off the amber roof bar.
      amberLightBar.flashFunction_00();
    }
  }
}

void grillLightsOnOffFunction(){  
    // Single push when the grill strobes are off will turn them on
    if(
      digitalRead( grillStrobeButton[0] ) == HIGH &&
      ( millis() - lastUserAction >= 1000 )    ||
      forceGrillLightsChange                   &&
      !grillStrobeOn
    ){
      // If the grill strobe is not on, it needs to be on.
      if( !grillStrobeOn ){
        // The grill strobe pattern cannot be 0 it must be 1.
        if( grillStrobePattern == 0 )grillStrobePattern++;
        
        String pretext = "Grill strobes on. Pattern: ";
        Serial.println(pretext + grillStrobePattern);
        
        digitalWrite( grillStrobeA, HIGH );
        digitalWrite( grillStrobeB, HIGH );
        digitalWrite( grillStrobeC, HIGH );
        digitalWrite( grillStrobeD, HIGH );
  
        grillStrobeOn = true;
  
        // Make force grill lights active off to avoid re-trigger.
        forceGrillLightsChange = false;
      } else {
        // The grill strobe is on, so we need to change the pattern.        
        Serial.println("Changing Patterns...");
    
        // If at max pattern, reset to zero
        if( grillStrobePattern==numberOfGrillStrobePatterns )grillStrobePattern=0;
    
        // Increment pattern to 1
        grillStrobePattern++;
    
        // Store the time of the pattern change.
        lastPatternChange = millis();
    
        // Display the new pattern in console.
        String pretext = "The new pattern is: ";
        Serial.println( pretext + grillStrobePattern );
    
        // Mute all strobes to clear for the new pattern
        muteAllStrobes();
      }

      // Reset LED sequenced time
      lastSequenced = 0;
    }

    // Double tapping the switch when the grill strobes are on will turn the grill strobes off.
    if(
      digitalRead( grillStrobeButton[0] ) == HIGH &&
      ( millis() - lastUserAction >= 100 )     &&
      ( millis() - lastUserAction <= 750 )     &&
      grillStrobeOn                            ||
      forceGrillLightsChange                   &&
      grillStrobeOn
    ){
      // Then turn the grill strobes off.
      Serial.println("Grill strobes off.");
      digitalWrite( grillStrobeA, LOW );
      digitalWrite( grillStrobeB, LOW );
      digitalWrite( grillStrobeC, LOW );
      digitalWrite( grillStrobeD, LOW );

      grillStrobeOn = false;

      // If this is the result of a double click action, grill strobes need to go back to the last pattern, as they got moved forward on the first click.
      if(( millis() - lastUserAction >= 100 ) && ( millis() - lastUserAction <= 750 )){
        grillStrobePattern--;
        if( grillStrobePattern == 0 )grillStrobePattern = numberOfGrillStrobePatterns;
      }

      // Store the time of the pattern change.
      lastPatternChange = millis();

      // Make force grill lights active off to avoid re-trigger.
      forceGrillLightsChange = false;

      // Let the grill strobes LED light up solidly
      digitalWrite( grillStrobeButton[1], HIGH );
    }

    // Record the user action
    if( digitalRead( grillStrobeButton[0] ) == HIGH )lastUserAction = millis();

    // If the grill strobes are on, and it's been more than a second since the button LED last changed state, change the state now.
    if( grillStrobeOn ){
      if( millis() - grillStrobeButtonLightLastFlashed >= 1000 ){
        grillStrobeButtonLightState = !grillStrobeButtonLightState;
        digitalWrite( grillStrobeButton[1], grillStrobeButtonLightState);
        grillStrobeButtonLightLastFlashed = millis();
      }
    }
}

void grillLightsStrobeFunction(){
  // Only strobe if the grill lights are actually switched on.
  if( !grillStrobeOn )return;

  // Only strobe if haven't strobed in the last x seconds.
  if( millis() - lastStrobedAt < grillStrobeTimer )return;

  // Strobe the lights
  for( int s=0; s<sizeof(grillStrobeStatus); s++ ){
    int pin = 0;
        
    switch( s ){
      case 0:
        pin = grillStrobeA;
        break;
      case 1:
        pin = grillStrobeB;
        break;
      case 2:
        pin = grillStrobeC;
        break;
      case 3:
        pin = grillStrobeD;
        break;
      default:
        return;
    }

    if( grillStrobeStatus[ s ] && grillStrobeTimer > 0){
      digitalWrite( pin, LOW );
    } else {
      if( grillStrobeAllowed[s] )digitalWrite( pin, HIGH );
    }

    // Flip the strobe status for the next cycle.
    if( grillStrobeTimer > 0 )grillStrobeStatus[s] = !grillStrobeStatus[s];

    lastStrobedAt = millis();
  }
}

void muteAllStrobes(){
  for( int i=0; i<sizeof( grillStrobeAllowed ); i++ ){
    grillStrobeAllowed[i] = false;
  }
}

void grillStrobePatternOneFunction(){
  if( grillStrobePattern != 1 )return;

  // If this is the first sequence, need to set the start arrangement
  if( lastSequenced == 0 ){
    grillStrobeAllowed[0] = true;
    grillStrobeAllowed[1] = false;
    grillStrobeAllowed[2] = true;
    grillStrobeAllowed[3] = false;
  }

  if( millis() - lastSequenced >= 500 || lastSequenced == 0 ){
    // Flip which LEDs are illuminated.
    grillStrobeAllowed[0] = !grillStrobeAllowed[0];
    grillStrobeAllowed[1] = !grillStrobeAllowed[1];
    grillStrobeAllowed[2] = !grillStrobeAllowed[2];
    grillStrobeAllowed[3] = !grillStrobeAllowed[3];
  
    // Set the strobe timer
    grillStrobeTimer = 25;
    
    // Store the time we last changed the LEDs
    lastSequenced=millis();
  }
}

void grillStrobePatternTwoFunction(){
  if( grillStrobePattern != 2 )return;

  // If this is the first sequence, need to set the start arrangement
  if( lastSequenced == 0 ){
    grillStrobeAllowed[0] = true;
    grillStrobeAllowed[1] = true;
    grillStrobeAllowed[2] = true;
    grillStrobeAllowed[3] = true;
  }

  if( millis() - lastSequenced >= 500  || lastSequenced == 0 ){

    // Flip which LEDs are illuminated.
    grillStrobeAllowed[0] = !grillStrobeAllowed[0];
    grillStrobeAllowed[1] = !grillStrobeAllowed[1];
    grillStrobeAllowed[2] = !grillStrobeAllowed[2];
    grillStrobeAllowed[3] = !grillStrobeAllowed[3];
  
    // Set the strobe timer
    grillStrobeTimer = 25;
    
    // Store the time we last changed the LEDs
    lastSequenced=millis();

  }
}

void grillStrobePatternThreeFunction(){
  if( grillStrobePattern != 3 )return;

  // If this is the first sequence, need to set the start arrangement
  if( lastSequenced == 0 ){
    grillStrobeAllowed[0] = true;
    grillStrobeAllowed[1] = true;
    grillStrobeAllowed[2] = true;
    grillStrobeAllowed[3] = true;
  
    // Set the strobe timer
    grillStrobeTimer = 500;
    
    // Store the time we last changed the LEDs
    lastSequenced=millis();

  }
}

void alternatingHeadlightsFlashLED(){
  if( alternatingHeadlightsEnabled ){
    // Only flashs if flashed a second ago
    if( millis() - alternatingHeadlightsLightLastFlash < 1000 )return;

    // Emit a flashing light when active
    digitalWrite( alternatingHeadlightsButton[1], !alternatingHeadlightsLight );

    // Make sure to flash the opposite state next time.
    alternatingHeadlightsLight = !alternatingHeadlightsLight;

    // Store last flash.
    alternatingHeadlightsLightLastFlash = millis();
  } else {
    // Just emit a steady light on the button pin.
    digitalWrite( alternatingHeadlightsButton[1], HIGH );
    if( !alternatingHeadlightsLight )alternatingHeadlightsLight = true;
  }
}

void tripleNineFlashLED(){
  // If the triple nine mode is active, flash the LED for the 999 button.
  if( tripleNineModeActive ){
    // Only flash if last flashed more than a second ago.
    if( millis() - tripleNineButtonLastFlash < 1000 )return;
    
    // Emit a flashing light to the button when active.
    digitalWrite( tripleNineButton[1], !tripleNineLight );

    // Make sure to use the opposite state next time this function runs.
    tripleNineLight = !tripleNineLight;

    // Update the time that the button was last flashed.
    tripleNineButtonLastFlash = millis();
  } else {
    // Just emit a steady light on that button.
    digitalWrite( tripleNineButton[1], HIGH );
    if(!tripleNineLight)tripleNineLight = true;
  }
}

void grillStrobePatternFourFunction(){
  if( grillStrobePattern != 4 )return;

  // If this is the first sequence, need to set the start arrangement
  grillStrobeAllowed[0] = true;
  grillStrobeAllowed[1] = true;
  grillStrobeAllowed[2] = true;
  grillStrobeAllowed[3] = true;

  // Set the strobe timer
  grillStrobeTimer = 0;
  
  // Store the time we last changed the LEDs
  lastSequenced=millis();
}

// Set up timed tasks.
TimedAction alternatingHeadlightsControl = TimedAction(50,  alternatingHeadlightsControlFunction);
TimedAction alternatingHeadlightsAction  = TimedAction(500, alternatingHeadlightsFunction);
TimedAction primeSirenControl            = TimedAction(50,  primeSirenFunction);
TimedAction sirenToneControl             = TimedAction(50,  sirenChangeToneFunction);
TimedAction arrivalControl               = TimedAction(50,  arrivalFunction);
TimedAction tripleNineControl            = TimedAction(50,  tripleNineFunction);
TimedAction grillStrobeControl           = TimedAction(50,  grillLightsOnOffFunction);
TimedAction grillStrobeFlash             = TimedAction(25,  grillLightsStrobeFunction);
TimedAction ambersControl                = TimedAction( 50, ambersControlFunction );
TimedAction ambersButtonState            = TimedAction(50, ambersButtonFunction);

// Rear reds control
TimedAction rearRedsControl              = TimedAction( 50, rearRedsControlFunction );
TimedAction rearRedsButtonFlash          = TimedAction( 1000, rearRedsButtonFlashFunction );

// Grill strobe patterns.
TimedAction grillStrobePatternOne        = TimedAction(50, grillStrobePatternOneFunction );
TimedAction grillStrobePatternTwo        = TimedAction(50, grillStrobePatternTwoFunction );
TimedAction grillStrobePatternThree      = TimedAction(50, grillStrobePatternThreeFunction );
TimedAction grillStrobePatternFour       = TimedAction(50, grillStrobePatternFourFunction );

// Siren sound timed tasks.
TimedAction sirenToneOneAudio            = TimedAction(sirenToneOneConfiguration[3], sirenToneOneFunction);
TimedAction sirenToneTwoAudio            = TimedAction(sirenToneTwoConfiguration[3], sirenToneTwoFunction);
TimedAction sirenToneThreeAudio          = TimedAction(sirenToneThreeConfiguration[3], sirenToneThreeFunction);
TimedAction sirenToneFourAudio           = TimedAction(sirenToneFourConfiguration[3], sirenToneFourFunction);
TimedAction sirenToneFiveAudio           = TimedAction(sirenToneFiveConfiguration[3], sirenToneFiveFunction);

TimedAction changeRoofBarStateFunction           = TimedAction(500, checkFlash);
void loop() {
  // Alternating headlights functions.
  alternatingHeadlightsControl.check();    // The function that detects if alternating headlights need to be turned on or off.
  alternatingHeadlightsAction.check();     // The function which alternates the headlights between left and right.

  // siren functions.
  primeSirenControl.check();  // The function that detects when the prime siren button is pressed.
  sirenToneControl.check();   // Will detect presses to the horn, and if the siren is primed, activate it at tone 1.

  // Rear reds functions
  rearRedsControl.check();
  rearRedsButtonFlash.check();

  // siren sounds.
  sirenToneOneAudio.check();   // Run function for siren tone one.
  sirenToneTwoAudio.check();   // Run function for siren tone two.
  sirenToneThreeAudio.check(); // Run function for siren tone three.
  sirenToneFourAudio.check();  // Run function for siren tone four.
  sirenToneFiveAudio.check();  // Run function for siren tone five.

  // grill lights
  grillStrobeControl.check();
  grillStrobeFlash.check();

  // grill lights patterns
  grillStrobePatternOne.check();
  grillStrobePatternTwo.check();
  grillStrobePatternThree.check();
  grillStrobePatternFour.check();

  ambersControl.check();
  ambersButtonState.check();

  // arrival functions.
  arrivalControl.check();      // Function that detects if the arrival button is pressed. This will shut everything off.

  // 999 mode functions.
  tripleNineControl.check();   // Function that detects if the 999 mode button is pressed. This will trigger everything.

  changeRoofBarStateFunction.check();
}
