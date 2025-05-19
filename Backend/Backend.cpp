#include "Backend.h"
#include "../FileBufferizer/FileBufferizer.h"

#include <ctype.h>
#include <assert.h>

static size_t GetFloatNumberDigits(char* FloatNumberStr);
static size_t GetNumberDigits(int Number);

static int GetKeyWordCode(const char* IRStatement, size_t* BufferIndex);
static int Get64ByteVariableBinaryString(void* Variable, char* String);

static size_t TranslateAssignment(char* Arguments, FILE* elf, TranslatorFunction_t* CurrentFunction);
static size_t TranslateFunctionCall(char* Arguments, FILE* elf, TranslatorFunction_t* CurrentFunction);
static size_t TranslateFunctionBody(char* Arguments, FILE* elf, TranslatorFunction_t* CurrentFunction);
static size_t TranslateReturn(char* Arguments, FILE* elf, TranslatorFunction_t* CurrentFunction);
static size_t TranslateLabel(char* Arguments, FILE* elf, TranslatorFunction_t* CurrentFunction);
static size_t TranslateConditionalJump(char* Arguments, FILE* elf, TranslatorFunction_t* CurrentFunction);
static size_t TranslateOperation(char* Arguments, FILE* elf, TranslatorFunction_t* CurrentFunction);


static int SearchSystemCall(char* FunctionLabel);


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
    FILE* outputELF = fopen("gnome.asm", "w+b");
    assert(outputELF);

    TranslatorFunction_t ZeroFunction = {};
    ZeroFunction.Index = -1;
    ZeroFunction.NumberOfArguments = 0;
    ZeroFunction.NumberOfLocalVariables = 0;

    TranslatorFunction_t CurrentFunction = ZeroFunction;

    fprintf(outputELF, 
                      "section .text\n\n"
                      "global _start\n"
                      "_start:\n"
                      "\tcall main\n"
                      "\tcall halt\n");
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
                i += TranslateFunctionBody(Buffer + i, outputELF, &CurrentFunction);
                while(*(Buffer + i) != '\n' && i < IRFileSize)
                {
                    i++;
                }
                break;
            }
            case IR_FUNCTION_CALL_INDEX:
            {
                i += TranslateFunctionCall(Buffer + i, outputELF, &CurrentFunction);
                while(*(Buffer + i) != '\n' && i < IRFileSize)
                {
                    i++;
                }
                break;
            }
            case IR_ASSIGNMENT_INDEX:
            {
                i += TranslateAssignment(Buffer + i, outputELF, &CurrentFunction);
                while(*(Buffer + i) != '\n' && i < IRFileSize)
                {
                    i++;
                }
                break;
            }
            case IR_RETURN_INDEX:
            {
                i += TranslateReturn(Buffer + i, outputELF, &CurrentFunction);
                while(*(Buffer + i) != '\n' && i < IRFileSize)
                {
                    i++;
                }
                break;
            }
            case IR_LABEL_INDEX:
            {
                i += TranslateLabel(Buffer + i, outputELF, &CurrentFunction);
                while(*(Buffer + i) != '\n' && i < IRFileSize)
                {
                    i++;
                }
                break;
            }
            case IR_CONDITIONAL_JUMP_INDEX:
            {
                i += TranslateConditionalJump(Buffer + i, outputELF, &CurrentFunction);
                while(*(Buffer + i) != '\n' && i < IRFileSize)
                {
                    i++;
                }
                break;
            }
            case IR_OPERATION_INDEX:
            {
                i += TranslateOperation(Buffer + i, outputELF, &CurrentFunction);
                while(*(Buffer + i) != '\n' && i < IRFileSize)
                {
                    i++;
                }
                break;
            }
            case IR_SYSTEM_FUNCTION_CALL_INDEX:
            {
                i += TranslateFunctionCall(Buffer + i, outputELF, &CurrentFunction);
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

    fprintf(outputELF,
                      "\n\nout:\n"
                      "\tpop rax\n"
                      "\tsub rsp, 80\n"
                      "\tpush rbp\n"
                      "\tmov rbp, rsp\n"
                      "\tadd rbp, 88\n"
                      "\tpush rbx\n"
                      "\tpush rcx\n"
                      "\tpush rdx\n"
                      "\tpush rdi\n"
                      "\tpush rsi\n"
                      "\tpush r9\n"
                      "\tmov r9, rax\n"
                      "\tmov rdi, rbp\n"
                      "\tsub rdi, 1\n"
                      "\tmov qword rax, [rbp]\n\n"
                      "\tmov rbx, 01h\n"
                      "\tmov rdx, rax\n"
                      "\tmov rcx, 64\n"
                      "\tmov byte [rdi], 0x0A\n"
                      "\tdec rdi\n"
                      "llPrintingLoop:\n"
                      "\tmov rdx, rax\n"
                      "\tcmp rcx, 0x00\n"
                      "\tje llPrintingLoopEnd\n"
                      "\tand rdx, rbx\n"
                      "\tadd rdx, 0x30\n"
                      "\tmov byte [rdi], dl\n"
                      "\tdec rdi\n"
                      "\tshr rax, 1\n"
                      "\tdec rcx\n"
                      "\tjmp llPrintingLoop\n"
                      "llPrintingLoopEnd:\n"
                      "\tmov rax, 0x01\n"
                      "\tmov rsi, rdi\n"
                      "\tmov rdi, 0x01\n"
                      "\tmov rdx, 66\n"
                      "\tsyscall\n"
                      "\tsub rbp, 8\n"
                      "\tmov [rbp], r9\n"
                      "\tpop r9\n"
                      "\tpop rsi\n"
                      "\tpop rdi\n"
                      "\tpop rdx\n"
                      "\tpop rsi\n"
                      "\tpop rbx\n"
                      "\tpop rbp\n"
                      "\tadd rsp, 72\n"
                      "\tret\n\n");
    
    fprintf(outputELF, 
                      "halt:\n"
                      "\tmov rax, 0x3c\n"
                      "\tsyscall\n");
    free(Buffer);
    fclose(IRFile);
    fclose(outputELF);
}

static size_t TranslateConditionalJump(char* Arguments, FILE* elf, __attribute((unused))TranslatorFunction_t* CurrentFunction)
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
    if(isdigit(Arguments[LocalBufferIndex]))
    {
        fprintf(elf, "\tpush 1\n");
    }

    while(Arguments[LocalBufferIndex] != ')')
    {
        LocalBufferIndex++;
    }

    LocalBufferIndex += 2;

    fprintf(elf, 
                "\tpop rdx ; condition\n"
                "\tcmp rdx, 0\n"
                "\tjne %s\n", LabelName);

    return LocalBufferIndex;
}

static size_t TranslateLabel(char* Arguments, FILE* elf, __attribute((unused))TranslatorFunction_t* CurrentFunction)
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
    fprintf(stderr, "%s:\n", LabelName);

    fprintf(elf, "\n%s:\n", LabelName);

    LocalBufferIndex += 1;

    return LocalBufferIndex;
}

static size_t TranslateReturn(char* Arguments, FILE* elf, __attribute((unused))TranslatorFunction_t* CurrentFunction)
{
    assert(Arguments);
    assert(elf);

    assert(*Arguments == '(');
    
    size_t LocalBufferIndex = 1;

    fprintf(elf, 
                "\tpop rax\n ; return"
                "\tpop rbp\n"
                "\tmov rsp, r10\n"
                "\tpop rcx\n"
                "\tpush rbx\n"
                "\tret\n"
                );
    
    return LocalBufferIndex;
}

static size_t TranslateFunctionBody(char* Arguments, FILE* elf, TranslatorFunction_t* CurrentFunction)
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
        fprintf(elf, "\n\nmain:\n");
    }
    else
    {
        fprintf(elf, "\n\n%s:\n", FunctionLabel);
    }

    LocalBufferIndex++;
    int ArgumentsNumber = atoi(Arguments + LocalBufferIndex);
    LocalBufferIndex += GetNumberDigits(ArgumentsNumber);

    LocalBufferIndex += 2;

    int LocalVariablesNumber = atoi(Arguments + LocalBufferIndex);
    LocalBufferIndex += GetNumberDigits(LocalVariablesNumber);

    fprintf(elf, 
                "\tmov rcx, rsp ; stack frame cration\n"
                "\tpop rbx\n"
                "\tsub rsp, 8 * %d\n"
                "\tpush rbp\n"
                "\tmov r10, rsp\n"
                "\tadd r10, 8 * %d\n"
                "\tmov rbp, r10\n\n"
                , LocalVariablesNumber - ArgumentsNumber, LocalVariablesNumber);
    
    LocalBufferIndex += 3;

    strncpy(CurrentFunction->Label, FunctionLabel, strlen(FunctionLabel));
    CurrentFunction->NumberOfArguments = (size_t)ArgumentsNumber;
    CurrentFunction->NumberOfLocalVariables = (size_t)LocalVariablesNumber;

    return LocalBufferIndex;
}

static size_t TranslateFunctionCall(char* Arguments, FILE* elf, __attribute((unused))TranslatorFunction_t* CurrentFunction)
{
    assert(Arguments);
    assert(elf);

    assert(*Arguments == '(');
    
    size_t LocalBufferIndex = 1;

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

    fprintf(elf,
                "\tcall %s\n ; function call"
                "\tpush rax\n\n", FunctionLabel);

    return LocalBufferIndex;
}

static int SearchSystemCall(char* FunctionLabel)
{
    assert(FunctionLabel);

    for(size_t i = 0; i < kIR_SYS_CALL_NUMBER; i++)
    {
        if(strncmp(FunctionLabel, kIR_SYS_CALL_ARRAY[i].Name, strlen(kIR_SYS_CALL_ARRAY[i].Name)))
        {
            return (int)i;
        }
    }

    return -1;
}

static size_t GetNumberDigits(int Number)
{
    size_t Digits = 0;
    while(Number > 0)
    {
        Digits++;
        Number /= 10;
    }
    
    return Digits;
}

static size_t TranslateAssignment(char* Arguments, FILE* elf, __attribute((unused))TranslatorFunction_t* CurrentFunction)
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

static size_t TranslateOperation(char* Arguments, FILE* elf, TranslatorFunction_t* CurrentFunction)
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