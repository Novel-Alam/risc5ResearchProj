#!/bin/bash

current_arch=$(uname -p) #x86_64, arm64, etc
unameOut="$(uname -s)"
case "${unameOut}" in
    Linux*)     machine=Linux;;
    Darwin*)    machine=Mac;;
    CYGWIN*)    machine=Cygwin;;
    MINGW*)     machine=MinGw;;
    MSYS_NT*)   machine=Git;;
    *)          machine="UNKNOWN:${unameOut}"
esac

# debug_flag="DEBUG=FALSE"

# # Check the value of debug_flag and determine whether to run the debugger
# if [[ "$debug_flag" == "DEBUG=TRUE" ]]; then
#     echo "Launching in debug mode..."
#     if [ "$machine" = "Mac" ]; then
#         lldb ./out/bin/main
#     elif [ "$machine" = "Linux" ]; then
#         gdb ./out/bin/main
#     else
#         echo "Debugger not supported on this system."
#         exit 1
#     fi
# else

if [ "$#" -ne 0 ]; then # Parse arguments
    python3 build.py "$@"
    if [ $? -ne 0 ]; then
        echo "Python script status code $?."
        exit 1
    fi
fi

echo "Executing image"
./out/bin/main

