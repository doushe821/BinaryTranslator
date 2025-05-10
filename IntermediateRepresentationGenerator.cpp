#include "PYAM_IR/libpyam_ir.h"
#include "CompilerTables.h"
#include "Syntaxer.h"
#include "Syntax.h"

#include <assert.h>

#include <stdio.h>
#include <stdlib.h>

int GenerateIntermediateRepresentation(Tree_t* Tree, FunctionTable_t* FunctionTable, FILE* OutputFile)
{
    assert(Tree);
    assert(FunctionTable);
    assert(OutputFile);

    int MainIndex = -1;
    for(size_t i = 0; i < FunctionTable->Free; i++)
    {
        if(strncmp(KeyWordsArray[MAIN_INDEX].Name, FunctionTable->FunctionsArray[i].Name, strlen(KeyWordsArray[MAIN_INDEX].Name)) == 0)
        {
            MainIndex = i;
            break;
        }
    }

    if(MainIndex == -1)
    {
        fprintf(stderr, "If There Is No God, There Is No Free Will..\nPerish Now, lowly fiend!\n");
        return -1; // TODO error codes
    }                   

    size_t NumberOfFunctions = 0;
    memcpy(&NumberOfFunctions, GetNodeData(Tree->root, DATA_FIELD_CODE, 0), sizeof(NumberOfFunctions));

    size_t DescendantMainIndex = 0;
    for(size_t i = 0; i < NumberOfFunctions; i++)
    {
        void* ProgramDescendant = GetNodeData(Tree->root, DESCENDANTS_FIELD_CODE, i);
        size_t NodeType = 0;
        memcpy(&NodeType, GetNodeData(ProgramDescendant, TYPE_FIELD_CODE, 0), sizeof(NodeType));

        if(NodeType == FUNCTION_BODY_NODE)
        {
            size_t Index = 0;
            memcpy(&Index, GetNodeData(ProgramDescendant, DATA_FIELD_CODE, 0), sizeof(NodeType));

            if(Index == (size_t)MainIndex)
            {
                DescendantMainIndex = i;
                break;
            }
        }
    }

    GenerateFunctionBodyRepresentation(GetNodeData(Tree->root, DESCENDANTS_FIELD_CODE, DescendantMainIndex), FunctionTable, MainIndex, OutputFile);

    for(size_t i = 0; i < NumberOfFunctions; i++)
    {
        if(i == DescendantMainIndex)
        {
            continue;
        }

        void* FunctionNode = GetNodeData(Tree->root, DESCENDANTS_FIELD_CODE, i);
        size_t TableIndex = 0;
        memcpy(&TableIndex, GetNodeData(FunctionNode, DATA_FIELD_CODE, 0), sizeof(TableIndex));
        GenerateFunctionBodyRepresentation(FunctionNode, FunctionTable, TableIndex, OutputFile);
    }

    return 0;
}

static int GenerateFunctionBodyRepresentation(void* TreeNode, FunctionTable_t* FunctionTable, int FunctionIndex, FILE* out)
{
    assert(TreeNode);
    assert(FunctionTable);

    size_t NodeType = 0;
    memcpy(&NodeType, GetNodeData(TreeNode, DATA_FIELD_CODE, 0), sizeof(NodeType));
    if(NodeType != FUNCTION_BODY_NODE)
    {
        assert(0);
    }  

    void* FunctionArguments = NULL;
    memcpy(&FunctionArguments, GetNodeData(TreeNode, DESCENDANTS_FIELD_CODE, 1), sizeof(void*));
    assert(FunctionArguments);

    void* FunctionBodyScope = NULL;
    memcpy(&FunctionBodyScope, GetNodeData(TreeNode, DESCENDANTS_FIELD_CODE, 0), sizeof(void*));
    assert(FunctionBodyScope);

    size_t NumberOfArguments = 0;
    memcpy(&NumberOfArguments, GetNodeData(FunctionArguments, DATA_FIELD_CODE, 0), sizeof(NumberOfArguments));

    fprintf(stderr, "FunctionBody(func_%d_%zu, %zu)\t# %s", FunctionIndex, NumberOfArguments, NumberOfArguments, FunctionTable->FunctionsArray[FunctionIndex].Name);

    size_t StatementCounter = 0;
    memcpy(&StatementCounter, GetNodeData(FunctionBodyScope, DATA_FIELD_CODE, 0), sizeof(StatementCounter));

    size_t TemporalVariableIndex = 0;
    for(size_t i = 0; i < StatementCounter; i++)
    {
        void* StatementNode = NULL;
        memcpy(&StatementNode, GetNodeData(FunctionBodyScope, DESCENDANTS_FIELD_CODE, i), sizeof(void*));

        size_t StatementNodeType = 0;
        memcpy(&StatementNodeType, GetNodeData(StatementNode, TYPE_FIELD_CODE, 0), sizeof(StatementNodeType));

        switch(StatementNodeType)
        {
            case STATEMENT_OPERAND_NODE:
            {
                GenerateAssignmentRepresentation(StatementNode, &TemporalVariableIndex, out);
            }
            case CONDITION_NODE:
            {
                GenerateConditionRepresentation(StatementNode, out);
            }
            case RETURN_NODE:
            {
                GenerateReturnRepresentation(StatementNode, out);
            }
            default:
            {
                fprintf(stderr, "Hail to the Edge Lord!\n");
                assert(0);
            }
        }
    }
}

static int GenerateAssignmentRepresentation(void* TreeNode, size_t* TemporalVariableIndex, FILE* out)
{
    assert(TreeNode);
    assert(TemporalVariableIndex);
    assert(out);

    void* LeftVariable = NULL;
    memcpy(&LeftVariable, GetNodeData(TreeNode, DESCENDANTS_FIELD_CODE, 0), sizeof(void*));
    assert(LeftVariable);
    int VariableTableIndex = 0;
    memcpy(&VariableTableIndex, GetNodeData(LeftVariable, DATA_FIELD_CODE, 0), sizeof(VariableTableIndex));
    char VariableName[VARIABLE_NAME_MAX] = {};
    memcpy(VariableName, GetNodeData(LeftVariable, DATASIZE_FIELD_CODE, 0) + sizeof(VariableTableIndex), VARIABLE_NAME_MAX);


    void* Expression = NULL;
    memcpy(&Expression, GetNodeData(TreeNode, DESCENDANTS_FIELD_CODE, 1), sizeof(void*));
    assert(Expression);

    GenerateExpressionRepresentation(Expression, TemporalVariableIndex, out);

    fprintf(out, "assignment(var%d, %zu_tmp)\t# %s\n", VariableTableIndex, *TemporalVariableIndex, VariableName);
    return 0;
}

static int GenerateExpressionRepresentation(void* TreeNode, size_t* TemporalVariableIndex, FILE* out)
{
    assert(TreeNode);
    assert(TemporalVariableIndex);
    assert(out);

    size_t CurrentNodeType = 0;
    memcpy(&CurrentNodeType, GetNodeData(TreeNode, TYPE_FIELD_CODE, 0), sizeof(CurrentNodeType));

    switch(CurrentNodeType)
    {
        case INTEGER_VALUE_NODE:
        {
            int Data = 0;
            memcpy(&Data, GetNodeData(TreeNode, DATA_FIELD_CODE, 0), sizeof(Data));
            fprintf(out, "assignment(%zu_tmp, %d)\n", *TemporalVariableIndex, Data);
            break;
        }
        case FLOAT_VALUE_NODE:
        {
            double Data = 0;
            memcpy(&Data, GetNodeData(TreeNode, DATA_FIELD_CODE, 0), sizeof(Data));
            fprintf(out, "assignment(%zu_tmp, %ld)\n", *TemporalVariableIndex, Data);
            break;
        }
        case OPERATION_NODE:
        {
            GenerateOperationRepresentation(TreeNode, TemporalVariableIndex, out);

            break;
        }
    }
}

static int GenerateIntegerValueRepresentation(void* TreeNode, size_t* TemporalVariableIndex, FILE* out)
{
    
}

static int GenerateOperationRepresentation(void* TreeNode, size_t* TemporalVariableIndex, FILE* out)
{
    assert(TreeNode);
    assert(TemporalVariableIndex);
    assert(out);

    int OperationCode = 0;
    memcpy(&OperationCode, GetNodeData(TreeNode, DATA_FIELD_CODE, 0), sizeof(OperationCode));

    void* LeftDescendant = NULL;
    memcpy(&LeftDescendant, GetNodeData(TreeNode, DESCENDANTS_FIELD_CODE, 0), sizeof(void*));
    assert(LeftDescendant);

    size_t LeftDescendantType = 0;
    memcpy(&LeftDescendantType, GetNodeData(LeftDescendant, TYPE_FIELD_CODE, 0), sizeof(LeftDescendantType));

    switch(LeftDescendantType)
    {
        case RIGHT_VARIABLE_NODE:
        {
            int VariableIndex = 0;
            memcpy(&VariableIndex, GetNodeData(TreeNode, DATA_FIELD_CODE, 0), sizeof(VariableIndex));
            char VariableName[VARIABLE_NAME_MAX] = {};
            memcpy(&VariableName, GetNodeData(TreeNode, DATA_FIELD_CODE, 0) + sizeof(VariableIndex), VARIABLE_NAME_MAX);
            fprintf(out, "assignment(%zu_tmp, var_%d)\t # %s\n", *TemporalVariableIndex, VariableIndex, VariableName);
            break;
        }
        case OPERATION_NODE:
        {
            GenerateOperationRepresentation(TreeNode, TemporalVariableIndex, out);
            break;
        }
        case 
    }

    void* RightDescendant = NULL;
    memcpy(&RightDescendant, GetNodeData(TreeNode, DESCENDANTS_FIELD_CODE, 1), sizeof(void*));
    assert(RightDescendant);

    fprintf(out, "operation(%d, %zu_tmp, )");
}