# **README - Image processing and creation**

The program starts in the function `main`, initializing to 0 and `NULL` the current state of the program and the undo and redo stacks, since nothing is loaded into memory yet, and then it begins reading commands. For each read line, I extract the first token (the command), then extend the processing into other verification and argument-extraction functions (files, numeric values, text to write on the image).

I chose the stack principle because undo and redo are based on the LIFO principle. In addition, I split the solution into headers to separate the functions that process the image, the L-system, drawing on the image, the font and writing on the image, and undo/redo.

## 1. LOAD

The function `check_load` is called, which handles extracting the image file name in PPM format and loading it into the undoable commands stack, as well as into the current program state as the current image.

* the function `img_load` extracts image data from the `.ppm` file — first the magic bytes P6, the image width and height, the maximum channel value, and then it reads the pixels; all extracted data forms the final image which is returned into the program state;

  * I use an image structure where I store the width, height, and the pixels as an array of size `3 * width * height` (one element per channel for the 3 channels) — every 3 elements a new pixel begins.
* if no valid image can be extracted from the file, the failure message is printed; otherwise, using `stack_add` I add to the undoable commands stack the current program state with the newly loaded image, as well as the command type and file name so that undo and redo can be performed later.
* `stack_add` performs a copy of the current state (the function `state_deep_copy`), via a deep copy for each program element (the functions `image/character/font/lsystem_deep_copy`) and also stores the attributes of the command it adds to the stack.
* after adding the command to the undoable stack, the redoable commands stack is freed because nothing can be redone once a new command executes successfully — it will be filled again only when the `UNDO` command is called;
* the new image replaces the old one and the corresponding message for the `LOAD` command is printed.

## 2. SAVE

The function `check_save` is called, which extracts the name of the file that will contain the image that exists in program memory — if it does not exist, the failure message is printed.

* the function `save` saves the image from memory by writing to the given file the header (P6, width, height, 255) and the pixel array in binary format.

## 3. UNDO

The function `undo` is called, which checks whether an entry can be removed from the stack (i.e., whether there exists at least one undoable command before) through the function `stack_remove`. If a state can be removed, it is added via `stack_add` to the redo stack and the current state becomes the previous state.

* the previous state is given by `stack_remove`, which extracts the last entry in the undo stack and points to it, and also frees the memory allocated for the program stack, decreasing its size by 1.
* if `stack_remove` returns 1, then `undo` also returns 1 and the undo is performed; otherwise, the corresponding message is printed.

## 4. REDO

The function `redo` is called, which checks whether an entry can be removed from the stack (i.e., whether the previously undone state can be restored) through the function `stack_remove`. If possible, the state is removed from the redo stack and I add the current state to the undo stack so it can be undone again if needed, and then the current state becomes the next state (after redo, the state extracted by `stack_remove`).

* depending on the command to which redo is applied, the corresponding messages are printed (LSYSTEM, LOAD, TURTLE, TYPE, FONT).
* if `stack_remove` returns 1, then `redo` also returns 1 and the redo is performed; otherwise, the corresponding message is printed.

## 5. LSYSTEM

The function `check_lsystem` is called, which extracts the file name and loads into program memory the contained L-system, and also into the undoable commands stack.

* for loading, `lsys_load` is called and if it succeeds, the corresponding message is printed; otherwise, it proceeds identically to the image case — load into the undo stack via `stack_add`, free redo, the new L-system becomes the one in the current state and the success message is printed.

  * `lsys_load` opens the file and starts loading the structure corresponding to the L-system by reading the axiom (with the function `read_text`, which reads a line of unknown size using a temporary buffer that it reallocates when needed), then the number of derivation rules and afterwards the actual rules in the given format (it first reads the character, then the string representing the rule, and stores the extracted derivation rule in the pointer array at the position corresponding to the character (association via ASCII code)).

## 6. DERIVE

The function `check_derive` is called, which extracts the requested derivation order from the given command and then calls `derive`, printing the derived string after the function call.

* `derive` performs the n-th derivation of the string — n times, each character that has a rule is replaced with that rule — and it always computes the next derivation string length (`calculate_next_len` — knowing the rule, it adds to the current string length the rule length, or 1 if the character remains unchanged, thus avoiding inefficient reallocations inside loops).

  * character by character, they are replaced with the rule until the entire string is processed, and at the end of the n derivations the result is returned and printed in the verification function.

## 7. TURTLE

The function `check_turtle` is called, which checks whether an image and an L-system exist in program memory and then, if so, extracts in order the command attributes — the starting coordinates, step size, orientation angle, angular step, the derivation order of the L-system that will describe the path, and the R, G, B channel values that determine the drawing color (if during parsing one of these attributes is missing, command execution stops). Like for the image and L-system, loading into the undo stack follows the same principle.

* if the image, L-system, and all arguments exist, the function `turtle_movement` is called, which performs drawing on the image.

  * the function first derives the L-system to the n-th order, then assigns the turtle its starting coordinates and starting angle; here I also use a structure that stores turtle states so I know where to return when encountering the characters `[` or `]` in the string, via `add_state` and `restore_state`:

    * in `add_state`, one extra slot is allocated (representing the new state) and the current state is added on the last position.
    * in `restore_state`, the last turtle state is extracted and becomes the current state, and the now-unused memory is freed.
  * if the encountered symbol is `F`, then:

    * the angle value is converted from degrees to radians
    * the planar distances are computed
    * the coordinates are rounded from real numbers to integers
    * the straight line segment between (x0,y0) and (x1,y1) is drawn on the image;

      * if a coordinate pair is equal, I draw only the pixel (through `draw_pixel` — as long as we are within the image bounds, I determine the row to draw on and the pixel position in the pixel array, and for the three pixel values (channels) I set the given R, G, B values); otherwise, I draw the line through the function `draw_line` (which contains an implementation of Bresenham’s algorithm).
    * the turtle coordinates are updated to the end of the drawn segment.
  * if the encountered symbols are `+` or `-`, then the orientation angle is increased/decreased by the delta value (angular step).

## 8. FONT

The function `check_font` is called, which extracts the name of the file containing the font and loads it into program memory through `load_font`. Identical to the image and L-system: if it doesn’t exist, a failure message is printed; otherwise it is loaded into the undo stack, the new font replaces the old one, and the success message is printed.

* the function `load_font` opens the font file, allocates the required memory, and starts loading it into memory — here I use two structures: one for the font that stores the name, number of characters, and characters, and one for characters that stores writing-related aspects: code, axis offsets, starting coordinates, character extent, and the bitmap for each of them.
* depending on the keyword at the beginning of each line in the `.bdf` file, various functions are called:

  * **FONT** - font start, so the font name is added into the font structure.
  * **CHARS** - `chars_attribute` extracts the number of characters the font will contain and allocates memory in the font for that many characters
  * **STATCHAR** - allocates the structure for a new character
  * **ENCODING** - `encoding_attribute` extracts the code for the given character and adds it to the character structure (ASCII code).
  * **DWIDTH** - `dwidth_attribute` extracts the values by which the cursor will move on the image after writing a character and adds them to the character structure.
  * **BBX** - `bbx_attribute` extracts the four values for the character’s bounding box, i.e., the dimensions of the pixel matrix and the shift on the Ox and Oy axes, adding them to the character structure.
  * **BITMAP** - determines the required number of bytes for writing the character, allocates and builds the bitmap:

    * `allocate_bitmap` allocates a matrix of size `bbh` (bounding box height) * the previously determined number of bytes — I use `calloc` to directly fill with 0 the needed whitespace.
    * `build_bitmap` reads lines one by one from the `.bdf` file and converts each line from base 16 to binary using `hex_to_binary`, then copies it onto the corresponding row in the bitmap.

      * `hex_to_binary` uses bit operations and builds the binary number based on the hex input, passing through base 10; each character is transformed into a number (0-15), then I shift and extract the 4 bits (always combinations between 0000-1111) from left to right for each base-10 number, extract each bit and transform it into a character, which I append into the binary string.
  * **ENDCHAR** - moves the configured character (with all attributes) into the glyphs vector (font characters) inside the font structure.

## 9. TYPE

The function `check_type` is called, which checks whether an image and a font exist in program memory and then extracts the command attributes — the text to be written on the image, the coordinates where text writing begins, and the R, G, B values that determine the color in which it will be drawn. Like for the image, L-system, and font, loading into the undo stack follows the same principle.

* if the image, font, and all arguments exist, the function `draw_text` is called and thus the text is drawn on the image, and the success message is printed.

  * `draw_text` determines the length of the text to be written and for each character it searches it in the font glyphs vector (through `find_glyph`, which extracts the character and all its characteristics) and determines the starting position.
  * then it iterates through the character bitmap (only the `bbh * bbw` area; whitespace is not relevant here) and wherever it is 1 (i.e., part of the character), it computes the coordinates that must be drawn (and then calls `draw_pixel`).
  * finally, for each character, the cursor is moved to where writing of the next character will begin.

## 10. BITCHECK

First it is checked whether an image exists in program memory (otherwise the corresponding message is printed); else the function `check_pixel_string` is called, which performs the bitcheck process.

* the total number of bits in the image is computed (width * height * 24 = number of bits in a pixel = 3 channels * 8 bits each [0,255]).
* sequences of four bits are extracted and verified:

  * extraction with `extract_bit`, which extracts the bit at the given position by finding the channel it belongs to and the bit position inside the channel;
  * bits are extracted from left to right and shifted so that they preserve their order;
  * if the obtained number is 2 (0010) or 13 (1101), the problematic bit (always the third one) is taken and changed to its negation (0 or 1) inside the image (through `change_bit`);
  * then I determine the channel and pixel in which the bit is located, the image coordinates, I print the required message, and then I restore the image to its initial form, since the modification is not persistent.

## 11. EXIT

The EXIT command means that commands have finished being read, so the reading loop ends, program memory is freed, and execution ends.
