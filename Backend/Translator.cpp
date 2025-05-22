#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Emitter.h"
#include "Backend.h"

#include "Backend.h"
#include "../FileBufferizer/FileBufferizer.h"

#include <ctype.h>
#include <assert.h>

static size_t GetFloatNumberDigits(char* FloatNumberStr);
static size_t GetNumberDigits(int Number);

static int GetKeyWordCode(const char* IRStatement, size_t* BufferIndex);
static int Get64ByteVariableBinaryString(void* Variable, char* String);

static int LabelTableAppend(BackendLabelTable* LabelTable, char* Name, int Address);
static int LabelTableSearch(BackendLabelTable* LabelTable, char* Name);

static size_t TranslateAssignment(char* Arguments, char* elf, TranslatorFunction_t* CurrentFunction     , size_t* ip, BackendLabelTable* LabelTable);
static size_t TranslateFunctionCall(char* Arguments, char* elf, TranslatorFunction_t* CurrentFunction   , size_t* ip, BackendLabelTable* LabelTable);
static size_t TranslateFunctionBody(char* Arguments, char* elf, TranslatorFunction_t* CurrentFunction   , size_t* ip, BackendLabelTable* LabelTable);
static size_t TranslateReturn(char* Arguments, char* elf, TranslatorFunction_t* CurrentFunction         , size_t* ip, BackendLabelTable* LabelTable);
static size_t TranslateLabel(char* Arguments, char* elf, TranslatorFunction_t* CurrentFunction          , size_t* ip, BackendLabelTable* LabelTable);
static size_t TranslateConditionalJump(char* Arguments, char* elf, TranslatorFunction_t* CurrentFunction, size_t* ip, BackendLabelTable* LabelTable);
static size_t TranslateOperation(char* Arguments, char* elf, TranslatorFunction_t* CurrentFunction      , size_t* ip, BackendLabelTable* LabelTable);


static int SearchSystemCall(char* FunctionLabel);

static const size_t InitialTableSize;

// asm listiing
int main()
{
    FILE* IRFile = fopen("../ir.pyam", "r+b");
    if(IRFile == NULL)
    {
        return -1;
    }

    size_t IRFileSize = GetFileSize(IRFile);

    char* Buffer = FileToString(IRFile);

    if(Buffer == NULL)
    {
        fclose(IRFile);
        return -1;
    }
    FILE* elf = fopen("gnome.o", "w+b");
    assert(elf);

    TranslatorFunction_t ZeroFunction = {};
    ZeroFunction.Index = -1;
    ZeroFunction.NumberOfArguments = 0;
    ZeroFunction.NumberOfLocalVariables = 0;

    TranslatorFunction_t CurrentFunction = ZeroFunction;

    fprintf(elf, 
                      "section .text\n\n"
                      "global _start\n"
                      "_start:\n"
                      "\tcall main\n"
                      "\tcall halt\n");

    BackendLabelTable LabelTable = {};
    LabelTable.Labels = (BackendLabel*)calloc(sizeof(BackendLabel), InitialTableSize);
    assert(LabelTable.Labels);
    LabelTable.Capacity = InitialTableSize;
    size_t ip = 0;
    char* elf = (char*)calloc(8192, 1); //FIXME -
    assert(elf);
    for(size_t i = 0; i < IRFileSize; i++)
    {
        while(!isalpha(Buffer[i]) && i < IRFileSize)
        {
            i++;
        }

        int KeyWordCode = GetKeyWordCode(Buffer + i, &i); 

        switch(KeyWordCode)
        {
            case IR_FUNCTION_BODY_INDEX:
            {
                i += TranslateFunctionBody(Buffer + i, elf, &CurrentFunction, &ip, &LabelTable);
                while(*(Buffer + i) != '\n' && i < IRFileSize)
                {
                    i++;
                }
                break;
            }
            case IR_FUNCTION_CALL_INDEX:
            {
                i += TranslateFunctionCall(Buffer + i, elf, &CurrentFunction, &ip, &LabelTable);
                while(*(Buffer + i) != '\n' && i < IRFileSize)
                {
                    i++;
                }
                break;
            }
            case IR_ASSIGNMENT_INDEX:
            {
                i += TranslateAssignment(Buffer + i, elf, &CurrentFunction, &ip, &LabelTable);
                while(*(Buffer + i) != '\n' && i < IRFileSize)
                {
                    i++;
                }
                break;
            }
            case IR_RETURN_INDEX:
            {
                i += TranslateReturn(Buffer + i, elf, &CurrentFunction, &ip, &LabelTable);
                while(*(Buffer + i) != '\n' && i < IRFileSize)
                {
                    i++;
                }
                break;
            }
            case IR_LABEL_INDEX:
            {
                i += TranslateLabel(Buffer + i, elf, &CurrentFunction, &ip, &LabelTable);
                while(*(Buffer + i) != '\n' && i < IRFileSize)
                {
                    i++;
                }
                break;
            }
            case IR_CONDITIONAL_JUMP_INDEX:
            {
                i += TranslateConditionalJump(Buffer + i, elf, &CurrentFunction, &ip, &LabelTable);
                while(*(Buffer + i) != '\n' && i < IRFileSize)
                {
                    i++;
                }
                break;
            }
            case IR_OPERATION_INDEX:
            {
                i += TranslateOperation(Buffer + i, elf, &CurrentFunction, &ip, &LabelTable);
                while(*(Buffer + i) != '\n' && i < IRFileSize)
                {
                    i++;
                }
                break;
            }
            case IR_SYSTEM_FUNCTION_CALL_INDEX:
            {
                i += TranslateFunctionCall(Buffer + i, elf, &CurrentFunction, &ip, &LabelTable);
                while(*(Buffer + i) != '\n' && i < IRFileSize)
                {
                    i++;
                }
                break;
            }
            default:
            {
                assert(0);
                break;
            }
        }
    }

    //fprintf(elf, "%s\n", SystemCallOut);
    //
    //fprintf(elf, "%s\n", SystemCallHalt);
    //fprintf(elf, "%s\n", SystemCallIn);
    free(Buffer);
    free(elf);
    free(LabelTable.Labels);
    fclose(IRFile);

    //fclose(elf);
}

static size_t TranslateConditionalJump(char* Arguments, char* elf, __attribute((unused))TranslatorFunction_t* CurrentFunction, size_t* ip, BackendLabelTable* LabelTable)
{
    assert(Arguments);
    assert(elf);

    assert(*Arguments == '(');
    
    size_t LocalBufferIndex = 1;
    
    char LabelName[TRANSLATOR_FUNCTION_LABEL_NAME_MAX] = {};

    for(size_t i = 0; i < TRANSLATOR_FUNCTION_LABEL_NAME_MAX; i++)
    {
        if(Arguments[LocalBufferIndex + i] == ',')
        {
            LabelName[i] = '\0';
            LocalBufferIndex += i;
            break;
        }
        LabelName[i] = Arguments[LocalBufferIndex + i];
    }
    
    LocalBufferIndex += 2;
    if(isdigit(Arguments[LocalBufferIndex])) // Normal jump TODO push imm
    {
        //fprintf(elf, "\tpush 1\n");
        ip += EmitPush(1, elf, *ip);
    }

    while(Arguments[LocalBufferIndex] != ')')
    {
        LocalBufferIndex++;
    }

    LocalBufferIndex += 2;


    //fprintf(elf, 
    //            "\tpop rdx ; condition\n"
    //            "\tcmp rdx, 0\n"
    //            "\tjne .%s\n", LabelName);
    ip += EmitPop(emi_rdx, elf, *ip);
    EmitterOperand Op1 = {};
    Op1.Type = emiRegisterOperand;
    Op1.Data.IntValue = emi_rdx;
    EmitterOperand Op2 = {};
    Op2.Type = emiIntValueOperand;
    Op2.Data.IntValue = 0;
    ip += EmitCmp(Op1, Op2, elf, *ip);

    int Shift = 0;
    int LabelIndex = LabelTableSearch(LabelTable, LabelName);
    if(LabelIndex == -1)
    {
        LabelTableAppend(LabelTable, LabelName, 0);
    }
    else
    {
        Shift = LabelTable->Labels[LabelIndex].Address;
    }

    ip += EmitConditionalJump(emiNearJneOpCode, Shift, elf, *ip);

    return LocalBufferIndex;
}

static int LabelTableSearch(BackendLabelTable* LabelTable, char* Name)
{
    assert(LabelTable);
    assert(Name);

    for(size_t i = 0; i < LabelTable->Free; i++)
    {
        if(strncmp(Name, LabelTable->Labels[i].Name, strlen(LabelTable->Labels[i].Name)) == 0)
        {
            return i;
        }
    }
    return -1;
}

static int LabelTableAppend(BackendLabelTable* LabelTable, char* Name, int Address)
{
    assert(LabelTable);
    assert(Name);

    for(size_t i = 0; i < LabelTable->Free; i++)
    {
        if(strncmp(Name, LabelTable->Labels[i].Name, strlen(LabelTable->Labels[i].Name)) == 0)
        {
            if(LabelTable->Labels[i].Address == 0)
            {
                LabelTable->Labels[i].Address = Address;
                return;
            }
            else
            {
                assert(0);
            }
        }
    }

    if(LabelTable->Free >= LabelTable->Capacity)
    {
        LabelTable->Labels = (BackendLabel*)realloc(LabelTable->Labels, LabelTable->Capacity * 4);
        LabelTable->Capacity *= 4;
        assert(LabelTable->Labels);
    }

    LabelTable->Labels[LabelTable->Free].Address = Address;
    strncpy(LabelTable->Labels[LabelTable->Free].Name, Name, KEY_WORD_NAME_MAX);

    return 0;
}

static size_t TranslateLabel(char* Arguments, char* elf, __attribute((unused))TranslatorFunction_t* CurrentFunction, size_t* ip, BackendLabelTable* LabelTable)
{
    assert(Arguments);
    assert(elf);

    assert(*Arguments == '(');
    
    size_t LocalBufferIndex = 1;

    char LabelName[TRANSLATOR_FUNCTION_LABEL_NAME_MAX] = {};

    for(size_t i = 0; i < TRANSLATOR_FUNCTION_LABEL_NAME_MAX; i++)
    {
        if(Arguments[LocalBufferIndex + i] == ')')
        {
            LabelName[i] = '\0';
            LocalBufferIndex += i;
            break;
        }
        LabelName[i] = Arguments[LocalBufferIndex + i];
    }

    //fprintf(elf, "\n.%s:\n", LabelName);
    int LabelIndex = LabelTableSearch(LabelTable, LabelName);
    if(LabelIndex == -1)
    {
        LabelTableAppend(LabelTable, LabelName, *ip);
    }
    else
    {
        fprintf(stderr, "scrubbish redefenition\n");
        assert(0);
    }


    LocalBufferIndex += 1;

    return LocalBufferIndex;
}

static size_t TranslateReturn(char* Arguments, char* elf, __attribute((unused))TranslatorFunction_t* CurrentFunction, size_t* ip, BackendLabelTable* LabelTable)
{
    assert(Arguments);
    assert(elf);

    assert(*Arguments == '(');
    
    size_t LocalBufferIndex = 1;

    //fprintf(elf, 
    //            "\tpop rax ; return\n"
    //            "\tpop rbp\n"
    //            "\tpop r10\n"
    //            "\tpop r9\n"
    //            "\tpop r8\n"
    //            "\tpop rsi\n"
    //            "\tpop rdi\n"
    //            "\tpop rdx\n"
    //            "\tpop rcx\n"
    //            "\tpop rbx\n"
    //            "\tadd rsp, %d\n"
    //            "\tret\n\n", (int)CurrentFunction->NumberOfLocalVariables * 8);
    ip += EmitPop(emi_rax, elf, *ip);
    ip += EmitPop(emi_rbp, elf, *ip);
    ip += EmitPop(emi_r10, elf, *ip);
    ip += EmitPop(emi_r9, elf, *ip);
    ip += EmitPop(emi_r8, elf, *ip);
    ip += EmitPop(emi_rsi, elf, *ip);
    ip += EmitPop(emi_rdi, elf, *ip);
    ip += EmitPop(emi_rdx, elf, *ip);
    ip += EmitPop(emi_rcx, elf, *ip);
    ip += EmitPop(emi_rbx, elf, *ip);
    EmitterOperand Op1 = {};
    Op1.Type = emiRegisterOperand;
    Op1.Data.IntValue = emi_rsp;
    EmitterOperand Op2 = {};
    Op2.Type = emiIntValueOperand;
    Op2.Data.IntValue = (int)CurrentFunction->NumberOfLocalVariables * 8;
    ip += EmitAdd(Op1, Op2, elf, *ip);
    
    return LocalBufferIndex;
}

static size_t TranslateFunctionBody(char* Arguments, char* elf, TranslatorFunction_t* CurrentFunction, size_t* ip, BackendLabelTable* LabelTable)
{
    assert(Arguments);
    assert(elf);

    assert(*Arguments == '(');

    size_t LocalBufferIndex = 1;

    char FunctionLabel[TRANSLATOR_FUNCTION_LABEL_NAME_MAX] = {};

    for(size_t i = 0; i < TRANSLATOR_FUNCTION_LABEL_NAME_MAX; i++)
    {
        if(*(Arguments + LocalBufferIndex + i) == ',')
        {
            FunctionLabel[i++] = '\0';
            LocalBufferIndex += i;
            break;
        }
        FunctionLabel[i] = *(Arguments + LocalBufferIndex + i);
    }


    if(strncmp(FunctionLabel, "main", 4) == 0)
    {
       // fprintf(elf, "\n\nmain:\n"); // TODO
    }
    else
    {
        //fprintf(elf, "\n\n%s:\n", FunctionLabel); // TODO
    }

    LocalBufferIndex++;
    int ArgumentsNumber = atoi(Arguments + LocalBufferIndex);
    LocalBufferIndex += GetNumberDigits(ArgumentsNumber);

    LocalBufferIndex += 2;

    int LocalVariablesNumber = atoi(Arguments + LocalBufferIndex);
    LocalBufferIndex += GetNumberDigits(LocalVariablesNumber);

    //fprintf(elf, 
    //            "\tsub rsp, %d ; Stack frame creation\n"
    //            "\tpush rbx\n"
    //            "\tpush rcx\n"
    //            "\tpush rdx\n"
    //            "\tpush rdi\n"
    //            "\tpush rsi\n"
    //            "\tpush r8\n"
    //            "\tpush r9\n"
    //            "\tpush r10\n"
    //            "\tpush rbp\n"
    //            "\tmov rcx, rsp\n"
    //            "\tadd rcx, %d + 72\n"
    //            "\tmov rbp, rcx\n"
    //            , LocalVariablesNumber * 8, 8 * (LocalVariablesNumber - 1));

    

    if(ArgumentsNumber > 0)
    {
        for(int i = ArgumentsNumber; i > 0; i--)
        {
            //fprintf(elf, 
            //            "\tmov rbx, [rbp + %d]\n"
            //            "\tmov [rbp - %d], rbx\n\n", 8 + i * 8, ArgumentsNumber - i);
        }

    }
    else
    {
        //fprintf(elf, "\n");
    }
    
    LocalBufferIndex += 3;

    strncpy(CurrentFunction->Label, FunctionLabel, strlen(FunctionLabel));
    CurrentFunction->NumberOfArguments = (size_t)ArgumentsNumber;
    CurrentFunction->NumberOfLocalVariables = (size_t)LocalVariablesNumber;

    return LocalBufferIndex;
}

static size_t TranslateFunctionCall(char* Arguments, char* elf, __attribute((unused))TranslatorFunction_t* CurrentFunction, size_t* ip)
{
    assert(Arguments);
    assert(elf);

    assert(*Arguments == '(');
    
    size_t LocalBufferIndex = 1;

    while(!isdigit(Arguments[LocalBufferIndex]))
    {
        LocalBufferIndex++;
    }
    int ReturnTmpIndex = atoi(Arguments + LocalBufferIndex);

    char FunctionLabel[TRANSLATOR_FUNCTION_LABEL_NAME_MAX] = {};
    while(Arguments[LocalBufferIndex] != ' ')
    {
        LocalBufferIndex++;
    }

    LocalBufferIndex++;

    for(size_t i = 0; i < TRANSLATOR_FUNCTION_LABEL_NAME_MAX; i++)
    {
        if(*(Arguments + LocalBufferIndex + i) == ',' || *(Arguments + LocalBufferIndex + i) == ')')
        {
            FunctionLabel[i++] = '\0';
            LocalBufferIndex += i;
            break;
        }
        FunctionLabel[i] = *(Arguments + LocalBufferIndex + i);
    }

    LocalBufferIndex += 2;

    int ArgNumber = 0;
    int SysCallIndex = SearchSystemCall(FunctionLabel);
    if(SysCallIndex != -1)
    {
        ArgNumber = kIR_SYS_CALL_ARRAY[SysCallIndex].NumberOfArguments;  
    }
    else
    {
        int ArgnumStringIndex = 5;
        int CalleeLocalVariablesNum = atoi(FunctionLabel + ArgnumStringIndex);
        ArgnumStringIndex += (int)GetNumberDigits(CalleeLocalVariablesNum);
        ArgnumStringIndex++;
    
        ArgNumber = atoi(FunctionLabel + ArgnumStringIndex);
    }
    size_t NextUsedTmpIndex = LocalBufferIndex;
    while(Arguments[NextUsedTmpIndex] != '\n')
    {
        NextUsedTmpIndex++;
    }
    while(!isalpha(Arguments[NextUsedTmpIndex]))
    {
        NextUsedTmpIndex++;
    }
    int ReturnFlag = 0;
    if(GetKeyWordCode(Arguments + NextUsedTmpIndex, &NextUsedTmpIndex) == IR_ASSIGNMENT_INDEX)
    {
        while(Arguments[NextUsedTmpIndex] != ' ')
        {
            NextUsedTmpIndex++;
        }
        while(!isdigit(Arguments[NextUsedTmpIndex]))
        {
            NextUsedTmpIndex++;
        }

        int NextTmp = atoi(Arguments + NextUsedTmpIndex);
        if(NextTmp == ReturnTmpIndex)
        {
            ReturnFlag = 1;
        }
    }

    if(ReturnFlag)
    {
        fprintf(elf,
                    "\tcall %s\n"
                    "\tadd rsp, %d\n"
                    "\tpush rax\n\n", FunctionLabel, 8 * ArgNumber);
        LocalBufferIndex = NextUsedTmpIndex;
    }
    else
    {
        fprintf(elf,
                    "\tcall %s\n"
                    "\tadd rsp, %d\n\n", FunctionLabel, 8* ArgNumber);
    }

    return LocalBufferIndex;
}

static int SearchSystemCall(char* FunctionLabel)
{
    assert(FunctionLabel);

    for(size_t i = 0; i < kIR_SYS_CALL_NUMBER; i++)
    {
        if(strncmp(FunctionLabel, kIR_SYS_CALL_ARRAY[i].Name, strlen(kIR_SYS_CALL_ARRAY[i].Name)) == 0)
        {
            return (int)i;
        }
    }

    return -1;
}

static size_t GetNumberDigits(int Number)
{
    size_t Digits = 0;
    if(Number < 0)
    {
        Digits++;
        Number = Number * (-1);
    }
    if(Number == 0)
    {
        return 1;
    }
    while(Number > 0)
    {
        Digits++;
        Number /= 10;
    }
    
    return Digits;
}

static size_t TranslateAssignment(char* Arguments, char* elf, __attribute((unused))TranslatorFunction_t* CurrentFunction, size_t* ip)
{
    assert(Arguments);
    assert(elf);
    assert(*Arguments == '(');
    
    size_t LocalBufferIndex = 1;

    char OperandType[TRANSLATOR_FUNCTION_LABEL_NAME_MAX] = {};
    for(size_t i = 0; i < TRANSLATOR_FUNCTION_LABEL_NAME_MAX; i++)
    {
        if(isdigit(Arguments[LocalBufferIndex + i]))
        {
            OperandType[i] = '\0';
            LocalBufferIndex += i;
            break;
        }
        OperandType[i] = Arguments[LocalBufferIndex + i];
    }

    if(strcmp(OperandType, "tmp") == 0)
    {
        while(Arguments[LocalBufferIndex] != ' ')
        {
            LocalBufferIndex++;
        }
        LocalBufferIndex++;

        if(isalpha(Arguments[LocalBufferIndex]))
        {
            while(!isdigit(Arguments[LocalBufferIndex]))
            {
                LocalBufferIndex++;
            }
            
            int LocalVariableIndex = atoi(Arguments + LocalBufferIndex);
            LocalBufferIndex += GetNumberDigits(LocalVariableIndex);

            fprintf(elf, 
                        "\tmov r8, [rbp + 8 * %d]\n"
                        "\tpush r8\n\n", LocalVariableIndex);
        }
        else if(isdigit(Arguments[LocalBufferIndex]))
        {
            double NumericalValue = atof(Arguments + LocalBufferIndex);
            LocalBufferIndex += GetFloatNumberDigits(Arguments + LocalBufferIndex);

            char BinaryFloatRepresentation[sizeof(double) * 8 + 2] = {};
            Get64ByteVariableBinaryString(&NumericalValue, BinaryFloatRepresentation);

            fprintf(elf,
                        "\tmov rax, %s\n"
                        "\tpush rax\n\n", BinaryFloatRepresentation);
        }
        else
        {
            assert(0);
        }
    }
    else if(strcmp(OperandType, "var") == 0)
    {
        int LocalVarIndex = atoi(Arguments + LocalBufferIndex);
        LocalBufferIndex += GetNumberDigits((int)LocalBufferIndex);

        while(Arguments[LocalBufferIndex] != ',')
        {
            LocalBufferIndex++;
        }

        LocalBufferIndex += 2;

        if(Arguments[LocalBufferIndex] == 't')
        {
            while(Arguments[LocalBufferIndex] != ')')
            {
                LocalBufferIndex++;
            }

            LocalBufferIndex += 2;
            fprintf(elf, 
                "\tpop r8 ; assigning value to variable\n"
                //"\tmov [rbp + 8 * %d], r8\n\n"
                "\tmov rdi, rbp\n"
                "\tadd rdi, 8 * %d\n"
                "\tmov [rdi], r8\n\n", LocalVarIndex);
        }
        else if(Arguments[LocalBufferIndex] == 'a')
        {
            while(Arguments[LocalBufferIndex] != ')')
            {
                LocalBufferIndex++;
            }

            LocalBufferIndex += 2;
        }
        else
        {
            assert(0);
        }
    }
    else if(strcmp(OperandType, "arg") == 0)
    {
        return LocalBufferIndex;
    }
    else
    {
        assert(0);
    }

    LocalBufferIndex++;


    return LocalBufferIndex;
}

static size_t TranslateOperation(char* Arguments, char* elf, TranslatorFunction_t* CurrentFunction, size_t* ip)
{
    assert(Arguments);
    assert(elf);
    assert(CurrentFunction);

    assert(*Arguments == '(');

    size_t LocalBufferIndex = 1;

    while(Arguments[LocalBufferIndex] != ',')
    {
        LocalBufferIndex++;
    }

    LocalBufferIndex++;

    int OperationIndex = atoi(Arguments + LocalBufferIndex);

    LocalBufferIndex += GetNumberDigits(OperationIndex);

    fprintf(elf, 
                "\tmovsd xmm0, [rsp] ; moving tmp variables from stack to xmm registers\n"
                "\tadd rsp, 8\n"
                "\tmovsd xmm1, [rsp]\n"
                "\tadd rsp, 8\n");

    switch(OperationIndex)
    {
        case IR_OP_TYPE_MUL:
        {
            fprintf(elf, 
                        "\tmulsd xmm1, xmm0 ; multiplication\n"
                        "\tsub rsp, 8\n"
                        "\tmovq [rsp], xmm1\n\n");
            break;
        }
        case IR_OP_TYPE_DIV:
        {
            fprintf(elf, 
                        "\tdivsd xmm1, xmm0 ; division\n"
                        "\tsub rsp, 8\n"
                        "\tmovq [rsp], xmm1\n\n");
            break;
        }
        case IR_OP_TYPE_EQ:
        {
            fprintf(elf, 
                        "\tcmpsd xmm1, xmm0, 0 ; equality check\n"
                        "\tmovq [rsp], xmm1\n"
                        "\tsub rsp, 8\n"
                        "\tpop r8\n"
                        "\tshr r8, 31\n"
                        "\tpush r8\n\n");
            break;
        }
        case IR_OP_TYPE_GREAT:
        {
            fprintf(elf, 
                        "\tcmpsd xmm1, xmm0, 6 ; greater check\n"
                        "\tmovq [rsp], xmm1\n"
                        "\tsub rsp, 8\n"
                        "\tpop r8\n"
                        "\tshr r8, 31\n"
                        "\tpush r8\n\n");
            break;
        }
        case IR_OP_TYPE_GREATEQ:
        {
            fprintf(elf, 
                        "\tcmpsd xmm1, xmm0, 5 ; greater or equal check\n"
                        "\tmovq [rsp], xmm1\n"
                        "\tsub rsp, 8\n"
                        "\tpop r8\n"
                        "\tshr r8, 31\n"
                        "\tpush r8\n\n");
            break;
        }
        case IR_OP_TYPE_LESS:
        {
            fprintf(elf, 
                        "\tcmpsd xmm1, xmm0, 1 ; less check\n"
                        "\tmovq [rsp], xmm1\n"
                        "\tsub rsp, 8\n"
                        "\tpop r8\n"
                        "\tshr r8, 31\n"
                        "\tpush r8\n\n");
            break;
        }
        case IR_OP_TYPE_LESSEQ:
        {
            fprintf(elf, 
                        "\tcmpsd xmm1, xmm0, 2 ; less or equal check\n"
                        "\tmovq [rsp], xmm1\n"
                        "\tsub rsp, 8\n"
                        "\tpop r8\n"
                        "\tshr r8, 31\n"
                        "\tpush r8\n\n");
            break;
        }
        case IR_OP_TYPE_NEQ:
        {
            fprintf(elf, 
                        "\tcmpsd xmm1, xmm0, 4 ; not equal check\n"
                        "\tmovq [rsp], xmm1\n"
                        "\tsub rsp, 8\n"
                        "\tpop r8\n"
                        "\tshr r8, 31\n"
                        "\tpush r8\n\n");
            break;
        }
        case IR_OP_TYPE_SUB:
        {
            fprintf(elf, 
                        "\tsubsd xmm1, xmm0 ; substraction\n"
                        "\tsub rsp, 8\n"
                        "\tmovq [rsp], xmm1\n\n");
            break;
        }
        case IR_OP_TYPE_SUM:
        {
            fprintf(elf, 
                        "\taddsd xmm1, xmm0 ; sum\n"
                        "\tsub rsp, 8\n"
                        "\tmovq [rsp], xmm1\n\n");
            break;
        }
        default:
        {
            assert(0);
        }

    }



    return LocalBufferIndex;
}

static int Get64ByteVariableBinaryString(void* Variable, char* String)
{
    assert(Variable);
    size_t Mask = 1;
    for(size_t i = 0; i < 64; i++)
    {
        if(*(size_t*)(Variable) & Mask)
        {
            String[63 - i] = '1';
        }
        else
        {
            String[63 - i] = '0';
        }
        Mask *= 2;
    }
    String[64] = 'b';
    String[65] = '\0';

    return 0;
}

static size_t GetFloatNumberDigits(char* FloatNumberStr)
{
    assert(FloatNumberStr);
    size_t Digits = 0;
    while(FloatNumberStr[Digits] != '.' && FloatNumberStr[Digits] != ')')
    {
        Digits++;
    }

    if(FloatNumberStr[Digits] == '.')
    {
        Digits++;
        while(isdigit(FloatNumberStr[Digits]))
        {
            Digits++;
        }
    }
    
    return Digits;
}

static int GetKeyWordCode(const char* IRStatement, size_t* BufferIndex)
{
    assert(IRStatement);

    const char* KeyWordEnd = strchr(IRStatement, '(');

    size_t KeyWordSize = (size_t)KeyWordEnd - (size_t)IRStatement;

    const char KeyWord[KEY_WORD_NAME_MAX] = {};

    memcpy((void*)KeyWord, IRStatement, KeyWordSize);
    fprintf(stderr, "Parsing key word: %s\n", KeyWord);
    
    int KeyWordIndex = -1;
    for(int i = 0; i < (int)kIR_KEY_WORD_NUMBER; i++)
    {
        if(strncmp(KeyWord, kIR_KEY_WORD_ARRAY[i], strlen(kIR_KEY_WORD_ARRAY[i])) == 0)
        {
            KeyWordIndex = i;
            break;
        }
    }

    (*BufferIndex) += KeyWordSize;
    return KeyWordIndex;
}