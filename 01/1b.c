#include <stdio.h>
#include <stdlib.h>

#define LINE_BUF_SIZE 256

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
    FILE* file = fopen(filename, "r");

    /* Our output! */
    int fuel_mass = 0;

    while (fgets(line, sizeof(line), file)) {
        /* Calculate fuel mass for this individual module */
        const int module_mass = atoi(line);

        int module_fuel_mass = (module_mass / 3) - 2;
        int extra_fuel = (module_fuel_mass / 3) - 2;

        while (extra_fuel > 0) {
            module_fuel_mass += extra_fuel;
            extra_fuel = (extra_fuel / 3) - 2;
        }

        fuel_mass += module_fuel_mass;
    }

    printf("Total fuel mass: %d\n", fuel_mass);

    fclose(file);

    return 0;
}
