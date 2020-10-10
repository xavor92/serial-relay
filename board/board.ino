/*
  Serial Relay - Arduino Sketch

  (C) 2020 Oliver Westermann
*/

/* Settings */
#define DEBUG
//#undef DEBUG
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

// the setup function runs once when you press reset or power the board
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

/**
 * @brief Update a number by "appending" a char
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

void set_channel(int channel, int state) {
  if (state)
    digitalWrite(channel_mapping[channel], ACTIVE_POL);
  else
    digitalWrite(channel_mapping[channel], INACTIVE_POL);
}

// the loop function runs over and over again forever
void loop() {
  char in;
  static State state = WAIT_FOR_START;
  static int channel = 0;

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
        }
        break;
      case READ_VALUE:
        debug("Handling for READ_VALUE\nChannel:");
        debugln(channel);
        if (in == end_byte) {
          debugln("READ_CHANNEL: TBD\nNew State: WAIT_FOR_START");
          state = WAIT_FOR_START;
        }
        break;
    }
  }

}
