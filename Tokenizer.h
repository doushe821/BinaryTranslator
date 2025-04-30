#ifndef TOKENIZER_H_INCLUDED
#define TOKENIZER_H_INCLUDED

#include "CompilerTables.h"
#include "Syntax.h"

TokenTable_t* Tokenize(char* Source, size_t SourceSize);

#endif