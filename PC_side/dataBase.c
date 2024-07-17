#include "dataBaseFuncs.h"


/**
 * @brief Main function for the parking system.
 * 
 * This function initializes shared memory, listens for changes in shared memory,
 * processes check-in and check-out actions based on incoming data, and save it in a database file.
 * 
 * @return Returns 0 on successful execution, 1 if there's an error in shared memory operations.
 */
int main() {
    int shmid;
    key_t key;
    char *shm_ptr;
    double totalPrice = 0.0; // Initialize totalPrice
    char prev_shm[SHM_SIZE] = ""; // Variable to store previous shared memory content
    int number;
    char entryType;
    double shm_value;
    
    key = ftok("/tmp", 'S');
    if (key == -1) {
        perror("ftok");
        return 1;
    }

    shmid = shmget(key, SHM_SIZE, 0666);
    if (shmid == -1) {
        perror("shmget");
        return 1;
    }

    shm_ptr = shmat(shmid, NULL, 0);
    if (shm_ptr == (char *) -1) {
        perror("shmat");
        return 1;
    }

    printf("Shared memory attached at %p\n", shm_ptr);

    while (PROGRAMRUNNING) {
        // Wait for the shared memory to change
        while (SHAREDMEMOCHANGE) {
            if (strcmp(shm_ptr, prev_shm) != 0) {
                strcpy(prev_shm, shm_ptr); // Update previous shared memory content
                break;
            }
           else
              usleep(100000); // Sleep for 100 ms before checking again
        }

        printf("Shared memory changed. %s\n", shm_ptr);
        sscanf(shm_ptr, "%d%c", &number, &entryType);

        if (entryType == 'q') {
            break; // Exit the loop if 'q' is entered
        }

        if (entryType != 'I' && entryType != 'O') {
            printf("Invalid entry type. Please enter 'I' or 'O'.\n");
            continue; // Skip this entry and continue with the next one
        }

        if (entryType == 'I') {
            handleCheckin(number);
            
        } else if (entryType == 'O') {
            handleCheckout(number);
            struct Entry rowData = getRowDataForNumber(number);     
            usleep(100000); // Sleep for 100 ms before checking again
            sprintf(shm_ptr, "Number: %d, Time In: %s, Time Out: %s, Price: %.2f, Total Time: %d seconds, Total Price: %.2f",
            rowData.number, rowData.time_in, rowData.time_out,
            rowData.price, rowData.total_time, rowData.total_price);
        }
    }

    printf("Exiting program.\n");

    // Detach the shared memory segment
    shmdt(shm_ptr);

    return 0;
}

