#include <iostream>
#include <fstream>
#include <list>
#include <cstring>
#include <regex>
#include "nlohmann/json.hpp"
#include "external/sqlite/sqlite3.h"
#include "Random.h"


using json = nlohmann::json;

namespace Word {
    struct Meaning
    {
        std::string def {""};
        std::string example {""};
        std::string speech_part {""};
        std::vector<std::string> synonyms {};

        Meaning() = default;
        /* Meaning(std::string def_, std::string example_, std::string speech_part_, std::vector<std::string> synonyms_)
        : def(std::move(def_)), example(std::move(example_)), speech_part(std::move(speech_part_)), synonyms(synonyms_) */
    };
    struct Word
    {
        std::string name {};
        std::vector<Meaning> meanings;
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Meaning, def, example, speech_part, synonyms)
}

class Dictionary
{
    private:

    // a vector of keys (word names) so that we can efficiently pull a random word

    /*
    If you want to remove an element from the map after it was selected, swap
    the key you selected with the back() element of your key vector and call pop_back
    (), after that erase the element from the map and return the value - takes constant time
    */
    std::vector<std::string> m_keys;

    // hash map of word to meanings
    // to get meanings: std::get<0>(m_words[word].meanings)
    std::unordered_map<std::string, Word::Word> m_words;

    public:

    int get_dict_size()
    {
        return static_cast<int>(m_keys.size());
    }

    Word::Word get_word(std::string word)
    {
        return m_words[word];
    }
    
    void add_word(Word::Word word)
    {
        // add to key list
        m_keys.emplace_back(word.name);
        // add to words list
        m_words[word.name] = word;
    }

    Word::Word get_random_word()
    {
        std::string word {m_keys[static_cast<std::size_t>(Random::get(0, static_cast<int>(m_keys.size() - 1)))]};
        return m_words[word];
    }

    void save_to_database(sqlite3* db);
};

void Dictionary::save_to_database(sqlite3* db)
{
    sqlite3_stmt* stmt_word;
    sqlite3_stmt* stmt_meaning;
    sqlite3_stmt* stmt_synonym;

    int rc{};

    const int batch_size = 1000;
    int count = 0;


    // Prepare the SQL statements
    const char* sql_insert_word = "INSERT INTO word (word) VALUES (?);";
    const char* sql_insert_meaning = "INSERT INTO meaning (definition, example, speech_part, word_id) VALUES (?, ?, ?, (SELECT id FROM word WHERE word = ?));";
    const char* sql_insert_synonym = "INSERT INTO synonym (synonym, meaning_id) VALUES (?, (SELECT id FROM meaning WHERE word_id = (SELECT id FROM word WHERE word = ?)));";

    rc = sqlite3_prepare_v2(db, sql_insert_word, -1, &stmt_word, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare word insert statement: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    rc = sqlite3_prepare_v2(db, sql_insert_meaning, -1, &stmt_meaning, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare meaning insert statement: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt_word);
        return;
    }

    rc = sqlite3_prepare_v2(db, sql_insert_synonym, -1, &stmt_synonym, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare synonym insert statement: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt_word);
        sqlite3_finalize(stmt_meaning);
        return;
    }


    // Begin Transaction
    // Am I loading too much into this transaction?

    rc = sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to begin transaction: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    

    for (const auto& word : m_words)
    {
        // Chat told me to define these, but I don't think I need to?
        // const std::string& word_name = word.first;
        // const Word::Word& word_word = word.second;

        // Insert word into word table
        // Note: the second argument indicates which ? you are binding to

        sqlite3_bind_text(stmt_word, 1, word.first.c_str(), -1, SQLITE_STATIC);

        // Step

        rc = sqlite3_step(stmt_word);

        // Check
        if (rc != SQLITE_DONE && rc != SQLITE_CONSTRAINT) {
            std::cerr << "Word insert execution failed: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_finalize(stmt_word);
            sqlite3_finalize(stmt_meaning);
            sqlite3_finalize(stmt_synonym);
            sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
            return;
        }
        // reset
        sqlite3_reset(stmt_word);

        // Insert meanings into meaning table

        for (const auto& meaning : word.second.meanings)
        {
            
            // bind

            // What happens if some of these are null?

            sqlite3_bind_text(stmt_meaning, 1, meaning.def.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt_meaning, 2, meaning.example.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt_meaning, 2, meaning.speech_part.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt_meaning, 2, word.first.c_str(), -1, SQLITE_STATIC);

            // step

            rc = sqlite3_step(stmt_meaning);

            // check

            if (rc != SQLITE_DONE && rc != SQLITE_CONSTRAINT) {
                std::cerr << "Meaning insert execution failed: " << sqlite3_errmsg(db) << std::endl;
                sqlite3_finalize(stmt_word);
                sqlite3_finalize(stmt_meaning);
                sqlite3_finalize(stmt_synonym);
                sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
                return;
            }

            sqlite3_reset(stmt_meaning);

            

            for (const auto& synonym : meaning.synonyms)
            {
                

                // bind
                sqlite3_bind_text(stmt_synonym, 1, synonym.c_str(), -1, SQLITE_STATIC);
                sqlite3_bind_text(stmt_synonym, 2, word.first.c_str(), -1, SQLITE_STATIC);

                // step
                rc = sqlite3_step(stmt_synonym);

                // check

                if (rc != SQLITE_DONE && rc != SQLITE_CONSTRAINT) {
                    std::cerr << "Meaning insert execution failed: " << sqlite3_errmsg(db) << std::endl;
                    sqlite3_finalize(stmt_word);
                    sqlite3_finalize(stmt_meaning);
                    sqlite3_finalize(stmt_synonym);
                    sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
                    return;
                }

                sqlite3_reset(stmt_meaning);
                }
        }
        
        std::cout << "added: " << word.first << std::endl; 

        count++;
        if (count % batch_size == 0)
        {
            rc = sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr);
            if (rc != SQLITE_OK) {
                std::cerr << "Failed to commit transaction: " << sqlite3_errmsg(db) << std::endl;
                break;
            }
            rc = sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);
            if (rc != SQLITE_OK) {
                std::cerr << "Failed to begin transaction: " << sqlite3_errmsg(db) << std::endl;
                break;
            }
        }
    
    }


    // Finalize the statements
    sqlite3_finalize(stmt_word);
    sqlite3_finalize(stmt_meaning);
    sqlite3_finalize(stmt_synonym);

     // Commit transaction
    rc = sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK)
    {
        std::cerr << "Failed to commit final transaction: " << sqlite3_errmsg(db) << std::endl;
    }

}



int main()
{


    // declare Dictionary class object
    Dictionary dictionary {};

    // open JSON dict

    std::ifstream file("test_dict.json");
    
    json::parser_callback_t cb = [](int depth, json::parse_event_t event, json & parsed)
    {
        // skip object elements with incorrect keys
        if (event == json::parse_event_t::key and parsed == json("contributors"))
        {
            return false;
        }
        else if (event == json::parse_event_t::key and parsed == json("editors"))
        {
            return false;
        }
        else if (event == json::parse_event_t::key and parsed == json("wordset_id"))
        {
            return false;
        }
        else if (event == json::parse_event_t::key and parsed == json("id"))
        {
            return false;
        }
        else if (event == json::parse_event_t::key and parsed == json("labels"))
        {
            return false;
        }
        else
        {
            return true;
        }
    };

    json j = json::parse(file, cb);


    // a is meanings and word all together
    for (auto& a : j)
    {
        Word::Word temp_word{};

        // b is multiple meanings grouped together, with word name after
        for (auto& b : a)
        {
            // c is to separate meanings, with word name after
            for (auto& c : b)
            {
                
                if (c.is_structured())
                {
                    // because this is emplace_back, meanings will be in opposite order of how we want them
                    temp_word.meanings.emplace_back(c.template get<Word::Meaning>());

                    // test
                    // std::cout << c << '\n';

                }
                else
                {
                    temp_word.name = c.template get<std::string>();

                    //test
                    // std:: cout << c << '\n';
                }

            }
            // std::cout << b << "\n\n"; 
        }

        dictionary.add_word(temp_word);
    }


    // test to see if dictionary has individual word
    // std::cout << dictionary.get_word("crimson clover").name << '\n';
    


    int number_of_random_words{10};
    for (int i = 0; i < number_of_random_words; i++)
    {
        std::cout << dictionary.get_random_word().name << '\n';
    }

    std::cout << "Dictionary size: " << dictionary.get_dict_size() << '\n';


        // Check Feature Flags and adjust dict_map

}
