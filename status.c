#include <assert.h>

#include <stdio.h>
#include <stdlib.h>


#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/inotify.h>
#include <sys/mman.h>


/* BATi capacity: A single ASCII decimal integer from  0 to 100. */
#define F_BATTERY_CAPACITY  "/sys/class/power_supply/BAT0/capacity"
/* BATi status: Either "Full", "Discharging", or "Charging". */
#define F_BATTERY_STATUS    "/sys/class/power_supply/BAT0/status"


/* 
 * A list of files to map and check for updates.
 */
static struct mapped_file {
    const char *name;
    int fd;
    char *data;
    const int size;

} mapped_files[] = {
    /* Database of all open files here; fd *MUST* start out as negative! */
    { .name = F_BATTERY_CAPACITY,   .size = 4,  .fd = -1},
    { .name = F_BATTERY_STATUS,     .size = 2,  .fd = -1 },
};

/* Some macros to the mapped files. */
#define b_capacity  (mapped_files[0])
#define b_status    (mapped_files[1])


_Static_assert(NULL == 0, "Cannot assume NULL is 0");



static void read_int(const char *filename, int *location) {
    FILE* infile = fopen(filename, "r");

    if (infile == NULL) {
        perror("Could not open file");
        exit(EXIT_FAILURE);
    }

    fscanf(infile, "%d", location);
    fclose(infile);
}


static void map_file(struct mapped_file *file) {
    file->fd = open(file->name, O_RDONLY);

    if (file->fd < 0) {
        perror("Could not open file");
        exit(EXIT_FAILURE);
    }

    file->data = mmap(NULL, file->size,
            PROT_READ, MAP_SHARED,
            file->fd, 0);

    if (file->data == MAP_FAILED) {
        perror("Could not map file");
        exit(EXIT_FAILURE);
    }

    printf("Mapped file: %s(%d)\n", file->name, file->fd);
    close(file->fd);
}


static void map_files() {
    int i;
    for (i = 0; i < sizeof(mapped_files)/sizeof(struct mapped_file); i++) {
        map_file(mapped_files + i);
    }
}


/* Opens mapped files. */
static void setup() {
    map_files();
}


#define NO_MAP 1
#if NO_MMAP
static double battery_percentage() {
    int battery_capacity;

    /* Read the battery status now. */
    read_int(F_BATTERY_CAPACITY, &battery_capacity);
    assert(battery_capacity >= 0 && battery_capacity <= 100);

    return battery_capacity;
}
#else
static double battery_percentage() {
    int battery_capacity;

    /* Read the battery status now. */
    read_int(F_BATTERY_CAPACITY, &battery_capacity);
    assert(battery_capacity >= 0 && battery_capacity <= 100);

    return battery_capacity;
}
#endif


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
    
    answer = b_status.data[0];
    
    return status_to_string(answer);
}


int main() {
    setup();

    while (1) {
        printf("Battery is: %lg%%, %s\n",
                battery_percentage(), battery_status());
        sleep(1);
    }

    return 0;
}
