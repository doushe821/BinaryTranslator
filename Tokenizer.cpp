#include "Tokenizer.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include <assert.h>

static int TokenArrayAppend(TokenTable_t* TokenTable, Token_t Token);
static void TokenDumpArray(TokenTable_t* TokenTable);

TokenTable_t* Tokenize(char* Source, size_t SourceSize)
{
    if(Source == NULL)
    {
        return NULL;
    }

    if(SourceSize == 0)
    {
        return 0;
    }

    TokenTable_t* TokenTable = (TokenTable_t*)calloc(1, sizeof(TokenTable_t));

    if(TokenTable == NULL)
    {
        return NULL;
    }

    TokenTable->TokenArray = (Token_t*)calloc(InitalArraySize, sizeof(Token_t));
    TokenTable->Capacity = InitalArraySize;

    if(TokenTable->TokenArray == NULL)
    {
        free(TokenTable);
        return NULL;
    }

    for(size_t i = 0; i < SourceSize; i++)
    {
        switch(Source[i])
        {
            case ' ':
            {
                continue;
            }
            case '\n':
            {
                continue;
            }
            case '\r':
            {
                continue;
            }
            case STATEMENT_SEPARATOR_CODE:
            {
                TokenArrayAppend(TokenTable, {STATEMENT_SEPARATOR_TOKEN, 0});
                continue;
            }
            case ARG_SEPARATOR_CODE:
            {
                TokenArrayAppend(TokenTable, {ARG_SEPARATOR_TOKEN, 0});
                continue;
            }
            case PARENTHESES_CLOSE_CODE:
            {
                TokenArrayAppend(TokenTable, {PARENTHESES_CLOSE_TOKEN, 0});
                continue;
            }
            case PARENTHESES_OPEN_CODE:
            {
                TokenArrayAppend(TokenTable, {PARENTHESES_OPEN_TOKEN, 0});
                continue;
            }
            case CURLY_CLOSE_CODE:
            {
                TokenArrayAppend(TokenTable, {CURLY_CLOSE_TOKEN, 0});
                continue;
            }
            case CURLY_OPEN_CODE:
            {
                TokenArrayAppend(TokenTable, {CURLY_OPEN_TOKEN, 0});
                continue;
            }
            default:
            {
                if(isdigit(Source[i]))
                {
                    size_t LocalIndex = i + 1;
                    while(isdigit(Source[LocalIndex]))
                    {
                        LocalIndex++;
                    }
                    if(Source[LocalIndex] == '.')
                    {
                        if(isdigit(Source[LocalIndex + 1]))
                        {
                            Token_t Token = {};
                            Token.TokenType = FLOAT_CONST_TOKEN; 
                            Token.TokenData.FloatingPointValue = atof(Source + i);
                            TokenArrayAppend(TokenTable, Token);
                            i = LocalIndex + 1;
                            while(isdigit(Source[i]))
                            {
                                i++;
                            }
                            i--;
                            continue;
                        }
                        else
                        {
                            fprintf(stderr, "CoC\n");
                            return NULL;
                        }
                    }
                    else
                    {
                        Token_t Token = {};
                        Token.TokenType = INTEGER_CONST_TOKEN;
                        Token.TokenData.IntegerValue = atoi(Source + i);
                        TokenArrayAppend(TokenTable, Token);
                        i = LocalIndex - 1;
                        continue;
                    }

                }
                else
                {
                    size_t LocalIndex = i;
                    while((Source[LocalIndex] != STATEMENT_SEPARATOR_CODE) && (Source[LocalIndex] != ' ') && (Source[LocalIndex] != ARG_SEPARATOR_CODE) && 
                          (Source[LocalIndex] != '\n') && (Source[LocalIndex] != '\r') &&
                          (Source[LocalIndex] != PARENTHESES_CLOSE_CODE) && (Source[LocalIndex] != PARENTHESES_OPEN_CODE) &&
                          (Source[LocalIndex] != CURLY_CLOSE_CODE) &&
                          (Source[LocalIndex] != CURLY_OPEN_CODE))
                    {
                        LocalIndex++;
                        if(LocalIndex >= SourceSize)
                        {
                            break;
                        }
                    }

                    char LongToken[MAX_TOKEN_LENGTH] = {};
                    strncpy(LongToken, Source + i, LocalIndex - i);
                    Token_t Token = {};
                    Token.TokenType = LONG_TOKEN;
                    strncpy(Token.TokenData.LongTokenName, LongToken, LocalIndex - i);
                    Token.TokenData.LongTokenName[LocalIndex - i] = '\0';
                    TokenArrayAppend(TokenTable, Token);
                    i += LocalIndex - i - 1;
                }
            }
        }
    }

    #ifndef NDEBUG
    TokenDumpArray(TokenTable);
    #endif

    return TokenTable;
}

static int TokenArrayAppend(TokenTable_t* TokenTable, Token_t Token)
{
    fprintf(stderr, "Token appended\n");
    assert(TokenTable);
    assert(TokenTable->TokenArray);

    if(TokenTable->Free >= TokenTable->Capacity)
    {
        TokenTable->TokenArray = (Token_t*)realloc(TokenTable->TokenArray, (TokenTable->Capacity * ArrayResizeMultiplier) * sizeof(Token_t));
        TokenTable->Capacity *= ArrayResizeMultiplier;
        assert(TokenTable->TokenArray);
    }

    TokenTable->TokenArray[TokenTable->Free] = Token;
    (TokenTable->Free)++;

    return 0;
}

// DEBUG ZONE

#ifndef NDEBUG

static void TokenDumpArray(TokenTable_t* TokenTable)
{
    assert(TokenTable);
    FILE* fp = fopen("TokenArrayDumpDEBUG.txt", "w+b");
    assert(fp);
    fprintf(stderr, "%zu\n", TokenTable->Free);
    for(size_t i = 0; i < TokenTable->Free; i++)
    {
        switch(TokenTable->TokenArray[i].TokenType)
        {
            case FLOAT_CONST_TOKEN:
            {
                fprintf(fp, "%zu. %lf\n", i, TokenTable->TokenArray[i].TokenData.FloatingPointValue);
                continue;
            }
            case INTEGER_CONST_TOKEN:
            {
                fprintf(fp, "%zu. %d\n", i, TokenTable->TokenArray[i].TokenData.IntegerValue);
                continue;  
            }
            case STATEMENT_SEPARATOR_TOKEN:
            {
                fprintf(fp, "%zu. %c\n", i, STATEMENT_SEPARATOR_CODE);
                continue;
            }
            case ARG_SEPARATOR_TOKEN:
            {
                fprintf(fp, "%zu. %c\n", i, ARG_SEPARATOR_CODE);
                continue;
            }
            case PARENTHESES_CLOSE_TOKEN:
            {
                fprintf(fp, "%zu. %c\n", i, PARENTHESES_CLOSE_CODE);
                continue;
            }
            case PARENTHESES_OPEN_TOKEN:
            {
                fprintf(fp, "%zu. %c\n", i, PARENTHESES_OPEN_CODE);
                continue;
            }
            case CURLY_CLOSE_TOKEN:
            {
                fprintf(fp, "%zu. %c\n", i, CURLY_CLOSE_CODE);
                continue;
            }
            case CURLY_OPEN_TOKEN:
            {
                fprintf(fp, "%zu. %c\n", i, CURLY_OPEN_CODE);
                continue;
            }
            case LONG_TOKEN:
            {
                fprintf(fp, "%zu. %s\n", i, TokenTable->TokenArray[i].TokenData.LongTokenName);
                continue;
            }
            default:
            {
                fprintf(fp, "Something went badly wrong..\n");
                break;
            }

        }
    }
    fclose(fp);
}

#endif

// DEBUG ZONE


