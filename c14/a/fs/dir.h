#ifndef __FS_DIR_H
#define __FS_DIR_H

#include "inode.h"
#include "fs.h"
#define MAX_DIR_NAME_LEN 16  //最大文件名长度

/* 目录结构*/
struct dir
{
    struct inode* inode; //指向目录所在的inode
    uint32_t dir_pos;    //当前目录读取的位置
    uint8_t dir_buf[512]; //目录数据缓存区
};

/* 目录项结构 */
struct dir_entry
{
    char file_name[MAX_DIR_NAME_LEN]; //文件名
    uint32_t i_no; // 普通文件或目录对应的inode编号
    enum file_type f_type; // 文件类型
};

#endif