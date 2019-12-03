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
};

/*! Counts the number of R or L in the string to figure out the
 *  number of horizontal line segments */
int count_horizontal_line_segments(char *line) {
  int num_line_segments = 0;

  for (int i = 0; i < LINE_BUF_SIZE; i++) {
    const char this_letter = line[i];

    if (this_letter == 'R' || this_letter == 'L') {
      num_line_segments++;
    }

    if (this_letter == '\0') {
      break;
    }
  }

  return num_line_segments;
}

/*! Counts the number of U or D in the string to figure out the
 *  number of vertical line segments */
int count_vertical_line_segments(char *line) {
  int num_line_segments = 0;

  for (int i = 0; i < LINE_BUF_SIZE; i++) {
    const char this_letter = line[i];

    if (this_letter == 'U' || this_letter == 'D') {
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
void parse_line_to_segments(char *line,
                            struct line_segment *horizontal_segments,
                            struct line_segment *vertical_segments) {

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

  int horizontal_segment_count = 0;
  int vertical_segment_count = 0;
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
            vertical_segments[vertical_segment_count].start = y;
            vertical_segments[vertical_segment_count].end = y + movement;
            vertical_segments[vertical_segment_count].constant = x;
            y += movement;
            vertical_segment_count++;
            break;
          case 'D':
            vertical_segments[vertical_segment_count].end = y;
            vertical_segments[vertical_segment_count].start = y - movement;
            vertical_segments[vertical_segment_count].constant = x;
            y -= movement;
            vertical_segment_count++;
            break;
          case 'R':
            horizontal_segments[horizontal_segment_count].start = x;
            horizontal_segments[horizontal_segment_count].end = x + movement;
            horizontal_segments[horizontal_segment_count].constant = y;
            x += movement;
            horizontal_segment_count++;
            break;
          case 'L':
            horizontal_segments[horizontal_segment_count].end = x;
            horizontal_segments[horizontal_segment_count].start = x - movement;
            horizontal_segments[horizontal_segment_count].constant = y;
            x -= movement;
            horizontal_segment_count++;
            break;
        }

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
    printf("Line segment %d: %d->%d at %d\n", i, line_segments[i].start,
           line_segments[i].end, line_segments[i].constant);
  }

  return;
}

/*! Finds the intersections between two arrays of segments. Note
    that you should, of course, provide one of these segment arrays
    as horizontal and one as vertical for opposing segment sets. */
int find_intersections(struct line_segment *horizontal_line_segments,
                       struct line_segment *vertical_line_segments,
                       int num_horizontal_segments, int num_vertical_segments) {

  /* This is a little complex of a solution to this one, but that's what this
   * is all about, right? :) */

  int shortest_manhattan_distance = INT_MAX;

  for (int horizontal_segment_count = 0;
       horizontal_segment_count < num_horizontal_segments;
       horizontal_segment_count++) {

    const int x_start =
        horizontal_line_segments[horizontal_segment_count].start;
    const int x_end = horizontal_line_segments[horizontal_segment_count].end;
    const int y_const =
        horizontal_line_segments[horizontal_segment_count].constant;

    for (int vertical_segment_count = 0;
         vertical_segment_count < num_vertical_segments;
         vertical_segment_count++) {
      const int y_start = vertical_line_segments[vertical_segment_count].start;
      const int y_end = vertical_line_segments[vertical_segment_count].end;
      const int x_const =
          vertical_line_segments[vertical_segment_count].constant;

      /* Here's all of the fun logic.
       *
       * We now have two line segments. All we care about is where they
       * intersect. We have constructed everything up until now so that
       * we now basically have to do nothing. Note that if two parallel
       * lines overlap, we also by construction of the problem have to catch
       * that case here, as we always have a corner inside one of the lines.
       *
       * We know they don't intersect if y_start > y_const, or
       * y_end < y_const.
       *
       * We know they don't intersect if x_start > x_const, or x_end < x_const,
       * just from a simple geometric argument.
       *
       * Otherwise, they intersect at exactly x_const
       * and y_const. We then immediately know the manhattan distance. */

      if (!(y_start > y_const || y_end < y_const || x_start > x_const ||
            x_end < x_const)) {

        const int manhattan_distance = abs(x_const) + abs(y_const);
        printf("Found intersection at %d, %d (%d)\n", x_const, y_const,
               manhattan_distance);

        if (manhattan_distance < shortest_manhattan_distance &&
            manhattan_distance != 0) {
          shortest_manhattan_distance = manhattan_distance;
        }
      }
    }
  }

  return shortest_manhattan_distance;
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

  /* First figure out the number of horizontal or vertical segments
   * so we can avoid using linked lists and instead allocate arrays. */
  int line_one_num_horizontal_segments =
      count_horizontal_line_segments(input_line_one);
  int line_one_num_vertical_segments =
      count_vertical_line_segments(input_line_one);
  int line_two_num_horizontal_segments =
      count_horizontal_line_segments(input_line_two);
  int line_two_num_vertical_segments =
      count_vertical_line_segments(input_line_two);

  printf("Found %d horizontal and %d vertical segments for line one.\n",
         line_one_num_horizontal_segments, line_one_num_vertical_segments);
  printf("Found %d horizontal and %d vertical segments for line two.\n",
         line_two_num_horizontal_segments, line_two_num_vertical_segments);

  /* Allocate the segment arrays separately for horizontal and vertical
   * as we will loop through them separately later */
  struct line_segment *line_one_horizontal_segments =
      (struct line_segment *)malloc(sizeof(struct line_segment) *
                                    line_one_num_vertical_segments);
  struct line_segment *line_one_vertical_segments =
      (struct line_segment *)malloc(sizeof(struct line_segment) *
                                    line_one_num_vertical_segments);
  struct line_segment *line_two_horizontal_segments =
      (struct line_segment *)malloc(sizeof(struct line_segment) *
                                    line_two_num_vertical_segments);
  struct line_segment *line_two_vertical_segments =
      (struct line_segment *)malloc(sizeof(struct line_segment) *
                                    line_two_num_vertical_segments);

  parse_line_to_segments(input_line_one, line_one_horizontal_segments,
                         line_one_vertical_segments);
  parse_line_to_segments(input_line_two, line_two_horizontal_segments,
                         line_two_vertical_segments);

  printf("Line one horizontal segments:\n");
  print_line_segments(line_one_horizontal_segments,
                      line_one_num_horizontal_segments);

  int manhattan_one_two = find_intersections(
      line_one_horizontal_segments, line_two_vertical_segments,
      line_one_num_horizontal_segments, line_two_num_vertical_segments);

  int manhattan_two_one = find_intersections(
      line_two_horizontal_segments, line_one_vertical_segments,
      line_two_num_horizontal_segments, line_one_num_vertical_segments);

  const int best_manhattan = MIN(manhattan_one_two, manhattan_two_one);

  printf("Best Manhattan distance: %d\n", best_manhattan);

  free(line_one_horizontal_segments);
  free(line_one_vertical_segments);
  free(line_two_horizontal_segments);
  free(line_two_vertical_segments);

  return 0;
}
