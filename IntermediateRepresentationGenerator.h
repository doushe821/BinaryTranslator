#ifndef INTERMEDIATE_REPRESENTATION_GENERATOR_H_INCLUDED
#define INTERMEDIATE_REPRESENTATION_GENERATOR_H_INCLUDED

#include "CompilerTables.h"
#include "Syntaxer.h"
#include "Syntax.h"

#include "PYAM_IR/include/libpyam_ir.h"

#include <assert.h>

#include <stdio.h>
#include <stdlib.h>

int GenerateIntermediateRepresentation(Tree_t* Tree, FunctionTable_t* FunctionTable, FILE* OutputFile);

#endif