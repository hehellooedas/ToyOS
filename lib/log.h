#ifndef __LIB_LOG_H
#define __LIB_LOG_H


#include <stdarg.h>
#include <printk.h>


/* 日志信息记录,默认占用一行
 * __DATE__ : 编译日期
 * __TIME__ : 编译时间
 * __FILE__ : 当前文件名
 * __LINE__ : 当前行号
 * __func__ : 当前正在执行的函数名
 */




enum log_level{
    INFO,       //打印和查看基本信息
    WARNING,    //警告
    ERROR,      //错误
    DEBUG       //调试使用
};



void _log_to_screen(enum log_level level,char* filename,char* func,int line,char* condition,...);


#define log_to_screen(level,...) _log_to_screen(level,__FILE__,__func__,__LINE__,__VA_ARGS__)


#endif