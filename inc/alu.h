#ifndef ALU_H
#define ALU_H

#include <stdint.h>
#include <stdbool.h>

/**
 * The control unit fetch state populates this 
 */
typedef struct {
    int32_t operand_1; //Parameter 1
    uint32_t operand_2; //Parameter 2
    uint8_t rd;
    uint32_t result;
    uint8_t microOp;  /* Set by the control unit*/
    bool zero_flag;;
    bool sign_flag;    
    bool parity_flag;
    bool overflow_flag;
} decoder_to_execute; 

extern decoder_to_execute aluOut;

// void ALU_Runner(decoder_to_execute *alu, uint8_t op, uint32_t *parameters);

// void alu_ld(decoder_to_execute *alu, uint32_t operand1, uint32_t operand2);

//==========================================Arithmetic Operations==========================================
uint32_t alu_add(uint32_t a, uint32_t b);
uint32_t alu_sub(uint32_t a, uint32_t b);
// uint32_t alu_mul(uint32_t a, uint32_t b);
// uint32_t alu_div(uint32_t a, uint32_t b);
// int32_t alu_div_signed(int32_t a, int32_t b);
// uint32_t alu_mod(uint32_t a, uint32_t b);
// //=========================================================================================================


// //==========================================Logical Operations=============================================
// uint32_t alu_and(uint32_t a, uint32_t b);
// uint32_t alu_or(uint32_t a, uint32_t b);
// uint32_t alu_xor(uint32_t a, uint32_t b);
// uint32_t alu_not(uint32_t a);
// //=========================================================================================================


// //==========================================Shift Operations===============================================
// uint32_t alu_sll(uint32_t a, uint32_t b);
// uint32_t alu_srl(uint32_t a, uint32_t b);
// int32_t alu_sra(int32_t a, uint32_t b);
// //=========================================================================================================

// //==========================================Compariuson Operations=========================================
// uint32_t alu_slt(int32_t a, int32_t b);
// uint32_t alu_sltu(uint32_t a, uint32_t b);
// uint32_t alu_eq(uint32_t a, uint32_t b);
// uint32_t alu_ne(uint32_t a, uint32_t b);
// //=========================================================================================================


// //==========================================Immediate Operations===========================================
// uint32_t alu_addi(uint32_t a, uint32_t imm);
// uint32_t alu_andi(uint32_t a, uint32_t imm);
// uint32_t alu_ori(uint32_t a, uint32_t imm);
// uint32_t alu_xori(uint32_t a, uint32_t imm);
// //=========================================================================================================

// //==========================================Overflow Detection=============================================
// uint32_t alu_add_overflow(uint32_t a, uint32_t b);
// uint32_t alu_sub_overflow(uint32_t a, uint32_t b);
// //=========================================================================================================

#endif
