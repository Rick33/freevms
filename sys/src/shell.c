// $Id$
// $Locker$

// Author. Roar Thron�s.

#include<linux/linkage.h>
#include<linux/sched.h>
#include<linux/completion.h>
#include<linux/personality.h>
#include<linux/mm.h>
#include<linux/module.h>

#include<system_data_cells.h>
#include<descrip.h>
#include<ipldef.h>
#include<phddef.h>
#include<pqbdef.h>
#include<prcdef.h>
#include<pridef.h>
#include<rdedef.h>
#include<secdef.h>
#include<ssdef.h>
#include<starlet.h>

#include <linux/sched.h>
#include <linux/bootmem.h>
#include <linux/mman.h>

#include <linux/slab.h>

#include <asm/pgalloc.h>
#include <asm/mmu_context.h>

long phys_pte_addr(struct _pcb * pcb) {
  pgd_t *pgd;
  pmd_t *pmd;
  pte_t *pte;

  long page=0x7ffdf000;

  struct mm_struct * mm=pcb->mm;

  pgd = pgd_offset(mm, page);

  pmd = pgd;

  if (pmd) {
    pte = pte_offset(pmd, page);
  }

  return (*(unsigned long*)pte)&0xfffff000;

}

void init_p1pp_long(unsigned long addr, signed long offset, signed long val) {
  signed long * valp=addr+offset;
  *valp=val;
}

void init_p1pp_data(struct _pcb * pcb, struct _phd * phd,signed long offset) {
  init_p1pp_long(&ctl$gl_pcb,offset,pcb);
  init_p1pp_long(&ctl$gl_phd,offset,phd);
  init_p1pp_long(&ctl$gl_ccbbase,offset,&ctl$ga_ccb_table);
  init_p1pp_long(&ctl$gl_chindx,offset,CHANNELCNT);
}

void init_sys_p1pp() {
  extern struct _phd system_phd;
  struct _pcb * pcb = &init_task;
  pcb->pcb$l_phd=&system_phd;
  long phd=pcb->pcb$l_phd;

  pgd_t *pgd;
  pmd_t *pmd;
  pte_t *pte;

  long page=0x7ffff000;

  struct mm_struct * mm=&init_mm;

  long pt=alloc_bootmem_pages(4096);
  long pa=alloc_bootmem_pages(4096);

  puts("alloced pt pa\n");

  memset(pt,0,4096);
  memset(pa,0,4096);

  puts("memset them\n");

  pgd = pgd_offset(mm, page);

  puts("pgd\n");

#ifdef __arch_um__
  *(unsigned long *)pgd=pt|_PAGE_PRESENT|_PAGE_RW;
#else
  *(unsigned long *)pgd=__pa(pt)|_PAGE_PRESENT|_PAGE_RW;
#endif

  puts("set pgd\n");

  pmd = pgd; //pmd_offset(pgd, page);

#ifdef __arch_um__
  pte = pte_offset(pmd, page);
#else
  pte = pte_offset(((pmd_t*)__pa(pmd)), page);
#endif

  char c[40];
  sprintf(c,"%x %x %x %x\n",swapper_pg_dir,pgd,pmd,pte);

  puts("pte\n");
  puts(c);

#ifdef __arch_um__
  long pfn=pa>>12;
#else
  long pfn=__pa(pa)>>12;
#endif

#ifdef __arch_um__
  *(unsigned long *)pte=((unsigned long)(pfn<<PAGE_SHIFT))|_PAGE_NEWPAGE|_PAGE_PRESENT|_PAGE_RW|_PAGE_USER|_PAGE_ACCESSED|_PAGE_DIRTY;
  unmap(page,4096);
  map(page, pte_address(*pte), PAGE_SIZE, 1, 1, 0);
  //  my_fix_range(mm, page, page + PAGE_SIZE, 1);
  ctl$gl_pcb=&init_task_union;
#if 0
  flush_tlb_range(pcb->mm, page, page + PAGE_SIZE);
#endif
  memset(page,0,PAGE_SIZE); // must zero content also
#else
#define _PAGE_NEWPAGE 0
  *(unsigned long *)pte=((unsigned long)/*__va*/(pfn*PAGE_SIZE))|_PAGE_NEWPAGE|_PAGE_PRESENT|_PAGE_RW|_PAGE_USER|_PAGE_ACCESSED|_PAGE_DIRTY;
  puts("pte content\n");
  ctl$gl_pcb=&init_task_union;
  puts("ctl$gl_pcb set\n");
  flush_tlb_range(pcb->mm, page, page + PAGE_SIZE);
  puts("flush\n");
  ctl$gl_chindx=42;
  puts("42\n");
  if ( (*(unsigned long*)0x7ffff000)==42)
    puts("still 42\n");
  memset(page,0,PAGE_SIZE); // must zero content also
  puts("memset page\n");
  if ( (*(unsigned long*)0x7ffff000)==0)
    puts("not 42\n");
#endif

  init_p1pp_data(pcb,phd,0);
}

void init_p1pp(struct _pcb * pcb, struct _phd * phd) {
  pgd_t *pgd;
  pmd_t *pmd;
  pte_t *pte;

  long page=0x7ffff000;

  struct mm_struct * mm=pcb->mm;

  if (mm==0)
    mm=&init_mm;

  pgd = pgd_offset(mm, page);

  pmd = pmd_alloc(mm, pgd, page);

  if (pmd) {
    pte = pte_alloc(mm, pmd, page);
    if (pte)
      *(long*)pte=0;
  }

#ifdef CONFIG_VMS
  int pfn=mmg$ininewpfn(pcb,phd,page,pte);
  mem_map[pfn].pfn$q_bak=*(unsigned long *)pte;
#else
  int pfn=_alloc_pages(GFP_KERNEL,0)-mem_map;
#endif
#ifdef __arch_um__
  *(unsigned long *)pte=((unsigned long)__va(pfn*PAGE_SIZE))|_PAGE_NEWPAGE|_PAGE_PRESENT|_PAGE_RW|_PAGE_USER|_PAGE_ACCESSED|_PAGE_DIRTY;
#if 0
  flush_tlb_range(pcb->mm, page, page + PAGE_SIZE);
#endif
  my_fix_range(mm, page, page + PAGE_SIZE, 1);
  memset(page,0,PAGE_SIZE); // must zero content also
#else
#define _PAGE_NEWPAGE 0
  *(unsigned long *)pte=((unsigned long)/*__va*/(pfn*PAGE_SIZE))|_PAGE_NEWPAGE|_PAGE_PRESENT|_PAGE_RW|_PAGE_USER|_PAGE_ACCESSED|_PAGE_DIRTY;
  ctl$gl_pcb=pcb;
  ctl$gl_chindx=42;
  //printk("pte content\n");
  flush_tlb_range(pcb->mm, page, page + PAGE_SIZE);
  //printk("flush\n");
  ctl$gl_chindx=42;
  //printk("42\n");
  if ( (*(unsigned long*)0x7ffff000)==42)
    //printk("still 42\n");
  memset(page,0,PAGE_SIZE); // must zero content also
  ctl$gl_pcb=pcb;
  //printk("memset page\n");
  if ( (*(unsigned long*)0x7ffff000)==0)
    //printk("not 42\n");
#endif
#if 0
  do_mmap_pgoff(0,page,4096,PROT_READ | PROT_WRITE,MAP_FIXED | MAP_PRIVATE,0);
#if 0
  long vma = find_vma(pcb->mm, page);
  handle_mm_fault(pcb->mm, vma, page, 1);
#endif
#endif

  init_p1pp_data(pcb,phd,0);
}

int init_fork_p1pp(struct _pcb * pcb, struct _phd * phd, struct _pcb * oldpcb, struct _phd * oldphd) {
  int uml_map=0;

  pgd_t *pgd;
  pmd_t *pmd;
  pte_t *pte;
  pgd_t *oldpgd;
  pmd_t *oldpmd;
  pte_t *oldpte;

  long oldpage=0x7fffe000;
  long page=0x7ffff000;

  struct mm_struct * mm=pcb->mm;
  struct mm_struct * oldmm=oldpcb->mm;

  //printk("%x %x %x %x ",page,pcb,phd,mm);
  //printk("%x %x %x %x\n",oldpage,oldpcb,oldphd,oldmm);

  if (oldmm==0)
    oldmm=&init_mm;

  if (mm==0) {
    int retval=0;
    struct _pcb * tsk = pcb;
    mm=&init_mm;
#define allocate_mm()	(kmem_cache_alloc(mm_cachep, SLAB_KERNEL))
    mm = allocate_mm();
    if (!mm)
      panic("goto fail_nomem\n");// was: goto fail_nomem;
    
    /* Copy the current MM stuff.. */
    memcpy(mm, oldmm, sizeof(*mm));
    if (!mm_init(mm))
      panic("goto fail_nomem\n");// was: goto fail_nomem;

    down_write(&oldmm->mmap_sem);
    // not needed? retval = dup_mmap(mm);
#ifdef CONFIG_MM_VMS
    retval = dup_stuff(mm,tsk->pcb$l_phd);
#endif
    up_write(&oldmm->mmap_sem);
    
    if (retval)
      goto free_pt;
    
    /*
     * child gets a private LDT (if there was an LDT in the parent)
     */
    copy_segments(tsk, mm);
    
    if (init_new_context(tsk,mm))
      goto free_pt;
    
  good_mm:
    tsk->mm = mm;
    tsk->active_mm = mm;
    goto out;

  free_pt:
    mmput(mm);
    return retval;
  }

 out:

  pgd = pgd_offset(mm, page);
  oldpgd = pgd_offset(oldmm, oldpage);

  pmd = pmd_alloc(mm, pgd, page);
  oldpmd = pmd_alloc(oldmm, oldpgd, oldpage);

  if (pmd) {
    pte = pte_alloc(mm, pmd, page);
#if 0
    if (pte)
      *(long*)pte=0;
#endif
  }
  if (oldpmd) {
    oldpte = pte_alloc(oldmm, oldpmd, oldpage);
#if 0
    if (oldpte)
      *(long*)oldpte=0;
#endif
  }

  //printk("%x %x %x %x\n",swapper_pg_dir,pgd,pmd,pte);
  //printk("%x %x %x %x\n",swapper_pg_dir,oldpgd,oldpmd,oldpte);

#ifdef CONFIG_VMS
  int pfn=mmg$ininewpfn(pcb,phd,page,pte);
  mem_map[pfn].pfn$q_bak=*(unsigned long *)pte;
#else
  int pfn=_alloc_pages(GFP_KERNEL,0)-mem_map;
#endif
#ifdef __arch_um__
  *(unsigned long *)pte=((unsigned long)__va(pfn*PAGE_SIZE))|_PAGE_NEWPAGE|_PAGE_PRESENT|_PAGE_RW|_PAGE_USER|_PAGE_ACCESSED|_PAGE_DIRTY;
  *(unsigned long *)oldpte=((unsigned long)__va(pfn*PAGE_SIZE))|_PAGE_NEWPAGE|_PAGE_PRESENT|_PAGE_RW|_PAGE_USER|_PAGE_ACCESSED|_PAGE_DIRTY;
  uml_map=__va(pfn*PAGE_SIZE);
#if 0
  flush_tlb_range(pcb->mm, page, page + PAGE_SIZE);
  flush_tlb_range(oldpcb->mm, oldpage, oldpage + PAGE_SIZE);
#endif
  my_fix_range(oldmm, oldpage, oldpage + PAGE_SIZE, 1);
  memset(oldpage,0,PAGE_SIZE); // must zero content also
#else
#define _PAGE_NEWPAGE 0
  *(unsigned long *)pte=((unsigned long)/*__va*/(pfn*PAGE_SIZE))|_PAGE_NEWPAGE|_PAGE_PRESENT|_PAGE_RW|_PAGE_USER|_PAGE_ACCESSED|_PAGE_DIRTY;
  *(unsigned long *)oldpte=((unsigned long)/*__va*/(pfn*PAGE_SIZE))|_PAGE_NEWPAGE|_PAGE_PRESENT|_PAGE_RW|_PAGE_USER|_PAGE_ACCESSED|_PAGE_DIRTY;
  //printk("pte content\n");
  flush_tlb_range(pcb->mm, page, page + PAGE_SIZE);
  flush_tlb_range(oldpcb->mm, oldpage, oldpage + PAGE_SIZE);
  //printk("flush\n");
  init_p1pp_long(&ctl$gl_chindx,-4096,42);
  //printk("42\n");
  if ( (*(unsigned long*)0x7fffe000)==42)
    //printk("still 42\n");
  memset(oldpage,0,PAGE_SIZE); // must zero content also
  //printk("memset page\n");
  if ( (*(unsigned long*)0x7fffe000)==0)
    //printk("not 42\n");
#endif

  init_p1pp_data(pcb,phd,-4096);
  //printk("init p1pp data\n");
  return uml_map;
}

#define P1PP __attribute__ ((section ("p1pp")))

unsigned short ctl$gw_nmioch P1PP ;
short short_aligner;
unsigned long ctl$gl_chindx P1PP = 4;
unsigned long ctl$gl_lnmhash P1PP ;
unsigned long ctl$gl_lnmdirect P1PP ;
unsigned long ctl$al_stack[4] P1PP ;
unsigned long long ctl$gq_lnmtbl_cache P1PP ;
unsigned long ctl$gl_cmsupr P1PP ;
unsigned long ctl$gl_cmuser P1PP ;
unsigned long ctl$gl_cmhandler P1PP ;
unsigned long ctl$aq_excvec[8] P1PP ;
unsigned long ctl$gl_thexec P1PP ;
unsigned long ctl$gl_thsupr P1PP ;
unsigned long long ctl$gq_common P1PP ;
unsigned long ctl$gl_getmsg P1PP ;
unsigned long ctl$al_stacklim[4] P1PP ;
unsigned long ctl$gl_ctlbasva P1PP ;
unsigned long ctl$gl_imghdrbf P1PP ;
unsigned long ctl$gl_imglstptr P1PP ;
unsigned long ctl$gl_phd P1PP ;
unsigned long ctl$gq_allocreg[2] P1PP ;
unsigned long long ctl$gq_mountlst P1PP ;
unsigned char ctl$t_username[12] P1PP ;
unsigned char ctl$t_account[8] P1PP ;
unsigned long long ctl$gq_login P1PP ;
unsigned long ctl$gl_finalsts P1PP ;
unsigned long ctl$gl_wspeak P1PP ;
unsigned long ctl$gl_virtpeak P1PP ;
unsigned long ctl$gl_volumes P1PP ;
unsigned long long ctl$gq_istart P1PP ;
unsigned long ctl$gl_icputim P1PP ;
unsigned long long ctl$gq_ifaults P1PP ;
unsigned long ctl$gl_ifaultio P1PP ;
unsigned long ctl$gl_iwspeak P1PP ;
unsigned long ctl$gl_ipagefl P1PP ;
unsigned long ctl$gl_idiocnt P1PP ;
unsigned long ctl$gl_ibiocnt P1PP ;
unsigned long ctl$gl_ivolumes P1PP ;
unsigned char ctl$t_nodeaddr[7] P1PP ;
unsigned char ctl$t_nonename[7] P1PP ;
unsigned char ctl$t_remoteid[17] P1PP ;
unsigned char spare_for_alignment1 P1PP ;
unsigned long long ctl$gq_procpriv P1PP ;
unsigned long ctl$gl_usrchmk P1PP ;
unsigned long ctl$gl_usrchme P1PP ;
unsigned long ctl$gl_powerast P1PP ;
unsigned char ctl$gb_pwrmode P1PP ;
unsigned char ctl$gb_ssfilter P1PP ;
unsigned char ctl$gb_reenable_asts P1PP ;
unsigned char spare_for_alignment2 P1PP ;
unsigned long ctl$al_finalexc[4] P1PP ;
struct _ccb * /*unsigned long*/ ctl$gl_ccbbase P1PP ;
unsigned long long ctl$gq_dbgarea P1PP ;
unsigned long ctl$gl_rmsbase P1PP ;
unsigned long ctl$gl_ppmsg[2] P1PP ;
unsigned char ctl$gb_msgmask P1PP ;
unsigned char ctl$gb_deflang P1PP ;
unsigned short ctl$gw_ppmsgchn P1PP ;
unsigned long ctl$gl_usrundwn P1PP ;
struct _pcb * ctl$gl_pcb P1PP ;
unsigned long ctl$gl_ruf P1PP ;
unsigned long ctl$gl_sitespec P1PP ;
unsigned long ctl$gl_knownfil P1PP ;
unsigned long ctl$al_ipastvec[8] P1PP ;
unsigned long ctl$gl_cmcntx P1PP ;
unsigned long ctl$gl_aiflnkptr P1PP ;
unsigned long ctl$gl_f11bxqp P1PP ;
unsigned long long ctl$gq_p0alloc P1PP ;
unsigned long ctl$gl_prcallcnt P1PP ;
unsigned long ctl$gl_rdiptr P1PP ;
unsigned long ctl$gl_lnmdirseq P1PP ;
unsigned long long ctl$gq_helpflags P1PP ;
unsigned long long ctl$gq_termchar P1PP ;
unsigned long ctl$gl_krpfl P1PP ;
unsigned long ctl$gl_krpbl P1PP ;
unsigned long ctl$gl_creprc_flags P1PP ;
unsigned long ctl$gl_thcount[3] P1PP ;
unsigned long long ctl$gq_cwps_q1 P1PP ;
unsigned long long ctl$gq_cwps_q2 P1PP ;
unsigned long ctl$gl_cwps_l1 P1PP ;
unsigned long ctl$gl_cwps_l2 P1PP ;
unsigned long ctl$gl_cwps_l3 P1PP ;
unsigned long ctl$gl_cwps_l4 P1PP ;
unsigned long ctl$gl_prcprm_kdata2 P1PP ;
unsigned long ctl$gl_usrundwn_exec P1PP ;