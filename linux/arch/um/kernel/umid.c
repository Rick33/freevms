#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/param.h>
#include "user.h"
#include "umid.h"
#include "init.h"

#define UMID_LEN 64
#define UML_DIR "~/.uml/"

static char umid[UMID_LEN] = { 0 };
static char *uml_dir = UML_DIR;

static int umid_inited = 0;

static int make_umid(void);

static int __init set_umid(char *name, int *add)
{
	if(umid_inited){
		printk("Unique machine name can't be set twice\n");
		return(-1);
	}

	if(strlen(name) > UMID_LEN - 1)
		printk("Unique machine name is being truncated to %s "
		       "characters\n", UMID_LEN);
	strncpy(umid, name, UMID_LEN - 1);
	umid[UMID_LEN - 1] = '\0';

	umid_inited = 1;
	return 0;
}

__uml_setup("umid=", set_umid,
"umid=<name>\n"
"    This is used to assign a unique identity to this UML machine and\n"
"    is used for naming the pid file and management console socket.\n\n"
);

int __init umid_file_name(char *name, char *buf, int len)
{
	int n;

	if(!umid_inited && make_umid()) return(-1);

	n = strlen(uml_dir) + strlen(umid) + strlen(name) + 1;
	if(n > len){
		printk("umid_file_name : buffer too short\n");
		return(-1);
	}

	sprintf(buf, "%s%s/%s", uml_dir, umid, name);
	return(0);
}

extern int tracing_pid;

static int __init create_pid_file(void)
{
	char file[strlen(uml_dir) + UMID_LEN + sizeof("/pid\0")];
	char pid[sizeof("nnnnn\0")];
	int fd;

	if(umid_file_name("pid", file, sizeof(file))) return 0;

	if((fd = open(file, O_RDWR | O_CREAT | O_EXCL, 0644)) < 0){
		printk("Open of machine pid file \"%s\" failed - "
		       "errno = %d\n", file, errno);
		return 0;
	}

	sprintf(pid, "%d\n", (tracing_pid == -1) ?  getpid() : tracing_pid);
	if(write(fd, pid, strlen(pid)) != strlen(pid))
		printk("Write of pid file failed - errno = %d\n", errno);
	close(fd);
	return 0;
}

static int actually_do_remove(char *dir)
{
	DIR *directory;
	struct dirent *ent;
	int len;
	char file[256];

	if((directory = opendir(dir)) == NULL){
		printk("actually_do_remove : couldn't open directory '%s', "
		       "errno = %d\n", dir, errno);
		return(1);
	}
	while((ent = readdir(directory)) != NULL){
		if(!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, ".."))
			continue;
		len = strlen(dir) + sizeof("/") + strlen(ent->d_name) + 1;
		if(len > sizeof(file)){
			printk("Not deleting '%s' from '%s' - name too long\n",
			       ent->d_name, dir);
			continue;
		}
		sprintf(file, "%s/%s", dir, ent->d_name);
		if(unlink(file) < 0){
			printk("actually_do_remove : couldn't remove '%s' "
			       "from '%s', errno = %d\n", ent->d_name, dir, 
			       errno);
			return(1);
		}
	}
	if(rmdir(dir) < 0){
		printk("actually_do_remove : couldn't rmdir '%s', "
		       "errno = %d\n", dir, errno);
		return(1);
	}
	return(0);
}

void remove_umid_dir(void)
{
	char dir[strlen(uml_dir) + UMID_LEN + 1];
	if(!umid_inited) return;

	sprintf(dir, "%s%s", uml_dir, umid);
	actually_do_remove(dir);
}

char *get_umid(void)
{
	return(umid);
}

int not_dead_yet(char *dir)
{
	char file[strlen(uml_dir) + UMID_LEN + sizeof("/pid\0")];
	char pid[sizeof("nnnnn\0")], *end;
	int dead, fd, p;

	sprintf(file, "%s/pid", dir);
	dead = 0;
	if((fd = open(file, O_RDONLY)) < 0){
		if(errno != ENOENT){
			printk("not_dead_yet : couldn't open pid file '%s', "
			       "errno = %d\n", file, errno);
			return(1);
		}
		dead = 1;
	}
	if(fd > 0){
		if(read(fd, pid, sizeof(pid)) < 0){
			printk("not_dead_yet : couldn't read pid file '%s', "
			       "errno = %d\n", file, errno);
			return(1);
		}
		p = strtoul(pid, &end, 0);
		if(end == pid){
			printk("not_dead_yet : couldn't parse pid file '%s', "
			       "errno = %d\n", file, errno);
			dead = 1;
		}
		if(((kill(p, 0) < 0) && (errno == ESRCH)) ||
		   (p == tracing_pid)) 
			dead = 1;
	}
	if(!dead) return(1);
	return(actually_do_remove(dir));
	return(0);
}

static int __init set_uml_dir(char *name, int *add)
{
	if((strlen(name) > 0) && (name[strlen(name) - 1] != '/')){
		uml_dir = malloc(strlen(name) + 1);
		if(uml_dir == NULL){
			printk("Failed to malloc uml_dir - error = %d\n",
			       errno);
			uml_dir = name;
			return(0);
		}
		sprintf(uml_dir, "%s/", name);
	}
	else uml_dir = name;
	return 0;
}

static int __init make_uml_dir(void)
{
	char dir[MAXPATHLEN + 1] = { '\0' };
	int len;

	if(*uml_dir == '~'){
		char *home = getenv("HOME");

		if(home == NULL){
			printk("make_uml_dir : no value in environment for "
			       "$HOME\n");
			exit(1);
		}
		strncpy(dir, home, sizeof(dir));
		uml_dir++;
	}
	len = strlen(dir);
	strncat(dir, uml_dir, sizeof(dir) - len);
	len = strlen(dir);
	if((len > 0) && (len < sizeof(dir) - 1) && (dir[len - 1] != '/')){
		dir[len] = '/';
		dir[len + 1] = '\0';
	}

	if((uml_dir = malloc(strlen(dir) + 1)) == NULL){
		printf("make_uml_dir : malloc failed, errno = %d\n", errno);
		exit(1);
	}
	strcpy(uml_dir, dir);
	
	if((mkdir(uml_dir, 0777) < 0) && (errno != EEXIST)){
	        printk("Failed to mkdir %s - errno = %i\n", uml_dir, errno);
		return(-1);
	}
	return 0;
}

static int __init make_umid(void)
{
	int fd, err;
	char tmp[strlen(uml_dir) + UMID_LEN + 1];

	strncpy(tmp, uml_dir, sizeof(tmp) - 1);
	tmp[sizeof(tmp) - 1] = '\0';

	if(*umid == 0){
		strcat(tmp, "XXXXXX");
		fd = mkstemp(tmp);
		if(fd < 0){
			printk("set_umid - mkstemp failed, errno = %d\n",
			       errno);
			return(1);
		}

		close(fd);
		/* There's a nice tiny little race between this unlink and
		 * the mkdir below.  It'd be nice if there were a mkstemp
		 * for directories.
		 */
		unlink(tmp);
		strcpy(umid, &tmp[strlen(uml_dir)]);
	}
	
	sprintf(tmp, "%s%s", uml_dir, umid);

	if((err = mkdir(tmp, 0777)) < 0){
		if(errno == EEXIST){
			if(not_dead_yet(tmp)){
				printk("umid '%s' is in use\n", umid);
				return(-1);
			}
			err = mkdir(tmp, 0777);
		}
	}
	if(err < 0){
		printk("Failed to create %s - errno = %d\n", umid, errno);
		return(-1);
	}

	return(0);
}

__uml_setup("uml_dir=", set_uml_dir,
"uml_dir=<directory>\n"
"    The location to place the pid and umid files.\n\n"
);

__uml_postsetup(make_uml_dir);
__uml_postsetup(make_umid);
__uml_postsetup(create_pid_file);

/*
 * Overrides for Emacs so that we follow Linus's tabbing style.
 * Emacs will notice this stuff at the end of the file and automatically
 * adjust the settings for this buffer only.  This must remain at the end
 * of the file.
 * ---------------------------------------------------------------------------
 * Local variables:
 * c-file-style: "linux"
 * End:
 */
