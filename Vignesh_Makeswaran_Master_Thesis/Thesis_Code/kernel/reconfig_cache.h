
/* Master Thesis: Operating System Support for Reconfigurable Cache
 * Student name: Vignesh Makeswaran
 * Department of Computer Science
 * University of Paderborn
 * 2015
 */

/* File: reconfig_cache.h
 * Description: This is the header file for Reconfigurable Cache Kernel Module (RCKM) 
 * Usage info: Used to enable the Reconfigurable Cache Kernel Module in other modules and declaration of the shared function with other modules
 * More info: Please refer the Master thesis report for "Operating System Support for Reconfigurable Cache"
 */



#define __RECONFIGURABLE_CACHE__

void rckm_next_process_info( struct task_struct*);







