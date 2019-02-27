// Simple command-line kernel monitor useful for
// controlling the kernel and exploring the system interactively.

#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/memlayout.h>
#include <inc/assert.h>
#include <inc/x86.h>

#include <kern/console.h>
#include <kern/monitor.h>
#include <kern/kdebug.h>
#include <kern/trap.h>

#include <kern/pmap.h>   // for challenge 2 in lab2
#define CMDBUF_SIZE	80	// enough for one VGA text line


struct Command {
	const char *name;
	const char *desc;
	// return -1 to force monitor to exit
	int (*func)(int argc, char** argv, struct Trapframe* tf);
};

static struct Command commands[] = {
	{ "help", "Display this list of commands", mon_help },
	{ "kerninfo", "Display information about the kernel", mon_kerninfo },
	{"backtrace","Display infomation about the call stack", mon_backtrace },
	{"map"," display the physical mappings that apply to a particular range of virtual addresses",mon_showmappings},
	{"setPTE_P","set the flag of PTE_P",mon_setPTE_P},
	{"clearPTE_P","clear the flag of PTE_P",mon_clearPTE_P},
	{"setPTE_W","set the flag of PTE_W",mon_setPTE_W},
	{"clearPTE_W","clear the flag of PTE_W",mon_clearPTE_W},
	{"setPTE_U","set the flag of PTE_U",mon_setPTE_U},
	{"clearPTE_U","clear the flag of PTE_U",mon_clearPTE_U},
	{"change_flags","change the permission",mon_change_flags},
	{"mem","dump the contents of a range VA/PA address range ",mon_mem}
};

/***** Implementations of basic kernel monitor commands *****/

int
mon_help(int argc, char **argv, struct Trapframe *tf)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(commands); i++)
		cprintf("%s - %s\n", commands[i].name, commands[i].desc);
	return 0;
}

int
mon_kerninfo(int argc, char **argv, struct Trapframe *tf)
{
	extern char _start[], entry[], etext[], edata[], end[];

	cprintf("Special kernel symbols:\n");
	cprintf("  _start                  %08x (phys)\n", _start);
	cprintf("  entry  %08x (virt)  %08x (phys)\n", entry, entry - KERNBASE);
	cprintf("  etext  %08x (virt)  %08x (phys)\n", etext, etext - KERNBASE);
	cprintf("  edata  %08x (virt)  %08x (phys)\n", edata, edata - KERNBASE);
	cprintf("  end    %08x (virt)  %08x (phys)\n", end, end - KERNBASE);
	cprintf("Kernel executable memory footprint: %dKB\n",
		ROUNDUP(end - entry, 1024) / 1024);
	return 0;
}

int
mon_backtrace(int argc, char **argv, struct Trapframe *tf)
{
	// Your code here.
	uint32_t *ebp = (uint32_t*)read_ebp();
	cprintf("Stack backtrace:\n");
	int i ;
	struct Eipdebuginfo info;
	while (ebp)
	{	
		uint32_t eip = ebp[1];
		cprintf("ebp %08x  eip %08x  ",ebp,eip);
		cprintf("args");
		for ( i = 2 ; i < 7 ; i++)
		{
			cprintf(" %08x",*(ebp+i));
		}
		cprintf("\n");
		int status = debuginfo_eip(eip,&info);
		if (status == 0)
		{
 
		  cprintf("%s:%d: ",info.eip_file,info.eip_line);
		  cprintf("%.*s+%d\n",info.eip_fn_namelen,info.eip_fn_name,eip-info.eip_fn_addr);
		}	
		ebp = (uint32_t*)*ebp;
	}


	return 0;
}


int
mon_showmappings(int argc, char **argv, struct Trapframe *tf)
{
	if (argc<3)
	{
		cprintf("USAGE: map [startVA] [endVA] \n");
		return -1;
	}
	char * sstartVA = argv[1];
	char * sendVA = argv[2];
	//cprintf("[%s,%s]\n",sstartVA,sendVA);
	uintptr_t istartVA = strtol(sstartVA,NULL,16);
	uintptr_t iendVA = strtol(sendVA,NULL,16);
	//cprintf("int: [%08x,%08x]\n",istartVA,iendVA);
	int cnt = ((iendVA - istartVA)>>12)&0xFFFFFF;
	//cprintf("cnt %d\n",cnt);
	cprintf("virtual address   phycisal address  PTE_U  PTE_W  PTE_P\n");
	for ( int i = 0 ; i < cnt ; i++)
	{
		uintptr_t curVA = istartVA + i * 0x1000;
		cprintf("   %08x   ",curVA);
		pte_t * entry ;
		struct PageInfo *pginfo = page_lookup(kern_pgdir,(void *)curVA,&entry);
		if (!pginfo)
		{
			cprintf("       None     ");
			cprintf("       None ");
			cprintf("  None");
			cprintf("  None\n");
		}
		else
		{
			physaddr_t pa = PTE_ADDR(*entry);
			cprintf("       %08x    ",pa);
			cprintf("     %d      %d     %d\n",1-!(*entry&PTE_U),1-!(*entry&PTE_W),1-!(*entry&PTE_P));
		}
	}	
	return 0;
}

int
mon_setPTE_P(int argc, char **argv, struct Trapframe *tf)
{
	char *sVA = argv[1];
	uintptr_t VA = strtol(sVA,NULL,16);
	pte_t * entry = pgdir_walk(kern_pgdir,(void *)VA,0);
	if (!entry)
	{
		cprintf("Page table entry not exist!\n");
		return -1;
	}
	*entry = *entry | PTE_P;
	return 0;
}
int
mon_clearPTE_P(int argc, char **argv, struct Trapframe *tf)
{
	char *sVA = argv[1];
	uintptr_t VA = strtol(sVA,NULL,16);
	pte_t * entry = pgdir_walk(kern_pgdir,(void *)VA,0);
	if (!entry)
	{
		cprintf("Page table entry not exist!\n");
		return -1;
	}
	//cprintf("entry %08x\n",*entry);
	//cprintf(" PTE_p %08x\n",(~PTE_P));
	 *entry = (*entry) & (~PTE_P);
	//cprintf("entry %08x\n",*entry);
	return 0;
}

	

int
mon_setPTE_W (int argc, char **argv, struct Trapframe *tf)
{
	char *sVA = argv[1];
	uintptr_t VA = strtol(sVA,NULL,16);
	pte_t * entry = pgdir_walk(kern_pgdir,(void *)VA,0);
	if (!entry)
	{
		cprintf("Page table entry not exist!\n");
		return -1;
	}
	*entry = *entry | PTE_W;
	return 0;

}

int
mon_clearPTE_W(int argc, char **argv, struct Trapframe *tf)
{
	char *sVA = argv[1];
	uintptr_t VA = strtol(sVA,NULL,16);
	pte_t * entry = pgdir_walk(kern_pgdir,(void *)VA,0);
	if (!entry)
	{
		cprintf("Page table entry not exist!\n");
		return -1;
	}
	*entry = (*entry) & (~PTE_W);
	return 0;

}

int
mon_setPTE_U(int argc, char **argv, struct Trapframe *tf)
{
	char *sVA = argv[1];
	uintptr_t VA = strtol(sVA,NULL,16);
	pte_t * entry = pgdir_walk(kern_pgdir,(void *)VA,0);
	if (!entry)
	{
		cprintf("Page table entry not exist!\n");
		return -1;
	}
	*entry = *entry | PTE_U;
	return 0;

}
int
mon_clearPTE_U(int argc, char **argv, struct Trapframe *tf)
{
	char *sVA = argv[1];
	uintptr_t VA = strtol(sVA,NULL,16);
	pte_t * entry = pgdir_walk(kern_pgdir,(void *)VA,0);
	if (!entry)
	{
		cprintf("Page table entry not exist!\n");
		return -1;
	}
	*entry = (*entry ) & (~PTE_U);
	return 0;

}

int
mon_change_flags(int argc, char **argv, struct Trapframe *tf)
{
	if (argc<3)
	{
		cprintf("USAGE: change_flags [VA] [permission] \n");
		return -1;
	}
	char *sVA = argv[1];
	char *sPer = argv[2];
	uintptr_t VA = strtol(sVA,NULL,16);
	int Per = strtol(sPer,NULL,10);
	//cprintf("Permission:%d\n",Per);
	pte_t *entry = pgdir_walk(kern_pgdir,(void *)VA,0);
	if (!entry)
	{
		cprintf("Page table entry not exist!\n");
		return -1;
	}
	*entry =( (*entry) & (~0x7) ) | Per;
	return 0;
}


int 
mon_mem(int argc, char **argv, struct Trapframe *tf)
{
	if (argc<4)
	{
		cprintf("usage: mem [VA/PA(start)]  [VA/PA(end)] P|V \n");
		return -1;
	}
	char *sstartA = argv[1];
	char *sendA = argv[2];
	char *type = argv[3];
	if (type[0]!='P'&&type[0]!='V')
	{
		cprintf("usage: mem [VA/PA(start)]  [VA/PA(end)] P|V \n");
		return -1;
	}


	uintptr_t startVA,endVA;
	if (type[0]=='P')
	{
		startVA = strtol(sstartA,NULL,16) + KERNBASE;
		endVA = strtol(sendA,NULL,16) + KERNBASE;
	}
	else 
	{
		startVA = strtol(sstartA,NULL,16);
		endVA = strtol(sendA,NULL,16);
	}
	startVA = ROUNDUP(startVA,4);
	endVA = ROUNDUP(endVA,4);
	int cnt = ((endVA - startVA)>>2);;
	cprintf("startVA: %08x endVA:%08x cnt:%d\n",startVA,endVA,cnt);
	for ( int i = 0 ; i < cnt ; i++)
	{
		void ** cur_VA = (void **)startVA + i;
		cprintf("[%08x]:%08x\n",cur_VA,*cur_VA);
	}

	return 0;
	
}


/***** Kernel monitor command interpreter *****/

#define WHITESPACE "\t\r\n "
#define MAXARGS 16

static int
runcmd(char *buf, struct Trapframe *tf)
{
	int argc;
	char *argv[MAXARGS];
	int i;

	// Parse the command buffer into whitespace-separated arguments
	argc = 0;
	argv[argc] = 0;
	while (1) {
		// gobble whitespace
		while (*buf && strchr(WHITESPACE, *buf))
			*buf++ = 0;
		if (*buf == 0)
			break;

		// save and scan past next arg
		if (argc == MAXARGS-1) {
			cprintf("Too many arguments (max %d)\n", MAXARGS);
			return 0;
		}
		argv[argc++] = buf;
		while (*buf && !strchr(WHITESPACE, *buf))
			buf++;
	}
	argv[argc] = 0;

	// Lookup and invoke the command
	if (argc == 0)
		return 0;
	for (i = 0; i < ARRAY_SIZE(commands); i++) {
		if (strcmp(argv[0], commands[i].name) == 0)
			return commands[i].func(argc, argv, tf);
	}
	cprintf("Unknown command '%s'\n", argv[0]);
	return 0;
}

void
monitor(struct Trapframe *tf)
{
	char *buf;

	cprintf("Welcome to the JOS kernel monitor!\n");
	cprintf("Type 'help' for a list of commands.\n");

	if (tf != NULL)
		print_trapframe(tf);

	while (1) {
		buf = readline("K> ");
		if (buf != NULL)
			if (runcmd(buf, tf) < 0)
				break;
	}
}
