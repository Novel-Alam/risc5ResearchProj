#!/usr/bin/env python3
"""
File: build.py
Author: Brian Chiang
Created on: 2024-10-12

Description:
Python script to build runnable image, stored in out/bin/main
"""

import argparse
import subprocess
import sys
from pathlib import Path
def main():
    parser = argparse.ArgumentParser(description="Python script to compile and build runnable image, stored in out/bin/main")
    parser.add_argument('-c', '--clean', action='store_true', help='Clean up out folder')
    parser.add_argument('-b', '--build', action='store_true', help='Build and Compile Image')
    parser.add_argument('-d', '--debug', action='store_true', help='Pass in debugger points to final image')
    parser.add_argument('-v', '--verbose', action='store_true', help='Enable verbose mode')
    if '-h' in sys.argv or '--help' in sys.argv:
        parser.print_help()
        sys.exit(1)  # Return 1 if --help is passed
    make_path = Path(__file__).resolve().parent / 'build.mk'
    make_call_args = ['make', '-f', make_path]
    verbose = False
    args = parser.parse_args()
    if args.verbose:
        verbose = True
    if args.clean:
        result = subprocess.run(make_call_args + ['clean'], capture_output=True, text=True)
        if verbose: print(result.stdout)
        if not args.build:
            print("ERROR: Cleaning file system but not building. No image will be created.")
            sys.exit(1)
    if args.debug:
        make_call_args.append('DEBUG=1')
    if args.build:
        result = subprocess.run(make_call_args, capture_output=True, text=True)
        if verbose: print(result.stdout)
    if result.returncode == 0:
        print("Make succeeded. :3")
        if verbose:
            print("Output:")
            print(result.stdout)  # Output from the command
        if args.debug and args.build:
            print("Debug mode activated. Activate debugger on out/bin/main")
            sys.exit(1)
    else:
        print("Make failed.")
        print("Error:")
        print(result.stderr)  # Error message from the command
    # print("Build Complete! Please call ./exec.sh to run program")
    sys.exit(0)

if __name__ == "__main__":
    main()
