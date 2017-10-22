#ifndef R_STRING_HPP
#define R_STRING_HPP
#include <list>
#include <string>

namespace rlib{
	using std::string;
	using std::list;
	list<string> splitString(const string &tod, const char divider = ' ')
	{
		size_t lastPos = 0;
		size_t thisPos = tod.find(divider);
	    list<string> sbuf;
		if (thisPos != ::std::string::npos)
		{
			sbuf.push_back(tod.substr(0, thisPos));
			goto gt_1;
		}
		else
	    {
	        sbuf.push_back(tod);
	        return sbuf;
	    }
		do {
			sbuf.push_back(tod.substr(lastPos + 1, thisPos - lastPos - 1));
		gt_1:
			lastPos = thisPos;
			thisPos = tod.find(divider, lastPos + 1);
		} while (thisPos != ::std::string::npos);
		sbuf.push_back(tod.substr(lastPos + 1));
		return ::std::move(sbuf);
	}
    void replaceSubString(std::string& str, const std::string &from, const std::string& to) 
    {
       if(from.empty())
            return;
        size_t start_pos = 0;
        while((start_pos = str.find(from, start_pos)) != std::string::npos)
        {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
        }
    }
    bool replaceSubStringOnce(std::string& str, const std::string& from, const std::string& to) 
    {
        size_t start_pos = str.find(from);
        if(start_pos == std::string::npos)
            return false;
        str.replace(start_pos, from.length(), to);
        return true;
    }

}

#endif
