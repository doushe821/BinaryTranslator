#include "FileBufferizer/FileBufferizer.h"

#include "Tokenizer.h"
#include "Syntaxer.h"
#include "IntermediateRepresentationGenerator.h"

#include <assert.h>

int main()
{
    FILE* fp = fopen("TokenizerTest.txt", "r+b");
    assert(fp);

    size_t TokenTableSize = GetFileSize(fp);
    char* Buffer = FileToString(fp);

    TokenTable_t* TokenTable = Tokenize(Buffer, TokenTableSize);

    Tree_t Tree =
    {
        NewNode,
        CloneTree,
        NodeDump,
        NULL,
    };


    List_t* VariableTablesList = NULL;
    ListInit(&VariableTablesList, InitalArraySize, sizeof(void*), 0);
    if(VariableTablesList == NULL)
    {
        free(Buffer);
        free(TokenTable->TokenArray);
        assert(0);
        return -1;
    }

   // VariableTable_t VariableTable = {};
   // VariableTable.VariablesArray = (Variable_t*)calloc(InitalArraySize, sizeof(Variable_t));
   // VariableTable.Capacity = InitalArraySize;
   // if(VariableTable.VariablesArray == NULL)
   // {
   //     free(Buffer);
   //     free(TokenTable->TokenArray);
   //     assert(0);
   //     return -1;
   // }

    FunctionTable_t FunctionTable = {};
    FunctionTable.FunctionsArray = (Function_t*)calloc(InitalArraySize, sizeof(Function_t));
    FunctionTable.Capacity = InitalArraySize;
    if(FunctionTable.FunctionsArray == NULL)
    {
        free(Buffer);
        free(TokenTable->TokenArray);
        ListDstr(VariableTablesList);
        assert(0);
        return -1;
    }

    size_t TokenIndex = 0;
    GetProgram(&Tree, TokenTable, &TokenIndex, VariableTablesList, &FunctionTable);

    #ifndef NDEBUG
    FILE* TreeDumpDebug = fopen("TreeDump.dot", "w+b");
    assert(TreeDumpDebug);
    PrintTreeGraphViz(TreeDumpDebug, Tree, "TreeDump.pdf");
    fclose(TreeDumpDebug);
    #endif

    FILE* IntermediateRepresentationOutputFile = fopen("ir.pyam", "w+b");
    assert(IntermediateRepresentationOutputFile);
    GenerateIntermediateRepresentation(&Tree, &FunctionTable, IntermediateRepresentationOutputFile);

    fclose(IntermediateRepresentationOutputFile);

    BurnTree(&Tree);
    for(size_t i = 1; i < VariableTablesList->free; i++)
    {
        VariableTable_t* VarTable = NULL;
        memcpy(&VarTable, ListGetNodeValueInd(VariableTablesList, i), sizeof(void*));
        free(VarTable->VariablesArray);
        free(VarTable);
    }
    ListDstr(VariableTablesList);
    free(FunctionTable.FunctionsArray);
    free(Buffer);
    free(TokenTable->TokenArray);
    free(TokenTable);
    return 0;
    
}