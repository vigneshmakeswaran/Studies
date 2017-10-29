
#define _GNU_SOURCE
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/time.h> // support measurement with software-based
#include <string.h>

#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sched.h>

#include <time.h>
#include <unistd.h>
#include <sys/signal.h>
#include <dirent.h>

#include <stdbool.h>


/*
 * force the calling process to run on the core
 */
int main(int argc,char *argv[])
{
	int cid,cid1;
    cpu_set_t cs;
    int err = 0, num_cpus, i;
	char cmd[100] = "", **p;
	
	if(argc < 2)
	{
		err = -1;
		printf("Error too less argument\n");
		return err;
	}

	cid = atoi(argv[1]);
	cid1 = atoi(argv[2]);
    num_cpus = sysconf( _SC_NPROCESSORS_ONLN );

    printf("Number of CPU online: %d \n", num_cpus);

    if (cid >= num_cpus) {
        printf("cid (%d) > num_cpus(%d) \n", cid, num_cpus);
        return -1;
    }
	if (cid1 >= num_cpus) {
        printf("cid1 (%d) > num_cpus(%d) \n", cid1, num_cpus);
        return -1;
    }

    err = sched_getaffinity(getpid(),sizeof(cs),&cs);
    if (err == -1) {
        printf("sched_setaffinity return error\n");
        return err;
    }

    for (i = 0; i < num_cpus; i++) {
        if (CPU_ISSET(i, &cs))
            printf("process %d can run on core %d\n", getpid(), i);
    }

    /*force to run on the cpu*/
    CPU_ZERO(&cs); CPU_SET(cid, &cs); CPU_SET(cid1, &cs);
    err = sched_setaffinity(0, sizeof(cs), &cs);
    if (err == -1) {
        printf("sched_setaffinity return error\n");
        return err;
    }

    sched_getaffinity(getpid(), sizeof(cs), &cs);
    if (err == -1) {
        printf("after forcing, sched_setaffinity return error\n");
        return err;
    }
    for (i = 0; i < 4; i++) {
        if (CPU_ISSET(i, &cs))
            printf("after forcing, process %d is running on "
            		"core %d\n", getpid(), i);
    }
	
	strcat(cmd, argv[3]);
    for (p = &argv[4]; *p; p++)
    {
    	strcat(cmd, " ");
        strcat(cmd, *p);
    }
    system(cmd);    

    return err;
}
