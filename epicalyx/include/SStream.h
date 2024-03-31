#pragma once 


#include <string>


namespace epi::cotyl {

struct StringStream {
    StringStream() { }
    StringStream(std::string&& base) : buffer{std::move(base)} { }
    StringStream(const std::string& base) : buffer{base} { }


    StringStream& operator<<(const char& c) {
        buffer.push_back(c);
        return *this;
    }

    StringStream& operator<<(const std::string& c) {
        buffer.append(c);
        return *this;
    }

    std::string&& finalize() {
        return std::move(buffer);
    }

    bool empty() const {
        return buffer.empty();
    }

    const std::string& current() const {
        return buffer;
    }

    void clear() {
      buffer.clear();
    }

private:
    std::string buffer{};
};

}