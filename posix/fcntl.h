#ifndef __POSIX_FCNTL_H
#define __POSIX_FCNTL_H




#define O_RDONLY        00000000    //只读
#define O_WRONLY        00000001    //只写
#define O_RDWR          00000002    //可读写
#define O_ACCMODE       00000003    //访问模式掩码

#define O_CREAT         00000100    //创建文件,如果文件不存在
#define O_EXCL          00000200    //确保所请求的文件不存在
#define O_NOCTTY        00000400    //不会被设置为调用进程的控制终端

#define O_TRUNC         00001000    //在文件存在且可写的情况下会截断文件长度为0

#define O_APPEND        00002000    //文件指针指向文件末尾
#define O_NONBLOCK      00004000    //对应的文件操作不会使调用进程阻塞

#define O_EXEC          00010000    //仅用于可执行
#define O_SEARCH        00020000    //只允许打开目录用于搜索操作
#define O_DIRECTORY     00040000    //确保打开的是一个目录
#define O_NOFOLLOW      00100000    //确保指定的路径不是一个符号链接




#endif