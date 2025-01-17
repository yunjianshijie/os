#ifndef __KERNEL_INTERRUPT_H
#define __KERNEL_INTERRUPT_H
#include "stdint.h"
typedef void *intr_handler; // 将intr_handler定义为void*同类型
void idt_init(void);
/* 定义中断的两种状态 枚举类型用来管理中断，值为0
* INTR_OFF值为0 表示关中断
* INTR_ON值为1  表示开中断
*/
enum intr_status
{
    INTR_OFF,
    INTR_ON
};
/* 开中断并返回开中断前的状态*/
enum intr_status intr_enable();
/* 关中断，并且返回关中断前的状态 */
enum intr_status intr_disable();
/* 将中断状态设置为 status */
enum intr_status intr_set_status(enum intr_status status);
/* 获取当前中断状态 */
enum intr_status intr_get_status();

void register_handler(uint8_t vector_no, intr_handler function);
#endif