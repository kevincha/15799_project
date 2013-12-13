#define UPPER_ADDRESS		0xF8000000
#define LOWER_ADDRESS		0xC0000000

#define	IA32_PERF_GLOBAL_CTRL	0x38F
#define	IA32_PERFEVTSEL0	0x186
#define	IA32_PERFEVTSEL1	0x187
#define	IA32_PMC0		0x0C1
#define	IA32_PMC1		0x0C2

inline void start_tsc(void){
}

inline unsigned long long rd_tsc(void){
	unsigned long long result;
	asm volatile ("rdtsc" : "=A" (result) : : "memory" );
	return result;
}

inline unsigned long long rd_msr(unsigned long adr){
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

unsigned int program_address(void){
	unsigned int pc_result;
	unsigned int a, b, c, d;
	asm volatile (	"pop %4\n\t"
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

	printk(KERN_ALERT "Program Code Address [%X]\n", pc_result);

	return pc_result;
}

unsigned long long single_access_time(int* addr1, int* addr2){
	unsigned long long result;
	asm volatile (
			"mov (%1),%%ecx\n\t"  /* fill TLB for addr1 */
			"mov (%2),%%ecx\n\t"  /* fill TLB for addr2 */
			"mfence\n\t"          /* memory fence */
			"wbinvd\n\t"     /* write back and invalidate cache  */
			"mov (%1),%1\n\t"     /* memory access(addr1) */
			"rdtsc\n\t"           /* read timestamp counter */
			"mov %%eax,%%ecx\n\t" /* copy timestamp */
			"mov %%edx,%%esi\n\t" /* copy timestamp */
			"mov (%2),%2\n\t"     /* memory access(addr2) */
			"rdtsc\n\t"           /* read timestamp counter */
			"mov %%esi,%%ebx\n\t" /* prepare timestamp data */
			"sub %%ecx,%%eax\n\t" /* timestamp diff */
			"sbb %%ebx,%%edx\n\t" /* timestamp diff */
			: "=A" (result)
			 : "a" (addr1), "b" (addr2)
			 : "%ecx", "%esi", "memory" ); 

	return result;
}
