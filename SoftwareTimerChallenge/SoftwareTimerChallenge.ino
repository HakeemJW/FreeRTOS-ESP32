#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

// Settings
static const uint8_t buf_len = 255;

// Pins
static const int led_pin = LED_BUILTIN;

// Globals
static TimerHandle_t one_shot_timer = NULL;
static volatile uint8_t msg_flag = 0;
static char *msg_ptr = NULL;

//***************************************************************************************
// Callbacks 

// Called when one of the timer expires
void myTimerCallback(TimerHandle_t xTimer)
{
  // Print if timer 0 expired
  if ((uint32_t)pvTimerGetTimerID(xTimer) == 0)
  {
    Serial.println("One-Shot Timer Expired");
    digitalWrite(led_pin, LOW);  
  }
}

//***************************************************************************************
// Tasks

// Task: Echo serial monitor
void printSerial(void *parameter) {
  while (1) {
    if (msg_flag == 1) {
      Serial.print("You said: ");
      Serial.println(msg_ptr);
      vPortFree(msg_ptr);
      msg_ptr = NULL;
      msg_flag = 0;
      digitalWrite(led_pin, HIGH);
      xTimerStart(one_shot_timer,portMAX_DELAY);
    }
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
          digitalWrite(led_pin, HIGH);
        }

        // Reset receive buffer and index counter
        memset(buf, 0, buf_len);
        idx = 0;
      } 
    }
  }
}



//***************************************************************************************
// Main

void setup() {
  
  // Configure Serial
  Serial.begin(115200);

  // Configue LED
  pinMode(led_pin, OUTPUT);
  
  // Wait a moment
  vTaskDelay(1000/portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("---FREERTOS Timer Challenge");

  // Create Tasks
  // Start serial read task
  xTaskCreatePinnedToCore(  // Use xTaskCreate() in vanilla FreeRTOS
            readSerial,     // Function to be called
            "Read Serial",  // Name of task
            1024,           // Stack size (bytes in ESP32, words in FreeRTOS)
            NULL,           // Parameter to pass
            1,              // Task priority (must be same to prevent lockup)
            NULL,           // Task handle
            app_cpu);       // Run on one core for demo purposes (ESP32 only)
  
  // Start print task
  xTaskCreatePinnedToCore(  // Use xTaskCreate() in vanilla FreeRTOS
            printSerial,      // Function to be called
            "Print Serial",   // Name of task
            1024,           // Stack size (bytes in ESP32, words in FreeRTOS)
            NULL,           // Parameter to pass
            1,              // Task priority
            NULL,           // Task handle
            app_cpu);       // Run on one core for demo purposes (ESP32 only)
  // Delete "setup and loop" task

  // Create one-shot timer
  one_shot_timer = xTimerCreate(
    "One-Shot timer",             // Name of timer
    5000 / portTICK_PERIOD_MS,    // Period of timer in Ticks
    pdFALSE,                      // Auto-reload
    (void *)0,                    // Timer ID
    myTimerCallback);             // Callback Function

  // Check to see if timer was created
  if(one_shot_timer == NULL)
  {
    Serial.println("Timer could not be created");
  }
  else
  {
    // Wait then print that the timer is starting
    vTaskDelay(1000/portTICK_PERIOD_MS);
    Serial.println("Starting Timers...");

    // Start Timer (max block time if queue is full)
    xTimerStart(one_shot_timer,portMAX_DELAY);

  }

  // Delete setup
  vTaskDelete(NULL);
}

void loop() {
  // Execution should never get here

}
