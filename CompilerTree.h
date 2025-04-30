#ifndef COMPILER_TREE_H_INCLUDED
#define COMPILER_TREE_H_INCLUDED

#include "Tree/Tree.h"
#include "Syntax.h"
#include "CompilerTables.h"

static const size_t ATL_BASE_NODE_SIZE = 32;

void* NewNode(const void* Tree, const size_t Type, const size_t DataSize, const void* Value, const size_t Degree, ...);

int NodeDump(const void* Node, FILE* Out);

void* CloneTree(const void* Tree, void* Root);

int AddDescendant(void* Node, const void* Descendant, size_t DescendantNumber);

void* GetNodeData(const void* Node, const int FieldCode, const size_t DescendantNumber);

#endif