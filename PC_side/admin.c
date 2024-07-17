#include "adminFuncs.h"

/**
 * @brief Main function to handle user interaction for database management.
 * 
 * This function connects to a SQLite database, then enters a loop to continuously
 * prompt the user for commands to add entries, remove entries, update prices,
 * or quit the program.
 * 
 * @return 0 on successful execution.
 */
int main() {

    char command;
    int number;
    char time_in[20];
    char time_out[20];
    double price;
    int total_time;
    double total_price;

    // Connect to database
    createDatabase();
    sqlite3 *db;
    if (sqlite3_open(DB_FILE, &db) != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }

    // Main loop to continuously listen for user commands
    while (PROGRAMRUNNING) {
        printf("Enter command (a: add line, r: remove line, u: update prices, q: quit): ");
        scanf(" %c", &command);

        switch (command) {
            case 'a':
                printf("Enter number: ");
                scanf("%d", &number);
                price = getPriceFromPrices(number);
                if (price < 0) {
                    printf("Error: Price not found for number %d in prices.txt.\n", number);
                    break;
                }
                printf("Enter time in: ");
                scanf("%s", time_in);
                printf("Enter time out: ");
                scanf("%s", time_out);
                if (strcmp(time_in, time_out) >= 0) {
                    printf("Error: Time in should be before time out.\n");
                    break;
                }
                addLineToDatabase(number, time_in, time_out, price);
                break;
            case 'r':
                printf("Enter id to remove: ");
                scanf("%d", &number);
                removeLineFromDatabase(number);
                break;
            case 'u':
                // Update price for a specific number
                printf("Enter number to update price: ");
                scanf("%d", &number);
                printf("Enter new price: ");
                scanf("%lf", &price);
                updatePriceForNumber(number, price);
                updateDatabaseFromPrices();
                break;
            case 'q':
                printf("Exiting admin process...\n");
                sqlite3_close(db);
                exit(0);
            default:
                printf("Invalid command. Please try again.\n");
                break;
        }
    }

    return 0;
}
