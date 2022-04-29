# Multithreaded Word Wrap

## Purpose
The purpose of this program is to demonstrate the producer-consumer model using Posix threads. The producer threads <b>concurrently</b> enqueue directory and sub-directory paths in the unbounded directory stack and also enqueue regular file paths in the bounded file queue. The consumer threads also <b>concurrently</b> dequeue the regular file paths from the file queue, wrapping the content from the regular file and finally writing the wrapped content to a new regular file in the same directory as the regular input file. The new file can be identified by ```wrap.input_file_name.txt```.

## Unbounded Directory Stack or Pool

### Data Structure Purpose
The pool itself is a linear LIFO (Last In First Out) data structure that has the capability to expand once the pool is full. It is used as directory stack in our project.

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
3. The ```pool_size``` field stores the inital capacity of Pool. The ```pool_size``` is doubled everytime the pool is full of data, thus the unbounded nature of the directory stack. The initial ```pool_size``` is stored as a macro called ```POOLSIZE``` in the ```word_break.h``` file.
4. The ```lock``` field is a mutex lock of type ```pthread_mutex_t```. Only one lock is initialized when the pool is initialized and destroyed when the pool is destroyed. This lock makes our pool thread-safe meaning it protects all the fields of ```Pool_Model``` from thread races. For more details, please look at the Mutex part in the Synchronization section.
5. The ```ready_to_consume``` is a condition variable. In short, it is used to block the dequeue method until enqueue method fills data in the ```Pool```. For more details, please look at the Condition Variable part in the Synchronization section.
6. The ```number_of_elements_buffered``` field stores the number of elements present inside the pool at any given time. The ```number_of_elements_buffered``` field is incremented when an element in enqueued to the pool and decremented when an element is dequeued.
7. The ```number_of_active_producers``` field is developed to keep track of the number of directory threads that are reading directories at a given point of time. This count is incremented when a directory thread dequeues the directory path from the pool and starts reading the directory. The count is decremented after the  directory thread has enqueued all sub-directories from the directory path and done reading the directory.
8. The ```close``` field is a boolean flag to indicate whether the pool will accept anymore data. If the flag is set to ```True``` then all of the remaining directory threads that are responsbile for enqueuing and dequeuing the directories from the directory stack are woken up using the ```broadcast``` signal method, and thus all the remaining directory threads are signaled to finish enqueuing or dequeuing the remaining data. ```Note:``` This field prevents the directory threads from getting hanged because the dequeue method will ```not``` wait/block until the enqueue fills data because the ```close``` field indicates that there is no data available for the enqueue method to enqueue data anymore.
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
2. Threading Test is designed to test out the producer-consumer model on the pool. We created a producer worker function that produces/enqueues ```n``` number of elements in the pool. We also created a consumer worker function that consumes/dequeues the elements from the pool until the pool is empty, and the producer has no more data to produce. Notice that unlike the vanilla test, this test will concurrently enqueue, and dequeue elements from the pool. This will also test if the pthread synchronization devices work as expected. So if the pool is empty after every test and there are no memory leaks then the threading Test ran successfully.
3. General conditions for the test. 
    1. For the threading test, we tried different combinations for the number of producer, and the number of consumer threads that will target the producer worker function, and consumer worker function respectivily. For instance let's say a pair of producer, and consumer threads is represented as ```(p,c)```. Then we tested the threading test with the pairs (1,1), (3,5), (5,3), (4,4).
    2. For the vanilla, and threading test we tested it with 1000 elements. This number can be changed in the main method of ```unit_test/pool_test.c```
    3. Since this is an unbounded stack, we wanted to also test the reallocation mechanism of our stack. So we ran vanilla and threading test with the initial pool size of 1, 10, 100, 1000. These initial sizes reallocated the stack when neccessary (see condition for reallocation above).

    

## Bounded File Queue

### Data Structure Purpose
The queue is a linear FIFO (First In First Out) data structure that is circular and bounded. It is used as a file queue in our project.

### Queue Model
```
struct Queue_Data
{
    char *input_file;
    char *output_file;
};
typedef struct Queue_Data queue_data_type;

struct Queue_Model
{
    queue_data_type** data;
    size_t start;
    size_t end;
    size_t queue_size;
    pthread_mutex_t lock;
    pthread_cond_t ready_to_consume;
    pthread_cond_t ready_to_produce;
    size_t number_of_elements_buffered;
    bool close;
};
```
1. The ```queue_data_type** data``` field is the actual bounded file queue where every element in the queue is a pointer to the ```queue_data_type``` struct. The ```queue_data_type``` struct consist of a char pointer that will store the path of the input file that is to be wrapped and another char pointer that will store the output file where the wrapped content from the input file will be stored. 
2. The ```start``` field is the start pointer that is always incremented when an element is dequeued from the queue. Since the queue is circular, if the start pointer is at the end of the queue ```(Condition: start == queue_size)``` then start will be reset back to the beginning of the queue ```(start=0)``` by doing ```start % queue_size```. Initially ```start``` is set to 0. 
3. The ```end``` field is the end pointer that is always incremented when an element in enqueued to the queue. Since the queue is circular, if the end pointer is at the end of the queue ```(Condition: end == queue_size)``` then start will be reset back to the beginning of the queue ```(end=0)``` by doing ```end % queue_size```. Initially ```end``` is also set to 0.
4. The ```queue_size``` field stores the inital and fixed capacity of Queue. It is never changed during the lifetime of the Queue. The initial ```queue_size``` is stored as a macro called ```QUEUESIZE``` in the ```word_break.h``` file.
5. The ```lock``` field is a mutex lock of type ```pthread_mutex_t```. Only one lock is initialized when the Queue is initialized and destroyed when the Queue is destroyed. This lock makes our Queue thread-safe meaning it protects all the fields of ```Queue_Model``` from thread races.
6. The ```ready_to_consume``` is a condition variable. In short, when the queue is empty, it is used to block the dequeue method until enqueue method fills data in the ```Queue```. For more details, please look at the Condition Variable part in the Synchronization section.
7. The ```ready_to_produce``` is a condition variable. In short, when the queue is full, it is used to block the enqueue method until the dequeue method dequeues an element from the ```Queue```. For more details, please look at the Condition Variable part in the Synchronization section.
6. The ```number_of_elements_buffered``` field stores the number of elements present inside the queue at any given time. The ```number_of_elements_buffered``` is incremented when an element in enqueued to the queue and decremented when an element is dequeued.
7. The ```close``` field is a boolean flag to indicate whether the queue will accept anymore data. If the flag is set to ```True``` then all of the remaining file threads/consumer threads that are responsbile for dequeuing the file paths from the file queue and wrapping them to their respective output file path are woken up using the ```broadcast``` signal method. Thus, all the remaining file threads are signaled to finish dequeuing and wrapping the remaining file paths from the file queue. ```Note:``` This field prevents the file threads from finishing early because the directory threads will only set this flag to true when all of the directories and sub-directories have been read and all the files from those directories and sub-directories have been enqueued into the file queue.

### Unit-Tests for Queue
Our unit test for Queue can be found in ```unit_test/queue_test.c```.
We tested the Queue by two main testing approaches. 
1. Vanilla Test (no threading) is meant to just enqueue certain number of elements, and dequeue the queue until its empty without the use of any type of threading. So if the Queue is empty and there are no memory leaks then the Vanilla Test ran successfully.
2. Threading Test is designed to test out the producer-consumer model on the Queue. We created a producer worker function that produces/enqueues ```n``` number of elements in the Queue. We also created a consumer worker function that consumes/dequeues the elements from the Queue until the Queue is empty, and the producer has no more data to produce. Notice that unlike the vanilla test, this test will concurrently enqueue, and dequeue elements from the Queue. This will also test if the pthread synchronization devices work as expected. So if the Queue is empty after every test and there are no memory leaks then the threading Test ran successfully.
3. General conditions for the test. 
    1. For the threading test, we tried different combinations for the number of producer, and the number of consumer threads that will target the producer worker function, and consumer worker function respectivily. For instance let's say a pair of producer, and consumer threads is represented as ```(p,c)```. Then we tested the threading test with the pairs (1,1), (3,5), (5,3), (4,4).
    2. For the vanilla, and threading test we tested it with n (10, 100, 1000, 10000) as the bounded queue is of size n. This number can be changed in the main method of ```unit_test/pool_test.c```

## Utils
The purpose of this program it is a shared tool functions interface that is used in ```word_break.c``` and manipulate it accordingly.

## Utils functions that is used for debugging purpose

1. ```print_buffer``` is used for debugging purpose and print the buffer in ```wrap_text```.
2. ```safe_write``` checks whether or not it is succesfully written ```number of bytes``` into the file.  Otherwise, it returns -1 when error occurs.
## Util functions
Our util can be found in ```src/utils.c```.

1. ```check_file_or_directory``` returns 1 when it is a regular file, returns 2 when it is a directory name, otherwise it returns 0.
2. ```check_rsyntax``` includes ```-r``` syntax.
3. ```fill_param_by_use_argumemt``` checks how many ```*producer_threads``` and ```*consumer_threads``` is used according to the given first argument. 
    - In first case, we check if the first argument only includes ```-r``` syntax, which uses only 1 thread for reading and 1 thread for wrapping. 
    - In second case, we assume there is always ```,``` after number of threads are provided beforehand. If in our first argument there is only number of ```*consumer_threads``` provided for wrapping, the number of ```*producer_threads``` used for reading automatically would be updated to 1. 
    - In third case, if both ```*producer_threads``` and ```*consumer_threads```(after comma) is provided, it will update the number of ```*producer_threads``` and ```*consumer_threads``` is used. In last case, if the ```-r``` syntax is not provided it updates ```*isrecursive``` parameter to be 0. 
At the end. function checks the error case if the first argument is provided using ```*max_width``` parameter.

4. ```handle_multiple_input_files``` handles multiple files and directories in the user interface where client is asked for to wrap. The first parameter of this function ```widthindex``` it has the index value of the ```width``` parameter that client provided. This function will loop through the arguments greater than the widthindex to trace through the input files and directories. If the argument is a regular file, it will use the ```wrapt_text()``` function. If it is a directory, it will be ```enqued``` in the directory_pool which is provided as ```*dir_pool```. If there is only one regular file provided, it wraps and writes the standard output according to ```max_width```.
5. ```*concat_string``` it concatenates the first and second parameter accoroding to ```optional_prev_length``` and ```optional_new_length```. If ```optional_prev_length``` = -1 or ```optional_new_length``` = -1, the string length for the appropriate string will be computed, else it is the client's responsibility to give a valid string length for the appropriate string.```*concat_string``` is used to concatenate the ```dir_of_interest```regular file name string that we are interested in with string of ``wrap.`` before the regular file name interest.
6. ```*append_file_path_to_existing_path``` appends the name of a directory or regular file to an existing given path.
### Unit-Tests for Utils
Our unit test for Utils can be found in ```unit_test/utils_test.c```.
1. ```test_append_file_path_to_existing_path``` we test whether or not directory or given files name```e_file_name_2``` includes ```/``` at the end or not ```e_file_name_1```, once we test these cases we can see that ```e_file_name_1``` appends with ```new_file_1``` and ```e_file_name_2```  appends with ```new_file_2``` succesfully.
2. ```test_concat_strings``` if the given ```concat_string``` function concatenates the strings succesfully. We check whether there are no string is given on both parameters if its concetenating. We check if the one of the string is not given to concatenate, it returns the already given string name after concatenation.

## Word Break
This is the program where all of the previous components like the bounded queue, unbounded stack, util functions get combined into a multithreaded word wrapping program. We tried to use the best modularization practices, however, if you have any suggestions please feel free to comment it. 

We used a producer-consumer approach to tackle the multithreaded word-wrap program. Our producer worker function ```void *produce_files_to_wrap(void *arg);``` is targetted by the producer/directory thread and our consumer worker function ```void* consume_files_to_wrap(void *arg);``` is targetted by the consumer/wrapping threads. Our user interface allows us to control how many producer or how many consumer threads we desire, however you can read more about that in our user interface section. 

### produce_files_to_wrap
```produce_files_to_wrap``` takes in a ```struct file_producer``` as the producer thread arguement. In that struct we have a ```file_queue``` field that is responsible for enqueuing all the files from the directories and sub-directories. Then we have a field ```dir_pool``` which is our unbounded directory stack that is responsible for recursively enqueuing/dequeuing directories and sub directories. We also have a field ```isrecursive``` to enable recursive directory traversal or not (for the extra credit). Finally we have a ```error_code``` field that is an integer pointer and stores any error that occured in the producer worker function.

The directory pool initially is ```not``` passed empty to the ```produce_files_to_wrap``` function because we need a initial parent directory or some parent directories (extra credit) that the producer worker function can start traversing. In the loop ```produce_files_to_wrap``` function first ```dequeues``` a ```parent_dir_path``` from the ```dir_pool``` by calling the ```pool_dequeue``` function. 
Then a helper function ```int fill_pool_and_queue_with_data(char *parent_dir_path, Pool *dir_pool, Queue *file_q, int isrecursive);``` is called and it is responsible for taking the recently dequeued ```parent_dir_path``` and filling up the ```dir_pool``` and ```file_q``` with sub-directories and regular files from the ```parent_dir_path```. Internally, ```fill_pool_and_queue_with_data``` calls ```pool_enqueue``` function to enqueue sub-directories in the ```dir_pool``` and ```queue_enqueue``` function to enqueue regular file path in the ```file_q```. One more thing to mention, ```fill_pool_and_queue_with_data``` function also takes a ```isrecursive``` option which basically if ```not``` enabled then it indicates ```fill_pool_and_queue_with_data``` to not insert any sub-directory in the ```dir_pool```. This ```isrecursive``` option was specifically made for the extra credit section because the user interface can disable recursive directory traversal. 

### Ending condition for produce_files_to_wrap
The ending condition for our producer/directory threads are 1) the directory queue must be empty and 2) the number of directory threads that are actively reading a directory must be 0. To check if directory queue is not empty, we called our function ```pool_is_empty``` function from ```pool.c```, which basically checks if the number of elements in the pool is 0. Moreover, to check the number of directory threads, we use the field ```number_of_active_producers``` in the Pool struct to keep track of the number of directory threads that are currently working with a directory. So if a directory path is successfully dequeued from the ```dir_pool``` then the directory thread has some directory to work with and therefore the ```number_of_active_producers``` count is incremented by calling the ```increment_active_producers```. However, once ```fill_pool_and_queue_with_data``` finishes reading the directory, and fills up the stack and queues appropriately then the ```number_of_active_producers``` is decremented to indicate that the directory thread working on that particular directory has finished its work. So before we dequeue (the condition to start a directory thread working), we just check if the directory queue is empty and the ```number_of_active_producers``` is 0 then we just exit the loop and close the directory pool. We close the directory pool to just indicate that there is no more ```new``` directories so whichever directory thread was in the process of reading a directory should just finish their directory traversal and exit when they are done.

### consume_files_to_wrap
```consume_files_to_wrap``` takes in a ```struct file_consumer``` as the consumer thread arguement. In that struct we have a ```file_queue``` field that is the bounded file queue populated with the regular file paths created by the producer threads. Then we have a ```max_width``` field that is an argument to the ```wrap_text``` function inside the ```consume_files_to_wrap```. Finally we have a ```error_code``` field that is an integer pointer and stores any error that occured in the consumer worker function.

The main purpose of this cosumer worker function is to ```dequeue``` the data from the provided ```file_queue``` by calling ```queue_dequeue``` function, and to wrap the regular file path by calling the ```wrap_text``` algorithm written in the previous project. The item dequeued from the ```file_queue``` is of type ```queue_data_type``` and it consists of the ```input-file-path```, and ```output-file-path```. This input and output file paths are passed to the ```wrap_text``` function and a new wrapped file is written to ```output-file-path```. Since the producer loans the consumer an allocated heap-space item of type ```queue_data_type```, so it is also the responsiblity of this consumer function to free this object.

### Ending condition for consume_files_to_wrap
The ending condition for our consumer/wrapping threads are 1) The file queue must be empty 2) the file queue must also be closed. To check if directory queue is not empty, we called our function ```queue_is_empty``` function from ```queue.c```, which basically checks if the number of elements in the queue is 0. Moreover, since the file queue is closed only when the directory threads have finished traversing through all directories and sub directories, the consumers won't finish "early" and exit due to this condition because we want all the regular files from every given sub-directory and directory to be wrapped.

<br/>
<br/>


## Testing the algorithm
1. Empty file (```0 bytes```)
    - <b>Result</b>: The program return an empty file.
2. ```./bin/word_break -r note.txt```
    - <b>Result</b>: fill_param_by_user_arguememt(): Max width was either not provided or it cannot be 0!
3. ```./bin/word_break -r 0 note.txt```
    - <b>Result</b>: Error w./bin/word_break -r 0 note.txt 
fill_param_by_user_arguememt(): Max width was either not provided or it cannot be 0! main(): Error with parsing arguements.
3. ```./bin/word_break -r 10 note.txt```
    - <b>Result</b>: It wraps the 10 characters per line, creates ```wrap.note.txt``` file and writes to standart output recursively. It uses 1 ```*producer_threads``` and 1 ```*producer_threads```.
4. ```./bin/word_break -r20, 10 note.txt```
    - <b>Result</b>: It wraps the 10 characters per line, creates ```wrap.note.txt``` file and writes to standart output recursively. It uses 1 ```*producer_threads``` and 20 ```*consumer_threads```.
5. ```./bin/word_break -r20, 10 note.txt```
    - <b>Result</b>: It wraps the 10 characters per line, creates ```wrap.note.txt``` file and writes to standart output recursively. It uses 1 ```*producer_threads``` and 20 ```*consumer_threads```.
6. ```./bin/word_break -r20,5 10 note.txt```
    - <b>Result</b>: It wraps the 10 characters per line, creates ```wrap.note.txt``` file and writes to standart output recursively. It uses 20 ```*producer_threads``` and 5 ```*producer_threads```.
7. ```./bin/word_break -r20,5 10 note.txt```
    - <b>Result</b>: It wraps the 10 characters per line, creates ```wrap.note.txt``` file and writes to standart output recursively. It uses 20 ```*producer_threads``` and 5 ```*producer_threads```.
8. ```./bin/word_break -r 10 note.txt commands.txt```
    - <b>Result</b>: It wraps the 10 characters per line in created ```wrap.note.txt``` and ```wrap.commands.txt``` files recursively. It uses 1 ```*producer_threads``` and 1 ```*producer_threads```.
9. ```./bin/word_break -r3, 20 note.txt commands.txt```
    - <b>Result</b>: It wraps the 20 characters per line in created```wrap.note.txt``` and ```wrap.commands.txt``` files recursively. It uses 1 ```*producer_threads``` and 3 ```*producer_threads```.
10. ```./bin/word_break 20 note.txt commands.txt```
    - <b>Result</b>: It wraps the 20 characters per line in created ```wrap.note.txt``` and ```wrap.commands.txt``` files without using multithreads.
11. ``./bin/word_break 20 tests``
    - <b>Result</b>: It wraps the 20 characters per line in created all the ```wrap.*``` version of the regular files in the ```tests``` directory files without using multithreads, but it does not create ```wrap.*``` version of the files in the sub directories such as  ```tests/foo```, ```tests/foo/foo_a```, ```tests/foo/foo_b```.
12. ``./bin/word_break -r3,5 20 tests``
    - <b>Result</b>: It wraps the 20 characters per line in created all the ```wrap.*``` version of the regular files including its subdirectory regular files in the ```tests``` directory recursively. It uses 3 ```*producer_threads``` and 5 ```*producer_threads```.
13. ``./bin/word_break -r3,5 20 tests/foo tests2``
    - <b>Result</b>: It wraps the 20 characters per line in created all the ```wrap.*``` version of the regular files including its subdirectory regular files in the ```tests/foo``` directory recursively.It also  wraps the 20 characters per line in created all the ```wrap.*``` version of the regular files including its subdirectory regular files in the ```tests2``` directory recursively. It uses 3 ```*producer_threads``` and 5 ```*producer_threads```.
14. ```./bin/word_break -r3,5 20 tests/foo/foo_a/foo_c/a.txt tests2```
    - <b>Result</b>: It wraps the 20 characters per line in created ```tests/foo/foo_a/foo_c/a.txt``` file recursively. It also  wraps the 20 characters per line in created all the ```wrap.*``` version of the regular files including its subdirectory regular files in the ```tests2``` directory recursively. It uses 3 ```*producer_threads``` and 5 ```*producer_threads```.
15. ```./bin/word_break -r3,5 20 tests tests2 tests3```
    - <b>Result</b>: It wraps the 20 characters per line in created all the ```wrap.*``` version of the regular files including its subdirectory regular files in the ```tests``` ```tests2``` ```tests3``` directory recursively. It also  wraps the 20 characters per line in created all the ```wrap.*``` version of the regular files including its subdirectory regular files in the ```tests``` ```tests2``` ```tests3``` directory recursively. It uses 3 ```*producer_threads``` and 5 ```*producer_threads```.
16. ```./bin/word_break  20 tests tests2 tests3```
    - <b>Result</b>: It wraps the 20 characters per line in created all the ```wrap.*``` version of the regular files including its subdirectory regular files in the ```tests``` ```tests2``` ```tests3``` directories without using multithreads. It does not create ```wrap.*``` version of the files in the sub directories for ```tests``` ```tests2``` ```tests3``` directories.














## Terminology
1. Producer threads are the same as directory threads.
2. Consumer threads are the same as wrapping threads.