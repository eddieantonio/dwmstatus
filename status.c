#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/inotify.h>


/* BATi capacity: A single ASCII decimal integer from  0 to 100. */
#define F_BATTERY_CAPACITY  "/sys/class/power_supply/BAT0/capacity"
/* BATi status: Either "Full", "Discharging", or "Charging". */
#define F_BATTERY_STATUS    "/sys/class/power_supply/BAT0/status"


/* 
 * A list of files to map and check for updates.
 */
static struct watched_file {
    const char *name;
    int watch_id;
} watched_files[] = {
    /* All watched files here; watch_id *MUST* start out as negative! */
    { .name = F_BATTERY_CAPACITY,   .watch_id = -1},
    { .name = F_BATTERY_STATUS,     .watch_id = -1 },
    { .name = "./herp",             .watch_id = -1 },
};

static const int num_watched_files =
    sizeof(watched_files)/sizeof(struct watched_file);

/* Some macros for the watched files. */
#define b_capacity  (watched_files[0])
#define b_status    (watched_files[1])


/* The inotify instance used to watch the given files. */
static int watch_instance = -1;



static void read_int(const char *filename, int *location) {
    FILE* infile = fopen(filename, "r");

    if (infile == NULL) {
        perror("Could not open file");
        exit(EXIT_FAILURE);
    }

    fscanf(infile, "%d", location);
    fclose(infile);
}


static void watch_file(struct watched_file *file) {
    assert(watch_instance > 0 && "Uninitialized watch instance!");

    /* Try to watch the file for modifications. */
    file->watch_id = inotify_add_watch(watch_instance, file->name, IN_MODIFY);

    if (file->watch_id < 0) {
        perror("Could not add file to watch list");
        exit(EXIT_FAILURE);
    }
}

static void setup_watch() {
    int i;

    watch_instance = inotify_init();
    if (watch_instance < 0) {
        perror("Could not start inotify instance");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < num_watched_files; i++) {
        watch_file(watched_files + i);
    }
}


/* Sets up watched files. */
static void setup() {
    setup_watch();
}


static double battery_percentage() {
    int battery_capacity;

    /* Read the battery status now. */
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
    char answer = 'F';
    return status_to_string(answer);
}

static const char *wd_name(int wd) {
    int i;
    for (i = 0; i < num_watched_files; i++) {
        if (watched_files[i].watch_id != wd)
            continue;
        return watched_files[i].name;
    }
    return "<unknown>";
}

static void print_event(struct inotify_event *evt) {
    printf("\tEvent [%-3d]: mask %08x (%s)\n", evt->wd, evt->mask,
            wd_name(evt->wd));
}

int main() {
    struct inotify_event event;

    setup();

    while (1) {
        printf("Battery is: %lg%%, %s\n",
                battery_percentage(), battery_status());

        read(watch_instance, &event, sizeof(struct inotify_event));
        print_event(&event);
    }

    return 0;
}
