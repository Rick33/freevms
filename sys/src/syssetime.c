// $Id$
// $Locker$

// Author. Roar Thron�s.

#include<linux/linkage.h>
#include <system_data_cells.h>	/* SYS$ header file */
#include <sys$routines.h>	/* SYS$ header file */
#include <ssdef.h>	/* SYS$ header file */

/* Author: Roar Thron�s */

asmlinkage int exe$setime(unsigned long long *timadr)
{
  long * tmp=(long *)&exe$gq_systime;
  if (!timadr) return SS$_ACCVIO;
  printk("setime %x %x\n",tmp[0],tmp[1]);
  memcpy(timadr,&exe$gq_systime,8);
  printk("setime %x %x\n",tmp[0],tmp[1]);
  return SS$_NORMAL;
}
