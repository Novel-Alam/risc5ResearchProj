"""
File: build.py
Author: Brian Chiang
Created on: 2024-10-12

Description:
Python script to build runnable image, stored in out/bin/main
"""
import platform
import subprocess
def main():
    # OS = platform.system()
    # processor = 
    platform_version = platform.uname().version
    print(f"platform_version: {platform_version}")

if __name__ == "__main__":
    main()
