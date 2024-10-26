# RISC-V Simulation Project

This project provides a simulation environment for RISC-V architecture, using modular components to emulate core CPU operations such as fetching, decoding, and executing instructions. Built with Python, Make, and shell scripting, the system supports multiple platforms and allows configurable debugging, verbose output, and clean rebuilds for development.
(current ongoing development for Core Instruction Formats) 

## Development Usage

- `./exec.sh` — Execute current binary  
- `./exec.sh -b` — Compile, build, and run binary (from cache if available)  
- `./exec.sh -v` — Verbose execution/build  
- `./exec.sh -c -b` — Clean, compile, build, and execute  
- `./exec.sh -b -d` — Build with debugger support (requires `-b`)  
- `./exec.sh --help` — List available flags  

> **Note**: Avoid using `-c` without `-b` to prevent binary deletion without rebuilding.
