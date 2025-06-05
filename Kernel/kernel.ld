OUTPUT_FORMAT("binary")
ENTRY(loader)
SECTIONS
{
	.text 0x100000 :
	{
		kernelText = .;
		*(.text*)
		. = ALIGN(0x1000);
		kernelRodata = .;
		*(.rodata*)
	}
	.data ALIGN(0x1000) :/* AT(ADDR(.data)) Por lo que investigué esto está al pedo...*/
	{
		kernelData = .;
		*(.data*)
		endOfKernelBinary = .;
	}
	.bss ALIGN(0x1000) :/* AT(ADDR(.bss)) */
	{
		kernelBss = .;
		*(.bss*)
		*(EXCLUDE_FILE (*.o) COMMON)
	}
	. = ALIGN(0x1000);
	endOfKernel = .;
}

/*
 *
 * text                 ->   0x100000
 * rodata               ->   Align(text + sizeof(text))
 * data                 ->   Align(rodata + sizeof(rodata))
 * endOfKernelBinary    ->   data + sizeof(data)
 * bss                  ->   Align(endOfKernelBinary)
 * endOfKernel          ->   Align(bss + sizeof(bss))
 *
 * Eg:
 * text                 ->   0x100000 - 0x219087
 * rodata               ->   0x220000 - 0x221349
 * data                 ->   0x222000 - 0x228932
 * endOfKernelBinary    ->   0x228932
 * bss                  ->   0x229000 - 0x331234
 * endOfKernel          ->   0x332000
 *
**/
