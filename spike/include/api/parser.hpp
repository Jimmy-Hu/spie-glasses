#ifndef API_PARSER_HPP_INCLUDED
#define API_PARSER_HPP_INCLUDED
#include <iostream>
#include <utility>
#include <memory>
#include <vector>
#include <string>

namespace api {
    template <typename StringT = std::string>
    struct basic_argument_parser
    {
        using value_type = StringT;
        using argument_type = StringT;
        using iterator = typename StringT::iterator;

        basic_argument_parser();
        ~basic_argument_parser();
        void clear();
        iterator parse(iterator begin, iterator end);
        argument_type & get();
        argument_type const & get() const;
        bool is_complete() const;

        std::string arg;
        bool is_empty;    // = true;
        bool qouted;      // = false;
        bool backslashed; // = false;
    };

    template <typename StringT>
    basic_argument_parser<StringT>::basic_argument_parser()
        : is_empty(true)
        , qouted(false)
        , backslashed(false)
    {}

    template <typename StringT>
    basic_argument_parser<StringT>::~basic_argument_parser()
    {}

    template <typename StringT>
    void
    basic_argument_parser<StringT>::clear()
    {
        is_empty = true;
        qouted = false;
        backslashed = false;
        arg.clear();
    }

    template <typename StringT>
    typename StringT::iterator
    basic_argument_parser<StringT>::parse(typename StringT::iterator begin, typename StringT::iterator end)
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
                case '\'':
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
                case '\'':
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

    template <typename StringT>
    StringT &
    basic_argument_parser<StringT>::get()
    {
        return arg;
    }

    template <typename StringT>
    StringT const &
    basic_argument_parser<StringT>::get() const
    {
        return arg;
    }

    template <typename StringT>
    bool
    basic_argument_parser<StringT>::is_complete() const
    {
        return !qouted && !backslashed;
    }





    template <typename ContainerT = std::vector<basic_argument_parser<>::argument_type>>
    struct basic_command_parser
    {
        using command_type = ContainerT;
        using arguemnt_type = typename ContainerT::value_type;
        using iterator = typename arguemnt_type::iterator;

        basic_command_parser();
        ~basic_command_parser();
        void clear();
        iterator parse(iterator begin, iterator end);
        command_type & get();
        command_type const & get() const;
        bool is_complete() const;

        command_type command;
        basic_argument_parser<arguemnt_type> ap;
    };

    template <typename ContainerT>
    basic_command_parser<ContainerT>::basic_command_parser()
        : command()
        , ap()
    {}

    template <typename ContainerT>
    basic_command_parser<ContainerT>::~basic_command_parser()
    {}

    template <typename ContainerT>
    typename ContainerT::value_type::iterator
    basic_command_parser<ContainerT>::parse(typename ContainerT::value_type::iterator begin, typename ContainerT::value_type::iterator end)
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

    template <typename ContainerT>
    ContainerT &
    basic_command_parser<ContainerT>::get()
    {
        return command;
    }

    template <typename ContainerT>
    ContainerT const &
    basic_command_parser<ContainerT>::get() const
    {
        return command;
    }

    template <typename ContainerT>
    void
    basic_command_parser<ContainerT>::clear()
    {
        ap.clear();
        command.clear();
    }

    template <typename ContainerT>
    bool
    basic_command_parser<ContainerT>::is_complete() const
    {
        return ap.is_complete();
    }

    using argument_parser = basic_argument_parser<>;
    using command_parser = basic_command_parser<>;

} // namespace api

#endif // API_PARSER_HPP_INCLUDED
