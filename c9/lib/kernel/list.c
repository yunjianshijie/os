#include "list.h"
#include "interrupt.h"

// 初始化
void list_init(struct list *list){
    
    list->head.next = &list->tail;
    list->head.prev = NULL;
    list->tail.prev = &list->head;
    list->tail.next = NULL;
}
//  插入
void list_insert_before(struct list_elem *brfore, struct list_elem *elem){
    

}
//
void list_push(struct list *plist, struct list_elem *elem);
//
void list_iterate(struct list *plist);
//
void list_append(struct list *plist, struct list_elem *elem);
//
void list_remove(struct list_elem *elem);
// 弹出
struct list_elem *list_pop(struct list *plist);
// 是否为空
bool list_empty(struct list *plist);
//
uint32_t list_len(struct list *plist);
//
struct list_elem *list_traversal(struct list *plist, function func, int arg);
//
bool elem_find(struct list *plist, struct list_elem *obj_elem);