import os
import subprocess

compiler = "./a.out"
compiler =  "./tiger-compiler"
testcasesDir = "testcases/"
outbinDir = "outbin/"
stdoutDir = "stdout/"


def executeCMD(cmd):
    out = subprocess.getoutput(cmd)
    if out != "":
        print(out)


status, files = subprocess.getstatusoutput(f'ls {testcasesDir}')
files = files.split("\n")

for file in files:
    cmds = [
        f'{compiler} {testcasesDir + file} >/dev/null',
        f'gcc -m32 -c {testcasesDir + file + ".s"}',
        f'gcc -m32 {file + ".o"} runtime.o -o {outbinDir + file[0:len(file)-4]}',
        f'rm {file + ".o"}',
    ]
    if file.endswith(".tig"):
        for cmd in cmds:
            executeCMD(cmd)
        # already compiled

status, files = subprocess.getstatusoutput(f'ls {outbinDir}')
files = files.split("\n")
for file in files:
    executeCMD(f'{outbinDir + file} > temp')
    executeCMD(f'diff {stdoutDir + file + ".out"} temp')
