#ifndef SYNTAX_H_INCLUDED
#define SYNTAX_H_INCLUDED

#include <stdlib.h>

// Syntax rules of MeowLang

const size_t KeyWordMaxLength = 512;
const size_t NumberOfKeyWords = 64;

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

    STATEMENT_OPERAND_SECTION_START,
    ASSIGNMENT_INDEX,
    STATEMENT_OPERAND_SECTION_END,

    PARENTHESES_OPEN_INDEX,
    PARENTHESES_CLOSE_INDEX,
    CURLY_OPEN_INDEX,
    CURLY_CLOSE_INDEX,
    ARIPHMETICAL_SUM_INDEX,
    ARIPHMETICAL_SUB_INDEX,
    ARIPHMETICAL_MUL_INDEX,
    ARIPHMETICAL_DIV_INDEX,
    LOGICAL_AND_INDEX,
    LOGICAL_OR_INDEX,
    LOGICAL_XOR_INDEX,

    TYPE_SECTION_START,
    INT_TYPE_INDEX,
    FLOAT_TYPE_INDEX,
    TYPE_SECTION_END,

    CONDITIONAL_CYCLES_SECTION_START,
    IF_INDEX,
    WHILE_INDEX,
    FOR_INDEX,
    CONDITIONAL_CYCLES_SECTION_END,

    EQUALITY_INDEX,
    
};

struct KeyWord
{
    char Name[KeyWordMaxLength];
    int Code;
};

__attribute((unused)) static KeyWord KeyWordsArray[NumberOfKeyWords] = 
{
    {";"        , STATEMENT_SEPARATOR_CODE        },
    {","        , ARG_SEPARATOR_CODE              },

    {""         , STATEMENT_OPERAND_SECTION_START },
    {"="        , ASSIGNMENT_CODE                 },
    {""         , STATEMENT_OPERAND_SECTION_END   },

    {"("        , PARENTHESES_OPEN_CODE           },
    {")"        , PARENTHESES_CLOSE_CODE          },
    {"{"        , CURLY_OPEN_CODE                 },
    {"}"        , CURLY_CLOSE_CODE                },
    {"+"        , NO_CODE                         },
    {"-"        , NO_CODE                         },
    {"*"        , NO_CODE                         },
    {"/"        , NO_CODE                         },
    {"&"        , NO_CODE                         },
    {"|"        , NO_CODE                         },
    {"^"        , NO_CODE                         },

    {""         , TYPE_SECTION_START              },
    {"int"      , NO_CODE                         },
    {"float"    , NO_CODE                         },
    {""         , TYPE_SECTION_END                },

    {""         , CONDITIONAL_CYCLES_SECTION_START},
    {"if"       , NO_CODE                         },
    {"while"    , NO_CODE                         },
    {"for"      , NO_CODE                         },
    {""         , CONDITIONAL_CYCLES_SECTION_END  },

    {"<"        , NO_CODE},
    {">"        , NO_CODE},
    {"<="       , NO_CODE},
    {">="       , NO_CODE},
    {}

};


#endif