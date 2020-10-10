/*
  Serial Relay - Arduino Sketch

  (C) 2020 Oliver Westermann
*/


/* Settings */
//#define DEBUG
#undef DEBUG
#define ACTIVE_POL    LOW
#define INACTIVE_POL  HIGH


/* Debug print helpers */
#ifdef DEBUG
#define debug(...) do{ Serial.print( __VA_ARGS__ ); } while( false )
#define debugln(...) do{ Serial.println( __VA_ARGS__ ); } while( false )
#else
#define debug(...) do{ } while ( false )
#define debugln(...) do{ } while ( false )
#endif


/* special bytes for the protocol, check README.md for details */
const char start_byte = ':';
const char end_byte = ';';


/* channel mapping to Arduino digital outputs
 *   channel_mapping[channel] = digital output
 */
int channel_mapping[] {
  2,
  3,
  4,
  5,
  6,
  7,
  8,
  9
};


typedef enum state {
  WAIT_FOR_START,
  READ_CHANNEL,
  READ_VALUE,
} State;


/**
 * Update a number by "appending" a char
 *
 * Used when a number is provided char-by-char over a serial connection. Append 'new_char'
 * to the int number
 * 
 * @param number   Pointer to int to update
 * @param new_char char to append, must be '0' to '9'
 * @return         0 in case of success
 */
int update_number_by_char(int *number, char new_char) {
  if (new_char >= '0' && new_char <= '9') {
    *number = *number * 10 + (new_char - '0');
    return 0;
  } else {
    return -1;
  }
}


/**
 * Set channel to a value
 *
 * Polarity can be influenced by the ACTIVE_POL/INACTIVE_POL defines
 * 
 * @param channel   Channel to update
 * @param state     must be 1 or 0
 * @return          0 in case of success, -1 for invalid channel, -2 for invalid state
 */
int set_channel(int channel, int state) {
  size_t channel_count = sizeof(channel_mapping)/sizeof(channel_mapping[0]);
  if (channel > channel_count)
    return -1;
  
  if (state == 1) {
    digitalWrite(channel_mapping[channel], ACTIVE_POL);
    return 0;
  } else if (state == 0) {
    digitalWrite(channel_mapping[channel], INACTIVE_POL);
    return 0;
  }
  return -2;
}


/* Arduino setup() */
void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  int i;
  pinMode(LED_BUILTIN, OUTPUT);
  for (i = 2; i <= 9; i++) {
    digitalWrite(i, HIGH);
    pinMode(i, OUTPUT);
  }
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
}


/* Main loop */
void loop() {
  char in;
  static State state = WAIT_FOR_START;
  static int channel = 0, value = 0;

  in = Serial.read();
  if (in > 0) {
    debug("------------------\nGot char: ");
    debugln(in);
    switch (state) {
      case WAIT_FOR_START:
        debugln("Handling WAIT_FOR_START");
        if (in == start_byte) {
          state = READ_CHANNEL;
          channel = 0;
          value = 0;
          debugln("New State: READ_CHANNEL");
        }
        break;
      case READ_CHANNEL:
        debug("Handling for READ_CHANNEL\ncurrent Channel: ");
        debugln(channel);
        if ( in == '=' ) {
          state = READ_VALUE;
          debugln("New State: READ_VALUE");
        } else if (!update_number_by_char(&channel, in)) {
          debug("Updated Channel: ");
          debugln(channel);
        } else {
          debugln("Invalid char, back to WAIT_FOR_START");
          state = WAIT_FOR_START;
          Serial.write("ERR\n");
        }
        break;
      case READ_VALUE:
        debug("Handling for READ_VALUE\nChannel:");
        debugln(channel);
        if (in == '0' || in == '1') {
          value = in - '0';
        } else if (in == end_byte) {
          /* time for some action */
          if (set_channel(channel, value)) {
            debug("set_channel failed for channel ");
            debug(channel);
            debug(", value ");
            debug(channel);
            debugln(", back to WAIT_FOR_START");
            Serial.write("ERR\n");
          } else {
            Serial.write("OK\n");
          }
          state = WAIT_FOR_START;
        } else {
          debugln("Invalid char, back to WAIT_FOR_START");
          state = WAIT_FOR_START;
          Serial.write("ERR\n");
        }
        break;
    }
  }
}
