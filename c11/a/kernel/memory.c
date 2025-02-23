// #include "memory.h"
// #include "bitmap.h"
// #include "debug.h"
// #include "global.h"
// #include "print.h"
// #include "stdint.h"
// #include "string.h"

// #define PG_SIZE 4096

// /* ******************* 位图地址 *************************
//  *   因为0xc009f000是内核主线程栈顶，0xc009e000是内核主线程的pcb
//  *   一个页框大小的位图可表示128MB内存，位图位置安排在地址0xc009a000
//  *   这样本系统最大支持4个页框的位图m512MB*/
// #define MEM_BITMAP_BASE 0xc009a000
// /***********************************************************/

// #define PDE_IDX(addr) ((addr & 0xffc00000) >> 22) // 最高10位的掩码
// #define PTE_IDX(addr) ((addr & 0x003ff000) >> 12) // 中间10位的掩码

// /* 0xc0000000 是内核从虚拟地址3G起。
// 0x100000 意指跨过低端1MB 内存，使虚拟地址在逻辑上连续*/
// #define K_HEAP_START 0xc0100000

// /* 内存池结构，生成两个实例用于管理内核内存池和用户内存池*/
// struct pool {
//   struct bitmap pool_bitmap; // 本内存池用到的位图结构，用于管理物理内存
//   uint32_t phy_addr_start; // 内存池所管理物理内存的起始地址
//   uint32_t pool_size;      // 内存池大小
// };

// struct pool kernel_pool, user_pool; // 生成内核内存池和用户内存池
// struct virtual_addr kernel_vaddr;   // 此结构用来给内核虚拟地址池

// /* 在pf表示的虚拟内存池中申请pg_cnt个虚拟页
//  * 成功则返回虚拟页的起始地址，失败则返回NULL*/
// static void *vaddr_get(enum pool_flags pf, uint32_t pg_cnt) {
//   int vaddr_start = 0, bit_idx_start = -1;
//   uint32_t cnt = 0;
//   if (pf == PF_KERNEL) {
//     // 内核内存池
//     // 申请内核内存池的虚拟地址
//     bit_idx_start = bitmap_scan(&kernel_pool.pool_bitmap, pg_cnt);
//     // put_int(bit_idx_start);
//     // put_str("\n");
//     // while (1) // 悬停实施
//     //   ;
//     // int vaddr_start = bitmap_scan(&kernel_pool.pool_bitmap, pg_cnt);
//     if (bit_idx_start == -1) {
//       return NULL;
//     }
//     //
//     while (cnt < pg_cnt) {
//       bitmap_set(&kernel_pool.pool_bitmap, vaddr_start + cnt++, 1);
//       // put_int(cnt);
//     }
//     vaddr_start = kernel_vaddr.vaddr_start + vaddr_start * PG_SIZE;
//   } else {
//     // 用户内存池
//   }
//   // put_str("vaddr_start:");
//   // put_int(vaddr_start);
//   // put_str("\n");
//   // while (1)
//   //   ;
//   return (void *)vaddr_start;
// }

// /* 得到虚拟地址vaddr对应的pte指针*/
// uint32_t *pte_ptr(uint32_t vaddr) {
//   /*先访问到页表自己 +
//    * 再用页目录pde（页目录内页表的索引）作为pte的索引访问到页表
//    * 再用pte的索引作为页内偏移
//    */
//   // uint32_t *pte =(uint32_t *)(0xfffff000 + PTE_IDX(vaddr) * 4 + (vaddr &
//   // 0xffc00000) >>10);
//   uint32_t *pte = (uint32_t *)(0xffc00000 + ((vaddr & 0xffc00000) >> 10)
//   +PTE_IDX(vaddr) * 4); return pte;
// }

// /* 得到虚拟地址vaddr对应的pde指针*/
// uint32_t *pde_ptr(uint32_t vaddr) {
//   /* 0xfffff 用来访问到页表本身所在的地址 */
//   uint32_t *pde = (uint32_t *)((0xfffff000) + PDE_IDX(vaddr) * 4);
//   return pde;
// }

// /* 在m_pool指向的物理内存池中分配1个物理页，
//  * 成功则返回页框的物理地址，失败则返回NULL*/
// static void *palloc(struct pool *m_pool) {
//   /* 扫描或设置位图要保证原子操作*/
//   int bit_idx = bitmap_scan(&m_pool->pool_bitmap, 1); // 找一个物理页面
//   if (bit_idx == -1) {
//     return NULL;
//   }
//   bitmap_set(&m_pool->pool_bitmap, bit_idx, 1); // 将此位bit_idx置为1
//   uint32_t page_phyaddr = m_pool->phy_addr_start + bit_idx * PG_SIZE;
//   return (void *)page_phyaddr;
// }

// // vaddr-> 虚拟地址， page_phyaddr-> 页框物理地址
// static void page_table_add(void *_vaddr, void *_page_phyaddr) {

//   uint32_t vaddr = (uint32_t)_vaddr, page_phyaddr = (uint32_t)_page_phyaddr;
//   uint32_t *pde = pde_ptr(vaddr); // 得到页目录项的虚拟地址
//   uint32_t *pte = pte_ptr(vaddr); // 得到页表项的虚拟地址
//   // put_str("pde:");
//   // put_int((int)*pde);
//   // put_str("\n");
//   // put_str("pte:");
//   // put_int((int)*pte);
//   // put_str("\n");
//   /* *********************** 注意 *************************
//    * 执行*pte，会访问到空的 pde。所以确保 pde 创建完成后才能执行*pte，
//    * 否则会引发 page_fault。因此在*pde 为 0 时，
//    *pte 只能出现在下面 else 语句块中的*pde 后面。
//    ************************************************************/
//   if (*pde & 0x00000001) {
//     // 页目录项和页表项的第0位为P,此处判断目录项是否存在
//     ASSERT(!(*pte & 0x00000001));
//     put_int(*pte);
//     put_str("    1\n");
//     if (!(*pte & 0x00000001))
//     {
//       // 只要是创建页表，pte就应该不存在，所以多判断一下
//       *pte = (page_phyaddr | PG_US_U | PG_RW_W | PG_P_1); // 100|010 |001
//       // US=1,RW=1,P=1
//     }
//     else
//     {
//       // 目前应该不会执行到这里，因为上面的ASSERT会先执行
//       PANIC("pte repeat"); // 当前页表项存在
//       *pte = (page_phyaddr | PG_US_U | PG_RW_W | PG_P_1);
//       // US=1,RW=1,P=1
//     }
//   } else {
//     // 页目录项不存在,所以说要先创建页目录再创建页表项
//     /* 页表中用到的页框一律从内核空间分配*/
//     uint32_t pde_phyaddr = (uint32_t)palloc(&kernel_pool);
//     *pde = (pde_phyaddr | PG_US_U | PG_RW_W | PG_P_1);
//     /* 分配到的物理页地址pde_phyaddr对应的物理内存清0
//      * 避免里面的陈旧数据变成了页表项，从此让页表混乱
//      * 访问到pde对应的物理地址，用pte取高20位便可
//      * 因为pte基于该pde对应的物理地址内再寻址
//      * 把低12位 置0，便是该pde对应的物理页的起始 */
//     memset((void *)((int)pte & 0xfffff000), 0, PG_SIZE);
//     put_int(*pte);
//     put_str("2\n");
//     ASSERT(!(*pte & 0x00000001));
//     *pte = (page_phyaddr | PG_US_U | PG_RW_W | PG_P_1);
//     // US=100,RW=010,P=001
//   }
// }

// /* 分配pg_cnt 个页空间，，成功返回起始虚拟地址，失败时返回NULL*/
// void *malloc_page(enum pool_flags pf, uint32_t pg_cnt) {
//   ASSERT(pg_cnt > 0 && pg_cnt <= 1024);
//   /*********** malloc_page 的原理是三个动作的合成: ***********
//    1 通过 vaddr_get 在虚拟内存池中申请虚拟地址
//    2 通过 palloc 在物理内存池中申请物理页
//    3 通过 page_table_add 将以上得到的虚拟地址和物理地址在页表中完成映射
//   ********************************************************************/
//   // 1.申请虚拟地址
//   void *vaddr_start = vaddr_get(pf, pg_cnt);
//   if (vaddr_start == NULL) { // 判断错误
//     return NULL;
//   }

//   // 2 palloc 在物理内存池中申请物理页
//   uint32_t vaddr = (uint32_t)vaddr_start;
//   uint32_t cnt = pg_cnt; // 有多少个页框需要映射
//   // 内核和用户内存池的物理地址不同，所以需要判断
//   struct pool *mem_pool = (pf & PF_KERNEL) ? &kernel_pool : &user_pool;
//   /* 因为虚拟地址是连续的，但是物理地址可以不连续，所以要一个一个映射*/

//   while (cnt-- > 0) {
//     void *page_phyaddr = palloc(mem_pool); // 申请物理地址
//     // put_str("malloc_page: page_phyaddr = 0x");
//     // put_int((uint32_t)page_phyaddr);
//     if (page_phyaddr == NULL) // 判断是否申请成功
//     {
//       //
//       失败时要将曾经已申请的虚拟地址和物理地址的映射关系全部撤销，在将来完成内存回收时再补充
//       return NULL;
//     }
//     page_table_add((void *)vaddr, page_phyaddr); // 建立映射关系

//     vaddr += PG_SIZE; // 虚拟地址加一页
//   }

//   return vaddr_start; // 返回虚拟地址
// }

// /* 3从内核物理内存池中申请 pg_cnt 页内存，
//  成功则返回其虚拟地址，失败则返回 NULL */
// void *get_kernel_pages(uint32_t pg_cnt) {
//   void *vaddr = malloc_page(PF_KERNEL, pg_cnt);
//   if (vaddr != NULL) { // 如果分配地址不为空，将页框清0后返回
//     memset(vaddr, 0, pg_cnt * PG_SIZE); // 4KB * pg_cnt
//   }
//   return vaddr;
// }

// /* 初始化内存池*/
// static void mem_pool_init(uint32_t all_mem) {
//   put_str(" mem_pool_init start\n");
//   uint32_t page_table_size = PG_SIZE * 256; // 页目录表大小
//   一个页目录0x100000

//   // 页表示大小=1页的页目录表 + 第0 和 第768个页目录指向同一个页表+
//   // 第 789~1022 个页目录项共指向254个页表，共256个页框

//   uint32_t used_mem = page_table_size + 0x100000;
//   // 0x100000为低端1mb内存
//   uint32_t free_mem = all_mem - used_mem;
//   uint16_t all_free_pages = free_mem / PG_SIZE;
//   // 1页为4kb,不管总内存是不是4kb的倍数，都向上取整
//   //  对于以页为单位分配策略，不足1页的内存不用考虑来

//   uint16_t kernel_free_page = all_free_pages / 2;
//   uint16_t user_free_page = all_free_pages - kernel_free_page;
//   /* 为简单位图操作，余数不处理，坏处这样做会丢内存。
//   好处是不用做内存的越界检查，因为位图表示的内存少于于实际物理内存*/
//   uint32_t kbm_length = kernel_free_page / 8;
//   // kernel BitMap 的长度，位图中的一位表示一页，以字节为单位
//   uint32_t ubm_length = user_free_page / 8;
//   // User BitMap 的长度
//   uint32_t kp_start = used_mem;
//   // Kernel Pool start 内核内存池的起始地址
//   uint32_t up_start = kp_start + kernel_free_page * PG_SIZE;
//   // User Pool start 用户内存池的起始地址

//   kernel_pool.phy_addr_start = kp_start;
//   user_pool.phy_addr_start = up_start;

//   kernel_pool.pool_size = kernel_free_page * PG_SIZE;
//   user_pool.pool_size = user_free_page * PG_SIZE;

//   kernel_pool.pool_bitmap.btmp_bytes_len = kbm_length;
//   user_pool.pool_bitmap.btmp_bytes_len = ubm_length;

//   /********* 内核内存池和用户内存池位图 ***********
//    * 位图是全局的数据，长度不固定。
//    * 全局或静态的数组需要在编译时知道其长度，
//    * 而我们需要根据总内存大小算出需要多少字节，
//    * 所以改为指定一块内存来生成位图。
//    * ************************************************/
//   // 内核使用的最高地址是 0xc009f000，这是主线程的栈地址
//   // （内核的大小预计为 70KB 左右）
//   // 32MB 内存占用的位图是 2KB
//   // 内核内存池的位图先定在 MEM_BITMAP_BASE(0xc009a000)处
//   kernel_pool.pool_bitmap.bits = (void *)MEM_BITMAP_BASE;
//   // 用户内存池的位图紧跟在内核内存池位图之后
//   user_pool.pool_bitmap.bits = (void *)(MEM_BITMAP_BASE + kbm_length);

//   /********************输出内存池信息**********************/
//   put_str("   kernel_pool_bitmap_start:");
//   put_int((int)kernel_pool.pool_bitmap.bits);
//   put_str("   kernel_pool_phy_addr_start:");
//   put_int((int)kernel_pool.phy_addr_start);
//   put_str("\n");
//   put_str("   user_pool_bitmap_start:");
//   put_int((int)user_pool.pool_bitmap.bits);
//   put_str("   user_pool_phy_addr_start:");
//   put_int((int)user_pool.phy_addr_start);
//   put_str("\n");
//   /* 将位图置 0*/
//   bitmap_init(&kernel_pool.pool_bitmap);
//   bitmap_init(&user_pool.pool_bitmap);
//   /* 下面初始化内核虚拟地址的位图，按实际物理内存大小生成数组。*/
//   kernel_vaddr.vaddr_bitmap.btmp_bytes_len = kbm_length;
//   // 用于维护内核堆的虚拟地址，所以要和内核内存池大小一致
//   /* 位图的数组指向一块未使用的内存，
//       目前定位在内核内存池和用户内存池之外*/
//   kernel_vaddr.vaddr_bitmap.bits =
//       (void *)(MEM_BITMAP_BASE + kbm_length + ubm_length);
//   kernel_vaddr.vaddr_start = K_HEAP_START;
//   bitmap_init(&kernel_vaddr.vaddr_bitmap);
//   put_str(" mem_pool_init done\n");
// }

// void mem_init() {
//   put_str("mem_init start\n");
//   uint32_t mem_bytes_total = (*(uint32_t *)(0xb00));
//   mem_pool_init(mem_bytes_total); // 初始化内存池
//   put_str("mem_init end\n");
// }

#include "memory.h"
#include "debug.h"
#include "print.h"
#include "stdint.h"
#include "string.h"

#define PG_SIZE 4096 // 一页的大小
#define MEM_BITMAP_BASE                                                        \
  0xc009a000 // 这个地址是位图的起始地址，1MB内存布局中，9FBFF是最大一段可用区域的边界，而我们计划这个可用空间最后的位置将来用来
// 放PCB，而PCB占用内存是一个自然页，所以起始地址必须是0xxxx000这种形式，离0x9fbff最近的符合这个形式的地址是0x9f000。我们又为了将来可能的拓展，
//  所以让位图可以支持管理512MB的内存空间，所以预留位图大小为16KB，也就是4页，所以选择0x9a000作为位图的起始地址

// 定义内核堆区起始地址，堆区就是用来进行动态内存分配的地方，咱们的系统内核运行在c00000000开始的1MB虚拟地址空间，所以自然要跨过这个空间，
// 堆区的起始地址并没有跨过256个页表，没关系，反正使用虚拟地址最终都会被我们的页表转换为物理地址，我们建立物理映射的时候，跳过256个页表就行了
#define K_HEAP_START 0xc0100000

/* 核心数据结构，物理内存池， 生成两个实例用于管理内核物理内存池和用户物理内存池
 */
struct pool {
  struct bitmap pool_bitmap; // 本内存池用到的位图结构,用于管理物理内存
  uint32_t phy_addr_start;   // 本内存池所管理物理内存的起始地址
  uint32_t pool_size;        // 本内存池字节容量
  struct lock lock;
};

struct pool kernel_pool, user_pool;
// 为kernel与user分别建立物理内存池，让用户进程只能从user内存池获得新的内存空间，
// 以免申请完所有可用空间,内核就不能申请空间了
struct virtual_addr kernel_vaddr; // 用于管理内核虚拟地址空间

// 初始化内核物理内存池与用户物理内存池
static void mem_pool_init(uint32_t all_mem) {
  put_str("   mem_pool_init start\n");
  uint32_t page_table_size = PG_SIZE * 256;
  // 页表大小= 1页的页目录表+第0和第768个页目录项指向同一个页表+
  // 第769~1022个页目录项共指向254个页表,共256个页表
  uint32_t used_mem =
      page_table_size + 0x100000; // 已使用内存 = 1MB + 256个页表
  uint32_t free_mem = all_mem - used_mem;
  uint16_t all_free_pages =
      free_mem /
      PG_SIZE; // 将所有可用内存转换为页的数量，内存分配以页为单位，丢掉的内存不考虑
  uint16_t kernel_free_pages =
      all_free_pages /
      2; // 可用内存是用户与内核各一半，所以分到的页自然也是一半
  uint16_t user_free_pages =
      all_free_pages - kernel_free_pages; // 用于存储用户空间分到的页

  /* 为简化位图操作，余数不处理，坏处是这样做会丢内存。
  好处是不用做内存的越界检查,因为位图表示的内存少于实际物理内存*/
  uint32_t kbm_length =
      kernel_free_pages /
      8; // 内核物理内存池的位图长度,位图中的一位表示一页,以字节为单位
  uint32_t ubm_length = user_free_pages / 8; // 用户物理内存池的位图长度.

  uint32_t kp_start =
      used_mem; // Kernel Pool start,内核使用的物理内存池的起始地址
  uint32_t up_start =
      kp_start + kernel_free_pages *
                     PG_SIZE; // User Pool start,用户使用的物理内存池的起始地址

  kernel_pool.phy_addr_start = kp_start; // 赋值给内核使用的物理内存池的起始地址
  user_pool.phy_addr_start = up_start;   // 赋值给用户使用的物理内存池的起始地址

  kernel_pool.pool_size =
      kernel_free_pages * PG_SIZE; // 赋值给内核使用的物理内存池的总大小
  user_pool.pool_size =
      user_free_pages * PG_SIZE; // 赋值给用户使用的物理内存池的总大小

  kernel_pool.pool_bitmap.btmp_bytes_len =
      kbm_length; // 赋值给管理内核使用的物理内存池的位图长度
  user_pool.pool_bitmap.btmp_bytes_len =
      ubm_length; // 赋值给管理用户使用的物理内存池的位图长度

  /*********    内核内存池和用户内存池位图   ***********
   *   位图是全局的数据，长度不固定。
   *   全局或静态的数组需要在编译时知道其长度，
   *   而我们需要根据总内存大小算出需要多少字节。
   *   所以改为指定一块内存来生成位图.
   *   ************************************************/
  // 内核使用的最高地址是0xc009f000,这是主线程的栈地址.(内核的大小预计为70K左右)
  // 32M内存占用的位图是2k.内核内存池的位图先定在MEM_BITMAP_BASE(0xc009a000)处.
  kernel_pool.pool_bitmap.bits =
      (void *)MEM_BITMAP_BASE; // 管理内核使用的物理内存池的位图起始地址

  /* 用户内存池的位图紧跟在内核内存池位图之后 */
  user_pool.pool_bitmap.bits =
      (void *)(MEM_BITMAP_BASE +
               kbm_length); // 管理用户使用的物理内存池的位图起始地址
  /******************** 输出内存池信息 **********************/
  put_str("      kernel_pool_bitmap_start:");
  put_int((int)kernel_pool.pool_bitmap.bits);
  put_str(" kernel_pool_phy_addr_start:");
  put_int(kernel_pool.phy_addr_start);
  put_str("\n");
  put_str("      user_pool_bitmap_start:");
  put_int((int)user_pool.pool_bitmap.bits);
  put_str(" user_pool_phy_addr_start:");
  put_int(user_pool.phy_addr_start);
  put_str("\n");

  /* 将位图置0*/
  bitmap_init(&kernel_pool.pool_bitmap);
  bitmap_init(&user_pool.pool_bitmap);

  /* 下面初始化内核虚拟地址的位图,按实际物理内存大小生成数组。*/
  kernel_vaddr.vaddr_bitmap.btmp_bytes_len =
      kbm_length; // 赋值给管理内核可以动态使用的虚拟地址池（堆区）的位图长度，
                  // 其大小与管理内核可使用的物理内存池位图长度相同，因为虚拟内存最终都要转换为真实的物理内存，可用虚拟内存大小超过可用物理内存大小在
                  // 我们这个简单操作系统无意义（现代操作系统中有意义，因为我们可以把真实物理内存不断换出，回收，来让可用物理内存变相变大)

  /* 位图的数组指向一块未使用的内存,目前定位在内核内存池和用户内存池之外*/
  kernel_vaddr.vaddr_bitmap.bits =
      (void
           *)(MEM_BITMAP_BASE + kbm_length +
              ubm_length); // 赋值给管理内核可以动态使用的虚拟内存池（堆区）的位图起始地址

  kernel_vaddr.vaddr_start =
      K_HEAP_START; // 赋值给内核可以动态使用的虚拟地址空间的起始地址
  bitmap_init(
      &kernel_vaddr
           .vaddr_bitmap); // 初始化管理内核可以动态使用的虚拟地址池的位图
  put_str("   mem_pool_init done\n");
}

/* 在pf表示的虚拟内存池中申请pg_cnt个虚拟页,
 * 成功则返回虚拟页的起始地址, 失败则返回NULL */
static void *vaddr_get(enum pool_flags pf, uint32_t pg_cnt) {
  int vaddr_start = 0, bit_idx_start = -1;
  uint32_t cnt = 0;
  if (pf == PF_KERNEL) { // 内核内存池
    bit_idx_start = bitmap_scan(&kernel_vaddr.vaddr_bitmap, pg_cnt);
    if (bit_idx_start == -1) {
      return NULL;
    }
    while (cnt < pg_cnt) {
      bitmap_set(&kernel_vaddr.vaddr_bitmap, bit_idx_start + cnt++, 1);
    }
    vaddr_start = kernel_vaddr.vaddr_start + bit_idx_start * PG_SIZE;
  } else {
    // 用户内存池,将来实现用户进程再补充
    struct task_struct *cur = running_thread();
    bit_idx_start = bitmap_scan(&cur->userprog_vaddr.vaddr_bitmap, pg_cnt);
    if (bit_idx_start == -1) {
      return NULL;
    }
    while (cnt < pg_cnt) {
      bitmap_set(&cur->userprog_vaddr.vaddr_bitmap, bit_idx_start + cnt++, 1);
    }
    vaddr_start = cur->userprog_vaddr.vaddr_start + bit_idx_start * PG_SIZE;
    /*  (0xc0000000 - PG_SIZE)作为用户 3 级栈已经在 start_process 被分配 
    ASSERT((uint32_t)vaddr_start < (0xc0000000 - PG_SIZE));
    */
   return (void *)(vaddr_start);
  }
  return (void *)vaddr_start;
}

/* 在m_pool指向的物理内存池中分配1个物理页,
 * 成功则返回页框的物理地址,失败则返回NULL */
static void *palloc(struct pool *m_pool) {
  /* 扫描或设置位图要保证原子操作 */
  int bit_idx = bitmap_scan(&m_pool->pool_bitmap, 1); // 找一个物理页面
  if (bit_idx == -1) {
    return NULL;
  }
  bitmap_set(&m_pool->pool_bitmap, bit_idx, 1); // 将此位bit_idx置1
  uint32_t page_phyaddr = ((bit_idx * PG_SIZE) + m_pool->phy_addr_start);
  return (void *)page_phyaddr;
}

#define PDE_IDX(addr) ((addr & 0xffc00000) >> 22)
#define PTE_IDX(addr) ((addr & 0x003ff000) >> 12)

/* 得到虚拟地址vaddr对应的pde的指针 */
uint32_t *pde_ptr(uint32_t vaddr) {
  /* 0xfffff是用来访问到页表本身所在的地址 */
  uint32_t *pde = (uint32_t *)((0xfffff000) + PDE_IDX(vaddr) * 4);
  return pde;
}

/* 得到虚拟地址vaddr对应的pte指针*/
uint32_t *pte_ptr(uint32_t vaddr) {
  /* 先访问到页表自己 +
   * 再用页目录项pde(页目录内页表的索引)做为pte的索引访问到页表 +
   * 再用pte的索引做为页内偏移*/
  uint32_t *pte = (uint32_t *)(0xffc00000 + ((vaddr & 0xffc00000) >> 10) +
                               PTE_IDX(vaddr) * 4);
  return pte;
}

/* 页表中添加虚拟地址_vaddr与物理地址_page_phyaddr的映射 */
static void page_table_add(void *_vaddr, void *_page_phyaddr) {
  uint32_t vaddr = (uint32_t)_vaddr, page_phyaddr = (uint32_t)_page_phyaddr;
  uint32_t *pde = pde_ptr(vaddr);
  uint32_t *pte = pte_ptr(vaddr);

  /************************   注意   *************************
   * 执行*pte,会访问到空的pde。所以确保pde创建完成后才能执行*pte,
   * 否则会引发page_fault。因此在*pde为0时,*pte只能出现在下面else语句块中的*pde后面。
   * *********************************************************/
  /* 先在页目录内判断目录项的P位，若为1,则表示该表已存在 */
  if (*pde & 0x00000001) { // 页目录项和页表项的第0位为P,此处判断目录项是否存在
    ASSERT(!(*pte & 0x00000001));

    if (!(*pte & 0x00000001)) { // 只要是创建页表,pte就应该不存在,多判断一下放心
      *pte = (page_phyaddr | PG_US_U | PG_RW_W | PG_P_1); // US=1,RW=1,P=1
    } else { // 应该不会执行到这，因为上面的ASSERT会先执行。
      PANIC("pte repeat");
      *pte = (page_phyaddr | PG_US_U | PG_RW_W | PG_P_1); // US=1,RW=1,P=1
    }
  } else { // 页目录项不存在,所以要先创建页目录再创建页表项.
           /* 页表中用到的页框一律从内核空间分配 */
    uint32_t pde_phyaddr = (uint32_t)palloc(&kernel_pool);

    *pde = (pde_phyaddr | PG_US_U | PG_RW_W | PG_P_1);

    /* 分配到的物理页地址pde_phyaddr对应的物理内存清0,
     * 避免里面的陈旧数据变成了页表项,从而让页表混乱.
     * 访问到pde对应的物理地址,用pte取高20位便可.
     * 因为pte是基于该pde对应的物理地址内再寻址,
     * 把低12位置0便是该pde对应的物理页的起始*/
    memset((void *)((int)pte & 0xfffff000), 0, PG_SIZE);

    ASSERT(!(*pte & 0x00000001));
    *pte = (page_phyaddr | PG_US_U | PG_RW_W | PG_P_1); // US=1,RW=1,P=1
  }
}

/* 分配pg_cnt个页空间,成功则返回起始虚拟地址,失败时返回NULL */
void *malloc_page(enum pool_flags pf, uint32_t pg_cnt) {
  ASSERT(pg_cnt > 0 && pg_cnt < 3840);
  /***********   malloc_page的原理是三个动作的合成:   ***********
        1通过vaddr_get在虚拟内存池中申请虚拟地址
        2通过palloc在物理内存池中申请物理页
        3通过page_table_add将以上得到的虚拟地址和物理地址在页表中完成映射
  ***************************************************************/
  void *vaddr_start = vaddr_get(pf, pg_cnt);
  if (vaddr_start == NULL) {
    return NULL;
  }

  uint32_t vaddr = (uint32_t)vaddr_start, cnt = pg_cnt;
  struct pool *mem_pool = pf & PF_KERNEL ? &kernel_pool : &user_pool;

  /* 因为虚拟地址是连续的,但物理地址可以是不连续的,所以逐个做映射*/
  while (cnt-- > 0) {
    void *page_phyaddr = palloc(mem_pool);
    if (page_phyaddr ==
        NULL) { // 失败时要将曾经已申请的虚拟地址和物理页全部回滚，在将来完成内存回收时再补充
      return NULL;
    }
    page_table_add((void *)vaddr, page_phyaddr); // 在页表中做映射
    vaddr += PG_SIZE;                            // 下一个虚拟页
  }
  return vaddr_start;
}

/* 从内核物理内存池中申请pg_cnt页内存,成功则返回其虚拟地址,失败则返回NULL */
void *get_kernel_pages(uint32_t pg_cnt) {
  void *vaddr = malloc_page(PF_KERNEL, pg_cnt);
  if (vaddr != NULL) { // 若分配的地址不为空,将页框清0后返回
    memset(vaddr, 0, pg_cnt * PG_SIZE);
  }
  return vaddr;
}

/* 内存管理部分初始化入口 */
void mem_init() {
  put_str("mem_init start\n");
  uint32_t mem_bytes_total = (*(uint32_t *)(0xb00));
  mem_pool_init(mem_bytes_total); // 初始化内存池
  put_str("mem_init done\n");
}
