#ifndef __FS_FS_H
#define __FS_FS_H

#define MAX_FILES_PER_RART 4096
// 每个分区所支持最大创建的文件数

#define BITS_PER_SECTOR 4096 // 每扇区的位数
#define SECTOR_SIZE 512 // 每扇区大小
#define BLOCK_SIZE SECTOR_SIZE  // 每个块的大小

/* 文件类型*/
enum file_types
{
    FT_UNKNOWN, //  不支持的文件类型
    FT_REGULAR, // 普通文件
    FT_DIRECTORY, // 目录
};


#endif