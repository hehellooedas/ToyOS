OUTPUT_FORMAT("elf64-x86-64","elf64-x86-64","elf64-x86-64")
OUTPUT_ARCH(i386:x86-64)
ENTRY(_start) #设置程序的入口地址
SECTIONS
{

	. = 0xffff800000000000 + 0x100000;
	.text :
	{
		_text = .;
		*(.text)

		_etext = .;
	}
	. = ALIGN(8);
	.data :
	{
		_data = .;
		*(.data)
		
		_edata = .;
	}
	.rodata :
	{
		_rodata = .;
		*(.rodata)

		_erodata = .;
	}
	. = ALIGN(32768); #32KB对齐(0x8000)

	.data.init_task :
	{
		*(.data.init_task)  #init_task_union变量被绑定到这个区域
	}
	
	.bss :
	{
		_bss = .;
		*(.bss)
		_ebss = .;
	}

	_end = .;  #表示整个程序的结束
}