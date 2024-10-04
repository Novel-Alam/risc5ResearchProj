/**
 * Global variable to hold state of the system 
 * Function to wake up on the positive clock edge and transition to the next state.
 * Function to fetch the instruction at the program counter if the current state is 'fetch', and store it in the instruction register.
 * Function to decode the instruction if the current state is 'decode', identifying the instruction type (R, I, etc.), operation (based on opcode and function bits), and source/destination registers.
 * Function to execute the operation in the 'execute' state, using a global variable to determine the function (e.g., add, sub, or, shift) and register/immediate operands. Store alu ouput in global var
 * Function to access memory in the 'memory access' state if a load/store instruction is encountered, using the calculated address to access RAM.
 * Function to write back data from the ALU to the destination register in the 'register writeback' state.
 */
 
