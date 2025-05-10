#include "IntermediateRepresentationGenerator.h"

static size_t GenerateReturnRepresentation(void* TreeNode, size_t* TemporalVariableIndex, size_t* LocalLabelIndex, FILE* OutputFile);
static size_t GenerateFloatValueRepresentation(void* TreeNode, size_t* TemporalVariableIndex, size_t* LocalLabelIndex, FILE* OutputFile);
static size_t GenerateIntegerValueRepresentation(void* TreeNode, size_t* TemporalVariableIndex, size_t* LocalLabelIndex, FILE* OutputFile);
static size_t GenerateExpressionRepresentation(void* TreeNode, size_t* TemporalVariableIndex, size_t* LocalLabelIndex, FILE* OutputFile);
static size_t GenerateAssignmentRepresentation(void* TreeNode, size_t* TemporalVariableIndex, size_t* LocalLabelIndex, FILE* OutputFile);
static size_t GenerateFunctionBodyRepresentation(void* TreeNode, FunctionTable_t* FunctionTable, int FunctionIndex, size_t* LocalLabelIndex, FILE* OutputFile);
static size_t GenerateOperationRepresentation(void* TreeNode, size_t* TemporalVariableIndex, size_t* LocalLabelIndex, FILE* OutputFile);
static size_t GenerateScopeRepresentation(void* TreeNode, size_t* TemporalVariableIndex, size_t* LocalLabelIndex, FILE* OutputFile);
static size_t GenerateConditionRepresentation(void* TreeNode, size_t* TemporalVariableIndex, size_t* LocalLabelIndex, FILE* OutputFile);

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
            MainIndex = (int)i;
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
        void* ProgramDescendant = NULL;
        memcpy(&ProgramDescendant, GetNodeData(Tree->root, DESCENDANTS_FIELD_CODE, i), sizeof(void*));
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

    void* MainBody = NULL;
    memcpy(&MainBody, GetNodeData(Tree->root, DESCENDANTS_FIELD_CODE, DescendantMainIndex), sizeof(void*));

    size_t LocalLabelIndex = 0;

    GenerateFunctionBodyRepresentation(MainBody, FunctionTable, MainIndex, &LocalLabelIndex, OutputFile);

    for(size_t i = 0; i < NumberOfFunctions; i++)
    {

        if(i == DescendantMainIndex)
        {
            continue;
        }

        void* FunctionNode = NULL;
        memcpy(&FunctionNode, GetNodeData(Tree->root, DESCENDANTS_FIELD_CODE, i), sizeof(void*));
        size_t TableIndex = 0;
        memcpy(&TableIndex, GetNodeData(FunctionNode, DATA_FIELD_CODE, 0), sizeof(TableIndex));
        GenerateFunctionBodyRepresentation(FunctionNode, FunctionTable, (int)TableIndex, &LocalLabelIndex, OutputFile);
    }

    return 0;
}

static size_t GenerateFunctionBodyRepresentation(void* TreeNode, FunctionTable_t* FunctionTable, int FunctionIndex, size_t* LocalLabelIndex, FILE* OutputFile)
{
    assert(TreeNode);
    assert(FunctionTable);

    size_t NodeType = 0;
    memcpy(&NodeType, GetNodeData(TreeNode, TYPE_FIELD_CODE, 0), sizeof(NodeType));
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

    fprintf(OutputFile, "FunctionBody(func_%d_%zu, %zu)\t# %s\n", FunctionIndex, NumberOfArguments, NumberOfArguments, FunctionTable->FunctionsArray[FunctionIndex].Name);

    size_t TemporalVariableIndex = 0;

    GenerateScopeRepresentation(FunctionBodyScope, &TemporalVariableIndex, LocalLabelIndex, OutputFile);

    return 0;
}

static size_t GenerateScopeRepresentation(void* TreeNode, size_t* TemporalVariableIndex, size_t* LocalLabelIndex, FILE* OutputFile)
{
    size_t StatementCounter = 0;
    memcpy(&StatementCounter, GetNodeData(TreeNode, DATA_FIELD_CODE, 0), sizeof(StatementCounter));

    for(size_t i = 0; i < StatementCounter; i++)
    {
        void* StatementNode = NULL;
        memcpy(&StatementNode, GetNodeData(TreeNode, DESCENDANTS_FIELD_CODE, i), sizeof(void*));

        size_t StatementNodeType = 0;
        memcpy(&StatementNodeType, GetNodeData(StatementNode, TYPE_FIELD_CODE, 0), sizeof(StatementNodeType));

        switch(StatementNodeType)
        {
            case STATEMENT_OPERAND_NODE:
            {
                GenerateAssignmentRepresentation(StatementNode, TemporalVariableIndex, LocalLabelIndex, OutputFile);
                break;
            }
            case RETURN_NODE:
            {
                GenerateReturnRepresentation(StatementNode, TemporalVariableIndex, LocalLabelIndex, OutputFile);
                break;
            }
            case CONDITION_NODE:
            {
                size_t OldTempIndex = *TemporalVariableIndex;
                GenerateConditionRepresentation(StatementNode, &OldTempIndex, LocalLabelIndex, OutputFile);
                break;
            }
            default:
            {
                fprintf(stderr, "Hail to the Edge Lord!\n");
                assert(0);
            }
        }
    }

    return 0;
}

static size_t GenerateConditionRepresentation(void* TreeNode, size_t* TemporalVariableIndex, size_t* LocalLabelIndex, FILE* OutputFile)
{
    assert(TreeNode);
    assert(TemporalVariableIndex);
    assert (OutputFile);

    void* Condition = NULL;
    memcpy(&Condition, GetNodeData(TreeNode, DESCENDANTS_FIELD_CODE, 1), sizeof(void*));
    assert(Condition);

    int ConditionIndex = 0;
    memcpy(&ConditionIndex, GetNodeData(TreeNode, DATA_FIELD_CODE, 0), sizeof(ConditionIndex));

    switch(ConditionIndex)
    {
        case IF_INDEX:
        {
            GenerateExpressionRepresentation(Condition, TemporalVariableIndex, LocalLabelIndex, OutputFile);
            fprintf(OutputFile, "conditional_jump(%zu_tmp, label_%zu)\n", *TemporalVariableIndex - 1, *LocalLabelIndex);
            size_t IfEnd = *LocalLabelIndex;
            (*LocalLabelIndex)++;

            void* IfScope = NULL;
            memcpy(&IfScope, GetNodeData(TreeNode, DESCENDANTS_FIELD_CODE, 0), sizeof(void*));
            assert(IfScope);

            GenerateScopeRepresentation(IfScope, TemporalVariableIndex, LocalLabelIndex, OutputFile);
            fprintf(OutputFile, "label(%zu)\t# %s\n", IfEnd, "If's end");

            break;
        }
        case WHILE_INDEX:
        {
            size_t WhileStart = *LocalLabelIndex;
            (*LocalLabelIndex)++;
            fprintf(OutputFile, "label(%zu)\t# %s\n", WhileStart, "While's start");
            GenerateExpressionRepresentation(Condition, TemporalVariableIndex, LocalLabelIndex, OutputFile);

            size_t WhileEnd = *LocalLabelIndex;
            (*LocalLabelIndex)++;

            fprintf(OutputFile, "conditional_jump(%zu_tmp, label_%zu)\n", *TemporalVariableIndex - 1, WhileEnd);

            void* WhileScope = NULL;
            memcpy(&WhileScope, GetNodeData(TreeNode, DESCENDANTS_FIELD_CODE, 0), sizeof(void*));
            assert(WhileScope);

            GenerateScopeRepresentation(WhileScope, TemporalVariableIndex, LocalLabelIndex, OutputFile);
            fprintf(OutputFile, "jump(label_%zu)\n", WhileStart);
            fprintf(OutputFile, "label(%zu)\t# %s\n", WhileEnd, "While's end");
            break;
        }
        default:
        {
            assert(0 && "I cum blood\n");
            break;
        }
    }

    return 0;
}

static size_t GenerateAssignmentRepresentation(void* TreeNode, size_t* TemporalVariableIndex, size_t* LocalLabelIndex, FILE* OutputFile)
{
    assert(TreeNode);
    assert(TemporalVariableIndex);
    assert (OutputFile);

    void* LeftVariable = NULL;
    memcpy(&LeftVariable, GetNodeData(TreeNode, DESCENDANTS_FIELD_CODE, 0), sizeof(void*));
    assert(LeftVariable);
    int VariableTableIndex = 0;
    memcpy(&VariableTableIndex, GetNodeData(LeftVariable, DATA_FIELD_CODE, 0), sizeof(VariableTableIndex));
    char VariableName[VARIABLE_NAME_MAX] = {};
    memcpy(VariableName, (char*)GetNodeData(LeftVariable, DATASIZE_FIELD_CODE, 0) + sizeof(VariableTableIndex), VARIABLE_NAME_MAX);


    void* Expression = NULL;
    memcpy(&Expression, GetNodeData(TreeNode, DESCENDANTS_FIELD_CODE, 1), sizeof(void*));
    assert(Expression);

    GenerateExpressionRepresentation(Expression, TemporalVariableIndex, LocalLabelIndex, OutputFile);

    fprintf(OutputFile, "assignment(var%d, %zu_tmp)\t# %s\n", VariableTableIndex, *TemporalVariableIndex - 1, VariableName);
    //(*TemporalVariableIndex)++;
    return 0;
}

static size_t GenerateExpressionRepresentation(void* TreeNode, size_t* TemporalVariableIndex, size_t* LocalLabelIndex, FILE* OutputFile)
{
    assert(TreeNode);
    assert(TemporalVariableIndex);
    assert (OutputFile);

    size_t CurrentNodeType = 0;
    memcpy(&CurrentNodeType, GetNodeData(TreeNode, TYPE_FIELD_CODE, 0), sizeof(CurrentNodeType));

    switch(CurrentNodeType)
    {
        case INTEGER_VALUE_NODE:
        {
            GenerateIntegerValueRepresentation(TreeNode, TemporalVariableIndex, LocalLabelIndex, OutputFile);
            break;
        }
        case FLOAT_VALUE_NODE:
        {
            GenerateFloatValueRepresentation(TreeNode, TemporalVariableIndex, LocalLabelIndex, OutputFile);
            break;
        }
        case OPERATION_NODE:
        {
            GenerateOperationRepresentation(TreeNode, TemporalVariableIndex, LocalLabelIndex, OutputFile);

            break;
        }
        case RIGHT_VARIABLE_NODE:
        {
            int VariableIndex = 0;
            memcpy(&VariableIndex, GetNodeData(TreeNode, DATA_FIELD_CODE, 0), sizeof(VariableIndex));
            fprintf(OutputFile, "assignment(%zu_tmp, var_%d)\n", *TemporalVariableIndex, VariableIndex);
            (*TemporalVariableIndex)++;
            break;  
        }
        default:
        {
            assert(0);
        }
    }
    return 0;
}

static size_t GenerateIntegerValueRepresentation(void* TreeNode, size_t* TemporalVariableIndex, size_t* LocalLabelIndex, FILE* OutputFile)
{
    int Data = 0;
    memcpy(&Data, GetNodeData(TreeNode, DATA_FIELD_CODE, 0), sizeof(Data));
    fprintf(OutputFile, "assignment(%zu_tmp, %d)\n", *TemporalVariableIndex, Data);
    (*TemporalVariableIndex)++;
    return *TemporalVariableIndex - 1;  
}

static size_t GenerateFloatValueRepresentation(void* TreeNode, size_t* TemporalVariableIndex, size_t* LocalLabelIndex, FILE* OutputFile)
{
    double Data = 0;
    memcpy(&Data, GetNodeData(TreeNode, DATA_FIELD_CODE, 0), sizeof(Data));
    fprintf(OutputFile, "assignment(%zu_tmp, %lg)\n", *TemporalVariableIndex, Data);  
    (*TemporalVariableIndex)++;
    return *TemporalVariableIndex - 1;
}

static size_t GenerateReturnRepresentation(void* TreeNode, size_t* TemporalVariableIndex, size_t* LocalLabelIndex, FILE* OutputFile)
{
    assert(TreeNode);
    assert(TemporalVariableIndex);
    assert (OutputFile);

    void* ReturnValue = NULL;
    memcpy(&ReturnValue, GetNodeData(TreeNode, DESCENDANTS_FIELD_CODE, 0), sizeof(void*));
    assert(ReturnValue);

    GenerateExpressionRepresentation(ReturnValue, TemporalVariableIndex, LocalLabelIndex, OutputFile);

    fprintf(OutputFile, "return(%zu_tmp)\n", *TemporalVariableIndex - 1);

    return 0;
}


static size_t GenerateOperationRepresentation(void* TreeNode, size_t* TemporalVariableIndex, size_t* LocalLabelIndex, FILE* OutputFile)
{
    assert(TreeNode);
    assert(TemporalVariableIndex);
    assert (OutputFile);

    int OperationCode = 0;
    memcpy(&OperationCode, GetNodeData(TreeNode, DATA_FIELD_CODE, 0), sizeof(OperationCode));

    size_t Fertility = 0;
    memcpy(&Fertility, GetNodeData(TreeNode, DEGREE_FIELD_CODE, 0), sizeof(Fertility));

    size_t ResultTmp[2] = {};

    for(size_t i = 0; i < Fertility; i++)
    {
        void* Descendant = NULL;
        memcpy(&Descendant, GetNodeData(TreeNode, DESCENDANTS_FIELD_CODE, i), sizeof(void*));
        assert(Descendant);
    
        size_t DescendantType = 0;
        memcpy(&DescendantType, GetNodeData(Descendant, TYPE_FIELD_CODE, 0), sizeof(DescendantType));
    
        switch(DescendantType)
        {
            case RIGHT_VARIABLE_NODE:
            {
                int VariableIndex = 0;
                memcpy(&VariableIndex, GetNodeData(Descendant, DATA_FIELD_CODE, 0), sizeof(VariableIndex));
                fprintf(OutputFile, "assignment(%zu_tmp, var_%d)\n", *TemporalVariableIndex, VariableIndex);
                ResultTmp[i] = *TemporalVariableIndex;
                (*TemporalVariableIndex)++;
                break;
            }
            case OPERATION_NODE:
            {
                ResultTmp[i] = GenerateOperationRepresentation(Descendant, TemporalVariableIndex, LocalLabelIndex, OutputFile);
                break;
            }
            case INTEGER_VALUE_NODE:
            {
                ResultTmp[i] = GenerateIntegerValueRepresentation(Descendant, TemporalVariableIndex, LocalLabelIndex, OutputFile);
                break;
            }
            case FLOAT_VALUE_NODE:
            {
                ResultTmp[i] = GenerateFloatValueRepresentation(Descendant, TemporalVariableIndex, LocalLabelIndex, OutputFile);
                break;
            }
            default:
            {
                assert(0);
                break;
            }
        }
    }

    fprintf(OutputFile, "operation(%d, %zu_tmp, %zu_tmp, %zu_tmp)\n", OperationCode, ResultTmp[0], ResultTmp[1], *TemporalVariableIndex);
    (*TemporalVariableIndex)++;

    return *TemporalVariableIndex - 1;
}