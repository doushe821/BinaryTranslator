#ifndef COMPILER_TABLES_INCLUDED
#define COMPILER_TABLES_INCLUDED

#include "List/List.h"
#include "List/ListStruct.h"


// TOKENIZER SECTION START
// Contains information about how source code is tokenized.

const size_t MAX_TOKEN_LENGTH = 512;
const size_t VARIABLE_NAME_MAX = 512;
const size_t FUNCTION_NAME_MAX = 512;


union TokenData_u
{
    double FloatingPointValue;
    int IntegerValue;
    char LongTokenName[MAX_TOKEN_LENGTH];
};

enum TokenTypes
{
    FLOAT_CONST_TOKEN,
    INTEGER_CONST_TOKEN,
    STATEMENT_SEPARATOR_TOKEN,
    ARG_SEPARATOR_TOKEN,
    PARENTHESES_OPEN_TOKEN,
    PARENTHESES_CLOSE_TOKEN,
    CURLY_OPEN_TOKEN,
    CURLY_CLOSE_TOKEN,
    LONG_TOKEN,
};

struct Token_t
{
    TokenTypes TokenType;
    TokenData_u TokenData;
    int AdditionalData;
};

struct TokenTable_t
{
    Token_t* TokenArray;
    size_t Free;
    size_t Capacity;
};

const size_t ArrayResizeMultiplier = 4;
const size_t InitalArraySize = 512;

// TOKENIZER SECTION END

// SYNTAXER SECTION START

const int NOT_FOUND = -1;

enum ATLNodeTypes
{
    INTEGER_VALUE_NODE,
    FLOAT_VALUE_NODE,
    OPERATION_NODE,
    RIGHT_VARIABLE_NODE,
    LEFT_VARIABLE_NODE,
    FUNCTION_BODY_NODE,
    SCOPE_NODE,
    STATEMENT_OPERAND_NODE,
    CONDITION_NODE,
    FUNCTION_ARGUMENTS_NODE,
    PROGRAM_NODE,
    FUNCTION_CALL_NODE,
    COMPARISON_OPERAND_NODE,
    RETURN_NODE,
};

union VariableData_u
{
    int Integer;
    double Float;
};

struct Variable_t
{
    size_t Type;
    bool Assigned;
    VariableData_u Data;
    char Name[VARIABLE_NAME_MAX];

    size_t ListIndex;
};

struct VariableTable_t
{
    Variable_t* VariablesArray;
    size_t Free;
    size_t Capacity;
};

struct Function_t
{
    size_t ReturnType;
    size_t NumberOfArguments;
    char Name[FUNCTION_NAME_MAX];
};

struct FunctionTable_t
{
    Function_t* FunctionsArray;
    size_t Free;
    size_t Capacity;
};
// SYNTAXER SECTION END


// ENGLISH lesson: 
// I hope, before I die, I will have pet a nerpa
// By the end of the semester I will have been studying in MIPT for an entire year

#endif