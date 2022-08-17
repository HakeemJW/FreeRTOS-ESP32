#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

// Settings
enum {BUF_SIZE = 5};
static const int num_prod_task = 5;
static const int num_cons_task = 2;
static const int num_writes = 3;

// Globals
static int buf[BUF_SIZE];
static int head = 0;
static int tail = 0;
static SemaphoreHandle_t bin_sem;
static SemaphoreHandle_t fill_sem;
static SemaphoreHandle_t empty_sem;
static SemaphoreHandle_t mutex;

// Tasks
void producer(void *parameters)
{
  int num  = *(int *)parameters;

  // Release the binary semaphore
  xSemaphoreGive(bin_sem);
  
  
  // Critical Section
  for (int i = 0; i < num_writes; i++)
  {
    // Wait for empty slot in buffer to be available
    xSemaphoreTake(empty_sem,portMAX_DELAY);

    // Take the mutex to  lock critical section
    xSemaphoreTake(mutex, portMAX_DELAY);
    buf[head] = num;
    head = (head + 1) % BUF_SIZE;
    xSemaphoreGive(mutex);

    // Signal to consumer tasks that a slot in the buffer has been filled
    xSemaphoreGive(fill_sem);
  }
  // Delete Task
  vTaskDelete(NULL);
}

void consumer(void *parameters)
{
  int val;

  // Read from buffer
  while (1)
  {
    // Wait for at least one slot in buffer to be filled
    xSemaphoreTake(fill_sem,portMAX_DELAY);
    
    // Take the mutex to  lock critical section
    xSemaphoreTake(mutex, portMAX_DELAY);
    val = buf[tail];
    tail = (tail + 1) % BUF_SIZE;
    Serial.println(val);
    xSemaphoreGive(mutex);
    
    // Signal to producer thread that a slot in the buffer is free
    xSemaphoreGive(empty_sem);
  }
}

void setup() {
  char task_name[12];
  
  // Configure Serial
  Serial.begin(115200);

  vTaskDelay(1000/portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("---FREERTOS Semaphore Challenge");

  // Create mutex before starting tasks
  mutex = xSemaphoreCreateMutex();

  // Create semaphore (start at zero)
  fill_sem  = xSemaphoreCreateCounting(BUF_SIZE,0);
  bin_sem = xSemaphoreCreateBinary();
  empty_sem = xSemaphoreCreateCounting(BUF_SIZE,BUF_SIZE);

  for (int i = 0; i < num_prod_task; i++)
  {
    sprintf(task_name, "Producer %i", i);
    xTaskCreatePinnedToCore(
      producer,
      task_name,
      1024,
      (void *)&i,
      1,
      NULL,
      app_cpu);

    xSemaphoreTake(bin_sem,portMAX_DELAY);
  }

  for (int i = 0; i < num_cons_task; i++)
  {
    sprintf(task_name, "Consumer %i", i);
    xTaskCreatePinnedToCore(
      consumer,
      task_name,
      1024,
      NULL,
      1,
      NULL,
      app_cpu);
  }

   // Notify that all tasks have been created
  Serial.println("All tasks created");
}

void loop() {
  // put your main code here, to run repeatedly:
  vTaskDelay(1000/portTICK_PERIOD_MS);
}
