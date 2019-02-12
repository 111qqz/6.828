#ifndef JOS_INC_ELF_H
#define JOS_INC_ELF_H

#define ELF_MAGIC 0x464C457FU	/* "\x7FELF" in little endian */

struct Elf {
	uint32_t e_magic;	// must equal ELF_MAGIC
	uint8_t e_elf[12];
	/* e_elf[0] 1 for signed 32 bit , 2 for signed 64-bit
	        [1] 1 for little endianness ,2 for big endianness
                [2] version type
                [3] target OS
                [4] ABI version
                [5..11]  unused
	*/
	uint16_t e_type;     // object file type
	uint16_t e_machine;  // instruction set arch , x86/MIPS/IA-64 and etc.
	uint32_t e_version; 
	uint32_t e_entry;    // the memory address of the entry point where process start executing.
	uint32_t e_phoff;    // points to the start of the program header table.
	uint32_t e_shoff;    // Points to the start of the section header table.
	uint32_t e_flags;  
	uint16_t e_ehsize;   // size of this header. 64byte for 64-bit,52bytes for 32-bit
	uint16_t e_phentsize; // the size of a program header table entry.
	uint16_t e_phnum;    // the number of entries in the program header table.
	uint16_t e_shentsize; // the size of a section  header table entry.
	uint16_t e_shnum;    // the number of entries in the section header table.
	uint16_t e_shstrndx; 
};

struct Proghdr {
	uint32_t p_type;    // type of the segment
	uint32_t p_offset;  //  offset of the segment in the file image
	uint32_t p_va;      // virtual address of the segment in memory
	uint32_t p_pa;      // physical address for segment(?)
	uint32_t p_filesz;  // Size in bytes of the segment in the file image. May be 0.
	uint32_t p_memsz;   // Size in bytes of the segment in memory. May be 0.
	uint32_t p_flags;
	uint32_t p_align;   // 0 and 1 specify no alignment. Otherwise should be a positive, integral power of 2
};

struct Secthdr {
	uint32_t sh_name; // An offset to a string in the .shstrtab section that represents the name of this section
	uint32_t sh_type; // the type of this header
	uint32_t sh_flags; // the attributes of the section
	uint32_t sh_addr; // Virtual address of the section in memory
	uint32_t sh_offset;  // Offset of the section in the file image
	uint32_t sh_size;    // Size in bytes of the section in the file image. May be 0.
	uint32_t sh_link;    // 
	uint32_t sh_info;
	uint32_t sh_addralign;
	uint32_t sh_entsize;
        /*
          Contains the size, in bytes, of each entry, for sections that contain fixed-size entries. 
          Otherwise, this field contains zero.
         */
};

// Values for Proghdr::p_type
#define ELF_PROG_LOAD		1

// Flag bits for Proghdr::p_flags
#define ELF_PROG_FLAG_EXEC	1
#define ELF_PROG_FLAG_WRITE	2
#define ELF_PROG_FLAG_READ	4

// Values for Secthdr::sh_type
#define ELF_SHT_NULL		0
#define ELF_SHT_PROGBITS	1
#define ELF_SHT_SYMTAB		2
#define ELF_SHT_STRTAB		3

// Values for Secthdr::sh_name
#define ELF_SHN_UNDEF		0

#endif /* !JOS_INC_ELF_H */
