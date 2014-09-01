#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>


/* BATi capacity: A single ASCII decimal integer from  0 to 100. */
#define F_BATTERY_CAPACITY  "/sys/class/power_supply/BAT0/capacity"
/* BATi status: Either "Full", "Discharging", or "Charging". */
#define F_BATTERY_STATUS    "/sys/class/power_supply/BAT0/status"

/* Resets cursor at first column and clears the line. */
#define RESET_LINE "\033[1G\033[K" 



static void read_int(const char *filename, int *location) {
    FILE* infile = fopen(filename, "r");

    if (infile == NULL) {
        perror("Could not open file");
        exit(EXIT_FAILURE);
    }

    fscanf(infile, "%d", location);
    fclose(infile);
}


static int battery_percentage() {
    int battery_capacity;

    /* Read the battery capacity now. */
    read_int(F_BATTERY_CAPACITY, &battery_capacity);
    assert(battery_capacity >= 0 && battery_capacity <= 100);

    return battery_capacity;
}


static char *status_to_string(char status) {
    switch (status) {
        case 'C': return "Charging";
        case 'D': return "Discharging";
        case 'F': return "Full";
        default:
            assert(0 && "Should not get here!");
    }
}


static char* battery_status() {
    char answer;
    FILE *infile = fopen(F_BATTERY_STATUS, "r");

    if (infile == NULL) {
        perror("Could not open file");
        exit(EXIT_FAILURE);
    }

    /* Read exactly one byte from the stream... */
    assert(fread(&answer, 1, 1, infile) == 1);

    fclose(infile);
    return status_to_string(answer);
}


int main() {

    while (1) {
        printf(RESET_LINE "Battery is: %d%%, %s",
                battery_percentage(), battery_status());
        fflush(stdout);

        sleep(1);
    }

    return 0;
}
