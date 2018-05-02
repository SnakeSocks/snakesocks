#!/usr/bin/env python3

import subprocess
import sys

current_process = subprocess.Popen(["sleep", "1d"], stdout=subprocess.PIPE, universal_newlines=True)

def realtime_exec(cmd):
    global current_process
    current_process.kill() # Kill old task.
    current_process = subprocess.Popen(cmd, stdout=subprocess.PIPE, universal_newlines=True)
    for stdout_line in iter(current_process.stdout.readline, ""):
        yield stdout_line 
    current_process.stdout.close()
    return_code = current_process.wait()
    if return_code:
        raise subprocess.CalledProcessError(return_code, cmd)

while True:
    booming = False
    for line in realtime_exec(["sksrv"] + sys.argv[1:]):
        if 'accept4: too many open files' in line:
            print('[{} > wrapper] sksrv booms with `{}`.'.format(str(datetime.datetime.now()), line))
            booming = True
            break
        else:
            print(line)
    if not booming:
        break # Exited because of unknown reason.
    # restart sksrv


