// $Id$
// $Locker$

// Author. Roar Thron�s.

#include<ssdef.h>

#include<linux/linkage.h>

asmlinkage int exe$cantim(unsigned long reqidt, unsigned int acmode){
  exe$rmvtimq(reqidt,acmode);
  return SS$_NORMAL;
}

asmlinkage void exe$canwak(void){
}
