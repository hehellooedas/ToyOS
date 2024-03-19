#include <mouse.h>
#include <lib.h>
#include <memory.h>


static struct keyboard_inputbuffer* p_mouse = NULL;
static int mouse_count = 0;


void mouse_init(void)
{
    p_mouse = (struct keyboard_inputbuffer*)kmalloc(sizeof(struct keyboard_inputbuffer),0);
}


void analysis_mousecode(void)
{

}