#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <assert.h>

#include "Emitter.h"

int EmitPush(int RegisterName, char* elf, size_t elfIndex)
{
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
    return 0;
}

int EmitAdd(EmitterOperand Operand1, EmitterOperand Operand2, char* elf, size_t elfIndex)
{
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

    return 0;
}

int EmitMov(EmitterOperand Operand1, EmitterOperand Operand2, char* elf, size_t elfIndex)
{
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

            return 0;
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
            
            return 0;
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
                return 0;
            }
            else
            {
                int OperandSpecifier = emiMov32bitDisplacementMode + Operand1.Data.IntValue;
                memcpy(elf + elfIndex, &OperandSpecifier, 1); 
                elfIndex++;
                return 0;  
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
            return 0;   
        }
        else
        {
            int OperandSpecifier = emiMov32bitDisplacementMode + Operand1.Data.IntValue;
            memcpy(elf + elfIndex, &OperandSpecifier, 1); 
            elfIndex++;
            return 0;   
        }

    }
}

int EmitPop(int RegisterName, char* elf, size_t elfIndex)
{
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
    return 0;
}

int EmitRet(char* elf, size_t elfIndex)
{
    int OpCode = emiRetOpCode;
    memcpy(elf + elfIndex, &OpCode, 1);
    elfIndex++;
    return 0;
}

int EmitCmp(EmitterOperand Operand1, EmitterOperand Operand2, char* elf, size_t elfIndex)
{
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

    return 0;
}

int EmitSSE2Ariphmetics(int Operation, EmitterOperand Operand1, EmitterOperand Operand2, char* elf, size_t elfIndex)
{
    int Prefix = emiSSE;
    int OpCode = Operation;

    int OperandSpecifier = emiMovRegisterMode + ((Operand1.Data.IntValue - emiAVXRegistersNamesSectionStart) >> 3) + (Operand2.Data.IntValue >>3 - emiAVXRegistersNamesSectionEnd);

    memcpy(elf + elfIndex, &Prefix, 1);
    elfIndex++;
    mempcpy(elf + elfIndex, &OpCode, 2);
    elfIndex += 2;
    memcpy(elf + elfIndex, &OperandSpecifier, 1); 
    elfIndex++;

    return 0;
}

int EmitMovsd(EmitterOperand Operand1, EmitterOperand Operand2, char* elf, size_t elfIndex)
{
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

    return 0;
}

int EmitMovq(EmitterOperand Operand1, EmitterOperand Operand2, char* elf, size_t elfIndex)
{
    int Prefix = emiSSE2;
    int OpCode = emiMovqOpCode;
    int OperandSpecifier = emiMovNoDisplacementMode + ((Operand1.Data.IntValue - emiAVXRegistersNamesSectionStart) >> 3) + Operand2.Data.IntValue;

    memcpy(elf + elfIndex, &Prefix, 1);
    elfIndex++;
    mempcpy(elf + elfIndex, &OpCode, 2);
    elfIndex += 2;
    memcpy(elf + elfIndex, &OperandSpecifier, 1); 
    elfIndex++;
    
    return 0;
}

int EmitConditionalJump(int Operation, int Label, char* elf, size_t elfIndex)
{
    return 0;
}