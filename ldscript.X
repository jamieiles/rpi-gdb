OUTPUT_ARCH(arm)
ENTRY(_vec_reset)
SECTIONS {
	.text.vectors 0x00000000 : {
		*(.text.vectors)
	}

	.text : {
		*(.text*)
	}

	.rodata : {
		*(.rodata)
		*(.rodata.*)
		*(.rodata.str1.4)
		*(.shstrtab)
		*(.strtab)
	}

	__bss_start = . ;
	.bss : {
		*(.bss)
	}
	__bss_end = . ;

	__bug_start = . ;
	.bugtable : {
		*(.bugtable)
	}
	__bug_end = . ;

	/DISCARD/ : {
		*(.note.*)
	}
}
