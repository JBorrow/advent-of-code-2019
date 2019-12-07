/* This one's quite fun - we're going to actually use a hash table for the first
 * time. The key to using a hash table in a way that doesn't just eat all of
 * your memory is coming up with a good hash function. Here we consider that in
 * the function get_hash.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE_BUF_SIZE 128

/* Note - if you change these, make sure they're consistent with each other! */
#define INDICATOR_LENGTH 3
#define INDICATOR_CHARACTER_RANGE 35
#define HASHTABLE_LENGTH (36 * 36 * 36 + 1) /* Need 1 because 0 is reserved */

/*#define DEBUG 1*/

#ifdef DEBUG
#define message(s, ...) ({ printf("" s "\n", ##__VA_ARGS__); })
#else
#define message(s, ...) ({})
#endif

/*! Planet/comet/etc... container for information about
 *  who orbits who!
 *
 *  This struct only has one member at the moment, but I anticipate
 *  we're going to get told to do something fun later... */
struct celestial_body {

  /*! Who do we orbit around? If this is 0, we don't exist!
   *  (For generality, we should compute the hash of the COM and set that
   *  as the default) */
  int orbits_around;
};

/*! Get the hash for a given INDICATOR_LENGTH input char array. */
int get_hash(char *input) {
  int hash = 0;
  int base = 1;
  int position_in_string = 0;
  char current_char = input[position_in_string];

  /* Here we convert our characters that are in the range [A-Z], [0-9]
   * to an integer. A-Z is in the ASCII range 65-90, so we take off 65,
   * and 0-9 are in the range 48-57 - so we take off 22. This lets
   * each value have its own hash! */

  while (current_char != '\0') {
    int current_value = (int)current_char;

    if (current_value < 58) {
      /* Must be an integer */
      current_value -= 22;
    } else if (current_value < 91) {
      /* Must be an uppercase character */
      current_value -= 65;
    }

    /* The real magic happens here, basically computing a three digit number in
     * base 35 */
    hash += base * current_value;
    base *= INDICATOR_CHARACTER_RANGE;

    if (position_in_string++ > INDICATOR_LENGTH) {
      break;
    }

#ifdef DEBUG
    message("Hash: %s, current: %c, value: %d, current_value: %d", input,
            current_char, hash, current_value);
#endif

    current_char = input[position_in_string];
  }

  return hash + 1; /* So that 0 is reserved for 'empty' bodies */
}

int main(int argc, char **argv) {
  char *filename;

  if (argc > 1) {
    filename = argv[1];
  } else {
    printf("Please provide a filename and a parameter.\n");
    exit(1);
  }

  /* Set up the hashtable */
  struct celestial_body *bodies = (struct celestial_body *)malloc(
      HASHTABLE_LENGTH * sizeof(struct celestial_body));

  /* Zero it all - by default everyone has a falsey orbit_around */
  bzero(bodies, HASHTABLE_LENGTH * sizeof(struct celestial_body));
  message("Zeroed hashtable.");

  message("Provided filename %s", filename);

  /* Actually read the file */
  char line[LINE_BUF_SIZE];
  /* Need to over-allocate indicator strings so they terminate correctly */
  char inner_body[INDICATOR_LENGTH + 1];
  char outer_body[INDICATOR_LENGTH + 1];
  /* Should check this result really... */
  FILE *file = fopen(filename, "r");

  while (fgets(line, sizeof(line), file)) {
    /* Each line has the following structure: ABC DEF. */
    /* Note that you should have changed your input file such that this
     * is true */
    /* We need to zero each time as we cannot ensure that each
     * body has an exactly 3-length code */
    bzero(inner_body, sizeof(char) * (INDICATOR_LENGTH + 1));
    bzero(outer_body, sizeof(char) * (INDICATOR_LENGTH + 1));

    sscanf(line, "%s %s\n", inner_body, outer_body);

    int const inner_hash = get_hash(inner_body);
    int const outer_hash = get_hash(outer_body);

#ifdef DEBUG
    if (inner_hash > HASHTABLE_LENGTH) {
      message("Inner hash %d out of bounds.", inner_hash);
      exit(-1);
    }
    if (outer_hash > HASHTABLE_LENGTH) {
      message("Outer hash %d out of bounds.", outer_hash);
      exit(-1);
    }
#endif

    message("Read Inner: %s (%d) -> Outer: %s (%d)", inner_body, inner_hash,
            outer_body, outer_hash);

    /* Whack em in the hashtable! */
    bodies[outer_hash].orbits_around = inner_hash;
  }

  /* Now the fun bit! We get to follow the hash tables all the way to the CoM!
   */
  int const com_hash = get_hash("COM");
  int total_orbits = 0;
  int current_hash = 0;

  for (int i = 0; i < HASHTABLE_LENGTH; i++) {
    if (bodies[i].orbits_around) {
      current_hash = i;

      while (current_hash != com_hash) {
        /* Follow the hash table all the way to the CoM */
        current_hash = bodies[current_hash].orbits_around;
        total_orbits++;
      }
    }
  }

  printf("There are %d orbits in your input file.\n", total_orbits);

  return 0;
}
