// $Id$
// $Locker$

// Author. Roar Thron�s.

#include <linux/sched.h>
#include <linux/smp.h>
#include <cebdef.h>
#include <cpudef.h>
#include <ipldef.h>
#include <ssdef.h>
#include <system_data_cells.h>
#include <internals.h>

int exe$wait(unsigned int efn, unsigned int mask, int waitallflag) {
  struct _pcb * p=ctl$gl_pcb;
  int efncluster=(efn&224)>>5;
  unsigned long * clusteraddr;
  struct _wqh * wq;
  setipl(IPL$_ASTDEL);
  /* no legal check yet */
  if (efn>127)
    return SS$_ILLEFC;
  clusteraddr=getefc(p,efn);
  p->pcb$b_wefc=efncluster;
  if (efncluster>1 && !clusteraddr)
    return SS$_UNASEFC;

  vmslock(&SPIN_SCHED,IPL$_SCHED);

#if 0
  if (efncluster==0 || efncluster==1) goto notcommon;
  /* not impl */
  printk("in a wait non impl routine %x %x %x %x\n",efn,mask,clusteraddr,efncluster);
  return;
 notcommon:
#endif

  if (efncluster<2) {
    wq=sch$gq_lefwq;
    /* do */
  } else {
    unsigned long * l=getefcp(p,efn);
    wq=&((struct _ceb *)(*l))->ceb$l_wqfl;
  }

  if (!waitallflag && (mask & *clusteraddr)) {
  out:
    /* unlock sched */
    return SS$_NORMAL;
  }
  if (waitallflag && ((mask & *clusteraddr) == mask))
    goto out;
  if (waitallflag) {
    p->pcb$l_sts|=PCB$M_WALL;
    p->pcb$l_efwm|=~mask;
  } else 
    p->pcb$l_efwm|=~mask;

  insque(p,&wq->wqh$l_wqfl); // temporary... see about corruption in rse.c
  sch$wait(p,wq);
  return SS$_NORMAL;
}

int exe$waitfr(unsigned int efn) {
  return exe$wait(efn,1<<(efn&31),0);
}

int exe$wflor(unsigned int efn, unsigned int mask) {
  return exe$wait(efn,mask,0);
}

int exe$wfland(unsigned int efn, unsigned int mask) {
  return exe$wait(efn,mask,1);
}
