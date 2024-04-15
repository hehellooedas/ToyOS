#include <log.h>




void _log_to_screen(enum log_level level,char* filename,char* func,int line,char* condition,...)
{
    char log[50];
    va_list args;
    va_start(args,condition );
    vsprintf(log,condition ,args );
    switch (level) {
        case INFO:
            color_printk(WHITE,BLACK ,"[info] %s\n",log );
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



void _log_to_file()
{

}