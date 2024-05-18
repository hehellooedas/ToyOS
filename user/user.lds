OUTPUT_FORMAT("elf64-x86-64","elf64-x86-64","elf64-x86-64")
OUTPUT_ARCH(i386:x86-64)
ENTRY(_start) #设置程序的入口地址



SECTIONS
{

	. = 0x800000;
	.text :
	{
		PROVIDE(_text = .);

		build/head.o (.text)
		EXCLUDE_FILE (build/head.o) *(.text)

		PROVIDE(_etext = .);
	}

	. = ALIGN(8);

	.data :
	{
		PROVIDE(_data = .);

		*(.data)

		PROVIDE(_edata = .);
	}

	.rodata :
	{
		PROVIDE(_rodata = .);

		*(.rodata)

		PROVIDE(_erodata = .);
	}

	. = ALIGN(32768); #32KB对齐(0x8000)

	.data.init_task :
	{
		*(.data.init_task)  #init_task_union变量被绑定到这个区域
	}

	.bss :
	{
		PROVIDE(_bss = .);

		*(.bss)

		PROVIDE(_ebss = .);
	}

	PROVIDE(_end = .);
}