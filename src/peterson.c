/*
 * Peterson's solution for mutual exclusion (2 processes)
 * This program demonstrates process synchronization using
 * busy waiting. It ensures that only one process enters
 * the critical section at a time
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>   // for shared memory with mmap()

/*
 * Shared structure so both child processes see
 * the same flags and turn value.
 */
typedef struct {
    int flag[2];   // flag[i] = 1 means process i wants to enter critical section
    int turn;      // indicates whose turn it is
} SharedData;

/*
 * Logger helper function
 * ----------------------
 * This function sends a message to the centralized logger.
 * It creates a child process and runs the logger executable.
 */
void log_message(const char *message) {
    pid_t pid = fork();   // create a child process for logging

    // if fork fails, print an error and return
    if (pid < 0) {
        perror("fork failed for logger");
        return;
    }

    // child process runs the logger program
    if (pid == 0) {
        execl("./logger", "logger", message, NULL);

        // this line only runs if execl fails
        perror("execl failed for logger");
        exit(1);
    }

    // parent waits for logger child to finish
    waitpid(pid, NULL, 0);
}

/*
 * Critical section
 * ----------------
 * This simulates the part of the program where
 * only one process should execute at a time.
 */
void critical_section(int process) {
    char logbuf[128];

    printf("Process %d: ENTERING critical section\n", process);

    snprintf(logbuf, sizeof(logbuf),
             "PETERSON MODULE: Process %d entered critical section",
             process);
    log_message(logbuf);

    sleep(1); // simulate work

    printf("Process %d: EXITING critical section\n", process);

    snprintf(logbuf, sizeof(logbuf),
             "PETERSON MODULE: Process %d exited critical section",
             process);
    log_message(logbuf);
}

/*
 * Code run by each process
 * ------------------------
 * i = current process number (0 or 1)
 * data = shared flags and turn variable
 */
void process_code(SharedData *data, int i) {
    int j = 1 - i;    // other process
    char logbuf[128];

    // Entry Section: request access to critical section
    printf("Process %d: In entry section\n", i);
    snprintf(logbuf, sizeof(logbuf),
             "PETERSON MODULE: Process %d entered entry section", i);
    log_message(logbuf);

    data->flag[i] = 1;
    data->turn = j;

    // Busy waiting: wait while other process wants to enter
    // AND it is their turn
    while (data->flag[j] && data->turn == j) {
        /* busy wait */
    }

    // Critical Section
    critical_section(i);

    // Exit Section
    printf("Process %d: In exit section\n", i);
    snprintf(logbuf, sizeof(logbuf),
             "PETERSON MODULE: Process %d entered exit section", i);
    log_message(logbuf);

    data->flag[i] = 0;

    // Remainder Section
    printf("Process %d: In remainder section\n", i);
    snprintf(logbuf, sizeof(logbuf),
             "PETERSON MODULE: Process %d entered remainder section", i);
    log_message(logbuf);
}

int main() {
    pid_t p1, p2;
    SharedData *data;

    // log module start
    log_message("PETERSON MODULE: started");

    /*
     * Allocate shared memory so both child processes
     * access the same flag[] and turn variables.
     */
    data = mmap(NULL, sizeof(SharedData),
                PROT_READ | PROT_WRITE,
                MAP_SHARED | MAP_ANONYMOUS,
                -1, 0);

    // check if mmap failed
    if (data == MAP_FAILED) {
        perror("mmap failed");
        return 1;
    }

    // initialize shared variables
    data->flag[0] = 0;
    data->flag[1] = 0;
    data->turn = 0;

    p1 = fork();        // create first child process

    if (p1 == 0) {
        process_code(data, 0);
        exit(0);
    } else {
        p2 = fork();

        if (p2 == 0) {
            process_code(data, 1);
            exit(0);
        }
    }

    // parent waits for both child processes
    wait(NULL);
    wait(NULL);

    // log module end
    log_message("PETERSON MODULE: finished successfully");

    return 0;
}
