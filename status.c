#include <assert.h>

#include <stdio.h>
#include <stdlib.h>

#include <sys/inotify.h>


#define F_BATTERY_NOW       "/sys/class/power_supply/BAT0/energy_now"
#define F_BATTERY_FULL      "/sys/class/power_supply/BAT0/energy_full"
#define F_BATTERY_VOLTAGE   "/sys/class/power_supply/BAT0/voltage_now"

static int battery_full = 0;

static void read_int(const char *filename, int *location) {
    FILE* infile = fopen(filename, "r");

    if (infile == NULL) {
        perror("Could not open file");
        exit(EXIT_FAILURE);
    }

    fscanf(infile, "%ld", location);
}

static double calc_battery_percentage(int battery_now, int voltage_now) {
    int battery_now, voltage_now;
    assert(battery_full > 0);

    /* Read the battery status now. */
    read_int(F_BATTERY_NOW, &battery_now);
    read_int(F_BATTERY_VOLTAGE, &voltage_now);
    assert(battery_now > 0);

    return 100.0L * ((double) battery_now) / ((double) battery_full);
}

int main() {

    /* Assume that "battery full" never changes. */
    read_int(F_BATTERY_FULL, &battery_full);

    printf("Battery is: %lg%%\n", calc_battery_percentage(battery_now, 0));

    return 0;
}
