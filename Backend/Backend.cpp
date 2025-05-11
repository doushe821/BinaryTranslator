#include "Backend.h"
#include "../FileBufferizer/FileBufferizer.h"

#include <ctype.h>
#include <assert.h>

static const size_t VARIABLE_NAME_MAX = 512;
static const size_t KEY_WORD_NAME_MAX = 32;
static const size_t KEY_WORD_NUMBER = 16;

struct IRStatement_t
{
    int OpCode;
    int IntVal;
    double FloatVal;
    int Op1Type;
    int Op2Type;
    int Op3Type;
    char Op1Name[VARIABLE_NAME_MAX];
    char Op2Name[VARIABLE_NAME_MAX];
    char Op3Name[VARIABLE_NAME_MAX];
};

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
            case IR_ASSIGNMENT_INDEX:
            {
                i += ParseAssignment(Buffer + i);
                break;
            }
        }
    }
}


static size_t ParseAssignment(char* Arguments)
{
    assert(Arguments[0] == '(');

    size_t LocalBufferIndex = 0;
    LocalBufferIndex++;

    int Prefix = atoi(Arguments + LocalBufferIndex);
    switch(Prefix)
    {
        case IR_TMP_OPERAND_CODE:
        {
            assert(0 && "sad and gay\n");
        }
        case IR_VAR_OPERAND_CODE:
        {
            assert(0 && "sad and gay\n");
        }
        case IR_ARG_OPERAND_CODE:
        {
            assert(0 && "sad and gay\n");
        }
        default:
        {
            assert(0 && "sad and gay\n");
        }
    }
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