#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <assert.h>

#include "Emitter.h"

static char SyscallInBinary[] = "H��@H��PSQRWVAPAQARUH��H���   H��H��H��8H���    H�G�H�      �?H�G�H�       @H�G�H�      @H�G�H�      @H�G�H�      @H�G�H�      @H�G�H�      @H�G�H�       @H�G�H�      \"@H�G�H��I���    VH���    �@   ^H��H��H��8H��H�H��M1�AP�$H��I9�t4H1ۊH��.teH��0H��H)�H�H�P�$H���S�X�H��I����H�      $@P�$H��I�� t	�^�I����fH~�]AZAYAX^_ZY[H�Đ   �M1�M��H���M1�AR�$H��M��H�      $@P�$H��I�� t	I���Y����t���,              @     �                      u         @     �@         ";
static size_t InBinarySize = strlen(SyscallInBinary);
static char SyscallOutBinary[] = "XH��@UH��H��HSQRWVAQI��H��H��H�E �   H�¹@   H��H�� tH!�H��0�H��H��H����   H���   �@   H��L�M AY^_Z^[]H��8�,              @     u                       v         @     u@         out.asm /home/doushe/Documents/DED/TrueCompiler/Backend/ NASM 2.16.01 � @              %  . @   `       �       ";
static size_t OutBinarySize = strlen(SyscallOutBinary);
static char SyscallHaltBinary[] = {0x48, 0xc7, 0xc0, 0x3c, 0x00, 0x00, 0x00, 0x0f, 0x05};
static size_t HaltBinarySize = strlen(SyscallHaltBinary);

size_t EmitPush(int RegisterName, char* elf, size_t elfIndex)
{
    size_t OldIndex = elfIndex;
    if(RegisterName <= emiGeneralPurposeRegistersNamesSectionEnd && RegisterName >= emiGeneralPurposeRegistersNamesSectionStart)
    {
        int OpCode = emiPushOpCode + RegisterName - emiGeneralPurposeRegistersNamesSectionStart;
        memcpy(elf + elfIndex, &OpCode, 1);
        elfIndex += 1;
        
    }
    else if(RegisterName <= emiExtendedRegistersNamesSectionEnd && RegisterName >= emiExtendedRegistersNamesSectionStart)
    {
        int Prefix = emiREX_B;
        memcpy(elf + elfIndex, &Prefix, 1);
        elfIndex += 1;
        int OpCode = emiPushOpCode + RegisterName - emiExtendedRegistersNamesSectionStart;
        memcpy(elf + elfIndex, &OpCode, 1);
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
    if(Operand1.Type != emiRegisterOperand && Operand1.Type != emiExtendedRegisterOperand)
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

size_t EmitShr(__attribute((unused))EmitterOperand Operand1, EmitterOperand Operand2, char* elf, size_t elfIndex)
{
    size_t OldIndex = elfIndex;
    int Prefix = emiREX_W;
    int OpCode = emiShr8bitOpCode;
    int OperandSpecifier = 0xe8;

    mempcpy(elf + elfIndex, &Prefix, 1);
    elfIndex++;
    mempcpy(elf + elfIndex, &OpCode, 1);
    elfIndex++;
    memcpy(elf + elfIndex, &OperandSpecifier, 1);
    elfIndex++;
    memcpy(elf + elfIndex, &Operand2.Data.IntValue, 1);
    elfIndex += 1;

    return elfIndex - OldIndex;
}

size_t EmitCmpsd(int ComparisonMode, EmitterOperand Operand1, EmitterOperand Operand2, char* elf, size_t elfIndex)
{
    size_t OldIndex = elfIndex;
    int OpCode = emiCmpsdOpCode;
    int Prefix = emiSSE;
    memcpy(elf + elfIndex, &Prefix, 1);
    elfIndex++;
    int OperandSpecifier = emiMovRegisterMode + ((Operand1.Data.IntValue - emiAVXRegistersNamesSectionStart) << 3) + Operand2.Data.IntValue - emiAVXRegistersNamesSectionStart;
    memcpy(elf + elfIndex, &OpCode, 2);
    elfIndex += 2;
    memcpy(elf + elfIndex, &OperandSpecifier, 1);
    elfIndex++;
    memcpy(elf + elfIndex, &ComparisonMode, 1);
    elfIndex++;
    return elfIndex - OldIndex;
}

size_t EmitCall(int Shift, char* elf, size_t elfIndex)
{
    Shift -= 5;
    size_t OldIndex = elfIndex;
    int OpCode = emiCallOpCode;
    memcpy(elf + elfIndex, &OpCode, 1);
    elfIndex++;
    mempcpy(elf + elfIndex, &Shift, 4);
    elfIndex += 4;
    return elfIndex - OldIndex;
}

size_t EmitSub(EmitterOperand Operand1, EmitterOperand Operand2, char* elf, size_t elfIndex)
{
    size_t OldIndex = elfIndex;
    if(Operand1.Type != emiRegisterOperand && Operand1.Type != emiExtendedRegisterOperand)
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
    int OpCode = emiSubImm32OpCode;
    mempcpy(elf + elfIndex, &OpCode, 1);
    elfIndex++;
    int OperandsSpecifier = 0xe8 + Operand1.Data.IntValue;
    memcpy(elf + elfIndex, &OperandsSpecifier, 1);
    elfIndex++;
    memcpy(elf + elfIndex, &Operand2.Data.IntValue, 1);
    elfIndex += 1;

    return elfIndex - OldIndex;
}

size_t EmitMov(EmitterOperand Operand1, EmitterOperand Operand2, char* elf, size_t elfIndex)
{
    size_t OldIndex = elfIndex;
    if(Operand1.Type == emiRegisterOperand)
    {
        fprintf(stderr," hoot hoot %d\n", Operand2.Type);
    
        if(Operand2.Type == emiMemoryOperand)
        {
            fprintf(stderr, "memory\n\n");
            int OpCode = emiMovMemRegOpCode;
            int Prefix = emiREX_W;
            memcpy(elf + elfIndex, &Prefix, 1);
            elfIndex++;
            mempcpy(elf + elfIndex, &OpCode, 1);
            elfIndex++;
            
            if(Operand2.Data.IntValue == emi_rbp)
            {

                int SpecialCase = 0x45;
                memcpy(elf + elfIndex, &SpecialCase, 1);
                elfIndex++;
                if(Operand2.MemoryShift == 0)
                {
                    elfIndex++;
                }
                else
                {
                    memcpy(elf + elfIndex, &Operand2.MemoryShift, 1);
                    elfIndex++;
                }
                return elfIndex - OldIndex;
            }

            if(Operand2.MemoryShift == 0)
            {
                int OperandSpecifier = emiMovNoDisplacementMode + Operand1.Data.IntValue;
                memcpy(elf + elfIndex, &OperandSpecifier, 2); 
                elfIndex += 2;
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

        else if(Operand2.Type == emiRegisterOperand)
        {
            int Prefix = emiREX_W;
            int OpCode = emiMov64bitOpCode;
            int OperandSpecifier = emiMovRegisterMode + (Operand2.Data.IntValue << 3) + Operand1.Data.IntValue;     
            memcpy(elf + elfIndex, &Prefix, 1);
            elfIndex++;
            mempcpy(elf + elfIndex, &OpCode, 1);
            elfIndex++;
            memcpy(elf + elfIndex, &OperandSpecifier, 1);
            elfIndex++;

            return elfIndex - OldIndex;
        }

        else if(Operand2.Type == emiIntValueOperand)
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

        else if(Operand2.Type == emiDoubleValueOperand)
        {
            int Prefix = emiREX_W;
            int OpCode = emiMov64bitImmOpCode + Operand1.Data.IntValue;
            memcpy(elf + elfIndex, &Prefix, 1);
            elfIndex++;
            mempcpy(elf + elfIndex, &OpCode, 1);
            elfIndex++;

            memcpy(elf + elfIndex, &Operand2.Data.Int64Value, 8);
            elfIndex += 8;
            
            return elfIndex - OldIndex;
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
    return elfIndex - OldIndex;
}

size_t EmitPop(int RegisterName, char* elf, size_t elfIndex)
{
    size_t OldIndex = elfIndex;
    if(RegisterName <= emiGeneralPurposeRegistersNamesSectionEnd && RegisterName >= emiGeneralPurposeRegistersNamesSectionStart)
    {
        int OpCode = emiPopOpCode + RegisterName - emiGeneralPurposeRegistersNamesSectionStart;
        memcpy(elf + elfIndex, &OpCode, 1);
        elfIndex += 1;
        
    }
    else if(RegisterName <= emiExtendedRegistersNamesSectionEnd && RegisterName >= emiExtendedRegistersNamesSectionStart)
    {
        int Prefix = emiREX_B;
        memcpy(elf + elfIndex, &Prefix, 1);
        elfIndex += 1;
        int OpCode = emiPopOpCode + RegisterName - emiExtendedRegistersNamesSectionStart;
        memcpy(elf + elfIndex, &OpCode, 1);
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

size_t  EmitSSE2Ariphmetics(int Operation, EmitterOperand Operand1, EmitterOperand Operand2, char* elf, size_t elfIndex)
{
    size_t OldIndex = elfIndex;
    int Prefix = emiSSE;
    int OpCode = Operation;

    int OperandSpecifier = emiMovRegisterMode + ((Operand1.Data.IntValue - emiAVXRegistersNamesSectionStart) << 3) + Operand2.Data.IntValue - emiAVXRegistersNamesSectionStart;

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
    int OperandSpecifier = emiMovNoDisplacementMode + ((Operand1.Data.IntValue - emiAVXRegistersNamesSectionStart) << 3) +  Operand2.Data.IntValue;

    memcpy(elf + elfIndex, &Prefix, 1);
    elfIndex++;
    mempcpy(elf + elfIndex, &OpCode, 2);
    elfIndex += 2;
    memcpy(elf + elfIndex, &OperandSpecifier, 1); 
    elfIndex++;
    int MagicNumber = 0x24;
    memcpy(elf + elfIndex, &MagicNumber, 1);
    elfIndex++;

    return elfIndex - OldIndex;
}

size_t EmitMovq(EmitterOperand Operand1, EmitterOperand Operand2, char* elf, size_t elfIndex)
{
    size_t OldIndex = elfIndex;
    int Prefix = emiXMMtoMem;
    int OpCode = emiStoreSSEOpCode;
    int OperandSpecifier = emiMovNoDisplacementMode + ((Operand2.Data.IntValue - emiAVXRegistersNamesSectionStart) << 3) + Operand1.Data.IntValue;

    memcpy(elf + elfIndex, &Prefix, 1);
    elfIndex++;
    mempcpy(elf + elfIndex, &OpCode, 2);
    elfIndex += 2;
    memcpy(elf + elfIndex, &OperandSpecifier, 1); 
    elfIndex++;
    int SIB = emiSIB;
    memcpy(elf + elfIndex, &SIB, 1);
    elfIndex++;

    return elfIndex - OldIndex;
}

size_t EmitConditionalJump(int Operation, int Offset, char* elf, size_t elfIndex)
{
    Offset -= 6;
    Offset -= (int)elfIndex;
    size_t OldIndex = elfIndex;
    mempcpy(elf + elfIndex, &Operation, 2);
    elfIndex += 2;
    mempcpy(elf + elfIndex, &Offset, 2);
    elfIndex += 4;
    return elfIndex - OldIndex;
}