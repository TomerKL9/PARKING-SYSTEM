#include "dataBaseFuncs.h"


void getCurrentTime(char *time_buffer) {
    time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(time_buffer, 20, "%H:%M:%S", timeinfo); // Include seconds
}


void calculateTotalTime(const char *time_in, const char *time_out, int *total_time) {
    struct tm tm_in = {0}, tm_out = {0};
    time_t t_in, t_out;

    // Parse time_in and time_out strings
    strptime(time_in, "%H:%M:%S", &tm_in); // Format: HH:MM:SS
    strptime(time_out, "%H:%M:%S", &tm_out); // Format: HH:MM:SS

    // Convert tm structs to time_t
    t_in = mktime(&tm_in);
    t_out = mktime(&tm_out);

    // Calculate difference in seconds
    *total_time = difftime(t_out, t_in);
}


double getPrice(int number) {
    FILE *file = fopen(PRICES_FILE, "r");
    if (file == NULL) {
        perror("Cannot open prices file");
        exit(1);
    }

    char line[SIZELINE];
    int lineNumber = 0;
    double price = 0.0;

    while (fgets(line, sizeof(line), file)) {
        lineNumber++;
        if (lineNumber == number) {
            price = atof(line);
            break;
        }
    }

    fclose(file);
    return price;
}


void handleCheckin(int number) {
    char time_in[SIZETIME];
    double price = getPrice(number); // Get the price as double

    getCurrentTime(time_in);

    sqlite3 *db;
    int rc = sqlite3_open(DB_FILE, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open entries: %s\n", sqlite3_errmsg(db));
        return;
    }

    sqlite3_stmt *stmt;
    char *sql_max_id = "SELECT MAX(id) FROM entries;";
    sqlite3_prepare_v2(db, sql_max_id, -1, &stmt, 0);

    rc = sqlite3_step(stmt);
    int max_id = 0;
    if (rc == SQLITE_ROW) {
        max_id = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);

    char *sql_insert = "INSERT INTO entries (id, number, time_in, price) VALUES (?, ?, ?, ?);";
    sqlite3_prepare_v2(db, sql_insert, -1, &stmt, 0);
    sqlite3_bind_int(stmt, 1, max_id + 1); // Increment max_id by 1 for the new entry
    sqlite3_bind_int(stmt, 2, number);
    sqlite3_bind_text(stmt, 3, time_in, -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 4, price); // Bind price as double

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
        printf("Check-in added successfully.\n");
    } else {
        printf("SQL error: %s\n", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}


void handleCheckout(int number) {
    char time_out[SIZETIME], time_in[SIZETIME];
    double price, total_price;
    int total_time; // Total time in seconds

    getCurrentTime(time_out);

    sqlite3 *db;
    int rc = sqlite3_open(DB_FILE, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return;
    }

    sqlite3_stmt *stmt;
    char *sql = "SELECT time_in, price FROM entries WHERE number = ? ORDER BY id DESC LIMIT 1;";
    sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    sqlite3_bind_int(stmt, 1, number);
    rc = sqlite3_step(stmt);

    if (rc == SQLITE_ROW) {
        strcpy(time_in, (const char *)sqlite3_column_text(stmt, 0));
        price = sqlite3_column_double(stmt, 1);

        calculateTotalTime(time_in, time_out, &total_time);
        total_price = price * total_time;

        sqlite3_finalize(stmt);
        sql = "UPDATE entries SET time_out = ?, total_time = ?, total_price = ? WHERE number = ? AND time_out IS NULL;";
        sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
        sqlite3_bind_text(stmt, 1, time_out, -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, total_time);
        sqlite3_bind_double(stmt, 3, total_price);
        sqlite3_bind_int(stmt, 4, number);
        rc = sqlite3_step(stmt);

        if (rc == SQLITE_DONE) {
            printf("Check-out added successfully.\n");
        } else {
            printf("SQL error: %s\n", sqlite3_errmsg(db));
        }
    } else {
        printf("Check-in record not found for number %d.\n", number);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}


double getTotalPriceForNumber(int number) {
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc;
    double total_price = 0.0; // Initialize to 0.0 or any default value as needed

    // Open the SQLite database
    rc = sqlite3_open(DB_FILE, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return total_price; // Return default value
    }

    // Prepare SQL statement to fetch total_price (7th column)
    const char *sql = "SELECT total_price FROM entries WHERE number = ? AND time_out IS NOT NULL;";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return total_price; // Return default value
    }

    // Bind number parameter to the prepared statement
    sqlite3_bind_int(stmt, 1, number);

    // Execute the statement
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        // Retrieve total_price from the 1st column (index 0)
        total_price = sqlite3_column_double(stmt, 0);
    } else if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to fetch data: %s\n", sqlite3_errmsg(db));
    }

    // Finalize the statement and close the database
    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return total_price;
}


struct Entry getRowDataForNumber(int number) {
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc;
    struct Entry result = {0}; // Initialize to 0 or default values as needed

    // Open the SQLite database
    rc = sqlite3_open(DB_FILE, &db); // Replace with your DB file path
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return result; // Return default-initialized struct
    }

    // Prepare SQL statement to fetch all columns for a given number
    const char *sql = "SELECT id, number, time_in, time_out, price, total_price, total_time FROM entries WHERE number = ? AND time_out IS NOT NULL;";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return result; // Return default-initialized struct
    }

    // Bind number parameter to the prepared statement
    sqlite3_bind_int(stmt, 1, number);

    // Execute the statement
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        // Retrieve columns from the row
        result.id = sqlite3_column_int(stmt, 0);
        result.number = sqlite3_column_int(stmt, 1);
        snprintf(result.time_in, sizeof(result.time_in), "%s", (const char *)sqlite3_column_text(stmt, 2));
        snprintf(result.time_out, sizeof(result.time_out), "%s", (const char *)sqlite3_column_text(stmt, 3));
        result.price = sqlite3_column_double(stmt, 4);
        result.total_price = sqlite3_column_double(stmt, 5);
        result.total_time = sqlite3_column_int(stmt, 6);
        // Add retrieval for other columns as needed
    } else if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to fetch data: %s\n", sqlite3_errmsg(db));
    }

    // Finalize the statement and close the database
    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return result;
}
