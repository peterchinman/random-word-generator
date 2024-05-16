#include <iostream>
#include <string>
#include "external/nlohmann/json.hpp"
#include "external/sqlite/sqlite3.h"

int execute_sql(sqlite3 *db, const std::string &sql)
{
    sqlite3_stmt* myStatement;
    const char* tail;
    int rc = sqlite3_prepare_v2(db,
        sql.c_str(),
        -1, &myStatement, &tail);

    while (rc == SQLITE_OK && myStatement)
    {
        rc = sqlite3_step(myStatement);
        if (rc != SQLITE_DONE)
        {
            std::cerr << "Execution failed: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_finalize(myStatement);
            return rc;
        }
        rc = sqlite3_prepare_v2(db, tail, -1, &myStatement, &tail);
    }

    if (myStatement)
    {
        sqlite3_finalize(myStatement);
    }

    return rc == SQLITE_DONE ? SQLITE_OK : rc;
}

int main()
{
    sqlite3 *db; // pointer to database connection

    int rc = sqlite3_open("dictionary.sqlite", &db);

    if (rc == SQLITE_OK)
    {
        std::cout << "Successfully opened the database" << std::endl;

        std::string sql = R"(
            CREATE TALBE IF NOT EXISTS word (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                word TEXT NOT NULL,
            );
            CREATE TALBE IF NOT EXISTS meaning (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                definition TEXT NOT NULL,
                example TEXT,
                speech_part TEXT,
                word_id INTEGER NOT NULL,
                FOREIGN KEY (word_id) REFERENCES word(id)
            );
            CREATE TALBE IF NOT EXISTS pronunciation (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                ARPAbet TEXT NOT NULL,
                word_id INTEGER NOT NULL,
                FOREIGN KEY (word_id) REFERENCES word(id)
            );
            CREATE TALBE IF NOT EXISTS synonym (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                synonym TEXT NOT NULL,
                meaning_id INTEGER NOT NULL,
                FOREIGN KEY (meaning_id) REFERENCES meaning(id)
            );
        )";

        rc = execute_sql(db, sql);

        if (rc != SQLITE_OK)
        {
            std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
        }
        else
        {
            std::cout << "Tables created successfully" << std::endl;
        }

        // below here unupdated
        
        
    }
    else
    {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return rc;
    }
    

    // Set-up loop to insert each word into database

    sqlite3_close(db);



    return 0;
}
