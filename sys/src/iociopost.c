#include<linux/sched.h>
#include"../../freevms/lib/src/irpdef.h"
#include"../../freevms/lib/src/acbdef.h"

dirpost(void) { }

bufpost(void) { }

asmlinkage void ioc$iopost(void) {
  struct _irp * i;
  struct _pcb * p;
 again:
  if (!rqempty(ioc$gq_postiq)) {
    i=remqhi(ioc$gq_postiq,i);
  } else {
    return;
  }
  p=find_process_by_pid(i->irp$l_pid);
  if (i->irp$w_sts & IRP$M_BUFIO) goto bufio;

 dirio:
  //  i->irp$b_rmod|=ACB$V_KAST;
  ((struct _acb *) i)->acb$l_kast=dirpost;
  /* find other class than 1 */
  sch$postef(p->pid,1,i->irp$b_efn);
  sch$qast(p->pid,1,i);
  goto again;

 bufio:

  //  i->irp$b_rmod|=ACB$V_KAST;
  ((struct _acb *) i)->acb$l_kast=bufpost;
  /* find other class than 1 */
  sch$postef(p->pid,1,i->irp$b_efn);
  sch$qast(p->pid,1,i);
  goto again;
}

void ioc$myiopost(struct _pcb * p,unsigned long priclass) {
  sch$postef(p,priclass);
}

