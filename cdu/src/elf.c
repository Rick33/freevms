// $Id$
// $Locker$

// Author. Roar Thron�s.

#include <sys/types.h>
#include <unistd.h>
#include <libelf.h>
#include <ssdef.h>
#include <descrip.h>
#include <misc.h>

static int myfunc(int (*func)(),void * start, int count) {
#ifdef __i386__
  __asm__ __volatile__(
		       "pushl %ebx\n\t"
		       "pushl %ecx\n\t"
		       "pushl %edx\n\t"
		       "pushl %edi\n\t"
		       "pushl %esi\n\t"
		       "movl 0x8(%ebp),%eax\n\t"
		       "movl 0xc(%ebp),%esi\n\t"
		       "movl 0x10(%ebp),%ecx\n\t"
		       "movl $0x400,%ecx\n\t"
		       "movl %ecx,%edx\n\t"
		       "pushl %eax\n\t"
		       "call is_user_mode\n\t"
		       "andl $1, %eax\n\t"
		       "popl %eax\n\t"
		       "je 1f\n\t"
		       "subl $0x1000,%esp\n\t"
		       "movl %esp,%edi\n\t"
		       "rep ; movsl\n\t"
		       "jmp *%eax\n\t"
		       "1:\n\t"
		       // check. related to CLI supervisor
		       "movl $0x7ffe0000, %edi\n\t"
		       "subl $0x1000, %edi\n\t"
		       "rep ; movsl\n\t"
		       "movl %eax, %edi\n\t"
		       "movl $0x7ffe0000, %ecx\n\t"
		       "subl $0x1000, %ecx\n\t"
		       "int $0xb4\n\t"
		       "movl 0x7ffff0a8, %eax\n\t"
		       "movl $0xf, 2032(%eax)\n\t" /* psl */
		       "movl 2124(%eax), %eax\n\t" /* ipr_sp[3] */
		       "movl %esp, %edx\n\t"
		       "addl $-0x14, %edx\n\t"
		       "movl %edi, 0x0(%edx)\n\t"
		       "movl $0x23, 0x4(%edx)\n\t"
		       "movl $0x200, 0x8(%edx)\n\t"
		       "movl %ecx, 0xc(%edx)\n\t"
		       "movl $0x2b, 0x10(%edx)\n\t"
		       "addl $-0x14, %esp\n\t"
		       "movl $0x2b, %eax\n\t"
		       "movl %eax, %ds\n\t"
		       "movl %eax, %es\n\t"
		       "iret\n\t"
		       );
  // return eax default?
#endif
#ifdef __x86_64__
  __asm__ __volatile__(
		       "pushq %rbx\n\t"
		       "pushq %rcx\n\t"
		       "pushq %rdx\n\t"
		       "pushq %rdi\n\t"
		       "pushq %rsi\n\t"
		       "movq %rdi,%rax\n\t"
		       "movq %rsi,%rsi\n\t"
		       "movq %rdx,%rcx\n\t"
		       "movq $0x400,%rcx\n\t"
		       "movq %rcx,%rdx\n\t" 
		       "subq $0x1000,%rsp\n\t"
		       "movq %rsp,%rdi\n\t"
		       "rep ; movsl\n\t"
		       "jmpq *%rax\n\t"
		       );
  // return eax default?
#endif
}

static int mymyfunc(int dummy,int (*func)(),void * start, int count) {
#ifdef __i386__ 
  long * ret = &func;
  ret=&dummy;
#else
  long * ret;
  __asm__ __volatile__ ("movq %%rbp,%0; ":"=r" (ret) );
  ret+=2;
#endif
  struct _exh exh;
  memset(&exh, 0, sizeof(exh));
  exh.exh$l_handler=ret[-1];
  exh.exh$l_first_arg=&ret[-1];
  int sts = sys$dclexh(&exh);
  return myfunc(*func,start,count);
}

static long mymymyfunc(int (*func)(),void * start, int count) {
  register long __res;
#ifdef __i386__
  __asm__ ( "movl %%ebp,%%eax\n\t" :"=a" (__res) );
  mymyfunc(__res,*func,start,count);
  __asm__ ( "movl (%esp),%ebp\n\t" );
#endif
#ifdef __x86_64__
  __asm__ __volatile__ (
			"movq %rbp,%rax\n\t"
			"pushq %rax\n\t"
			);
  mymyfunc(__res,*func,start,count);
  __asm__ ( "movq (%rsp),%rbp\n\t" );
#endif
}

static int dummy_routine(void) {
#ifdef __i386__
  exit(0);
#else
  sys$exit(0);
#endif
  return 0;
}

int load_elf(char * filename) {
  int sts;
  void (*func)();
  struct dsc$descriptor dflnam;
  dflnam.dsc$w_length=strlen(filename);
  dflnam.dsc$a_pointer=filename;
  char * hdrbuf=malloc(512 * 8);
  memset(hdrbuf, 0, 512 * 8);

  sts=sys$imgact(&dflnam,&dflnam,hdrbuf,0,0,0,0,0);
  printf("imgact got sts %x\n",sts);
  sts=sys$imgfix();
  printf("imgfix got sts %x\n",sts);

  if (sts!=SS$_NORMAL) return sts;

#ifdef __i386__
  Elf32_Ehdr * elf = hdrbuf;
#else
  Elf64_Ehdr * elf = hdrbuf;
#endif
  func = elf->e_entry;
  printf("entering image? %x\n",func);
  long entry=func;
  long arg=0;
  int *addr=&elf->e_version; // check. fix later. was: long
  if (*addr!=func) {
    arg=func;
    func=*addr;
  }
  addr=&elf->e_ident;
  int i;
  long * prep=*addr;
  for(i=0;i<100;i++) {
    if (prep[i]==9 && prep[i+1]==entry) {
      prep[i+1]=dummy_routine;
      if (func==entry)
	func=dummy_routine;
      break;
    }
  }
  int offset = ((long)(*addr)) - ((long)elf);
  sts = mymymyfunc(func,*addr,(4096-offset)>>2);
  printf("after image\n");
  return SS$_NORMAL;
}

long elf_get_symbol(char * filename, char * name){
  int i;
  int fildes;
  Elf * arf, * elf;
#ifdef __i386__
  Elf32_Ehdr * ehdr;
  Elf32_Shdr * section_headers;
  Elf32_Shdr * section;
  Elf32_Shdr * strtabsh = 0, * symtabsh = 0;
  Elf32_Sym * symtab;
#else
  Elf64_Ehdr * ehdr;
  Elf64_Shdr * section_headers;
  Elf64_Shdr * section;
  Elf64_Shdr * strtabsh = 0, * symtabsh = 0;
  Elf64_Sym * symtab;
#endif
  char * strtab, * mystrtab;
  int offs;
  int val=0;
   
  fildes=open(filename, 0);
  if (elf_version(EV_CURRENT) == EV_NONE)
    {
      printf("dated\n");
      return 0;
    }
  arf = elf_begin(fildes, ELF_C_READ, (Elf *)0);
  elf = elf_begin(fildes, ELF_C_READ, arf);
#ifdef __i386__
  ehdr = elf32_getehdr(elf);
#else
  ehdr = elf64_getehdr(elf);
#endif

#ifdef __i386__
  section_headers = (void *) malloc(sizeof(Elf32_Shdr)*ehdr->e_shnum);
#else
  section_headers = (void *) malloc(sizeof(Elf64_Shdr)*ehdr->e_shnum);
#endif
  lseek(fildes,ehdr->e_shoff,SEEK_SET);
#ifdef __i386__
  read(fildes, section_headers, sizeof(Elf32_Shdr)*ehdr->e_shnum);
#else
  read(fildes, section_headers, sizeof(Elf64_Shdr)*ehdr->e_shnum);
#endif
  for (i = 0, section = section_headers;
       i < ehdr->e_shnum;
       i++, section++)
    {
      if (  (section->sh_flags&SHF_ALLOC) || i==ehdr->e_shstrndx || ( section->sh_type != SHT_SYMTAB && section->sh_type != SHT_STRTAB))
	continue;
      if (section->sh_type==SHT_SYMTAB)
	symtabsh=section;
      if (section->sh_type==SHT_STRTAB)
	strtabsh=section;
    }
  if (strtabsh==0 || symtabsh==0)
    return 0;
  strtab = (void *) malloc(strtabsh->sh_size);
  lseek(fildes, strtabsh->sh_offset, SEEK_SET);
  read(fildes, strtab, strtabsh->sh_size);
  symtab = (void *) malloc(symtabsh->sh_size);
  lseek(fildes, symtabsh->sh_offset, SEEK_SET);
  read(fildes, symtab, symtabsh->sh_size);
  mystrtab=strtab;
  mystrtab++;
  for(;*mystrtab;mystrtab+=strlen(mystrtab)+1) {
    if (0==strcmp(name,mystrtab))
      break;
  }
  offs=mystrtab-strtab;
  if (offs) {
#ifdef __i386__
    int max=symtabsh->sh_size/sizeof(Elf32_Sym);
#else
    int max=symtabsh->sh_size/sizeof(Elf64_Sym);
#endif
    for(i=0;i<max;i++) {
      if(symtab[i].st_name==offs) {
	val=symtab[i].st_value;
	break;
      }
    }
  }
  elf_end(elf);
  elf_end(arf);
  close(fildes);
  free(section_headers);
  free(symtab);
  free(strtab);
  return val;
}
