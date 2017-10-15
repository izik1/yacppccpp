import sys
import os
import subprocess

def main():
    if len(sys.argv) != 3:
        print("invalid args!")
        return 1
    testDir = os.path.join(os.path.dirname(os.path.realpath(__file__)))
    p = subprocess.check_output([sys.argv[1], os.path.join(testDir, "pass", sys.argv[2])]).decode('ascii')
    try:
        open(os.path.join(testDir, "pass-expected_output", sys.argv[2]), 'w').write(p)
    except:
        print("Write failed")
        exit(1)
    print("Success")

if __name__ == "__main__":
    main()

