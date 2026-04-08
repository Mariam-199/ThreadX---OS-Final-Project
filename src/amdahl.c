#include <stdio.h>      // For printf(), scanf(), snprintf()
#include <stdlib.h>     // For exit()
#include <unistd.h>     // For fork(), execl()
#include <sys/wait.h>   // For waitpid()

/*
 * This function sends a message to the centralized logger.
 * It creates a child process and runs the logger executable.
 */
void log_message(const char *message) {
    pid_t pid = fork();   // Create a child process for logging

    // Check if fork failed
    if (pid < 0) {
        perror("fork failed for logger");
        return;
    }

    // Child process runs the logger
    if (pid == 0) {
        execl("./logger", "logger", message, NULL);

        // This only runs if execl fails
        perror("execl failed for logger");
        exit(1);
    }

    // Parent waits for logger child to finish
    waitpid(pid, NULL, 0);
}

/*
 * This function clears leftover invalid input from the keyboard buffer.
 */
void clear_input_buffer(void) {
    int c;   // Variable used to read one character at a time

    // Keep reading until newline or end-of-file
    while ((c = getchar()) != '\n' && c != EOF) {
        /* discard extra input */
    }
}

/*
 * This function safely reads a decimal number from the user.
 */
double read_double(const char *prompt) {
    double value;   // Variable to store the decimal input

    while (1) {
        // Show the prompt
        printf("%s", prompt);

        // Try to read a double value
        if (scanf("%lf", &value) == 1) {
            clear_input_buffer();   // Clear leftover characters
            return value;           // Return valid input
        }

        // If input is invalid, show error
        printf("Invalid input. Please enter a decimal number.\n");

        // Clear bad input from buffer
        clear_input_buffer();
    }
}

/*
 * This function safely reads an integer from the user.
 */
int read_int(const char *prompt) {
    int value;   // Variable to store integer input

    while (1) {
        // Show the prompt
        printf("%s", prompt);

        // Try to read an integer
        if (scanf("%d", &value) == 1) {
            clear_input_buffer();   // Clear leftover characters
            return value;           // Return valid input
        }

        // If input is invalid, show error
        printf("Invalid input. Please enter an integer.\n");

        // Clear bad input
        clear_input_buffer();
    }
}

/*
 * Main function for the Amdahl's Law module.
 */
int main(void) {
    double P;              // Parallel fraction
    int N;                 // Number of cores
    double speedup;        // Calculated speedup
    double serial_time;    // Serial execution time entered by user
    double parallel_time;  // Estimated parallel execution time
    char logbuf[256];      // Buffer for building log messages

    // Log that the module has started
    log_message("AMDAHL MODULE: started");

    // Display module title
    printf("=== Amdahl's Law Calculator ===\n");

    // Read and validate parallel fraction
    P = read_double("Enter parallel fraction (0 to 1): ");
    while (P < 0 || P > 1) {
        printf("Invalid input. Parallel fraction must be between 0 and 1.\n");
        P = read_double("Enter parallel fraction (0 to 1): ");
    }

    // Read and validate number of cores
    N = read_int("Enter number of cores: ");
    while (N <= 0) {
        printf("Invalid input. Number of cores must be greater than 0.\n");
        N = read_int("Enter number of cores: ");
    }

    // Read and validate serial execution time
    serial_time = read_double("Enter serial execution time: ");
    while (serial_time <= 0) {
        printf("Invalid input. Serial execution time must be greater than 0.\n");
        serial_time = read_double("Enter serial execution time: ");
    }

    /*
     * Apply Amdahl's Law formula:
     * Speedup = 1 / [ (1 - P) + (P / N) ]
     */
    speedup = 1.0 / ((1.0 - P) + (P / N));

    // Calculate estimated parallel time
    parallel_time = serial_time / speedup;

    // Display calculation results
    printf("\n--- Results ---\n");
    printf("Parallel Fraction (P): %.2f\n", P);
    printf("Number of Cores (N): %d\n", N);
    printf("Speedup: %.4f\n", speedup);

    // Display comparison of serial vs parallel execution
    printf("\n--- Execution Comparison ---\n");
    printf("Serial Execution Time: %.2f\n", serial_time);
    printf("Parallel Execution Time: %.4f\n", parallel_time);
    printf("Time Saved: %.4f\n", serial_time - parallel_time);

    // Helpful note if there is no parallel part
    if (P == 0) {
        printf("\nNote: No part of the program is parallel, so there is no speed improvement.\n");
    }

    // Helpful note if everything is parallel
    if (P == 1) {
        printf("\nNote: The whole program is parallel, so speedup approaches the number of cores.\n");
    }

    // Build a final log message with the calculation details
    snprintf(logbuf, sizeof(logbuf),
             "AMDAHL MODULE: P=%.2f N=%d serial=%.2f speedup=%.4f parallel=%.4f",
             P, N, serial_time, speedup, parallel_time);

    // Send the detailed result to the logger
    log_message(logbuf);

    // Log that the module finished successfully
    log_message("AMDAHL MODULE: finished successfully");

    // Return success
    return 0;
}
