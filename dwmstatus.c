/* -*- coding: utf-8 -*- */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <signal.h>
#include <unistd.h>

#include <X11/Xlib.h>


/* BATi capacity: A single ASCII decimal integer from  0 to 100. */
#define F_BATTERY_CAPACITY  "/sys/class/power_supply/BAT0/capacity"
/* BATi status: Either "Full", "Discharging", or "Charging". */
#define F_BATTERY_STATUS    "/sys/class/power_supply/BAT0/status"

/* Resets cursor at first column and clears the line. */
#define RESET_LINE "\033[1G\033[K"

/* Threshold percentage at which to start warning about battery level. */
#define BATTERY_WARNING 10

#define MSG_FONT        "Droid Sans Mono"
#define MSG_WARN_COLOR  "#c03333"
#define MSG_PREAMBLE    "<span font_family='" MSG_FONT "'><b>"
#define MSG_END         "</b></span>"

/* Placeholders are:
 *  0: span_start       -- either "<span color="#cc0000">" or ""
 *  1: battery_status   -- one of the given emoji
 *  2: battery_capacity -- a decimal number from 0 to 100
 *  3: span_end         -- either "</span>" or ""
 *  4: weekday          -- three letter weekday
 *  5: hour
 *  6: minute
 */
#define MSG_TEMPLATE    MSG_PREAMBLE "%s%s %3d%%%s %s %02d:%02d" MSG_END
#define MSG_SIZE        256

static const char* weekday_name[] = {
    "sun", "mon", "tue", "wed", "thu", "fri", "sat"
};


/* Globals. */
/* Buffer where message is constructed. */
char message_buffer[MSG_SIZE];
static Display *dpy;



/* Essentially xsetroot -name. Provided by dwmstatus on suckless. */
void set_status(char *str) {
	XStoreName(dpy, DefaultRootWindow(dpy), str);
	XSync(dpy, False);
}

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


static char *status_to_emoji(char status) {
    switch (status) {
        case 'C': return "🔌";
        case 'D': return "🔋";
        case 'F': return "🔌";
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
    return status_to_emoji(answer);
}


/* Reformats the string into message_buffer. */
static void recalculate_string() {
    int charge = battery_percentage();
    char* status = battery_status();
    char *span_start, *span_end;

    time_t time_result;
    struct tm *now;


    if (charge < BATTERY_WARNING) {
        span_start = "<span color=\"" MSG_WARN_COLOR "\">";
        span_end = "</span>";
    } else {
        /* Set both to the empty string. */
        span_start = span_end = "";
    }

    /* Fetch the time. */
    time_result = time(NULL);
    now = localtime(&time_result);

    if (now == NULL) {
        perror("Could not fetch time");
        exit(-1);
    }

    int written = snprintf(message_buffer, MSG_SIZE, MSG_TEMPLATE,
            span_start, status, charge, span_end,
            weekday_name[now->tm_wday],
            now->tm_hour, now->tm_min);

    assert((written + 1) < MSG_SIZE);
}


static void print_status() {
    recalculate_string();

#if STATUS_TO_STDIO
    printf(RESET_LINE "%s", message_buffer);
    fflush(stdout);
#else
    set_status(message_buffer);
#endif
}


/* Called by atexit hook. */
static void notify_terminated() {
    set_status("<span color='red'>"
               "<span font_family='Droid Sans Mono'>dwmstatus</span>"
               "<b> stopped!</b></span>");
}


int main() {

    /* First, open the X display. */
    if (!(dpy = XOpenDisplay(NULL))) {
        perror("Cannot open display");
        exit(EXIT_FAILURE);
    }

    /* Make sure the status is affected when this program is killed. */
    atexit(notify_terminated);
    signal(SIGINT, exit);
    signal(SIGTERM, exit);

    while (1) {
        print_status();
        sleep(1);
    }

    return 0;
}
