/* Master Thesis: Operating System Support for Reconfigurable Cache
 * Student name: Vignesh Makeswaran
 * Department of Computer Science
 * University of Paderborn
 * 2015
 */

/* File: reconfig_cache.c
 * Description: This is the Reconfigurable Cache Kernel Module (RCKM). This moduel supports the Reconfigurable cache. This moduel handles all the reconfiguratble cache related kernel operations 
 * Usage info: called from context_switch().Communicated to Reconfigurable Cache User Controller (RCUC) using netlink. Manages Reconfigurable Cache Table and Core Table
 * More info: Please refer the Master thesis report for "Operating System Support for Reconfigurable Cache"
 */


/* Header Files */
/* Linux Header Files */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <asm/string.h>
#include <linux/slab.h>
#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#include <linux/stat.h>
#include <asm/uaccess.h>
#include <linux/sched.h>
#include <linux/err.h>
/* RCKM Module Header Files */
#include "reconfig_cache.h"
/* Character Device Header Files */
#include "leon_recon_ctrl_ext.h"


/* Debug definition for RCKM (please uncomment to enable debugging) */
//#define RCKM_DEBUG


/* Definitions */

/* Enable and Disable Reconfigurable Cache */
#define RC_SUPPORT_ENABLE 		1
#define RC_SUPPORT_DISABLE		0

/* Netlink port client RCUC uses the same */
#define NETLINK_USER_PROTOCOL 17

/* Length of the cache mapping values */
#define CACHE_LEN 100

/* Length of the process name */
#define NAME_LEN 50

/* Number of CPUs supported in the Core Table in RCKM */
#define NO_OF_CPU 32

/* Instruction cache and Data cache */
#define IC					0
#define DC					1

/* Set and Unset value in core tables*/
#define RCKM_SET			1
#define RCKM_UNSET			0

/* Netlink data commands supported */
#define RCKM_ADD			1
#define RCKM_DEL 			2
#define RCKM_PRINT			3
#define RCKM_ERASE			4
#define RCKM_ENABLE			5
#define RCKM_DISABLE		6

/* Code readable definitions*/
#define RCKM_FAILURE		0
#define RCKM_SUCCESS	 	1

/* Reconfigurable Cache OS support deciding variable */
int rckm_support = 0;

/* Exported functions for other modules to use*/
EXPORT_SYMBOL(rckm_next_process_info);

/* Length of the Linux default modulo mapping */
int default_cache_len = 80;

/* License statement of this module */
MODULE_LICENSE("GPL");

/* Linux default Modulo cache mapping values for vrc/srlc model */
const unsigned int default_cache[80] = {
    0xAAAAAAAA,0xAAAAAAAA,0xAAAAAAAA,0xAAAAAAAA,0xAAAAAAAA,
    0xAAAAAAAA,0xCCCCCCCC,0xAAAAAAAA,0xAAAAAAAA,0xAAAAAAAA,
    0xAAAAAAAA,0xAAAAAAAA,0xCCCCCCCC,0xAAAAAAAA,0xAAAAAAAA,
    0xAAAAAAAA,0xCCCCCCCC,0xCCCCCCCC,0xAAAAAAAA,0xAAAAAAAA,
    0xAAAAAAAA,0xAAAAAAAA,0xAAAAAAAA,0xCCCCCCCC,0xAAAAAAAA,
    0xAAAAAAAA,0xCCCCCCCC,0xAAAAAAAA,0xCCCCCCCC,0xAAAAAAAA,
    0xAAAAAAAA,0xAAAAAAAA,0xCCCCCCCC,0xCCCCCCCC,0xAAAAAAAA,
    0xAAAAAAAA,0xCCCCCCCC,0xCCCCCCCC,0xCCCCCCCC,0xAAAAAAAA,
    0xAAAAAAAA,0xAAAAAAAA,0xAAAAAAAA,0xAAAAAAAA,0xCCCCCCCC,
    0xAAAAAAAA,0xCCCCCCCC,0xAAAAAAAA,0xAAAAAAAA,0xCCCCCCCC,
    0xAAAAAAAA,0xAAAAAAAA,0xCCCCCCCC,0xAAAAAAAA,0xCCCCCCCC,
    0xAAAAAAAA,0xCCCCCCCC,0xCCCCCCCC,0xAAAAAAAA,0xCCCCCCCC,
    0xAAAAAAAA,0xAAAAAAAA,0xAAAAAAAA,0xCCCCCCCC,0xCCCCCCCC,
    0xAAAAAAAA,0xCCCCCCCC,0xAAAAAAAA,0xCCCCCCCC,0xCCCCCCCC,
    0xAAAAAAAA,0xAAAAAAAA,0xCCCCCCCC,0xCCCCCCCC,0xCCCCCCCC,
    0xAAAAAAAA,0xCCCCCCCC,0xCCCCCCCC,0xCCCCCCCC,0xCCCCCCCC
};

/* Netlink socket structure definition */
struct sock *nl_sk = NULL;

/* Core table single entry structure */
struct cpu_info
{
	int set;
	int icache_set;
	int dcache_set;
	int pid;
	char process_name[NAME_LEN];
};

/*Core Table declaration*/
struct cpu_info cpu[NO_OF_CPU];

/* Reconfigurable Cache Table single entry structure */
struct data
{
	int name_len;
    int icache_len;
    int dcache_len;
	char process_name[NAME_LEN];	
	unsigned long icache[CACHE_LEN];
	unsigned long dcache[CACHE_LEN];
};

/* Netlink message structure */
struct msg_nl
{
	int type;
	struct data data;
};

/* Reconfigurable Cache Table declaratiin*/
struct doubly_link
{
    struct data *data;
    struct doubly_link *prev;
    struct doubly_link *next;
};

/* Declaration to manage the Reconfigurable Cache Table */
typedef struct linked_list
{
    int count;
    struct doubly_link *first;
    struct doubly_link *last;
}linked_list;

/* Global variable to access the  Reconfigurable Cache Table  */
linked_list *list;

/*

Function name : rckm_link_init
Calling functions : reconfig_cache_init
Description : Called during the initialisation of RCKM, it initialises the Reconfigurable Cache Table access structure
Parameter: NIL
Return: void

*/
void rckm_link_init(void)
{

#ifdef RCKM_DEBUG
	printk(KERN_INFO "Function: %s\n", __FUNCTION__);
#endif

	/* initialisation */
    list->first = NULL;
    list->last = NULL;
    list->count = 0;

#ifdef RCKM_DEBUG
    printk(KERN_INFO "Linked list initialized \n");
#endif

}

/*

Function name : rckm_link_print
Calling functions : rckm_process_nl_msg
Description : Replies with netlink message to the RCUC for the print netlink message received from RCUC
Parameter:	process id of the RCUC		
Return: Error mesages if any. Netlink messages with process name. One message for each entry in the Reconfigurable caceh table

*/
int rckm_link_print(int pid)
{
	/* Declaration and initialisation */
    struct doubly_link *plink;
    struct nlmsghdr *nlh;
    struct sk_buff *skb_out = NULL;
    int msg_size=0;
    char msg[NAME_LEN+5];
    int ret;

#ifdef RCKM_DEBUG
	printk(KERN_INFO "Function: %s\n", __FUNCTION__);
#endif
	   
	/* Creates a netlink message with process name for each entry in the Reconfigurable Cache Table */
	for(plink = list->first; plink!=NULL; plink = plink->next)
   	{
		memset(msg,'\0',sizeof(char)*(NAME_LEN+5));
		msg[0]=RCKM_PRINT;
		msg[1]=plink->data->name_len;
#ifdef RCKM_DEBUG
		printk(KERN_INFO " %s \n",plink->data->process_name);
#endif

		strcat(msg,plink->data->process_name);		
		msg_size = strlen(msg);	
#ifdef RCKM_DEBUG
		printk(KERN_INFO " %s \n",msg);
		printk(KERN_INFO " %d \n",msg_size);
#endif	

		/* New netlink message creation */	
    	skb_out = nlmsg_new(msg_size, 0);
    	if(!skb_out)
    	{
#ifdef RCKM_DEBUG
    	    printk(KERN_ERR "Failed to allocate new skb\n");
#endif
    	    return RCKM_FAILURE;
    	}

		/* Filling the netlink message data */
    	nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0);
    	NETLINK_CB(skb_out).dst_group = 0;
    	memcpy(nlmsg_data(nlh), msg, msg_size);
		
		/* Unicasting the netlink message to the RCUC usign the input pid */
    	ret = nlmsg_unicast(nl_sk, skb_out, pid);
    	if(ret < 0)
		{
#ifdef RCKM_DEBUG
    	    printk(KERN_INFO "Error while sending back to user\n");
#endif
			return RCKM_FAILURE;
		}
#ifdef RCKM_DEBUG
		printk(KERN_INFO "Loop Check\n");
#endif
   	}
	/* Null entry netlink message to indicate end of netlink message to the RCUC */
	memset(msg,'\0',sizeof(char)*(NAME_LEN+5));
	msg[0]=RCKM_PRINT;
	msg_size = strlen(msg);	
   	skb_out = nlmsg_new(msg_size, 0);
   	if(!skb_out)
   	{
#ifdef RCKM_DEBUG
   	    printk(KERN_ERR "Failed to allocate new skb\n");
#endif
   	    return RCKM_FAILURE;
   	}
   	nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0);
   	NETLINK_CB(skb_out).dst_group = 0;
   	memcpy(nlmsg_data(nlh), msg, msg_size);

   	ret = nlmsg_unicast(nl_sk, skb_out, pid);
   	if(ret < 0)
	{
#ifdef RCKM_DEBUG
   	    printk(KERN_INFO "Error while sending bak to user\n");
#endif
	}

#ifdef RCKM_DEBUG
	printk(KERN_INFO "\n**************");
	printk(KERN_INFO "\n*PROCESS_NAME*\n");
	printk(KERN_INFO "**************\n");

	for(plink = list->first; plink!=NULL; plink = plink->next)
   	{
		printk(KERN_INFO " %s \n",plink->data->process_name);		
   	}
    printk(KERN_INFO "**************\n");
#endif
	return RCKM_SUCCESS;
}

/*

Function name : rckm_link_find
Calling functions : rckm_linked_add, rckm_next_process_info
Description : Used to find the given process name is avilable in the Recnfigurable cache table. rckm_next_process_info uses to decide whether to reconfigure the cache mapping or not. rckm_linked_add uses to decide whether the new entry should be added in the Reconfigurable Cache Table
Parameter:	structure of the reconfig, copies the values of the entry if found on the table to this structure. process name to search in the reconfigurable cache table
Return: success if found entry else failure

*/
int rckm_link_find(struct data* reconfig_data,char *next_process)
{
	/* Declaration and initialisation */
	int ret=RCKM_FAILURE;
    struct doubly_link *link;

#ifdef RCKM_DEBUG
	printk(KERN_INFO "Function: %s\n", __FUNCTION__);
#endif

	/* Get the first entry of the Reconfigurable Cache Table */
	link= list->first;
	
    while(link)
    {
		ret= memcmp(link->data->process_name,next_process,link->data->name_len);
        if(ret == 0)
        {
			/* Entry found copy entry data */
			memcpy(reconfig_data,link->data,sizeof(struct data));
			return RCKM_SUCCESS;
		}
     	link = link->next;
    }
	return RCKM_FAILURE;
}

/*

Function name : rckm_linked_add
Calling functions : rckm_process_nl_msg
Description : Adds new entry to the Reconfigurable cache table
Parameter:	netlink message	from the RCUC with add command	
Return: creates a new entry in the Reconfigurable Caceh Table if the process name is not available in the table

*/
int rckm_linked_add(struct msg_nl *nl_msg)
{
	/* Declaration and initialisation */
    struct doubly_link *link;
	struct data *data1;
	int ret = RCKM_FAILURE;

#ifdef RCKM_DEBUG
	printk(KERN_INFO "Function: %s\n", __FUNCTION__);
#endif

	/* Memory allocation to create new entry */
    link = kmalloc(sizeof(struct doubly_link),GFP_KERNEL);
    if(!link)
    {
#ifdef RCKM_DEBUG
        printk(KERN_INFO "kmalloc failed \n");
#endif
        return ret;
    }
	memset(link, 0, sizeof(link));   

    data1 = kmalloc(sizeof(struct data),GFP_KERNEL);	
    if(!data1)
    {
#ifdef RCKM_DEBUG
        printk(KERN_INFO "kmalloc failed \n");
#endif
        return ret;
    }
	memset(data1, 0, sizeof(data1));

	/* Check the process name is already available */
	ret = rckm_link_find(data1,nl_msg->data.process_name);	
	if(ret == RCKM_SUCCESS)
	{
		/* If already available free all the alocated memory and return error */
		kfree(data1);
		kfree(link);
#ifdef RCKM_DEBUG
		printk(KERN_INFO "Error: Application name already present. Please remove it before adding \n");
#endif
		ret = RCKM_FAILURE;
		return ret;
	}
	
	/* If entry not available fill the input data tot he new entry and update the linked list */
	link->prev = NULL;
	link->next = NULL;
	memcpy(data1,&nl_msg->data, sizeof(struct data));

	link->data = data1;

#ifdef RCKM_DEBUG
	printk(KERN_INFO "i_cache len : %d\n", link->data->icache_len);	

	for(ret = 0; ret < link->data->icache_len; ret++)
	printk(KERN_INFO "%x\n", (unsigned int)link->data->icache[ret]);	

	printk(KERN_INFO "d_cache len : %d\n", link->data->dcache_len);	

	for(ret = 0; ret < link->data->dcache_len; ret++)
	printk(KERN_INFO "%x\n", (unsigned int)link->data->dcache[ret]);

	ret = 0;		
#endif
	
	list->count++;
    if(list->last)
    {
		list->last->next = link;
		link->prev = list->last;
        list->last = link;
    }
    else
    {
        list->first = link;
        list->last = link;
    }
	ret = RCKM_SUCCESS;
#ifdef RCKM_DEBUG
	printk(KERN_INFO "\nlinked list entry: %d \n",list->count);        
#endif

	return ret;
}

/*

Function name : rckm_delete_one
Calling functions : rckm_delete
Description : This function used to delete one entry from the Reconfigurable Cache Table
Parameter:	pointer link to the entry to be deleted		
Return: void

*/
void rckm_delete_one(struct doubly_link *link)
{
    struct doubly_link *prev;
    struct doubly_link *next;

#ifdef RCKM_DEBUG
	printk(KERN_INFO "Function: %s\n", __FUNCTION__);
#endif

    prev = link->prev;
    next = link->next;
    if(prev)
    {
        if(next)
        {
            prev->next = next;
            next->prev = prev;
        }
        else
        {
            prev->next = NULL;
            list->last = prev;
        }
    }
    else
    {
        if(next)
        {
            next->prev = NULL;
            list->first = next;
        }
        else
        {
            list->first = NULL;
            list->last = NULL;
        }
    }

    kfree(link->data);
    kfree(link);
    list->count--;
}

/*

Function name : rckm_delete
Calling functions : rckm_process_nl_msg
Description : Called when netlink message with delete single entry is received
Parameter: netlink message data contains the process name of the entry to be deleted		
Return: success of deleted else failure

*/
int rckm_delete(struct msg_nl *nl_msg)
{
	/* Declaration and initialisation */
	int ret=RCKM_FAILURE;
	int len=0;
    struct doubly_link *link;

#ifdef RCKM_DEBUG
	printk(KERN_INFO "Function: %s\n", __FUNCTION__);
#endif

	len = nl_msg->data.name_len;

	/* Finding the entry to be delted */
    for(link = list->first; link;)
    {		
		ret= memcmp(nl_msg->data.process_name,link->data->process_name,len);
        if((ret == 0) && (link->data->name_len == len))
        {
			/* Entry found passing the entry pointer to delete */
            rckm_delete_one(link);
			return RCKM_SUCCESS;
        }
		link = link->next;
    }
	return ret;
}

/*

Function name : rckm_delete_all
Calling functions : rckm_process_nl_msg
Description : Called when netlink message with delete all entry is received
Parameter: NIL		
Return: always success

*/
int rckm_delete_all(void)
{
	/* Declaration */
    struct doubly_link *link;

#ifdef RCKM_DEBUG
	printk(KERN_INFO "Function: %s\n", __FUNCTION__);
#endif

	/* Initialisation */
	link= list->first;

	/* Freeing all the entries */	
    while(link)
    {
		kfree(link->data);
        kfree(link);
		list->count--;
     	link = link->next;
    }
	
	/* Reinitializing the Reconfigurable Cache Table */
	if(list->count == 0)
	{
#ifdef RCKM_DEBUG
		printk(KERN_INFO "\nlinked list empty\n");
#endif
		list->first = NULL;
		list->last= NULL;
	}
	
	return RCKM_SUCCESS;
}

/*

Function name : rckm_cpu_tbl_init
Calling functions : rckm_process_nl_msg
Description : This function initialises the core table when the reconfigurable cache enable command is received
Parameter: NIL			
Return: void

*/
void rckm_cpu_tbl_init(void)
{
	/* Declaration */
	int init;
	
#ifdef RCKM_DEBUG
	printk(KERN_INFO "Function: %s\n", __FUNCTION__);
#endif

	/* Initialising the Core Table */
	for(init=0;init<NO_OF_CPU;init++)
	{
		cpu[init].set = RCKM_UNSET;
		cpu[init].icache_set = RCKM_UNSET;
		cpu[init].dcache_set = RCKM_UNSET;
		cpu[init].pid = RCKM_UNSET;
		memset(cpu[init].process_name,'\0',NAME_LEN) ;
	}
}

/*

Function name : rckm_default_cache
Calling functions : rckm_process_nl_msg
Description : This function is called when disable reconfigurable cache support netlink message is received. This function resets the cache reconfigured cores to default cache mapping again
Parameter:	NIL		
Return: void

*/
void rckm_default_cache(void)
{
	/* Declaration */
	int init;   

#ifdef RCKM_DEBUG
	printk(KERN_INFO "Function: %s\n", __FUNCTION__);
#endif

	/* Check for the cache reconfigured cores*/
	for(init=0;init<NO_OF_CPU;init++)
	{	
			if(cpu[init].set == RCKM_SET)
			{	
				cpu[init].set = RCKM_UNSET;

				if(cpu[init].icache_set == RCKM_SET)
				{
					/* Reconfigure the instruction cache to default */
					cpu[init].icache_set = RCKM_UNSET;
					__chfctrl_write(IC, init,(unsigned char*)default_cache,default_cache_len);
				}

				if(cpu[init].dcache_set == RCKM_SET)
				{
					/* Reconfigure the data cache to default */
					cpu[init].dcache_set = RCKM_UNSET;
					__chfctrl_write(DC, init,(unsigned char*)default_cache,default_cache_len);
				}
				cpu[init].pid = RCKM_UNSET;
				memset(cpu[init].process_name,'\0',NAME_LEN) ;
			}
	}
}

/*

Function name : rckm_process_nl_msg
Calling functions : rckm_nl_recv_msg
Description : This function processes the netlink message data, identifies the command and calls the required function and action
Parameter:	process id of the RCUC in case to pass it to print function. netlink data structure containing the netlink data		
Return: success or failure according to the whether the input netlink message is served successfully

*/
static int rckm_process_nl_msg(struct msg_nl *msg,char *ret_msg,int pid)
{
	/* Declaration and Initialisation */
	int ret= RCKM_FAILURE;

	ret_msg[0] = msg->type;

#ifdef RCKM_DEBUG
	printk(KERN_INFO "Function: %s\n", __FUNCTION__);
#endif

	/* Identifying the message type */
	switch(msg->type)
	{
		case RCKM_ADD:
			/* Add netlink message */
#ifdef RCKM_DEBUG			
			printk(KERN_INFO "Add entry command executed\n");
#endif
			ret = rckm_linked_add(msg);
		break;
		case RCKM_DEL:
			/* Delete one entry in reconfigurable Cache Table netlink message */
#ifdef RCKM_DEBUG
			printk(KERN_INFO "Delete entry command executed\n");
#endif
			ret = rckm_delete(msg);
		break;
		case RCKM_PRINT:
			/* Print netlink message */
#ifdef RCKM_DEBUG
			printk(KERN_INFO "Print table command executed\n");
#endif
			ret = rckm_link_print(pid);
		break;
		case RCKM_ERASE:
			/* Delete all entry in the Reconfigurable Cache Table netlink message */
			ret = rckm_delete_all();
#ifdef RCKM_DEBUG
			printk(KERN_INFO "Delete table command executed\n");	
#endif
		break;
		case RCKM_ENABLE:
			/* Enable Reconfigurable Cache Support netlink message */
			if(rckm_support == RC_SUPPORT_DISABLE)
			{				
				ret = RCKM_SUCCESS;
				rckm_support = RC_SUPPORT_ENABLE ;
				rckm_cpu_tbl_init();
			}			
#ifdef RCKM_DEBUG
			printk(KERN_INFO "Enable reconfigure cache module command executed\n");
#endif
		break;
		case RCKM_DISABLE:
			/* Disable Reconfigurable Cache Support netlink message */
			if(rckm_support == RC_SUPPORT_ENABLE )
			{				
				ret = RCKM_SUCCESS;
				rckm_support = RC_SUPPORT_DISABLE;
				rckm_default_cache();
			}			
#ifdef RCKM_DEBUG
			printk(KERN_INFO "Disable reconfigure cache module command executed\n");
#endif
		break;
		default:
			/* Unidentified netlink message */
#ifdef RCKM_DEBUG
			printk(KERN_INFO "Error netlink message not identified %d\n", msg->type);
#endif
		break;
	}

    ret_msg[1] = ret;

	return ret;
}

/*

Function name : rckm_nl_recv_msg
Calling functions : netlink queue 
Description : When the netlink queue receives a message from the RCUC, this function is called
Parameter:	netlink soclet buffer		
Return: sends out reply netlink message to the RCUC according to the recevied netlink messages

*/
static void rckm_nl_recv_msg(struct sk_buff *skb)
{
	/* Declaration and initialisation */
	struct msg_nl *nl_msg;
    struct nlmsghdr *nlh;
    int pid;
    struct sk_buff *skb_out = NULL;
    int msg_size=0;
    char msg[NAME_LEN+5];
    int ret;

#ifdef RCKM_DEBUG
	printk(KERN_INFO "Function: %s\n", __FUNCTION__);
#endif

	memset(msg,'\0',sizeof(char)*(NAME_LEN+5));   
	
	/* Extract the netlink data */
    nlh = (struct nlmsghdr *)skb->data;	
    printk(KERN_INFO "Netlink Received Message from RCUC\n");
	pid = nlh->nlmsg_pid; /*pid of sending process */
    nl_msg = (struct msg_nl*)nlmsg_data(nlh);
	
	/* Send the metlink data to be processed */
    ret = rckm_process_nl_msg(nl_msg,msg,pid);    
	
	/* Createing and sending the reply netlink message */
	if(msg[0] != RCKM_PRINT)
	{
		msg_size = strlen(msg);	
    	skb_out = nlmsg_new(msg_size, 0);
    	if(!skb_out)
    	{
    	    printk(KERN_ERR "Failed to allocate new skb\n");
    	    return;
    	}
		
		/* Fill the output netlink socket buffer */
    	nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0);
    	NETLINK_CB(skb_out).dst_group = 0;
    	memcpy(nlmsg_data(nlh), msg, msg_size);
	  
		/* Unicasting netlink message to RCUC */
    	ret = nlmsg_unicast(nl_sk, skb_out, pid);
    	if(ret < 0)
		{
    	    printk(KERN_INFO "Error while sending bak to user\n");
		}
	}
}

/*

Function name : rckm_next_process_info
Calling functions : context_switch()
Description : Handles the reconfiguring the cache mapping
Parameter:	next process task structure		
Return: reconfigure the cache mapping of all protocols are satisfied

*/
void rckm_next_process_info( struct task_struct *process)
{

	/* Declaration adn initialisation */
	int pid, ret = 0;    
    struct data reconfig_data;
	unsigned int core_id;
	char process_name[NAME_LEN];
#ifdef RCKM_DEBUG
	int i;
#endif

#ifdef RCKM_DEBUG
	printk(KERN_INFO "Function: %s\n", __FUNCTION__);
#endif

	/* Check if the reconfigurable cache is supported */
	if(rckm_support == RC_SUPPORT_ENABLE)
	{
	    pid = process->pid;			
		memset(process_name,'\0',NAME_LEN);
		memcpy(process_name,process->comm, strlen(process->comm));
		core_id = task_cpu(process);
		memset(&reconfig_data,'\0',sizeof(reconfig_data));

		/* Check the process name in Reconfigurable Cache Table */
		ret = rckm_link_find(&reconfig_data,process_name);	
		if(ret == RCKM_SUCCESS)
		{
			/* Check whether the core is reconfigred previously */
			if(cpu[core_id].set == RCKM_SET)
			{	
#ifdef RCKM_DEBUG
				printk(KERN_INFO "Core id: %d\n", core_id);	
				printk(KERN_INFO "reconfig_data.name_len: %d\n", reconfig_data.name_len);
				printk(KERN_INFO "reconfig_data.cache_len: %d\n", reconfig_data.icache_len);
				printk(KERN_INFO "reconfig_data.cache_len: %d\n", reconfig_data.dcache_len);				
#endif
				/* Check the core reconfigured for given process id */
				if(cpu[core_id].pid == pid)
				{
#ifdef RCKM_DEBUG
					printk(KERN_INFO "Same core and same process id: DO NOTHING\n");
#endif
					return;
				}
				
				/* Check the core reconfigured for given process name */
				if(memcmp(cpu[core_id].process_name,process->comm,strlen(process->comm)) == 0)
				{
#ifdef RCKM_DEBUG
					printk(KERN_INFO "Same core and same process name: DO NOTHING\n");
#endif
					return;
				}
			}

			/* Update the core table for teh given core and the new process */
			cpu[core_id].pid = pid;
			memcpy(cpu[core_id].process_name,process->comm, strlen(process->comm));
			
			/* Update the given core is set */
			printk(KERN_INFO "Calling reconfigurable cache function.....\n");
			cpu[core_id].set = RCKM_SET;

			/* Check whether instruction cache to be reconfigured */
			if(reconfig_data.icache_len > 0)
			{
				printk(KERN_INFO "icache_len: %d\n", reconfig_data.icache_len);
#ifdef RCKM_DEBUG
				for(i=0;i<reconfig_data.icache_len; i++)
				printk(KERN_INFO "%x\n", (unsigned int)reconfig_data.icache[i]);	
#endif
				/* Update the given core in core table for instruction cache set */
				cpu[core_id].icache_set = RCKM_SET;
				
				/* Call charactor device to reconfigure the instruction cache mapping */
				__chfctrl_write(IC, core_id,(unsigned char*)reconfig_data.icache,reconfig_data.icache_len);
			}
			else
			{
				/* Check whether the instruction cache to be configured to default cache mapping */
				if(cpu[core_id].icache_set == RCKM_SET)
				{
					/* Update the given core in core table for instruction cache set */
					cpu[core_id].icache_set = RCKM_UNSET;

					/* Call charactor device to default configure the instruction cache mapping */
					__chfctrl_write(IC, core_id,(unsigned char*)default_cache,default_cache_len);
				}
			}
			
			/* Check whether data cache to be reconfigured */
			if(reconfig_data.dcache_len > 0)
			{
				printk(KERN_INFO "dcache_len: %d\n", reconfig_data.dcache_len);
#ifdef RCKM_DEBUG
				for(i=0;i<reconfig_data.dcache_len; i++)
				printk(KERN_INFO "%x\n", (unsigned int)reconfig_data.dcache[i]);	
#endif
				/* Update the given core in core table for data cache set */
				cpu[core_id].dcache_set = RCKM_SET;

				/* Call charactor device to reconfigure the data cache mapping */
				__chfctrl_write(DC, core_id,(unsigned char*)reconfig_data.dcache,reconfig_data.dcache_len);
			}
			else
			{
				/* Check whether the data cache to be configured to default cache mapping */
				if(cpu[core_id].dcache_set == RCKM_SET)
				{
					/* Update the given core in core table for data cache set */
					cpu[core_id].dcache_set = RCKM_UNSET;

					/* Call charactor device to default configure the data cache mapping */
					__chfctrl_write(DC, core_id,(unsigned char*)default_cache,default_cache_len);
				}
			}
		}
	}
}

/*

Function name : main
Calling functions : NIL
Description :
Parameter:			
Return:

*/
static int reconfig_cache_init(void)
{
    printk(KERN_INFO "Reconfigurable Cache Mapping Modlue init: %s\n", __FUNCTION__);
    list = kmalloc(sizeof(linked_list),GFP_KERNEL);
	if (!list)
	{
        printk(KERN_ALERT "Error allocating memory\n");
        return -ENOMEM;
	}

    rckm_link_init();

    nl_sk = netlink_kernel_create(&init_net, NETLINK_USER_PROTOCOL, 0, rckm_nl_recv_msg, NULL, THIS_MODULE);
    if(!nl_sk)
    {
        printk(KERN_ALERT "Error creating socket\n");
        return -ENOMEM;
    }

	printk(KERN_INFO "Reconfigurable Cache inserted in kernel\n");
    return 0;
}

/*

Function name : main
Calling functions : NIL
Description :
Parameter:			
Return:

*/
static void reconfig_cache_exit(void)
{

	rckm_delete_all();

    if(nl_sk)
	{
        netlink_kernel_release(nl_sk);
    }

    if(list)
	{
	kfree(list);
	}

	printk("Reconfigurable Cache Mapping Modlue exit: %s\n", __FUNCTION__);

	printk(KERN_INFO "Reconfigurable Cache removed in kernel\n");

}

/* Reconfigurable Cache Kernel Module Initialization */
module_init(reconfig_cache_init);

/* Reconfigurable Cache Kernel Module Exit */
module_exit(reconfig_cache_exit);
