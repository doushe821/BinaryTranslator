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

const char* SystemCallOut = 
{
    "\n\nout:\n"
    "\tpop rax\n"
    "\tsub rsp, 80\n"
    "\tpush rbp\n"
    "\tmov rbp, rsp\n"
    "\tadd rbp, 88\n"
    "\tpush rbx\n"
    "\tpush rcx\n"
    "\tpush rdx\n"
    "\tpush rdi\n"
    "\tpush rsi\n"
    "\tpush r9\n"
    "\tmov r9, rax\n"
    "\tmov rdi, rbp\n"
    "\tsub rdi, 1\n"
    "\tmov qword rax, [rbp]\n\n"
    "\tmov rbx, 01h\n"
    "\tmov rdx, rax\n"
    "\tmov rcx, 64\n"
    "\tmov byte [rdi], 0x0A\n"
    "\tdec rdi\n"
    ".PrintingLoop:\n"
    "\tmov rdx, rax\n"
    "\tcmp rcx, 0x00\n"
    "\tje .PrintingLoopEnd\n"
    "\tand rdx, rbx\n"
    "\tadd rdx, 0x30\n"
    "\tmov byte [rdi], dl\n"
    "\tdec rdi\n"
    "\tshr rax, 1\n"
    "\tdec rcx\n"
    "\tjmp .PrintingLoop\n"
    ".PrintingLoopEnd:\n"
    "\tmov rax, 0x01\n"
    "\tmov rsi, rdi\n"
    "\tmov rdi, 0x01\n"
    "\tmov rdx, 66\n"
    "\tsyscall\n"
    "\tsub rbp, 8\n"
    "\tmov [rbp], r9\n"
    "\tpop r9\n"
    "\tpop rsi\n"
    "\tpop rdi\n"
    "\tpop rdx\n"
    "\tpop rsi\n"
    "\tpop rbx\n"
    "\tpop rbp\n"
    "\tadd rsp, 72\n"
    "\tret\n\n"
};

const char* SystemCallIn = 
{
    "in:"
    "\tsub rsp, 8 ; Stack frame creation\n"
    "\tsub rsp, 5\n"
    "\tpush rbx\n"
    "\tpush rcx\n"
    "\tpush rdx\n"
    "\tpush rdi\n"
    "\tpush rsi\n"
    "\tpush r8\n"
    "\tpush r9\n"
    "\tpush r10\n"
    "\tpush rbp\n"
    "\tmov rcx, rsp\n"
    "\tmov rbp, rcx\n"
    "\tmov rdi, rbp\n"
    "\tmov [rbp], '\"'\n"
    "\tinc rbp\n"
    "\tmov [rbp], '%'\n"
    "\tinc rbp\n"
    "\tmov [rbp], 'l'\n"
    "\tinc rbp\n"
    "\tmov [rbp], 'f'\n"
    "\tinc rbp\n"
    "\tmov [rbp], '\"'\n"
    "\tint rbp\n"
    "\tmov rsi, rbp\n"
    "\txor rax, rax\n"
    "\tcall scanf\n"
    "\tadd rsp, 8\n"
    "\tmovsd xmm0, [rsi]\n"
    "\tmovq rax, xmm0\n"
    "\tpop rbp\n"
    "\tpop r10\n"
    "\tpop r9\n"
    "\tpop r8\n"
    "\tpop rsi\n"
    "\tpop rdi\n"
    "\tpop rdx\n"
    "\tpop rcx\n"
    "\tpop rbx\n"
    "\tadd rsp, 8\n"
    "\tadd rsp, 5\n"
    "\tret\n\n"
};
const char* SystemCallHalt = 
{
    "halt:\n"
    "\tmov rax, 0x3c\n"
    "\tsyscall\n"
};

#endif