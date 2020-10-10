Serial Relay

# Install

TODO

# Setup

# Usage

TODO

# Serial

The serial interface has a very minimal syntax. A start byte (':'), followed by channelnumber=value, ended by a stop-byte (';'). No linebreaks or similar end of line markers are required, but can be send as other chars send after the stop-byte will be ignored.

Channel needs to be within the defined channels of the uC Code, 0-7 by default.
Value needs to be 0 or 1.

`:<channel>=<value>;`

Example

`:1=1,4=0;`
