#include <iostream>
#include <fstream>
#include <list>
#include <cstring>
#include <regex>
#include "nlohmann/json.hpp"
#include "Random.h"


using json = nlohmann::json;

namespace Word {
    struct Meaning
    {
        std::string_view def {""};
        std::string_view example {""};
        std::string_view speech_part {""};
        std::vector<std::string_view> synonyms {};

        Meaning() = default;
        /* Meaning(std::string def_, std::string example_, std::string speech_part_, std::vector<std::string> synonyms_)
        : def(std::move(def_)), example(std::move(example_)), speech_part(std::move(speech_part_)), synonyms(synonyms_) */
    };
    struct Word
    {
        std::string_view name {};
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
    std::vector<std::string_view> m_keys;

    // hash map of word to meanings
    // to get meanings: std::get<0>(m_words[word].meanings)
    std::unordered_map<std::string_view, Word::Word> m_words;

    public:

    int get_dict_size()
    {
        return static_cast<int>(m_keys.size());
    }

    Word::Word get_word(std::string_view word)
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
        std::string_view word {m_keys[static_cast<std::size_t>(Random::get(0, static_cast<int>(m_keys.size() - 1)))]};
        return m_words[word];
    }
};



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
                    temp_word.name = c.template get<std::string_view>();

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
