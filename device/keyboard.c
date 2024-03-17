#include <keyboard.h>
#include <lib.h>
#include <memory.h>



static struct keyboard_inputbuffer* p_kb = NULL;
static int shift_l,shift_r,ctrl_l,ctrl_r,alt_l,alt_r; //几个重要的控制按键


void keyboard_init(void)
{

}



/*  驱动卸载程序  */
void keyboard_exit(void)
{
    kfree(p_kb);
}