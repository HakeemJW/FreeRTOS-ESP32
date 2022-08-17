# What is a Queue?
A queue is a first-in, first-out (FIFO) system that is used to store and pass information between tasks in an RTOS. Data copied to a queue will appear at the front of the queue, and any data added after that will line up behind it. When a task reads from a queue, the value at the front is removed, and all other tasks shift forward by one slot.

# The Challenge
Use FreeRTOS to create two tasks and two queues. 

Two tasks using two queues to communicate

Task A should print any new messages it receives from Queue 2. Additionally, it should read any Serial input from the user and echo back this input to the serial input. If the user enters “delay” followed by a space and a number, it should send that number to Queue 1.

Task B should read any messages from Queue 1. If it contains a number, it should update its delay rate to that number (milliseconds). It should also blink an LED at a rate specified by that delay. Additionally, every time the LED blinks 100 times, it should send the string “Blinked” to Queue 2. You can also optionally send the number of times the LED blinked (e.g. 100) as part of struct that encapsulates the string and this number.
