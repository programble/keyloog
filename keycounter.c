/* For usleep() */
#define _BSD_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <math.h>
#include <X11/Xlib.h>

bool end = false;

void stop(int s)
{
    end = true;
}

int main(int argc, char** argv)
{
    signal(SIGTERM, stop);
    signal(SIGQUIT, stop);
    signal(SIGINT, stop);
    
    Display *display = XOpenDisplay(NULL);
    if (!display) {
        return -1;
    }
    
    char keys_current[32], keys_last[32];
    int count = 0;
    time_t start_time = time(NULL);
    
    XQueryKeymap(display, keys_last);
    while (!end) {
        fflush(stdout);
        usleep(5000);
        XQueryKeymap(display, keys_current);
        for (int i = 0; i < 32; i++) {
            if (keys_current[i] != keys_last[i] && keys_current[i] != 0) {
                count++;
                //printf("\r%d", count);
            }
            keys_last[i] = keys_current[i];
        }
    }
    XCloseDisplay(display);
    
    /* C is teh awsumz */
    long int elapsed_seconds = lrint(difftime(time(NULL), start_time));
    long int format_seconds = elapsed_seconds;
    long int format_minutes = format_seconds / 60;
    format_seconds %= 60;
    long int format_hours = format_minutes / 60;
    format_minutes %= 60;
    long int format_days = format_hours / 24;
    format_hours %= 24;
    
    printf("You pressed %d keys in ", count);
    if (format_days > 0)
        printf("%ld days, ", format_days);
    if (format_hours > 0)
        printf("%ld hours, ", format_hours);
    if (format_minutes > 0)
        printf("%ld minutes, ", format_minutes);
    printf("%ld seconds\n", format_seconds);
    
    double keys_per_second = ((double) count) / elapsed_seconds;
    printf("You pressed %.2f keys per second\n", keys_per_second);
    printf("You pressed %.2f keys per minute\n", keys_per_second * 60);
    printf("You pressed %.2f keys per hour\n", keys_per_second * 60 * 60);
    printf("You pressed %.2f keys per day\n", keys_per_second * 60 * 60 * 24);
    
    return 0;
}
