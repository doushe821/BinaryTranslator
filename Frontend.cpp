#include "FileBufferizer/FileBufferizer.h"

#include "Tokenizer.h"
#include "Syntaxer.h"

#include <assert.h>

int main()
{
    FILE* fp = fopen("TokenizerTest.txt", "r+b");
    assert(fp);

    size_t TokenTableSize = GetFileSize(fp);
    char* Buffer = FileToString(fp);

    TokenTable_t* TokenTable = Tokenize(Buffer, TokenTableSize);

    fprintf(stderr, "%p\n", TokenTable);
    Tree_t Tree =
    {
        NewNode,
        CloneTree,
        NodeDump,
        NULL,
    };

    VariableTable_t VariableTable = {};
    VariableTable.VariablesArray = (Variable_t*)calloc(InitalArraySize, sizeof(Variable_t));
    VariableTable.Capacity = InitalArraySize;
    if(VariableTable.VariablesArray == NULL)
    {
        free(Buffer);
        free(TokenTable->TokenArray);
        assert(0);
        return -1;
    }

    FunctionTable_t FunctionTable = {};
    FunctionTable.FunctionsArray = (Function_t*)calloc(InitalArraySize, sizeof(Function_t));
    FunctionTable.Capacity = InitalArraySize;
    if(VariableTable.VariablesArray == NULL)
    {
        free(Buffer);
        free(TokenTable->TokenArray);
        free(VariableTable.VariablesArray);
        assert(0);
        return -1;
    }

    size_t TokenIndex = 0;
    GetProgram(&Tree, TokenTable, &TokenIndex, &VariableTable, &FunctionTable);

    #ifndef NDEBUG
    FILE* TreeDumpDebug = fopen("TreeDump.dot", "w+b");
    assert(TreeDumpDebug);
    PrintTreeGraphViz(TreeDumpDebug, Tree, "TreeDump.pdf");
    fclose(TreeDumpDebug);
    #endif

    BurnTree(&Tree);
    free(VariableTable.VariablesArray);
    free(FunctionTable.FunctionsArray);
    free(Buffer);
    free(TokenTable->TokenArray);
    free(TokenTable);
    return 0;
    
}