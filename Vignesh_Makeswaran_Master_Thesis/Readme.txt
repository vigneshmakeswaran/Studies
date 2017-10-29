
Master Thesis Topic : Operating System Support for Reconfigurable Cache

Student Name: Vignesh Makeswaran
Matric. Number: 6672622
University of Paderborn

This software copy contains the implemented code, thesis report, testing scripts and related documents.

Folder Structure:

Vignesh_Makeswaran_Master_Thesis => This home folder
|
|
|----- Readme.txt => This file, gives introfor this softcopy and describes the folder structure.
|
|----- Thesis_Report => The folder contains the Master Thesis Document softcopy.
|            |
|            |
|            |----- Vignesh_Makeswaran_Master_Thesis.pdf => Complete report of the work done during the Master Thesis.
|
|
|----- Thesis_Code => The folder contains the implemented code for the Thesis.
           |
           |
           |----- kernel => contains all the Linux kernel modified and created files
           |         |
           |         |
           |         |----- Makefile => File belongs to  Linux 1.0.6 kernel,
           |         |                  in location linuxbuild-1.0.6/linux/linux-2.6-git/arch/sparc/kernel.
           |         |                  added code to compile the Reconfigurable Cache Kernel Module in lines  64 and 65.
           |         |
           |         |
           |         |----- reconfig_cache.c => The main Reconfigurable Cache Kernel Module file created for this thesis.
           |         |                          Location in kernel linuxbuild-1.0.6/linux/linux-2.6-git/arch/sparc/kernel.
           |         |                          The file calls a character device in another file which is not a default 
           |         |                          kernel file.
           |         |                          It was developed by the thesis supervisor Mr. Nam Ho.
           |         |
           |         |
           |         |----- reconfig_cache.h => The header file for Reconfigurable Cache Kernel Module to share functions
           |         |                          with other modules. created for this Thesis.
           |         |                          Location in kernel linuxbuild-1.0.6/linux/linux-2.6-git/arch/sparc/kernel.
           |         |
           |         |
           |         |----- sched.c => Default Linux kernel file. 
           |                           Location in kernel linuxbuild-1.0.6/linux/linux-2.6-git/kernel.
           |                           Calls the function in Reconfigurable Cache Kernel Module in line 2844 to 2846.
           |                           The only connection point between the Linux default kernel code and Reconfigurable
           |                           Cache Kernel Module.
           |       
           |
           |
           |----- testing => Testing scripts and application, all files in this folder created for this thesis.           
           |         |
           |         |----- all_test.sh => Test script to test all four applications at the same time, all core scenario.
           |         |
           |         |
           |         |----- bzip_test.sh => Test script to test hardware events for bzip using perf, single core scenario.
           |         |
           |         |
           |         |----- force_app_core.c => Test application to set affinity to run test applications on isolated cores.
           |         |
           |         |
           |         |----- jpeg_test.sh => Test script to test hardware events for jpeg using perf, single core scenario.
           |         |
           |         |
           |         |----- pat_bzip.sh => Test script to test two applications patricia and bzip using perf, multi core scenario.
           |         |
           |         |
           |         |----- patricia_test.sh => Test script to test hardware events for patricia using perf, single core scenario.
           |         |
           |         |
           |         |----- sha_test.sh => Test script to test hardware events for sha using perf, single core scenario.
           |         |
           |         |
           |         |----- time_bzip.sh => Test script to test execution, systema nd user time for bzip using time.
           |         |
           |         |
           |         |----- time_jpeg.sh => Test script to test execution, systema nd user time for jpeg using time.
           |         |
           |         |
           |         |----- time_patricia.sh => Test script to test execution, systema nd user time for patricia using time.
           |         |
           |         |
           |         |------ time_sha.sh => Test script to test execution, systema nd user time for sha using time.
           |        
           |
           |
           |----- user_application => contains the user space application for the thesis and the input configuration files. All files created for the thesis.
                     |
                     |
                     |----- bzip_icache.conf => Configuration file for bzip instruction cache.
                     |
                     |
                     |----- bzip_idcache.conf => Configuration file for bzip instruction and data cache.
                     |
                     |
                     |----- jpeg_dcache.conf => Configuration file for jpeg data cache.
                     |
                     |----- jpeg_idcache.conf => Configuration file for jpeg instruction and data cache.
                     |
                     |
                     |----- Makefile => To compile for sparc or normal system.
                     |
                     |
                     |----- patricia_icache.conf => Configuration file for patricia instruction cache.
                     |
                     |
                     |----- reconfig_cache_app.c => Reconfigurable Cache User Controller, user space application.
                     |
                     |
                     |----- reconfig_cache1.conf => Configuration file sample.
                     |
                     |
                     |----- reconfig_cache2.conf => Configuration file sample.                    
                     |
                     |
                     |----- reconfig_cache3.conf => Configuration file sample.
                     |
                     |
                     |----- sha_icache.conf => Configuration file for sha instruction cache.
                     |
                     |
                     |----- sha_idcache.conf => Configuration file for sha instruction and data cache.
                     |
                     |
                     |----- user_manual.txt => user manual for Reconfigurable Cache User Controller application.