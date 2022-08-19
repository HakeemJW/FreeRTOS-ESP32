#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

// Settings
static const uint16_t timer_divider = 8; // Clock ticks at 10 MHz
static const uint64_t timer_max_count = 1000000;
static const TickType_t task_delay = 2000 / portDELAY_PERIOD_MS;

// Pins
static const int led_pin = LED_BUILTIN;

// Globals
static hw_timer_t * timer = NULL;

//*****************************************************************
// Interrupt Service Routines

// This function executes when timer reaches max (and resets)
void IRAM_ATTR onTimer()
{
  // Toggle LED
  int pin_state = digitalRead(led_pin);
  digitalWrite(led_pin,!pin_state);
}

void setup() {
  // Configure Serial
  Serial.begin(115200);

  // Configue LED
  pinMode(led_pin, OUTPUT);

  // Create and start timer (num,divider,countUP)
  timer = timerBegin(0, timer_divider, true);

  // Provide ISR to timer (timer, function, edge)
  timerAttachInterrupt(timer,&onTimer,true);

  // At what count should the timer trigger the ISR (timer,count,autoreload)
  timerAlarmWrite(timer,timer_max_count,true);

  // Allow ISR to trigger
  timerAlarmEnable(timer);

}

void loop() {
  // put your main code here, to run repeatedly:

}
