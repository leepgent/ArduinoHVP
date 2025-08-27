# ArduinoHVP

This sketch is based on the ArduinoISP sketch.

It is used to upload a sketch to an ATTINY85 microcontroller using a High-Voltage Programmer (HVP).

The fuses can also be set using this program.

Futher modified to:
- run a boost converter circuit
- support the High-Voltage Programming of Attiny 25/45/85s
- Integrate original LV ISP ArduinoISP sketch and allow the switching between LV and HV mode
- Designed to work [this 'Universal Tiny Programmer' circuit](https://gitlab.com/leegent/universal_tiny_programmer).

## Programming Flash

Connect the Arduino to the ATTINY85 as shown at https://www.rickety.us/wp-content/uploads/2010/03/diagram.png

Upload the sketch to the Arduino

Upload a sketch to the ATTINY85 using the 'Arduino As ISP' programmer.

## Setting Fuses

The ATTINY85 fuses can be set by opening a Serial console (19200 Baud), typing 'Z', and follow the prompts.

## License

Firmware is licensed under the [2-Clause BSD License](https://opensource.org/licenses/bsd-license.php).

Copyright (c) 2008-2011 Randall Bohn
If you require a license, see
http://www.opensource.org/licenses/bsd-license.php

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
