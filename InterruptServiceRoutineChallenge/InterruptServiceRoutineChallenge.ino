#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

// Settings
static const uint16_t timer_divider = 8; // Clock ticks at 10 MHz
static const uint64_t timer_max_count = 1000000;
static const TickType_t task_delay = 1000 / portTICK_PERIOD_MS;
static const TickType_t read_delay = 20 / portTICK_PERIOD_MS;
static const uint8_t buf_len = 255;
enum {BUF_SIZE = 10};
static const int max_samples = 10;

// Pins
static const int led_pin = LED_BUILTIN;
static const int adc_pin = A0;

// Globals
static hw_timer_t * timer = NULL;
static volatile int isr_counter;
static portMUX_TYPE spinlock = portMUX_INITIALIZER_UNLOCKED;
static volatile uint8_t msg_flag = 0;
static char *msg_ptr = NULL;
static float adc_avg = 0;
static volatile uint16_t val;
static int total = 0;
static int buf[BUF_SIZE];
static int head = 0;
static int tail = 0;
static SemaphoreHandle_t fill_sem;
static SemaphoreHandle_t empty_sem;
static SemaphoreHandle_t mutex;

//*****************************************************************
// Interrupt Service Routines

// This function executes when timer reaches max (and resets)
void IRAM_ATTR onTimer()
{
  BaseType_t task_woken = pdFALSE;
  
  //xSemaphoreTakeFromISR(empty_sem, &task_woken);
  // Perform action (read from ADC)
  val = analogRead(adc_pin);
  buf[head] = val;
  head = (head + 1) % BUF_SIZE;
  
  // Give semaphore to tell task that new value is ready
  xSemaphoreGiveFromISR(fill_sem, &task_woken);
  
  // Exit from ISR (ESP-IDF)
  if (task_woken) {
    portYIELD_FROM_ISR();
  }
}

//*****************************************************************************
// Tasks

// Wait for semaphore and print out ADC value when received
void computeAverage(void *parameters) {

  int count = 0;
  // Loop forever
  while (1) {
    
    // Wait for at least one slot in buffer to be filled
    xSemaphoreTake(fill_sem,portMAX_DELAY);

    // Take the mutex to  lock critical section
    xSemaphoreTake(mutex, portMAX_DELAY);
    total += buf[tail];
    tail = (tail + 1) % BUF_SIZE;
    count++;
    
    if (count == 9 ) 
    {
      adc_avg = total/max_samples;
      count = 0;
      total = 0;
    }
    xSemaphoreGive(mutex);

    // Signal to producer thread that a slot in the buffer is free
    xSemaphoreGive(empty_sem);
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

        if (msg_flag == 1) 
        {
          Serial.print("You said: ");
          Serial.println(msg_ptr);

          if (strcmp(msg_ptr,"avg") == 0)
          {
            Serial.print("Average: ");
            Serial.println(adc_avg);
          }
          vPortFree(msg_ptr);
          msg_ptr = NULL;
          msg_flag = 0;
        }
          
        // Reset receive buffer and index counter
        memset(buf, 0, buf_len);
        idx = 0;
      } 
    }
  }
  // Wait 1 seconds while ISR increments counter a few times
  vTaskDelay(read_delay);
}


void setup() {
 // Configure Serial
  Serial.begin(115200);

  // Wait a moment to start (so we don't miss Serial output)
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("---FreeRTOS ISR Critical Section Demo---");

  // Create mutex before starting tasks
  mutex = xSemaphoreCreateMutex();

  // Create semaphore (start at zero)
  fill_sem  = xSemaphoreCreateCounting(BUF_SIZE,0);
  empty_sem = xSemaphoreCreateCounting(BUF_SIZE,BUF_SIZE);

  // Start task to print out results
  xTaskCreatePinnedToCore(readSerial,
                          "readSerial",
                          2000,
                          NULL,
                          1,
                          NULL,
                          app_cpu);
    
  xTaskCreatePinnedToCore(computeAverage,
                        "computeAverage",
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

  // Delete "setup and loop" task
  vTaskDelete(NULL);
}

void loop() {
  // put your main code here, to run repeatedly:

}
