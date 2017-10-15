import sys
import os
import subprocess

def run_test(path, outpath, name):
    p = subprocess.check_output([sys.argv[1], path]).decode('ascii')
    if len(sys.argv) == 3 and sys.argv[2] == "-v":
       print(p)
    try:
        x = open(outpath).read()
        if len(sys.argv) == 3 and sys.argv[2] == "-v":
            print(x)
        print(name)
        print(p.split() == x.split())
    except:
        print("Test output doesn't exist")
    return

def main():
    if len(sys.argv) < 2 or len(sys.argv) > 3:
        print("invalid args!")
        return 0
    testDir = os.path.dirname(os.path.realpath(__file__))
    for filename in os.listdir(os.path.join(testDir, "pass")):
        run_test(
            os.path.join(testDir, "pass", filename),
            os.path.join(testDir, "pass-expected_output", filename),
            filename)


if __name__ == "__main__":
    main()
