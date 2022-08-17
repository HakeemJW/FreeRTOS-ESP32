// Needed for atoi()
#include <stdlib.h>

// Use only core 1 for demo purposes
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
static int led_delay = 500;   // ms

static char *msg_ptr = NULL;
static volatile uint8_t msg_flag = 0;

//*****************************************************************************
// Tasks

// Task: Blink LED at rate set by global variable
void printSerial(void *parameter) {
  while (1) {
    if (msg_flag == 1) {
      Serial.print("You said: ");
      Serial.println(msg_ptr);
      vPortFree(msg_ptr);
      msg_ptr = NULL;
      msg_flag = 0;
      digitalWrite(led_pin, LOW);
    }
  }
}

// Task: Read from serial terminal
// Feel free to use Serial.readString() or Serial.parseInt(). I'm going to show
// it with atoi() in case you're doing this in a non-Arduino environment. You'd
// also need to replace Serial with your own UART code for non-Arduino.
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

//*****************************************************************************
// Main

void setup() {
  pinMode(led_pin, OUTPUT);
  // Configure serial and wait a second
  Serial.begin(115200);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println("Echo String Demo");
  Serial.println("Enter a message and watch for it's return.");


            
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
  vTaskDelete(NULL);
}

void loop() {
  // Execution should never get here
}