#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE_BUF_SIZE 2048
#define NUMBER_SIZE 6

/*! Checks if a number (really a char array) contains
 *  two adjagent digits, but that these are not part
 *  of a larger group.
 */
int includes_two_adjacent_digits_not_larger(char *number) {
  char previous_digit = number[0];
  int digit_count = 1;

  for (int i = 1; i < NUMBER_SIZE; i++) {
    if (previous_digit == number[i]) {
      digit_count++;
    } else {
      if (digit_count == 2) {
        /* Exact termination condition! We have a strict set
         * of exactly 2 digits (including this previous_digit) next to each
         * other, somewhere in the string. */
        return 1;
      } else {
        /* A new digit */
        digit_count = 1;
      }
    }

    previous_digit = number[i];
  }

  /* Need to check if the last two digits are the same */
  return (digit_count == 2);
}

/*! Checks if a number (really a char array) has
 *  increasing or equal digits from left to right. */
int digits_strictly_increase(char *number) {
  char previous_digit = number[0];

  for (int i = 1; i < NUMBER_SIZE; i++) {
    if (previous_digit > number[i]) {
      return 0;
    }

    previous_digit = number[i];
  }

  return 1;
}

/*! Increments a char array representing an integer by 1 */
void increment_number(char *number) {

  for (int i = NUMBER_SIZE - 1; i >= 0; i--) {
    if (number[i] == '9') {
      number[i] = '0';
    } else {
      number[i] = number[i] + (char)1;
      break;
    }
  }

  return;
}

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
  char input_line_one[LINE_BUF_SIZE];
  char input_line_two[LINE_BUF_SIZE];
  bzero(input_line_one, sizeof(char) * LINE_BUF_SIZE);
  bzero(input_line_two, sizeof(char) * LINE_BUF_SIZE);
  /* Should check this result really... */
  FILE *file = fopen(filename, "r");

  /* This comes on two lines, so we can just deal with these separately. */
  fgets(input_line_one, sizeof(char) * LINE_BUF_SIZE, file);
  fgets(input_line_two, sizeof(char) * LINE_BUF_SIZE, file);
  fclose(file);

  /* Let's just brute force it today... First need to slice out
   * our strings into smaller pieces (well, we don't really, but
   * why not torture myself with more string nonsense?) */
  char start[NUMBER_SIZE + 1];
  char end[NUMBER_SIZE + 1];
  char current[NUMBER_SIZE + 1];

  for (int i = 0; i < NUMBER_SIZE; i++) {
    start[i] = input_line_one[i];
    end[i] = input_line_two[i];
    current[i] = input_line_one[i];
  }

  start[NUMBER_SIZE] = '\0';
  end[NUMBER_SIZE] = '\0';
  current[NUMBER_SIZE] = '\0';

  printf("Read in for line one: (%s) %s", start, input_line_one);
  printf("Read in for line two: (%s) %s", end, input_line_two);

  /* Now actually check for the number of, well valid numbers */
  int valid_number_count = 0;

  while (strcmp(current, end)) {
    if (includes_two_adjacent_digits_not_larger(current) &&
        digits_strictly_increase(current)) {
      valid_number_count++;
    }

    increment_number(current);
  }

  printf("Found %d valid numbers in your range.\n", valid_number_count);

  return 0;
}
