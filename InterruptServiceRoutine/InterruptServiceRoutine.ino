#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

// Settings
static const uint16_t timer_divider = 8; // Clock ticks at 10 MHz
static const uint64_t timer_max_count = 1000000;
static const TickType_t task_delay = 2000 / portTICK_PERIOD_MS;
static const uint8_t buf_len = 255;

// Pins
static const int led_pin = LED_BUILTIN;

// Globals
static hw_timer_t * timer = NULL;
static volatile int isr_counter;
static portMUX_TYPE spinlock = portMUX_INITIALIZER_UNLOCKED;
static volatile uint8_t msg_flag = 0;
static char *msg_ptr = NULL;
static volatile float adc_avg;

//*****************************************************************
// Interrupt Service Routines

// This function executes when timer reaches max (and resets)
void IRAM_ATTR onTimer()
{
  // ESP-IDF version of a critical section (in an ISR)
  portENTER_CRITICAL_ISR(&spinlock);
  isr_counter++;
  portEXIT_CRITICAL_ISR(&spinlock);

  // Vanilla FreeRTOS version
  //UBaseType_t saved_int_status;
  //saved_int_status = taskENTER_CRITICAL_FROM_ISR();
  //isr_counter++;
  //taskEXIT_CRITICAL_FROM_ISR(saved_int_status);
}

//*****************************************************************************
// Tasks

// Wait for semaphore and print out ADC value when received
void printValues(void *parameters) {

  // Loop forever
  while (1) {
    
    // Count down and print out counter value
    while (isr_counter > 0) {

      // Print value of counter
      Serial.println(isr_counter);
  
      // ESP-IDF version of a critical section (in a task)
      portENTER_CRITICAL(&spinlock);
      isr_counter--;
      portEXIT_CRITICAL(&spinlock);

      // Vanilla FreeRTOS version of a critical section (in a task)
      //taskENTER_CRITICAL();
      //isr_counter--;
      //taskEXIT_CRITICAL();
    }
  
    // Wait 2 seconds while ISR increments counter a few times
    vTaskDelay(task_delay);
  }
}

// Task: Read from serial terminal
void readSerial(void *parameters) {
  char c;
  char buf[buf_len];
  uint8_t idx = 0;

  // Clear whole buffer
  memset(buf, 0, buf_len);

  // Loop forever
  while (1) 
  {
    // Read characters from serial
    if (Serial.available() > 0) 
    {
      c = Serial.read();
      //Serial.print(c);
      if (idx < buf_len - 1) {
        buf[idx] = c;
        idx++;
      }
      
      // Update delay variable and reset buffer if we get a newline character
      if (c == '\n') 
      {
        // The last character in the string is '\n', so we need to replace
        // it with '\0' to make it null-terminated
        buf[idx - 1] = '\0';

        // Try to allocate memory and copy over message. If message buffer is
        // still in use, ignore the entire message.
        if (msg_flag == 0) {
          msg_ptr = (char *)pvPortMalloc(idx * sizeof(char));

          // If malloc returns 0 (out of memory), throw an error and reset
          configASSERT(msg_ptr);

          // Copy message
          memcpy(msg_ptr, buf, idx);
          
          // Notify other task that message is ready
          msg_flag = 1;
          //digitalWrite(led_pin, HIGH);
        }

        // Reset receive buffer and index counter
        memset(buf, 0, buf_len);
        idx = 0;
      } 
    }
  }
}


void setup() {
 // Configure Serial
  Serial.begin(115200);

  // Wait a moment to start (so we don't miss Serial output)
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("---FreeRTOS ISR Critical Section Demo---");

  // Start task to print out results
  xTaskCreatePinnedToCore(printValues,
                          "Print values",
                          1024,
                          NULL,
                          1,
                          NULL,
                          app_cpu);

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
