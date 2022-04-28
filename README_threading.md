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
2. Threading Test is designed to test out the producer-consumer model on the pool. We created a producer worker function that produces/enqueues ```n``` number of elements in the pool. We also created a consumer worker function that consumes/dequeues the elements from the pool until the pool is empty, and the producer has no more data to produce. Notice that unlike the vanilla test, this test will concurrently enqueue, and dequeue elements from the pool. This will also test if the pthread synchronization devices work as expected. So if the pool is empty after every test and there are no memory leaks then the threading Test ran successfully.
    - For the threading test, we tried different combinations for the number of producer, and the number of consumer threads that will target the producer worker function, and consumer worker function respectivily. For instance let's say a pair of producer, and consumer threads is represented as ```(p,c)```. Then we tested the threading test with the pairs (1,1), (3,5), (5,3), (4,4).

## Bounded File Queue
d
## Producer threads

## Consumer threads

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