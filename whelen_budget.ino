#include <TimedAction.h>
#include <SoftwareSerial.h>

// Variables for input pins.
int alternatingHeadlightsButtonPin = 2;    // The button that activates or deactivates the alternating headlamps.
int sirenPrimeButtonPin            = 3;    // The button that primes and un-primes the siren.
int hornButtonPin                  = 4;    // Connects to vehicle horn and controls changing of the siren tones when the siren is primed.
int arrivalButtonPin               = 5;    // This button will deactivate everything.
int tripleNineButtonPin            = 6;    // Turns the system to 999 mode (everything on)
int sirenOutputPin                 = 7;    // The pin which outputs the note for the siren.

// Variables for output pins.
int leftHeadlight  = 12;
int rightHeadlight = 13;

// Time variables.
unsigned long lastUserAction = 0;

// Alternating headlight variables.
bool alternatingHeadlightsEnabled = false;
int alternatingHeadlightsLamp = 0; // 0 = left; 1 = right;

// Siren variables.
bool sirenIsPrimed = false;
int sirenTone = 0;          // 0 = off; 1 = Siren; 2 =hi-lo; 3 = wail-1 up; 4 = wail-1 down; 5= wail-2 up; 6=wail-2 down;
int currentSirenNote = 500; // The note that the siren is currently playing.
int sirenSwingDirection = 1; // 1 = up, 2 = dn
unsigned long sirenHoldStarted = 0;

// These are the variables that are used for the siren sounds.
// They are in format [ lower tone limit, upper tone limit, tone step, delay between each tone ]
int sirenToneOneConfiguration[4] = {500, 3000, 5, 4};
int sirenToneTwoConfiguration[4] = {500, 3000, 50, 4};
int sirenToneThreeConfiguration[4] = {250, 750, 50, 1000};
int sirenToneFourConfiguration [4] = {240, 750, 15, 4};
int sirenToneFiveConfiguration [4] = {135, 660, 15, 2};

// 999 Mode
bool tripleNineModeActive = false;

void setup() {
  // Initialise the serial connection ( for monitoring );
  Serial.begin( 38400 );
  
  // Configure the input pins.
  pinMode( alternatingHeadlightsButtonPin, INPUT );
  pinMode( sirenPrimeButtonPin, INPUT );
  pinMode( hornButtonPin, INPUT );
  pinMode( arrivalButtonPin, INPUT );

  // Configure the output pins.
  pinMode( leftHeadlight,  OUTPUT );
  pinMode( rightHeadlight, OUTPUT );
  pinMode( sirenOutputPin, OUTPUT );

  // Setup siren
  digitalWrite(sirenOutputPin, HIGH);
}

// Function which detects input on button for alternating headlights and turns them on or off.
void alternatingHeadlightsControlFunction(){
    // If the button is pressed, and it's been more than a second since the last user action, change the state.
    if( digitalRead( alternatingHeadlightsButtonPin ) == HIGH && ( ( millis() - lastUserAction ) >= 1000 ) ){
       // The button is pressed and we need to change alternating headlamps state to either on or off (opposite what they are now)
       alternatingHeadlightsEnabled = !alternatingHeadlightsEnabled;

       // Record this as a user action.
       lastUserAction = millis();

       // Print a line to the serial connection to show that the state has changed.
       if( alternatingHeadlightsEnabled ){ Serial.println( "Alternating Headlights Switched On." ); } else { Serial.println( "Alternating Headlights Switched Off." ); }
    }
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
  if( digitalRead( sirenPrimeButtonPin ) == HIGH && ( ( millis() - lastUserAction ) >= 1000 ) ){
      // Flip the siren primed flag
      sirenIsPrimed = !sirenIsPrimed;

      // Record this as a user action.
      lastUserAction = millis();

      // Log in the serial box for debugging reasons.
      if( sirenIsPrimed ){ Serial.println( "Siren is primed." ); } else { Serial.println( "Siren is not primed." ); }
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
  if( digitalRead( hornButtonPin ) == HIGH && ( ( millis() - lastUserAction ) >= 1000 )){
      // If the siren is not primed, do nothing.
      if( !sirenIsPrimed ){
        Serial.println("Siren was not activated as you have not pressed the prime siren button.");
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
        currentSirenNote=sirenToneThreeConfiguration[0];
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
   // Change the tone of the siren.
   sirenTone = 0;

   // Record this as a user action
   lastUserAction = millis();

   // Make sure the siren pin is not doing a tone.
   noTone(sirenOutputPin);
  
   // Print the new siren tone (no siren connected yet -- DEBUG ONLY )
   Serial.println( "Siren is now switched off." );
 }
}

void arrivalFunction(){
  // If the arrival button is pressed and it's more than a second since last user action
  if( digitalRead( arrivalButtonPin ) == HIGH && ( millis() - lastUserAction >= 1000 ) ){
    // Record this as a user interaction
    lastUserAction = millis();

    // Un-prime the siren. This will de-activate it and prevent triggering from the horn, which reverts to normal operation.
    sirenIsPrimed = false;

    // Turn off the alternating headlights.
    alternatingHeadlightsEnabled = false;

    // Ensure that the 999 mode flag is de-activated so the button can be used again.
    tripleNineModeActive = false;

    // Print log message.
    Serial.println("Arrival activated. Everything switched off.");
  }
}

void tripleNineFunction(){
  // If user has pressed the 999 mode button and it's been more than a second since the last user interaction
  if( digitalRead( tripleNineButtonPin ) == HIGH && ( millis() - lastUserAction >= 1000 ) ){
    // Record this as a user interaction
    lastUserAction = millis();
    
    if( tripleNineModeActive ){
      Serial.println( "999 Mode is already activated.");
      return;
    }

    // Log to say 999 mode activated.
    Serial.println("999 Mode activated.");

    // Prime the siren and set tone to 1 so that it starts straight away.
    sirenIsPrimed = true;
    sirenTone = 1;
    sirenChangeToneFunction();
    
    // Enable alternating headlights.
    alternatingHeadlightsEnabled = true;

    // Set 999 mode flag
    tripleNineModeActive = true;
  }
}

// Set up timed tasks.
TimedAction alternatingHeadlightsControl = TimedAction(50,  alternatingHeadlightsControlFunction);
TimedAction alternatingHeadlightsAction  = TimedAction(500, alternatingHeadlightsFunction);
TimedAction primeSirenControl            = TimedAction(50,  primeSirenFunction);
TimedAction sirenToneControl             = TimedAction(50,  sirenChangeToneFunction);
TimedAction arrivalControl               = TimedAction(50,  arrivalFunction);
TimedAction tripleNineControl            = TimedAction(50,  tripleNineFunction);

// Siren sound timed tasks.
TimedAction sirenToneOneAudio            = TimedAction(sirenToneOneConfiguration[3], sirenToneOneFunction);
TimedAction sirenToneTwoAudio            = TimedAction(sirenToneTwoConfiguration[3], sirenToneTwoFunction);
TimedAction sirenToneThreeAudio          = TimedAction(sirenToneThreeConfiguration[3], sirenToneThreeFunction);
TimedAction sirenToneFourAudio           = TimedAction(sirenToneFourConfiguration[3], sirenToneFourFunction);
TimedAction sirenToneFiveAudio           = TimedAction(sirenToneFiveConfiguration[3], sirenToneFiveFunction);

void loop() {
  // Alternating headlights functions.
  alternatingHeadlightsControl.check();    // The function that detects if alternating headlights need to be turned on or off.
  alternatingHeadlightsAction.check();     // The function which alternates the headlights between left and right.

  // siren functions.
  primeSirenControl.check();  // The function that detects when the prime siren button is pressed.
  sirenToneControl.check();   // Will detect presses to the horn, and if the siren is primed, activate it at tone 1.

  // siren sounds.
  sirenToneOneAudio.check();   // Run function for siren tone one.
  sirenToneTwoAudio.check();   // Run function for siren tone two.
  sirenToneThreeAudio.check(); // Run function for siren tone three.
  sirenToneFourAudio.check();  // Run function for siren tone four.
  sirenToneFiveAudio.check();  // Run function for siren tone five.

  // arrival functions.
  arrivalControl.check();      // Function that detects if the arrival button is pressed. This will shut everything off.

  // 999 mode functions.
  tripleNineControl.check();   // Function that detects if the 999 mode button is pressed. This will trigger everything.
}