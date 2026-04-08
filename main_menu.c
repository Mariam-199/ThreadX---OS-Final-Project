#include <stdio.h>      // For printf(), scanf(), snprintf()
#include <stdlib.h>     // For exit()
#include <unistd.h>     // For fork(), execl()
#include <sys/wait.h>   // For wait(), waitpid()

/*
 * This function sends a message to the centralized logger.
 * It creates a child process and runs the logger executable.
 */
void log_message(const char *message) {
    pid_t log_pid;   // Stores process ID for the logger child

    // Create a child process for the logger
    log_pid = fork();

    // If fork fails, print an error
    if (log_pid < 0) {
        perror("fork failed for logger");
        return;
    }

    // Child process runs the logger executable
    if (log_pid == 0) {
        execl("./logger", "logger", message, NULL);

        // This only runs if execl fails
        perror("execl failed for logger");
        exit(1);
    }

    // Parent waits for logger child to finish
    waitpid(log_pid, NULL, 0);
}

/*
 * This function clears leftover characters from the input buffer.
 * It helps prevent problems when the user enters invalid input.
 */
void clear_input_buffer(void) {
    int c;   // Variable used to read characters one by one

    // Keep reading until newline or end-of-file
    while ((c = getchar()) != '\n' && c != EOF) {
        /* discard extra characters */
    }
}

/*
 * Main function of the menu-driven system.
 */
int main(void) {
    int choice;          // Stores the user's menu selection
    pid_t pid;           // Stores process ID returned by fork()
    int status;          // Stores child exit status
    char logbuf[256];    // Buffer to build log messages

    // Log that the main menu has started
    log_message("MAIN MENU: started");

    // Keep showing the menu until the user chooses to exit
    while (1) {

        // Display menu title
        printf("\n=== MAIN MENU ===\n");

        // Display all module options
        printf("1. File Management\n");
        printf("2. Peterson Solution\n");
        printf("3. Memory Allocation\n");
        printf("4. Amdahl's Law\n");
        printf("0. Exit\n");

        // Ask user for input
        printf("Enter choice: ");

        /*
         * Try to read an integer.
         * If the input is not an integer, scanf returns something other than 1.
         */
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input. Please enter a number.\n");

            // Log invalid non-integer input
            log_message("MAIN MENU: invalid non-integer input entered");

            // Clear bad input from buffer
            clear_input_buffer();

            // Restart loop
            continue;
        }

        // Clear any extra input left in buffer
        clear_input_buffer();

        // If user chooses 0, exit program
        if (choice == 0) {
            printf("Exiting system...\n");

            // Log user exit
            log_message("MAIN MENU: exited by user");

            break;
        }

        // Reject invalid menu choices before creating a child
        if (choice < 0 || choice > 4) {
            printf("Invalid choice. Please try again.\n");

            // Log invalid choice
            log_message("MAIN MENU: invalid menu choice entered");

            continue;
        }

        /*
         * Log which module the user is trying to open.
         * snprintf builds the text safely into logbuf.
         */
        if (choice == 1) {
            snprintf(logbuf, sizeof(logbuf),
                     "MAIN MENU: launching File Management module");
        } else if (choice == 2) {
            snprintf(logbuf, sizeof(logbuf),
                     "MAIN MENU: launching Peterson module");
        } else if (choice == 3) {
            snprintf(logbuf, sizeof(logbuf),
                     "MAIN MENU: launching Memory Allocation module");
        } else if (choice == 4) {
            snprintf(logbuf, sizeof(logbuf),
                     "MAIN MENU: launching Amdahl module");
        }

        // Send launch message to logger
        log_message(logbuf);

        // Create a child process to run the selected module
        pid = fork();

        // If fork fails, print error and exit
        if (pid < 0) {
            perror("fork failed");
            log_message("MAIN MENU: fork failed while launching module");
            return 1;
        }

        // Child process block
        if (pid == 0) {

            // Run File Management executable
            if (choice == 1) {
                execl("./file_management", "file_management", NULL);
            }

            // Run Peterson executable
            else if (choice == 2) {
                execl("./peterson", "peterson", NULL);
            }

            // Run Memory Allocation executable
            else if (choice == 3) {
                execl("./memory", "memory", NULL);
            }

            // Run Amdahl executable
            else if (choice == 4) {
                execl("./amdahl", "amdahl", NULL);
            }

            /*
             * If execl succeeds, the child process is replaced
             * and the code below never runs.
             * So this part only happens if execl fails.
             */
            perror("execl failed");
            log_message("MAIN MENU: execl failed while launching module");
            exit(1);
        }

        // Parent process block
        else {
            // Wait for the child module to finish
            waitpid(pid, &status, 0);

            // Log that the selected module has finished
            if (choice == 1) {
                log_message("MAIN MENU: File Management module finished");
            } else if (choice == 2) {
                log_message("MAIN MENU: Peterson module finished");
            } else if (choice == 3) {
                log_message("MAIN MENU: Memory Allocation module finished");
            } else if (choice == 4) {
                log_message("MAIN MENU: Amdahl module finished");
            }
        }
    }

    // Return success
    return 0;
}
