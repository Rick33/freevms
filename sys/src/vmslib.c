// $Id$
// $Locker$

// Author. Roar Thron�s.

#include <ssdef.h>

#include <descrip.h>

#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/mm.h>

#include <stdarg.h>

#ifdef NOKERNEL
#define printk printf
#define kfree free
#define kmalloc malloc
#endif

CH$FILL(ch, size, addr) {
  memset(addr,ch, size);
  return addr+size;
  printk("CH$FILL not implemented\n");
}

CH$RCHAR_A() {
  printk("CH$RCHAR_A not implemented\n");
}

CH$ALLOCATION() {
  printk("CH$ALLOCATION not implemented\n");
}

CH$PTR(int X) {
  return X;
}

int CH$EQL(n1,ptr1,n2,ptr2) {
  int n = ( n1 < n2 ? n1 : n2);
  return (0==memcmp(ptr1,ptr2,n));
}

int CH$NEQ(n1,ptr1,n2,ptr2) {
  int n = ( n1 < n2 ? n1 : n2);
  return (memcmp(ptr1,ptr2,n));
}

UPLIT() {
  printk("UPLIT not implemented\n");
}

PLIT() {
  printk("PLIT not implemented\n");
}

CH$MOVE(size, src, addr) {
  memcpy(addr,src, size);
  return addr+size;
  printk("CH$MOVE not implemented\n");
}

MIN(x,y) {
  return (x<y ? x :  y);
  printk("MIN not implemented\n");
}

MAX(x,y) {
  return (x>y ? x :  y);
  printk("MAX not implemented\n");
}

MINU(x,y) {
  return MIN(x,y);
  printk("MINU not implemented\n");
}

MAXU(x,y) {
  return MAX(x,y);
  printk("MAXU not implemented\n");
}

ROT(x,y) {
  return x>>y;
  printk("ROT not implemented\n");
}

CH$DIFF() {
  printk("CH$DIFF not implemented\n");
}

FORKLOCK() {
  return;
  printk("FORKLOCK not implemented\n");
}

FORKUNLOCK () {
  return;
  printk("FORKUNLOCK not implemented\n");
}

ch$move(a,b,c) {
  return CH$MOVE(a,b,c);
  printk("ch$move not implemented\n");
}

Begin_Lock() {
  printk("Begin_Lock not implemented\n");
}

CH$PLUS(x,y) {
  return x+y;
  printk("CH$PLUS not implemented\n");
}

SCH$IOLOCKW() {
  printk("SCH$IOLOCKW not implemented\n");
  return SS$_NORMAL;
}

SCH$IOUNLOCK() {
  printk("SCH$IOUNLOCK not implemented\n");
  return SS$_NORMAL;
}

UNlock_IODB() {
  printk("UNlock_IODB not implemented\n");
}

Unlock_IODB() {
  printk("Unlock_IODB not implemented\n");
}

Subm() {
  printk("Subm not implemented\n");
}

Addm() {
  printk("Addm not implemented\n");
}

End_Lock() {
  printk("End_Loc not implemented\n");
}

find_cpu_data(long * l) {
  int cpuid = smp_processor_id();
  * l=smp$gl_cpu_data[cpuid];
}

rpc_service() {
  printk("rpc_service not implemented\n");
}

exe$finish_rdb() {
  printk("exe$finish_rdb not implemented\n");
}

CH$RCHAR() {
  printk("CH$RCHAR not implemented\n");
}

//ENTRY  SWAPBYTES,^M<>
	int swapbytes(WrdCnt,Start)
{
  int *R0,*R2,*R3,*R4,*R5,*R6,*R7,*R8,*R9;
  unsigned char * R1 = Start;			// starting word address.
 Swp_Loop:
  R0 = *R1;				// low ==> temp
  *R1 = R1[1];			// high ==> low
  R1++;
  *R1++ = R0;			// temp ==> High
  if	(--WrdCnt) goto Swp_Loop;		// decr word's left to do
  return R0;
}

inline BLISSIF(int i) {
  return i&1;
}

inline BLISSIFNOT(int i) {
  return BLISSIF(i)==0;
}

inline DEVICELOCK(){
  return;
  printk("DEVICELOCK not impl\n");
}

inline DEVICEUNLOCK(){
  return;
  printk("DEVICEUNLOCK not impl\n");
}