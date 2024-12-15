#include "list.h"
#include "interrupt.h"

// 初始化
void list_init(struct list *list) {

  list->head.next = &list->tail;
  list->head.prev = NULL;
  list->tail.prev = &list->head;
  list->tail.next = NULL;
}
/* 把链表元素 elem 插入在元素 before 之前 */
void list_insert_before(struct list_elem *before, struct list_elem *elem) {
  // 关掉中断状态
  enum intr_status old_status = intr_disable();
  /* 将before 前驱元素的后续元素更新为elem，暂时使before脱离链表*/
  before->prev->next = elem;
  /* 更新elem自己的前驱结点为before的前驱，*/
  /* 更新elem自己的后继结点为before，于是before又回到链表*/
  elem->prev = before->prev;
  elem->next = before;
  /* 更新before的前驱为elem，完成插入操作 */
  before->prev = elem;

  // 恢复中断状态
  intr_set_status(old_status);
}
/* 添加元素到列表队首，类似栈 push 操作 */
void list_push(struct list *plist, struct list_elem *elem) {
  list_insert_before(plist->head.next, elem); // 插入到head之后
}
//
void list_iterate(struct list *plist) {}
/* 追加元素到链表队尾，类似队列的先进先出操作 */
void list_append(struct list *plist, struct list_elem *elem) {
  list_insert_before(plist->tail.prev, elem); // 插入到tail之前
}
/* 使元素 pelem 脱离链表 */
void list_remove(struct list_elem *pelem) {
  enum intr_status old_status = intr_disable();
  // 前面连后面，后面连前面
  pelem->prev->next = pelem->next;
  pelem->next->prev = pelem->prev;
  // 恢复中断状态
  intr_set_status(old_status);
}
/* 将链表第一个元素弹出并返回，类似栈的 pop 操作 */
struct list_elem *list_pop(struct list *plist) {
  struct list_elem *elem = plist->head.next;
  list_remove(elem);
  return elem;
}
/* 判断链表是否为空，空时返回 true，否则返回 false */
bool list_empty(struct list *plist){
    return (plist->head.next == &plist->tail);
}
//返回链表长度
uint32_t list_len(struct list *plist){
    struct list_elem *elem = plist->head.next;
    uint32_t len = 0;
    while(elem!= &plist->tail){
        len++;
        elem = elem->next;
    }
    return len;
}
/* 把列表 plist 中的每个元素 elem 和 arg 传给回调函数 func，
 * arg 给 func 用来判断 elem 是否符合条件．
 * 本函数的功能是遍历列表内所有元素 ， 逐个判断是否有符合条件的元素。
 * 找到符合条件的元素返回元素指针，否则返回 NULL */
struct list_elem *list_traversal(struct list *plist, function func, int arg) {
  struct list_elem *elem = plist->head.next;
  if(list_empty(plist)){
    return NULL;
  }
  while (elem != &plist->tail) {
    if (func(elem, arg)) {
      // func 返回 ture，则认为该元素在回调函数中符合条件，命中，故停止继续遍历
      return elem;
    }
    // 若回调函数 func 返回 true，则继续遍历
    elem = elem->next;
  }
  return NULL;
}
/* 从链表中查找元素 obj_elem，成功时返回 true，失败时返回 false */
bool elem_find(struct list *plist, struct list_elem *obj_elem) {
  struct list_elem *elem = plist->head.next;
  while (elem != &plist->tail) {
    if (elem == obj_elem) {
      return true;
    }
    elem = elem->next;
  }
  return false;
}
