#define UPPER_ADDRESS		0xEE000000
#define LOWER_ADDRESS		0xC0000000

#define PMCNTNSET_C_BIT		0x80000000
#define PMCR_D_BIT		0x00000008
#define PMCR_C_BIT		0x00000004
#define PMCR_P_BIT		0x00000002
#define PMCR_E_BIT		0x00000001

#define memtest_addr_t		unsigned long

inline void start_tsc(void){
	unsigned long tmp;

	tmp = PMCNTNSET_C_BIT;
	asm volatile ("mcr p15, 0, %0, c9, c12, 1" : : "r" (tmp));


	asm volatile ("mrc p15, 0, %0, c9, c12, 0" : "=r" (tmp));
	tmp |= PMCR_C_BIT | PMCR_E_BIT;
	asm volatile ("mcr p15, 0, %0, c9, c12, 0" : : "r" (tmp));
}

inline unsigned long rd_tsc(void){
	unsigned long result;
	asm volatile ("mrc p15, 0, %0, c9, c13, 0" : "=r" (result));
	return result;
}

unsigned int program_address(void){
	unsigned int pc_result;
	asm volatile (	"mov %0, lr" : "=r" (pc_result) );

	printk(KERN_ALERT "Program Code Address [%X]\n", pc_result);

	return pc_result;
}

unsigned long long no_access_time(void){
	unsigned long result;
	asm volatile (
			"mrc p15, 0, r4, c9, c13, 0\n\t" // read TSC
//			"dsb\n\t" // Data Synchronization Barrier
			"isb\n\t" // Instruction Synchronization Barrier
			"dsb\n\t" // Data Synchronization Barrier
			"isb\n\t" // Instruction Synchronization Barrier
			"mrc p15, 0, r5, c9, c13, 0\n\t" // read TSC
			"sub %0, r5, r4\n\t" // timestamp diff
			: "=r" (result)
			 :: "r4", "r5", "cc", "memory" ); 

	return result;
}

unsigned long long single_access_time(int* addr1, int* addr2){
	unsigned long result;
//	unsigned long l2x0cinvpa = 0xF8600000 + 0x700;
	unsigned long pa1 = (unsigned long)addr1 - 0x80000000;
	unsigned long pa2 = (unsigned long)addr2 - 0x80000000;

	asm volatile (
//			"mrc p15, 0, r3, c1, c0, 0\n\t" // Disable data cache
//			"bic r3, r3, #0x1 << 2\n\t"
//			"dsb\n\t" // Data Synchronization Barrier
//			"mcr p15, 0, r3, c1, c0, 0\n\t"

			"mrs r3, cpsr\n\t" // disable irq
			"orr r3, r3, #0x80\n\t"
			"msr cpsr_c, r3\n\t"

			"ldr r3,[%1]\n\t"     // fill TLB for addr1
			"ldr r3,[%2]\n\t"     // fill TLB for addr1

//			"dsb\n\t" // Data Synchronization Barrier

			"mov r4, #0\n\t"
			"mov r3, #0xF8000000\n\t"
			"add r3, #0x00600000\n\t"
			"add r3, #0x00000700\n\t"

			"dsb\n\t" // Data Synchronization Barrier

			"mcr p15, 0, %1, c7, c10, 1\n\t" // clean L1 for addr1
			"mcr p15, 0, %2, c7, c10, 1\n\t" // clean L1 for addr2
			"dsb\n\t" // Data Synchronization Barrier

			"str %3,[r3, #0xF0]\n\t"     // clean & inv L2 for addr1
			"str %4,[r3, #0xF0]\n\t"     // clean & inv L2 for addr2
			"str r4,[r3, #0x30]\n\t"     // cache sync L2

			"mcr p15, 0, %1, c7, c14, 1\n\t" // clean & inv L1 for addr1
			"mcr p15, 0, %2, c7, c14, 1\n\t" // clean & inv L1 for addr2

			"dsb\n\t" // Data Synchronization Barrier
			"isb\n\t" // Instruction Synchronization Barrier

			"ldr r3,[%1]\n\t"     // memory access(addr1)
			"mrc p15, 0, r4, c9, c13, 0\n\t" // read TSC
//			"dsb\n\t" // Data Synchronization Barrier
			"isb\n\t" // Instruction Synchronization Barrier
			"ldr r3,[%2]\n\t"     // memory access(addr2)
			"dsb\n\t" // Data Synchronization Barrier
			"isb\n\t" // Instruction Synchronization Barrier
			"mrc p15, 0, r5, c9, c13, 0\n\t" // read TSC
			"sub %0, r5, r4\n\t" // timestamp diff

			"mrs r3, cpsr\n\t" // enable irq
			"bic r3, r3, #0x80\n\t"
			"msr cpsr_c, r3\n\t"

			: "=r" (result)
//			 : "r" (addr1), "r" (addr2), "r" (pa1), "r" (pa2), "r"(l2x0cinvpa)
			 : "r" (addr1), "r" (addr2), "r" (pa1), "r" (pa2)
			 : "r3", "r4", "r5", "cc", "memory" ); 

	return result;
}

unsigned long long ten_accesses_time(int* addr1, int* addr2){
	unsigned long result;
	asm volatile (
			"mrc p15, 0, r4, c9, c13, 0\n\t" // read TSC
//			"ldr r3,[%1]\n\t"     // fill TLB for addr1
//			"ldr r3,[%2]\n\t"     // fill TLB for addr1
			"mcr p15, 0, %1, c7, c14, 1\n\t" // invalidate cache addr1
			"ldr r3,[%1]\n\t"     // memory access(addr1)
			"mcr p15, 0, %2, c7, c14, 1\n\t" // invalidate cache addr2
			"ldr r3,[%2]\n\t"     // memory access(addr2)
			"mcr p15, 0, %1, c7, c14, 1\n\t" // invalidate cache addr1
			"ldr r3,[%1]\n\t"     // memory access(addr1)
			"mcr p15, 0, %2, c7, c14, 1\n\t" // invalidate cache addr2
			"ldr r3,[%2]\n\t"     // memory access(addr2)
			"mcr p15, 0, %1, c7, c14, 1\n\t" // invalidate cache addr1
			"ldr r3,[%1]\n\t"     // memory access(addr1)
			"mcr p15, 0, %2, c7, c14, 1\n\t" // invalidate cache addr2
			"ldr r3,[%2]\n\t"     // memory access(addr2)
			"mcr p15, 0, %1, c7, c14, 1\n\t" // invalidate cache addr1
			"ldr r3,[%1]\n\t"     // memory access(addr1)
			"mcr p15, 0, %2, c7, c14, 1\n\t" // invalidate cache addr2
			"ldr r3,[%2]\n\t"     // memory access(addr2)
			"mcr p15, 0, %1, c7, c14, 1\n\t" // invalidate cache addr1
			"ldr r3,[%1]\n\t"     // memory access(addr1)
			"mcr p15, 0, %2, c7, c14, 1\n\t" // invalidate cache addr2
			"ldr r3,[%2]\n\t"     // memory access(addr2)
			"mcr p15, 0, %1, c7, c14, 1\n\t" // invalidate cache addr1
			"ldr r3,[%1]\n\t"     // memory access(addr1)
			"mcr p15, 0, %2, c7, c14, 1\n\t" // invalidate cache addr2
			"ldr r3,[%2]\n\t"     // memory access(addr2)
			"mcr p15, 0, %1, c7, c14, 1\n\t" // invalidate cache addr1
			"ldr r3,[%1]\n\t"     // memory access(addr1)
			"mcr p15, 0, %2, c7, c14, 1\n\t" // invalidate cache addr2
			"ldr r3,[%2]\n\t"     // memory access(addr2)
			"mcr p15, 0, %1, c7, c14, 1\n\t" // invalidate cache addr1
			"ldr r3,[%1]\n\t"     // memory access(addr1)
			"mcr p15, 0, %2, c7, c14, 1\n\t" // invalidate cache addr2
			"ldr r3,[%2]\n\t"     // memory access(addr2)
			"mcr p15, 0, %1, c7, c14, 1\n\t" // invalidate cache addr1
			"ldr r3,[%1]\n\t"     // memory access(addr1)
			"mcr p15, 0, %2, c7, c14, 1\n\t" // invalidate cache addr2
			"ldr r3,[%2]\n\t"     // memory access(addr2)
			"mcr p15, 0, %1, c7, c14, 1\n\t" // invalidate cache addr1
			"ldr r3,[%1]\n\t"     // memory access(addr1)
			"mcr p15, 0, %2, c7, c14, 1\n\t" // invalidate cache addr2
			"ldr r3,[%2]\n\t"     // memory access(addr2)
			"mrc p15, 0, r5, c9, c13, 0\n\t" // read TSC
			"sub %0, r5, r4\n\t" // timestamp diff
			: "=r" (result)
			 : "r" (addr1), "r" (addr2)
			 : "r3", "r4", "r5", "cc", "memory" ); 

	return result;
}
