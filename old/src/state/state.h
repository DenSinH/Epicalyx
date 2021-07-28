#ifndef EPICALYX_STATE_H
#define EPICALYX_STATE_H

#include <list>
#include <string>
#include <iostream>

#define INFILE_ARGS std::shared_ptr<const std::string> file, size_t line_no, std::shared_ptr<const std::string> line
#define INFILE_VALUES file, line_no, line
#define INFILE_CONSTRUCTOR InFile(INFILE_VALUES)

class InFile {
public:
    InFile(INFILE_ARGS) :
        File(file),
        LineNo(line_no),
        Line(line) {
    }

    std::string Loc() const {
        return *File + ":" + std::to_string(LineNo) + ": " + *Line;
    }

    const std::shared_ptr<const std::string> Line;
    const size_t LineNo;
    const std::shared_ptr<const std::string> File;
};


class Stateful {
public:
    template<typename S>
    auto context(const S& context) {
        struct Context {
            Context(Stateful& s, const S& context) : state(s) {
                state.Stack.push_back(context);
            }

            ~Context() {
                state.Stack.pop_back();
            }

            Stateful& state;
        };

        return Context(*this, context);
    }

    std::string Trace() {
        std::string trace;
        for (auto& s : Stack) {
            trace += "\nwhile " + s;
        }
        return trace;
    }

    void Dump() {
        std::cout << Trace() << std::endl;
    }

private:
    std::list<std::string> Stack;
};

#endif //EPICALYX_STATE_H