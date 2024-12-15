#ifndef _LIB_KERNEL_LIST_H
#define _LIB_KERNEL_LIST_H

#include "global.h"
#include <stdbool.h>
#define offser(struct_type, member) (int)(&((struct_type *)0)->member)
#define element_type(struct_type, struct_member_name, elem_ptr)                \
  (struct_type *)((int)elem_ptr - offser(struct_type, struct_member_name))
  
#define NULL ((void *)0)
/* * 定义链表结点成员结构*
 * 结点中不需要数据成元素，只要求前驱和后继结点指针*/
struct list_elem {
  struct list_elem *prev; // 前驱结点
  struct list_elem *next; // 后继结点
};

/* 链表结构，用来实现队列*/
struct list {
  /* head 是队首，是固定不变的，不是第一个元素，第一个元素为head.next*/
  struct list_elem head; // 队首
  struct list_elem tail; // 队尾
};
/* 自定义函数类型function，用于在list_traversal中回调函数*/
typedef bool(function)(struct list_elem *, int arg);

// 一些函数
// 初始化
void list_init(struct list *list);
//  插入
void list_insert_before(struct list_elem *before, struct list_elem *elem);
//
void list_push(struct list *plist, struct list_elem *elem);
//
void list_iterate(struct list *plist);
//
void list_append(struct list *plist, struct list_elem *elem);
//
void list_remove(struct list_elem *pelem);
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
#endif