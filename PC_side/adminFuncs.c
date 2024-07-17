#include "adminFuncs.h"


void calculateTotalTime(const char *time_in, const char *time_out, int *total_time) {
    struct tm tm_in = {0}, tm_out = {0};

    // Parse time_in and time_out strings without date component
    if (strptime(time_in, "%H:%M:%S", &tm_in) != NULL && strptime(time_out, "%H:%M:%S", &tm_out) != NULL) {
        // Parsing succeeded
    } else {
        fprintf(stderr, "Error parsing time_in or time_out\n");
        exit(1);
    }

    // Calculate total time difference in seconds
    *total_time = difftime(mktime(&tm_out), mktime(&tm_in));
}


void addLineToDatabase(int number, const char *time_in, const char *time_out, double price) {
    sqlite3 *db;
    char *err_msg = 0;
    int rc;
    double total_price;
    int total_time; // Total time in seconds

    // Connect to the database
    rc = sqlite3_open(DB_FILE, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }
    calculateTotalTime(time_in, time_out, &total_time);
    total_price = price * total_time;

    // Prepare the SQL statement for inserting a new entry
    char sql[SQLNEWENTERY];
    sprintf(sql, "INSERT INTO entries (number, time_in, time_out, price, total_price, total_time) VALUES (%d, '%s', '%s', %lf, %lf, %d);", number, time_in, time_out, price, total_price, total_time);
    
    // Execute the SQL statement
    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        exit(1);
    }

    // Close the database
    sqlite3_close(db);
}


void removeLineFromDatabase(int id) {
    sqlite3 *db;
    sqlite3_stmt *stmt;
    char *err_msg = 0;
    int rc;

    // Connect to database
    rc = sqlite3_open(DB_FILE, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }

    // Prepare SQL statement for deleting an entry
    const char *sql = "DELETE FROM entries WHERE id = ?";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }

    // Bind the ID parameter
    sqlite3_bind_int(stmt, 1, id);

    // Execute the prepared statement
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }

    // Finalize the statement and close the database
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}


void createDatabase() {
    sqlite3 *db;
    char *err_msg = 0;
    int rc;

    rc = sqlite3_open(DB_FILE, &db);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }

    char *sql = "CREATE TABLE IF NOT EXISTS entries ("
                "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                "number INTEGER, "
                "time_in TEXT, "
                "time_out TEXT, "
                "price REAL, "
                "total_time INTEGER, " // Changed to INTEGER for seconds
                "total_price REAL);";

    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        exit(1);
    }

    sqlite3_close(db);
}


double getPriceFromPrices(int line_number) {
    FILE *file = fopen(PRICES_FILE, "r");
    if (!file) {
        perror("Error opening prices file");
        exit(1);
    }

    double price;
    int current_line = 1;

    // Move to the desired line
    while (current_line < line_number) {
        // Read characters until a newline character is encountered
        while (fgetc(file) != '\n') {
            if (feof(file)) {
                // Reached end of file before reaching the desired line
                fclose(file);
                return -1; // Indicate price not found
            }
        }
        current_line++;
    }

    // Read the price from the desired line
    if (fscanf(file, "%lf", &price) != 1) {
        // Error occurred while reading price
        fclose(file);
        return -1; // Indicate price not found
    }

    fclose(file);
    return price;
    }

 
void updateDatabaseFromPrices() {
    sqlite3 *db;
    char *err_msg = 0;
    int rc;

    // Connect to the database
    rc = sqlite3_open(DB_FILE, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }

    // Prepare SQL statement for updating prices
    const char *sql = "UPDATE entries SET price = ? WHERE number = ?";

    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }

    // Update database with prices from prices file
    for (int number = 1; ; number++) {
        double price = getPriceFromPrices(number);
        if (price == -1) {
            // No more prices in the file
            break;
        }

        // Bind parameters to prepared statement
        sqlite3_bind_double(stmt, 1, price);
        sqlite3_bind_int(stmt, 2, number);

        // Execute the prepared statement
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            exit(1);
        }

        // Reset the prepared statement for the next iteration
        sqlite3_reset(stmt);
    }

    // Update total_price based on the existing total_time for each entry
    sql = "UPDATE entries SET total_price = price * total_time WHERE total_time IS NOT NULL";
    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        exit(1);
    }

    // Close database and finalize statement
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}


void updatePriceForNumber(int number, double new_price) {
    // Open the prices file
    FILE *prices_file = fopen("prices.txt", "r");
    if (prices_file == NULL) {
        fprintf(stderr, "Error: Unable to open prices.txt\n");
        exit(1);
    }

    // Create a temporary file to store updated prices
    FILE *temp_file = fopen("temp.txt", "w");
    if (temp_file == NULL) {
        fprintf(stderr, "Error: Unable to create temp file\n");
        fclose(prices_file);
        exit(1);
    }

    // Loop through each line in the prices file
    char line[SQLNEWENTERY];
    int current_number = 0;
    while (fgets(line, sizeof(line), prices_file)) {
        current_number++;
        if (current_number == number) {
            // Update the price for the given number
            fprintf(temp_file, "%.2lf\n", new_price);
        } else {
            // Copy the line as is to the temporary file
            fprintf(temp_file, "%s", line);
        }
    }

    // Close both files
    fclose(prices_file);
    fclose(temp_file);

    // Remove the original prices file
    remove("prices.txt");

    // Rename the temporary file to prices.txt
    rename("temp.txt", "prices.txt");
}