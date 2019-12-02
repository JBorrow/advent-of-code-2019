#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE_BUF_SIZE 2048
#define OPCODE_BUF_SIZE 32

int main(int argc, char **argv) {
  char *filename;

  if (argc > 1) {
    filename = argv[1];
  } else {
    printf("Please provide a filename\n");
    exit(1);
  }

  printf("Provided filename %s\n", filename);

  /* Actually read the file */
  char line[LINE_BUF_SIZE];
  /* Should check this result really... */
  FILE *file = fopen(filename, "r");

  /* We know that this input only comes on one line, so
   * we write a little parser here. */
  fgets(line, sizeof(line), file);
  fclose(file);

  printf("Read in: %s", line);

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
  printf("There are %d opcodes.\n", number_of_opcodes);

  /* Create an array to store opcodes in */
  int *opcodes = (int *)malloc(number_of_opcodes * sizeof(int));

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

  printf("I think that I managed to parse %d opcodes.\n", current_opcode);

  /* Let's see those opcodes then! */
  for (int i = 0; i < number_of_opcodes; i++) {
    printf("Opcode %d = %d\n", i, opcodes[i]);
  }

  /* Now that we have the opcodes array, let's do the arithmetic! */

  current_opcode = 0;
  int current_position, x, y, out_position = 0;

  while (current_opcode != 99) {
    /* Main program loop */

    current_opcode = opcodes[current_position];
    printf("Current opcode: %d\n", current_opcode);

    switch (current_opcode) {
      case 99:
        /* Termination */
        break;

      case 1:
        /* Addition x + y */
        x = opcodes[opcodes[current_position + 1]];
        y = opcodes[opcodes[current_position + 2]];
        out_position = opcodes[current_position + 3];

        opcodes[out_position] = x + y;
        current_position += 4;
        printf("Addition, %d + %d\n", x, y);
        continue;

      case 2:
        /* Multiplication x * y */
        x = opcodes[opcodes[current_position + 1]];
        y = opcodes[opcodes[current_position + 2]];
        out_position = opcodes[current_position + 3];

        opcodes[out_position] = x * y;
        current_position += 4;
        printf("multiplication, %d * %d\n", x, y);
        continue;

      default:
        printf("Invalid opcode %d\n", current_opcode);
        break;
    }
  }

  printf("Post computation:\n");
  for (int i = 0; i < number_of_opcodes; i++) {
    printf("Opcode %d = %d\n", i, opcodes[i]);
  }

  /* Final result is stored in 0 */
  printf("Final result is: %d\n", opcodes[0]);

  return 0;
}
