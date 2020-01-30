
# v1.4.0 API CHANGE

`client_query.destination_port` in decoded DNS query, and decoded datapack, is not in HOST byteorder anymore. It should be NETWORK byte order. Add `htons()` to your sksrv implementation.

