#include <iostream>
#include <string>
#include "external/nlohmann/json.hpp"
#include "external/sqlite/sqlite3.h"

int main()
{
    sqlite3 *db; // pointer to database connection

    int statusOfOpen = sqlite3_open("dictionary.sqlite", &db);

    if (statusOfOpen == SQLITE_OK)
    {
        std::cout << "Successfully opened the database" << std::endl;

        sqlite3_stmt* myStatement;
        int statusOfPrep = sqlite3_prepare_v2(db,
            "CREATE TABLE IF NOT EXISTS Students (ssn INTEGER PRIMARY KEY, first TEXT, last TEXT)",
            -1, &myStatement, NULL);
        
        if (statusOfPrep == SQLITE_OK)
        {
            int statusOfStep = sqlite3_step(myStatement);

            if (statusOfStep == SQLITE_DONE)
            {
                std::cout << "Successfully created the table" << std::endl;
            }

            else
            {
                std::cout << "Problem creating the table" << std::endl;
            }

            sqlite3_finalize(myStatement);
        }
        else
        {
            std::cout << "Problem creating a prepared statement (sqlite3_prepare_v2)" << std::endl;
        }
    }
    else
    {
        std::cout << "Problem creating a prepared statement (sqlite3_open)" << std::endl;
    }
    
    // Need to Create tables for words

    // Set-up loop to insert each word into database

    sqlite3_close(db);



    return 0;
}
