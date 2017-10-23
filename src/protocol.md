<!-- coding: utf-8 (zh_CN) -->

## 这是不完整的SnakeSocks隧道协议的文档

#### dns

客户端可以在任何时候发出一个dns请求。

此时，客户端构造一个数据包，其内容为`$s+$domain+'\0'`。其中$s为编码进代码的一个32字节字符串，$domain为ASCII表示的域名字符串。

服务端在同一个连接中发送回复，其内容为`$s+$status+$data`。其中$s同上，$status为一个int8\_t(-1表示解析失败，4表示ipv4，6表示ipv6)，$data是数据(len($data)在失败时为0，ipv4时为4Byte，ipv6时为16Byte)。

