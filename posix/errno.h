#ifndef __POSIX_ERRNO_H
#define __POSIX_ERRNO_H



/*
 * POSIX标准制定的错误码
 * https://www.man7.org/linux/man-pages/man3/errno.3.html
 */




/*  错误类型  */
#define E2BIG           1       //参数列表太长/输出缓冲区空间不足/参数大于系统的设定值
#define EACCES          2       //权限被拒绝
#define EADDRINUSE      3       //地址已在使用中
#define EADDRNOTAVAIL   4       //地址不可用
#define EAFNOSUPPORT    5       //地址簇不支持
#define EAGAIN          6       //资源暂时不可用
#define EALREADY        7       //连接已在进行中
#define EBADF           8       //文件描述符错误
#define EBADMSG         9       //错误的消息
#define EBUSY           10      //资源繁忙
#define ECANCELED       11      //操作被取消
#define ECHILD          12      //没有子进程
#define ECONNABORTED    13      //连接被中止
#define ECONNREFUSED    14      //连接被拒绝
#define ECONNRESET      15      //连接被重置
#define EDEADLK         16      //资源死锁会发生
#define EDESTADDRREQ    17      //需要目的地址
#define EDOM            18      //域错误
#define EDQUOT          19      //超过配额限制
#define EEXIST          20      //文件已存在
#define EFAULT          21      //错误的地址
#define EFBIG           22      //文件过大
#define EHOSTUNREACH    23      //主机不可达
#define EIDRM           24      //标识符被移除
#define EILSEQ          25      //非法字符被移除
#define EINPROGRESS     26      //操作正在进行中,无法立即建立连接
#define EINTR           27      //函数调用被中断
#define EINVAL          28      //无效的参数
#define EIO             29      //输入输出错误
#define EISCONN         30      //套接字已连接
#define EISDIR          31      //是一个目录
#define ELOOP           32      //符号链接循环
#define EMFILE          33      //文件描述符值过大
#define EMLINK          34      //链接数量过大
#define EMSGSIZE        35      //消息过大
#define EMULTIHOP       36      //多跳(保留)
#define ENAMETOOLONG    37      //文件名太长
#define ENETDOWN        38      //网络不可用
#define ENETRESET       39      //连接被网络重置
#define ENETUNREACH     40      //网络不可达
#define ENFILE          41      //系统打开的文件过多
#define ENOBUFS         42      //没有缓冲区空间可用
#define ENODATA         43      //没有可用的消息
#define ENODEV          44      //没有这样的设备
#define ENOENT          45      //没有这样的文件或目录
#define ENOEXEC         46      //可执行文件格式错误
#define ENOLCK          47      //没有可用的锁
#define ENOLINK         48      //连接断开
#define ENOMEM          49      //内存不足
#define ENOMSG          50      //无类型消息
#define ENOPROTOOPT     51      //协议不可用
#define ENOSPC          52      //设备上没有剩余空间
#define ENOSR           53      //没有stream资源
#define ENOSTR          54      //不是stream
#define ENOSYS          55      //功能未实现
#define ENOTCONN        56      //socket未连接
#define ENOTDIR         57      //不是一个目录
#define ENOEMPTY        58      //目录非空
#define ENOTRECOVERABLE 59      //状态不可恢复
#define ENOTSOCK        60      //不是套接字
#define ENOTSUP         61      //不支持
#define ENOTTY          62      //不适当的I/O控制操作
#define ENXIO           63      //没有这样的设备或地址
#define EOPNOTSUPP      64      //套接字上不支持操作
#define EOVERFLOW       65      //值太大无法存储在数据类型中
#define EOWNERDEAD      66      //前一个拥有者死去
#define EPERM           67      //不允许操作
#define EPIPE           68      //管道破裂
#define EPROTO          69      //协议错误
#define EPROTONOSUPPORT 70      //协议不支持
#define EPROTOTYPE      71      //协议不适合于套接字
#define ERANGE          72      //结果太大或太小
#define EROFS           73      //只读文件系统
#define ESPIPE          74      //无效的寻道操作
#define ESRCH           75      //没有这样的进程
#define ESTALE          76      //句柄陈旧(保留)
#define ETIME           77      //STREAM ioctl超时
#define ETIMEDOUT       78      //连接或操作超时
#define ETXTBSY         79      //文本文件繁忙
#define EWOULDBLOCK     80      //操作会阻塞
#define EXDEV           81      //不正确的链接



#endif