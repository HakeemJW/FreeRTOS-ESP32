#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

// Globals
static TimerHandle_t one_shot_timer = NULL;
static TimerHandle_t auto_timer = NULL;

//***************************************************************************************
// Callbacks 

// Called when one of the timer expires
void myTimerCallback(TimerHandle_t xTimer)
{
  // Print if timer 0 expired
  if ((uint32_t)pvTimerGetTimerID(xTimer) == 0)
  {
    Serial.println("One-Shot Timer Expired");  
  }
  
  // Print if timer 0 expired
  if ((uint32_t)pvTimerGetTimerID(xTimer) == 1)
  {
    Serial.println("Auto Timer Expired");  
  }
}


//***************************************************************************************
// Main

void setup() {
  
  // Configure Serial
  Serial.begin(115200);
  
  // Wait a moment
  vTaskDelay(1000/portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("---FREERTOS Timer Demo");

  // Create one-shot timer
  one_shot_timer = xTimerCreate(
    "One-Shot timer",             // Name of timer
    2000 / portTICK_PERIOD_MS,    // Period of timer in Ticks
    pdFALSE,                      // Auto-reload
    (void *)0,                    // Timer ID
    myTimerCallback);             // Callback Function

  auto_timer = xTimerCreate(
    "One-Shot timer",             // Name of timer
    500 / portTICK_PERIOD_MS,    // Period of timer in Ticks
    pdTRUE,                       // Auto-reload
    (void *)1,                    // Timer ID
    myTimerCallback);             // Callback Function

  // Check to see if timer was created
  if(one_shot_timer == NULL || auto_timer == NULL)
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
    xTimerStart(auto_timer,portMAX_DELAY);
  }

  // Delete setup
  vTaskDelete(NULL);
}

void loop() {
  // Execution should never get here

}
