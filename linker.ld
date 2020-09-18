OUTPUT_FORMAT(elf32-i386)
ENTRY(loader)
SECTIONS
 {
   . = 1M;
   .text : { *(.text) }
   .data : { *(.data) }
   .bss  : { 
				*(COMMON)
				*(.bss)  
			}
 }