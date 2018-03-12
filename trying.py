import shlex
import subprocess
import time
if __name__ == '__main__':
    shell_cmd = './dealer_renju asdddd 1000 Alice Bob'
    cmd = shlex.split(shell_cmd)
    print(cmd)
    p = subprocess.Popen(cmd, shell=False, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    i = 0
    while p.poll() is None:
        line = p.stdout.readline()
        line = line.strip()
        if line and i <10:
            print('Subprogram output: [{}]'.format(line))
            i += 1
    if p.returncode == 0:
        print('Subprogram success')
    else:
        print('Subprogram failed')