#include <nlohmann/json.hpp>
#include <iostream>

using json = nlohmann::json;

template<class UnaryFunction>
void recursive_iterate(const json& j, UnaryFunction f)
{
    for(auto it = j.begin(); it != j.end(); ++it)
    {
        if (it->is_structured())
        {
            recursive_iterate(*it, f);
        }
        else
        {
            f(it);
        }
    }
}

int main()
{
    json j = R"({
    "Assets": [
        {
            "Asset": {
                "File": "assets/music/Music.wav",
                "Name": "Music",
                "Type": 3
            }
        },
        {
            "Asset": {
                "File": "assets/images/Image.png",
                "Name": "Success",
                "Type": 1
            }
        }
    ],
    "Layer": {
        "Asset": {
            "File": "assets/data/sub.json",
            "Name": "File",
            "Type": 0
        }
    }
}
)"_json;

    recursive_iterate(j, [](json::const_iterator it){
        std::cout << it.key() << " : " << it.value() << std::endl;
    });
}

/* OUTPUT
File : "assets/music/Music.wav"
Name : "Music"
Type : 3
File : "assets/images/Image.png"
Name : "Success"
Type : 1
File : "assets/data/sub.json"
Name : "File"
Type : 0
*/
