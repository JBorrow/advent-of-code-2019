/* Modified from 2a.c. What does this require?
 *
 * We need to add:
 *
 * + Opcode 3 (takes a single input and saves it in only parameter),
 *     3, 50, X stores X in address 50
 * + Opcode 4 (outputs the value of its only parameter),
 *     4, 50 outputs whatever's at 50
 *     (unclear what this actually does? I think this means that it
 *      prints that value to screen?)
 *     Apparently 3, 0, 4, 0, 99 outputs the 'input' and halts?
 *
 *  + Add support for parameter modes. Currently all parameters are position
 *    mode. This mode allows for constant values for the input of an
 *    instruction.
 *
 *    This makes the opocodes a nightmare to parse as they are now 5 digit
 *    numbers...:
 *
 *     ABCDE -
 *      DE - two-digit opcode
 *      C - first parameter mode
 *      B - second parameter mode
 *      A - mode of third parameter
 *
 *    Leading 0s are omitted
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE_BUF_SIZE 2048
#define OPCODE_BUF_SIZE 32
#define MAX_INSTRUCTION_LENGTH 4
#define DEBUG 1

#ifdef DEBUG
#define message(s, ...) ({ printf("" s "\n", ##__VA_ARGS__); })
#else
#define message(s, ...) ({})
#endif

/*! Parses a 5 digit opcode to the actual output parameters
 *  See the header comment for more information. */
void parse_opcode(int instruction, int *out_opcode, int *out_first_param_mode,
                  int *out_second_param_mode, int *out_third_param_mode) {

  /* Do this in reverse order to avoid costly %s - there has to be a better
   * way of doing this! */
  *out_third_param_mode = instruction / 10000;
  instruction -= *out_third_param_mode * 10000;
  *out_second_param_mode = instruction / 1000;
  instruction -= *out_second_param_mode * 1000;
  *out_first_param_mode = instruction / 100;
  instruction -= *out_first_param_mode * 100;
  *out_opcode = instruction;

  message("Opcode, mode_x, mode_y, mode_z: %d, %d, %d, %d", *out_opcode,
          *out_first_param_mode, *out_second_param_mode, *out_third_param_mode);

  return;
}

/*! Gets a value of a parameter based on the mode and position */
int get_value_of_parameter(int *opcodes, int position, int mode) {
  int initial_value = opcodes[position];

  switch (mode) {
    case 0:
      /* Position mode */
      return opcodes[initial_value];

    case 1:
      /* Immediate mode */
      return initial_value;
  }

  message("Unhandled instruction %d at %d (Mode: %d).", initial_value, position,
          mode);
  exit(-1);

  return 0;
}

int main(int argc, char **argv) {
  char *filename;
  int user_input;

  if (argc > 1) {
    filename = argv[1];
    user_input = atoi(argv[2]);
  } else {
    message("Please provide a filename");
    exit(1);
  }

  message("Provided filename %s", filename);

  /* Actually read the file */
  char line[LINE_BUF_SIZE];
  /* Should check this result really... */
  FILE *file = fopen(filename, "r");

  /* We know that this input only comes on one line, so
   * we write a little parser here. */
  fgets(line, sizeof(line), file);
  fclose(file);

#ifdef DEBUG
  printf("Read in: %s", line);
#endif

  /* First we need to parse the number of opcodes; simply number
   * of commas plus one. */
  int number_of_opcodes = 0;

  for (int i = 0; i < LINE_BUF_SIZE; i++) {
    const char current_char = line[i];

    switch (current_char) {
      case '\0':
        break;

      case ',':
        number_of_opcodes++;
        continue;

      default:
        continue;
    }

    /* Only reach here if we break out of the switch */
    break;
  }

  /* Add on our one extra... */
  number_of_opcodes++;
  message("There are %d opcodes.", number_of_opcodes);

  /* Create an array to store opcodes in lenghtened by MAX_INSTRUCTION_LENGTH
   * to prevent segfaults if we ever try to read parameters past the end
   * of the array. */
  int *opcodes =
      (int *)malloc((number_of_opcodes + MAX_INSTRUCTION_LENGTH) * sizeof(int));

  /* Parse the opcodes array */
  char opcode_buf[OPCODE_BUF_SIZE];
  int current_opcode, position_in_opcode = 0;
  bzero(opcode_buf, sizeof(char) * OPCODE_BUF_SIZE);

  for (int i = 0; i < LINE_BUF_SIZE; i++) {
    const char current_char = line[i];

    switch (current_char) {
      case ',':
        /* Save the buffer and clean it out */
        opcodes[current_opcode] = atoi(opcode_buf);
        bzero(opcode_buf, sizeof(char) * OPCODE_BUF_SIZE);
        /* Move on with life... */
        current_opcode++;
        position_in_opcode = 0;
        continue;

      case '\0':
        /* Save final number */
        opcodes[current_opcode] = (int)atoi(opcode_buf);
        bzero(opcode_buf, sizeof(char) * OPCODE_BUF_SIZE);
        /* Update these as well for consistency checks */
        current_opcode++;
        position_in_opcode = 0;
        break;

      default:
        opcode_buf[position_in_opcode] = current_char;
        position_in_opcode++;
        continue;
    }

    /* Only reach here in case \0 */
    break;
  }

  message("I think that I managed to parse %d opcodes.", current_opcode);

  /* Let's see those opcodes then! */
  for (int i = 0; i < number_of_opcodes; i++) {
    message("Opcode %d = %d", i, opcodes[i]);
  }

  /* Now that we have the opcodes array, let's do the arithmetic! */

  current_opcode = 0;
  /* Functions are opcode(x, y, z) */
  int current_position = 0, current_instruction = 0;
  int x = 0, y = 0, z = 0;
  int mode_x = 0, mode_y = 0, mode_z = 0;

  while (current_opcode != 99) {
    /* Main program loop */

    current_instruction = opcodes[current_position];

    message("Current instruction: %d", current_instruction);
    parse_opcode(current_instruction, &current_opcode, &mode_x, &mode_y,
                 &mode_z);

    /* A lot of code duplication here, but I can't see a good way around this.
     */

    switch (current_opcode) {
      case 99:
        /* Termination */
        break;

      case 1:
        x = get_value_of_parameter(opcodes, current_position + 1, mode_x);
        y = get_value_of_parameter(opcodes, current_position + 2, mode_y);
        z = opcodes[current_position + 3];

        /* Addition x + y -> z*/
        opcodes[z] = x + y;
        current_position += 4;
        message("Addition, %d + %d", x, y);
        continue;

      case 2:
        x = get_value_of_parameter(opcodes, current_position + 1, mode_x);
        y = get_value_of_parameter(opcodes, current_position + 2, mode_y);
        z = opcodes[current_position + 3];

        /* Multiplication x * y -> z */
        opcodes[z] = x * y;
        current_position += 4;
        message("Multiplication, %d * %d", x, y);
        continue;

      case 3:
        /* This is _strictly_ an immediate mode instruction. */
        x = opcodes[current_position + 1];

        /* Request user input and store in -> x */
        opcodes[x] = user_input;
        current_position += 2;
        message("Storage %d -> %d", user_input, x);
        continue;

      case 4:
        /* This is _strictly_ an immediate mode instruction. */
        x = opcodes[current_position + 1];

        /* Output x */
        current_position += 2;
        printf("Output: %d from %d\n", opcodes[x], x);
        continue;

      default:
        printf("Invalid opcode %d\n", current_opcode);
        exit(-1);
        break;
    }
  }

#ifdef DEBUG
  printf("Post computation:\n");
  for (int i = 0; i < number_of_opcodes; i++) {
    printf("Opcode %d = %d\n", i, opcodes[i]);
  }
#endif

  return 0;
}
