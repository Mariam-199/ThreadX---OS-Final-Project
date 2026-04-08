#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*
 * logger.c
 * --------
 * A simple centralized logger.
 *
 * Other executables call this program using fork() + exec().
 * This program then appends a timestamped log entry to ../logs/logs.txt.
 *
 * Usage:
 *   ./logger "message to record"
 *Author: Yusupha Njie
 */
int main(int argc, char *argv[]) {
    FILE *fp;
    time_t now;
    struct tm *local;
    char time_buffer[64];

    if (argc < 2) {
        fprintf(stderr, "Logger error: no message provided.\n");
        return 1;
    }

    /* Open the central log file in append mode. */
    fp = fopen("../logs/logs.txt", "a");
    if (fp == NULL) {
        perror("Unable to open ../logs/logs.txt");
        return 1;
    }

    /* Get current local time for timestamping. */
    now = time(NULL);
    local = localtime(&now);
    if (local == NULL) {
        fprintf(stderr, "Logger error: could not read local time.\n");
        fclose(fp);
        return 1;
    }

    /* Convert time into a readable format. */
    strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", local);

    /* Write one full log line. */
    fprintf(fp, "[%s] %s\n", time_buffer, argv[1]);
    fclose(fp);

    return 0;
}
