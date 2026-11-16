#!/usr/bin/env python3
"""Exercise production page-table allocation above the nominal shared-memory limit."""
import argparse
from pathlib import Path
import subprocess
import tempfile

parser = argparse.ArgumentParser()
parser.add_argument('--source', type=Path)
args = parser.parse_args()
root = Path(__file__).resolve().parents[3]
source = args.source or root / 'drivers/gpu/mali400/r3p2/mali/common/mali_kernel_mem_os.c'
s = source.read_text()
a = s.index('static mali_physical_memory_allocation_result os_allocator_allocate_page_table_block(void * ctx, mali_page_table_block * block)\n{')
functions = s[a:]
prefix = r'''
#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
typedef uint32_t u32;
typedef int mali_physical_memory_allocation_result;
enum {MALI_MEM_ALLOC_NONE, MALI_MEM_ALLOC_FINISHED, MALI_MEM_ALLOC_INTERNAL_FAILURE};
#define CONFIG_FORCE_MAX_ZONEORDER 12
#define _MALI_OSK_CPU_PAGE_SIZE 4096U
#define _MALI_OSK_LOCKMODE_RW 0
#define _MALI_OSK_ERR_OK 0
#define MALI_DEBUG_ASSERT_POINTER(p) assert(p)
#define MALI_DEBUG_ASSERT(p) assert(p)
#define MALI_DEBUG_PRINT(level, message) ((void)0)
typedef struct { void *mutex; u32 num_pages_max, num_pages_allocated, cpu_usage_adjust; } os_allocator;
typedef struct mali_page_table_block {
 void (*release)(struct mali_page_table_block *);
 void *ctx, *handle; u32 size, phys_base; void *mapping;
} mali_page_table_block;
static int held, fail_lock, attempts;
static u32 max_dma_size=65536;
static int _mali_osk_lock_wait(void *m, int mode) {
 (void)m; (void)mode;
 if(fail_lock) return -1;
 assert(!held); held=1; return 0;
}
static void _mali_osk_lock_signal(void *m, int mode) {
 (void)m; (void)mode; assert(held); held=0;
}
static int _mali_osk_mem_check_allocated(u32 max) { (void)max; return 1; }
static void *_mali_osk_mem_allocioregion(u32 *phys, u32 size) {
 assert(held); attempts++;
 if(size>max_dma_size) return NULL;
 *phys=0x100000; return malloc(size);
}
static void _mali_osk_mem_freeioregion(u32 phys, u32 size, void *p) {
 assert(held && phys==0x100000 && size==65536); free(p);
}
static void os_allocator_page_table_block_release(mali_page_table_block *);
'''
tests = r'''
int main(void) {
 os_allocator info={NULL,65536,0,0};
 for(unsigned i=0;i<3;i++) {
  u32 before= i==0 ? 0 : i==1 ? info.num_pages_max : info.num_pages_max+1024;
  info.num_pages_allocated=before;
  mali_page_table_block b={0}; attempts=0;
  assert(os_allocator_allocate_page_table_block(&info,&b)==MALI_MEM_ALLOC_FINISHED);
  assert(!held && attempts>1 && b.size==65536);
  assert(info.num_pages_allocated==before+16);
  b.release(&b);
  assert(!held && info.num_pages_allocated==before);
 }
 max_dma_size=0; attempts=0;
 u32 before=info.num_pages_allocated;
 mali_page_table_block b={0};
 assert(os_allocator_allocate_page_table_block(&info,&b)==MALI_MEM_ALLOC_NONE);
 assert(!held && attempts==12 && info.num_pages_allocated==before && !b.mapping);
 fail_lock=1; attempts=0;
 assert(os_allocator_allocate_page_table_block(&info,&b)==MALI_MEM_ALLOC_INTERNAL_FAILURE);
 assert(!held && !attempts && info.num_pages_allocated==before);
}
'''
with tempfile.TemporaryDirectory(prefix='mali-page-table-') as d:
    cpp=Path(d)/'allocator.c'; binary=Path(d)/'allocator'
    cpp.write_text(prefix+functions+tests)
    subprocess.run(['cc','-std=gnu99','-fsanitize=address,undefined','-fno-omit-frame-pointer','-Wno-int-to-pointer-cast','-Wno-pointer-to-int-cast',str(cpp),'-o',str(binary)],check=True)
    subprocess.run([str(binary)],check=True)
print('PASS: below/at/above quota, DMA fallback, release accounting, OOM and lock failure')
