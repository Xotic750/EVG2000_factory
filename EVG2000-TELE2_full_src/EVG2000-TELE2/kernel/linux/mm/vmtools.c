/*
<:copyright-gpl
 Copyright 2006 Broadcom Corp. All Rights Reserved.

 This program is free software; you can distribute it and/or modify it
 under the terms of the GNU General Public License (Version 2) as
 published by the Free Software Foundation.

 This program is distributed in the hope it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 for more details.

 You should have received a copy of the GNU General Public License along
 with this program; if not, write to the Free Software Foundation, Inc.,
 59 Temple Place - Suite 330, Boston MA 02111-1307, USA.
:>
*/

/********************************************************
    vmtools.c

    Getting memory information for memory footprint optimization

      4/18/2006  Xi Wang      Created  
     11/12/2008  Xi Wang      Ported to 2.6.21

 ********************************************************/


#include <linux/mm.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/kernel_stat.h>
#include <linux/swap.h>
#include <linux/pagemap.h>
#include <linux/init.h>
#include <linux/highmem.h>
#include <linux/file.h>
#include <linux/writeback.h>
#include <linux/suspend.h>
#include <linux/blkdev.h>
#include <linux/mm_inline.h>
#include <linux/pagevec.h>
#include <linux/backing-dev.h>
#include <linux/rmap.h>
#include <linux/topology.h>
#include <linux/cpu.h>
#include <linux/notifier.h>

#include <asm/tlbflush.h>
#include <asm/div64.h>

#include <linux/swapops.h>


int pagewalk(char *print);


// Traverse all pages in use and print out owners 
int pagewalk(char *print)
{
    #define PGS 4
    pg_data_t *pgdat;
    struct zone *zone;
    struct list_head *curlist;
    struct page *page;
    int i;
    int total=0, file_mapped=0, anon_mapped=0, unmapped=0, dirty=0;

    pgdat = NODE_DATA(0);
    zone = &pgdat->node_zones[0];

    spin_lock_irq(&zone->lru_lock);

    for (i=0; i<2; i++) {

        curlist= i?&zone->active_list:&zone->inactive_list;

        list_for_each_entry(page, curlist, lru) {
            struct address_space *a_space;
            struct inode *i_node;
            struct dentry *d_entry;
            total++;

            printk("addr:%x,    ", (unsigned int)page_address(page));

            printk("%s,    ", i? "active" : "inactive");
            
            if (page_count(page) == 0) {
                printk("free,    ");
            }
            
            else {
            
                if (page_mapped(page)) {
                
                    printk("mapped,    ");

                    if (!PageAnon(page)) { // mapped to file (code)
                        file_mapped++;
                        printk("to_file,    ");
                        a_space = page_mapping(page);
                        if (a_space) {
                            //printk("pages %d    ", (unsigned)(a_space->nrpages));
                        	i_node = a_space->host;
                            if (i_node) {
                                //printk("size %d    ", (int)(i_node->i_size));
                                d_entry = list_entry(i_node->i_dentry.next, struct dentry, d_alias);
                                if (d_entry) {
                                    printk("name:%s,    ", d_entry->d_name.name);
                                }
                            }
                        }
                    }

                    else { // anonymous (data)
                        struct anon_vma *anon_mapping;
                        struct vm_area_struct *vm_area;
                        struct mm_struct *mm;
                        struct task_struct *itask;

                        anon_mapped++;
                        printk("anonymous,    ");
                        anon_mapping = (struct anon_vma *) ((unsigned long)page->mapping - PAGE_MAPPING_ANON);
                        list_for_each_entry(vm_area, &(anon_mapping->head), anon_vma_node) {
                            mm = vm_area->vm_mm;
                            for_each_process(itask) {
                                if (itask->mm == mm || itask->active_mm == mm) {
                                  	int res = 0;
                                    unsigned int len;
                                    char buffer[256];

                                 	len = mm->arg_end - mm->arg_start;
                                 
                                	if (len > sizeof(buffer))
                                		len = sizeof(buffer);
                                 
                                	res = access_process_vm(itask, mm->arg_start, buffer, len, 0);

                                	// If the nul at the end of args has been overwritten, then
                                	// assume application is using setproctitle(3).
                                	if (res > 0 && buffer[res-1] != '\0') {
                                		len = strnlen(buffer, res);
                                		if (len < res) {
                                		    res = len;
                                		} else {
                                			len = mm->env_end - mm->env_start;
                                			if (len > sizeof(buffer) - res)
                                				len = sizeof(buffer) - res;
                                			res += access_process_vm(itask, mm->env_start, buffer+res, len, 0);
                                			res = strnlen(buffer, res);
                                		}
                                	}

                                    printk("owner:%d/%s ", itask->pid, buffer);
                                }
                            }
                        }
                    }

                }
                else {
                    unmapped++;
                    printk("unmapped (kernel mem),    ");
                }
            }

            if (PageLocked(page)) {
                printk("locked ");
            }
            if (PageDirty(page)) {
                dirty++;
                printk("dirty ");
            }
            if (PageSlab(page)) {
                printk("slab ");
            }
            if (PagePrivate(page)) {
                printk("private field used (buffer pages) ");
            }
            if (PageWriteback(page)) {
                printk("writeback ");
            }
            if (PageSwapCache(page)) {
                printk("swapcache ");
            }
            printk("\n");
        }
    }
    
	spin_unlock_irq(&zone->lru_lock);

    printk("\ntotal paged memory in use %dk    file mapped %dk    anonymously mapped %dk    unmapped %dk    dirty %dk\n", total*PGS, file_mapped*PGS, anon_mapped*PGS, unmapped*PGS, dirty*PGS);

    return 0;
 }

