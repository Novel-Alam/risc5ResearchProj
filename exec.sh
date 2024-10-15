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
if [ "$#" -ne 0 ]; then # Parse arguments
    python3 build.py "$@"
    if [ $? -ne 0 ]; then
        echo "Python script status code $?."
        exit 1
    fi
fi

echo "Executing image"
./out/bin/main

