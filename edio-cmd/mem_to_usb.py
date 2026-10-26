import os
import sys
import subprocess

USBTOOL = "../edlink.py"

ARGS = ["usbrd", "--file", "rx.bin", "--len", "2048"]


def main():
    if not os.path.isfile(USBTOOL):
        print("error: edlink.py not found: " + USBTOOL, file=sys.stderr)
        sys.exit(1)

    cmd = [sys.executable, USBTOOL] + ARGS

    result = subprocess.run(cmd)
    sys.exit(result.returncode)

if __name__ == "__main__":
    main()
