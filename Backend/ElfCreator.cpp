#include "ElfCreator.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <elf.h>



#define ALIGNMENT 0x1000
#define ENTRY_ADDR 0x400000
const char* shstrtab = "\0.shstrtab\0.text";
const size_t shstrtabSize = 16;
int CreateElf(size_t TextSize, void* Text)
{

    size_t TextAlignmentSize = ALIGNMENT - TextSize % ALIGNMENT;
    Elf64_Ehdr elfHeader =
    {
        .e_ident = {ELFMAG0, ELFMAG1, ELFMAG2, ELFMAG3, ELFCLASS64, ELFDATA2LSB, EV_CURRENT, ELFOSABI_SYSV, 0, 0, 0, 0, 0, 0, 0, 0},
        .e_type = ET_EXEC,
        .e_machine = EM_X86_64,
        .e_version = EV_CURRENT,
        .e_entry = ENTRY_ADDR,
        .e_phoff = sizeof(Elf64_Ehdr),              
        .e_shoff = ALIGNMENT + TextAlignmentSize,
        .e_flags = 0,
        .e_ehsize = sizeof(Elf64_Ehdr),
        .e_phentsize = sizeof(Elf64_Phdr),
        .e_phnum = 1,
        .e_shentsize = sizeof(Elf64_Shdr),
        .e_shnum = 3,
        .e_shstrndx = 1,
    };

    Elf64_Phdr elfProgHeader =
    {
        .p_type = PT_LOAD,
        .p_flags = PF_X | PF_R,
        .p_offset = ALIGNMENT,
        .p_vaddr = elfHeader.e_entry,
        .p_paddr = elfHeader.e_entry,
        .p_filesz = TextSize,
        .p_memsz = TextSize,
        .p_align = ALIGNMENT,
    };

    Elf64_Shdr sectionNullHeader = 
    {
        .sh_name = 0,
        .sh_type = SHT_NULL,
        .sh_flags = 0,
        .sh_addr = 0,
        .sh_offset = 0,
        .sh_size = 0,
        .sh_link = SHN_UNDEF,
        .sh_info = 0,
        .sh_addralign = 0,
        .sh_entsize = 0
    };

    Elf64_Shdr sectionShstrtabHeader = 
    {
        .sh_name = 1,
        .sh_type = SHT_STRTAB,
        .sh_flags = SHF_STRINGS,
        .sh_addr = 0,
        .sh_offset = elfHeader.e_shoff - shstrtabSize,
        .sh_size = shstrtabSize,
        .sh_link = 0,
        .sh_info = 0,
        .sh_addralign = 1,
        .sh_entsize = 0
    };

    Elf64_Shdr sectionTextHeader = 
    {
        .sh_name = 11,
        .sh_type = SHT_PROGBITS,
        .sh_flags = SHF_ALLOC | SHF_EXECINSTR,
        .sh_addr = ENTRY_ADDR,
        .sh_offset = ALIGNMENT,
        .sh_size = TextSize,
        .sh_link = 0,
        .sh_info = 0,
        .sh_addralign = ALIGNMENT,
        .sh_entsize = 0
    };

    size_t HeaderAlignmentSize = ALIGNMENT - sizeof(Elf64_Ehdr) - sizeof(Elf64_Phdr);

    elf64File elfFile = 
    {
        .elfHeader = elfHeader,
        .elfProgHeader = elfProgHeader,
        .HeaderAlignment = calloc(HeaderAlignmentSize, 1),
        .sectionText = calloc(TextSize, 1),
        .TextAlignment = calloc(TextAlignmentSize, 1),
        .shstrtab = shstrtab,
        .shdrZero = sectionNullHeader,
        .shdrShstrtab = sectionShstrtabHeader,
        .shdrText = sectionTextHeader,
    };

    memcpy(elfFile.sectionText, Text, TextSize);

    FILE* elfOut = fopen("Test", "w+b");
    assert(elfOut);

    fwrite(&elfFile, sizeof(elfFile.elfHeader) + sizeof(elfFile.elfProgHeader), 1, elfOut);
    fwrite(elfFile.HeaderAlignment, HeaderAlignmentSize, 1, elfOut);
    fwrite(elfFile.sectionText, TextSize, 1, elfOut);
    fwrite(elfFile.TextAlignment, TextAlignmentSize, 1, elfOut);
    fwrite(elfFile.shstrtab, shstrtabSize, 1, elfOut);
    fwrite(&elfFile.shdrZero, sizeof(Elf64_Shdr) * 3, 1, elfOut);
    fclose(elfOut);

    free(elfFile.HeaderAlignment);
    free(elfFile.sectionText);
    free(elfFile.TextAlignment);

    return 0;
}