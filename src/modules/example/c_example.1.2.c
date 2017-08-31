// This is just a sample to tell you how to use api.
// You must not regard it as real c source code.
#include "stdafx.1.2.h"


// Client-side function implement.
binary_safe_string client_make_auth_query(connect_info *sinf)
{
    bstr = malloc();
    do_something(sinf, bstr);
    return bstr;
}

bool client_deal_auth_reply(connect_info *, binary_safe_string server_reply); //Return true if auth is ok.
{
    return isok(server_reply);
}
binary_safe_string client_encode(connect_info *, client_query data)
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
client_query client_decode(connect_info *, binary_safe_string data)
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

// Server-side function implement.
binary_safe_string server_make_auth_reply(connect_info *sinf,binary_safe_string data, bool * is_closing)
{
    bstr = malloc();
    *is_closing = do_something(sinf,data,bstr);
    return bstr;
}
binary_safe_string server_encode(connect_info *, client_query data)
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
client_query server_decode(connect_info *, binary_safe_string data)
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