#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/mm.h>
#include <linux/delay.h>
#include <asm/io.h>
//#include <unistd.h>
//#include "includes/arm.h"
#include "includes/ia64.h"

#define CPU_AFFINITY	1
#define LOOP_NUM_LOG	6
#define BIT_SHIFT	29
#define CHANGING_ACCESS		2	/* it must be 1 or 2 */
//MODULE_LICENSE("Dual BSD/GPL");
#define NUM_ELEMENT 100
#define TEST_ITERATIONS 10

#define BUFFER_LENGTH	128
#define MAX_PARAMS	3
#define ROW_SIZE (8 * 1024)
static struct proc_dir_entry *proc_entry;
static char *param_buffer;

void striding(int *row);
void zigzag(int *row1, int *row2);

static void memory_access_test(void *p) {
    int *row0_bank0; // Not necessarily 0th row and 0th bank
    int *row0_bank1;
    int *row1_bank0;

    //////////////////////////////////////////////////////////////
    // Page allocations
    if (smp_processor_id() == CPU_AFFINITY)
    {
        /*unsigned long region1 = get_zeroed_page(GFP_KERNEL);
        unsigned long region2 = get_zeroed_page(GFP_KERNEL);
        unsigned long region3 = get_zeroed_page(GFP_KERNEL);
        unsigned long region4 = get_zeroed_page(GFP_KERNEL);*/
        // Allocate a contiguous 128KB of memory region -- 2^N pages
        unsigned long region = __get_free_pages(GFP_KERNEL, 7);

        printk(KERN_ALERT "PAGE SIZE %lu", PAGE_SIZE);
        printk(KERN_ALERT "Memory start address 0x%lx\n", region);

        // Page selection
        row0_bank0 = (int *)region;
        row0_bank1 = (int *)(region + ROW_SIZE); // Skip to the fourth page (or second 8KB-row)

        printk(KERN_ALERT "Page access test started at CPUID=%d.\n", smp_processor_id());
        program_address();
        start_tsc();

        // Timing test on the same page
        printk(KERN_ALERT "Same row test\n");
        striding(row0_bank0);

        // Timing test on differen pages in the same bank
        printk(KERN_ALERT "Diff row and diff bank test\n");
        zigzag(row0_bank0, row0_bank1);

        // Timing test on differen pages
        printk(KERN_ALERT "Diff row and same bank test\n");
        // Due to XORing b/w 13th and 16th bit, we need to increment one extra row to get back to bank 0
        row1_bank0 = (int *)(region + 9 * ROW_SIZE);
        zigzag(row0_bank0, row1_bank0);

        // Done Testing -- Free those pages
        free_pages(region, 7);
        //////////////////////////////////////////////////////////////

        printk(KERN_ALERT "Acctest started[%d].\n", smp_processor_id());

        unsigned long long pre, post;
        unsigned long long diff;
        unsigned long long diffsum[BIT_SHIFT];
        unsigned long long diffmax[BIT_SHIFT], diffmin[BIT_SHIFT];
        //	memtest_addr_t ptr_org  = ACC1_BASE_ADDR;
        //	memtest_addr_t ptr_org2 = ACC2_BASE_ADDR;
        memtest_addr_t ptr_org, ptr_org2;
        memtest_addr_t ptr1, ptr2, offset;
        int i, j;

        char *fields[MAX_PARAMS];
        char *buff = (char *)p;
        int paramnum;

        for (i = 0; i < MAX_PARAMS; i++){
            fields[i] = &buff[0];
        }
        paramnum = 1;
        for (i = 0; i < BUFFER_LENGTH - 1; i++){
            if (buff[i] == ','){
                buff[i] = '\0';
                if (buff[i + 1] != '\0' && buff[i + 1] != '\n'){
                    fields[paramnum] = &buff[i + 1];
                    paramnum++;
                }
            } else if (buff[i] == '\n'){
                buff[i] = '\0';
            }
        }
        buff[BUFFER_LENGTH - 1] = '\0';

        if (kstrtoull(fields[0], 16, &ptr_org)){
            printk(KERN_ALERT "Parameter Error\n");
        }

        printk(KERN_ALERT "paramnum %d\n", paramnum);
        if (paramnum > 1){
            if (kstrtoull(fields[1], 16, &ptr_org2)){
                printk(KERN_ALERT "Parameter Error\n");
            }
        } else {
            ptr_org2 = LOWER_ADDRESS;
        }

        printk(KERN_ALERT "[%s], [%s(%016llx)], [%s], [%s]\n", (char*)p, fields[0],
                ptr_org ,fields[1], fields[2]);
        printk(KERN_ALERT "VIRTUAL  | ADDR1=%016llx ADDR2=%016llx\n", ptr_org, ptr_org2);
        printk(KERN_ALERT "PHYSICAL | ADDR1=%llx ADDR2=%llx\n", virt_to_phys((void *)ptr_org),
               virt_to_phys((void *)ptr_org2));
        printk(KERN_ALERT "Start Access Test (processor[%d])\n",
                smp_processor_id());
        program_address();
        start_tsc();

        //	diffsum[0] = 0;
        //	for (j = 0; j < (1 << LOOP_NUM_LOG); j++){
        //		diffsum[0] += no_access_time();
        //		pre  = rd_tsc();
        //		post = rd_tsc();
        //		diffsum[0] += post - pre;
        //	}
        //	printk(KERN_ALERT "No Access %llu\n", diffsum[0] >> LOOP_NUM_LOG);
        //	printk(KERN_ALERT "No Access(Without Shift) %llu\n", diffsum[0]);
        //
        printk(KERN_ALERT "After empty start\n");

        /* Initialize */
        for (i = 0; i < BIT_SHIFT; i++){
            diffsum[i] = 0;
            diffmax[i] = 0;
            diffmin[i] = 0x7FFFFFFFFFFFFFFFULL;
        }

        offset = 1;
        for (i = 0; i < BIT_SHIFT; i++){
        //for (j = 0; j < (1 << LOOP_NUM_LOG); j++){
            offset = offset << 1;
            //for (i = 0; i < BIT_SHIFT; i++){
            for (j = 0; j < (1 << LOOP_NUM_LOG); j++){
                //offset = offset << 1;
                if (CHANGING_ACCESS == 1){
                    ptr1 = ptr_org  | (offset & 0x3FFFFFFE);
                    ptr2 = ptr_org2;
                } else if (CHANGING_ACCESS == 2){
                    ptr1 = ptr_org;
                    ptr2 = ptr_org2 | (offset & 0x3FFFFFFE);
                }
                if (ptr1 >= UPPER_ADDRESS || ptr1 < LOWER_ADDRESS ||
                        ptr2 >= UPPER_ADDRESS || ptr2 < LOWER_ADDRESS){
                    break;
                }

                // This is getting the access time of the second ptr
                diff = single_access_time((int*)ptr1, (int*)ptr2);
                //printk(KERN_ALERT "IT:%d IDX:%d Time:%llu\n", j, i, diff);
                //			diff = ten_accesses_time((int*)ptr1, (int*)ptr2);
                diffsum[i] += diff;
                if (diffmax[i] < diff)
                    diffmax[i] = diff;
                if (diffmin[i] > diff)
                    diffmin[i] = diff;
            }
        }
        offset = 1;
        for (i = 0; i < BIT_SHIFT; i++){
            offset = offset << 1;
            if (CHANGING_ACCESS == 1){
                ptr1 = ptr_org  | (offset & 0x3FFFFFFE);
                ptr2 = ptr_org2;
            } else if (CHANGING_ACCESS == 2){
                ptr1 = ptr_org;
                ptr2 = ptr_org2 | (offset & 0x3FFFFFFE);
            }
            if (ptr1 >= UPPER_ADDRESS || ptr1 < LOWER_ADDRESS ||
                    ptr2 >= UPPER_ADDRESS || ptr2 < LOWER_ADDRESS){

                printk(KERN_ALERT "Invalid address[%llx], [%llx]\n",
                        ptr1, ptr2);
                break;
            }

            printk(KERN_ALERT "Acctest[%d][%X]-[%X]. bit %u %llu, %llu, %llu\n",
                    smp_processor_id(), (unsigned int)ptr1,
                    (unsigned int)ptr2, (unsigned int)i + 1, diffsum[i] >> LOOP_NUM_LOG,
                    diffmax[i], diffmin[i]);
        }

        printk(KERN_ALERT "Test(%d times) Finished.\n",
                (1 << LOOP_NUM_LOG));
    }
}

/*static int memory_access_test_all(void){
  smp_call_function(memory_access_test, NULL, 1);
  memory_access_test(NULL);
  return 0;
  }*/

// Cacheline Striding Test -- skip multiple cachelines to avoid prefetching effect
void striding(int *row)
{
    unsigned long long diff = 0;
    unsigned long long min_diff = 0;
    int stride_len = 64; // 64 integers (64*4 = 256 bytes = 4 cachelines)
    int total_it   = TEST_ITERATIONS;
    int i, it;
    printk(KERN_ALERT "Stride Test:\n");
    for (i = 0; i < 5; i++)
    {
        min_diff = 0;
        for (it = 0; it < total_it; it++)
        {
            diff = single_access_time(row+(i*stride_len), row+((i+1)*stride_len));
            //printk(KERN_ALERT "Pair index %d: time = %llu\n", i, diff);
            if (min_diff == 0 || diff < min_diff)
                min_diff = diff;
        }
        printk(KERN_ALERT "S Pair index %d: Min time = %llu\n", i, min_diff);
    }
}

// Cacheline zigzage test across two rows
void zigzag(int *row1, int *row2)
{
    unsigned long long diff = 0;
    unsigned long long min_diff = 0;
    int stride_len = 64;
    int total_it   = TEST_ITERATIONS;
    int i, it;
    printk(KERN_ALERT "Zigzag Test:\n");
    for (i = 0; i < 5; i++)
    {
        min_diff = 0;
        for (it = 0; it < total_it; it++)
        {
            diff = single_access_time(row1+(i*stride_len), row2+(i*stride_len));
            //printk(KERN_ALERT "Pair index %d: time = %llu\n", i, diff);
            if (min_diff == 0 || diff < min_diff)
                min_diff = diff;
        }
        printk(KERN_ALERT "Z Pair index %d: Min time = %llu\n", i, min_diff);
    }
}

ssize_t memtest_write(struct file *filp, const char __user *buff, unsigned long len, void *data){

    if (len > BUFFER_LENGTH) {
        printk(KERN_ALERT "memtest: buffer doesn't have enough space.");
        return -ENOSPC;
    }

    printk(KERN_ALERT "memtest: len[%lu]\n", len);
    if (copy_from_user(&param_buffer[0], buff, len)) {
        return -EFAULT;
    }

    smp_call_function(memory_access_test, param_buffer, 1);
    memory_access_test(param_buffer);

    return len;
}

static const struct file_operations proc_file_fops = {
write: memtest_write
};

static int init_test_module(void){
    param_buffer = (char*)vmalloc(BUFFER_LENGTH);
    if (!param_buffer){
        printk(KERN_ALERT "Couldn't vmalloc buffer.\n");
        return -ENOMEM;
    }

    //proc_entry = create_proc_entry("memtest", 0644, NULL);
    proc_entry = proc_create("memtest", 0666, NULL, &proc_file_fops);
    if (proc_entry == NULL){
        printk(KERN_ALERT "Couldn't create proc entry.\n");
        return -ENOMEM;
    }

    //proc_entry->write_proc = (write_proc_t *)memtest_write;

    printk(KERN_ALERT "Acctest loaded.\n");

    // Disable Intel TurboBoost and SpeedStep
    {
        uint64_t regs_old, regs;
        rdmsrl(MSR_IA32_MISC_ENABLE, regs_old);
        regs = regs_old;
        regs |= MSR_IA32_MISC_ENABLE_TURBO_DISABLE; // Disable TurboBoost
        regs &= ~MSR_IA32_MISC_ENABLE_ENHANCED_SPEEDSTEP; // Disable SpeedStep
        wrmsrl(MSR_IA32_MISC_ENABLE, regs);
        printk(KERN_ALERT "MSR_IA32_MISC_ENABLE : %llx -> %llx (TurboBoost/SpeedStep disabled)\n", regs_old, regs);
    }
    return 0;
}

static void remove_test_module(void){
    remove_proc_entry("memtest", NULL);
    vfree (param_buffer);
    printk(KERN_ALERT "Acctest removed[%d].\n", smp_processor_id());
}

module_init(init_test_module);
module_exit(remove_test_module);

