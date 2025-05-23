#ifndef HEADER_MAKER_H_INCLUDED
#define HEADER_MAKE_H_INCLUDED

#include <elf.h>
#include <stdlib.h>

int CreateElf(size_t TextSize, void* Text);

struct elf64File
{
    Elf64_Ehdr elfHeader;
    Elf64_Phdr elfProgHeader;
    void* HeaderAlignment;
    void* sectionText;
    void* TextAlignment;
    const char* shstrtab;
    uint64_t shstrtabSize;
    Elf64_Shdr shdrZero;
    Elf64_Shdr shdrShstrtab;
    Elf64_Shdr shdrText;
};

#endif