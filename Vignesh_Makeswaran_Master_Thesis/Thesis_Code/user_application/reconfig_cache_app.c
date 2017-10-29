
/* Master Thesis: Operating System Support for Reconfigurable Cache
 * Student name: Vignesh Makeswaran
 * Department of Computer Science
 * University of Paderborn
 * 2015
 */

/* File: reconfig_cache_app.c
 * Description: This program, Reconfigurable Cache User Controller (RCUC) runs in the user space. Using this application, we can configure the Reconfigurable Cache Kernel Module behaviour 
 * Usage info: User input commands
 * More info: Please refer the user manual for how to use this application and/or Master thesis report for "Operating System Support for Reconfigurable Cache"
 */
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/netlink.h>

/* Macro definition for debugging (Please enable to print additional information)*/
//#define RCUC_DEBUG

/* Definitions */

/* Netlink command definition */
#define RCUC_ADD			1
#define RCUC_DEL 			2
#define RCUC_PRINT			3
#define RCUC_ERASE			4
#define RCUC_ENABLE			5
#define RCUC_DISABLE		6

/* Reconfigurable Cache support enable or disable */
#define RCUC_UNSET			0
#define RCUC_SET 			1

/* Code readable definitions*/
#define RCUC_FAILURE		0
#define RCUC_SUCCESS	 	1

/* Netlink port - same number defined in Reconfigurable Cache User Controller */
#define NETLINK_USER_PROTOCOL 17

/* Maximum length of the reconfigured cache vlaues supported (Please modify this vairable to support the required cache function length) */
#define CACHE_LEN 100

/* Maximum length of the process name supported (Please modify this vairable to support the required process name length) */
#define NAME_LEN 50


/* Reconfigurable Cache Table Structure */
struct reconf_cache_data
{
	int name_len;
    int icache_len;
    int dcache_len;
	char process_name[NAME_LEN];	
	unsigned long icache[CACHE_LEN];
	unsigned long dcache[CACHE_LEN];
};

/* Netlink message data structure */
struct data_nl
{
	int type;
	struct reconf_cache_data data;
};

/* Maximum size of the payload in netlink */
#define MAX_PAYLOAD sizeof(struct reconf_cache_data) 


/*
Function name : rcuc_nl_msg
Calling functions : rcuc_parse_config, main
Description : This function sends the netlink message to the Reconfigurable Cache  Kernel Module using the input parameter as the netlink message data
Parameter:	struct reconf_cache_data - contains the structure for adding entry to the Reconfigurable Cache Table, for other messages unused fields ignored
Return : Reply netlink message from the kernel module
*/
int rcuc_nl_msg(struct reconf_cache_data *datas, int type)
{
	/* Declaration and Initialisation */
	struct sockaddr_nl src_addr, dest_addr;
	struct nlmsghdr *nlh = NULL;
	struct iovec iov;
	int sock_user;
	struct msghdr msg;
	char buff[NAME_LEN+5];	
	int print = RCUC_SUCCESS, init =RCUC_SUCCESS;
    int ret =0, loop;
	struct data_nl nl_data;  
#ifdef RCUC_DEBUG
	int i;
#endif

	/* Creating the netlink socket */	
    sock_user = socket(PF_NETLINK, SOCK_RAW, NETLINK_USER_PROTOCOL);
    if(sock_user < 0)
	{
		printf("\nERROR: %d \n", sock_user);
        return sock_user;
	}

	/* Fill the netlink data from the input parameters */	
	nl_data.type = type;
	nl_data.data.name_len = datas->name_len;
	nl_data.data.icache_len = datas->icache_len;
	nl_data.data.dcache_len = datas->dcache_len;
	strncpy(nl_data.data.process_name, datas->process_name,50);

	for(loop=0;loop<CACHE_LEN;loop++)
	nl_data.data.icache[loop] = datas->icache[loop];

	for(loop=0;loop<CACHE_LEN;loop++)
	nl_data.data.dcache[loop] = datas->dcache[loop];

    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = getpid(); 

	/* Bind the socket to address */
    ret = bind(sock_user, (struct sockaddr *)&src_addr, sizeof(src_addr));
	if(ret < 0)
	{
		close(sock_user);
		printf("ERROR %d: Binding socket failed\n", ret);
		return ret;
	}

    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = 0; 
    dest_addr.nl_groups = 0; 

	/* Update the netlink header*/
	memset(&msg,0,sizeof(struct msghdr));
    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
    memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
    nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
    nlh->nlmsg_pid = getpid();
    nlh->nlmsg_flags = 0;

    memcpy(NLMSG_DATA(nlh),(void *)&nl_data,sizeof(struct data_nl));
    iov.iov_base = (void *)nlh;
    iov.iov_len = nlh->nlmsg_len;
    msg.msg_name = (void *)&dest_addr;
    msg.msg_namelen = sizeof(dest_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    
	/* Send the netlink message to kernel */
	printf("Sending netlink message to kernel \n");
    ret = sendmsg(sock_user, &msg, 0);
	if(ret < 0)
	{
		close(sock_user);
		printf("ERROR %d: Sending message to kernel failed %s\n", ret,strerror(errno));
		return ret;
	}
	
    /* Receive the netlink message from kernel */
	printf("Receiving netlink message from kernel \n");
    ret = recvmsg(sock_user, &msg, 0);
    if(ret < 0)
	{
		close(sock_user);
		printf("ERROR %d: Receiving message from kernel failed %s\n", ret,strerror(errno));
		return ret; 
	}

#ifdef RCUC_DEBUG
	printf("\n%d\n",nlh->nlmsg_len);
#endif    
	
	memset(buff,'\0',NAME_LEN+5);
	strcpy(buff,(char*)NLMSG_DATA(nlh));

#ifdef RCUC_DEBUG
	for(i=0;i<strlen(buff);i++)
	printf("Message from kernel: %d\n", buff[i]);
#endif

	/* Processing the kernel reply netlink message and prints the user understandable formate */
	printf("Message from Kernel:\n");
	switch(buff[0])
	{
		case RCUC_ADD:			
			if(buff[1] == RCUC_SUCCESS)
			{
				printf("Added successfully in Reconfigurable Cache Table\n");
			}
			else
			{
				printf("ERROR: Add not successfull in Reconfigurable Cache Table.\n");
				printf("Remove the entry if already present before adding\n");
			}			
		break;
		case RCUC_DEL:
			if(buff[1] == RCUC_SUCCESS)
			{
				printf("Deleted successfully in Reconfigurable Cache Table\n");
			}
			else
			{
				printf("ERROR: Delete not successfull in Reconfigurable Cache Table.\n");
				printf("Add the entry if already present before deleting\n");
			}
		break;
		case RCUC_PRINT:
			if(strlen(buff) > 1)
			{
				for(init =2;init<=(buff[1]+1);init++)
				printf("%c",buff[init]);
				printf("\n");								
			}
			else
			{
				printf("WARNING: Empty Reconfigurable Cache Table\n");
				print = RCUC_FAILURE;
			}	
			while(print == RCUC_SUCCESS)
			{				
		    	ret = recvmsg(sock_user, &msg, 0);
		    	if(ret < 0)
				{
				close(sock_user);
				printf("ERROR %d: Receiving message from kernel failed %s\n", ret,strerror(errno));
				return ret; 
				}				   
				
				memset(buff,'\0',NAME_LEN+5);
				strcpy(buff,(char*)NLMSG_DATA(nlh));		
				if(strlen(buff) > 1)
				{
					for(init =2;init<=(buff[1]+1);init++)
					printf("%c",buff[init]);
					printf("\n");					
				}
				else
				{
					print = RCUC_FAILURE;
				}
			}
		break;
		case RCUC_ERASE:
			if(buff[1] == RCUC_SUCCESS)
			{
				printf("Erased the entries in Reconfigurable Cache Table\n");
			}
			else
			{
				printf("ERROR: Problem to erase entries in Reconfigurable Cache Table.\n");
			}
			break;
		case RCUC_ENABLE:
			if(buff[1] == RCUC_SUCCESS)
			{
				printf("Successfully enabled Operating System support for Reconfigurable Cache\n");
			}
			else
			{
				printf("WARNING: Already enabled Operating System support for Reconfigurable Cache\n");
			}
		break;
		case RCUC_DISABLE:
			if(buff[1] == RCUC_SUCCESS)
			{
				printf("Successfully disabled Operating System support for Reconfigurable Cache\n");
			}
			else
			{
				printf("WARNING: Already disabled Operating System support for Reconfigurable Cache\n");
			}
		break;
		default:
			printf("Error netlink message not identified %d\n", buff[0]);
		break;
	}
	
    close(sock_user);
	
    return RCUC_SUCCESS;

}

/*

Function name : rcuc_parse_config
Calling functions : main
Description : This function validates and extracts the required data from the configuration file to add an entry to the Reconfigurable  Cache Table
Parameter:	configuration file (verify the thesis report for the configuration file formate) 		
Return: Returns success when the new entry is created. Gives the extracted data to rcuc_nl_msg to send netlink message
*/
static signed long rcuc_parse_config(unsigned char *a_config_file)
{
	/* Declaration and initialisation*/
	int i=0;
    char buff[1000];
    FILE *rcuc_config=NULL;
    int file_desc=0,ret =0,type=0;
    char *token;
    char temp[20],temp1[20];
    unsigned long no_of_process;
	int group,no_group=0,iinit=0,dinit=0;
	int iextract=0, dextract=0, process_set=0;
    struct reconf_cache_data process;
	unsigned int value=0;
	char *string;

    /* Processing a null configuration file */
    if(a_config_file != NULL)
    {
		memset(&process,'\0',sizeof(process));
        printf("Reconfigurable Cache Configuration File: %s\n", a_config_file);
        rcuc_config = fopen(a_config_file, "r");
		
        if(rcuc_config == NULL)
        {
            printf("Unable to Open file %s\n",a_config_file);

            return 0;
        }

        while(!feof(rcuc_config))
        {
			if(fgets(buff,1000, rcuc_config) == NULL)
            {

                continue;
            }

            if(buff[0] == '#'|| buff[0] =='\n')
            {

                continue;
            }

            token = strtok(buff," \n");
			
			/* Identifying the end of instruction cache values*/
            if(iextract == 1)
			{
    	       	if((!memcmp(token,"}", strlen("}"))) || (!memcmp(token,"DCACHE", strlen("DCACHE"))))
    	       	{
					iextract = 0;
					printf("\nSending icache info of Process: %s to  the kernel module\n", process.process_name);					
					process.icache_len = iinit;
    	        }
	            else
				{
                   goto icache;
				}
            }
			
			/* Identifying the end of data cache values*/
            if(dextract == 1)
			{
    	       	if((!memcmp(token,"}", strlen("}"))) || (!memcmp(token,"ICACHE", strlen("ICACHE"))))
    	       	{
					dextract = 0;
					printf("\nSending dcache info of Process: %s to  the kernel module\n", process.process_name);
					process.dcache_len = dinit;
    	        }
	            else
				{
                   goto dcache;
				}
            }

           	if(!memcmp(token,"GROUP{", strlen("GROUP{")))
           	{
				group=1;
            }
			/* Identifying the end of process name */
            if(!memcmp(token,"PROCESS", strlen("PROCESS")))
			{

                group=0;
                no_group++;
				token = strtok(NULL," ");
                if(!memcmp(token,"=", strlen("=")))
			    {
                   	while(token != '\0')
			    	{
						token = strtok(NULL,",\" \n");
						if(token)
                        {
							process.name_len = strlen(token);
                            strcpy(process.process_name,token);							                      
                        }
                   	}					
                }
            	process_set=1;
				continue;

			}
			/* Extracting the instruction cache values*/
		  	if(!memcmp(token,"ICACHE", strlen("ICACHE")))
			{
				token = strtok(NULL," ");

                if(!memcmp(token,"=", strlen("=")))
			    {
                    iextract=1;		
icache:             while(token != '\0')
			    	{							
						token = strtok(NULL," ,\" \n }\t");							
						memset(temp,'\0',20);
						if(token)
                        {
							strcpy(temp,token);
							sscanf(temp,"%x",&value);							
							process.icache[iinit] = value;
                            iinit++;

                        }
                   	}
                }
				continue;
			}
			
			/* Extracting data cache values*/
		  	if(!memcmp(token,"DCACHE", strlen("DCACHE")))
			{
				token = strtok(NULL," ");

                if(!memcmp(token,"=", strlen("=")))
			    {
                    dextract=1;		
dcache:             while(token != '\0')
			    	{	
#ifdef RCUC_DEBUG
						printf("* %s\n", token);
#endif
						token = strtok(NULL," ,\" \n }\t");		
						memset(temp1,'\0',20);
						if(token)
                        {
#ifdef RCUC_DEBUG
							printf("\nDCACHE: %d", dinit);
#endif
							strcpy(temp1,token);
							sscanf(temp1,"%x",&value);							
							process.dcache[dinit] = value;
#ifdef RCUC_DEBUG
							printf("D: %x\t", process.dcache[dinit]);							
#endif
                            dinit++;
                        }
                   	}
                }
				continue;
			}
        }
		fclose(rcuc_config);
		if((process.name_len >0) && ((process.icache_len>0)||(process.dcache_len>0)))
		{
			type = RCUC_ADD;
			rcuc_nl_msg(&process,type);
		}
		else
		{
			printf("Error: configuration file formate error. \n");
		}
    }

	return RCUC_SUCCESS;

}

/*

Function name : rcuc_usage
Calling functions : main
Description : This function briefs the input commands accepted by this RCUC application
Parameter: NIL		
Return: Prints the commands in screen and always success

*/
int rcuc_usage()
{

	printf("\nUsage:  reconfig_cache -c <configration file name> -x -r -e -d -h -p ? \n");
	printf("\n for further information pleae refer user manual\n");
	return RCUC_SUCCESS;

}


/*

Function name : main
Calling functions : NIL
Description : The main function validates the user inputs and calls the necessary functions to validate, help and create netlink messages.
Parameter:	user command line input values		
Return: always success, other calling functions provide the necessary output results

*/
int main(int argc, char *argv[])
{

	/* Variable declaration */

    signed long file_desc;
    unsigned char *config_file= NULL;
	unsigned char *process_name= NULL;
    unsigned char *module_name;
    unsigned char opt_m;
    signed long ret;
	struct reconf_cache_data data_nl;
	int type=0;
    unsigned long init;
    signed long status;
    unsigned long pid;
    unsigned char rcuc_pid[10];
    int lop= 0,i=0;
    int module_insert=RCUC_UNSET, mod_param=RCUC_FAILURE;
    char *ppm_exit_cmd[3];
	struct reconf_cache_data dummy;
    unsigned long opt_c= RCUC_UNSET,opt_x= RCUC_UNSET;
    struct option longopts[] =
    {
    {"configure",2,0,'c'},
	{"remove",2,0,'r'},
	{"enable",0,0,'e'},
	{"disable",0,0,'d'},
    {"erase",0,0,'x'},
    {"print",0,0,'p'},
    {"help",0,0,'h'},
    {NULL, 0, 0, 0}
    };

	/* Processing the user input commands */
    while ((init = getopt_long(argc, argv, "c::pr::xedh", longopts, &lop)) != EOF)
    {
        switch (init)
        {
            case 'c':
				/* Processing command to add a new entry in the Reconfigruable Cache Table */
                opt_c = RCUC_SET;
                if(argv[optind] != NULL)
                {
                    config_file = malloc(strlen(argv[optind]) + 1);
                    if(config_file == NULL)
                    {
                       printf("memory allocation error@%s Line=%d\n", __FILE__, __LINE__);
                       execv("/sbin/halt",NULL);
                    }
					memset(config_file,'\0',strlen(argv[optind]) + 1);
                    strncpy(config_file,argv[optind],strlen(argv[optind]));
                }
                else
                {
                    printf("\nConfigure File name not provided, \n Configuration option -c requires configuration files.\n Please check user manual for further details\n");
                }
	            /*Parse the configuration file*/
	            mod_param = rcuc_parse_config(config_file);
	            if(mod_param == RCUC_FAILURE)
	            {
	                if(config_file != NULL)
	                {
	                    free(config_file);
	                }
				}            
                break;

            case 'p':
				/* Processing command to print process name in Reconfigruable Cache Table*/   
				dummy.name_len=0;
    			dummy.icache_len=0;
				dummy.icache_len=0;;
				memset(dummy.process_name,'\0',50);
				strcpy(dummy.process_name,"dummy data");	
				memset(dummy.icache,'\0',100);
                for(i=0;i<100;i++)
				dummy.icache[i] = 0;
				memset(dummy.dcache,'\0',100);
				for(i=0;i<100;i++)
				dummy.dcache[i] = 0;          
				type = RCUC_PRINT;
               	rcuc_nl_msg(&dummy,type);				
                break;

            case 'r':
				/* Processing command to remove an entry in the Reconfigruable Cache Table for the given process name */
                if(argv[optind] != NULL)
                {
                    process_name = malloc(strlen(argv[optind]) + 1);
                    if(process_name == NULL)
                    {
                       printf("memory allocation error@%s Line=%d\n", __FILE__, __LINE__);
                       execv("/sbin/halt",NULL);
                    }
					memset(process_name,'\0',strlen(argv[optind]) + 1);
                    strncpy(process_name,argv[optind],strlen(argv[optind]));
                }
                else
                {
                    printf("\nProcess name to be deleted from the list not provided, \n Remove option -r requires process name.\n Please check user manual for further details\n");
                }

				dummy.name_len=strlen(process_name);
    			dummy.icache_len=0;
				dummy.dcache_len=0;
				memset(dummy.process_name,'\0',50);
				strncpy(dummy.process_name,process_name,strlen(process_name));	
				memset(dummy.icache,'\0',100);
                for(i=0;i<100;i++)
				dummy.icache[i] = 0;
				memset(dummy.dcache,'\0',100);
                for(i=0;i<100;i++)
				dummy.dcache[i] = 0;             
				type = RCUC_DEL;
               	rcuc_nl_msg(&dummy,type);
				free(process_name);				
                break;

            case 'x':  
				/* Processing command to erase all the entries in the Reconfigruable Cache Table */ 
				dummy.name_len=0;
    			dummy.icache_len=0;
				dummy.icache_len=0;;
				memset(dummy.process_name,'\0',50);
				strcpy(dummy.process_name,"dummy data");	
				memset(dummy.icache,'\0',100);
                for(i=0;i<100;i++)
				dummy.icache[i] = 0;
				memset(dummy.dcache,'\0',100);
				for(i=0;i<100;i++)
				dummy.dcache[i] = 0;          
				type = RCUC_ERASE;
               	rcuc_nl_msg(&dummy,type);				
                break;

            case 'e':
				/* Processing command to enable Operating System Support for Reconfigurable Cache  */				
				dummy.name_len=0;
    			dummy.icache_len=0;
				dummy.dcache_len = 0;
				memset(dummy.process_name,'\0',50);
				strcpy(dummy.process_name,"no data");	
				memset(dummy.icache,'\0',100);
                for(i=0;i<100;i++)
				dummy.icache[i] = 0;
				memset(dummy.dcache,'\0',100);
                for(i=0;i<100;i++)
				dummy.dcache[i] = 0;            
				type = RCUC_ENABLE;
               	rcuc_nl_msg(&dummy,type);				
                break;

            case 'd':
				/* Processing command to disable Operating System Support for Reconfigurable Cache  */
				dummy.name_len=0;
    			dummy.icache_len=0;
				dummy.dcache_len = 0;
				memset(dummy.process_name,'\0',50);
				strcpy(dummy.process_name,"no data");	
				memset(dummy.icache,'\0',100);
                for(i=0;i<100;i++)
				dummy.icache[i] = 0;
				memset(dummy.dcache,'\0',100);
                for(i=0;i<100;i++)
				dummy.dcache[i] = 0;             
				type = RCUC_DISABLE;
               	rcuc_nl_msg(&dummy,type);				
                break;

            case 'h':
            default:
				 /* Processing command wrong input or help  */
                 rcuc_usage();
                 return RCUC_SUCCESS;
        }
    }
    return RCUC_SUCCESS;
}



