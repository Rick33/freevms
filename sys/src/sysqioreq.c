// $Id$
// $Locker$

// Author. Roar Thron�s.

#include<linux/spinlock.h>

#include<acbdef.h>
#include<aqbdef.h>
#include<cdrpdef.h>
#include<ddtdef.h>
#include<fdtdef.h>
#include<internals.h>
#include<iodef.h>
#include<ipldef.h>
#include<irpdef.h>
#include<pridef.h>
#include<ssdef.h>
#include<starlet.h>
#include<system_data_cells.h>
#include<ucbdef.h>
#include<vcbdef.h>

#include<linux/vmalloc.h>
#include<linux/linkage.h>
#include<linux/kernel.h>
#include<asm/hw_irq.h>

void exe$insertirp(void * u, struct _irp * i) {
  struct _irp *tmp=((struct _irp *)u)->irp$l_ioqfl;
  while (tmp!=u && i->irp$b_pri>tmp->irp$b_pri)
    tmp=tmp->irp$l_ioqfl;
  insque(i,tmp);
}

void exe_std$abortio(struct _irp * i, struct _pcb * p, struct _ucb * u, unsigned long s) {
}

void exe$altqueuepkt (void) {
}

void exe$finishio (long long * iosb, struct _irp * i, struct _pcb * p, struct _ucb * u) {
  /* do iosb stuff */
  forklock(u->ucb$b_flck,-1);
  insque(i,&smp$gl_cpu_data[smp_processor_id()]->cpu$l_psbl);
  SOFTINT_IOPOST_VECTOR;
  forkunlock(u->ucb$b_flck,-1);
}

void exe$finishioc (long long * iosb, struct _irp * i, struct _pcb * p, struct _ucb * u) {
  *iosb&=0xffffffff00000000;
  exe$finishio(iosb,i,p,u);
}

int exe$insioq (struct _irp * i, struct _ucb * u) {
  /* raise ipl */
  int savipl=forklock(u->ucb$b_flck,u->ucb$b_flck);
  if (u->ucb$l_sts & UCB$M_BSY)
    exe$insertirp(&u->ucb$l_ioqfl,i);
  else {
    u->ucb$l_sts|=UCB$M_BSY;
    ioc$initiate(i,u);
  }
  /* restore ipl */
  /* release fork lock */
  forkunlock(u->ucb$b_flck,savipl);
  return;
}

asmlinkage int exe$qiow (struct struct_qio * q) {

  /* I think this is about it */

  int status=exe$qio(q);
  if ((status&1)==0) return status;
  return exe$synch(q->efn,q->iosb);

}

/* put this into a struct */
asmlinkage int exe$qio (struct struct_qio * q) {
  int func;
  unsigned int c, d;
  struct _pcb * p=current;
  struct _irp * i;
  exe$clref(q->efn);
  if (q->chan<0 || q->chan>ctl$gl_chindx) return SS$_IVCHAN;
  /*ccb$l_wind stuff*/
  func=q->func;
  /*spooled?*/
  /*no access check yet*/
  /*check fdt mask*/
  /*check func code*/
  /*check iosb*/
  setipl(IPL$_ASTDEL);
  /*check proc quota*/
  i=vmalloc(sizeof(struct _irp));
  bzero(i,sizeof(struct _irp));
  i->irp$b_efn=q->efn;
  i->irp$l_ast=q->astadr;
  i->irp$l_astprm=q->astprm;
  i->irp$l_iosb=q->iosb;
  i->irp$w_chan=q->chan;
  i->irp$l_func=q->func;
  i->irp$b_pri=p->pcb$b_pri;
  i->irp$l_qio_p1=q->p1;
  i->irp$l_qio_p2=q->p2;
  i->irp$l_qio_p3=q->p3;
  i->irp$l_qio_p4=q->p4;
  i->irp$l_qio_p5=q->p5;
  i->irp$l_qio_p6=q->p6;
  i->irp$l_ucb=ctl$gl_ccbbase[q->chan].ccb$l_ucb;
  i->irp$l_pid=current->pid;
  i->irp$l_sts|=IRP$M_BUFIO; /* no DIRIO because of no mmg$svaptechk */
  /* do preprocessing */
  /* does it do one or more functions */
  //  for(c=0,d=1;c<64;c++,d=d<1) /* right order? */
  //  if (d&func) {
  ctl$ga_ccb_table[q->chan].ccb$l_ucb->ucb$l_ddt->ddt$l_fdt->fdt$ps_func_rtn[func](i,p,i->irp$l_ucb,&ctl$gl_ccbbase[q->chan]); // a real beauty, isn't it :)
      //  }
}

void exe$qioacppkt (struct _irp * i, struct _pcb * p, struct _ucb * u) {
  int wasempty;
  struct _vcb * v=u->ucb$l_vcb;
  struct _aqb * a=v->vcb$l_aqb;
  wasempty=aqempty(&a->aqb$l_acpqfl);
  exe$insioq(i,&a->aqb$l_acpqfl);
  if (wasempty) return SS$_NORMAL;
  if (a->aqb$l_acppid==0) {
    exe$qioqxqppkt(p,&i->irp$l_fqfl);
    return;
  }
  sch$wake(a->aqb$l_acppid);
  /* restore ipl to 0 */
  return SS$_NORMAL;
}

int exe$qiodrvpkt (struct _irp * i, struct _pcb * p, struct _ucb * u) {

  exe$insioq(i,u);
  /* restore ipl to 0 */
  return SS$_NORMAL;
}

int exe_std$qiodrvpkt (struct _irp * i, struct _ucb * u) {
  return exe$qiodrvpkt(i,0,u);
}

extern f11b$dispatch();

void exe$qioqxqppkt (struct _pcb * p, struct _cdrp * c) {
  struct _acb *a=c;
  //  struct _f11b * f=ctl$gl_f11bxqp;

  a->acb$l_ast=f11b$dispatch;
  a->acb$l_astprm=c->cdrp$l_fqfl;
  sch$qast(p->pid,PRI$_RESAVL,a);
}


