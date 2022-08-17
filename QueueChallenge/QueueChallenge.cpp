// Use only core 1 for demo purposes
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif


// Settings
static const uint8_t msg_queue_len = 5;
static const uint8_t buf_len = 255;

// Pins
static const int led_pin = LED_BUILTIN;

// Globals
static QueueHandle_t msg_queue;
static QueueHandle_t msg_queue_2;

// Message struct: used to wrap strings (not necessary, but it's useful to see
// how to use structs here)
typedef struct Message {
  char body[20];
  int count;
} Message;

//Tasks
void printMessage(void *parameters) // Task A
{

  Message rx_msg;
  int num;
  // Loop forever
  while (1)
  {
    // See if there's a message in the queue (do nto block)
    if (xQueueReceive(msg_queue_2, (void *)&rx_msg,0) == pdTRUE)
    {
      Serial.print(rx_msg.body);
      Serial.println(rx_msg.count);
    }
    
    // Read characters from serial
    if (Serial.available() > 0) 
    {
      num = Serial.parseInt();

      // Update delay variable and reset buffer if we get a newline character
      if (num > 0) 
      {
        //led_delay = c;
        Serial.print("Updated LED delay to: ");
        Serial.println(num);

        // Try to add item to Queue for 10 ticks, fail if Queue is full
        if (xQueueSend(msg_queue, (void *)&num, 10) != pdTRUE)
        {
          Serial.println("Queue Full");
        }
        num = 0;
      } 
    }
    // Wait before trying again
    vTaskDelay(1000/portTICK_PERIOD_MS);
  }
}

void BlinkLED(void *parameters) // Task B
{
  Message msg;
  int blinkCount = 0;
  int led_delay = 500;   // ms

  // Set up pin
  pinMode(LED_BUILTIN, OUTPUT);
  
  // Loop forever
  while (1) 
  {
    // See if there's a message in the queue (do not block)
    xQueueReceive(msg_queue, (void *)&led_delay,0);

    // Blink LED
    digitalWrite(led_pin, HIGH);
    vTaskDelay(led_delay / portTICK_PERIOD_MS);
    digitalWrite(led_pin, LOW);
    vTaskDelay(led_delay / portTICK_PERIOD_MS);
    
    // Increment Blink Count
    blinkCount++;
    
    if (blinkCount >= 100)
    {
      // Construct message and send
      strcpy(msg.body, "Blinked: ");
      msg.count = blinkCount;
      
      // Send Message to Queue
      if (xQueueSend(msg_queue_2, (void *)&msg, 10) != pdTRUE)
        {
          Serial.println("Queue Full");
        }
      
      // Reset Blink Counter
      blinkCount = 0;
    }
  }
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  
  Serial.println();
  Serial.println("---FreeRTOS Queue Challenge---");
  Serial.println("Enter the command 'delay xxx' where xxx is your desired ");
  Serial.println("LED blink delay time in milliseconds");
  
  // Create Queues
  msg_queue = xQueueCreate(msg_queue_len,sizeof(int));
  msg_queue_2 = xQueueCreate(msg_queue_len,sizeof(Message));

  // Start Print Task
  // Start serial read task
  xTaskCreatePinnedToCore(  // Use xTaskCreate() in vanilla FreeRTOS
          printMessage,     // Function to be called
          "Print Message",  // Name of task
          1024,             // Stack size (bytes in ESP32, words in FreeRTOS)
          NULL,             // Parameter to pass
          1,                // Task priority (must be same to prevent lockup)
          NULL,             // Task handle
          app_cpu);         // Run on one core for demo purposes (ESP32 only)

  xTaskCreatePinnedToCore(  // Use xTaskCreate() in vanilla FreeRTOS
          BlinkLED,         // Function to be called
          "Blink LED",      // Name of task
          1024,             // Stack size (bytes in ESP32, words in FreeRTOS)
          NULL,             // Parameter to pass
          1,                // Task priority (must be same to prevent lockup)
          NULL,             // Task handle
          app_cpu);         // Run on one core for demo purposes (ESP32 only)

  // Delete "setup and loop" task
  vTaskDelete(NULL);
}

void loop() {

}
