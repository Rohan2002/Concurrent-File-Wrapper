# Word Wrap

A brief description of your testing strategy. How did you determine that your program is
correct? What sorts of files and scenarios did you check?

# Testing strategy
## General Properties of the Algorithm
1. For Word Wrap to ```Wrap Correctly``` the following properties must be satisfied.
    - The bytes read from an input file must be ordered correctly in the wrapped output file.
    - The number of characters in each sentence of the wrapped output file must be

## How did we test our program?


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