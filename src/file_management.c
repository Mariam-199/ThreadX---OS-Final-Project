#include <stdio.h>      // For input/output functions (printf, perror)
#include <stdlib.h>     // For exit()
#include <unistd.h>     // For system calls (read, write, close, unlink, fork, execl)
#include <fcntl.h>      // For open() and file flags (O_CREAT, O_RDONLY, etc.)
#include <sys/stat.h>   // For file permission modes (S_IRUSR, S_IWUSR)
#include <dirent.h>     // For directory operations (opendir, readdir)
#include <string.h>     // For string functions (strlen, strcmp, strcspn)
#include <sys/wait.h>   // For waitpid()

#define MAX 1000        // Maximum buffer size for file content
#define MAX_NAME 100    // Maximum length of file name

/*
 * This function sends a message to the centralized logger.
 * It creates a child process and executes the logger program.
 */
void log_message(const char *message) {
    pid_t pid = fork();   // Create a child process

    // If fork fails
    if (pid < 0) {
        perror("fork failed for logger");
        return;
    }

    // Child process
    if (pid == 0) {
        // Replace child process with logger program
        execl("./logger", "logger", message, NULL);

        // Only runs if execl fails
        perror("execl failed for logger");
        exit(1);
    }

    // Parent waits for logger process to finish
    waitpid(pid, NULL, 0);
}

/*
 * Clears leftover input from the keyboard buffer.
 * Prevents issues when user enters invalid input.
 */
void clear_input_buffer(void) {
    int c;

    // Keep reading characters until newline or end-of-file
    while ((c = getchar()) != '\n' && c != EOF) {
        /* discard */
    }
}

/*
 * Reads a full line of input safely (including spaces).
 */
void read_line(const char *prompt, char *buffer, int size) {
    printf("%s", prompt);   // Show prompt message

    // Read input from user
    if (fgets(buffer, size, stdin) != NULL) {

        // Remove newline character if present
        buffer[strcspn(buffer, "\n")] = '\0';
    }
}

/*
 * Creates a new file inside ../files/
 */
void createFile() {
    char filename[MAX_NAME];         // Stores file name
    char filepath[MAX_NAME + 20];    // Stores full path (../files/filename)
    char logbuf[256];                // Buffer for log message
    int fd;                          // File descriptor

    // Get filename from user
    read_line("Enter filename to create: ", filename, sizeof(filename));

    // Build full path
    snprintf(filepath, sizeof(filepath), "../files/%s", filename);

    // Create file (write only, truncate if exists)
    fd = open(filepath, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);

    // If open failed
    if (fd < 0) {
        printf("Error creating file.\n");
        log_message("FILE MODULE: failed to create file");
        return;
    }

    // Close file after creation
    close(fd);

    printf("File \"%s\" created successfully.\n", filename);

    // Log the action
    snprintf(logbuf, sizeof(logbuf), "FILE MODULE: created file '%s'", filename);
    log_message(logbuf);
}

/*
 * Reads and displays file content
 */
void readFile() {
    char filename[MAX_NAME];
    char filepath[MAX_NAME + 20];
    char buffer[MAX];
    char logbuf[256];
    int fd, bytesRead;

    read_line("Enter filename to read: ", filename, sizeof(filename));

    snprintf(filepath, sizeof(filepath), "../files/%s", filename);

    // Open file in read-only mode
    fd = open(filepath, O_RDONLY);

    if (fd < 0) {
        printf("File does not exist.\n");
        log_message("FILE MODULE: failed to read file");
        return;
    }

    printf("\n--- File Content ---\n");

    // Read file content into buffer
    bytesRead = read(fd, buffer, sizeof(buffer) - 1);

    if (bytesRead < 0) {
        printf("Error reading file.\n");
        close(fd);
        return;
    }

    // Add null terminator so it prints correctly
    buffer[bytesRead] = '\0';

    printf("%s\n", buffer);

    close(fd);

    snprintf(logbuf, sizeof(logbuf), "FILE MODULE: read file '%s'", filename);
    log_message(logbuf);
}

/*
 * Writes text into a file
 */
void writeFile() {
    char filename[MAX_NAME];
    char filepath[MAX_NAME + 20];
    char content[MAX];
    char logbuf[256];
    int fd;

    read_line("Enter filename to write: ", filename, sizeof(filename));

    snprintf(filepath, sizeof(filepath), "../files/%s", filename);

    // Open file in append mode
    fd = open(filepath, O_WRONLY | O_APPEND);

    if (fd < 0) {
        printf("File does not exist.\n");
        log_message("FILE MODULE: failed to write");
        return;
    }

    // Get content from user
    read_line("Enter content:\n", content, sizeof(content));

    // Write content to file
    write(fd, content, strlen(content));

    // Add newline
    write(fd, "\n", 1);

    close(fd);

    printf("Content written successfully.\n");

    snprintf(logbuf, sizeof(logbuf), "FILE MODULE: wrote to '%s'", filename);
    log_message(logbuf);
}

/*
 * Deletes a file
 */
void deleteFile() {
    char filename[MAX_NAME];
    char filepath[MAX_NAME + 20];
    char logbuf[256];

    read_line("Enter filename to delete: ", filename, sizeof(filename));

    snprintf(filepath, sizeof(filepath), "../files/%s", filename);

    // unlink removes file
    if (unlink(filepath) == 0) {
        printf("File deleted successfully.\n");

        snprintf(logbuf, sizeof(logbuf), "FILE MODULE: deleted '%s'", filename);
        log_message(logbuf);
    } else {
        printf("Error deleting file.\n");
        log_message("FILE MODULE: delete failed");
    }
}

/*
 * Lists all files in ../files/
 */
void listFiles() {
    DIR *dir;
    struct dirent *entry;

    // Open directory
    dir = opendir("../files");

    if (dir == NULL) {
        printf("Cannot open directory.\n");
        return;
    }

    printf("\n--- Files ---\n");

    // Loop through directory entries
    while ((entry = readdir(dir)) != NULL) {

        // Skip "." and ".."
        if (strcmp(entry->d_name, ".") != 0 &&
            strcmp(entry->d_name, "..") != 0) {

            printf("%s\n", entry->d_name);
        }
    }

    closedir(dir);

    log_message("FILE MODULE: listed files");
}

/*
 * Main menu for file management module
 */
int main(void) {
    int choice;

    log_message("FILE MODULE: started");
    //// keep repeating menu until user chooses 0
    do {
        printf("\n=== File Management ===\n");
        printf("1. Create File\n");
        printf("2. Read File\n");
        printf("3. Write File\n");
        printf("4. Delete File\n");
        printf("5. List Files\n");
        printf("0. Exit\n");

        printf("Enter your choice: ");

        // Validate integer input
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input.\n");
            log_message("FILE MODULE: invalid input");
            clear_input_buffer();
            continue;
        }

        clear_input_buffer();
        
        // if-else version of menu handling
        if (choice == 1) createFile();
        else if (choice == 2) readFile();
        else if (choice == 3) writeFile();
        else if (choice == 4) deleteFile();
        else if (choice == 5) listFiles();
        else if (choice == 0) {
            printf("Exiting...\n");
            log_message("FILE MODULE: exited");
        } else {
            printf("Invalid choice.\n");
        }

    } while (choice != 0);

    return 0;
}
