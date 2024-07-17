#ifndef ADMINFUNCS_H
#define ADMINFUNCS_H
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sqlite3.h>
#include <time.h>

#define DB_FILE "database.db"
#define PRICES_FILE "prices.txt"
#define SQLNEWENTERY 200

#define PROGRAMRUNNING 1 // While program is running

/**
 * @brief Calculates the total time between two timestamps.
 * 
 * @param time_in Start time as a string in format HH:MM:SS.
 * @param time_out End time as a string in format HH:MM:SS.
 * @param total_time Pointer to store the calculated total time in seconds.
 */
void calculateTotalTime(const char *time_in, const char *time_out, int *total_time);

/**
 * @brief Adds a line entry to the SQLite database.
 * 
 * @param number Entry number.
 * @param time_in Start time as a string in format HH:MM:SS.
 * @param time_out End time as a string in format HH:MM:SS.
 * @param price Price per unit time.
 */
void addLineToDatabase(int number, const char *time_in, const char *time_out, double price);

/**
 * @brief Removes an entry from the SQLite database.
 * 
 * @param id ID of the entry to be removed.
 */
void removeLineFromDatabase(int id);

/**
 * @brief Creates the SQLite database and initializes the 'entries' table if it does not exist.
 */
void createDatabase();

/**
 * @brief Updates the prices in the database from the prices file.
 */   
void updateDatabaseFromPrices();

/**
 * @brief Updates the price for a specific entry number in the prices file.
 * 
 * @param number Entry number.
 * @param new_price New price to update.
 */
void updatePriceForNumber(int number, double new_price);

/**
 * @brief Retrieves the price from the prices file for a given entry number.
 * 
 * @param line_number Entry number.
 * @return Price value if found, -1 if not found.
 */
double getPriceFromPrices(int line_number);

#endif /* ADMINFUNCS_H */
