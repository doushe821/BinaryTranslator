#ifndef TREE_H_INCLUDED
#define TREE_H_INCLUDED

#include <stdio.h>
#include <stdarg.h>

#include "TreeErrors.h"


typedef void* (*InitFunc)(const void*, const size_t, const size_t, const void*, const size_t, ...);
typedef void* (*CloneFunc)(const void*, void*);
typedef int   (*DumpFunc)(const void*, FILE*);

enum fieldCodes
{
    TYPE_FIELD_CODE        = 2,
    DATASIZE_FIELD_CODE    = 3,
    DATA_FIELD_CODE        = 4,
    DEGREE_FIELD_CODE      = 5,
    DESCENDANTS_FIELD_CODE = 6,
};

struct Tree_t
{
    InitFunc   InitNode;
    CloneFunc CloneTree;
    DumpFunc   DumpNode;

    void* root         ;
};

struct BaseNode
{
    void*  root       ;
    size_t type       ;
    size_t datasize   ;
    void*  value      ;
    size_t degree     ;
    void*  descendants;
};

size_t CalculateByteShift(const void* Node, const int FieldCode);
int    PrintTreeGraphViz(FILE* Out, const Tree_t Tree, const char* FileName);
int    BurnBranch(void* Node);
int    BurnTree(Tree_t* Tree);

#endif