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

    for(size_t i = 0; i < IRFileSize; i++)
    {
        while(!isalpha(Buffer[i]) && i < IRFileSize && Buffer[i] != '#')
        {
            i++;
        }

        if(Buffer[i] == '#')
        {
            while(Buffer[i] != '\n')
            {
                i++;
            }
            i++;
            continue;
        }

        int KeyWordCode = GetKeyWordCode(Buffer + i, &i);



        switch(KeyWordCode)
        {
            case IR_FUNCTION_BODY_INDEX:
            {
                i += TranslateFunctionBody(Buffer + i, outputELF, &CurrentFunction);
                break;
            }
            case IR_FUNCTION_CALL_INDEX:
            {
                i += TranslateFunctionCall(Buffer + i, outputELF, &CurrentFunction);
                break;
            }
            case IR_ASSIGNMENT_INDEX:
            {
                i += TranslateAssignment(Buffer + i, outputELF, &CurrentFunction);
                break;
            }
            case IR_RETURN_INDEX:
            {
                i += TranslateReturn(Buffer + i, outputELF, &CurrentFunction);
                break;
            }
            case IR_LABEL_INDEX:
            {
                i += TranslateLabel(Buffer + i, outputELF, &CurrentFunction);
                break;
            }
            case IR_CONDITIONAL_JUMP_INDEX:
            {
                i += TranslateConditionalJump(Buffer + i, outputELF, &CurrentFunction);
                break;
            }
            case IR_OPERATION_INDEX:
            {

            }
            default:
            {
                assert(0);
                break;
            }
        }
    }
    free(Buffer);
    fclose(IRFile);
    fclose(outputELF);
}

static size_t TranslateConditionalJump(char* Arguments, FILE* elf, TranslatorFunction_t* CurrentFunction)
{
    assert(Arguments);
    assert(elf);

    assert(*Arguments == '(');
    
    size_t LocalBufferIndex = 1;
    
    char LabelName[TRANSLATOR_FUNCTION_LABEL_NAME_MAX] = {};

    for(size_t i = 0; i < LocalBufferIndex; i++)
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
                "\tpop rdx\n"
                "\tcmp rdx, 0\n"
                "\tjne %s, 0\n", LabelName);

    return LocalBufferIndex;
}

static size_t TranslateLabel(char* Arguments, FILE* elf, TranslatorFunction_t* CurrentFunction)
{
    assert(Arguments);
    assert(elf);

    assert(*Arguments == '(');
    
    size_t LocalBufferIndex = 1;

    char LabelName[TRANSLATOR_FUNCTION_LABEL_NAME_MAX] = {};

    for(size_t i = 0; i < LocalBufferIndex; i++)
    {
        if(Arguments[LocalBufferIndex + i] == ')')
        {
            LabelName[i] = '\0';
            LocalBufferIndex += i;
            break;
        }
        LabelName[i] = Arguments[LocalBufferIndex + i];
    }

    fprintf(elf, "%s:\n", LabelName);

    LocalBufferIndex += 1;

    return LocalBufferIndex;
}

static size_t TranslateReturn(char* Arguments, FILE* elf, TranslatorFunction_t* CurrentFunction)
{
    assert(Arguments);
    assert(elf);

    assert(*Arguments == '(');
    
    size_t LocalBufferIndex = 1;

    fprintf(elf, 
                "\tpop rax\n"
                "\tpop rbp\n"
                "\tmov rsp, r1\n"
                "\tpop rcx\n"
                "\tpush rbx\n"
                );
    
    while(Arguments[LocalBufferIndex] != '\n')
    {
        LocalBufferIndex++;
    }
    LocalBufferIndex++;
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

    fprintf(elf, "%s:\n", FunctionLabel);

    LocalBufferIndex++;
    int ArgumentsNumber = atoi(Arguments + LocalBufferIndex);
    LocalBufferIndex += GetNumberDigits(ArgumentsNumber);

    LocalBufferIndex += 2;

    int LocalVariablesNumber = atoi(Arguments + LocalBufferIndex);
    LocalBufferIndex += GetNumberDigits(LocalVariablesNumber);

    fprintf(elf, 
                "\tmov rcx, rsp\n"
                "\tpop rbx\n"
                "\tsub rsp, 8 * %d\n"
                "\tpush rbp\n"
                "\tmov r1, rsp\n"
                "\tadd r1, 8 * %d\n"
                "\tmov rbp, r1\n"
                , LocalVariablesNumber - ArgumentsNumber, LocalVariablesNumber);
    
    LocalBufferIndex += 3;

    strncpy(CurrentFunction->Label, FunctionLabel, strlen(FunctionLabel));
    CurrentFunction->NumberOfArguments = (size_t)ArgumentsNumber;
    CurrentFunction->NumberOfLocalVariables = (size_t)LocalVariablesNumber;

    fprintf(stderr, "\n%s\n", Arguments + LocalBufferIndex);
    return LocalBufferIndex;
}

static size_t TranslateFunctionCall(char* Arguments, FILE* elf, TranslatorFunction_t* CurrentFunction)
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
        if(*(Arguments + LocalBufferIndex + i) == ',')
        {
            FunctionLabel[i++] = '\0';
            LocalBufferIndex += i;
            break;
        }
        FunctionLabel[i] = *(Arguments + LocalBufferIndex + i);
    }

    LocalBufferIndex += 2;

    fprintf(elf,
                "\tpush rip\n"
                "\tcall %s\n"
                "\tpush rax\n", FunctionLabel);

    return LocalBufferIndex;
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

static size_t TranslateAssignment(char* Arguments, FILE* elf, TranslatorFunction_t* CurrentFunction)
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
            while(isdigit(Arguments[LocalBufferIndex]))
            {
                LocalBufferIndex++;
            }

            int LocalVariableIndex = atoi(Arguments);
            LocalBufferIndex += GetNumberDigits(LocalVariableIndex);

            fprintf(elf, 
                        "\tpush [rbp + 8 * %d]\n", LocalVariableIndex);
        }
        else if(isdigit(Arguments[LocalBufferIndex]))
        {
            double NumericalValue = atof(Arguments);

            LocalBufferIndex += GetFloatNumberDigits(Arguments);

            char BinaryFloatRepresentation[sizeof(double) * 8 + 2] = {};

            Get64ByteVariableBinaryString(&NumericalValue, BinaryFloatRepresentation);

            fprintf(elf, "\tpush %s\n", BinaryFloatRepresentation);
        }
        else
        {
            assert(0);
        }
    }
    else if(strcmp(OperandType, "var") == 0)
    {
        int LocalVarIndex = atoi(Arguments);
        LocalBufferIndex += (size_t)GetNumberDigits(LocalBufferIndex);

        while(Arguments[LocalBufferIndex] != ')')
        {
            LocalBufferIndex++;
        }

        LocalBufferIndex += 2;

        fprintf(elf, 
                    "\tpop r2\n"
                    "\tmov [rbp + 8 * %d], r2\n", LocalVarIndex);
    }
    else
    {
        assert(0);
    }

    fprintf(stderr, "\n%s\n", Arguments + LocalBufferIndex);

    return LocalBufferIndex;
}

static int Get64ByteVariableBinaryString(void* Variable, char* String)
{
    assert(Variable);
    size_t Mask = 1;
    for(size_t i = 0; i < 64; i++)
    {
        
        String[i] = '0' + (char)((*(size_t*)(Variable) & Mask) << i);
        Mask *= 2;;
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

    size_t KeyWordSize = KeyWordEnd - IRStatement;

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