# Word Wrap

A brief description of your testing strategy. How did you determine that your program is
correct? What sorts of files and scenarios did you check?

# Testing strategy

## Testing the algorithm
1. Vanilla Test Case
    -   Empty files (0 bytes): The program will wrap nothing.
2. Paragraph with a single break
3. Paragraph with multiple breaks
4. One word per line
5. Word more than the ```max_width```
6. Skipping files that start with ```wrap.``` or ```.```
7. Varying read buffer-size does not affect the algorithm.


## Testing the errors
1. Invalid file or directory path
2. Only regular files or directories
3. Reading files with no read permission
4. Writing files with no write permission