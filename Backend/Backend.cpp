#include "Backend.h"
#include "../FileBufferizer/FileBufferizer.h"

#include <ctype.h>
#include <assert.h>

static const size_t VARIABLE_NAME_MAX = 512;
static const size_t KEY_WORD_NAME_MAX = 32;
static const size_t KEY_WORD_NUMBER = 16;

static const size_t TRANSLATOR_FUNCTION_LABEL_NAME_MAX = 32;

int main()
{
    FILE* IRFile = fopen("../ir.pyam", "r+b");
    if(IRFile == NULL)
    {
        return -1;
    }

    size_t IRFileSize = GetFileSize(IRFile);

    char* Buffer = (char*)calloc(IRFileSize, sizeof(char));
    
    if(Buffer == NULL)
    {
        fclose(IRFile);
        return -1;
    }

    FILE* outputELF = fopen("gnome.asm", "w+b");
    assert(outputELF);

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
                i += TranslateFunctionBody(Buffer + i, outputELF);
                break;
            }
            case IR_FUNCTION_CALL_INDEX:
            {
                i += TranslateFunctionCall(Buffer + i, outputELF);
                break;
            }
            case IR_ASSIGNMENT_INDEX:
            {
                i += TranslateAssignment(Buffer + i, outputELF);
                break;
            }
            case IR_RETURN_INDEX:
            {
                i += TranslateReturn(Buffer + i, outputELF);
                break;
            }
            case IR_LABEL_INDEX:
            {
                i += TranslateLabel(Buffer + i, outputELF);
                break;
            }
            case IR_CONDITIONAL_JUMP_INDEX:
            {
                i += TranslateConditionalJump(Buffer + i, outputELF);
                break;
            }
            case IR_OPERATION_INDEX:
            {

            }
            default:
            {
                break;
            }
        }
    }
}

static size_t TranslateConditionalJump(char* Arguments, FILE* elf)
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

static size_t TranslateLabel(char* Arguments, FILE* elf)
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

static size_t TranslateReturn(char* Arguments, FILE* elf)
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
    
    while(Arguments[LocalBufferIndex] != '\n');
    {
        LocalBufferIndex++;
    }
    LocalBufferIndex++;
    return LocalBufferIndex;
}

static size_t TranslateFunctionBody(char* Arguments, FILE* elf)
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
    
    LocalBufferIndex += 2;

    return LocalBufferIndex;
}

static size_t TranslateFunctionCall(char* Arguments, FILE* elf)
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

static size_t TranslateAssignment(char* Arguments, FILE* elf)
{
    assert(Arguments);
    assert(elf);
}

static int GetKeyWordCode(const char* IRStatement, size_t* BufferIndex)
{
    assert(IRStatement);

    const char* KeyWordEnd = strchr(IRStatement, '(');

    size_t KeyWordSize = KeyWordEnd - IRStatement;

    const char KeyWord[KEY_WORD_NAME_MAX] = {};

    memcpy((void*)KeyWord, IRStatement, KeyWordSize);
    
    int KeyWordIndex = -1;
    for(int i = 0; i < kIR_KEY_WORD_NUMBER; i++)
    {
        if(strncmp(KeyWord, kIR_KEY_WORD_ARRAY[i], kIR_KEY_WORD_NAME_MAX))
        {
            KeyWordIndex = i;
            break;
        }
    }

    (*BufferIndex) += KeyWordSize;
    return KeyWordIndex;
}