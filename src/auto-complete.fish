#!/usr/bin/env fish

complete -c skcli -f -s h -l help -d 'Show help messages'

complete -c skcli -f -s s -l server -d 'Server domain or ip'
complete -c skcli -f -s p -l server-port -d 'Server port'
complete -c skcli -f -s k -l passphrase -d 'Pre-shared passphrase'
complete -c skcli -f -s L -l listen -d 'Local address that socks5 server listens on'
complete -c skcli -f -s P -l listen-port -d 'Local port that socks5 server listens on'
complete -c skcli -f -s D -l debug -a '0 1 2 3 4 5' -d 'Debug level'
complete -c skcli -f -s m -l mod -a '(__fish_complete_path)' -d 'Module path'

complete -c skcli -f -s c -l conf -a 'NULL /etc/snakesocks/conf/client.conf (__fish_complete_path)' -d 'Configuration file to load'
complete -c skcli -f -s d -l daemon -d 'Run as daemon'
complete -c skcli -f -s l -l daemon-log -a '/var/log/skcli.log (__fish_complete_path)' -d 'Log file for daemon mode'
