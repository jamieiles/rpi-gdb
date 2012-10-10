OUTPUT_ARCH(arm)
ENTRY(_entry)
SECTIONS {
	.text 0x00008000 : {
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

	/DISCARD/ : {
		*(.note.*)
	}
}
