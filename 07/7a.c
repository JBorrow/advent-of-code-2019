/* Modified from 5b.c.
 *
 * What we'll do this time is write our intcode interpreter for one input, and
 * then stick them together with bash or something.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE_BUF_SIZE 4096
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
void parse_opcode(long instruction, long *out_opcode,
                  long *out_first_param_mode, long *out_second_param_mode,
                  long *out_third_param_mode) {

  /* Do this in reverse order to avoid costly %s - there has to be a better
   * way of doing this! */
  *out_third_param_mode = instruction / 10000;
  instruction -= *out_third_param_mode * 10000;
  *out_second_param_mode = instruction / 1000;
  instruction -= *out_second_param_mode * 1000;
  *out_first_param_mode = instruction / 100;
  instruction -= *out_first_param_mode * 100;
  *out_opcode = instruction;

  message("Opcode, mode_x, mode_y, mode_z: %ld, %ld, %ld, %ld", *out_opcode,
          *out_first_param_mode, *out_second_param_mode, *out_third_param_mode);

  return;
}

/*! Gets a value of a parameter based on the mode and position */
long get_value_of_parameter(long *opcodes, long position, long mode) {
  long initial_value = opcodes[position];

  switch (mode) {
    case 0:
      /* Position mode */
      return opcodes[initial_value];

    case 1:
      /* Immediate mode */
      return initial_value;
  }

  message("Unhandled instruction %ld at %ld (Mode: %ld).", initial_value,
          position, mode);
  exit(-1);

  return 0;
}

int main(int argc, char **argv) {
  char *filename;
  long user_input[2];

  if (argc > 2) {
    filename = argv[1];
    user_input[0] = (long)atoi(argv[2]);
    user_input[1] = (long)atoi(argv[3]);
  } else {
    printf("Please provide a filename and two input numbers.\n");
    exit(1);
  }

  /* Used below in the interpreter */
  int current_user_input = 0;

  message("Provided filename %s, parameters %ld, %ld.", filename, user_input[0],
          user_input[1]);

  /* Actually read the file */
  char *line = (char *)malloc(LINE_BUF_SIZE * sizeof(char));
  bzero(line, LINE_BUF_SIZE * sizeof(char));
  /* Should check this result really... */
  FILE *file = fopen(filename, "r");

  /* We know that this input only comes on one line, so
   * we write a little parser here. */
  fgets(line, sizeof(char) * LINE_BUF_SIZE, file);
  fclose(file);

#ifdef DEBUG
  printf("Read in: %s", line);
#endif

  /* First we need to parse the number of opcodes; simply number
   * of commas plus one. */
  long number_of_opcodes = 0;

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
  message("There are %ld opcodes.", number_of_opcodes);

  /* Create an array to store opcodes in lenghtened by MAX_INSTRUCTION_LENGTH
   * to prevent segfaults if we ever try to read parameters past the end
   * of the array. */
  long *opcodes = (long *)malloc((number_of_opcodes + MAX_INSTRUCTION_LENGTH) *
                                 sizeof(long));
  bzero(opcodes, (number_of_opcodes + MAX_INSTRUCTION_LENGTH) * sizeof(long));

  /* Parse the opcodes array */
  char opcode_buf[OPCODE_BUF_SIZE];
  long current_opcode = 0, position_in_opcode = 0;
  bzero(opcode_buf, sizeof(char) * OPCODE_BUF_SIZE);

  for (int i = 0; i < LINE_BUF_SIZE; i++) {
    const char current_char = line[i];

    switch (current_char) {
      case ',':
        /* Save the buffer and clean it out */
        opcodes[current_opcode] = (long)atoi(opcode_buf);
        bzero(opcode_buf, sizeof(char) * OPCODE_BUF_SIZE);
        /* Move on with life... */
        current_opcode++;
        position_in_opcode = 0;
        continue;

      case '\0':
        /* Save final number */
        opcodes[current_opcode] = (long)atoi(opcode_buf);
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

  free(line);

  message("I think that I managed to parse %ld opcodes.", current_opcode);

  /* Let's see those opcodes then! */
  for (int i = 0; i < number_of_opcodes; i++) {
    message("Opcode %d = %ld", i, opcodes[i]);
  }

  if (current_opcode != number_of_opcodes) {
    printf("Found an invalid number of opcodes (%ld != %ld)\n", current_opcode,
           number_of_opcodes);
    exit(-1);
  }

  /* Now that we have the opcodes array, let's do the arithmetic! */

  current_opcode = 0;
  /* Functions are opcode(x, y, z) */
  int current_position = 0;
  long current_instruction = 0;
  long x = 0, y = 0, z = 0;
  long mode_x = 0, mode_y = 0, mode_z = 0;

  while (current_opcode != 99) {
    /* Main program loop */

    current_instruction = opcodes[current_position];

    message("Current instruction: %ld, position: %d", current_instruction,
            current_position);
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
        message("Addition, %ld + %ld -> %ld", x, y, z);
        continue;

      case 2:
        x = get_value_of_parameter(opcodes, current_position + 1, mode_x);
        y = get_value_of_parameter(opcodes, current_position + 2, mode_y);
        z = opcodes[current_position + 3];

        /* Multiplication x * y -> z */
        opcodes[z] = x * y;
        current_position += 4;
        message("Multiplication, %ld * %ld -> %ld", x, y, z);

        continue;

      case 3:
        /* This is _strictly_ an immediate mode instruction. */
        x = opcodes[current_position + 1];

        /* Request user input and store in -> x */
        message("Old value of %ld is %ld", x, opcodes[x]);
        opcodes[x] = user_input[current_user_input];
        current_position += 2;
        message("Storage %ld -> %ld", user_input[current_user_input], x);
        current_user_input++;

        continue;

      case 4:
        x = get_value_of_parameter(opcodes, current_position + 1, mode_x);

        /* Output x */
        current_position += 2;
        message("Output: %ld\n", x);
        printf("%ld\n", x);

        continue;

      case 5:
        /* Jump-if-true. */
        x = get_value_of_parameter(opcodes, current_position + 1, mode_x);
        y = get_value_of_parameter(opcodes, current_position + 2, mode_y);

        if (x) {
          current_position = y;
          message("Updating current instruction pointer to %ld", y);
        } else {
          current_position += 3;
          message("Failed jump-if-true condition");
        }

        continue;

      case 6:
        /* Jump-if-false */
        x = get_value_of_parameter(opcodes, current_position + 1, mode_x);
        y = get_value_of_parameter(opcodes, current_position + 2, mode_y);

        if (x == 0) {
          current_position = y;
          message("Updating current instruction pointer to %ld", y);
        } else {
          current_position += 3;
          message("Failed jump-if-false condition");
        }

        continue;

      case 7:
        /* Less than; if x < y; 1 -> z */
        x = get_value_of_parameter(opcodes, current_position + 1, mode_x);
        y = get_value_of_parameter(opcodes, current_position + 2, mode_y);
        z = opcodes[current_position + 3];

        opcodes[z] = x < y;
        message("Less than (%ld < %ld): Storing %d in %ld", x, y, x < y, z);

        current_position += 4;

        continue;

      case 8:
        /* Equals; if x==y; 1 -> z */
        x = get_value_of_parameter(opcodes, current_position + 1, mode_x);
        y = get_value_of_parameter(opcodes, current_position + 2, mode_y);
        z = opcodes[current_position + 3];

        opcodes[z] = x == y;
        message("Equals (%ld == %ld): Storing %d in %ld", x, y, x == y, z);

        current_position += 4;

        continue;

      default:
        printf("Invalid opcode %ld\n", current_opcode);
        exit(-1);
        break;
    }
  }

#ifdef DEBUG
  printf("Post computation:\n");
  for (int i = 0; i < number_of_opcodes; i++) {
    printf("Opcode %d = %ld\n", i, opcodes[i]);
  }
#endif

  return 0;
}
