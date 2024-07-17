#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define PRICES_FILE "prices.txt"
double MAX_PRICE = 10.0;
double MIN_PRICE = 2.0;


double generate_random_price();
/**
 * @brief Main function to generate prices and write them to a file.
 * 
 * This function opens a file specified by PRICES_FILE, generates 255 random prices using
 * generate_random_price(), and writes each price to a new line in the file.
 * 
 * @return Returns 0 on successful execution, 1 if there is an error opening the file.
 */
int main() {
    FILE *file = fopen(PRICES_FILE, "w");
    if (!file) {
        fprintf(stderr, "Error opening prices file for writing.\n");
        return 1;
    }

    srand(time(NULL)); // Seed the random number generator

    // Generate and write prices to the file
    for (int i = 0; i < 255; i++) {
        double price = generate_random_price();
        fprintf(file, "%.2f\n", price);
    }

    fclose(file);

    return 0;
}

/**
 * @brief Generates a random price between 2.00 and 10.00.
 * 
 * This function utilizes the rand() function and scales the result to generate a random
 * price within the specified range.
 * 
 * @return Returns a randomly generated price as a double.
 */
double generate_random_price() {
    return ((double)rand() / RAND_MAX) * (MAX_PRICE - MIN_PRICE) + MIN_PRICE;
}