#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define NUMOFFILES 3
#define SYSCOMMAND 256
void run_program(const char *program_path);

/**
 * @brief Main function that runs multiple executable programs.
 * 
 * This function defines paths to three executable programs ('server.out', 'dataBase.out', 'admin.out')
 * and runs each of them using the run_program function.
 * 
 * @return Returns 0 on successful execution.
 */
int main() {
    // Define the paths to the three programs
    const char *program_paths[] = {
        "./server.out",
        "./dataBase.out",
        "./admin.out"
    };

    // Run each program
    for (int i = 0; i < NUMOFFILES; i++) {
        run_program(program_paths[i]);
    }

    return 0;
}

/**
 * @brief Runs a program specified by its path in a new terminal window.
 * 
 * This function checks if the specified program exists and is executable. If so,
 * it opens the program in a new terminal window using 'gnome-terminal'.
 * 
 * @param program_path Path to the executable program.
 */
void run_program(const char *program_path) {
    if (access(program_path, X_OK) == 0) {
        char command[SYSCOMMAND];
        // Open the program in a new terminal window
        sprintf(command, "gnome-terminal -- %s", program_path);
        if (system(command) != 0) {
            fprintf(stderr, "Error occurred while running %s\n", program_path);
        }
    } else {
        fprintf(stderr, "Program %s not found.\n", program_path);
    }
}