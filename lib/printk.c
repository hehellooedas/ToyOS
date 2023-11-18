#include <printk.h>
#include <lib.h>
#include <stddef.h>
#include <stdbool.h>


char buf[4096] = {0};
struct position Pos;
extern unsigned char font_ascii[256][16];


int skip_atoi(const char** s){
    int i = 0;
    while(is_digit(**s)){
        i = i * 10 + *((*s)++) - '0';
    }
    return i;
}


/*  整形转字符串(最高支持36进制)  */
static char* number(char* str,long num,int base,int size,int precision,int type){
    if(base < 2 || base > 36) return false;  //不满足条件
    char c,sign,tmp[50];  //填充符号,前缀,数字缓存
    int i;

    /*  确定字符的大小写  */
    const char*      digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    if(type & SMALL) digits = "0123456789abcdefghijklmnopqrstuvwxyz";

    /*  确定用空格填充还是0填充  */
    if(type & LEFT) type &= ~ZEROPAD;
    c = (type & ZEROPAD)? '0':' ';

    /*  确定前缀  */
    sign = 0;
    if(type & SIGN && num < 0){
        sign = '-';
        num = -num;
    }else{
        sign = (type & PLUS) ? '+':((type & SPACE) ? ' ' : 0);
    }

    /*  前缀会占用格式宽度  */
    if(sign) size--;
    if(type & SPECIAL){
        if(base == 16) size -= 2;  //0x占用两个宽度
        else if(base == 8) size--; //0占用一个宽度
    }

    /*  把数值部分进行转换  */
    i = 0;
    if(num == 0){
        tmp[i++] = '0';
    }else{
        while(num != 0){
            tmp[i++] = digits[do_div(num,base)];  //商会自动写回到num中
        }
    }

    /*    */
    if(i > precision) precision = i;
    size -= precision;

    if(!(type & (ZEROPAD + LEFT))){
        while(size-- > 0){
            *str++ = ' ';
        }
    }

    if(sign){  //加上符号
        *str++ = sign;
    }
    if(type & SPECIAL){  //加上特殊前缀
        switch (base) {
            case 2:
                *str++ = '0';
                *str++ = digits[11];
                break;
            case 8:
                *str++ = '0';
                break;
            case 16:
                *str++ = '0';
                *str++ = digits[33];  //由于不知道x的大小写情况所以直接用digits来判断
                break;
        }
    }

    /*  如果向右对齐  */
    if(!(type & LEFT)){
        while(size-- > 0){
            *str++ = c;
        }
    }
    /*  数据宽度不足则拿0当前缀  */
    while(i < precision--){
        *str++ = '0';
    }
    /*  把数据部分加上去  */
    while(i-- > 0){
        *str++ = tmp[i];
    }
    while(size-- > 0){
        *str++ = ' ';
    }
    return str;
}



/* 格式化字符串到缓冲区 */
int vsprintf(char* buf,const char* fmt,va_list args){
    char* str,*s;
    int flags;
    int field_width;  //指定输出字段的宽度
    int precision;    //对于浮点数,指定小数点后的位数(%.2f) | 对于字符串,指定最大输出字符个数(%.2s)
    int len,i;
    int qualifier;    //指定数据宽度(例:%lld)
    for(str=buf;*fmt;fmt++){
        if(*fmt != '%'){
            *str++ = *fmt;
            continue;
        }
    /*----------------------------------*/
        flags = 0;
        /*  格式修饰符(不在意顺序且可以不止一个)  */
        repeat:
            fmt++;
            switch (*fmt) {
                case '-':    //指定左对齐输出
                    flags |= LEFT;
                    goto repeat;
                case '+':    //在正数前面也输出+符号
                    flags |= PLUS;
                    goto repeat;
                case ' ':    //正数前面带空格,负数则带符号
                    flags |= SPACE;
                    goto repeat;
                case '#':    //为八进制和十六进制数添加前缀
                    flags |= SPECIAL;
                    goto repeat;
                case '0':    //用0来填充字段宽度指定的空白部分
                    flags |= ZEROPAD;
                    goto repeat;
            }

            /*  确定宽度  */
            field_width = -1;
            if(is_digit(*fmt)){
                field_width = skip_atoi(&fmt);
            }else if(*fmt == '*'){  //如果%后面不是数而是*则宽度在参数列表中
                fmt++;
                field_width = va_arg(args,int); //从参数列表中获取下一个参数并认为它是int类型
                if(field_width < 0){   //确定对齐方向
                    field_width = -field_width;
                    flags |= LEFT;
                }
            }

            /*  确定数据精度  */
            precision = -1;
            if(*fmt == '.'){
                fmt++;
                if(is_digit(*fmt)){
                    precision = skip_atoi(&fmt);
                }else if(*fmt == '*'){
                    fmt++;
                    precision = va_arg(args, int);  
                }
                if(precision < 0){
                    precision = 0;
                }
            }

            /*  确定数据的位数(h变短,l变长)  */
            qualifier = -1;
            if(*fmt == 'h' || *fmt == 'l' || *fmt == 'L' || *fmt == 'Z'){
                qualifier = *fmt;
                fmt++;
            }

            /*  确定打印的类型  */
            switch (*fmt) {
                case 'c':  //打印一个字符
                    if(!(flags & LEFT)){  //默认向右对齐
                        while(--field_width > 0){
                            *str++ = ' ';
                        }
                    }
                    *str++ = (unsigned char)va_arg(args,int);
                    while(--field_width > 0){
                        *str++ = ' ';
                    }
                    break;

                case 's':  //打印字符串
                    s = va_arg(args, char*);
                    if(!s){  //如果该字符串是空指针则设定为空串
                        s = "";
                    }
                    len = strlen(s);
                    if(precision < 0){
                        precision = len;
                    }else if(precision < len){  //设定的精度小于字符串长度
                        len = precision;  //则按照字符串长度来计算
                    }
                    if(!(flags & LEFT)){
                        while(len < field_width--){
                            *str++ = ' ';
                        }
                    }
                    for(int i=0;i<len;i++){
                        *str++ = *s++;
                    }
                    while(len < field_width--){
                        *str++ = ' ';
                    }
                    break;

                case 'o':  //打印一个八进制数
                    if(qualifier == '1'){
                        str = number(str, va_arg(args,unsigned long), 8, field_width, precision, flags);
                    }else{
                        str = number(str, va_arg(args,int), 8, field_width, precision, flags);
                    }
                    break;

                case 'p':  //打印一个地址
                    if(field_width == -1){
                        field_width = 2 * sizeof (void*);
                        flags |= ZEROPAD;
                    }
                    str = number(str, (unsigned long)va_arg(args,void*), 16, field_width, precision, flags);
                    break;

                case 'b':
                    flags |= SMALL;
                case 'B':
                    if(qualifier == 'l'){
                        str = number(str, va_arg(args,unsigned long), 2, field_width, precision, flags);
                    }else{
                        str = number(str, va_arg(args,int), 2, field_width, precision, flags);
                    }
                    break;

                case 'x':  //打印一个十六进制数
                    flags |= SMALL;
                    /*  不需要break直接下去执行  */
                case 'X':
                    if(qualifier == 'l'){
                        str = number(str, va_arg(args,unsigned long), 16, field_width, precision, flags);
                    }else{
                        str = number(str, va_arg(args,int), 16, field_width, precision, flags);
                    }
                    break;

                case 'd':
                case 'i':
                    flags |= SIGN;
                case 'u':
                    if(qualifier == 'l'){
                        str = number(str, va_arg(args,unsigned long), 10, field_width, precision, flags);
                    }else{
                        str = number(str, va_arg(args,unsigned int), 10, field_width, precision, flags);
                    }
                    break;

                case 'n':
                    if(qualifier == 'l'){
                        long* ip = va_arg(args, long*);
                        *ip = (str - buf);
                    }else{
                        int* ip = va_arg(args, int*);
                        *ip = (str - buf);
                    }
                    break;

                case '%':  //输出一个%
                    *str++ = '%';
                    break;

                default:  //遇到了未定义的字符
                    *str++ = '%';
                    if(*fmt){
                        *str++ = *fmt;
                    }else{
                        fmt--;
                    }
                    break;
            }
    }
    return str - buf;  //生成的字符串的长度
}



void putchar(unsigned int* fb,int Xsize,int x,int y,unsigned int FRcolor,unsigned int BKcolor,char font){
    unsigned int* addr = NULL;
    unsigned char* fontp = NULL;
    int testval = 0;
    fontp = font_ascii[font];
    for(int i=0;i<16;i++){   //每个字符16行
        addr = fb + Xsize * (y + i) + x;   //i+1 意味着地址也加了一行
        testval = 0x100;     //1 0000 0000b
        for(int j=0;j<8;j++){   //每行8个像素
            testval = testval >> 1;
            if(*fontp & testval){
                *addr = FRcolor;
            }else{
                *addr = BKcolor;
            }
            addr++;
        }
        fontp++;  //下一个字节
    }
}



int color_printk(unsigned int FRcolor,unsigned int BKcolor,const char* fmt,...){
    int i=0,count=0,line=0;

    /* 先格式化字符串  */
    va_list args;
    va_start(args, fmt);
    i = vsprintf(buf,fmt,args);  //返回字符串长度
    va_end(args);

    for(count=0;count<i||line;count++){
        if(line > 0){
            count--;
            goto Label_Tab;
        }
        if((unsigned char)*(buf + count) == '\n'){  //换行
            Pos.YPosition++;
            Pos.XPosition = 0;
        }else if((unsigned char)*(buf + count) == '\b'){
            Pos.XPosition--;
            /*  解决连锁反应  */
            if(Pos.XPosition < 0){
                Pos.XPosition = Pos.XResolution / Pos.XCharSize - 1;
                Pos.YPosition--;
                if(Pos.YPosition < 0){
                    Pos.YPosition = Pos.YResolution / Pos.YCharSize - 1;
                }
            }
            putchar(Pos.FB_addr, Pos.XResolution ,Pos.XPosition * Pos.XCharSize, Pos.YPosition * Pos.YCharSize, FRcolor, BKcolor, ' ');
        }else if((unsigned char)*(buf + count) == '\t'){   //制表符
            line = (((Pos.XPosition + 8) & ~(8 - 1)) - Pos.XPosition);

        Label_Tab:
            line--;
            putchar(Pos.FB_addr, Pos.XResolution, Pos.XPosition * Pos.XCharSize, Pos.YPosition * Pos.YCharSize, FRcolor, BKcolor, ' ');
            Pos.XPosition++;
        }else{   //普通字符
            putchar(Pos.FB_addr, Pos.XResolution, Pos.XPosition * Pos.XCharSize, Pos.YPosition * Pos.YCharSize, FRcolor, BKcolor, (unsigned char)*(buf + count));
            Pos.XPosition++;
        }

        if(Pos.XPosition >= (Pos.XResolution / Pos.XCharSize)){
            /*  主动换行  */
            Pos.XPosition = 0;
            Pos.YPosition++;
        }
        if(Pos.YPosition >= (Pos.YResolution / Pos.YCharSize)){
            Pos.YPosition = 0;
        }
    }
    return i;
}