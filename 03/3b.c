#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE_BUF_SIZE 2048
#define SEGMENT_BUF_SIZE 32

/* Nasty, but it'll do */
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

/*! Line segment - starting and ending positions. Note that
    start should be strictly less than end! */
struct line_segment {

  /*! Starting position of this line segment */
  int start;

  /*! Ending position of this line segment */
  int end;

  /*! Other co-ordinate that is constant for line segment */
  int constant;

  /*! Displacement */
  int displacement;

  /*! Horizontal or vertical */
  char direction;
};

/*! Counts the number of R, L, U, and D in the string to figure out the
 *  number of horizontal line segments */
int count_line_segments(char *line) {
  int num_line_segments = 0;

  for (int i = 0; i < LINE_BUF_SIZE; i++) {
    const char this_letter = line[i];

    if (this_letter == 'R' || this_letter == 'L' || this_letter == 'U' ||
        this_letter == 'D') {
      num_line_segments++;
    }

    if (this_letter == '\0') {
      break;
    }
  }

  return num_line_segments;
}

/*! Parses the string that is line to the array of horizontal
    and vertical line segments. */
void parse_line_to_segments(char *line, struct line_segment *segments) {

  char current_char = line[0];
  int current_position_in_line = 0;

  /* Should be U, D, L, R */
  char current_direction = 'A';
  /* Actual x, y position */
  int x = 0;
  int y = 0;

  /* String buffer for the current segment. This is filled with digits
   * then converted to an integer later */
  char segment_buf[SEGMENT_BUF_SIZE];
  int current_buf_position = 0;
  bzero(segment_buf, sizeof(char) * SEGMENT_BUF_SIZE);

  int segment_count = 0;
  int movement = 0;

  while (current_char != '\0') {
    switch (current_char) {
      /* Directional cases */
      case 'U':
        current_direction = 'U';
        break;
      case 'D':
        current_direction = 'D';
        break;
      case 'L':
        current_direction = 'L';
        break;
      case 'R':
        current_direction = 'R';
        break;

      /* Onto the next instruction */
      case ',':
        /* Find where we're going! */
        movement = atoi(segment_buf);
        bzero(segment_buf, sizeof(char) * SEGMENT_BUF_SIZE);
        current_buf_position = 0;

        switch (current_direction) {
          case 'U':
            segments[segment_count].start = y;
            segments[segment_count].end = y + movement;
            segments[segment_count].constant = x;
            segments[segment_count].displacement = movement;
            segments[segment_count].direction = 'V';
            y += movement;
            break;
          case 'D':
            segments[segment_count].end = y;
            segments[segment_count].start = y - movement;
            segments[segment_count].constant = x;
            segments[segment_count].displacement = -movement;
            segments[segment_count].direction = 'V';
            y -= movement;
            break;
          case 'R':
            segments[segment_count].start = x;
            segments[segment_count].end = x + movement;
            segments[segment_count].constant = y;
            segments[segment_count].displacement = movement;
            segments[segment_count].direction = 'H';
            x += movement;
            break;
          case 'L':
            segments[segment_count].end = x;
            segments[segment_count].start = x - movement;
            segments[segment_count].constant = y;
            segments[segment_count].displacement = -movement;
            segments[segment_count].direction = 'H';
            x -= movement;
            break;
        }

        segment_count++;
        break;

      /* Must be a digit, stick it in the buffer */
      default:
        segment_buf[current_buf_position] = current_char;
        current_buf_position++;
        break;
    }
    /* Move on to the next character */
    current_position_in_line++;
    current_char = line[current_position_in_line];
  }

  return;
}

/*! Prints an array of line segments */
void print_line_segments(struct line_segment *line_segments,
                         int num_of_line_segments) {
  for (int i = 0; i < num_of_line_segments; i++) {
    printf("Line segment %d: %d->%d at %d, %c\n", i, line_segments[i].start,
           line_segments[i].end, line_segments[i].constant,
           line_segments[i].direction);
  }

  return;
}

/*! Finds the intersections between two arrays of segments, and finds
    the shortest path length between those. */
int find_intersections(struct line_segment *line_one_segments,
                       struct line_segment *line_two_segments,
                       int num_segments_one, int num_segments_two) {

  /* This is a little complex of a solution to this one, but that's what this
   * is all about, right? :) */

  int shortest_wire_length = INT_MAX;
  int wire_length_one = 0;
  int wire_length_two = 0;

  for (int one = 0; one < num_segments_one; one++) {
    const int start_one = line_one_segments[one].start;
    const int end_one = line_one_segments[one].end;
    const int constant_one = line_one_segments[one].constant;
    const int direction_one = line_one_segments[one].direction;
    const int displacement_one = line_one_segments[one].displacement;

    /* We loop over two each time so need to re-set its length, otherwise
     * we count it num_segments_two many times */
    wire_length_two = 0;

    for (int two = 0; two < num_segments_two; two++) {
      const int start_two = line_two_segments[two].start;
      const int end_two = line_two_segments[two].end;
      const int constant_two = line_two_segments[two].constant;
      const int direction_two = line_two_segments[two].direction;
      const int displacement_two = line_two_segments[two].displacement;

      /* Only need to consider interactions for perpendicular wires */
      if (direction_one != direction_two) {
        if (!(start_one > constant_two || end_one < constant_two ||
              start_two > constant_one || end_two < constant_one)) {

          int wire_diff_one, wire_diff_two;

          /* Need to figure out if wire is coming from above or below */
          /* This can definitely be done in a better way... */
          if (displacement_one < 0) {
            /* We're going in the other direction! */
            wire_diff_one = abs(end_one - constant_two);
          } else {
            wire_diff_one = abs(constant_two - start_one);
          }

          if (displacement_two < 0) {
            /* We're going in the other direction! */
            wire_diff_two = abs(end_two - constant_one);
          } else {
            wire_diff_two = abs(constant_one - start_two);
          }

          /* Bang! The wires touch at this point. */
          const int one_wire_length_to_intersection =
              wire_length_one + wire_diff_one;
          const int two_wire_length_to_intersection =
              wire_length_two + wire_diff_two;

          const int round_trip_length =
              one_wire_length_to_intersection + two_wire_length_to_intersection;

          printf("Found intersection at %d, %d (%d and %d = %d)\n",
                 constant_one, constant_two, one_wire_length_to_intersection,
                 two_wire_length_to_intersection, round_trip_length);

          if ((one_wire_length_to_intersection != 0 &&
               two_wire_length_to_intersection != 0) &&
              round_trip_length < shortest_wire_length) {
            shortest_wire_length = round_trip_length;
          }
        }
      }

      /* Add on these pieces of wire */
      wire_length_two += abs(end_two - start_two);
    }
    wire_length_one += abs(end_one - start_one);
  }

  return shortest_wire_length;
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
  /* Should check this result really... */
  FILE *file = fopen(filename, "r");

  /* This comes on two lines, so we can just deal with these separately. */
  fgets(input_line_one, sizeof(char) * LINE_BUF_SIZE, file);
  fgets(input_line_two, sizeof(char) * LINE_BUF_SIZE, file);
  fclose(file);

  printf("Read in for line one: %s", input_line_one);
  printf("Read in for line two: %s", input_line_two);

  /* First figure out the number of segments so we can allocate */
  int line_one_num_segments = count_line_segments(input_line_one);
  int line_two_num_segments = count_line_segments(input_line_two);

  printf("Found %d segments for line one.\n", line_one_num_segments);
  printf("Found %d segments for line two.\n", line_two_num_segments);

  struct line_segment *line_one_segments = (struct line_segment *)malloc(
      sizeof(struct line_segment) * line_one_num_segments);
  struct line_segment *line_two_segments = (struct line_segment *)malloc(
      sizeof(struct line_segment) * line_two_num_segments);

  parse_line_to_segments(input_line_one, line_one_segments);
  parse_line_to_segments(input_line_two, line_two_segments);

  printf("Line one segments:\n");
  print_line_segments(line_one_segments, line_one_num_segments);

  int shortest_wire_length =
      find_intersections(line_one_segments, line_two_segments,
                         line_one_num_segments, line_two_num_segments);

  printf("Shortest wire length: %d\n", shortest_wire_length);

  free(line_one_segments);
  free(line_two_segments);

  return 0;
}
