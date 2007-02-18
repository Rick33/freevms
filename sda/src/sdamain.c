// $Id$
// $Locker$

// Author. Roar Thron�s.

#include<stdio.h>

#include<descrip.h> 

#ifdef __i386__
int errno;
#endif

int show_device(int mask);
int show_locks(int mask);
int sda$exit(int mask);

struct cli_struct {
  char * cliroutine;
  void (*fn)();
};

struct cli_struct cliroutines[]={
  {  "sda_exit", sda$exit, }, 
  {  "show_device", show_device, }, 
  {  "show_locks", show_locks, }, 
  { 0, 0 , },
};

void * get_cli_int(char * c) {
  int i;
  for (i=0;cliroutines[i].cliroutine;i++) {
    if (0==strncmp(c,cliroutines[i].cliroutine,strlen(cliroutines[i].cliroutine)))
      return cliroutines[i].fn;
  }
  return 0;
}

int vms_mm = 1;

extern long sda_parse[];

int do_ret = 0;

main() {
  printf("\nFreeVMS system analyzer\n\n");

  while(!do_ret) {
    printf("SDA> "); 
    fflush(stdout);

    char command_str[256];
    int len = read (0, command_str, 255);

    struct dsc$descriptor command_line;
    command_line.dsc$a_pointer = command_str;
    command_line.dsc$w_length = len;

    int sts = cli$dcl_parse(&command_line, sda_parse, 0, 0, 0);
    if (sts&1) 
      cli$dispatch();
  }
}
