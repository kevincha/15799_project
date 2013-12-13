#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/mm.h>
//#include "includes/arm.h"
#include "includes/ia64.h"

#define CPU_AFFINITY	1
#define LOOP_NUM_LOG	10
#define BIT_SHIFT	29
#define CHANGING_ACCESS		2	/* it must be 1 or 2 */
//MODULE_LICENSE("Dual BSD/GPL");
#define BUFFER_LENGTH	128
#define MAX_PARAMS	3
//#define PAGE_SIZE (4 * 1024)
#define NUM_ELEMENT 100

static struct proc_dir_entry *proc_entry;
static char *param_buffer;

void striding(int *row);
void zigzag(int *row1, int *row2);

static void memory_access_test(void *p) {

    // Page allocations
    /*void *region1 = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
    void *region2 = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
    void *region3 = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
    void *region4 = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);*/
    unsigned long region1 = get_zeroed_page(GFP_KERNEL);
    unsigned long region2 = get_zeroed_page(GFP_KERNEL);
    unsigned long region3 = get_zeroed_page(GFP_KERNEL);
    unsigned long region4 = get_zeroed_page(GFP_KERNEL);
    printk(KERN_ALERT "PAGE SIZE %lu", PAGE_SIZE);

    /*if (region1 == (void *)-1)
    {
        printk(KERN_ALERT "Failed to allocated page 1.\n");
        exit(-1);
    }
    if (region2 == (void *)-1)
    {
        printk(KERN_ALERT "Failed to allocated page 2.\n");
        exit(-1);
    }
    if (region3 == (void *)-1)
    {
        printk(KERN_ALERT "Failed to allocated page 3.\n");
        exit(-1);
    }
    if (region4 == (void *)-1)
    {
        printk(KERN_ALERT "Failed to allocated page 4.\n");
        exit(-1);
    }

    printk(KERN_ALERT "E1 start address %p\n", region1);
    printk(KERN_ALERT "E2 start address %p\n", region2);
    printk(KERN_ALERT "E3 start address %p\n", region3);
    printk(KERN_ALERT "E4 start address %p\n", region4);*/

    printk(KERN_ALERT "E1 start address 0x%lx\n", region1);
    printk(KERN_ALERT "E2 start address 0x%lx\n", region2);
    printk(KERN_ALERT "E3 start address 0x%lx\n", region3);
    printk(KERN_ALERT "E4 start address 0x%lx\n", region4);

    // Page initialization
    int i;
    int *rint1;
    int *rint4;
    rint1 = (int *)region1;
    rint4 = (int *)region4;
    for (i = 0; i < NUM_ELEMENT; i++)
    {
        *(rint1+i) = i;
        *(rint4+i) = i;
    }

    if (smp_processor_id() == CPU_AFFINITY)
    {
        program_address();
        start_tsc();

        // Timing test on the same page
        striding(rint1);

        // Timing test on differen pages
        zigzag(rint1, rint4);
    }

    printk(KERN_ALERT "Acctest started[%d].\n", smp_processor_id());
    if (smp_processor_id() == CPU_AFFINITY){
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

        /* Initialize */
        for (i = 0; i < BIT_SHIFT; i++){
            diffsum[i] = 0;
            diffmax[i] = 0;
            diffmin[i] = 0x7FFFFFFFFFFFFFFFULL;
        }

        for (j = 0; j < (1 << LOOP_NUM_LOG); j++){
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
                    break;
                }

                diff = single_access_time((int*)ptr1, (int*)ptr2);
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

    free_page(region1);
    free_page(region2);
    free_page(region3);
    free_page(region4);
}

/*static int memory_access_test_all(void){
  smp_call_function(memory_access_test, NULL, 1);
  memory_access_test(NULL);
  return 0;
  }*/

// Cacheline (64B) Striding Test
void striding(int *row)
{
    unsigned long long sum_diff = 0;
    int stride_len = 16;
    int total_it   = 10;
    int i, it;
    printk(KERN_ALERT "Stride Test:\n");
    for (i = 0; i < 5; i++)
    {
        sum_diff = 0;
        for (it = 0; it < total_it; it++)
            sum_diff += single_access_time(row+(i*stride_len), row+((i+1)*stride_len));
        printk(KERN_ALERT "Pair index %d: Avg time = %llu\n", i, sum_diff / total_it);
    }
}

// Cacheline zigzage test across two rows
void zigzag(int *row1, int *row2)
{
    unsigned long long sum_diff = 0;
    int stride_len = 16;
    int total_it   = 10;
    int i, it;
    printk(KERN_ALERT "Zigzag Test:\n");
    for (i = 0; i < 5; i++)
    {
        sum_diff = 0;
        for (it = 0; it < total_it; it++)
            sum_diff += single_access_time(row1+(i*stride_len), row2+(i*stride_len));
        printk(KERN_ALERT "Pair index %d: Avg time = %llu\n", i, sum_diff / total_it);
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

static int init_test_module(void){
    param_buffer = (char*)vmalloc(BUFFER_LENGTH);
    if (!param_buffer){
        printk(KERN_ALERT "Couldn't vmalloc buffer.\n");
        return -ENOMEM;
    }

    proc_entry = create_proc_entry("memtest", 0666, NULL);
    if (proc_entry == NULL){
        printk(KERN_ALERT "Couldn't create proc entry.\n");
        return -ENOMEM;
    }

    proc_entry->write_proc = (write_proc_t *)memtest_write;

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


//module_init(memory_access_test_all);
module_init(init_test_module);
module_exit(remove_test_module);

