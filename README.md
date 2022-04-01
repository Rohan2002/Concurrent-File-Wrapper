# Word Wrap

A brief description of your testing strategy. How did you determine that your program is
correct? What sorts of files and scenarios did you check?

# Testing strategy
## General Properties of the Algorithm
1. For Word Wrap to ```Wrap Correctly``` the following properties must be satisfied.
    - The bytes read from an input file must be ordered correctly in the wrapped output file.
    - The number of characters in each sentence of the wrapped output file must be less than the ```max_width```
    - 

## How did we test our program?
1. First make sure to build the program by typing ```make``` from the root directory.
2. To generate a wrapped file, run the ```word_break``` executable located in ```bin``` using the command ```./bin/word_break <max-width> <test-file>```.
    - Note: ```<test-file>``` can either be a file, directory or nothing. 
        - If it's a file then it will read the file, and write to ```STDOUT```.
        - If it's a directory then it will traverse through the directory, read the files, and finally write to a file with the prefix ```wrap.``` file in the directory. 
        - If it's nothing then it will read input from ```STDIN``` and write the wrap text to ```STDOUT```.
3. Now to check if the wrapped text is actually correct, we run the executable located in ```bin``` using the command ```./bin/wcheck <max-width> <wrapped-output-file>```. 
    - Note since the output is written to ```STDOUT```, you can either redirect the output to a text file using the command ```./bin/word_break <max-width> <test-file> > wrapped_<test_file>``` and run the wcheck program using ```./bin/wcheck <max_width> wrapped_<test_file>``` or
    you can pipe (easier, in my opinion) ```word_break``` output into ```wcheck``` using the command ```./bin/word_break <max-width> <test-file> | ./bin/wcheck <max-width>``` .


## Testing the algorithm
1. Empty file (```0 bytes```)
    - Result: The program will wrap nothing.
2. Paragraph with a single break (```a break is defined as two newlines between a consecutive sentence```)
    - Result: The program will wrap 
3. Paragraph with multiple breaks
    - multiple single breaks between sentence
        - Result: Wraps correctly, and single break remains a single break.
    - multiple multiple breaks between sentence.
        - Result: Wraps correctly, and multiple breaks become a single break
4. One word per line.
    - Result: Wraps correctly
5. Word more than the ```max_width```
    - Result: Wraps the word correctly in the line, but returns ```EXIT_FAILURE```
6. Skipping files that start with ```wrap.``` or ```.```
    - Result: Returns Nothing, but skips over the file when traversing the directory
7. Varying read buffer-size does not affect the algorithm.
8. Abnormal whitespace locations in random sentence of the paragraphs.
    - Result: Normalizes the sentence by removing any extra white spaces in the trailing, leading or any portion of the sentence. 
9. Large ```max_width``` value must wrap the paragraph into ```one``` single line.
10. Rewrapping a file with the same ```max_width```` used to wrap the file with a similar ```max_width`` before.
11. 

## Testing the errors
1. Invalid file or directory path
    - Result: Invalid File Path Error and returns ```EXIT_FAILURE```
2. Reading files with no read permission
    - Result: Permissionm Denied Error Message and returns ```EXIT_FAILURE```
3. Writing files with no write permission
    - Result: Permissionm Denied Error Message and returns ```EXIT_FAILURE```

## Terminology
1. a ```break``` is defined as two newlines between a consecutive sentence

## Authors
Rohan Deshpande, and Selin Denise Altiparmak.