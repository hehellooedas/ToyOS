#include <log.h>
#include <string.h>



void _log_to_screen(enum log_level level,const char* filename,const char* func,int line,char* condition,...)
{
    char log[100];
    va_list args;
    va_start(args,condition );
    int len = vsprintf(log,condition ,args );
    int str_len = strlen(log);
    va_end(args);
    switch (level) {
        case INFO:
            color_printk(WHITE,BLACK ,"[info] %s\n",log);
            break;
        case WARNING:
            color_printk(YELLOW,BLACK ,"[warning] [%s %s %d] %s\n",filename,func,line,log );
            break;
        case ERROR:
            color_printk(RED,BLACK ,"[error] [%s %s %d] %s\n",filename,func,line,log );
            break;
        case DEBUG:
            color_printk(RED,BLACK ,"[debug] [%s %s %d] %s\n",filename,func,line,log );
            break;
        default:
            color_printk(RED,BLACK ,"Parameter error occurred while logging\n" );
            break;
    }
}



static void _log_to_file()
{

}