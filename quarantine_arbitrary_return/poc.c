#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char target[256];

int main() {

    setbuf(stdin, NULL);
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    printf("libc @ %p\n", puts);
    strcpy(target, "CAN YOU OVERWRITE ME PLZ?");

    char* dumbAllocation = malloc(24);
    char* fakeAllocator = malloc(0x800 - 0x10);

    free(dumbAllocation); // dumbAllocation is now quarantined.

    char* overflow_chunk = malloc(0x1000 - 0x40); // this will allocate a 0x1000 chunk within the QuarantineBatch region.

    printf("Overflow chunk: %p\n", overflow_chunk);

    printf("Target @ %p\n", &target);

    puts("Fake allocator: ");
    read(0, fakeAllocator, 0x800 - 0x10);

    gets(overflow_chunk); // Now we have to spray a little bit & be lucky inorder to hit the Last QuarantineBatch.

    // If those conditions are met, by forcing a chunk to be quarantined we gain a Write-Where-Ptr primitive.
    // With this primitive we will try to corrupt the Allocator reference from the TSD. By this way we will be able to hijack almost everything.

    // Trigger our Write-Where-Ptr
    free(fakeAllocator);

    // Here, if our attack was successful, scudo must return a pointer to a writable
    // memory location of our choice.
    void *small_chunk = malloc(64);
    strcpy(small_chunk, "YES I CAN!");

    puts(target);

    _Exit(0);
    return 0;
}