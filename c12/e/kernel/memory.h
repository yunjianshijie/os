#ifndef __KERNEL_MEMORY_H
#define __KERNEL_MEMORY_H
#include "stdint.h"
#include "bitmap.h"
#include "list.h"
// 核心数据结构，虚拟内存池，有一个位图与其管理的起始虚拟地址
struct virtual_addr
{
   struct bitmap vaddr_bitmap; // 虚拟地址用到的位图结构
   uint32_t vaddr_start;       // 虚拟地址起始地址
};

extern struct pool kernel_pool, user_pool;
void mem_init(void);

#define PG_P_1 1  // 页表项或页目录项存在属性位
#define PG_P_0 0  // 页表项或页目录项存在属性位
#define PG_RW_R 0 // R/W 属性位值, 读/执行
#define PG_RW_W 2 // R/W 属性位值, 读/写/执行
#define PG_US_S 0 // U/S 属性位值, 系统级
#define PG_US_U 4 // U/S 属性位值, 用户级

/* 内存池标记,用于判断用哪个内存池 */
enum pool_flags
{
   PF_KERNEL = 1, // 内核内存池
   PF_USER = 2    // 用户内存池
};
/*n内存块*/
struct mem_block
{
   struct list_elem free_elem; // 空闲块链表
};

/* 内存块描述符*/
struct mem_block_desc{
   uint32_t block_size; // 内存块大小
   uint32_t blocks_per_arena; // 本 arena 中可容纳此 mem_block 的数量
   struct list free_list; // 空闲块链表
};

#define DESC_CNT 7 // 内存块描述符个数

void *get_kernel_pages(uint32_t pg_cnt);
void *malloc_page(enum pool_flags pf, uint32_t pg_cnt);
void malloc_init(void);
uint32_t *pte_ptr(uint32_t vaddr);
uint32_t *pde_ptr(uint32_t vaddr);
uint32_t addr_v2p(uint32_t vaddr);
void *get_user_pages(uint32_t pg_cnt);
void *get_a_page(enum pool_flags pf, uint32_t vaddr);

/* 为malloc做准备*/
void block_desc_init(struct mem_block_desc *desc_array);

/* 内存管理部分初始化入口*/
void mem_init(void);

/* 在堆中申请size个字节内存 */
void *sys_malloc(uint32_t size);
#endif