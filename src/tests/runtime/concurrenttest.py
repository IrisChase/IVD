import concurrent.futures
import subprocess
import os
import sys

nope, cutable = sys.argv

def do_all(IMES):
    def exec(arglist):
        try:
            subprocess.run(arglist, check=True)
        except:
            print("Failed for: %s" % (arglist[2]))
            print("\n")


    with concurrent.futures.ThreadPoolExecutor() as executor:
        futures = []
        for i in IMES:
            futures.append(executor.submit(exec, i))
        
        for f in concurrent.futures.as_completed(futures):
            pass

IME = []

for fileName in os.listdir():
    base, ext = os.path.splitext(fileName)
    if ext == '.ivd':
        x3thpath = os.path.join("reprodyne", base + ".x3th")
        
        #is it even existant
        if not os.path.isfile(x3thpath):
            print("Could not find x3th for: %s, skipping..." % (fileName))
            continue

        IME.append([cutable, 'play', fileName, x3thpath])
        


do_all(IME)

