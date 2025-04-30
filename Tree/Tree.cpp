#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "Tree.h"
#include "TreeErrors.h"

#define DESCENDANT_NODE ((const char*)node + CalculateByteShift(node, DESCENDANTS_FIELD_CODE))

static int PrintRecursion(FILE* out, void* node, size_t* NodeCounter, size_t rank, DumpFunc);

int BurnTree(Tree_t* tree)
{
    if(tree->root == NULL)
    {
        return NOTHING_TO_BURN_TREE;
    }
    
    return BurnBranch(tree->root);
}

int BurnBranch(void* node)
{
    if(node == NULL)
    {
        return NOTHING_TO_BURN_TREE;
    }
    size_t degree = 0;
    memcpy(&degree, ((char*)node + CalculateByteShift(node, DEGREE_FIELD_CODE)), sizeof(size_t));
    for(size_t i = 0; i < degree; i++)
    {
        char* addr = {};
        memcpy(&addr, DESCENDANT_NODE + sizeof(void*) * i, sizeof(void*));
        if(addr != NULL)
        {
            BurnBranch(addr);
        }
    }
    free(node);
    return 0;
}

int PrintTreeGraphViz(FILE* out, const Tree_t tree, __attribute((unused)) const char* filename)
{
    fprintf(out, 
    "digraph g {\n"
    "fontname=\"Helvetica,Arial,sans-serif\""
    "node [fontname=\"Helvetica,Arial,sans-serif\", color = \"green\"]\n"
    "graph [\n"
    "rankdir = \"TB\"\n"
    "];\n"
    "node [\n"
    "fontisze = \"16\"\n"
    "shape = \"ellipse\"\n"
    "style = filled\n"
    "];\n"                );


    size_t NodeCounter = 0;
    PrintRecursion(out, tree.root, &NodeCounter, 0, tree.DumpNode);
    fprintf(out, "}");
    
    return 0;
}

static int PrintRecursion(FILE* out, void* node, size_t* NodeCounter, size_t rank, DumpFunc DFunc)
{
    if(out == NULL)
    {
        return NULL_POINTER_TREE;
    }
    if(DFunc == NULL)
    {
        return NULL_POINTER_TREE;
    }
    if(node == NULL)
    {
        return NULL_POINTER_TREE;
    }

    fprintf(out, "\n\"node%zu\" [\n", *NodeCounter);
    fprintf(out, "rank = %zu\n", rank);                                                                                            
    DFunc(node, out);
    
    size_t curNode = *NodeCounter;
    size_t degree = 0;
    memcpy(&degree, (const void*)((const char*)node + CalculateByteShift(node, DEGREE_FIELD_CODE)), sizeof(degree));
    for(size_t i = 0; i < degree; i++)
    {
        char* addr = {};
        memcpy(&addr, DESCENDANT_NODE + sizeof(void*) * i, sizeof(void*));
        if((void*)addr!= NULL)
        {
            fprintf(out, "node%zu: <d%zu> -> node%zu[color = \"blue\"];\n", curNode, i, *NodeCounter + 1);
            (*NodeCounter)++;
            rank++;
            PrintRecursion(out, addr, NodeCounter, rank, DFunc);
        }
    }  
    return 0;
}

size_t CalculateByteShift(const void* node, const int fieldcode)
{
    BaseNode bn = {};
    
    switch(fieldcode)
    {
        case TYPE_FIELD_CODE:
        {
            return sizeof(bn.root);
        }
        case DATASIZE_FIELD_CODE:
        {
            return sizeof(bn.root) + sizeof(bn.type);
        }
        case DATA_FIELD_CODE:
        {
            return sizeof(bn.root) + sizeof(bn.type) + sizeof(bn.datasize);
        }
        case DEGREE_FIELD_CODE:
        {
            size_t datasize = 0;
            memcpy(&datasize, (const void*)((const char*)node + CalculateByteShift(node, DATASIZE_FIELD_CODE)), sizeof(datasize));
            return sizeof(bn.root) + sizeof(bn.type) + sizeof(bn.datasize) + datasize;
        }
        case DESCENDANTS_FIELD_CODE:
        { 
            size_t datasize = 0;
            memcpy(&datasize, (const void*)((const char*)node + CalculateByteShift(node, DATASIZE_FIELD_CODE)), sizeof(datasize));
            return sizeof(bn.root) + sizeof(bn.type) + sizeof(bn.datasize) + datasize + sizeof(bn.degree);
        }
        default:
        {
            return UNKNOWN_FIELD_CODE_TREE; 
        }
    }

    return UNKNOWN_ERROR_TREE;
}