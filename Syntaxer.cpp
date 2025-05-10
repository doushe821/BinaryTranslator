#include "Syntaxer.h"

#include <assert.h>
#include "List/List.h"

static void* GetType(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, List_t* VariableTablesList, FunctionTable_t* FunctionTable);
static void* GetVariableDeclaration(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, List_t* VariableTablesList, FunctionTable_t* FunctionTable);

static void* GetFunctionBody(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, List_t* VariableTablesList, FunctionTable_t* FunctionTable);
static void* GetFunctionBodyArguments(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, List_t* VariableTablesList, FunctionTable_t* FunctionTable);

static void* GetScope(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, List_t* VariableTablesList, FunctionTable_t* FunctionTable);
static void* GetStatement(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, List_t* VariableTablesList, FunctionTable_t* FunctionTable);
static void* GetReturn(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, List_t* VariableTablesList, FunctionTable_t* FunctionTable);
static void* GetConditionalCycle(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, List_t* VariableTablesList, FunctionTable_t* FunctionTable);
static void* GetConditionArguments(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, List_t* VariableTablesList, FunctionTable_t* FunctionTable);
static void* GetStatementOperand(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, List_t* VariableTablesList, FunctionTable_t* FunctionTable);
static void* GetLeftVariable(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, List_t* VariableTablesList, FunctionTable_t* FunctionTable);

static size_t FunctionTableAppend(FunctionTable_t* FunctionTable, Function_t Function);

static size_t SearchType(const TokenTable_t* TokenTable, size_t* TokenIndex);

static int FunctionSearch(const FunctionTable_t* FunctionTable, const TokenTable_t* TokenTable, size_t* TokenIndex);

static int VariableAppend(List_t* VariableTablesList, Variable_t Variable);
static int VariableTableSearch(VariableTable_t* VariableTable, Variable_t Variable);
static void* GetFunctionCall(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, List_t* VariableTablesList, FunctionTable_t* FunctionTable);
static void* GetExpression(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, List_t* VariableTablesList, FunctionTable_t* FunctionTable);
static void* GetLogicalXor(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, List_t* VariableTablesList, FunctionTable_t* FunctionTable);
static void* GetLogicalOr(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, List_t* VariableTablesList, FunctionTable_t* FunctionTable);
static void* GetLogicalAnd(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, List_t* VariableTablesList, FunctionTable_t* FunctionTable);
static void* GetAriphmeticalSum(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, List_t* VariableTablesList, FunctionTable_t* FunctionTable);
static void* GetAriphmeticalMul(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, List_t* VariableTablesList, FunctionTable_t* FunctionTable);
static void* GetComparisonOperand(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, List_t* VariableTablesList, FunctionTable_t* FunctionTable);
static size_t SearchComparisonOperand(const TokenTable_t* TokenTable, size_t* TokenIndex);
static void* GetExpressionBrackets(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, List_t* VariableTablesList, FunctionTable_t* FunctionTable);
static void* GetRightVariable(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, List_t* VariableTablesList, FunctionTable_t* FunctionTable);
static void* GetValue(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, List_t* VariableTablesList, FunctionTable_t* FunctionTable);

static int VariableTableListSearch(List_t* VariableTablesList, Variable_t Variable);
static int GetAbsoluteVariableIndexShift(List_t* VariableTablesList);

static size_t ScopeStatementCount(const TokenTable_t* TokenTable, size_t TokenIndex, size_t* StatementCounter);


int GetProgram(Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, List_t* VariableTablesList, FunctionTable_t* FunctionTable)
{
    assert(Tree);
    assert(TokenTable);
    assert(TokenIndex);
    assert(VariableTablesList);
    assert(FunctionTable);

    const size_t InitProgramListSize = 128;
    List_t* ProgramList = NULL;
    ListInit(&ProgramList, InitProgramListSize, sizeof(void*), 0);

    assert(ProgramList);

    VariableTable_t* GlobalVariableTable = (VariableTable_t*)calloc(1, sizeof(VariableTable_t));
    assert(GlobalVariableTable);
    GlobalVariableTable->Capacity = InitalArraySize;
    GlobalVariableTable->VariablesArray = (Variable_t*)calloc(InitalArraySize, sizeof(Variable_t));
    assert(GlobalVariableTable->VariablesArray);
    PushFront(VariableTablesList, &GlobalVariableTable);

    while(*TokenIndex < TokenTable->Free)
    {
        void* Branch = GetType(Tree, TokenTable, TokenIndex, VariableTablesList, FunctionTable);
        PushFront(ProgramList, &Branch); 
    }
    
    size_t ProgramListLength = GetLinearListSize(ProgramList) / ProgramList->elsize;
    fprintf(stderr, "Program list length = %zu\n", ProgramListLength);
    Tree->root = Tree->InitNode(Tree, PROGRAM_NODE, sizeof(ProgramListLength), &ProgramListLength, ProgramListLength);

    for(size_t i = 0; i < ProgramListLength; i++)
    {
        AddDescendant(Tree->root, (void*)(*(size_t*)ListGetNodeValueInd(ProgramList, i + 1)), i);
    }

    ListDstr(ProgramList);

    return 0;
}

static void* GetType(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, List_t* VariableTablesList, FunctionTable_t* FunctionTable)
{
    assert(Tree);
    assert(TokenTable);
    assert(TokenIndex);
    assert(VariableTablesList);
    assert(FunctionTable);
    
    if(TokenTable->TokenArray[*TokenIndex].TokenType == LONG_TOKEN)
    {
        for(size_t i = 1; i < TYPE_SECTION_END - TYPE_SECTION_START; i++)
        {
            if(strncmp(TokenTable->TokenArray[*TokenIndex].TokenData.LongTokenName, KeyWordsArray[TYPE_SECTION_START + i].Name, strlen(KeyWordsArray[TYPE_SECTION_END + i].Name)) == 0)
            {
                TokenTable->TokenArray[*TokenIndex].AdditionalData = TYPE_SECTION_START + (int)i;
                (*TokenIndex)++;
                return GetVariableDeclaration(Tree, TokenTable, TokenIndex, VariableTablesList, FunctionTable); // Statement with variable or function
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

static void* GetVariableDeclaration(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, List_t* VariableTablesList, FunctionTable_t* FunctionTable)
{
    assert(Tree);
    assert(TokenTable);
    assert(TokenIndex);
    assert(VariableTablesList);
    assert(FunctionTable);

    size_t TokenType = TokenTable->TokenArray[*TokenIndex].TokenType;
    //size_t Type = (size_t)TokenTable->TokenArray[*TokenIndex - 1].AdditionalData;

    if(TokenType == LONG_TOKEN)
    {
        if(TokenTable->TokenArray[*TokenIndex + 1].TokenType == PARENTHESES_OPEN_TOKEN)
        {
            return GetFunctionBody(Tree, TokenTable, TokenIndex, VariableTablesList, FunctionTable);
        }
        else
        {
            return GetLeftVariable(Tree, TokenTable, TokenIndex, VariableTablesList, FunctionTable);
        }
    }
    else 
    {
        assert(0);
        return NULL;
    }
    return NULL;
}

static void* GetFunctionBody(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, List_t* VariableTablesList, FunctionTable_t* FunctionTable)
{
    assert(Tree);
    assert(TokenTable);
    assert(TokenIndex);
    assert(VariableTablesList);
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

        (*TokenIndex)++;

        VariableTable_t* VariableTable = (VariableTable_t*)calloc(1, sizeof(VariableTable_t));
        assert(VariableTable);
        VariableTable->Capacity = InitalArraySize;
        VariableTable->VariablesArray = (Variable_t*)calloc(InitalArraySize, sizeof(Variable_t));
        assert(VariableTable->VariablesArray);
        PushFront(VariableTablesList, &VariableTable);

        void* Arguments = GetFunctionBodyArguments(Tree, TokenTable, TokenIndex, VariableTablesList, FunctionTable);

        Function.NumberOfArguments = *(size_t*)GetNodeData(Arguments, DATA_FIELD_CODE, 0);
        size_t FunctionIndex = FunctionTableAppend(FunctionTable, Function);

        assert(TokenTable->TokenArray[*TokenIndex].TokenType == PARENTHESES_CLOSE_TOKEN);

        (*TokenIndex)++;
        
        void* NewBranch = Tree->InitNode(Tree, FUNCTION_BODY_NODE, sizeof(FunctionIndex), &FunctionIndex, 2, GetScope(Tree, TokenTable, TokenIndex, VariableTablesList, FunctionTable), Arguments);

        VariableTable_t* RemovedTable = NULL;
        RemFront(VariableTablesList, &RemovedTable);
        free(RemovedTable->VariablesArray);
        free(RemovedTable);

        return NewBranch;
    }

    else
    {
        assert(0);
    }
    
} // ADD ARGUMENTS

static void* GetFunctionBodyArguments(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, List_t* VariableTablesList, FunctionTable_t* FunctionTable)
{
    assert(Tree);
    assert(TokenTable);
    assert(TokenIndex);
    assert(VariableTablesList);
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

        size_t TableIndex = (size_t)VariableAppend(VariableTablesList, Argument);

        void* VariableNodeData = calloc(sizeof(TableIndex) + VARIABLE_NAME_MAX, 1);
        assert(VariableNodeData);
        memcpy(VariableNodeData, &TableIndex, sizeof(TableIndex));
        memcpy((char*)VariableNodeData + sizeof(TableIndex), Argument.Name, VARIABLE_NAME_MAX);

        void* ArgumentNode = Tree->InitNode(Tree, LEFT_VARIABLE_NODE, sizeof(TableIndex) + VARIABLE_NAME_MAX, VariableNodeData, 0);

        free(VariableNodeData);

        AddDescendant(Arguments, ArgumentNode, i);

        (*TokenIndex) += 2;
    }
    (*TokenIndex)--;
    return Arguments;
}

static void* GetScope(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, List_t* VariableTablesList, FunctionTable_t* FunctionTable)
{
    assert(Tree);
    assert(TokenTable);
    assert(TokenIndex);
    assert(VariableTablesList);
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
            AddDescendant(ScopeNode, GetStatement(Tree, TokenTable, TokenIndex, VariableTablesList, FunctionTable), i);
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
        fprintf(stderr, "%zu\n", *TokenIndex);
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

static void* GetStatement(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, List_t* VariableTablesList, FunctionTable_t* FunctionTable)
{
    assert(Tree);
    assert(TokenTable);
    assert(TokenIndex);
    assert(VariableTablesList);
    assert(FunctionTable);

    for(size_t i = 1; i < CONDITIONAL_CYCLES_SECTION_END - CONDITIONAL_CYCLES_SECTION_START; i++)
    {
        if(strncmp(TokenTable->TokenArray[*TokenIndex].TokenData.LongTokenName, KeyWordsArray[CONDITIONAL_CYCLES_SECTION_START + i].Name, strlen(KeyWordsArray[CONDITIONAL_CYCLES_SECTION_START + i].Name)) == 0)
        {
            TokenTable->TokenArray[*TokenIndex].AdditionalData = CONDITIONAL_CYCLES_SECTION_START + (int)i;
            return GetConditionalCycle(Tree, TokenTable, TokenIndex, VariableTablesList, FunctionTable);
        }
    }
    
    int FunctionIndex = FunctionSearch(FunctionTable, TokenTable, TokenIndex);
    if(FunctionIndex != NOT_FOUND)
    {
        TokenTable->TokenArray[*TokenIndex].AdditionalData = FunctionIndex;
        void* FunctionCallNode = GetFunctionCall(Tree, TokenTable, TokenIndex, VariableTablesList, FunctionTable);
        assert(FunctionCallNode);
        if(TokenTable->TokenArray[*TokenIndex].TokenType == STATEMENT_SEPARATOR_TOKEN)
        {
            (*TokenIndex)++;
            return FunctionCallNode;
        }
        else
        {
            assert(0);
        }
    }

    if(strncmp(TokenTable->TokenArray[*TokenIndex].TokenData.LongTokenName, KeyWordsArray[RETURN_INDEX].Name, strlen(KeyWordsArray[RETURN_INDEX].Name)) == 0)
    {   
        fprintf(stderr, "sex");
        void* ReturnNode = GetReturn(Tree, TokenTable, TokenIndex, VariableTablesList, FunctionTable);
        if(TokenTable->TokenArray[*TokenIndex].TokenType == STATEMENT_SEPARATOR_TOKEN)
        {
            (*TokenIndex)++;
        }
        else
        {
            assert(0 && "Sperma");
        }
        return ReturnNode;
    }

    void* LeftPart = GetLeftVariable(Tree, TokenTable, TokenIndex, VariableTablesList, FunctionTable);
    void* OperandNode = GetStatementOperand(Tree, TokenTable, TokenIndex, VariableTablesList, FunctionTable);

    AddDescendant(OperandNode, LeftPart, 0);
    AddDescendant(OperandNode, GetExpression(Tree, TokenTable, TokenIndex, VariableTablesList, FunctionTable), 1);

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

static void* GetReturn(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, List_t* VariableTablesList, FunctionTable_t* FunctionTable)
{
    assert(Tree);
    assert(TokenTable);
    assert(TokenIndex);
    assert(VariableTablesList);
    assert(FunctionTable);
    (*TokenIndex)++;
    return Tree->InitNode(Tree, RETURN_NODE, 0, NULL, 1, GetExpression(Tree, TokenTable, TokenIndex, VariableTablesList, FunctionTable));
}

static void* GetFunctionCall(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, List_t* VariableTablesList, FunctionTable_t* FunctionTable)
{
    assert(Tree);
    assert(TokenTable);
    assert(TokenIndex);
    assert(VariableTablesList);
    assert(FunctionTable);

    int TableIndex = TokenTable->TokenArray[*TokenIndex].AdditionalData;

    (*TokenIndex)++;
    if(TokenTable->TokenArray[*TokenIndex].TokenType != PARENTHESES_OPEN_TOKEN)
    {
        assert(0);
    }

    void* FunctionCallNode = Tree->InitNode(Tree, FUNCTION_CALL_NODE, sizeof(TableIndex), &TableIndex, FunctionTable->FunctionsArray[TableIndex].NumberOfArguments);
    (*TokenIndex)++;
    for(size_t i = 0; i < FunctionTable->FunctionsArray[TableIndex].NumberOfArguments; i++)
    {
        AddDescendant(FunctionCallNode, GetExpression(Tree, TokenTable, TokenIndex, VariableTablesList, FunctionTable), i);
        if(TokenTable->TokenArray[*TokenIndex].TokenType == ARG_SEPARATOR_TOKEN)
        {
            (*TokenIndex)++;
        }
    }
    (*TokenIndex)++;
    return FunctionCallNode;
}

static void* GetConditionalCycle(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, List_t* VariableTablesList, FunctionTable_t* FunctionTable)
{
    assert(Tree);
    assert(TokenTable);
    assert(TokenIndex);
    assert(VariableTablesList);
    assert(FunctionTable);

    void* ConditionNode = Tree->InitNode(Tree, CONDITION_NODE, sizeof(TokenTable->TokenArray[*TokenIndex].AdditionalData), &TokenTable->TokenArray[*TokenIndex].AdditionalData, 2);

    (*TokenIndex)++;

    if(TokenTable->TokenArray[*TokenIndex].TokenType != PARENTHESES_OPEN_TOKEN)
    {
        assert(0);
    }

    (*TokenIndex)++;

    void* Condition =  GetConditionArguments(Tree, TokenTable, TokenIndex, VariableTablesList, FunctionTable);

    assert(Condition);
    assert(GetNodeData(Condition, DESCENDANTS_FIELD_CODE, 0));

    AddDescendant(ConditionNode, Condition, 1);

    if(TokenTable->TokenArray[*TokenIndex].TokenType == PARENTHESES_CLOSE_TOKEN)
    {
        (*TokenIndex)++;
    }

    VariableTable_t* VariableTable = (VariableTable_t*)calloc(1, sizeof(VariableTable_t));
    assert(VariableTable);
    VariableTable->Capacity = InitalArraySize;
    VariableTable->VariablesArray = (Variable_t*)calloc(InitalArraySize, sizeof(Variable_t));
    assert(VariableTable->VariablesArray);
    PushFront(VariableTablesList, &VariableTable);

    AddDescendant(ConditionNode, GetScope(Tree, TokenTable, TokenIndex, VariableTablesList, FunctionTable), 0);

    return ConditionNode;
}

static int FunctionSearch(const FunctionTable_t* FunctionTable, const TokenTable_t* TokenTable, size_t* TokenIndex)
{
    for(int i = 0; i < (int)FunctionTable->Free; i++)
    {
        if(strncmp(FunctionTable->FunctionsArray[i].Name, TokenTable->TokenArray[*TokenIndex].TokenData.LongTokenName, strlen(FunctionTable->FunctionsArray[i].Name)) == 0)
        {
            return i;
        }
    }
    return NOT_FOUND;
}

static void* GetConditionArguments(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, List_t* VariableTablesList, FunctionTable_t* FunctionTable)
{
    assert(Tree);
    assert(TokenTable);
    assert(TokenIndex);
    assert(VariableTablesList);
    assert(FunctionTable);

    return GetExpression(Tree, TokenTable, TokenIndex, VariableTablesList, FunctionTable);
}

static void* GetStatementOperand(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, List_t* VariableTablesList, FunctionTable_t* FunctionTable)
{
    assert(Tree);
    assert(TokenTable);
    assert(TokenIndex);
    assert(VariableTablesList);
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
        fprintf(stderr, "%s\n", TokenTable->TokenArray[*TokenIndex].TokenData.LongTokenName);
        assert(0);
    }
    else
    {
        assert(0);
    }
    
}

static void* GetLeftVariable(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, List_t* VariableTablesList, FunctionTable_t* FunctionTable)
{
    assert(Tree);
    assert(TokenTable);
    assert(TokenIndex);
    assert(VariableTablesList);
    assert(FunctionTable);

    size_t TokenType = TokenTable->TokenArray[*TokenIndex].TokenType;
    if(TokenType == LONG_TOKEN)
    {
        size_t VariableType = SearchType(TokenTable, TokenIndex) != 0;
        if(VariableType != 0)
        {
            (*TokenIndex)++;
            Variable_t LeftVariable = {};
            strncpy(LeftVariable.Name, TokenTable->TokenArray[*TokenIndex].TokenData.LongTokenName, strlen(TokenTable->TokenArray[*TokenIndex].TokenData.LongTokenName));
            LeftVariable.Type = VariableType;
            fprintf(stderr, "GAY\n");
            int VariableIndex = VariableAppend(VariableTablesList, LeftVariable);
    
            (*TokenIndex)++;
    
            void* VariableNodeData = calloc(sizeof(VariableIndex) + VARIABLE_NAME_MAX, 1);
            assert(VariableNodeData);
            memcpy(VariableNodeData, &VariableIndex, sizeof(VariableIndex));
            memcpy((char*)VariableNodeData + sizeof(VariableIndex), LeftVariable.Name, VARIABLE_NAME_MAX);

            void* NewNode = Tree->InitNode(Tree, LEFT_VARIABLE_NODE, sizeof(VariableIndex) + VARIABLE_NAME_MAX, VariableNodeData, 0);

            free(VariableNodeData);
            return NewNode;
        }
        else
        {
            Variable_t LeftVariable = {};
            strncpy(LeftVariable.Name, TokenTable->TokenArray[*TokenIndex].TokenData.LongTokenName, strlen(TokenTable->TokenArray[*TokenIndex].TokenData.LongTokenName));

            int VariableIndex = VariableTableListSearch(VariableTablesList, LeftVariable);

            if(VariableIndex == -1)
            {
                fprintf(stderr, "Filthy undefined variable, BE GONE!\n");
                assert(0);
            }
            (*TokenIndex)++;
        
            void* VariableNodeData = calloc(sizeof(VariableIndex) + VARIABLE_NAME_MAX, 1);
            assert(VariableNodeData);
            memcpy(VariableNodeData, &VariableIndex, sizeof(VariableIndex));
            memcpy((char*)VariableNodeData + sizeof(VariableIndex), LeftVariable.Name, VARIABLE_NAME_MAX);

            void* NewNode = Tree->InitNode(Tree, LEFT_VARIABLE_NODE, sizeof(VariableIndex) + VARIABLE_NAME_MAX, VariableNodeData, 0);

            free(VariableNodeData);
            return NewNode;
        }
    }
    else
    {
        assert(0 && "Ya sosu chlen");
    }
}

static size_t SearchType(const TokenTable_t* TokenTable, size_t* TokenIndex)
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


static int VariableAppend(List_t* VariableTablesList, Variable_t Variable)
{
    assert(VariableTablesList);

    VariableTable_t* VariableTable = NULL;
    memcpy(&VariableTable, ListGetNodeValueInd(VariableTablesList, VariableTablesList->free - 1), sizeof(void*));

    int VariableIndex = VariableTableSearch(VariableTable, Variable);
    
    if(VariableIndex >= 0)
    {
        fprintf(stderr, "SIKE, can't redefine in the same scope!\n");
        fprintf(stderr, "%s\n", Variable.Name);
        assert(0);
    }

    fprintf(stderr, "Free = %zu\nCapacity = %zu\n", VariableTable->Free, VariableTable->Capacity);
    if(VariableTable->Free >= VariableTable->Capacity)
    {
        VariableTable->VariablesArray = (Variable_t*)realloc(VariableTable->VariablesArray, VariableTable->Capacity * ArrayResizeMultiplier * sizeof(Variable_t));
        VariableTable->Capacity *= ArrayResizeMultiplier;
        assert(VariableTable->VariablesArray);
    }

    VariableTable->VariablesArray[VariableTable->Free] = Variable;
    (VariableTable->Free)++;

    int IndexShift = GetAbsoluteVariableIndexShift(VariableTablesList);

    return IndexShift + (int)VariableTable->Free - 1;
}

static int VariableTableListSearch(List_t* VariableTablesList, Variable_t Variable)
{
    int Shift = GetAbsoluteVariableIndexShift(VariableTablesList);
    int Index = Shift;

    for(size_t i = VariableTablesList->free - 1; i > 0; i--)
    {
        VariableTable_t* VariableTable = NULL;
        memcpy(&VariableTable, ListGetNodeValueInd(VariableTablesList, i), sizeof(void*));
        int LocalIndex = VariableTableSearch(VariableTable, Variable);
        if(LocalIndex >= 0)
        {
            return Index + LocalIndex;
        }
        Index -= (int)VariableTable->Free -  1;
    }    
    return -1;
}

static int VariableTableSearch(VariableTable_t* VariableTable, Variable_t Variable)
{
    for(size_t i = 0; i < VariableTable->Free; i++)
    {
        if(strncmp(VariableTable->VariablesArray[i].Name, Variable.Name, strlen(VariableTable->VariablesArray[i].Name)) == 0)
        {
            return (int)i;
        }
    }
    return -1;
}

static int GetAbsoluteVariableIndexShift(List_t* VariableTablesList)
{
    assert(VariableTablesList);

    int VariableCounter = 0;

    for(size_t i = 1; i < VariableTablesList->free - 1; i++)
    {
        VariableTable_t* VariableTable = NULL;
        memcpy(&VariableTable, ListGetNodeValueInd(VariableTablesList, i), sizeof(void*));
        fprintf(stderr, "kkk = %zu\n", VariableTable->Free);
        VariableCounter += (int)VariableTable->Free;
    }   

    fprintf(stderr, "%d\n", VariableCounter);
    return VariableCounter;
}


//
// ARIPHMETICO-LOGICAL EXPRESSIONS SECTION START
//


static void* GetExpression(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, List_t* VariableTablesList, FunctionTable_t* FunctionTable)
{
    assert(Tree);
    assert(TokenTable);
    assert(TokenIndex);
    assert(VariableTablesList);
    assert(FunctionTable);

    void* NewNode = GetLogicalXor(Tree, TokenTable, TokenIndex, VariableTablesList, FunctionTable);
    return NewNode;
}

static void* GetLogicalXor(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, List_t* VariableTablesList, FunctionTable_t* FunctionTable)
{
    assert(Tree);
    assert(TokenTable);
    assert(TokenIndex);
    assert(VariableTablesList);
    assert(FunctionTable);

    void* NewNode1 = GetLogicalOr(Tree, TokenTable, TokenIndex, VariableTablesList, FunctionTable);
    void* NewNode2 = NULL;
    void* Root = NewNode1;

    while(TokenTable->TokenArray[*TokenIndex].TokenType == LONG_TOKEN)
    {
        if(strncmp(TokenTable->TokenArray[*TokenIndex].TokenData.LongTokenName, KeyWordsArray[LOGICAL_XOR_INDEX].Name, strlen(KeyWordsArray[LOGICAL_XOR_INDEX].Name)) == 0)
        {
            (*TokenIndex)++;
            NewNode2 = GetLogicalOr(Tree, TokenTable, TokenIndex, VariableTablesList, FunctionTable);
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
            int OperationCode = LOGICAL_XOR_INDEX;
            Root = Tree->InitNode(Tree, OPERATION_NODE, sizeof(OperationCode), &OperationCode, 2, Root, NewNode2);
        }
        else
        {
            return Root;
        }
    }

    return Root;
}

static void* GetLogicalOr(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, List_t* VariableTablesList, FunctionTable_t* FunctionTable)
{
    assert(Tree);
    assert(TokenTable);
    assert(TokenIndex);
    assert(VariableTablesList);
    assert(FunctionTable);

    void* NewNode1 = GetLogicalAnd(Tree, TokenTable, TokenIndex, VariableTablesList, FunctionTable);
    void* NewNode2 = NULL;
    void* Root = NewNode1;

    while(TokenTable->TokenArray[*TokenIndex].TokenType == LONG_TOKEN)
    {
        if(strncmp(TokenTable->TokenArray[*TokenIndex].TokenData.LongTokenName, KeyWordsArray[LOGICAL_OR_INDEX].Name, strlen(KeyWordsArray[LOGICAL_OR_INDEX].Name)) == 0)
        {
            (*TokenIndex)++;
            NewNode2 = GetLogicalAnd(Tree, TokenTable, TokenIndex, VariableTablesList, FunctionTable);
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
            int OperationCode = LOGICAL_OR_INDEX;
            Root = Tree->InitNode(Tree, OPERATION_NODE, sizeof(OperationCode), &OperationCode, 2, Root, NewNode2);
        }
        else
        {
            return Root;
        }
    }

    return Root;
}

static void* GetLogicalAnd(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, List_t* VariableTablesList, FunctionTable_t* FunctionTable)
{
    assert(Tree);
    assert(TokenTable);
    assert(TokenIndex);
    assert(VariableTablesList);
    assert(FunctionTable);

    void* NewNode1 = GetAriphmeticalSum(Tree, TokenTable, TokenIndex, VariableTablesList, FunctionTable);
    void* NewNode2 = NULL;
    void* Root = NewNode1;

    while(TokenTable->TokenArray[*TokenIndex].TokenType == LONG_TOKEN)
    {
        if(strncmp(TokenTable->TokenArray[*TokenIndex].TokenData.LongTokenName, KeyWordsArray[LOGICAL_AND_INDEX].Name, strlen(KeyWordsArray[LOGICAL_AND_INDEX].Name)) == 0)
        {
            (*TokenIndex)++;
            NewNode2 = GetAriphmeticalMul(Tree, TokenTable, TokenIndex, VariableTablesList, FunctionTable);
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
            int OperationCode = LOGICAL_AND_INDEX;
            Root = Tree->InitNode(Tree, OPERATION_NODE, sizeof(OperationCode), &OperationCode, 2, Root, NewNode2);
        }
        else
        {
            return Root;
        }
    }

    return Root;
}

static void* GetAriphmeticalSum(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, List_t* VariableTablesList, FunctionTable_t* FunctionTable)
{
    assert(Tree);
    assert(TokenTable);
    assert(TokenIndex);
    assert(VariableTablesList);
    assert(FunctionTable);

    void* NewNode1 = GetAriphmeticalMul(Tree, TokenTable, TokenIndex, VariableTablesList, FunctionTable);
    void* NewNode2 = NULL;
    void* Root = NewNode1;

    while(TokenTable->TokenArray[*TokenIndex].TokenType == LONG_TOKEN)
    {
        if(strncmp(TokenTable->TokenArray[*TokenIndex].TokenData.LongTokenName, KeyWordsArray[ARIPHMETICAL_SUM_INDEX].Name, strlen(KeyWordsArray[ARIPHMETICAL_SUM_INDEX].Name)) == 0)
        {
            (*TokenIndex)++;
            NewNode2 = GetAriphmeticalMul(Tree, TokenTable, TokenIndex, VariableTablesList, FunctionTable);
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
            int OperationCode = ARIPHMETICAL_SUM_INDEX;
            Root = Tree->InitNode(Tree, OPERATION_NODE, sizeof(OperationCode), &OperationCode, 2, Root, NewNode2);
        }
        else if(strncmp(TokenTable->TokenArray[*TokenIndex].TokenData.LongTokenName, KeyWordsArray[ARIPHMETICAL_SUB_INDEX].Name, strlen(KeyWordsArray[ARIPHMETICAL_SUB_INDEX].Name)) == 0)
        {
            (*TokenIndex)++;
            NewNode2 = GetAriphmeticalMul(Tree, TokenTable, TokenIndex, VariableTablesList, FunctionTable);
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
            int OperationCode = ARIPHMETICAL_SUB_INDEX;
            Root = Tree->InitNode(Tree, OPERATION_NODE, sizeof(OperationCode), &OperationCode, 2, Root, NewNode2);
        }
        else
        {
            return Root;
        }
    }
    
    return Root;
}

static void* GetAriphmeticalMul(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, List_t* VariableTablesList, FunctionTable_t* FunctionTable)
{
    assert(Tree);
    assert(TokenTable);
    assert(TokenIndex);
    assert(VariableTablesList);
    assert(FunctionTable);

    void* NewNode1 = GetComparisonOperand(Tree, TokenTable, TokenIndex, VariableTablesList, FunctionTable);
    void* NewNode2 = NULL;
    void* Root = NewNode1;

    while(TokenTable->TokenArray[*TokenIndex].TokenType == LONG_TOKEN)
    {
        if(strncmp(TokenTable->TokenArray[*TokenIndex].TokenData.LongTokenName, KeyWordsArray[ARIPHMETICAL_MUL_INDEX].Name, strlen(KeyWordsArray[ARIPHMETICAL_MUL_INDEX].Name)) == 0)
        {
            (*TokenIndex)++;
            NewNode2 = GetComparisonOperand(Tree, TokenTable, TokenIndex, VariableTablesList, FunctionTable);
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
            int OperationCode = ARIPHMETICAL_MUL_INDEX;
            Root = Tree->InitNode(Tree, OPERATION_NODE, sizeof(OperationCode), &OperationCode, 2, Root, NewNode2);
        }
        else if(strncmp(TokenTable->TokenArray[*TokenIndex].TokenData.LongTokenName, KeyWordsArray[ARIPHMETICAL_DIV_INDEX].Name, strlen(KeyWordsArray[ARIPHMETICAL_DIV_INDEX].Name)) == 0)
        {
            (*TokenIndex)++;
            NewNode2 = GetComparisonOperand(Tree, TokenTable, TokenIndex, VariableTablesList, FunctionTable);
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
            int OperationCode = ARIPHMETICAL_DIV_INDEX;
            Root = Tree->InitNode(Tree, OPERATION_NODE, sizeof(OperationCode), &OperationCode, 2, Root, NewNode2);
        }
        else
        {
            return Root;
        }
    }
    
    return Root;
}

static void* GetComparisonOperand(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, List_t* VariableTablesList, FunctionTable_t* FunctionTable)
{
    assert(Tree);
    assert(TokenTable);
    assert(TokenIndex);
    assert(VariableTablesList);
    assert(FunctionTable);

    void* NewNode1 = GetExpressionBrackets(Tree, TokenTable, TokenIndex, VariableTablesList, FunctionTable);
    void* Root = NewNode1;
    void* NewNode2 = NULL;

    size_t KeyWordIndex = SearchComparisonOperand(TokenTable, TokenIndex);
    while(KeyWordIndex != 0)
    {
        (*TokenIndex)++;

        NewNode2 = GetExpressionBrackets(Tree, TokenTable, TokenIndex, VariableTablesList, FunctionTable);

        Root = Tree->InitNode(Tree, OPERATION_NODE, sizeof(KeyWordIndex), &KeyWordIndex, 2, Root, NewNode2);

        KeyWordIndex = SearchComparisonOperand(TokenTable, TokenIndex);
    }

    return Root;
}

static size_t SearchComparisonOperand(const TokenTable_t* TokenTable, size_t* TokenIndex)
{
    assert(TokenTable);
    assert(TokenIndex);

    for(size_t i = 1; i < COMPARISON_OPERANDS_SECTION_END - COMPARISON_OPERANDS_SECTION_START; i++)
    {
        if(strncmp(TokenTable->TokenArray[*TokenIndex].TokenData.LongTokenName, KeyWordsArray[COMPARISON_OPERANDS_SECTION_START + i].Name, strlen(KeyWordsArray[COMPARISON_OPERANDS_SECTION_START + i].Name)) == 0)
        {
            return COMPARISON_OPERANDS_SECTION_START + i;
        }
    }

    return 0;
}

static void* GetExpressionBrackets(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, List_t* VariableTablesList, FunctionTable_t* FunctionTable)
{
    assert(Tree);
    assert(TokenTable);
    assert(TokenIndex);
    assert(VariableTablesList);
    assert(FunctionTable);

    if((TokenTable->TokenArray[*TokenIndex].TokenType == PARENTHESES_OPEN_TOKEN))
    {
        (*TokenIndex)++;
        void* NewNode = GetExpression(Tree, TokenTable, TokenIndex, VariableTablesList, FunctionTable);

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
        return GetRightVariable(Tree, TokenTable, TokenIndex, VariableTablesList, FunctionTable);
    }

}

static void* GetRightVariable(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, List_t* VariableTablesList, FunctionTable_t* FunctionTable)
{
    assert(Tree);
    assert(TokenTable);
    assert(TokenIndex);
    assert(VariableTablesList);
    assert(FunctionTable);
    

    if(TokenTable->TokenArray[*TokenIndex].TokenType == LONG_TOKEN)
    {
        if(TokenTable->TokenArray[*TokenIndex + 1].TokenType == PARENTHESES_OPEN_TOKEN)
        {
            return GetFunctionCall(Tree, TokenTable, TokenIndex, VariableTablesList, FunctionTable);
        }
        fprintf(stderr, "Right variable on token %zu\n", *TokenIndex);
        Variable_t Variable = {};

        strncpy(Variable.Name, TokenTable->TokenArray[*TokenIndex].TokenData.LongTokenName, strlen(TokenTable->TokenArray[*TokenIndex].TokenData.LongTokenName));
        int VariableTableIndex = VariableTableListSearch(VariableTablesList, Variable);
        
        if(VariableTableIndex == NOT_FOUND)
        {
            fprintf(stderr, "Heretic! Tried to use an undefined variable! Get crucified!\n");
            fprintf(stderr, "Token number = %zu\n", *TokenIndex);
            assert(0);
        }

        void* VariableNodeData = calloc(sizeof(VariableTableIndex) + VARIABLE_NAME_MAX, 1);
        assert(VariableNodeData);
        memcpy(VariableNodeData, &VariableTableIndex, sizeof(VariableTableIndex));
        memcpy((char*)VariableNodeData + sizeof(VariableTableIndex), Variable.Name, VARIABLE_NAME_MAX);

        void* NewNode = Tree->InitNode(Tree, RIGHT_VARIABLE_NODE, sizeof(VariableTableIndex) + VARIABLE_NAME_MAX, VariableNodeData, 0);
        assert(NewNode);

        free(VariableNodeData);

        (*TokenIndex)++;
        return NewNode;
    }
    else
    {
        return GetValue(Tree, TokenTable, TokenIndex, VariableTablesList, FunctionTable);
    }
}

static void* GetValue(const Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, List_t* VariableTablesList, FunctionTable_t* FunctionTable)
{
    assert(Tree);
    assert(TokenTable);
    assert(TokenIndex);
    assert(VariableTablesList);
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