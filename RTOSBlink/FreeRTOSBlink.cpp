#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

int ledPin = 13;
static const int led_pin = 21; 

void toggleLED(void *parameter)
{
  while(1)
  {
    digitalWrite(ledPin, HIGH);
    vTaskDelay(500/portTICK_PERIOD_MS);
    digitalWrite(ledPin, LOW);
    vTaskDelay(500/portTICK_PERIOD_MS);
  }
}

void toggleLED2Hz(void *parameter)
{
  while(1)
  {
    digitalWrite(led_pin, HIGH);
    vTaskDelay(250/portTICK_PERIOD_MS);
    digitalWrite(led_pin, LOW);
    vTaskDelay(250/portTICK_PERIOD_MS);
  }
}

void setup()
{
    pinMode(ledPin, OUTPUT);
    pinMode(led_pin, OUTPUT);
    xTaskCreatePinnedToCore(   // Use xTaskCreate() in vanilla FreeRTOS
              toggleLED,      // Function to be called
              "Toggle LED",   // Name of task
              1024,           // Stack size (bytes in ESP32, words in FreeRTOS)
              NULL,           // Parameter to pass to function
              1,              // Task Priority (0 to configMAX_Priorities - 1)
              NULL,           // Task Handle
              app_cpu);       // Run on one core for demo purposes (ESP32 only)

    xTaskCreatePinnedToCore(   // Use xTaskCreate() in vanilla FreeRTOS
            toggleLED2Hz,      // Function to be called
            "Toggle LED 2Hz",  // Name of task
            1024,              // Stack size (bytes in ESP32, words in FreeRTOS)
            NULL,              // Parameter to pass to function
            1,                 // Task Priority (0 to configMAX_Priorities - 1)
            NULL,              // Task Handle
            app_cpu);          // Run on one core for demo purposes (ESP32 only)

    // If this was vanilla FreeRTOS, you'd want to use vTaskStartScheduler() in
    // main after seetting up your tasks. 
}

void loop()
{

}