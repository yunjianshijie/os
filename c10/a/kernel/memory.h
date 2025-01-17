
#ifndef _KERNEL_MEMORY_H
#define _KERNEL_MEMORY_H
/*内存池标记用于判断用那个内存池*/
#include "bitmap.h"
#include "stdint.h"
/* 虚拟地址池用于管理虚拟地址管理*/

struct virtual_addr {
  struct bitmap vaddr_bitmap; // 虚拟地址使用位图
  uint32_t vaddr_start;       // 虚拟地址起始地址
};

enum pool_flags {
  PF_KERNEL = 1, // 内核内存池
  PF_USER = 2,   // 用户内存池
};

#define PG_P_1 1 // 页表项或页目录项存在属性位，代表此页存在
#define PG_P_0 0 // 页表项或页目录项存在属性位，代表此页不存在
#define PG_RW_R 0 // R/W属性位值，读  执行
#define PG_RW_W 2 // R/W属性位值，写  执行，书上写的2
#define PG_US_S 0 // U/S属性位值，系统级，只允许特权级0,1,2程序访问此页内存
#define PG_US_U 4//书上写4 // U/S属性位值，用户级

// 高10位目录项pde的索引，中间10位页表项pte的索引，低12位页内偏移

extern struct pool kernel_pool, user_pool; // 内核内存池，用户内存池
void mem_init(void);

void *get_kernel_pages(uint32_t pg_cnt);

// 在pf表示的虚拟内存池中申请pg_cnt个虚拟页
// static void *vaddr_get(enum pool_flags pf, uint32_t pg_cnt); 
// // 得到虚拟地址vaddr对应的pte指针
// uint32_t *pte_ptr(uint32_t vaddr);
// // 在m_pool指向的物理内存池中分配1个物理页，成功则返回页框的物理地址，失败则返回NULL
// static void *palloc(struct pool *m_pool);

// // 页表中添加虚拟地址_vaddr与物理地址_page_phyaddr的映射
// static void page_table_add(void *_vaddr, void *_page_phyaddr);


#endif
