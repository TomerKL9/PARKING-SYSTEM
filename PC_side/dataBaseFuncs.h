#ifndef PARKING_SYSTEM_H
#define PARKING_SYSTEM_H
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#define DB_FILE "database.db"
#define PRICES_FILE "prices.txt"
#define SHM_SIZE 1024 // Adjust the shared memory size as needed
#define SHAREDMEMOCHANGE 1 // Share memory flag for a change
#define PROGRAMRUNNING 1 // While pogram is running
#define SIZELINE 200 // size of line.
#define SIZETIME 20 // size of time parameter


struct Entry {
    int id;
    int number;
    char time_in[SIZETIME];
    char time_out[SIZETIME];
    double price;
    double total_price;
    int total_time; // Total time in seconds

};

/**
 * @brief Retrieves the current time in the format HH:MM:SS.
 * 
 * @param time_buffer Pointer to a char array where the time string will be stored.
 */
void getCurrentTime(char *time_buffer);

/**
 * @brief Calculates the total time in seconds between two time strings.
 * 
 * @param time_in Time string representing the check-in time.
 * @param time_out Time string representing the check-out time.
 * @param total_time Pointer to an integer where the total time difference will be stored.
 */
void calculateTotalTime(const char *time_in, const char *time_out, int *total_time);

/**
 * @brief Retrieves the price from the prices file based on the given line number.
 * 
 * @param number Line number in the prices file (corresponding to the price to retrieve).
 * @return The price as a double.
 */
double getPrice(int number);

/**
 * @brief Handles the check-in process for a given parking number.
 * 
 * @param number Parking number to check in.
 */
void handleCheckin(int number);

/**
 * @brief Handles the check-out process for a given parking number.
 * 
 * @param number Parking number to check out.
 */
void handleCheckout(int number);

/**
 * @brief Retrieves the total price for a parking number from the database.
 * 
 * @param number Parking number to retrieve the total price for.
 * @return The total price as a double.
 */
double getTotalPriceForNumber(int number);

/**
 * @brief Retrieves all data for the latest check-out entry of a given parking number from the database.
 * 
 * @param number Parking number to retrieve data for.
 * @return Struct containing all data fields for the specified parking number's latest check-out entry.
 */
struct Entry getRowDataForNumber(int number);

#endif // PARKING_SYSTEM_H
