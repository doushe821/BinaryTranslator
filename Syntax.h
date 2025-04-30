#ifndef SYNTAX_H_INCLUDED
#define SYNTAX_H_INCLUDED

#include <stdlib.h>

// Syntax rules of MeowLang

const size_t KeyWordMaxLength = 512;
const size_t NumberOfKeyWords = 32;

enum KeyWordsCodes
{
    STATEMENT_SEPARATOR_CODE = ';',
    ARG_SEPARATOR_CODE = ',',
    ASSIGNMENT_CODE = '=',
    PARENTHESES_OPEN_CODE = '(',
    PARENTHESES_CLOSE_CODE = ')',
    CURLY_OPEN_CODE = '{',
    CURLY_CLOSE_CODE = '}',
    NO_CODE = 0,
};

enum KeyWordsTableIndexes // TODO Remove assignments later
{
    STATEMENT_SEPARATOR_INDEX,
    ARG_SEPARATOR_INDEX,
    IF_INDEX,
    WHILE_INDEX,

    STATEMENT_OPERAND_SECTION_START,
    ASSIGNMENT_INDEX,
    EQUALITY_INDEX ,
    STATEMENT_OPERAND_SECTION_END,

    PARENTHESES_OPEN_INDEX,
    PARENTHESES_CLOSE_INDEX,
    CURLY_OPEN_INDEX,
    CURLY_CLOSE_INDEX,
    ARIPHMETICAL_SUM,
    ARIPHMETICAL_SUB,
    ARIPHMETICAL_MUL,
    ARIPHMETICAL_DIV,
    LOGICAL_AND,
    LOGICAL_OR,
    LOGICAL_XOR,

    TYPE_SECTION_START,
    INT_TYPE,
    FLOAT_TYPE,
    TYPE_SECTION_END,
    
};

struct KeyWord
{
    char Name[KeyWordMaxLength];
    int Code;
};

__attribute((unused)) static KeyWord KeyWordsArray[NumberOfKeyWords] = 
{
    {";"        , STATEMENT_SEPARATOR_CODE       },
    {","        , ARG_SEPARATOR_CODE             },
    {"if"       , NO_CODE                        },
    {"while"    , NO_CODE                        },

    {""         , STATEMENT_OPERAND_SECTION_START},
    {"="        , ASSIGNMENT_CODE                },
    {"=="       , NO_CODE                        },
    {""         , STATEMENT_OPERAND_SECTION_END  },

    {"("        , PARENTHESES_OPEN_CODE          },
    {")"        , PARENTHESES_CLOSE_CODE         },
    {"{"        , CURLY_OPEN_CODE                },
    {"}"        , CURLY_CLOSE_CODE               },
    {"+"        , NO_CODE                        },
    {"-"        , NO_CODE                        },
    {"*"        , NO_CODE                        },
    {"/"        , NO_CODE                        },
    {"&"        , NO_CODE                        },
    {"|"        , NO_CODE                        },
    {"^"        , NO_CODE                        },

    {""         , TYPE_SECTION_START             },
    {"int"      , NO_CODE                        },
    {"float"    , NO_CODE                        },
    {""         , TYPE_SECTION_END               },

};


#endif