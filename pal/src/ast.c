// $Id$
// $Locker$

// Author. Roar Thron�s.

#include<linux/linkage.h>
#include<acbdef.h>
#include<cpudef.h>
#include<linux/sched.h>
#include <asmlink.h>
#include<asm/hw_irq.h>
#include <system_data_cells.h>

int in_sw_ast;

asmlinkage void sw_ast(void) {
  struct _cpu * cpu=smp$gl_cpu_data[smp_processor_id()];
  struct _pcb * p=ctl$gl_pcb;
  //in_sw_ast=1;
  //printk("sw_ast\n");
  //{ int i; for (i=0;i<10000000;i++) ; }
  //return;
  // varm stuff here REI
  // should reaaly be if (cpu->cpu$b_cur_mod >= p->pr_astlvl) etc
  //if (p->phd$b_astlvl<4)
  //if (p->psl_is) printk("dropping sw_ast\n");
  if (!p->psl_is)
    if (p->psl_cur_mod >= p->pr_astlvl)
      SOFTINT_ASTDEL_VECTOR;
  /* check sw interrupts */
  //in_sw_ast=0;
}
