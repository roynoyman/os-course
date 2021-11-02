
#define _GNU_SOURCE

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <err.h>
#include <sys/mman.h>

#include "os.h"

/* 2^20 pages ought to be enough for anybody */
#define NPAGES	(1024*1024)

static char* pages[NPAGES];

uint64_t alloc_page_frame(void)
{
	static uint64_t nalloc;
	uint64_t ppn;
	void* va;

	if (nalloc == NPAGES)
		errx(1, "out of physical memory");

	/* OS memory management isn't really this simple */
	ppn = nalloc;
	nalloc++;

	va = mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
	if (va == MAP_FAILED)
		err(1, "mmap failed");

	pages[ppn] = va;	
	return ppn;
}

void* phys_to_virt(uint64_t phys_addr)
{
	uint64_t ppn = phys_addr >> 12;
	uint64_t off = phys_addr & 0xfff;
	char* va = NULL;

	if (ppn < NPAGES)
		va = pages[ppn] + off;

	return va;
}

int main(int argc, char **argv)
{
	uint64_t pt = alloc_page_frame();
    page_table_update(pt, 0xcafe, 0xf00d);
    printf("2 ok \n");
    page_table_update(pt, 0xbaff, 0xbadd);
    printf("3 ok \n");
    assert(page_table_query(pt, 0xcafe) == 0xf00d);
    printf("4 ok \n");
    assert(page_table_query(pt, 0xbaff) == 0xbadd);
    printf("5 ok \n");
    page_table_update(pt, 0xbaff, NO_MAPPING);
    printf("6 ok \n");
    assert(page_table_query(pt, 0xbaff) == NO_MAPPING);
    printf("7 ok \n");
    page_table_update(pt, 0xcafe, 0xbadd);
    printf("8 ok \n");
    assert(page_table_query(pt, 0xcafe) == 0xbadd);
    printf("9 ok \n");
    page_table_update(pt, 0xcafe, NO_MAPPING);
    printf("10 ok \n");
    assert(page_table_query(pt, 0xcafe) == NO_MAPPING);
    printf("11 ok \n");
    printf("1st Test: PASSED\n");
//	assert(page_table_query(pt, 0xcafe) == 0xf00d);
//	page_table_update(pt, 0xcafe, NO_MAPPING);
//	assert(page_table_query(pt, 0xcafe) == NO_MAPPING);

	return 0;
}

