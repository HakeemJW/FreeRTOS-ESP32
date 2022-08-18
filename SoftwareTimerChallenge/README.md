# Software Timers
Timers (in embedded systems) allow us to delay the execution of some function or execute a function periodically. Software timers exist in code and are not hardware dependent (except for the fact that the RTOS tick timer usually relies on a hardware timer). When a timer is created, you assign a function (a “callback function”) that is called whenever the timer expires. Note that timers are dependent on the tick timer, which means you can never create a timer with less resolution than the tick (1 ms by default). Additionally, you can set the software timer to be “one-shot” (executes the callback function once after the timer expires) or “auto-reload” (executes the callback function periodically every time the timer expires).

# The Challenge?

Your challenge is to create an auto-dim feature using the onboard LED. We’ll pretend that the onboard LED (LED_BUILTIN) is the backlight to an LCD. 

Create a task that echoes characters back to the serial terminal (as we’ve done in previous challenges). When the first character is entered, the onboard LED should turn on. It should stay on so long as characters are being entered.

Use a timer to determine how long it’s been since the last character was entered (hint: you can use xTimerStart() to restart a timer’s count, even if it’s already running). When there has been 5 seconds of inactivity, your timer’s callback function should turn off the LED.