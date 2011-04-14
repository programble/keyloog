/* For usleep() */
#define _BSD_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <math.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <X11/Xlib.h>

#define TERM_CLEAR "\x1b[2J"
#define TERM_HOME "\x1b[H"

#define PLURAL(x) (x != 1) ? "s" : ""

int count;
time_t start_time;
char *option_file = NULL;

bool end = false;

void signal_quit(int s)
{
    end = true;
}

void print_stats()
{
    FILE *file = stdout;
    if (option_file) {
        file = fopen(option_file, "w");
        if (!file) {
            perror(option_file);
            exit(1);
        }
    }
    
    long int elapsed_seconds = lrint(difftime(time(NULL), start_time));
    long int format_seconds = elapsed_seconds;
    long int format_minutes = format_seconds / 60;
    format_seconds %= 60;
    long int format_hours = format_minutes / 60;
    format_minutes %= 60;
    long int format_days = format_hours / 24;
    format_hours %= 24;
    
    fprintf(file, "You pressed %d key%s in ", count, PLURAL(count));
    if (format_days > 0)
        fprintf(file, "%ld day%s, ", format_days, PLURAL(format_days));
    if (format_hours > 0)
        fprintf(file, "%ld hour%s, ", format_hours, PLURAL(format_hours));
    if (format_minutes > 0)
        fprintf(file, "%ld minute%s, ", format_minutes, PLURAL(format_minutes));
    fprintf(file, "%ld second%s\n", format_seconds, PLURAL(format_seconds));
    
    double keys_per_second = ((double) count) / elapsed_seconds;
    double keys_per_minute = keys_per_second * 60;
    double keys_per_hour = keys_per_minute * 60;
    double keys_per_day = keys_per_hour * 24;
    
    fprintf(file, "You pressed %.2f key%s per second\n", keys_per_second, PLURAL(keys_per_second));
    fprintf(file, "You pressed %.2f key%s per minute\n", keys_per_minute, PLURAL(keys_per_minute));
    fprintf(file, "You pressed %.2f key%s per hour\n", keys_per_hour, PLURAL(keys_per_hour));
    fprintf(file, "You pressed %.2f key%s per day\n", keys_per_day, PLURAL(keys_per_day));
    
    if (file != stdout)
        fclose(file);
}

void signal_hup(int s)
{
    print_stats();
}

void daemonize(const char *option_pidfile)
{
    // Fork
    pid_t pid = fork();
    if (pid < 0) {
        fprintf(stderr, "Error: Could not fork\n");
        exit(1);
    } else if (pid > 0)
        exit(0); // Exit parent
    
    // Write out PID file
    if (option_pidfile) {
        pid_t pid = getpid();
        FILE *pidfile = fopen(option_pidfile, "w");
        if (!pidfile) {
            perror(option_pidfile);
            exit(1);
        }
        fprintf(pidfile, "%d\n", pid);
        fclose(pidfile);
    }
    
    // Detach from controlling terminal
    setsid();
    
    // Close all file descriptors
    for (int i = getdtablesize(); i >= 0; i--)
        close(i);
    
    // Open stdin, stdout, stderr to /dev/null
    int fd = open("/dev/null", O_RDWR);
    dup(fd);
    dup(fd);
}

void print_usage(const char *exec_name)
{
    printf("Usage: %s [OPTION]... [FILE]\n\n", exec_name);
    printf("  -f, --follow          output keypress statistics as they update\n");
    printf("  -d, --daemonize       run in the background\n");
    printf("  -p, --pid-file=FILE   file to write PID to\n");
    printf("  -h, --help            display this help and exit\n");
}

int main(int argc, char** argv)
{
    bool option_follow = false, option_daemonize = false;
    char *option_pidfile = NULL;
    
    static struct option long_options[] = {
        {"follow", no_argument, NULL, 'f'},
        {"daemonize", no_argument, NULL, 'd'},
        {"pid-file", required_argument, NULL, 'p'},
        {"help", no_argument, NULL, 'h'},
        {0, 0, 0, 0}
    };
    
    char c;
    while ((c = getopt_long(argc, argv, "fdp:h", long_options, NULL)) != -1) {
        switch(c) {
        case 'f':
            option_follow = true;
            break;
        case 'd':
            option_daemonize = true;
            break;
        case 'p':
            option_pidfile = optarg;
            break;
        case 'h':
            print_usage(argv[0]);
            return 0;
        case '?':
            return 1;
        }
    }
    
    if (option_follow && option_daemonize) {
        fprintf(stderr, "Error: Cannot follow and daemonize\n");
        return 1;
    }
    
    if (optind < argc)
        option_file = argv[optind];
    
    if (option_daemonize)
        daemonize(option_pidfile);
    
    signal(SIGTERM, signal_quit);
    signal(SIGQUIT, signal_quit);
    signal(SIGINT, signal_quit);
    
    signal(SIGHUP, signal_hup);
    signal(SIGUSR1, signal_hup);
    
    // FIXME: Perhaps move this to before daemonization, so the error is
    // visible
    Display *display = XOpenDisplay(NULL);
    if (!display) {
        fprintf(stderr, "Error: Could not open X display\n");
        return 1;
    }
    
    char keys_current[32], keys_last[32];
    count = 0;
    start_time = time(NULL);
    
    if (option_follow) {
        printf(TERM_CLEAR TERM_HOME);
        print_stats();
    }
    
    XQueryKeymap(display, keys_last);
    while (!end) {
        fflush(stdout);
        usleep(5000);
        XQueryKeymap(display, keys_current);
        for (int i = 0; i < 32; i++) {
            if (keys_current[i] != keys_last[i] && keys_current[i] != 0) {
                count++;
                if (option_follow) {
                    printf(TERM_CLEAR TERM_HOME);
                    print_stats();
                }
            }
            keys_last[i] = keys_current[i];
        }
    }
    XCloseDisplay(display);
    
    if (!option_follow)
        print_stats();
    
    return 0;
}
