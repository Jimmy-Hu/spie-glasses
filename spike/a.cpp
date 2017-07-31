#include <iostream>
#include <vector>
#include <string>

void print_commands(std::vector<std::vector<std::string>> const & cmds)
{
    int cmd_i = 0;
    int arg_i = 0;
    for (auto const & args : cmds) {
        std::cout << "cmd[" << cmd_i << "]" << std::endl;
        arg_i = 0;
        for (auto const & s : args) {
            std::cout << "  arg[" << arg_i << "]=" << s << std::endl;
            ++arg_i;
        }
        ++cmd_i;
    }
}

struct ArgParser
{
    using iterator = std::string::iterator;
    ArgParser();
    ~ArgParser();
    std::string arg;
    bool is_empty;    // = true;
    bool qouted;      // = false;
    bool backslashed; // = false;
    void clear();
    iterator parse(iterator begin, iterator end);
    std::string & get();
    bool is_complete();
};
ArgParser::ArgParser()
    : is_empty(true)
    , qouted(false)
    , backslashed(false)
{}

ArgParser::~ArgParser()
{}

void
ArgParser::clear()
{
    is_empty = true;
    qouted = false;
    backslashed = false;
    arg.clear();
}

std::string::iterator
ArgParser::parse(std::string::iterator begin, std::string::iterator end)
{
    auto i = begin;
    for(; i != end; ++i) {
        auto c = *i;
        if (backslashed) {
            switch (c) {
            case '\n':
                break;
            default:
                arg.push_back(c);
            }
            backslashed = false;
        }
        else if (qouted) {
            switch (c) {
            case '\"':
                qouted = false;
                continue;
            case '\\':
                backslashed = true;
                continue;
            default:
                arg.push_back(c);
            }
        }
        else {
            switch (c) {
            case ' ':
            case '\t':
                if (is_empty)
                    continue;
                return i;
            case '\n':
                return i;
            case '\\':
                backslashed = true;
                break;
            case '\"':
                qouted = true;
                break;
            default:
                arg.push_back(c);
            }
            if (is_empty)
                is_empty = false;   
        }
    }
    return i;
}
std::string &
ArgParser::get()
{
    return arg;
}

bool
ArgParser::is_complete()
{
    return !qouted && !backslashed;
}

struct CommandParser
{
    CommandParser();
    ~CommandParser();
    using iterator = std::string::iterator;
    std::vector<std::string> command;
    ArgParser ap;
    void clear();
    iterator parse(iterator begin, iterator end);
    std::vector<std::string> & get();
    bool is_complete();
};
CommandParser::CommandParser()
    : command()
    , ap()
{}

CommandParser::~CommandParser()
{}

std::string::iterator
CommandParser::parse(std::string::iterator begin, std::string::iterator end)
{
    auto i = begin;
    while (i != end) {
        i = ap.parse(i,end);
        if (ap.is_complete()) {
            if (!ap.is_empty)
                command.push_back(std::move(ap.get()));
            ap.clear();
            if (*i == '\n')
                break;
        }
    }
    return i;
}
std::vector<std::string> &
CommandParser::get()
{
    return command;
}

void
CommandParser::clear()
{
    ap.clear();
    command.clear();
}

bool
CommandParser::is_complete()
{
    return ap.is_complete();
}
/*
  0, ' '  -> 0, x
  0, '\t' -> 0, x
  0, '\\' -> 2, x,
  0, '\n' -> 5, x, flush:arg,cmd
  0, '\"' -> 3, x
  0, else -> 1, c
  
  1, ' '  -> 5, x, flush:arg
  1, '\t' -> 5, x, flush:arg
  1, '\\' -> 2, x,
  1, '\n' -> 6, x, flush:arg,cmd
  1, '\"' -> 3, x
  1, else -> 1, c

  2, '\n' -> 1, x
  2, else -> 1, c

  3, '\\' -> 4, x,
  3, '\"' -> 1, x
  3, else -> 2, c

  4, '\n' -> 1, x
  4, else -> 1, c


 */

void print_command(std::vector<std::string> const & command) {
    int i = 0;
    for (auto & arg : command) {
        std::cout << "  arg[" << i << "]=" << arg << std::endl;
        ++i;
    }
}

int main()
{
    std::string line;
    CommandParser cp;
    ArgParser ap;
    bool continued = false;
    while (true) {
        if (!continued)
            std::cout << "> ";
        if (!std::getline(std::cin,line))
            return 0;
        cp.parse(line.begin(),line.end());
        if (cp.is_complete()) {
            // std::cout << "[log] complete" << std::endl;
            print_command(cp.get());
            cp.clear();
            continued = false;
        }
        else {
            // std::cout << "[log] not complete" << std::endl;
            continued = true;
        }
    }
    return 0;
}
