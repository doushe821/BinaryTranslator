#ifndef SYNTAXER_H_INCLUDED
#define SYNTAXER_H_INCLUDED

#include "CompilerTree.h"
#include "IntermediateRepresentationGenerator.h"

int GetProgram(Tree_t* Tree, const TokenTable_t* TokenTable, size_t* TokenIndex, List_t* VariableTablesList, FunctionTable_t* FunctionTable);

#endif