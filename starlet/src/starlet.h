#ifndef starlet_h
#define starlet_h

#include"misc.h"
#include"lksbdef.h"

/* rename eventually to sys$routines.h */

int sys$testcode(void); 

int sys$setprn  ( void *prcnam);

int sys$setpri(unsigned int *pidadr, void *prcnam, unsigned int pri, unsigned int *prvpri, unsigned int*pol, unsigned int *prvpol);

int sys$dclast( void (*astadr)(unsigned long), unsigned long astprm, unsigned int acmode);

int sys$waitfr(unsigned int efn);

int sys$wfland(unsigned int efn, unsigned int mask);

int sys$wflor  (unsigned int efn, unsigned int mask);

int sys$clref  (unsigned int efn);

int sys$setime  (unsigned long long  *timadr);

int sys$setimr  (unsigned int efn, signed long long *daytim,
		 void (*astadr)(long), unsigned
		 long reqidt, unsigned int flags);

int sys$cantim  (unsigned long long reqidt, unsigned int acmode);

int sys$numtim  (unsigned short int timbuf [7], unsigned long long * timadr);

int sys$gettim (unsigned long long * timadr);

int sys$schdwk(unsigned int *pidadr, void *prcnam, signed long long * daytim, signed long long * reptim);

int sys$resume (unsigned int *pidadr, void *prcnam);

int sys$exit(unsigned int code);

int sys$forcex(unsigned int *pidadr, void *prcnam, unsigned int code);

int sys$setef(unsigned int efn);

int sys$synch(unsigned int efn, struct _iosb *iosb);

int sys$readef(unsigned int efn, unsigned int *state);

int sys$enqw  (unsigned int efn, unsigned int lkmode, struct _lksb *lksb, unsigned int flags, void *resnam, unsigned int parid, void (*astadr)(), unsigned long astprm, void (*blkastadr)(), unsigned int acmode, unsigned int rsdm_id);

int sys$enq  (unsigned int efn, unsigned int lkmode, struct _lksb *lksb, unsigned int flags, void *resnam, unsigned int parid, void (*astadr)(), unsigned long astprm, void (*blkastadr)(), unsigned int acmode, unsigned int rsdm_id);

int sys$deq(unsigned int lkid, void *valblk, unsigned int acmode, unsigned int flags);

int sys$getlki(unsigned int efn, unsigned int *lkidadr,void *itmlst, struct _iosb *iosb, void (*astadr)(int), int astprm, unsigned int reserved);

int sys$getlkiw(unsigned int efn, unsigned int *lkidadr,void *itmlst, struct _iosb *iosb, void (*astadr)(int), int astprm, unsigned int reserved);

int sys$asctim  (unsigned short int *timlen, void *timbuf,
		 unsigned long long *timadr, char cvtflg);

int sys$bintim  (void *timbuf, unsigned long long *timadr);

int sys$crelnm  (unsigned int *attr, void *tabnam, void *lognam, unsigned char *acmode, void *itmlst);

int sys$crelnt  (unsigned int *attr, void *resnam, unsigned
                         int *reslen, unsigned int *quota,
                unsigned short *promsk, void *tabnam, void
                         *partab, unsigned char *acmode);

int sys$dellnm  (void *tabnam, void *lognam, unsigned char *acmode);

int sys$trnlnm  (unsigned int *attr, void *tabnam, void
		  *lognam, unsigned char *acmode, void *itmlst);

int sys$dassgn(unsigned short int chan);

int sys$assign(void *devnam, unsigned short int *chan,unsigned int acmode, void *mbxnam,int flags);

struct _iosb;

int sys$qiow(unsigned int efn, unsigned short int chan,unsigned int func, struct _iosb *iosb, void(*astadr)(__unknown_params), long  astprm, void*p1, long p2, long  p3, long p4, long p5, long p6);
int sys$qio(unsigned int efn, unsigned short int chan,unsigned int func, struct _iosb *iosb, void(*astadr)(__unknown_params), long  astprm, void*p1, long p2, long  p3, long p4, long p5, long p6);

int sys$clrast(void);

int sys$setast(char enbflg);

int sys$ascefc(unsigned int efn, void *name, char prot, char perm); 

int sys$dacefc(unsigned int efn);

int sys$dlcefc(void *name);

int exe$crembx  (char prmflg, unsigned short int *chan, unsigned int maxmsg, unsigned int bufquo, unsigned int promsk, unsigned int acmode, void *lognam,...);

int exe$delmbx  (unsigned short int chan);

struct _fab;

int exe$close (struct _fab * fab, void * err, void * suc);
int exe$connect (struct _fab * fab, void * err, void * suc);
int exe$create (struct _fab * fab, void * err, void * suc);
int exe$delete (struct _fab * fab, void * err, void * suc);
int exe$disconnect (struct _fab * fab, void * err, void * suc);
int exe$display (struct _fab * fab, void * err, void * suc);
int exe$enter (struct _fab * fab, void * err, void * suc);
int exe$erase (struct _fab * fab, void * err, void * suc);
int exe$extend (struct _fab * fab, void * err, void * suc);
int exe$find (struct _fab * fab, void * err, void * suc);
int exe$flush (struct _fab * fab, void * err, void * suc);
int exe$free (struct _fab * fab, void * err, void * suc);
int exe$get (struct _fab * fab, void * err, void * suc);
int exe$modify (struct _fab * fab, void * err, void * suc);
int exe$nxtvol (struct _fab * fab, void * err, void * suc);
int exe$open (struct _fab * fab, void * err, void * suc);
int exe$parse (struct _fab * fab, void * err, void * suc);
int exe$put (struct _fab * fab, void * err, void * suc);
int exe$read (struct _fab * fab, void * err, void * suc);
int exe$release (struct _fab * fab, void * err, void * suc);
int exe$remove (struct _fab * fab, void * err, void * suc);
int exe$rename (struct _fab * fab, void * err, void * suc);
int exe$rewind (struct _fab * fab, void * err, void * suc);
int exe$search (struct _fab * fab, void * err, void * suc);
int exe$space (struct _fab * fab, void * err, void * suc);
int exe$truncate (struct _fab * fab, void * err, void * suc);
int exe$update (struct _fab * fab, void * err, void * suc);
int exe$wait (struct _fab * fab, void * err, void * suc);
int exe$write (struct _fab * fab, void * err, void * suc);
int exe$filescan (struct _fab * fab, void * err, void * suc);
int exe$setddir (struct _fab * fab, void * err, void * suc);
int exe$setdfprot (struct _fab * fab, void * err, void * suc);
int exe$ssvexc (struct _fab * fab, void * err, void * suc);
int exe$rmsrundwn (struct _fab * fab, void * err, void * suc);

struct struct_crelnt {
 unsigned int *attr;
 void *resnam;
 unsigned int *reslen;
 unsigned int *quota;
 unsigned short *promsk;
 void *tabnam;
 void *partab;
 unsigned char *acmode;
};

struct struct_setpri {
unsigned int *pidadr;
void *prcnam;
 unsigned int pri;
 unsigned int *prvpri;
 unsigned int*pol;
 unsigned int *prvpol;
};

struct struct_qio {
  unsigned int efn;
  unsigned short int chan;
  unsigned int func;
  struct _iosb *iosb;
  void (*astadr)(long);
  long astprm;
  void *p1; 
  long p2;
  long p3;
  long p4;
  long p5;
  long p6;
};

struct struct_enq {
  unsigned int efn;
  unsigned int lkmode;
  struct _lksb *lksb;
  unsigned int flags;
  void *resnam;
  unsigned int parid;
  void (*astadr)();
  unsigned long astprm;
  void (*blkastadr)();
  unsigned int acmode;
  unsigned int rsdm_id;
  unsigned long null_arg;
};

struct struct_getlki {
  unsigned int efn;
  unsigned int *lkidadr;
  void *itmlst;
  struct _iosb *iosb;
  void (*astadr)(int);
  int astprm;
  unsigned int reserved;
};

struct struct_crembx {
 char prmflg;
 unsigned short int *chan;
 unsigned int maxmsg;
 unsigned int bufquo;
 unsigned int promsk;
 unsigned int acmode;
  void *lognam;
};

#endif


