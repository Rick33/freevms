// $Id$
// $Locker$

// Author. Roar Thron�s.

#include<linux/unistd.h>
#include<linux/linkage.h>
#include<linux/sched.h>
#include <system_data_cells.h>
#include <linux/mm.h>
#include <ipldef.h>
#include <mmgdef.h>
#include <pfndef.h>
#include <phddef.h>
#include <rdedef.h>
#include <ssdef.h>
#include <va_rangedef.h>

// exe$cntreg
// exe$cretva
// exe$deltva
// exe$expreg
// mmg$credel
// mmg$crepag
// mmg$cretva
// mmg$delpag
// mmg$fast_create
// mmg$try_all
// create_bufobj

struct _mmg mymmg;

int inline mmg$inadrini(){
}

int inline mmg$retadrini(){
}

int mmg$crepag (int acmode, void * va, struct _pcb * p, signed int pagedirection, struct _rde * rde, unsigned long newpte);

int mmg$credel(int acmode, void * first, void * last, void (* pagesubr)(), struct _va_range *inadr, struct _va_range *retadr, unsigned int acmodeagain, struct _pcb * p, int numpages) {
  unsigned long tmp=first;
  int newpte=0; // lots of zeros for demand page zero
  if (((long)first & 0x80000000) || ((long)last & 0x80000000))
    return SS$_NOPRIV;
  mymmg.mmg$l_pagesubr=pagesubr;
  while (tmp<last) {
    pagesubr(0,tmp,0,+PAGE_SIZE,newpte);      // crepag or del
    tmp=tmp+PAGE_SIZE;
  }
  return SS$_NORMAL;
}

asmlinkage int exe$cretva (struct _va_range *inadr, struct _va_range *retadr, unsigned int acmode) {
  struct _rde * rde = mmg$lookup_rde_va (inadr->va_range$ps_start_va, current->pcb$l_phd, 0, 2);
  void * first=inadr->va_range$ps_start_va;
  void * last=inadr->va_range$ps_end_va;
  struct _pcb * p=current;
  unsigned long numpages=(last-first)/PAGE_SIZE+1;
  mmg$credel(acmode, first, last, mmg$crepag, inadr, retadr, acmode, p, numpages);
}

int mmg$crepag (int acmode, void * va, struct _pcb * p, signed int pagedirection, struct _rde * rde, unsigned long newpte) {

  pgd_t *pgd;
  pmd_t *pmd;
  pte_t *pte;
  struct mm_struct * mm=current->mm;
  unsigned long address=va;
  
  if (va>=rde->rde$pq_first_free_va) {

  }

  pgd = pgd_offset(mm, address);
  pmd = pmd_alloc(mm, pgd, address);
  if (pmd) {
    pte = pte_alloc(mm, pmd, address);
  }

  if ((long)(*(long *)pte)) {
    return SS$_VA_IN_USE;
  } else {
    *(unsigned long *)pte=newpte;
  }

}

// really belongs in mmg_functions.h
inline struct _rde * mmg$lookup_rde_va (void * va, struct _phd * const phd, int function, int ipl) {
  struct _rde * head=phd->phd$ps_p0_va_list_flink;
  struct _rde * rde, *prev, *next, *newrde;
  int savipl=getipl();
  if (ipl < IPL$_ASTDEL) setipl(IPL$_ASTDEL);

  newrde = mmg$search_rde_va(va, head, &prev, &next);

  if (newrde) {
    if (ipl < IPL$_ASTDEL) setipl(savipl);
    return 0;
  }

  if (function==LOOKUP_RDE_EXACT) {
    if (ipl < IPL$_ASTDEL) setipl(savipl);
    return 0;
  }

  //else function is LOOKUP_RDE_HIGHER

  if (head->rde$v_descend) {
    if (ipl < IPL$_ASTDEL) setipl(savipl);
    return (prev);
  }

  if (va < head->rde$pq_start_va) {
    if (ipl < IPL$_ASTDEL) setipl(savipl);
    return (head);
  }

  if (next != head) {
    if (ipl < IPL$_ASTDEL) setipl(savipl);
    return (next);
  }

  head++;
  if (!head->rde$v_descend) {
    if (ipl < IPL$_ASTDEL) setipl(savipl);
    return (head); 
  }

  rde = head->rde$ps_va_list_blink;
  if (ipl < IPL$_ASTDEL) setipl(savipl);
  return (newrde);
}

// belongs same place. 
// is like find_vma?
inline struct _rde * mmg$search_rde_va (void * va, struct _rde *head, struct _rde **prev, struct _rde **next) {
  struct _rde * tmp=head;
  int dofirst=0;
  *prev=0;
  *next=head->rde$ps_va_list_flink;

  while (tmp!=*next) {
    if (!head->rde$v_descend)
      if ((tmp->rde$pq_start_va+tmp->rde$q_region_size) <= va) goto out;
    else
      if (tmp->rde$pq_start_va > va) goto out;
    *prev=tmp;
    tmp=*next;
    *next=tmp->rde$ps_va_list_flink;
  }
 out:
  if (!head->rde$v_descend) {
    if ((tmp->rde$pq_start_va+tmp->rde$q_region_size) > tmp) dofirst=1;
  } else {
    if (tmp->rde$pq_start_va <= va) dofirst=1;
  }
  if (dofirst) {
    if (tmp->rde$pq_start_va<=tmp) return tmp;
    *next=tmp;
    return 0;
  } else {
    *prev=tmp;
    return 0;
  }
}

int mmg$fast_create(struct _pcb * p, struct _rde *rde, void * start_va, void * end_va, unsigned long pages, unsigned long prot_pte) {
  unsigned long newpte;
  unsigned long page;
  unsigned long tmp=start_va;
  unsigned long new_pte;

  for(page=0;page<pages;page++) {
    newpte=prot_pte|page;
    mmg$crepag(0,tmp,0,+PAGE_SIZE,0,newpte);
    tmp+=PAGE_SIZE;
  }
}
