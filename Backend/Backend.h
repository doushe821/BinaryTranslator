#ifndef BACKEND_H_INCLUDED
#define BACKEND_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../List/List.h"

#include "../PYAM_IR/include/libpyam_ir.h"


static const size_t VARIABLE_NAME_MAX = 512;
static const size_t KEY_WORD_NAME_MAX = 32;
static const size_t KEY_WORD_NUMBER = 16;

static const size_t TRANSLATOR_FUNCTION_LABEL_NAME_MAX = 32;

struct TranslatorFunction_t
{
    int Index;
    size_t NumberOfArguments;
    size_t NumberOfLocalVariables;
    char Label[TRANSLATOR_FUNCTION_LABEL_NAME_MAX];
};

#endif