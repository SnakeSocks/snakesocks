

#define let auto

// Server guide
void on_connection_established()
{ // 面向过程风格
    let is_first_package = false;
    while(true)
    {
        let pkg = skcli.read_a_package().decode();
        if(is_first_package)
        {
            is_first_package = false;
            if(pkg.is_dns())
            {
                skcli.write_a_package(do_dns(pkg).encode());
                continue;
            }
        }
        ///////////////// 正常处理数据包
        if(!server.connected()) 
            server.connect();
        server.write_a_package(pkg.data);
        skcli.write_a_package(server.read_a_package().encode());
    }
}

