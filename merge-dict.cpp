#include <iostream>
#include <filesystem>
#include <vector>
#include <fstream>
#include "nlohmann/json.hpp"

using json = nlohmann::json;
namespace fs = std::filesystem;

int main() {
    // Define a vector to store JSON objects
    std::vector<json> jsonObjects;

    // Directory containing JSON files
    std::string directoryPath = "data/";

    // Iterate over files in the directory
    for (const auto& entry : fs::directory_iterator(directoryPath)) {
        if (entry.path().extension() == ".json") {
            // Read and parse JSON content
            std::ifstream file(entry.path());
            if (file.is_open()) {
                json jsonObject;
                file >> jsonObject;
                jsonObjects.push_back(jsonObject);
                file.close();
            } else {
                std::cerr << "Error: Unable to open file: " << entry.path() << std::endl;
            }
        }
    }

    // Merge JSON objects into one large JSON object
    json mergedJson;
    for (const auto& jsonObject : jsonObjects) {
        mergedJson.update(jsonObject);
    }

    // Write the merged JSON object to a new JSON file
    std::ofstream outputFile("merged.json");
    if (outputFile.is_open()) {
        outputFile << mergedJson.dump(4); // Pretty-print with indentation of 4 spaces
        outputFile.close();
        std::cout << "Merged JSON file created successfully." << std::endl;
    } else {
        std::cerr << "Error: Unable to create merged JSON file." << std::endl;
    }

    return 0;
}
