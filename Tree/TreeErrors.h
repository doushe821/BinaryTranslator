#ifndef TREE_ERRORS_H_INCLUDED
#define TREE_ERRORS_H_INCLUDED

enum TreeErrorsEnum
{
    UNKNOWN_ERROR_TREE = -67,
    ALLOCATION_FAILURE_TREE = -1,
    UNKNOWN_FIELD_CODE_TREE = -7,
    NOTHING_TO_BURN_TREE = -2,
    NULL_POINTER_TREE = -3,
};

struct error
{
    const char* message;
    const int code;
};

__attribute__((unused)) static error TreeErrors[10] =
{
    {"Unknown error on ", UNKNOWN_ERROR_TREE},
    {"Allocation failure on ", ALLOCATION_FAILURE_TREE},
    {"Got an unknwon filed code on ", UNKNOWN_FIELD_CODE_TREE},
    {"There is nothing to burn: root node is NULL\n", NOTHING_TO_BURN_TREE},
};



#endif