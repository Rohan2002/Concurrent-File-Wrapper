# Multithreaded Word Wrap

## Purpose
The purpose of this program is to demonstrate the producer-consumer model using Posix threads. The producer threads <b>concurrently</b> enqueue directory and sub-directory paths in the unbounded directory stack and also enqueue regular file paths in the bounded file queue. The consumer threads also <b>concurrently</b> dequeue the regular file paths from the file queue, wrapping the content from the regular file and finally writing the wrapped content to a new regular file in the same directory as the regular input file. The new file can be identified by ```wrap.input_file_name.txt```.

## Unbounded Directory Stack or Pool

### Data Structure Purpose
The pool itself is a linear LIFO (Last In First Out) data structure that has the capability to expand once the pool is full. 

### Pool Model
```
struct Pool_Data
{
    char *directory_path;
};
struct Pool_Model
{
    pool_data_type** data;
    int end;
    int pool_size;
    pthread_mutex_t lock;
    pthread_cond_t ready_to_consume;
    int number_of_elements_buffered;
    int number_of_active_producers;
    bool close;
};
```
1. The ```pool_data_type** data``` field is the actual unbounded directory stack or pool where every element in the pool is a pointer to the ```pool_data_type``` struct. The ```pool_data_type``` struct consist of only a char pointer that will store the directory or sub directories path. 
2. The ```end``` field is the end pointer that is incremented when an element in enqueued to the pool and decremented when an element is dequeued. Initially it is set to 0.
3. The ```pool_size``` field stores the inital capacity of Pool. The ```pool_size``` is doubled everytime the pool is full of data, thus the unbounded nature of the directory stack. The initial ```pool_size``` is stored as a macro called ```POOLSIZE``` in the ```word_break.c``` file.
4. The ```lock``` field is a mutex lock of type ```pthread_mutex_t```. Only one lock is initialized when the pool is initialized and destroyed when the pool is destroyed. This lock makes our pool thread-safe meaning it protects all the fields of ```Pool_Model``` from thread races. For more details, please look at the Mutex part in the Synchronization section.
5. The ```ready_to_consume``` is a condition variable. In short, it is used to block the dequeue method until enqueue method fills data in the ```Pool```. For more details, please look at the Condition Variable part in the Synchronization section.
6. The ```number_of_elements_buffered``` field stores the number of elements present inside the pool at any given time. The ```pool_size``` is incremented when an element in enqueued to the pool and decremented when an element is dequeued.
7. The ```number_of_active_producers``` field is developed to keep track of the number of directory threads that are reading directories at a given point of time. This count is incremented when a directory thread dequeues the directory path from the pool and starts reading the directory. The count is decremented after the  directory thread has enqueued all sub-directories from the directory path and done reading the directory.
8. The ```close``` field is a boolean flag to indicate whether the pool will accept anymore data. If the flag is set to ```True``` then all of the remaining directory threads that are responsbile for enqueuing and dequeuing the directories are woken up using the ```broadcast``` signal method, and thus all the remaining directory threads are signaled to finish enqueuing or dequeuing the remaining data. ```Note:``` This field prevents the directory threads from getting hanged because the dequeue method will ```not``` wait/block until the enqueue fills data as the ```close``` field indicates that there is no data available anymore.
### Synchronization

#### Mutex
We used ```one``` mutex lock provided as a field in the ```Stack``` struct to read/write any field in the ```Stack``` struct in a exclusive way. The mutex lock provides exclusive access for the threads to read and write the stack internals (e.g. data, pool_size) one at a time in order to avoid multiple threads reading and writing the stack internals at once. So when a thread ```t1``` acquires the lock for reading or writing the stack internals, and if another thread ```t2``` attempts to read or write the stack internals at the same time then ```t2``` will sleep until ```t1``` releases the mutex lock.
<br/>
<br/>
#### Condition Variable
We also used ```one``` condition variable called ```ready_to_consume``` provided as a field in the ```Stack``` struct for inter thread communication purposes. ```ready_to_consume``` is set to wait in our ```pool_dequeue``` method when the the stack is empty. Once the ```wait``` is called in dequeue, the thread responsible for calling dequeue is set to sleep, the mutex lock is released temporarily and finally, the control is passed to enqueue. Enqueue then locks the mutex again, and enqueues data into the stack and ```signals``` the condition variable ```ready_to_consume``` so that dequeue won't be blocked anymore, and then dequeue can start dequeuing data once again because there is some data in the stack.

### Unboundedness nature of the Stack
The reallocation of the stack happens in our ```pool_enqueue``` method. So when the stack is full, we ```double``` the size of the stack to ```2n``` where ```n``` is the original size of the stack. We also make sure to copy all of the existing data from the original stack of size ```n``` into the new stack of size ```2n```. 
<br/>
<br/>

### Why use an unbounded stack instead of a unbounded circular queue?
One more thing to mention is that reallocation was the primary reason we used a stack rather than a circular queue. Since stack only has ```one pointer``` at the end, while a circular queue has ```two pointers``` one at the start (for dequeue), and one at the end (for enqueue), so it was trivial to reallocate the stack with one pointer without worrying about any edge cases.

A example of when a stack is useful than a circular queue.
Lets say Circular Queue looks like this:
```
Circular_Queue = [ 1, 2, 3, 4, 5]
                            ^  ^
                            e  s
```
Assuming e = end pointer, and s = start pointer. If we reallocate Circular_Queue then we have to double the size of the Circular_Queue and re-order the data in the ```new``` double sized Circular_Queue so that ```start``` pointer comes first and the ```end``` pointer comes last. So the final result will look like [5, 1, 2, 3, 4, _, _, _, _, _, _]
```
Unbounded_Stack = [ 1, 2, 3, 4, 5]
                                ^
                                e
```
Assuming e = end pointer. If we reallocate Unbounded_Stack then we have to double the size of the Unbounded_Stack and just copy the data from index to 0 to the end pointer in the ```new``` double sized Unbounded_Stack. So the final result will look like [1, 2, 3, 4, 5, _, _, _, _, _, _]

### Unit-Tests for Pool
Our unit test for Pool can be found in ```unit_test/pool_test.c```.
We tested the pool by two main testing approaches. 
1. Vanilla Test (no threading) is meant to just enqueue certain number of elements, and dequeue the pool until its empty without the use of any type of threading. So if the pool is empty and there are no memory leaks then the Vanilla Test ran successfully.
2. Threading Test is designed to test out the producer-consumer model on the pool. We created a producer worker function that produces/enqueues ```n``` number of elements in the pool. We also created a consumer worker function that consumes/dequeues the elements from the pool until the pool is empty, and the producer has no more data to produce. Notice that unlike the vanilla test, this test will concurrently enqueue, and dequeue elements from the pool. So if the pool is empty and there are no memory leaks then the threading Test ran successfully.
    - For the threading test, we tried different combinations for the number of producer, and the number of consumer threads that will target the producer worker function, and consumer worker function respectivily. For instance let's say a pair of producer, and consumer threads is represented as ```(p,c)```. Then we tested the threading test with the pairs (1,1), (3,5), (5,3), (4,4).

## Bounded File Queue
d
## Producer threads

## Consumer threads


## Terminology
1. Producer threads are the same as directory threads.
2. Consumer threads are the same as wrapping threads.