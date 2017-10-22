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
#include <rlib/noncopyable.hpp>
#include <rlib/string/fstr.hpp>
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
            for(size_t cter = 1; cter < arglen; ++cter)
                args.push_back(std::move(std::string(argv[cter])));
        }

        std::string getValueArg(const std::string &argName, bool required = false)
        { //If required argument not exist, I'll throw. Else, return "" if arg is not read.
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
                throw std::invalid_argument(fstr("Required argument '%s' not provided.", argName.c_str()));
            if(pos == args.cend())
                return std::move(std::string(""));
            defer(([&, pos]{if(!useEqualSym) args.erase(pos+1); args.erase(pos);}));
            if(useEqualSym)
                return std::move(pos->substr(argName.size() + 1));
            else
            {
                if(++pos == args.cend())
                    throw std::invalid_argument(fstr("Argument '%s' must provide value.", argName.c_str()));
                return *pos;
            }
        }

        std::string getValueArg(const std::string &argName, const char *pAnotherCStr)
        { //getValueArg("--long", "-l") may be converted to getValueArg("--long", true).
            return std::move(getValueArg(argName, pAnotherCStr, false));
        }

        bool getBoolArg(const std::string &argName)
        { //Return if it's defined.
            auto pos = std::find(args.cbegin(), args.cend(), argName);
            if(pos == args.cend()) return false;
            args.erase(pos);
            return true;
        }

        std::string getValueArg(const std::string &longName, const std::string &shortName, bool required = false)
        {
            std::string valueL = getValueArg(longName);
            std::string valueS = getValueArg(shortName);
            
            std::string value = valueL.empty() ? valueS : valueL;
            if(required && value.empty())
                throw std::invalid_argument(fstr("Required argument '%s/%s' not provided.", longName.c_str(), shortName.c_str()));
            return value;
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
    };
}

#endif
