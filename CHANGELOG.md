
# v1.4.0 API CHANGE

(previous bug in socks5 server in skcli, forgot to convert destination port in socks5 query to HOST byteorder.) 

`client_query.destination_port` in decoded DNS query, and decoded datapack, is not in NETWORK byteorder anymore. It should be HOST byte order. Remove `htons()` in your sksrv implementation.

