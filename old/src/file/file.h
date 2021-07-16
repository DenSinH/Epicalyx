#ifndef EPICALYX_FILE_H
#define EPICALYX_FILE_H

#include <string>
#include <fstream>
#include <vector>

class File {
public:
    File(const std::string& file_name) : FileName(std::make_shared<const std::string>(file_name)) {
        std::ifstream file(file_name);
        if (!file.good()) {
            throw std::runtime_error("Could not open file!");
        }

        std::string line;
        while (std::getline(file, line)) {
            Lines.push_back(std::make_shared<std::string>(line));
        }
    }

    const std::shared_ptr<const std::string> FileName;
    std::vector<std::shared_ptr<std::string>> Lines;
};

#endif //EPICALYX_FILE_H
