#include <stdio.h> 
#include <ssdef.h> 
#include <descrip.h> 
#include <jpidef.h>
#include <starlet.h>

static char * states[]={"NONE","COLPG","MWAIT","CEF","PFW","LEF","LEFO","HIB","HIBO","SUSP","SUSPO","FPG","COM","COMO","CUR"};

show_system(){
int i;
struct item_list_3 lst[14];
char procname[15];
char proclen;
unsigned long upid,epid;
unsigned long upidlen,epidlen;
 unsigned long state,statelen;
 unsigned long pri,prilen;
 unsigned long pagep, pageplen;
 unsigned long pageg, pageglen;
 unsigned long pagef, pageflen;
int jpistatus;

lst[0].buflen=15;
lst[0].item_code=JPI$_PRCNAM;
lst[0].bufaddr=procname;
lst[0].retlenaddr=&proclen;
lst[1].buflen=4;
lst[1].item_code=JPI$_PID;
lst[1].bufaddr=&epid;
lst[1].retlenaddr=&epidlen;
lst[2].buflen=4;
lst[2].item_code=JPI$_MASTER_PID;
lst[2].bufaddr=&upid;
lst[2].retlenaddr=&upidlen;
lst[3].buflen=4;
lst[3].item_code=JPI$_STATE;
lst[3].bufaddr=&state;
lst[3].retlenaddr=&statelen;
lst[4].buflen=4;
lst[4].item_code=JPI$_PAGEFLTS;
lst[4].bufaddr=&pagef;
lst[4].retlenaddr=&pageflen;
lst[5].buflen=4;
lst[5].item_code=JPI$_PRI;
lst[5].bufaddr=&pri;
lst[5].retlenaddr=&prilen;
lst[6].buflen=4;
lst[6].item_code=JPI$_PPGCNT;
lst[6].bufaddr=&pagep;
lst[6].retlenaddr=&pageplen;
lst[7].buflen=4;
lst[7].item_code=JPI$_GPGCNT;
lst[7].bufaddr=&pageg;
lst[7].retlenaddr=&pageglen;
lst[8].buflen=0;
lst[8].item_code=0;
 printf(" FreeVMS V0.0  on node ABCDEF  NN-OOO-2003 PP:QQ:RR.SS  Uptime  TT XX:YY:ZZ\n");
 printf("  Pid    Process Name    State  Pri      I/O       CPU       Page flts  Pages\n");

do {
jpistatus=sys$getjpi(0,0,0,lst,0,0,0);
if (jpistatus == SS$_NORMAL)
  printf("%8x %-15s %-6s %3x %9x %17s %6x %6x\n",epid,procname,states[state],31-pri,upid,"",pagef,pagep+pageg);
} while (jpistatus == SS$_NORMAL);
//} while (jpistatus != SS$_NOMOREPROC);
}