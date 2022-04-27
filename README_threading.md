# Multithreaded Word Wrap

## Purpose
The purpose of this program is to demonstrate the producer-consumer model using Posix threads. The producer threads <b>concurrently</b> enqueue directory and sub-directory paths in the unbounded directory stack and also enqueue regular file paths in the bounded file queue. The consumer threads also <b>concurrently</b> dequeue the regular file paths from the file queue, wrapping the content from the regular file and finally writing the wrapped content to a new regular file in the same directory as the regular input file. The new file can be identified by ```wrap.input_file_name.txt```.

## Unbounded Directory Stack

### Data Structure Purpose
The stack itself is a linear LIFO (Last In First Out) data structure that has the capability to expand once the stack is full. 

### Synchronization
We used ```one``` mutex lock provided as a field in the ```Stack``` struct to read/write any field in the ```Stack``` struct in a exclusive way. The mutex lock provides exclusive access for the threads to read and write the stack internals (e.g. data, pool_size) one at a time in order to avoid multiple threads reading and writing the stack internals at once. So when a thread ```t1``` acquires the lock for reading or writing the stack internals, and if another thread ```t2``` attempts to read or write the stack internals at the same time then ```t2``` will sleep until ```t1``` releases the mutex lock.
<br/>
<br/>
We also used ```one``` condition variable called ```ready_to_consume``` provided as a field in the ```Stack``` struct for inter thread communication purposes. ```ready_to_consume``` is set to wait in our ```pool_dequeue``` method when the the stack is empty. Once the ```wait``` is called in dequeue, the thread responsible for calling dequeue is set to sleep, the mutex lock is released temporarily and finally, the control is passed to enqueue. Enqueue then locks the mutex again, and enqueues data into the stack and ```signals``` the condition variable ```ready_to_consume``` so that dequeue won't be blocked anymore, and then dequeue can start dequeuing data once again.

### Unboundedness nature of the Stack
The reallocation of the stack happens in our ```pool_enqueue``` method. So when the stack is full, we ```double``` the size of the stack to ```2n``` where ```n``` is the original size of the stack. We also make sure to copy all of the existing data from the original stack of size ```n``` into the new stack of size ```2n```. 
<br/>
<br/>
One more thing to mention is that reallocation was the primary reason we used a stack rather than a circular queue. Since stack only has one pointer at the end, while a circular queue has two pointers one at the start (for dequeue), and one at the end (for enqueue), it was trivial to reallocate the stack with one pointer without worrying about any edge cases.

Queue
Lets say Bounded Queue looks like this:
```
B_Q = [ 1, 2, 3, 4, 5]
           ^        ^
           end     start
If we reallocate B_Q then we have to order the 
```
## Bounded File Queue

## Producer threads

## Consumer threads


## Terminology
1. Producer threads are the same as directory threads.
2. Consumer threads are the same as wrapping threads.