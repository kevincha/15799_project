#define UPPER_ADDRESS		0xFFFF880040000000
#define LOWER_ADDRESS		0xFFFF880000000000
#define	IA32_PERF_GLOBAL_CTRL	0x38F
#define	IA32_PERFEVTSEL0	0x186
#define	IA32_PERFEVTSEL1	0x187
#define	IA32_PMC0		0x0C1
#define	IA32_PMC1		0x0C2

#define	memtest_addr_t		unsigned long long

inline void start_tsc(void){
}

inline volatile unsigned long long rd_tsc(void){
	unsigned long long result;
	asm volatile (	"rdtsc\n\t"
			"shlq $32,   %%rdx\n\t"
			"orq  %%rdx, %%rax"
			 : "=a" (result) :  : "%rdx" );
	return result;
}

inline volatile unsigned long long rd_msr(unsigned long adr){
	unsigned long long result;
	asm volatile ("rdmsr" : "=A" (result) : "c" (adr) : "memory" );
	return result;
}

inline void wr_msr(unsigned long adr, unsigned long long val){
	asm volatile ("wrmsr" : : "c" (adr), "A" (val) : "memory" );
}

inline void wb_invd(void){
	asm volatile ("wbinvd");
}

volatile unsigned long long program_address(void){
	unsigned long long pc_result;
	unsigned long long a, b, c, d;
	asm volatile (
			"pop %4\n\t"
			"pop %3\n\t"
			"pop %2\n\t"
			"pop %1\n\t"
			"pop %0\n\t"
			"push %0\n\t"
			"push %1\n\t"
			"push %2\n\t"
			"push %3\n\t"
			"push %4\n\t"
			: "=r" (pc_result),
			  "=r" (a) ,"=r" (b), "=r" (c), "=r" (d)
			 :  : "memory" );

	printk(KERN_ALERT "Program Code Address [%llX]\n", pc_result);

	return pc_result;
}

volatile unsigned long long single_access_time(int* addr1, int* addr2){
	unsigned long long result;
#if 0
	asm volatile (
			"mov (%1),%%ecx\n\t"   /* fill TLB for addr1 */
			"mov (%2),%%ecx\n\t"   /* fill TLB for addr2 */
			"mfence\n\t"           /* memory fence */
			"wbinvd\n\t"     /* write back and invalidate cache  */
			"mov (%1),%%ecx\n\t"   /* memory access(addr1) */
			"rdtsc\n\t"            /* read timestamp counter */
			"mov (%2),%%ecx\n\t"   /* memory access(addr2) */
			"shlq $32, %%rdx\n\t"  /* prepare timestamp data */ 
			"orq %%rdx,%%rax\n\t"  /* prepare timestamp data */ 
			"movq %%rax,%%rcx\n\t" /* copy timestamp */
			"rdtsc\n\t"            /* read timestamp counter */
			"shlq $32, %%rdx\n\t"  /* prepare timestamp data */ 
			"orq %%rdx,%%rax\n\t"  /* prepare timestamp data */ 
			"subq %%rcx,%%rax\n\t" /* timestamp diff */
			: "=a" (result)
			 : "a" (addr1), "b" (addr2)
			 : "%rcx", "%rdx", "memory" ); 
#endif
	asm volatile (
			"mov (%1),%%ecx\n\t"   /* fill TLB for addr1 */
			"mov (%2),%%ecx\n\t"   /* fill TLB for addr2 */
			"mfence\n\t"           /* memory fence */
			"wbinvd\n\t"     /* write back and invalidate cache  */
			"mov (%1),%%ecx\n\t"   /* memory access(addr1) */
			"mfence\n\t"           /* memory fence */
			"rdtsc\n\t"            /* read timestamp counter */
			"mfence\n\t"           /* memory fence */
			"mov (%2),%%ecx\n\t"   /* memory access(addr2) */
			"shlq $32, %%rdx\n\t"  /* prepare timestamp data */ 
			"orq %%rdx,%%rax\n\t"  /* prepare timestamp data */ 
			"movq %%rax,%%rcx\n\t" /* copy timestamp */
			"mfence\n\t"           /* memory fence */
			"rdtsc\n\t"            /* read timestamp counter */
			"shlq $32, %%rdx\n\t"  /* prepare timestamp data */ 
			"orq %%rdx,%%rax\n\t"  /* prepare timestamp data */ 
			"subq %%rcx,%%rax\n\t" /* timestamp diff */
			: "=a" (result)
			 : "a" (addr1), "b" (addr2)
			 : "%rcx", "%rdx", "memory" ); 

	return result;
}
