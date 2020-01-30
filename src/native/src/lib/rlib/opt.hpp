/*
This opt_parser works well for correct cmd args, 
but not guaranteed to works well in all condition
(for example, some ill formed argument).

It's possible to read wrong information rather than
raise an exception on some rare ill formed arguments.
*/
#ifndef R_OPT_HPP
#define R_OPT_HPP

#include <rlib/require/cxx14>
#include <rlib/class_decorator.hpp>
#include <rlib/string.hpp>
#include <rlib/scope_guard.hpp>

#include <string>
#include <vector>
#include <algorithm>
#include <stdexcept>

namespace rlib {
    class opt_parser : private noncopyable
    {
    public:
        opt_parser() = delete;
        opt_parser(size_t arglen, char **argv) {
            if(argv[0] == nullptr)
                throw std::runtime_error("Invalid argv passed to rlib::opt_parser. argv[0] is nullptr.");
            arg0 = argv[0];
            for(size_t cter = 1; cter < arglen; ++cter)
                args.push_back(std::string(argv[cter]));
        }

        rlib::string getSubCommand() {
            if(args.empty())
                throw std::runtime_error("No sub-command available.");
            auto cmd = std::move(args[0]);
            args.erase(args.begin());
            return std::move(cmd);
        }

        rlib::string getSubCommand(const std::string &def) {
            if(args.empty())
                return def;
            auto cmd = std::move(args[0]);
            args.erase(args.begin());
            return std::move(cmd);
        }

        rlib::string getSelf() {
            return arg0;
        }

        rlib::string getValueArg(const std::string &argName, bool required = true, const std::string &def = std::string())
        { //If required argument not exist, I'll throw. Else, return "" if arg is not read.
            using rlib::literals::operator "" _format;
            bool useEqualSym = false;
            auto pos = std::find_if(args.cbegin(), args.cend(), [&](auto &ele)->bool{
                if(ele == argName) return true;
                if(ele.size() > argName.size() && ele.substr(0, argName.size()+1) == argName + "=") {
                    useEqualSym = true;
                    return true;
                }
                return false;
            });
            if(required && pos == args.cend())
                throw std::invalid_argument("Required argument '{}' not provided."_format(argName));
            if(pos == args.cend())
                return std::move(def);
            rlib_defer(([&, pos]{if(!useEqualSym) args.erase(pos+1); args.erase(pos);}));
            if(useEqualSym)
                return pos->substr(argName.size() + 1);
            else
            {
                if(++pos == args.cend())
                    throw std::invalid_argument("Argument '{}' must provide value."_format(argName));
                return *pos;
            }
        }

        rlib::string getValueArg(const std::string &longName, const char *shortName)
        { //getValueArg("--long", "-l") may be converted to getValueArg("--long", true).
            return getValueArg(longName, shortName, true);
        }

        bool getBoolArg(const std::string &argName)
        { //Return if it's defined.
            auto pos = std::find(args.cbegin(), args.cend(), argName);
            if(pos == args.cend()) return false;
            args.erase(pos);
            return true;
        }

        rlib::string getValueArg(const std::string &longName, const std::string &shortName, bool required = true, const std::string &def = std::string())
        {
            using rlib::literals::operator "" _format;
            std::string valueL = getValueArg(longName, false);
            std::string valueS = getValueArg(shortName, false);
            
            const std::string &value = valueL.empty() ? valueS : valueL;
            if(value.empty()) {
                if(required)
                    throw std::invalid_argument("Required argument '{}/{}' not provided."_format(longName, shortName));
                else
                    return def;
            }
            return std::move(value);
        }

        bool getBoolArg(const std::string &longName, const std::string &shortName)
        {
            return getBoolArg(longName) || getBoolArg(shortName);
        }

        bool allArgDone() const
        {
            return args.empty();
        }
    private:
        std::vector<std::string> args;
        std::string arg0;
    };
}

#endif
