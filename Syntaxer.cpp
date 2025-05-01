#include "Syntaxer.h"

#include <assert.h>
#include "List/List.h"

static void* GetType(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, VariableTable_t* VariableTable, FunctionTable_t* FunctionTable);
static void* GetVariableDeclaration(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, VariableTable_t* VariableTable, FunctionTable_t* FunctionTable);

static void* GetFunctionBody(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, VariableTable_t* VariableTable, FunctionTable_t* FunctionTable);
static void* GetFunctionBodyArguments(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, VariableTable_t* VariableTable, FunctionTable_t* FunctionTable);

static void* GetScope(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, VariableTable_t* VariableTable, FunctionTable_t* FunctionTable);
static void* GetStatement(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, VariableTable_t* VariableTable, FunctionTable_t* FunctionTable);
static void* GetConditionalCycle(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, VariableTable_t* VariableTable, FunctionTable_t* FunctionTable);
static void* GetConditionArguments(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, VariableTable_t* VariableTable, FunctionTable_t* FunctionTable);
static void* GetStatementOperand(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, VariableTable_t* VariableTable, FunctionTable_t* FunctionTable);
static void* GetLeftVariable(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, VariableTable_t* VariableTable, FunctionTable_t* FunctionTable);

static size_t FunctionTableAppend(FunctionTable_t* FunctionTable, Function_t Function);

static size_t SearchType(TokenTable_t* TokenTable, size_t* TokenIndex);
static size_t VariableAppend(VariableTable_t* VariableTable, Variable_t Variable);
static int VariableTableSearch(VariableTable_t* VariableTable, Variable_t Variable);

static void* GetExpression(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, VariableTable_t* VariableTable, FunctionTable_t* FunctionTable);
static void* GetLogicalXor(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, VariableTable_t* VariableTable, FunctionTable_t* FunctionTable);
static void* GetLogicalOr(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, VariableTable_t* VariableTable, FunctionTable_t* FunctionTable);
static void* GetLogicalAnd(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, VariableTable_t* VariableTable, FunctionTable_t* FunctionTable);
static void* GetAriphmeticalSum(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, VariableTable_t* VariableTable, FunctionTable_t* FunctionTable);
static void* GetAriphmeticalMul(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, VariableTable_t* VariableTable, FunctionTable_t* FunctionTable);
static void* GetExpressionBrackets(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, VariableTable_t* VariableTable, FunctionTable_t* FunctionTable);
static void* GetRightVariable(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, VariableTable_t* VariableTable, FunctionTable_t* FunctionTable);
static void* GetValue(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, VariableTable_t* VariableTable, FunctionTable_t* FunctionTable);


static size_t ScopeStatementCount(const TokenTable_t* TokenTable, size_t TokenIndex, size_t* StatementCounter);


int GetProgram(Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, VariableTable_t* VariableTable, FunctionTable_t* FunctionTable)
{
    assert(Tree);
    assert(TokenTable);
    assert(TokenIndex);
    assert(VariableTable);
    assert(FunctionTable);

    const size_t InitProgramListSize = 128;
    List_t* ProgramList = NULL;
    ListInit(&ProgramList, InitProgramListSize, sizeof(void*), 0);

    assert(ProgramList);

    while(*TokenIndex < TokenTable->Free)
    {
        void* Branch = GetType(Tree, TokenTable, TokenIndex, VariableTable, FunctionTable);
        PushFront(ProgramList, &Branch); 
    }
    
    size_t ProgramListLength = GetLinearListSize(ProgramList) / ProgramList->elsize;
    fprintf(stderr, "%zu\n", ProgramListLength);
    Tree->root = Tree->InitNode(Tree, PROGRAM_NODE, 0, NULL, ProgramListLength);

    for(size_t i = 0; i < ProgramListLength; i++)
    {
        AddDescendant(Tree->root, (void*)(*(size_t*)ListGetNodeValueInd(ProgramList, i + 1)), i);
    }

    ListDstr(ProgramList);

    return 0;
}

static void* GetType(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, VariableTable_t* VariableTable, FunctionTable_t* FunctionTable)
{
    assert(Tree);
    assert(TokenTable);
    assert(TokenIndex);
    assert(VariableTable);
    assert(FunctionTable);
    
    if(TokenTable->TokenArray[*TokenIndex].TokenType == LONG_TOKEN)
    {
        for(size_t i = 1; i < TYPE_SECTION_END - TYPE_SECTION_START; i++)
        {
            if(strncmp(TokenTable->TokenArray[*TokenIndex].TokenData.LongTokenName, KeyWordsArray[TYPE_SECTION_START + i].Name, strlen(KeyWordsArray[TYPE_SECTION_END + i].Name)) == 0)
            {
                TokenTable->TokenArray[*TokenIndex].AdditionalData = TYPE_SECTION_START + (int)i;
                (*TokenIndex)++;
                return GetVariableDeclaration(Tree, TokenTable, TokenIndex, VariableTable, FunctionTable); // Statement with variable or function
            }
            else
            {
                assert(0);
            }
        }
    }
    else
    {
        assert(0);
    }
    assert(0);
    return NULL;
}

static void* GetVariableDeclaration(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, VariableTable_t* VariableTable, FunctionTable_t* FunctionTable)
{
    assert(Tree);
    assert(TokenTable);
    assert(TokenIndex);
    assert(VariableTable);
    assert(FunctionTable);

    size_t TokenType = TokenTable->TokenArray[*TokenIndex].TokenType;
    //size_t Type = (size_t)TokenTable->TokenArray[*TokenIndex - 1].AdditionalData;

    if(TokenType == LONG_TOKEN)
    {
        if(TokenTable->TokenArray[*TokenIndex + 1].TokenType == PARENTHESES_OPEN_TOKEN)
        {
            return GetFunctionBody(Tree, TokenTable, TokenIndex, VariableTable, FunctionTable);
        }
    }
    else // TODO global variable init
    {
        assert(0);
        return NULL;
    }
    return NULL;
}

static void* GetFunctionBody(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, VariableTable_t* VariableTable, FunctionTable_t* FunctionTable)
{
    assert(Tree);
    assert(TokenTable);
    assert(TokenIndex);
    assert(VariableTable);
    assert(FunctionTable);

    if(TokenTable->TokenArray[*TokenIndex].TokenType == LONG_TOKEN)
    {
        for(size_t i = 0; i < FunctionTable->Free; i++)
        {
            if(strncmp(TokenTable->TokenArray[*TokenIndex].TokenData.LongTokenName, FunctionTable->FunctionsArray[i].Name, strlen(FunctionTable->FunctionsArray[i].Name)) == 0)
            {
                assert(0);
            }
        }

        Function_t Function = {};
        strncpy(Function.Name, TokenTable->TokenArray[*TokenIndex].TokenData.LongTokenName, FUNCTION_NAME_MAX);
        Function.ReturnType = (size_t)TokenTable->TokenArray[*TokenIndex - 1].AdditionalData;

        int ParenthesisCounter = 1;
        size_t ArgumentCounter = 0;
        for(size_t i = *TokenIndex + 2; i < TokenTable->Free; i++)
        {
            size_t CurrentTokenType = TokenTable->TokenArray[i].TokenType;
            if(CurrentTokenType == ARG_SEPARATOR_TOKEN)
            {
                ArgumentCounter++;
            }
            else if(CurrentTokenType == PARENTHESES_OPEN_TOKEN)
            {
                ParenthesisCounter++;
            }
            else if(CurrentTokenType == PARENTHESES_CLOSE_TOKEN)
            {
                ParenthesisCounter--;
            }

            if(ParenthesisCounter == 0)
            {
                break;
            }
        }

        Function.NumberOfArguments = ArgumentCounter;

        size_t FunctionIndex = FunctionTableAppend(FunctionTable, Function);

        // TEMP SOLUTION
        // Now making cool solution
        (*TokenIndex)++;
        void* Arguments = GetFunctionBodyArguments(Tree, TokenTable, TokenIndex, VariableTable, FunctionTable);

        assert(TokenTable->TokenArray[*TokenIndex].TokenType == PARENTHESES_CLOSE_TOKEN);

        // TEMP SOLUTION

        (*TokenIndex)++;
        
        return Tree->InitNode(Tree, FUNCTION_BODY_NODE, sizeof(FunctionIndex), &FunctionIndex, 2, GetScope(Tree, TokenTable, TokenIndex, VariableTable, FunctionTable), Arguments);
    }
    else
    {
        assert(0);
    }
} // ADD ARGUMENTS

static void* GetFunctionBodyArguments(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, VariableTable_t* VariableTable, FunctionTable_t* FunctionTable)
{
    assert(Tree);
    assert(TokenTable);
    assert(TokenIndex);
    assert(VariableTable);
    assert(FunctionTable);

    if(TokenTable->TokenArray[*TokenIndex].TokenType != PARENTHESES_OPEN_TOKEN)
    {
        assert(0);
    }

    (*TokenIndex)++;

    size_t ArgumentCounter = 0;
    for(size_t i = *TokenIndex; i < TokenTable->Free; i++) // REFACTOR if pointers are added; hopefully no one will notice that I am cute little gay boy 
    {
        size_t CurrentTokenType = TokenTable->TokenArray[i].TokenType;
        if(CurrentTokenType == PARENTHESES_CLOSE_TOKEN)
        {
            break;
        }
        else if(CurrentTokenType == ARG_SEPARATOR_TOKEN)
        {
            ArgumentCounter++;
        }
    }

    if(TokenTable->TokenArray[*TokenIndex].TokenType != PARENTHESES_CLOSE_TOKEN)
    {
        ArgumentCounter++;
    }
    else
    {
        return Tree->InitNode(Tree, FUNCTION_ARGUMENTS_NODE, sizeof(ArgumentCounter), &ArgumentCounter, ArgumentCounter);
    }

    void* Arguments = Tree->InitNode(Tree, FUNCTION_ARGUMENTS_NODE, sizeof(ArgumentCounter), &ArgumentCounter, ArgumentCounter);

    for(size_t  i = 0; i < ArgumentCounter; i++) // when making stack frames, new frame should be addded precisely here, any variables from outer frames will be overshadowed by function's;
    {
        size_t Type = 0;
        for(size_t j = 1; j < TYPE_SECTION_END - TYPE_SECTION_START; j++)
        {
            if(strncmp(TokenTable->TokenArray[*TokenIndex].TokenData.LongTokenName, KeyWordsArray[TYPE_SECTION_START + j].Name, strlen(KeyWordsArray[TYPE_SECTION_START + j].Name)) == 0)
            {
                Type = j;
                break;
            }
        }

        assert(Type);
        
        Variable_t Argument = {};
        Argument.Type = Type;

        (*TokenIndex)++;

        strncpy(Argument.Name, TokenTable->TokenArray[*TokenIndex].TokenData.LongTokenName, strlen(TokenTable->TokenArray[*TokenIndex].TokenData.LongTokenName));

        size_t TableIndex = VariableAppend(VariableTable, Argument);

        void* ArgumentNode = Tree->InitNode(Tree, LEFT_VARIABLE_NODE, sizeof(TableIndex), &TableIndex, 0);

        AddDescendant(Arguments, ArgumentNode, i);

        (*TokenIndex) += 2;
    }
    (*TokenIndex)--;
    return Arguments;
}

static void* GetScope(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, VariableTable_t* VariableTable, FunctionTable_t* FunctionTable)
{
    assert(Tree);
    assert(TokenTable);
    assert(TokenIndex);
    assert(VariableTable);
    assert(FunctionTable);

    size_t TokenType = TokenTable->TokenArray[*TokenIndex].TokenType;
    if(TokenType == CURLY_OPEN_TOKEN)
    {
        (*TokenIndex)++;
        
        size_t StatementCounter = 0;
        ScopeStatementCount(TokenTable, *TokenIndex, &StatementCounter); // UPGRADE LATER; UPGRADED, pretty cool now;

        void* ScopeNode = Tree->InitNode(Tree, SCOPE_NODE, sizeof(StatementCounter), &StatementCounter, StatementCounter);

        for(size_t i = 0; i < StatementCounter; i++)
        {
            AddDescendant(ScopeNode, GetStatement(Tree, TokenTable, TokenIndex, VariableTable, FunctionTable), i);
        }

        if(TokenTable->TokenArray[*TokenIndex].TokenType != CURLY_CLOSE_TOKEN)
        {
            assert(0);
        }

        (*TokenIndex)++;

        return ScopeNode;
    }
    else
    {
        assert(0);
        return NULL;
    }
    return NULL;
}

static size_t ScopeStatementCount(const TokenTable_t* TokenTable, size_t TokenIndex, size_t* StatementCounter)
{
    size_t CurlyCounter = 1;
    size_t i = 0;
    for(i = TokenIndex; i < TokenTable->Free; i++)
    {
        size_t CurrentTokenType = TokenTable->TokenArray[i].TokenType;

        if(CurrentTokenType == STATEMENT_SEPARATOR_TOKEN)
        {
            (*StatementCounter)++;
        }

        else if(CurrentTokenType == CURLY_OPEN_TOKEN)
        {
            CurlyCounter++;
        }

        else if(CurrentTokenType == CURLY_CLOSE_TOKEN)
        {
            CurlyCounter--;
        }
        else if(CurrentTokenType == LONG_TOKEN)
        {
            for(size_t j = 1; j < CONDITIONAL_CYCLES_SECTION_END - CONDITIONAL_CYCLES_SECTION_START; j++) // RENAME RETARD, RETARD RETARD
            {
                if(strncmp(TokenTable->TokenArray[i].TokenData.LongTokenName, KeyWordsArray[CONDITIONAL_CYCLES_SECTION_START + j].Name, strlen(KeyWordsArray[CONDITIONAL_CYCLES_SECTION_START + j].Name)) == 0)
                {
                    (*StatementCounter)++;
                    while(TokenTable->TokenArray[i].TokenType != CURLY_OPEN_TOKEN)
                    {
                        i++;
                    }
                    i++;
                    size_t Fill = 0;
                    i += ScopeStatementCount(TokenTable, i, &Fill);
                    break;
                }
            }   
        }

        if(CurlyCounter == 0)
        {
            break;
        }
    }

    return i - TokenIndex;
}

static size_t FunctionTableAppend(FunctionTable_t* FunctionTable, Function_t Function)
{
    assert(FunctionTable);

    if(FunctionTable->Free >= FunctionTable->Capacity)
    {
        FunctionTable->FunctionsArray = (Function_t*)realloc(FunctionTable->FunctionsArray, FunctionTable->Capacity * ArrayResizeMultiplier * sizeof(Function_t));
        FunctionTable->Capacity *= ArrayResizeMultiplier;
        assert(FunctionTable->FunctionsArray);
    }

    FunctionTable->FunctionsArray[FunctionTable->Free] = Function;
    (FunctionTable->Free)++;

    return FunctionTable->Free - 1;
}

static void* GetStatement(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, VariableTable_t* VariableTable, FunctionTable_t* FunctionTable)
{
    assert(Tree);
    assert(TokenTable);
    assert(TokenIndex);
    assert(VariableTable);
    assert(FunctionTable);

    for(size_t i = 1; i < CONDITIONAL_CYCLES_SECTION_END - CONDITIONAL_CYCLES_SECTION_START; i++)
    {
        if(strncmp(TokenTable->TokenArray[*TokenIndex].TokenData.LongTokenName, KeyWordsArray[CONDITIONAL_CYCLES_SECTION_START + i].Name, strlen(KeyWordsArray[CONDITIONAL_CYCLES_SECTION_START + i].Name)) == 0)
        {
            TokenTable->TokenArray[*TokenIndex].AdditionalData = CONDITIONAL_CYCLES_SECTION_START + (int)i;
            return GetConditionalCycle(Tree, TokenTable, TokenIndex, VariableTable, FunctionTable);
        }
    }

    void* LeftPart = GetLeftVariable(Tree, TokenTable, TokenIndex, VariableTable, FunctionTable);
    void* OperandNode = GetStatementOperand(Tree, TokenTable, TokenIndex, VariableTable, FunctionTable);

    AddDescendant(OperandNode, LeftPart, 0);
    AddDescendant(OperandNode, GetExpression(Tree, TokenTable, TokenIndex, VariableTable, FunctionTable), 1);

    if(TokenTable->TokenArray[*TokenIndex].TokenType == STATEMENT_SEPARATOR_TOKEN)
    {
        (*TokenIndex)++;
        return OperandNode;
    }
    else
    {
        assert(0);
    }
}

static void* GetConditionalCycle(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, VariableTable_t* VariableTable, FunctionTable_t* FunctionTable)
{
    assert(Tree);
    assert(TokenTable);
    assert(TokenIndex);
    assert(VariableTable);
    assert(FunctionTable);

    void* ConditionNode = Tree->InitNode(Tree, CONDITION_NODE, sizeof(TokenTable->TokenArray[*TokenIndex].AdditionalData), &TokenTable->TokenArray[*TokenIndex].AdditionalData, 2);

    (*TokenIndex)++;

    if(TokenTable->TokenArray[*TokenIndex].TokenType != PARENTHESES_OPEN_TOKEN)
    {
        assert(0);
    }

    (*TokenIndex)++;

    void* Condition =  GetConditionArguments(Tree, TokenTable, TokenIndex, VariableTable, FunctionTable);

    assert(Condition);
    assert(GetNodeData(Condition, DESCENDANTS_FIELD_CODE, 0));

    (*TokenIndex)++;

    AddDescendant(ConditionNode, Condition, 1);

    AddDescendant(ConditionNode, GetScope(Tree, TokenTable, TokenIndex, VariableTable, FunctionTable), 0);

    return ConditionNode;
}

static void* GetConditionArguments(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, VariableTable_t* VariableTable, FunctionTable_t* FunctionTable)
{
    assert(Tree);
    assert(TokenTable);
    assert(TokenIndex);
    assert(VariableTable);
    assert(FunctionTable);

    return GetExpression(Tree, TokenTable, TokenIndex, VariableTable, FunctionTable);
}

static void* GetStatementOperand(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, VariableTable_t* VariableTable, FunctionTable_t* FunctionTable)
{
    assert(Tree);
    assert(TokenTable);
    assert(TokenIndex);
    assert(VariableTable);
    assert(FunctionTable);

    size_t TokenType = TokenTable->TokenArray[*TokenIndex].TokenType;
    if(TokenType == LONG_TOKEN)
    {
        for(size_t i = 1; i < STATEMENT_OPERAND_SECTION_END - STATEMENT_OPERAND_SECTION_START; i++)
        {
            if(strncmp(TokenTable->TokenArray[*TokenIndex].TokenData.LongTokenName, KeyWordsArray[STATEMENT_OPERAND_SECTION_START + i].Name, strlen(KeyWordsArray[STATEMENT_OPERAND_SECTION_START + i].Name)) == 0)
            {
                size_t Index = STATEMENT_OPERAND_SECTION_START + i;
                (*TokenIndex)++;
                return Tree->InitNode(Tree, STATEMENT_OPERAND_NODE, sizeof(Index), &Index, 2);
            }
        }
        assert(0);
    }
    else
    {
        assert(0);
    }
    
}

static void* GetLeftVariable(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, VariableTable_t* VariableTable, FunctionTable_t* FunctionTable)
{
    assert(Tree);
    assert(TokenTable);
    assert(TokenIndex);
    assert(VariableTable);
    assert(FunctionTable);

    size_t TokenType = TokenTable->TokenArray[*TokenIndex].TokenType;
    if(TokenType == LONG_TOKEN)
    {
        Variable_t LeftVariable = {};
        strncpy(LeftVariable.Name, TokenTable->TokenArray[*TokenIndex].TokenData.LongTokenName, strlen(TokenTable->TokenArray[*TokenIndex].TokenData.LongTokenName));
        LeftVariable.Data.Integer = 0;
        LeftVariable.Assigned = 1; // TODO change later

        size_t VariableIndex = VariableAppend(VariableTable, LeftVariable);

        (*TokenIndex)++;

        return Tree->InitNode(Tree, LEFT_VARIABLE_NODE, sizeof(VariableIndex), &VariableIndex, 0);

    }
    else
    {
        assert(0); 
    }
}

static size_t SearchType(TokenTable_t* TokenTable, size_t* TokenIndex)
{
    assert(TokenTable);
    assert(TokenIndex);

    for(size_t i = 1; i < TYPE_SECTION_END - TYPE_SECTION_START; i++)
    {
        if(strncmp(TokenTable->TokenArray[*TokenIndex].TokenData.LongTokenName, KeyWordsArray[TYPE_SECTION_START + i].Name, strlen(KeyWordsArray[TYPE_SECTION_START + i].Name)) == 0)
        {
            return TYPE_SECTION_START + i;
        }
    }
    return 0;
}


static size_t VariableAppend(VariableTable_t* VariableTable, Variable_t Variable)
{
    assert(VariableTable);

    int VariableIndex = VariableTableSearch(VariableTable, Variable);
    
    if(VariableIndex >= 0)
    {
        return VariableIndex;
    }

    if(VariableTable->Free >= VariableTable->Capacity)
    {
        VariableTable->VariablesArray = (Variable_t*)realloc(VariableTable->VariablesArray, VariableTable->Capacity * ArrayResizeMultiplier * sizeof(Variable_t));
        VariableTable->Capacity *= ArrayResizeMultiplier;
        assert(VariableTable->VariablesArray);
    }

    VariableTable->VariablesArray[VariableTable->Free] = Variable;
    (VariableTable->Free)++;

    return VariableTable->Free - 1;
}

static int VariableTableSearch(VariableTable_t* VariableTable, Variable_t Variable)
{
    for(size_t i = 0; i < VariableTable->Free; i++)
    {
        if(strncmp(VariableTable->VariablesArray[i].Name, Variable.Name, strlen(VariableTable->VariablesArray[i].Name)) == 0)
        {
            return i;
        }
    }
    return -1;
}

//
// ARIPHMETICO-LOGICAL EXPRESSIONS SECTION START
//


static void* GetExpression(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, VariableTable_t* VariableTable, FunctionTable_t* FunctionTable)
{
    assert(Tree);
    assert(TokenTable);
    assert(TokenIndex);
    assert(VariableTable);
    assert(FunctionTable);

    void* NewNode = Tree->InitNode(Tree, EXPRESSION_NODE, 0, NULL,  1, GetLogicalXor(Tree, TokenTable, TokenIndex, VariableTable, FunctionTable));
    return NewNode;
}

static void* GetLogicalXor(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, VariableTable_t* VariableTable, FunctionTable_t* FunctionTable)
{
    assert(Tree);
    assert(TokenTable);
    assert(TokenIndex);
    assert(VariableTable);
    assert(FunctionTable);

    void* NewNode1 = GetLogicalOr(Tree, TokenTable, TokenIndex, VariableTable, FunctionTable);
    void* NewNode2 = NULL;
    void* Root = NewNode1;

    while(TokenTable->TokenArray[*TokenIndex].TokenType == LONG_TOKEN)
    {
        if(strncmp(TokenTable->TokenArray[*TokenIndex].TokenData.LongTokenName, KeyWordsArray[LOGICAL_XOR_INDEX].Name, strlen(KeyWordsArray[LOGICAL_XOR_INDEX].Name)) == 0)
        {
            (*TokenIndex)++;
            NewNode2 = GetLogicalOr(Tree, TokenTable, TokenIndex, VariableTable, FunctionTable);
            size_t Type1 = *(size_t*)GetNodeData(Root, TYPE_FIELD_CODE, 0);
            size_t Type2 = *(size_t*)GetNodeData(NewNode2, TYPE_FIELD_CODE, 0);
            if(Type1 == INTEGER_VALUE_NODE && Type2 == INTEGER_VALUE_NODE)
            {
                int Value1 = *((int*)GetNodeData(Root, DATA_FIELD_CODE, 0));
                int Value2 = *((int*)GetNodeData(NewNode2, DATA_FIELD_CODE, 0));
                Value1 ^= Value2;
                free(Root);
                Root = Tree->InitNode(Tree, INTEGER_VALUE_NODE, sizeof(Value1), &Value1, 0);
                free(NewNode2);
                continue;
            }
            else if(Type1 == FLOAT_VALUE_NODE && Type2 == FLOAT_VALUE_NODE)
            {
                double Value1 = *((double*)GetNodeData(Root, DATA_FIELD_CODE, 0));
                double Value2 = *((double*)GetNodeData(NewNode2, DATA_FIELD_CODE, 0));
                Value1 = (size_t)Value2 ^ (size_t)Value1;
                free(Root);
                Root = Tree->InitNode(Tree, FLOAT_VALUE_NODE, sizeof(Value1), &Value1, 0);
                free(NewNode2);
                continue;
            }
            Root = Tree->InitNode(Tree, LOGICAL_XOR_NODE, 0, NULL, 2, Root, NewNode2);
        }
        else
        {
            return Root;
        }
    }

    return Root;
}

static void* GetLogicalOr(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, VariableTable_t* VariableTable, FunctionTable_t* FunctionTable)
{
    assert(Tree);
    assert(TokenTable);
    assert(TokenIndex);
    assert(VariableTable);
    assert(FunctionTable);

    void* NewNode1 = GetLogicalAnd(Tree, TokenTable, TokenIndex, VariableTable, FunctionTable);
    void* NewNode2 = NULL;
    void* Root = NewNode1;

    while(TokenTable->TokenArray[*TokenIndex].TokenType == LONG_TOKEN)
    {
        if(strncmp(TokenTable->TokenArray[*TokenIndex].TokenData.LongTokenName, KeyWordsArray[LOGICAL_OR_INDEX].Name, strlen(KeyWordsArray[LOGICAL_OR_INDEX].Name)) == 0)
        {
            (*TokenIndex)++;
            NewNode2 = GetLogicalAnd(Tree, TokenTable, TokenIndex, VariableTable, FunctionTable);
            size_t Type1 = *(size_t*)GetNodeData(Root, TYPE_FIELD_CODE, 0);
            size_t Type2 = *(size_t*)GetNodeData(NewNode2, TYPE_FIELD_CODE, 0);
            if(Type1 == INTEGER_VALUE_NODE && Type2 == INTEGER_VALUE_NODE)
            {
                int Value1 = *((int*)GetNodeData(Root, DATA_FIELD_CODE, 0));
                int Value2 = *((int*)GetNodeData(NewNode2, DATA_FIELD_CODE, 0));
                Value1 |= Value2;
                free(Root);
                Root = Tree->InitNode(Tree, INTEGER_VALUE_NODE, sizeof(Value1), &Value1, 0);
                free(NewNode2);
                continue;
            }
            else if(Type1 == FLOAT_VALUE_NODE && Type2 == FLOAT_VALUE_NODE)
            {
                double Value1 = *((double*)GetNodeData(Root, DATA_FIELD_CODE, 0));
                double Value2 = *((double*)GetNodeData(NewNode2, DATA_FIELD_CODE, 0));
                Value1 = (size_t)Value2 | (size_t)Value1;
                free(Root);
                Root = Tree->InitNode(Tree, FLOAT_VALUE_NODE, sizeof(Value1), &Value1, 0);
                free(NewNode2);
                continue;
            }
            Root = Tree->InitNode(Tree, LOGICAL_OR_NODE, 0, NULL, 2, Root, NewNode2);
        }
        else
        {
            return Root;
        }
    }

    return Root;
}

static void* GetLogicalAnd(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, VariableTable_t* VariableTable, FunctionTable_t* FunctionTable)
{
    assert(Tree);
    assert(TokenTable);
    assert(TokenIndex);
    assert(VariableTable);
    assert(FunctionTable);

    void* NewNode1 = GetAriphmeticalSum(Tree, TokenTable, TokenIndex, VariableTable, FunctionTable);
    void* NewNode2 = NULL;
    void* Root = NewNode1;

    while(TokenTable->TokenArray[*TokenIndex].TokenType == LONG_TOKEN)
    {
        if(strncmp(TokenTable->TokenArray[*TokenIndex].TokenData.LongTokenName, KeyWordsArray[LOGICAL_AND_INDEX].Name, strlen(KeyWordsArray[LOGICAL_AND_INDEX].Name)) == 0)
        {
            (*TokenIndex)++;
            NewNode2 = GetAriphmeticalMul(Tree, TokenTable, TokenIndex, VariableTable, FunctionTable);
            size_t Type1 = *(size_t*)GetNodeData(Root, TYPE_FIELD_CODE, 0);
            size_t Type2 = *(size_t*)GetNodeData(NewNode2, TYPE_FIELD_CODE, 0);
            if(Type1 == INTEGER_VALUE_NODE && Type2 == INTEGER_VALUE_NODE)
            {
                int Value1 = *((int*)GetNodeData(Root, DATA_FIELD_CODE, 0));
                int Value2 = *((int*)GetNodeData(NewNode2, DATA_FIELD_CODE, 0));
                Value1 &= Value2;
                free(Root);
                Root = Tree->InitNode(Tree, INTEGER_VALUE_NODE, sizeof(Value1), &Value1, 0);
                free(NewNode2);
                continue;
            }
            else if(Type1 == FLOAT_VALUE_NODE && Type2 == FLOAT_VALUE_NODE)
            {
                double Value1 = *((double*)GetNodeData(Root, DATA_FIELD_CODE, 0));
                double Value2 = *((double*)GetNodeData(NewNode2, DATA_FIELD_CODE, 0));
                Value1 = (size_t)Value2 & (size_t)Value1;
                free(Root);
                Root = Tree->InitNode(Tree, FLOAT_VALUE_NODE, sizeof(Value1), &Value1, 0);
                free(NewNode2);
                continue;
            }
            Root = Tree->InitNode(Tree, LOGICAL_AND_NODE, 0, NULL, 2, Root, NewNode2);
        }
        else
        {
            return Root;
        }
    }

    return Root;
}

static void* GetAriphmeticalSum(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, VariableTable_t* VariableTable, FunctionTable_t* FunctionTable)
{
    assert(Tree);
    assert(TokenTable);
    assert(TokenIndex);
    assert(VariableTable);
    assert(FunctionTable);

    void* NewNode1 = GetAriphmeticalMul(Tree, TokenTable, TokenIndex, VariableTable, FunctionTable);
    void* NewNode2 = NULL;
    void* Root = NewNode1;

    while(TokenTable->TokenArray[*TokenIndex].TokenType == LONG_TOKEN)
    {
        if(strncmp(TokenTable->TokenArray[*TokenIndex].TokenData.LongTokenName, KeyWordsArray[ARIPHMETICAL_SUM_INDEX].Name, strlen(KeyWordsArray[ARIPHMETICAL_SUM_INDEX].Name)) == 0)
        {
            (*TokenIndex)++;
            NewNode2 = GetAriphmeticalMul(Tree, TokenTable, TokenIndex, VariableTable, FunctionTable);
            size_t Type1 = *(size_t*)GetNodeData(Root, TYPE_FIELD_CODE, 0);
            size_t Type2 = *(size_t*)GetNodeData(NewNode2, TYPE_FIELD_CODE, 0);
            if(Type1 == INTEGER_VALUE_NODE && Type2 == INTEGER_VALUE_NODE)
            {
                int Value1 = *((int*)GetNodeData(Root, DATA_FIELD_CODE, 0));
                int Value2 = *((int*)GetNodeData(NewNode2, DATA_FIELD_CODE, 0));
                Value1 += Value2;
                free(Root);
                Root = Tree->InitNode(Tree, INTEGER_VALUE_NODE, sizeof(Value1), &Value1, 0);
                free(NewNode2);
                continue;
            }
            else if(Type1 == FLOAT_VALUE_NODE && Type2 == FLOAT_VALUE_NODE)
            {
                double Value1 = *((double*)GetNodeData(Root, DATA_FIELD_CODE, 0));
                double Value2 = *((double*)GetNodeData(NewNode2, DATA_FIELD_CODE, 0));
                Value1 += Value2;
                free(Root);
                Root = Tree->InitNode(Tree, FLOAT_VALUE_NODE, sizeof(Value1), &Value1, 0);
                free(NewNode2);
                continue;
            }
            Root = Tree->InitNode(Tree, ARIPHMETICAL_SUM_NODE, 0, NULL, 2, Root, NewNode2);
        }
        else if(strncmp(TokenTable->TokenArray[*TokenIndex].TokenData.LongTokenName, KeyWordsArray[ARIPHMETICAL_SUB_INDEX].Name, strlen(KeyWordsArray[ARIPHMETICAL_SUB_INDEX].Name)) == 0)
        {
            (*TokenIndex)++;
            NewNode2 = GetAriphmeticalMul(Tree, TokenTable, TokenIndex, VariableTable, FunctionTable);
            size_t Type1 = *(size_t*)GetNodeData(Root, TYPE_FIELD_CODE, 0);
            size_t Type2 = *(size_t*)GetNodeData(NewNode2, TYPE_FIELD_CODE, 0);
            if(Type1 == INTEGER_VALUE_NODE && Type2 == INTEGER_VALUE_NODE)
            {
                int Value1 = *((int*)GetNodeData(Root, DATA_FIELD_CODE, 0));
                int Value2 = *((int*)GetNodeData(NewNode2, DATA_FIELD_CODE, 0));
                Value1 -= Value2;
                free(Root);
                Root = Tree->InitNode(Tree, INTEGER_VALUE_NODE, sizeof(Value1), &Value1, 0);
                free(NewNode2);
                continue;
            }
            else if(Type1 == FLOAT_VALUE_NODE && Type2 == FLOAT_VALUE_NODE)
            {
                double Value1 = *((double*)GetNodeData(Root, DATA_FIELD_CODE, 0));
                double Value2 = *((double*)GetNodeData(NewNode2, DATA_FIELD_CODE, 0));
                Value1 -= Value2;
                free(Root);
                Root = Tree->InitNode(Tree, FLOAT_VALUE_NODE, sizeof(Value1), &Value1, 0);
                free(NewNode2);
                continue;
            }
            Root = Tree->InitNode(Tree, ARIPHMETICAL_SUB_NODE, 0, NULL, 2, Root, NewNode2);
        }
        else
        {
            return Root;
        }
    }
    
    return Root;
}

static void* GetAriphmeticalMul(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, VariableTable_t* VariableTable, FunctionTable_t* FunctionTable)
{
    assert(Tree);
    assert(TokenTable);
    assert(TokenIndex);
    assert(VariableTable);
    assert(FunctionTable);

    void* NewNode1 = GetExpressionBrackets(Tree, TokenTable, TokenIndex, VariableTable, FunctionTable);
    void* NewNode2 = NULL;
    void* Root = NewNode1;

    while(TokenTable->TokenArray[*TokenIndex].TokenType == LONG_TOKEN)
    {
        if(strncmp(TokenTable->TokenArray[*TokenIndex].TokenData.LongTokenName, KeyWordsArray[ARIPHMETICAL_MUL_INDEX].Name, strlen(KeyWordsArray[ARIPHMETICAL_MUL_INDEX].Name)) == 0)
        {
            (*TokenIndex)++;
            NewNode2 = GetExpressionBrackets(Tree, TokenTable, TokenIndex, VariableTable, FunctionTable);
            size_t Type1 = *(size_t*)GetNodeData(Root, TYPE_FIELD_CODE, 0);
            size_t Type2 = *(size_t*)GetNodeData(NewNode2, TYPE_FIELD_CODE, 0);
            if(Type1 == INTEGER_VALUE_NODE && Type2 == INTEGER_VALUE_NODE)
            {
                int Value1 = *((int*)GetNodeData(Root, DATA_FIELD_CODE, 0));
                int Value2 = *((int*)GetNodeData(NewNode2, DATA_FIELD_CODE, 0));
                Value1 *= Value2;
                free(Root);
                Root = Tree->InitNode(Tree, INTEGER_VALUE_NODE, sizeof(Value1), &Value1, 0);
                free(NewNode2);
                continue;
            }
            else if(Type1 == FLOAT_VALUE_NODE && Type2 == FLOAT_VALUE_NODE)
            {
                double Value1 = *((double*)GetNodeData(Root, DATA_FIELD_CODE, 0));
                double Value2 = *((double*)GetNodeData(NewNode2, DATA_FIELD_CODE, 0));
                Value1 *= Value2;
                free(Root);
                Root = Tree->InitNode(Tree, FLOAT_VALUE_NODE, sizeof(Value1), &Value1, 0);
                free(NewNode2);
                continue;
            }
            Root = Tree->InitNode(Tree, ARIPHMETICAL_MUL_NODE, 0, NULL, 2, Root, NewNode2);
        }
        else if(strncmp(TokenTable->TokenArray[*TokenIndex].TokenData.LongTokenName, KeyWordsArray[ARIPHMETICAL_DIV_INDEX].Name, strlen(KeyWordsArray[ARIPHMETICAL_DIV_INDEX].Name)) == 0)
        {
            (*TokenIndex)++;
            NewNode2 = GetExpressionBrackets(Tree, TokenTable, TokenIndex, VariableTable, FunctionTable);
            size_t Type1 = *(size_t*)GetNodeData(Root, TYPE_FIELD_CODE, 0);
            size_t Type2 = *(size_t*)GetNodeData(NewNode2, TYPE_FIELD_CODE, 0);
            if(Type1 == INTEGER_VALUE_NODE && Type2 == INTEGER_VALUE_NODE)
            {
                int Value1 = *((int*)GetNodeData(Root, DATA_FIELD_CODE, 0));
                int Value2 = *((int*)GetNodeData(NewNode2, DATA_FIELD_CODE, 0));

                assert(Value2);

                Value1 /= Value2;
                free(Root);
                Root = Tree->InitNode(Tree, INTEGER_VALUE_NODE, sizeof(Value1), &Value1, 0);
                free(NewNode2);
                continue;
            }
            else if(Type1 == FLOAT_VALUE_NODE && Type2 == FLOAT_VALUE_NODE)
            {
                double Value1 = *((double*)GetNodeData(Root, DATA_FIELD_CODE, 0));
                double Value2 = *((double*)GetNodeData(NewNode2, DATA_FIELD_CODE, 0));
                
                assert((int)Value2); // FIXME

                Value1 /= Value2;
                free(Root);
                Root = Tree->InitNode(Tree, FLOAT_VALUE_NODE, sizeof(Value1), &Value1, 0);
                free(NewNode2);
                continue;
            }
            Root = Tree->InitNode(Tree, ARIPHMETICAL_DIV_NODE, 0, NULL, 2, Root, NewNode2);
        }
        else
        {
            return Root;
        }
    }
    
    return Root;
}

static void* GetExpressionBrackets(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, VariableTable_t* VariableTable, FunctionTable_t* FunctionTable)
{
    assert(Tree);
    assert(TokenTable);
    assert(TokenIndex);
    assert(VariableTable);
    assert(FunctionTable);

    if((TokenTable->TokenArray[*TokenIndex].TokenType == PARENTHESES_OPEN_TOKEN))
    {
        (*TokenIndex)++;
        void* NewNode = GetExpression(Tree, TokenTable, TokenIndex, VariableTable, FunctionTable);

        if(TokenTable->TokenArray[*TokenIndex].TokenType == PARENTHESES_CLOSE_TOKEN)
        {
            (*TokenIndex)++;
            return NewNode;
        }
        else
        {
            return NULL;
        }
    }
    else
    {
        return GetRightVariable(Tree, TokenTable, TokenIndex, VariableTable, FunctionTable);
    }

}

static void* GetRightVariable(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, VariableTable_t* VariableTable, FunctionTable_t* FunctionTable)
{
    assert(Tree);
    assert(TokenTable);
    assert(TokenIndex);
    assert(VariableTable);
    assert(FunctionTable);

    if(TokenTable->TokenArray[*TokenIndex].TokenType == LONG_TOKEN)
    {
        char VariableName[MAX_TOKEN_LENGTH] = {};
        strcpy(VariableName, TokenTable->TokenArray[*TokenIndex].TokenData.LongTokenName);
        void* NewNode = Tree->InitNode(Tree, RIGHT_VARIABLE_NODE, strlen(VariableName), VariableName, 0);
        (*TokenIndex)++;
        return NewNode;
    }
    else
    {
        return GetValue(Tree, TokenTable, TokenIndex, VariableTable, FunctionTable);
    }
}

static void* GetValue(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, VariableTable_t* VariableTable, FunctionTable_t* FunctionTable)
{
    assert(Tree);
    assert(TokenTable);
    assert(TokenIndex);
    assert(VariableTable);
    assert(FunctionTable);

    size_t OldTokenIndex = *TokenIndex;
    (*TokenIndex)++;
    if(TokenTable->TokenArray[OldTokenIndex].TokenType == INTEGER_CONST_TOKEN)
    {
        return Tree->InitNode(Tree, INTEGER_VALUE_NODE, sizeof(TokenTable->TokenArray[OldTokenIndex].TokenData.IntegerValue), 
                              &TokenTable->TokenArray[OldTokenIndex].TokenData.IntegerValue, 0);
    }
    else if(TokenTable->TokenArray[OldTokenIndex].TokenType == FLOAT_CONST_TOKEN)
    {
        return Tree->InitNode(Tree, FLOAT_VALUE_NODE, sizeof(TokenTable->TokenArray[OldTokenIndex].TokenData.FloatingPointValue), 
                              &TokenTable->TokenArray[OldTokenIndex].TokenData.FloatingPointValue, 0);
    } // WYS
    else
    {
        return NULL;
    }
}

// ARIPHMETICO-LOGICAL SECTION END