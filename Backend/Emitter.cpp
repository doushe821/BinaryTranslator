#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <assert.h>

#include "Emitter.h"

size_t EmitPush(int RegisterName, char* elf, size_t elfIndex)
{
    size_t OldIndex = elfIndex;
    if(RegisterName - emiGeneralPurposeRegistersNamesSectionStart <= emiGeneralPurposeRegistersNamesSectionEnd)
    {
        int OpCode = emiPushOpCode + RegisterName - emiGeneralPurposeRegistersNamesSectionStart;
        memcpy(elf + elfIndex, &OpCode, 1);
        elfIndex += 1;
        
    }
    else if(RegisterName - emiExtendedRegistersNamesSectionStart <= emiExtendedRegistersNamesSectionEnd)
    {
        int OpCode = emiPushOpCode + RegisterName - emiExtendedRegistersNamesSectionStart;
        memcpy(elf + elfIndex, &OpCode, 1);
        int Prefix = emiREX_B;
        elfIndex += 1;
        memcpy(elf + elfIndex, &Prefix, 1);
        elfIndex += 1;
    }
    return elfIndex - OldIndex;
}

size_t EmitPushImm32(int Value, char* elf, size_t elfIndex)
{
    size_t OldIndex = elfIndex;
    int OpCode = emiPushImm32OpCode;
    memcpy(elf + elfIndex, &OpCode, 1);
    elfIndex++;
    memcpy(elf + elfIndex, &Value, 4);
    elfIndex += 4;

    return elfIndex - OldIndex;
}

size_t EmitAdd(EmitterOperand Operand1, EmitterOperand Operand2, char* elf, size_t elfIndex)
{
    size_t OldIndex = elfIndex;
    if(Operand1.Type != emiRegisterOperand || Operand1.Type != emiExtendedRegisterOperand)
    {
        assert(0);
    }

    if(Operand2.Type != emiIntValueOperand)
    {
        assert(0);
    }

    int Prefix = emiREX_W;
    memcpy(elf + elfIndex, &Prefix, 1);
    elfIndex++;
    int OpCode = emiAddImm32OpCode;
    mempcpy(elf + elfIndex, &OpCode, 1);
    elfIndex++;
    int OperandsSpecifier = emiMovRegisterMode + Operand1.Data.IntValue;
    memcpy(elf + elfIndex, &OperandsSpecifier, 1);
    elfIndex++;
    memcpy(elf + elfIndex, &Operand2.Data.IntValue, sizeof(Operand2.Data.IntValue));
    elfIndex += 4;

    return elfIndex - OldIndex;
}

size_t EmitSub(EmitterOperand Operand1, EmitterOperand Operand2, char* elf, size_t elfIndex)
{
    size_t OldIndex = elfIndex;
    if(Operand1.Type != emiRegisterOperand || Operand1.Type != emiExtendedRegisterOperand)
    {
        assert(0);
    }

    if(Operand2.Type != emiIntValueOperand)
    {
        assert(0);
    }

    int Prefix = emiREX_W;
    memcpy(elf + elfIndex, &Prefix, 1);
    elfIndex++;
    int OpCode = emiAddImm32OpCode;
    mempcpy(elf + elfIndex, &OpCode, 1);
    elfIndex++;
    int OperandsSpecifier = 0xe8 + Operand1.Data.IntValue;
    memcpy(elf + elfIndex, &OperandsSpecifier, 1);
    elfIndex++;
    memcpy(elf + elfIndex, &Operand2.Data.IntValue, sizeof(Operand2.Data.IntValue));
    elfIndex += 4;

    return elfIndex - OldIndex;
}

size_t EmitMov(EmitterOperand Operand1, EmitterOperand Operand2, char* elf, size_t elfIndex)
{
    size_t OldIndex = elfIndex;
    if(Operand1.Type = emiRegisterOperand)
    {
        if(Operand2.Type = emiRegisterOperand)
        {
            int Prefix = emiREX_W;
            int OpCode = emiMov64bitOpCode;
            int OperandSpecifier = emiMovRegisterMode + (Operand1.Data.IntValue >> 3) + Operand2.Data.IntValue;     
            memcpy(elf + elfIndex, &Prefix, 1);
            elfIndex++;
            mempcpy(elf + elfIndex, &OpCode, 1);
            elfIndex++;
            memcpy(elf + elfIndex, &OperandSpecifier, 1);
            elfIndex++;

            return elfIndex - OldIndex;
        }

        if(Operand2.Type == emiIntValueOperand)
        {
            int Prefix = emiREX_W;
            int OpCode = emiMov64bitImmOpCode;
            int OperandSpecifier = emiMovRegisterMode + Operand1.Data.IntValue;
            memcpy(elf + elfIndex, &Prefix, 1);
            elfIndex++;
            mempcpy(elf + elfIndex, &OpCode, 1);
            elfIndex++;
            memcpy(elf + elfIndex, &OperandSpecifier, 1); 
            elfIndex++;

            memcpy(elf + elfIndex, &Operand2.Data.Int64Value, 8);
            elfIndex += 8;
            
            return elfIndex - OldIndex;
        }

        if(Operand2.Type == emiMemoryOperand)
        {
            int OpCode = emiMov64bitImmOpCode;
            int Prefix = emiREX_W;
            memcpy(elf + elfIndex, &Prefix, 1);
            elfIndex++;
            mempcpy(elf + elfIndex, &OpCode, 1);
            elfIndex++;

            if(Operand2.MemoryShift == 0)
            {
                int OperandSpecifier = emiMovNoDisplacementMode + Operand1.Data.IntValue;
                memcpy(elf + elfIndex, &OperandSpecifier, 1); 
                elfIndex++;
                return elfIndex - OldIndex;
            }
            else
            {
                int OperandSpecifier = emiMov32bitDisplacementMode + Operand1.Data.IntValue;
                memcpy(elf + elfIndex, &OperandSpecifier, 1); 
                elfIndex++;
                return elfIndex - OldIndex;  
            }
        }  
    }
    else if(Operand1.Type == emiMemoryOperand)
    {
        if(Operand2.Type != emiRegisterOperand)
        {
            assert(0);
        }

        int Prefix = emiREX_W;
        int OpCode = emiMov64bitRMOpCode;
        memcpy(elf + elfIndex, &Prefix, 1);
        elfIndex++;
        mempcpy(elf + elfIndex, &OpCode, 1);
        elfIndex++;

        if(Operand1.MemoryShift == 0)
        {
            int OperandSpecifier = emiMovNoDisplacementMode + Operand1.Data.IntValue;
            memcpy(elf + elfIndex, &OperandSpecifier, 1); 
            elfIndex++;
            return elfIndex - OldIndex;   
        }
        else
        {
            int OperandSpecifier = emiMov32bitDisplacementMode + Operand1.Data.IntValue;
            memcpy(elf + elfIndex, &OperandSpecifier, 1); 
            elfIndex++;
            return elfIndex - OldIndex;   
        }

    }
}

size_t EmitPop(int RegisterName, char* elf, size_t elfIndex)
{
    size_t OldIndex = elfIndex;
    if(RegisterName - emiGeneralPurposeRegistersNamesSectionStart <= emiGeneralPurposeRegistersNamesSectionEnd)
    {
        int OpCode = emiPopOpCode + RegisterName - emiGeneralPurposeRegistersNamesSectionStart;
        memcpy(elf + elfIndex, &OpCode, 1);
        elfIndex += 1;
        
    }
    else if(RegisterName - emiExtendedRegistersNamesSectionStart <= emiExtendedRegistersNamesSectionEnd)
    {
        int OpCode = emiPopOpCode + RegisterName - emiExtendedRegistersNamesSectionStart;
        memcpy(elf + elfIndex, &OpCode, 1);
        int Prefix = emiREX_B;
        elfIndex += 1;
        memcpy(elf + elfIndex, &Prefix, 1);
        elfIndex += 1;
    }
    return elfIndex - OldIndex;
}

size_t EmitRet(char* elf, size_t elfIndex)
{
    size_t OldIndex = elfIndex;
    int OpCode = emiRetOpCode;
    memcpy(elf + elfIndex, &OpCode, 1);
    elfIndex++;
    return elfIndex - OldIndex;
}

size_t EmitCmp(EmitterOperand Operand1, EmitterOperand Operand2, char* elf, size_t elfIndex)
{
    size_t OldIndex = elfIndex;
    if(Operand1.Type != emiRegisterOperand)
    {
        assert(0);
    }
    if(Operand2.Type != emiIntValueOperand)
    {
        assert(0);
    }
    int Prefix = emiREX_W;
    int OpCode = emiAddImm32OpCode;
    int OperandSpecifier = emiMovRegisterMode + emiSlashSevenExt + Operand1.Data.IntValue;

    memcpy(elf + elfIndex, &Prefix, 1);
    elfIndex++;
    mempcpy(elf + elfIndex, &OpCode, 1);
    elfIndex++;
    memcpy(elf + elfIndex, &OperandSpecifier, 1); 
    elfIndex++;

    memcpy(elf + elfIndex, &Operand2.Data.IntValue, 8);
    elfIndex += 4;

    return elfIndex - OldIndex;
}

size_t EmitSSE2Ariphmetics(int Operation, EmitterOperand Operand1, EmitterOperand Operand2, char* elf, size_t elfIndex)
{
    size_t OldIndex = elfIndex;
    int Prefix = emiSSE;
    int OpCode = Operation;

    int OperandSpecifier = emiMovRegisterMode + ((Operand1.Data.IntValue - emiAVXRegistersNamesSectionStart) >> 3) + (Operand2.Data.IntValue >>3 - emiAVXRegistersNamesSectionEnd);

    memcpy(elf + elfIndex, &Prefix, 1);
    elfIndex++;
    mempcpy(elf + elfIndex, &OpCode, 2);
    elfIndex += 2;
    memcpy(elf + elfIndex, &OperandSpecifier, 1); 
    elfIndex++;

    return elfIndex - OldIndex;
}

size_t EmitMovsd(EmitterOperand Operand1, EmitterOperand Operand2, char* elf, size_t elfIndex)
{
    size_t OldIndex = elfIndex;
    if(Operand1.Type != emiRegisterOperand)
    {
        assert(0);
    }
    if(Operand2.Type != emiMemoryOperand)
    {
        assert(0);
    }

    int Prefix = emiSSE;
    int OpCode = emiMovsdOpCode;
    int OperandSpecifier = emiMovNoDisplacementMode + ((Operand1.Data.IntValue - emiAVXRegistersNamesSectionStart) >> 3) + Operand2.Data.IntValue;

    memcpy(elf + elfIndex, &Prefix, 1);
    elfIndex++;
    mempcpy(elf + elfIndex, &OpCode, 2);
    elfIndex += 2;
    memcpy(elf + elfIndex, &OperandSpecifier, 1); 
    elfIndex++;

    return elfIndex - OldIndex;
}

size_t EmitMovq(EmitterOperand Operand1, EmitterOperand Operand2, char* elf, size_t elfIndex)
{
    size_t OldIndex = elfIndex;
    int Prefix = emiSSE2;
    int OpCode = emiMovqOpCode;
    int OperandSpecifier = emiMovNoDisplacementMode + ((Operand1.Data.IntValue - emiAVXRegistersNamesSectionStart) >> 3) + Operand2.Data.IntValue;

    memcpy(elf + elfIndex, &Prefix, 1);
    elfIndex++;
    mempcpy(elf + elfIndex, &OpCode, 2);
    elfIndex += 2;
    memcpy(elf + elfIndex, &OperandSpecifier, 1); 
    elfIndex++;

    return elfIndex - OldIndex;
}

size_t EmitConditionalJump(int Operation, int Offset, char* elf, size_t elfIndex)
{
    size_t OldIndex = elfIndex;
    size_t OldIndex = elfIndex;
    mempcpy(elf + elfIndex, &Operation, 2);
    elfIndex += 2;
    mempcpy(elf + elfIndex, &Offset, 2);
    elfIndex += 4;
    return elfIndex - OldIndex;
}