#include <iostream>
#include <fstream>
#include <list>
#include <cstring>
#include <regex>
#include "nlohmann/json.hpp"
#include "Random.h"


using json = nlohmann::json;

namespace Word {
    struct Meanings
    {
        std::string def {};
        std::string example {};
        std::string speech_part {};
        std::vector<std::string> synonyms {};
    };
    struct Word
    {
        std::string name {};
        std::vector<Meanings> meanings;
    };
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
        std::string word = m_keys[Random::get(0, static_cast<int>(m_keys.size()))];
        return m_words[word];
    }
};

int main()
{


    // declare Dictionary class object
    Dictionary dictionary {};

    // open JSON dict

    std::ifstream file("test_dict.json");
    json j;
    file >> j;

    // should I use this callback with the parser? does it help?
    
    json::parser_callback_t cb = [](int depth, json::parse_event_t event, json & parsed)
    {
        // skip object elements with incorrect keys
        if (event == json::parse_event_t::key and parsed == json("contributors"))
        {
            return false;
        }
        elif (event == json::parse_event_t::key and parsed == json("editors"))
        {
            return false;
        }
        elif (event == json::parse_event_t::key and parsed == json("wordset_id"))
        {
            return false;
        }
        elif (event == json::parse_event_t::key and parsed == json("id"))
        {
            return false;
        }
        else
        {
            return true;
        }
    };

    // parse (with callback) and serialize JSON
    json j_filtered = json::parse(j, cb);



    // Read Dict
        // create unordered_map
        // create word struct
        // read read json object into word struct
        // add struct to unordered_map

    // Randomly select a word
        // Check Feature Flags and adjust dict_map

}
