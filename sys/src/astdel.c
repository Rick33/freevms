#include <linux/linkage.h>
#include "../../freevms/sys/src/system_data_cells.h"
#include "../../freevms/starlet/src/ssdef.h"
#include "../../freevms/lib/src/pridef.h"
#include "../../freevms/lib/src/acbdef.h"
#include "../../freevms/lib/src/evtdef.h"
#include "../../freevms/lib/src/statedef.h"
#include "../../freevms/pal/src/queue.h"
#include"../../freevms/lib/src/ipldef.h"
#include"../../freevms/pal/src/ipl.h"
#include <linux/sched.h>
#include <linux/smp.h>

extern int mydebug5;

int sch$qast(unsigned long pid, int priclass, struct _acb * a) {
  int savipl;
  struct _pcb * p=find_process_by_pid(pid);
  int status;
  if (!p) {
    return SS$_NONEXPR;
  }
  /* lock */
  savipl=getipl();
  setipl(IPL$_SYNCH);
  insque(a,&p->pcb$l_astqfl);
  /* just simple insert , no pris yet */
  //printk("bef rse\n");
  if (p->pcb$w_state!=SCH$C_CUR)
    status=sch$rse(p, priclass, EVT$_AST);
  //printk("aft rse\n");
  /* unlock */
  setipl(savipl);
  return status;
}

printast(struct _acb * acb) {
  printk("acb %x %x %x %x\n",acb,acb->acb$l_pid,acb->acb$l_ast,acb->acb$l_astprm);
}

asmlinkage void sch$astdel(void) {
  struct _cpu * cpu=smp$gl_cpu_data[smp_processor_id()];
  struct _pcb * p=cpu->cpu$l_curpcb;
  struct _acb * dummy, *acb;

  /*lock*/
  if (intr_blocked(IPL$_ASTDEL))
    return;

  regtrap(REG_INTR, IPL$_ASTDEL);

 more:
  setipl(IPL$_SYNCH);

  /* { int i;
     printk("here ast\n");
     for (i=0; i<1000000; i++) ;
     } */
  if (aqempty(&p->pcb$l_astqfl)) 
    return;
  /* { int i,j;
     printk("here ast2 %x %x %x\n",p->pid,p->pcb$l_astqfl,&p->pcb$l_astqfl);
     for (j=0; j<20; j++) for (i=0; i<1000000000; i++) ;
     } */
  acb=remque(p->pcb$l_astqfl,dummy);
  printk("here ast2 %x %x %x %x\n",p->pid,p->pcb$l_astqfl,&p->pcb$l_astqfl,acb);
  printast(acb);
  //  mydebug5=1;
  //  printk(KERN_EMERG "astdel %x\n",acb);
  if (acb->acb$b_rmod & ACB$M_KAST) {
    acb->acb$b_rmod&=~ACB$M_KAST;
    /* unlock */
    printk("astdel1 %x \n",acb->acb$l_kast);
    setipl(IPL$_ASTDEL);
    acb->acb$l_kast(acb->acb$l_astprm);
    goto more;
  }
  printk("astdel2 %x %x \n",acb->acb$l_ast,acb->acb$l_astprm);
  setipl(0);
  if(acb->acb$l_ast) acb->acb$l_ast(acb->acb$l_astprm); /* ? */
  /*unlock*/
  goto more;
}


