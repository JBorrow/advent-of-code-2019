/* Modified from 7a.c
 *
 * This requires a little more thought than the previous implementations.
 *
 * We need a way to pause the intcode interpreter and wait for intput...
 *
 * There is also starting to be a lot of code in here. If this gets
 * significantly more complex we need to consider splitting it out into multiple
 * files.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* For reading from string */
#define LINE_BUF_SIZE 4096
#define OPCODE_BUF_SIZE 32

/* Instruction size */
#define MAX_INSTRUCTION_LENGTH 4

/* Size of the input int array */
#define INPUT_SIZE 2

/* Size of the input sequence (how many interpreters we'll need) */
#define NUMBER_OF_INTERPRETERS 5

/*#define DEBUG 1*/

#ifdef DEBUG
#define message(s, ...) ({ printf("" s "\n", ##__VA_ARGS__); })
#else
#define message(s, ...) ({})
#endif

/*! Main intcode interpreter struct. Contains the memory allocated to that
 *  interpreter, as well as the current run-time variables. */
struct intcode_interpreter {

  /*! Is the interpreter currently active? */
  int is_active;

  /*! Has the interpreter ran to completion? */
  int has_exited;

  /*! The memory of the interpreter */
  int *memory;

  /*! Size of the memory */
  int memory_size;

  /*! Current instruction */
  int current_instruction;

  /*! Current opcode (with modes stripped) */
  int current_opcode;

  /*! Current position in memory */
  int current_position;

  /*! Input */
  int input[2];

  /*! Current position in that input (how many have been used?) */
  int current_position_in_input;

  /*! The last output from this interpreter */
  int last_output;

  /*! Return code from the last time we exited */
  int return_code;
};

/*! Prints the contents of an intcode interpreter's memory */
void print_memory(int *memory, int memory_size) {
  /* Let's see those opcodes then! */
  for (int i = 0; i < memory_size; i++) {
    message("Opcode %d = %d", i, memory[i]);
  }
}

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

/*! Creates an interpreter given an input filename */
void create_interpreter(char *filename,
                        struct intcode_interpreter *empty_interpreter) {

  /* Read the file */
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
  empty_interpreter->memory_size = 0;

  for (int i = 0; i < LINE_BUF_SIZE; i++) {
    const char current_char = line[i];

    switch (current_char) {
      case '\0':
        break;

      case ',':
        empty_interpreter->memory_size++;
        continue;

      default:
        continue;
    }

    /* Only reach here if we break out of the switch */
    break;
  }

  /* Add on our one extra, as we don't have a comma on the end... */
  empty_interpreter->memory_size++;
  message("There are %d opcodes.", empty_interpreter->memory_size);

  /* Create an array to store opcodes in lenghtened by MAX_INSTRUCTION_LENGTH
   * to prevent segfaults if we ever try to read parameters past the end
   * of the array. */
  empty_interpreter->memory = (int *)malloc(
      (empty_interpreter->memory_size + MAX_INSTRUCTION_LENGTH) * sizeof(int));

  /* Parse the opcodes array */
  char opcode_buf[OPCODE_BUF_SIZE];
  int current_opcode = 0, position_in_opcode = 0;
  bzero(opcode_buf, sizeof(char) * OPCODE_BUF_SIZE);

  for (int i = 0; i < LINE_BUF_SIZE; i++) {
    const char current_char = line[i];

    switch (current_char) {
      case ',':
        /* Save the buffer and clean it out */
        empty_interpreter->memory[current_opcode] = (int)atoi(opcode_buf);
        bzero(opcode_buf, sizeof(char) * OPCODE_BUF_SIZE);
        /* Move on with life... */
        current_opcode++;
        position_in_opcode = 0;
        continue;

      case '\0':
        /* Save final number */
        empty_interpreter->memory[current_opcode] = (int)atoi(opcode_buf);
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

  message("I think that I managed to parse %d opcodes.", current_opcode);

  print_memory(empty_interpreter->memory, empty_interpreter->memory_size);

  if (current_opcode != empty_interpreter->memory_size) {
    printf("Found an invalid number of opcodes (%d != %d)\n", current_opcode,
           empty_interpreter->memory_size);
    exit(-1);
  }

  /* Dummies required for parse_opcode */
  int mode_x, mode_y, mode_z;

  /* Now we can set dummies for the rest of the variables. */
  empty_interpreter->current_instruction = empty_interpreter->memory[0];
  empty_interpreter->current_position = 0;
  parse_opcode(empty_interpreter->current_instruction,
               &(empty_interpreter->current_opcode), &mode_x, &mode_y, &mode_z);

  /* Set these to -1 so it is recognisable if they are not set in the future */
  empty_interpreter->input[0] = -1;
  empty_interpreter->input[1] = -1;

  empty_interpreter->current_position_in_input = 0;
  empty_interpreter->is_active = 0;
  empty_interpreter->has_exited = 0;
  empty_interpreter->last_output = 0;
  empty_interpreter->return_code = 0;

  return;
}

/*! Copy the contents of an interpreter into another interpreter */
void copy_interpreter(struct intcode_interpreter *from,
                      struct intcode_interpreter *to) {

  /* First things first - need to copy the memory. */
  size_t size_of_memory =
      (from->memory_size + MAX_INSTRUCTION_LENGTH) * sizeof(int);
  to->memory = (int *)malloc(size_of_memory);

  memcpy(to->memory, from->memory, size_of_memory);

  /* Now copy the rest of the struct */
  to->is_active = from->is_active;
  to->has_exited = from->has_exited;
  to->memory_size = from->memory_size;
  to->current_instruction = from->current_instruction;
  to->current_opcode = from->current_opcode;
  to->current_position = from->current_position;
  to->input[0] = from->input[0];
  to->input[1] = from->input[1];
  to->current_position_in_input = from->current_position_in_input;
  to->last_output = from->last_output;
  to->return_code = from->return_code;

  return;
}

/*! Kill off an interpreter and free any assigned allocated memory */
void kill_interpreter(struct intcode_interpreter *interpreter) {
  free(interpreter->memory);
}

/*! Allows for an interpreter to take control of the thread and run to either:
 *
 * a) completion
 * b) it runs out of input
 *
 * If the interpreter exits for a reason,
 * it will return a code.
 *
 * Codes:
 *   0: more parameters needed
 *   1: success
 *   8: reached end of control without end instruction
 *   9: invalid instruction
 */
int run_interpreter(struct intcode_interpreter *interpreter) {

  /* Functions are opcode(x, y, z) */
  int x = 0, y = 0, z = 0;
  int mode_x = 0, mode_y = 0, mode_z = 0;

  while (interpreter->current_opcode != 99) {
    /* Main program loop */

    interpreter->current_instruction =
        interpreter->memory[interpreter->current_position];

    message("Current instruction: %d, position: %d",
            interpreter->current_instruction, interpreter->current_position);
    parse_opcode(interpreter->current_instruction,
                 &(interpreter->current_opcode), &mode_x, &mode_y, &mode_z);

    /* A lot of code duplication here, but I can't see a good way around this.
     */

    int address = interpreter->current_position;

    switch (interpreter->current_opcode) {
      case 99:
        /* Termination */
        interpreter->return_code = 1;
        return 1;
        break;

      case 1:
        x = get_value_of_parameter(interpreter->memory, address + 1, mode_x);
        y = get_value_of_parameter(interpreter->memory, address + 2, mode_y);
        z = interpreter->memory[address + 3];

        /* Addition x + y -> z*/
        interpreter->memory[z] = x + y;
        interpreter->current_position += 4;
        message("Addition, %d + %d -> %d", x, y, z);
        continue;

      case 2:
        x = get_value_of_parameter(interpreter->memory, address + 1, mode_x);
        y = get_value_of_parameter(interpreter->memory, address + 2, mode_y);
        z = interpreter->memory[address + 3];

        /* Multiplication x * y -> z */
        interpreter->memory[z] = x * y;
        interpreter->current_position += 4;
        message("Multiplication, %d * %d -> %d", x, y, z);

        continue;

      case 3:
        /* First need to check if we have any input left; if we do we need
         * to exit and do it _gracefully_! */
        if (interpreter->current_position_in_input == INPUT_SIZE) {
          interpreter->return_code = 0;
          return 0;
        }

        /* Otherwise, we should continue on to process input. */

        /* This is _strictly_ an immediate mode instruction. */
        x = interpreter->memory[address + 1];

        /* Request user input and store in -> x */
        message("Old value of %d is %d", x, interpreter->memory[x]);
        interpreter->memory[x] =
            interpreter->input[interpreter->current_position_in_input];
        interpreter->current_position += 2;
        message("Storage %d -> %d",
                interpreter->input[interpreter->current_position_in_input], x);
        interpreter->current_position_in_input++;

        continue;

      case 4:
        x = get_value_of_parameter(interpreter->memory, address + 1, mode_x);

        /* Output x */
        interpreter->current_position += 2;
        message("Output: %d\n", x);

        interpreter->last_output = x;

        continue;

      case 5:
        /* Jump-if-true. */
        x = get_value_of_parameter(interpreter->memory, address + 1, mode_x);
        y = get_value_of_parameter(interpreter->memory, address + 2, mode_y);

        if (x) {
          interpreter->current_position = y;
          message("Updating current instruction pointer to %d", y);
        } else {
          interpreter->current_position += 3;
          message("Failed jump-if-true condition");
        }

        continue;

      case 6:
        /* Jump-if-false */
        x = get_value_of_parameter(interpreter->memory, address + 1, mode_x);
        y = get_value_of_parameter(interpreter->memory, address + 2, mode_y);

        if (x == 0) {
          interpreter->current_position = y;
          message("Updating current instruction pointer to %d", y);
        } else {
          interpreter->current_position += 3;
          message("Failed jump-if-false condition");
        }

        continue;

      case 7:
        /* Less than; if x < y; 1 -> z */
        x = get_value_of_parameter(interpreter->memory, address + 1, mode_x);
        y = get_value_of_parameter(interpreter->memory, address + 2, mode_y);
        z = interpreter->memory[address + 3];

        interpreter->memory[z] = x < y;
        message("Less than (%d < %d): Storing %d in %d", x, y, x < y, z);

        interpreter->current_position += 4;

        continue;

      case 8:
        /* Equals; if x==y; 1 -> z */
        x = get_value_of_parameter(interpreter->memory, address + 1, mode_x);
        y = get_value_of_parameter(interpreter->memory, address + 2, mode_y);
        z = interpreter->memory[address + 3];

        interpreter->memory[z] = x == y;
        message("Equals (%d == %d): Storing %d in %d", x, y, x == y, z);

        interpreter->current_position += 4;

        continue;

      default:
        printf("Invalid opcode %d\n", interpreter->current_opcode);
        interpreter->return_code = 9;
        return 9;
        break;
    }
  }

  interpreter->return_code = 8;
  return 8;
}

int main(int argc, char **argv) {
  char *filename;
  char *input_sequence;
  char input_buffer[2];

  int phase_settings[NUMBER_OF_INTERPRETERS];
  struct intcode_interpreter master_interpreter;

  if (argc > 2) {
    /* Argv[1] is filename */
    create_interpreter(argv[1], &master_interpreter);
    input_sequence = argv[2];

    for (int i = 0; i < NUMBER_OF_INTERPRETERS; i++) {
      /* Must do this because atoi needs an end-of-string */
      input_buffer[0] = input_sequence[i];
      input_buffer[1] = '\0';
      phase_settings[i] = atoi(&input_buffer[0]);
      message("Phase setting for %d: %d", i, phase_settings[i]);
    }

    master_interpreter.current_position_in_input = 0;
    master_interpreter.input[0] = 0;
    master_interpreter.input[1] = 0;
  } else {
    printf("Please provide a filename and an input sequence.\n");
    exit(1);
  }

  message("Provided filename %s, sequence %s.", argv[1], argv[2]);

  /* Now that we have the master interpreter, we need to create enough
   * copies to fully staisfy our needs */
  struct intcode_interpreter interpreters[NUMBER_OF_INTERPRETERS];

  for (int i = 0; i < NUMBER_OF_INTERPRETERS; i++) {
    copy_interpreter(&master_interpreter, &interpreters[i]);
    interpreters[i].input[0] = phase_settings[i];
  }

  /* Now for the scary part - start them all off in sequence (wish us luck!) */
  int current_interpreter = 0, current_return_code = 0, input_from_previous = 0;

  /* We only care about the return code of E! */
  while (interpreters[NUMBER_OF_INTERPRETERS - 1].return_code != 1) {
    interpreters[current_interpreter].input[1] = input_from_previous;

    current_return_code = run_interpreter(&(interpreters[current_interpreter]));
    input_from_previous = interpreters[current_interpreter].last_output;
    message("Return code from %d: %d, last output: %d", current_interpreter,
            current_return_code, input_from_previous);

    /* Reset input for next time around, but don't re-read phases */
    interpreters[current_interpreter].current_position_in_input = 1;

    current_interpreter += 1;
    current_interpreter %= NUMBER_OF_INTERPRETERS;
  }

  /* Print our actual result, the last return value of interpreter E! */
  printf("%d\n", interpreters[NUMBER_OF_INTERPRETERS - 1].last_output);

  for (int i = 0; i < NUMBER_OF_INTERPRETERS; i++) {
    message("Last output from interpreter %d: %d", i,
            interpreters[i].last_output);
    kill_interpreter(&interpreters[i]);
  }

  return 0;
}
