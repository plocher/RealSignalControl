# RealSignalControl
## License: MIT License

Drive a LED signal head using relays or a Seeed Studio high current RGB strip driver with an external power supply.


[Sample signal LED lamp](https://products.currentbyge.com/transportation-lighting/rail-signals/wayside-signals)


In mid-2015, I helped demonstrate that early versions of a real railroad signal head had an unexpected
failure mode:  In certain conditions, they would blink instead of showing a steady color.

If a signal's power supply / battery had an internal resistance that lowered its output voltage slightly
under load, and it's output voltage was just above that needed to turn on the LED lamp, the following
behaviors would happen:

  1. OK - the steady state current capacity of the PS was sufficient to the load:
  * The LED lamp would turn on bright and stay on.
  2. Marginal - the current sourcing was less than that consumed by the LED lamp, but the regulation was sufficient
   to keep the voltage from dropping:
  * The LED lamp would light brightly, quickly dim and stay on at a dim level.  This can often be seen in the orange
      "Don't Walk" pedestrian traffic crosswalk signals if you know what to look for...
  3. Failure - the regulation can't keep the voltage high enough, such as in a discharging battery:
  * The lamp would light bright, quickly dim and then go out, and when the power supply recovered,
      the lamp would turn back on and repeat the cycle, producing a blinking light pattern at 1/2 to 2 Hz

This is a concern because railroad signals are supposed to be part of a FAIL SAFE system.
DARK signals (burnt out lamps) are more restrictive than steady-on ones, and
steady-on ones are more restrictive than blinking ones:
  - DARK signals mean STOP and contact dispatcher for instructions
  - STEADY RED means stop and wait,
  - BLINKING RED means STOP and proceed slowly
OOPS!  A malfunctioning LED signal like this will actually fail /LESS/ safely

Cause: LEDs are voltage-activated, current driven devices.  They stay off (don't conduct) until a
threshold voltage is reached, at which time they conduct, with the current flow proportional to their brightness.
This inrush current is typically supplied from the capacitors found in the power source, with the source's
linear or switching regulators tasked with keeping up with the demand.
If the continuous power delivery of the supply is less than that of its peak, but still sufficient,
the LEDs will start bright and quickly dim, giving a strobe-like visual appearance.
If their current draw exceeds the continuous capacity, the supply's voltage typically drops, and at some point in
time, will drop below the LED's turn-on voltage threshold - which will cause the LED to turn off and stop drawing
current.  This allows the supply time to recover, recharge itself, and raise its voltage back above the turn-on
threshold...

To reproduce:
   0. start with an early model railroad signal lamp LED head (current ones don't have this bug!)
   1. set up a bench supply with V=LED turn on voltage (say 10v)
   2. set up current limiting on the supply such that it is greater than needed
   3. connect the LED lamp to the power supply.
   4. slowly lower the current limit until the LEDs start to dim
   5. slowly lower the voltage limit until the lamp turns off
   6. very slightly RAISE the voltage - if your bench supply isn't perfect, the LED lamp should start blinking

Result of testing:  The signals that displayed this behavior were immediately taken out of service and replaced
by an updated version.
