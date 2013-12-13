Quick memo for memtest.ko (hyoseung)

1) load memtest.ko kernel module

2) sudo su

3) echo 0xFFFF880000000000,0xFFFF880000000000 > /proc/memtest
-> last column of each row shows the minimum access time
-> find the region where each row has a distint access time 
ex) Acctest[1][0]-[1000]. bit 12 268, 1400, 262
    Acctest[1][0]-[2000]. bit 13 268, 1400, 264
    Acctest[1][0]-[4000]. bit 14 309, 1400, 304  <-  access time jump
	Acctest[1][0]-[8000]. bit 15 309, 1400, 305
	Acctest[1][0]-[10000]. bit 16 308, 1400, 301
	...
	Acctest[1][0]-[100000]. bit 20 308, 1400, 302
	Acctest[1][0]-[200000]. bit 21 350, 1400, 342 <- access time jump
	
	--> bank(and rank) bit region: bit 14 - 20  
	--> note1: The bit index starts from 0)
	--> note2: The actual number of bits for bank/rank indexing
	           is smaller than that of the region above,
	           because the region is a result of XORing.
		   (ex, bank/rank bits <XOR> a portion of row bits)
	
4) echo 0xFFFF880000004000,0xFFFF880000000000 > /proc/memtest
-> run for each address bit in the region
ex) Acctest[1][4000]-[1000]. bit 12 297, 1400, 296   <- different bank
    Acctest[1][4000]-[2000]. bit 13 304, 1400, 301
    Acctest[1][4000]-[4000]. bit 14 96, 96, 96       <- same address
	Acctest[1][4000]-[8000]. bit 15 309, 1400, 305
	Acctest[1][4000]-[10000]. bit 16 308, 1400, 301
	Acctest[1][4000]-[20000]. bit 17 305, 1400, 301
	Acctest[1][4000]-[40000]. bit 18 352, 1400, 344  <-  access time jump: same bank, different rows (0x004000 and 0x040000)
	Acctest[1][4000]-[80000]. bit 19 304, 1400, 301
	Acctest[1][4000]-[100000]. bit 20 308, 1400, 301
	...

	--> bit 14 and 18 are XORed 
	
