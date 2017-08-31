// This is just a sample to tell you how to use api.
// You must not regard it as real cxx source code.
#include "stdafx.h"

#include <cxx_header>

[[returnval]] do_something_cxx(some_arg arg)
{
    return have_fun();
}

extern "C" {

// Client-side function implement.
binary_safe_string client_make_auth_query(const connect_info *sinf)
{
    bstr = malloc();
    do_something_by_cxx(sinf, bstr);
    return bstr;
}

bool client_deal_auth_reply(const connect_info *, binary_safe_string server_reply) //Return true if auth is ok.
{
    return isok(server_reply);
}
binary_safe_string client_encode(const connect_info *, binary_safe_string data)
{
    if (somecond)
    {
        return do_something_by_cxx(data);
    } else
    {
        bstr = malloc();
        do_something_by_cxx(data, bstr);
        free(data);
        return bstr;
    }
}
binary_safe_string client_decode(const connect_info *, binary_safe_string data)
{
    if (somecond)
    {
        return do_something_by_cxx(data);
    } else
    {
        bstr = malloc();
        do_something_by_cxx(data, bstr);
        free(data);
        return bstr;
    }
}


// Server-side function implement.
binary_safe_string server_make_auth_reply(const connect_info *sinf,binary_safe_string data, bool * is_closing)
{
    bstr = malloc();
    *is_closing = do_something(sinf,data,bstr);
    return bstr;
}
binary_safe_string server_encode(const connect_info *, binary_safe_string data)
{
    if(somecond)
    {
        return do_something(data);
    }
    else
    {
        bstr = malloc();
        do_something(data, bstr);
        free(data);
        return bstr;
    }
}
binary_safe_string server_decode(const connect_info *, binary_safe_string data)
{
    if(somecond)
    {
        return do_something(data);
    }
    else
    {
        bstr = malloc();
        do_something(data, bstr);
        free(data);
        return bstr;
    }
}

}// extern C