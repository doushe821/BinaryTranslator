#ifndef EMITTER_H_INCLUDED
#define EMITTER_H_INCLUDED

enum EmitterAllRegistersNames
{

    emi_rax = 0,
    emi_rcx ,
    emi_rdx ,
    emi_rbx ,
    emi_rsp ,
    emi_rbp ,
    emi_rsi ,
    emi_rdi ,
    emi_r8  ,
    emi_r9  ,
    emi_r10 ,
    emi_r11 ,
    emi_r12 ,
    emi_r13 ,
    emi_r14 ,
    emi_r15 ,
    emi_xmm0,
    emi_xmm1,
    emi_xmm2,
    emi_xmm3,
    emi_xmm4,
};

enum EmitterRegisterNamesSections
{
    emiGeneralPurposeRegistersNamesSectionStart = 0,
    emiGeneralPurposeRegistersNamesSectionEnd = 7,
    emiExtendedRegistersNamesSectionStart = 8,
    emiExtendedRegistersNamesSectionEnd = 15, 
    emiAVXRegistersNamesSectionStart = 16,
    emiAVXRegistersNamesSectionEnd = 20,
};



enum EmitterGeneralPurposeRegistersRd
{
    emi_rax_rd = 0,
    emi_rcx_rd,
    emi_rdx_rd,
    emi_rbx_rd,
    emi_rsp_rd,
    emi_rbp_rd,
    emi_rsi_rd,
    emi_rdi_rd,
};

enum EmitterExtendedRegistersRd
{
    emi_r8_rd = 0,
    emi_r9_rd ,
    emi_r10_rd,
    emi_r11_rd,
    emi_r12_rd,
    emi_r13_rd,
    emi_r14_rd,
    emi_r15_rd,
};

enum EmitterPrefixes
{
    emiREX_B = 0x41,
    emiREX_W = 0x48,
    emiREX = 0x49,
    emiSSE = 0xF2,
    emiSSE2 = 0xF3,
    emiXMMtoMem = 0x66,
    
};

enum EmitterMovModes
{
    emiMovRegisterMode = 0xC0,
    emiMovNoDisplacementMode = 0x00,
    emiMov32bitDisplacementMode = 0x80,

};

enum OpCodeExtensions
{
    emiSlashSevenExt = 0x38,
    emiSlashFiveExt = 0x28,
    emiSIB = 0x24,
};

union EmitterOperandData
{
    int IntValue;
    double DoubleValue;
    u_int64_t Int64Value;
    u_int8_t Int8Value;
};

struct EmitterOperand
{
    int Type;
    EmitterOperandData Data;
    int MemoryShift;
};

enum EmitterOperandTypes
{
    emiIntValueOperand,
    emiDoubleValueOperand,
    emiRegisterOperand,
    emiExtendedRegisterOperand,
    emiMemoryOperand,
};
const size_t InstructionNameMax = 16;
const size_t InstructionsNumber = 16;

enum EmitterOpCodes
{
    emiPushOpCode = 0x50,
    emiAddImm32OpCode = 0x81,
    emiSubImm32OpCode = 0x83,
    emiMov64bitOpCode = 0x89,
    emiMov64bitImmOpCode = 0xb8,
    emiMov64bitRMOpCode = 0x89,
    emiPopOpCode = 0x58,
    emiRetOpCode = 0xC3,
    emiAddsdOpCode = 0x580f,
    emiSubsdOpCode = 0x5c0f,
    emiMulsdOpCode = 0x590f,
    emiDivsdOpCode = 0x5e0f,
    emiMovsdOpCode = 0x100f,
    emiMovqOpCode = 0x7e0f,
    emiNearJneOpCode = 0x850f, 
   // emiSubImm32OpCode = 0x2D,
    emiPushImm32OpCode = 0x68,
    emiCallOpCode = 0xe8,
    emiCmpsdOpCode = 0xc20f,
    emiShr8bitOpCode = 0xc1,
    emiMovMemRegOpCode = 0x8b,
    emiStoreSSEOpCode = 0xd60f,
    
};

enum ComparisonTypes
{
    emiEqual = 0x0,
    emiLessThan,
    emiLessThanOrEqual,
    emiNotEqual,
    emiGreaterOrEqual,
    emiGreater,

};

size_t EmitShr(EmitterOperand Operand1, EmitterOperand Operand2, char* elf, size_t elfIndex);
size_t EmitCmpsd(int ComparisonMode, EmitterOperand Operand1, EmitterOperand Operand2, char* elf, size_t elfIndex);
size_t EmitCall(int Shift, char* elf, size_t elfIndex);
size_t EmitPushImm32(int Value, char* elf, size_t elfIndex);
size_t EmitSub(EmitterOperand Operand1, EmitterOperand Operand2, char* elf, size_t elfIndex);
size_t EmitRet(char* elf, size_t elfIndex);
size_t EmitPop(int RegisterName, char* elf, size_t elfIndex);
size_t EmitMov(EmitterOperand Operand1, EmitterOperand Operand2, char* elf, size_t elfIndex);
size_t EmitAdd(EmitterOperand Operand1, EmitterOperand Operand2, char* elf, size_t elfIndex);
size_t EmitPush(int RegisterName, char* elf, size_t elfIndex);
size_t EmitConditionalJump(int Operation, int Offset, char* elf, size_t elfIndex);
size_t EmitMovq(EmitterOperand Operand1, EmitterOperand Operand2, char* elf, size_t elfIndex);
size_t EmitMovsd(EmitterOperand Operand1, EmitterOperand Operand2, char* elf, size_t elfIndex);
size_t EmitSSE2Ariphmetics(int Operation, EmitterOperand Operand1, EmitterOperand Operand2, char* elf, size_t elfIndex);
size_t EmitCmp(EmitterOperand Operand1, EmitterOperand Operand2, char* elf, size_t elfIndex);

#endif