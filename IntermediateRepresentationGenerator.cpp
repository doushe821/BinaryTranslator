#include "IntermediateRepresentationGenerator.h"

// FIXME

#define CASE_ENUM_TO_STRING_(op_type_) case op_type_: return #op_type_
const char* ir_op_type_to_str(const enum IrOpType type)
{
    switch(type)
    {
        CASE_ENUM_TO_STRING_(IR_OP_TYPE_SUM);
        CASE_ENUM_TO_STRING_(IR_OP_TYPE_SUB);
        CASE_ENUM_TO_STRING_(IR_OP_TYPE_MUL);
        CASE_ENUM_TO_STRING_(IR_OP_TYPE_DIV);
        CASE_ENUM_TO_STRING_(IR_OP_TYPE_POW);
        CASE_ENUM_TO_STRING_(IR_OP_TYPE_EQ);
        CASE_ENUM_TO_STRING_(IR_OP_TYPE_NEQ);
        CASE_ENUM_TO_STRING_(IR_OP_TYPE_LESS);
        CASE_ENUM_TO_STRING_(IR_OP_TYPE_LESSEQ);
        CASE_ENUM_TO_STRING_(IR_OP_TYPE_GREAT);
        CASE_ENUM_TO_STRING_(IR_OP_TYPE_GREATEQ);
        CASE_ENUM_TO_STRING_(IR_OP_TYPE_INVALID_OPERATION);
        default:
            return "UNKNOWN_IR_OP_TYPE";
    }
    return "UNKNOWN_IR_OP_TYPE";
}
#undef CASE_ENUM_TO_STRING_

// FIXME

static size_t GenerateReturnRepresentation(void* TreeNode, size_t* TemporalVariableIndex, size_t* LocalLabelIndex, FILE* IR_file, __attribute((unused))FunctionTable_t* FunctionTable);
static size_t GenerateFloatValueRepresentation(void* TreeNode, size_t* TemporalVariableIndex, size_t* LocalLabelIndex, FILE* IR_file, __attribute((unused))FunctionTable_t* FunctionTable);
static size_t GenerateIntegerValueRepresentation(void* TreeNode, size_t* TemporalVariableIndex, size_t* LocalLabelIndex, FILE* IR_file, __attribute((unused))FunctionTable_t* FunctionTable);
static size_t GenerateExpressionRepresentation(void* TreeNode, size_t* TemporalVariableIndex, size_t* LocalLabelIndex, FILE* IR_file, __attribute((unused))FunctionTable_t* FunctionTable);
static size_t GenerateAssignmentRepresentation(void* TreeNode, size_t* TemporalVariableIndex, size_t* LocalLabelIndex, FILE* IR_file, __attribute((unused))FunctionTable_t* FunctionTable);
static size_t GenerateFunctionBodyRepresentation(void* TreeNode, FunctionTable_t* FunctionTable, int FunctionIndex, size_t* LocalLabelIndex, FILE* IR_file);
static size_t GenerateOperationRepresentation(void* TreeNode, size_t* TemporalVariableIndex, size_t* LocalLabelIndex, FILE* IR_file, __attribute((unused))FunctionTable_t* FunctionTable);
static size_t GenerateScopeRepresentation(void* TreeNode, size_t* TemporalVariableIndex, size_t* LocalLabelIndex, FILE* IR_file, __attribute((unused))FunctionTable_t* FunctionTable);
static size_t GenerateConditionRepresentation(void* TreeNode, size_t* TemporalVariableIndex, size_t* LocalLabelIndex, FILE* IR_file, __attribute((unused))FunctionTable_t* FunctionTable);
static size_t GenerateFunctionCallRepresentation(void* TreeNode, size_t* TemporalVariableIndex, size_t* LocalLabelIndex, FILE* IR_file, __attribute((unused))FunctionTable_t* FunctionTable);
static size_t GenerateMainBodyRepresentation(void* TreeNode, FunctionTable_t* FunctionTable, int FunctionIndex, size_t* LocalLabelIndex, FILE* IR_file);

static enum IrOpType SyntaxerOpCodesToPYAMConversion(int OpCode);

int GenerateIntermediateRepresentation(Tree_t* Tree, FunctionTable_t* FunctionTable, FILE* IR_file)
{
    assert(Tree);
    assert(FunctionTable);
    assert(IR_file);

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

    GenerateMainBodyRepresentation(MainBody, FunctionTable, MainIndex, &LocalLabelIndex, IR_file);

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
        GenerateFunctionBodyRepresentation(FunctionNode, FunctionTable, (int)TableIndex, &LocalLabelIndex, IR_file);
    }

    return 0;
}

static size_t GenerateMainBodyRepresentation(void* TreeNode, FunctionTable_t* FunctionTable, __attribute((unused))int FunctionIndex, size_t* LocalLabelIndex, FILE* IR_file)
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

    //fprintf(IR_file, "FunctionBody(func_%d_%zu, %zu)\t# %s\n", FunctionIndex, NumberOfArguments, NumberOfArguments, FunctionTable->FunctionsArray[FunctionIndex].Name);
    IR_MAIN_BODY_(NumberOfArguments);
    size_t TemporalVariableIndex = 0;

    for(size_t i = 0; i < NumberOfArguments; i++)
    {
        IR_TAKE_ARG_((long long int) TemporalVariableIndex, i, "");
        TemporalVariableIndex++;
    }

    GenerateScopeRepresentation(FunctionBodyScope, &TemporalVariableIndex, LocalLabelIndex, IR_file, FunctionTable);

    return 0;
}

static size_t GenerateFunctionBodyRepresentation(void* TreeNode, FunctionTable_t* FunctionTable, int FunctionIndex, size_t* LocalLabelIndex, FILE* IR_file)
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

    //fprintf(IR_file, "FunctionBody(func_%d_%zu, %zu)\t# %s\n", FunctionIndex, NumberOfArguments, NumberOfArguments, FunctionTable->FunctionsArray[FunctionIndex].Name);
    IR_FUNCTION_BODY_((size_t)FunctionIndex, NumberOfArguments,  FunctionTable->FunctionsArray[FunctionIndex].LocalVariableMaxIndex, FunctionTable->FunctionsArray[FunctionIndex].Name);
    size_t TemporalVariableIndex = 0;

    for(size_t i = 0; i < NumberOfArguments; i++)
    {
        IR_TAKE_ARG_((long long int) TemporalVariableIndex, i, "");
        TemporalVariableIndex++;
    }

    GenerateScopeRepresentation(FunctionBodyScope, &TemporalVariableIndex, LocalLabelIndex, IR_file, FunctionTable);

    return 0;
}

static size_t GenerateScopeRepresentation(void* TreeNode, size_t* TemporalVariableIndex, size_t* LocalLabelIndex, FILE* IR_file, __attribute((unused))FunctionTable_t* FunctionTable)
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
                GenerateAssignmentRepresentation(StatementNode, TemporalVariableIndex, LocalLabelIndex, IR_file, FunctionTable);
                break;
            }
            case RETURN_NODE:
            {
                GenerateReturnRepresentation(StatementNode, TemporalVariableIndex, LocalLabelIndex, IR_file, FunctionTable);
                break;
            }
            case CONDITION_NODE:
            {
                size_t OldTempIndex = *TemporalVariableIndex;
                GenerateConditionRepresentation(StatementNode, &OldTempIndex, LocalLabelIndex, IR_file, FunctionTable);
                break;
            }
            case FUNCTION_CALL_NODE:
            {
                GenerateFunctionCallRepresentation(StatementNode, TemporalVariableIndex, LocalLabelIndex, IR_file, FunctionTable);
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

static size_t GenerateFunctionCallRepresentation(void* TreeNode, size_t* TemporalVariableIndex, size_t* LocalLabelIndex, FILE* IR_file, __attribute((unused))FunctionTable_t* FunctionTable)
{
    assert(TreeNode);
    assert(TemporalVariableIndex);
    assert (IR_file);
    assert(LocalLabelIndex);

    int FunctionIndex = 0;
    memcpy(&FunctionIndex, GetNodeData(TreeNode, DATA_FIELD_CODE, 0), sizeof(FunctionIndex));
    size_t ArgumentCount = FunctionTable->FunctionsArray[FunctionIndex].NumberOfArguments;

    for(size_t i = 0; i < ArgumentCount; i++)
    {
        void* Argument = NULL;
        memcpy(&Argument, GetNodeData(TreeNode, DESCENDANTS_FIELD_CODE, i), sizeof(Argument));
        assert(Argument);

        GenerateExpressionRepresentation(Argument, TemporalVariableIndex, LocalLabelIndex, IR_file, FunctionTable);

        IR_GIVE_ARG_(i, *TemporalVariableIndex - 1);
    }

    IR_CALL_FUNC_(*TemporalVariableIndex, (size_t)FunctionIndex, ArgumentCount, "");
    (*TemporalVariableIndex)++;
    return 0;
}


static size_t GenerateConditionRepresentation(void* TreeNode, size_t* TemporalVariableIndex, size_t* LocalLabelIndex, FILE* IR_file, __attribute((unused))FunctionTable_t* FunctionTable)
{
    assert(TreeNode);
    assert(TemporalVariableIndex);
    assert (IR_file);
    assert(LocalLabelIndex);

    void* Condition = NULL;
    memcpy(&Condition, GetNodeData(TreeNode, DESCENDANTS_FIELD_CODE, 1), sizeof(void*));
    assert(Condition);

    int ConditionIndex = 0;
    memcpy(&ConditionIndex, GetNodeData(TreeNode, DATA_FIELD_CODE, 0), sizeof(ConditionIndex));

    switch(ConditionIndex)
    {
        case IF_INDEX:
        {
            GenerateExpressionRepresentation(Condition, TemporalVariableIndex, LocalLabelIndex, IR_file, FunctionTable);
            //fprintf(IR_file, "conditional_jump(%zu_tmp, label_%zu)\n", *TemporalVariableIndex - 1, *LocalLabelIndex);
            IR_COND_JMP_(*LocalLabelIndex, *TemporalVariableIndex - 1, "If's condition");
            size_t IfEnd = *LocalLabelIndex;
            (*LocalLabelIndex)++;

            void* IfScope = NULL;
            memcpy(&IfScope, GetNodeData(TreeNode, DESCENDANTS_FIELD_CODE, 0), sizeof(void*));
            assert(IfScope);

            GenerateScopeRepresentation(IfScope, TemporalVariableIndex, LocalLabelIndex, IR_file, FunctionTable);
            //fprintf(IR_file, "label(%zu)\t# %s\n", IfEnd, "If's end");
            IR_LABEL_(IfEnd, "If's end");

            break;
        }
        case WHILE_INDEX:
        {
            size_t WhileStart = *LocalLabelIndex;
            (*LocalLabelIndex)++;
            //fprintf(IR_file, "label(%zu)\t# %s\n", WhileStart, "While's start");
            IR_LABEL_(WhileStart, "While's start");
            GenerateExpressionRepresentation(Condition, TemporalVariableIndex, LocalLabelIndex, IR_file, FunctionTable);

            size_t WhileEnd = *LocalLabelIndex;
            (*LocalLabelIndex)++;

            //fprintf(IR_file, "conditional_jump(%zu_tmp, label_%zu)\n", *TemporalVariableIndex - 1, WhileEnd);
            IR_COND_JMP_(WhileEnd, *TemporalVariableIndex - 1, "While's condition");

            void* WhileScope = NULL;
            memcpy(&WhileScope, GetNodeData(TreeNode, DESCENDANTS_FIELD_CODE, 0), sizeof(void*));
            assert(WhileScope);

            GenerateScopeRepresentation(WhileScope, TemporalVariableIndex, LocalLabelIndex, IR_file, FunctionTable);
            //fprintf(IR_file, "jump(label_%zu)\n", WhileStart);
            IR_JMP_(WhileStart, "Jump to while's condition");
            //fprintf(IR_file, "label(%zu)\t# %s\n", WhileEnd, "While's end");
            IR_LABEL_(WhileEnd, "End of the while");
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

static size_t GenerateAssignmentRepresentation(void* TreeNode, size_t* TemporalVariableIndex, size_t* LocalLabelIndex, FILE* IR_file, __attribute((unused))FunctionTable_t* FunctionTable)
{
    assert(TreeNode);
    assert(TemporalVariableIndex);
    assert (IR_file);
    assert(LocalLabelIndex);

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

    GenerateExpressionRepresentation(Expression, TemporalVariableIndex, LocalLabelIndex, IR_file, FunctionTable);

    //fprintf(IR_file, "assignment(var%d, %zu_tmp)\t# %s\n", VariableTableIndex, *TemporalVariableIndex - 1, VariableName);
    IR_ASSIGN_VAR_((long long int)VariableTableIndex, *TemporalVariableIndex - 1, VariableName);
    //(*TemporalVariableIndex)++;
    return 0;
}

static size_t GenerateExpressionRepresentation(void* TreeNode, size_t* TemporalVariableIndex, size_t* LocalLabelIndex, FILE* IR_file, __attribute((unused))FunctionTable_t* FunctionTable)
{
    assert(TreeNode);
    assert(TemporalVariableIndex);
    assert (IR_file);
    assert(LocalLabelIndex);

    size_t CurrentNodeType = 0;
    memcpy(&CurrentNodeType, GetNodeData(TreeNode, TYPE_FIELD_CODE, 0), sizeof(CurrentNodeType));

    switch(CurrentNodeType)
    {
        case INTEGER_VALUE_NODE:
        {
            GenerateIntegerValueRepresentation(TreeNode, TemporalVariableIndex, LocalLabelIndex, IR_file, FunctionTable);
            break;
        }
        case FLOAT_VALUE_NODE:
        {
            GenerateFloatValueRepresentation(TreeNode, TemporalVariableIndex, LocalLabelIndex, IR_file, FunctionTable);
            break;
        }
        case OPERATION_NODE:
        {
            GenerateOperationRepresentation(TreeNode, TemporalVariableIndex, LocalLabelIndex, IR_file, FunctionTable);

            break;
        }
        case FUNCTION_CALL_NODE:
        {
            GenerateFunctionCallRepresentation(TreeNode, TemporalVariableIndex, LocalLabelIndex, IR_file, FunctionTable);
            break;
        }
        case RIGHT_VARIABLE_NODE:
        {
            int VariableIndex = 0;
            memcpy(&VariableIndex, GetNodeData(TreeNode, DATA_FIELD_CODE, 0), sizeof(VariableIndex));
            //fprintf(IR_file, "assignment(%zu_tmp, var_%d)\n", *TemporalVariableIndex, VariableIndex);
            IR_ASSIGN_TMP_VAR_(*TemporalVariableIndex, (long long int)VariableIndex, "");
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

static size_t GenerateIntegerValueRepresentation(void* TreeNode, size_t* TemporalVariableIndex, __attribute((unused))size_t* LocalLabelIndex, FILE* IR_file, __attribute((unused))FunctionTable_t* FunctionTable)
{
    int Data = 0;
    memcpy(&Data, GetNodeData(TreeNode, DATA_FIELD_CODE, 0), sizeof(Data));
    //fprintf(IR_file, "assignment(%zu_tmp, %d)\n", *TemporalVariableIndex, Data);
    IR_ASSIGN_TMP_NUM_(*TemporalVariableIndex, (double)Data);
    (*TemporalVariableIndex)++;
    return *TemporalVariableIndex - 1;  
}

static size_t GenerateFloatValueRepresentation(void* TreeNode, size_t* TemporalVariableIndex, __attribute((unused))size_t* LocalLabelIndex, FILE* IR_file, __attribute((unused))FunctionTable_t* FunctionTable)
{
    double Data = 0;
    memcpy(&Data, GetNodeData(TreeNode, DATA_FIELD_CODE, 0), sizeof(Data));
    //fprintf(IR_file, "assignment(%zu_tmp, %lg)\n", *TemporalVariableIndex, Data);  
    IR_ASSIGN_TMP_NUM_(*TemporalVariableIndex, Data);
    (*TemporalVariableIndex)++;
    return *TemporalVariableIndex - 1;
}

static size_t GenerateReturnRepresentation(void* TreeNode, size_t* TemporalVariableIndex, size_t* LocalLabelIndex, FILE* IR_file, __attribute((unused))FunctionTable_t* FunctionTable)
{
    assert(TreeNode);
    assert(TemporalVariableIndex);
    assert (IR_file);
    assert(LocalLabelIndex);

    void* ReturnValue = NULL;
    memcpy(&ReturnValue, GetNodeData(TreeNode, DESCENDANTS_FIELD_CODE, 0), sizeof(void*));
    assert(ReturnValue);

    GenerateExpressionRepresentation(ReturnValue, TemporalVariableIndex, LocalLabelIndex, IR_file, FunctionTable);

    //fprintf(IR_file, "return(%zu_tmp)\n", *TemporalVariableIndex - 1);
    IR_RET_(*TemporalVariableIndex - 1);

    return 0;
}


static size_t GenerateOperationRepresentation(void* TreeNode, size_t* TemporalVariableIndex, size_t* LocalLabelIndex, FILE* IR_file, __attribute((unused))FunctionTable_t* FunctionTable)
{
    assert(TreeNode);
    assert(TemporalVariableIndex);
    assert (IR_file);
    assert(LocalLabelIndex);

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
                //fprintf(IR_file, "assignment(%zu_tmp, var_%d)\n", *TemporalVariableIndex, VariableIndex);
                IR_ASSIGN_TMP_VAR_(*TemporalVariableIndex, (long long int)VariableIndex, "");
                ResultTmp[i] = *TemporalVariableIndex;
                (*TemporalVariableIndex)++;
                break;
            }
            case OPERATION_NODE:
            {
                ResultTmp[i] = GenerateOperationRepresentation(Descendant, TemporalVariableIndex, LocalLabelIndex, IR_file, FunctionTable);
                break;
            }
            case INTEGER_VALUE_NODE:
            {
                ResultTmp[i] = GenerateIntegerValueRepresentation(Descendant, TemporalVariableIndex, LocalLabelIndex, IR_file, FunctionTable);
                break;
            }
            case FLOAT_VALUE_NODE:
            {
                ResultTmp[i] = GenerateFloatValueRepresentation(Descendant, TemporalVariableIndex, LocalLabelIndex, IR_file, FunctionTable);
                break;
            }
            default:
            {
                assert(0);
                break;
            }
        }
    }

    //fprintf(IR_file, "operation(%d, %zu_tmp, %zu_tmp, %zu_tmp)\n", OperationCode, ResultTmp[0], ResultTmp[1], *TemporalVariableIndex);
    IR_OPERATION_(*TemporalVariableIndex, SyntaxerOpCodesToPYAMConversion(OperationCode), ResultTmp[0], ResultTmp[1]);
    (*TemporalVariableIndex)++;

    return *TemporalVariableIndex - 1;
}

static enum IrOpType SyntaxerOpCodesToPYAMConversion(int OpCode)
{
    switch(OpCode)
    {
        case ARIPHMETICAL_SUM_INDEX:
        {
            return IR_OP_TYPE_SUM;
        }
        case ARIPHMETICAL_SUB_INDEX:
        {
            return IR_OP_TYPE_SUB;
        }
        case ARIPHMETICAL_MUL_INDEX:
        {
            return IR_OP_TYPE_MUL;
        }
        case ARIPHMETICAL_DIV_INDEX:
        {
            return IR_OP_TYPE_DIV;
        }
        case EQUALITY_INDEX:
        {
            return IR_OP_TYPE_EQ;
        }
        case NOT_EQUAL_INDEX:
        {
            return IR_OP_TYPE_NEQ;
        }
        case LESS_INDEX:
        {
            return IR_OP_TYPE_LESS;
        }
        case LESS_OR_EQUAL_INDEX:
        {
            return IR_OP_TYPE_LESSEQ;
        }
        case MORE_INDEX:
        {
            return IR_OP_TYPE_GREAT;
        }
        case MORE_OR_EQUAL_INDEX:
        {
            return IR_OP_TYPE_GREATEQ;
        }
        default:
        {
            assert(0 && "Unknown operand");
        }
    }
}