//BENITA TABUKU A00477447
//1)display memory map, 2) first fit allocation 3) deallocate memory
#include <stdio.h> //printf,scanf
#include <string.h>//strcpy to cpy strings, strcmp to compare strings
#include <unistd.h> //fork(), execl()
#include <stdlib.h> //exit()
#include <sys/wait.h> //waitpid()
#include <stdio.h> //snprintf()

#define MAX_BLOCKS 100 //100 holes
#define MAX_PNAME_LEN 10 //10 characters max for process name
//structure to define memory blocks
struct Block {
    int start;//where the block begins
    int size;
    int is_free;  //1= yes 0 = no
    char name[MAX_PNAME_LEN]; //process name
};

struct Block blocks[MAX_BLOCKS]; //array of 100 memory blocks
int count = 0; //zero holes initially

/*
 * Logger helper function
 * ----------------------
 * This function sends a log message to the centralized logger.
 * It creates a child process using fork().
 * The child process then runs the logger executable using execl().
 * The parent process waits until logging is finished.
 */
void log_message(const char *message) {
    pid_t pid = fork(); // create a child process for the logger

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

//1) Display memory map (allocated + free blocks)
void display_memory() {
    printf("\n------------------------------------------------------");
    printf("\n    Memory Map:    \n");
    printf("------------------------------------------------------\n");
    printf("Start\t|Size\t|Status\t\t|Process\n");
    
    
    //loop through our memory blocks
    for (int i = 0; i < count; i++) {
        //check block; if free print size and start of block
        if (blocks[i].is_free == 1)
            printf("%d\t%d\tFREE\t\t-\n", blocks[i].start, blocks[i].size);
        //if allocated print start , size and process name currently in block    
        else
            printf("%d\t%d\tALLOCATED\t%s\n", blocks[i].start, blocks[i].size, blocks[i].name);
    }

    // log that the memory map was displayed
    log_message("MEMORY MODULE: displayed memory map");
}

//2)first-fit allocation 
void memory_allocation() {
    char pname[MAX_PNAME_LEN]; //process name
    int size;
    char logbuf[256]; // buffer used to build a detailed log message

    printf("Enter process name: ");
    scanf("%s", pname);

    printf("Enter memory size needed: ");
    scanf("%d", &size);

    for (int i = 0; i < count; i++) {
        //if the block is free and it's size is greater or equal to process size
        if (blocks[i].is_free == 1 && blocks[i].size >= size) {
            //check if exact fit
            if (blocks[i].size == size) {
                //uptdate variable is_free, now the block is not available 
                blocks[i].is_free = 0;
                //put process name in block
                strcpy(blocks[i].name, pname);
            }
            //if not exact fit, shift elements to teh right to avoid overwritting data
            //bottom to top to avoid overwritting data
            else {
                for (int j = count; j > i + 1; j--) {
                    blocks[j] = blocks[j - 1];
                }
                //new block starts where block process size ends
                blocks[i + 1].start = blocks[i].start + size;
                //size of new block = original size minus size of process
                blocks[i + 1].size = blocks[i].size - size;
                //new block is free
                blocks[i + 1].is_free = 1;
                strcpy(blocks[i + 1].name, "");
                //size of original block updated to size of process
                blocks[i].size = size;
                //block allocated
                blocks[i].is_free = 0;
                strcpy(blocks[i].name, pname);
                //update count, new block allocated
                count++;
            }

            printf("Memory allocated to %s\n", pname);

            // build and send a log message for successful allocation
            snprintf(logbuf, sizeof(logbuf),
                     "MEMORY MODULE: allocated process '%s' with size %d",
                     pname, size);
            log_message(logbuf);

            return;
        }
    }

    printf("No suitable free block found for process.\n");

    // log failed allocation attempt
    log_message("MEMORY MODULE: allocation failed because no suitable free block was found");
}
//external fragmentation: we merge adjacent free blocks for (contiguous memory)

void merge_free_blocks() {
    //count -1 to compare block i to block i+1
    for (int i = 0; i < count - 1; i++) {
        //if two blocks are next to one another merge them
        if (blocks[i].is_free == 1 && blocks[i + 1].is_free == 1 && blocks[i].start + blocks[i].size == blocks[i+1].start) {
            //we merge the two adjacent blocks
            blocks[i].size += blocks[i + 1].size;
            //we shift teh remaining blocks to the left because we do not have block i+1 anymore
            for (int j = i + 1; j < count - 1; j++) {
                blocks[j] = blocks[j + 1];
            }
            //update variable because now one block less
            count--;
            i--;   // check again by staying on the same index
        }
    }
}

//3) deallocate memory 
void memory_deallocation() {
    char pname[MAX_PNAME_LEN];
    char logbuf[256]; // buffer used to build a detailed log message
    
    printf("Enter process name to deallocate: ");
    scanf("%s", pname);

    for (int i = 0; i < count; i++) {
        //we check all the allocated blocks and check the name 
        if (blocks[i].is_free == 0 && strcmp(blocks[i].name, pname) == 0) {
            blocks[i].is_free = 1;
            //remove process name from block
            strcpy(blocks[i].name, "");
            printf("Process %s deallocated.\n", pname);
            //we call the merge_free_blocks to merge free adjacent blocks
            //external fragmentation
            merge_free_blocks();

            // build and send a log message for successful deallocation
            snprintf(logbuf, sizeof(logbuf),
                     "MEMORY MODULE: deallocated process '%s'",
                     pname);
            log_message(logbuf);

            return;
        }
    }

    printf("The process was not found\n");

    // log failed deallocation attempt
    log_message("MEMORY MODULE: deallocation failed because process was not found");
}

//compaction to support external fragmentation. Expensive operation to use only if merging is not enough 
void compact_memory() {
    struct Block temp[MAX_BLOCKS]; // new memory list
    int new_count = 0; //number of blocks in new list
    int current_start = 0; //where next block will start
    int total_free = 0; //total free memory

    for (int i = 0; i < count; i++) {
        //if block allocated
        if (blocks[i].is_free == 0) {
            //copy block into new list
            temp[new_count] = blocks[i];
            temp[new_count].start = current_start;
            current_start += temp[new_count].size;
            //update variable becasue we added one more block
            new_count++;
        } else {
            //if free we combine them
            total_free += blocks[i].size;
        }
    }
    //if there is free memory
    if (total_free > 0) {
        //new memory block with combine free blocks starts after last allocated block
        temp[new_count].start = current_start;
        temp[new_count].size = total_free;
        temp[new_count].is_free = 1;
        strcpy(temp[new_count].name, "");
        new_count++;
    }

    for (int i = 0; i < new_count; i++) {
        //old memory is replaced by compoacted one
        blocks[i] = temp[i];
    }
    //change count variable
    count = new_count;
    printf("Memory compacted successfully.\n");

    // log that compaction was performed
    log_message("MEMORY MODULE: compacted memory successfully");
}

int main() {
    int n, user_choice;

    // log that the memory module has started
    log_message("MEMORY MODULE: started");

    printf("Enter number of initial memory holes: ");
    scanf("%d", &n);

    count = n; //we have n memory blocks

    for (int i = 0; i < n; i++) {
        printf("\nHole %d\n", i + 1);
        printf("Enter start address: ");
        scanf("%d", &blocks[i].start);
        printf("Enter size: ");
        scanf("%d", &blocks[i].size);

        blocks[i].is_free = 1;
        strcpy(blocks[i].name, "");
    }

    do {
        printf("\n===== MEMORY MENU =====\n");
        printf("1. Display Memory Map\n");
        printf("2. Allocate Memory\n");
        printf("3. Deallocate Memory\n");
        printf("4. Compact Memory\n");
        printf("5. Exit\n");
        printf("Enter choice: ");
        scanf("%d", &user_choice);

        switch (user_choice) {
            case 1:
                display_memory();
                break;
            case 2:
                memory_allocation();
                break;
            case 3:
                memory_deallocation();
                break;
            case 4:
                compact_memory();
                break;
            case 5:
                printf("Exited successful!\n");

                // log that the user exited the memory module
                log_message("MEMORY MODULE: exited by user");
                break;
            default:
                printf("Your choice is invalid.\n");

                // log invalid menu choice
                log_message("MEMORY MODULE: invalid menu choice entered");
        }
    } while (user_choice != 5);

    return 0;
}
