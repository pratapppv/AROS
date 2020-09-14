## Welcome to AROS-A Random Operating System

This is the accompanying documentation for an old project of mine where in I had written a basic *x86* OS to get my hands dirty before jumping into my actual project of writing a distributed OS for small clusters. 

### AROS's functionality

AROS as it stands today is a minimalist OS which can boot up and execute an associated file which is attached with the OS during boot up. In terms of I/O AROS supports keyboard input and output to a VGA style display. The code comprises with specific sections of x86 assembly which might not be compatible with all Intel and AMD based Laptops and PC's.

### Disclaimer

I have not physically booted the OS on a computer(as yest) and have tested it on a virtual machine and am not responsible if anything goes wrong while utilizing this code or  any modifications of it xD.

## Booting
Between a person physically turning on a computer to the system loading up the OS and booting from it, a number of intermediate tasks are performed before execution control is passed to the OS. Our focus is limited till after the BIOS passes execution control t the booloader. For our bootloader, we are going to be using GRUB(GRand unified Bootloader) about which you can learn more about on [GRUB's Homepage](https://www.gnu.org/software/grub/). An interesting side note is that modern day x86/x86_64 processors have multiple modes of operations

1. 16bit real mode
2. 16bit protected mode
3. 32bit protected mode
4. 64bit long mode (present in x86_64 systems)

When the BIOS runs, the processor is configured to run in the 16bit real mode and GRUB reconfigures the CPU to run in the 32bit protected mode, allowing us to access upto 4GB of memory.

Coming back to the topic of using GRUB as a bootloader for our OS, GRUB is a multiboot compliant bootloader which requires us to perform certain additional steps in order for GRUB to recognize our kernel. Digging into the documentation for multiboot ([OSDev's](https://wiki.osdev.org/Multiboot#:~:text=The%20original%20Multiboot%20specification%20was,themselves%20with%20magic%20number%200x2BADB002.) page on it is a pretty good starting point) we can see three important parameters:

1. Magic number
2. Flags
3. Checksum

The magic number is an arbitrary constant chosen by the multiboot specification which for multiboot complient kernels is chosen to be `0x1BADB002`. The Flags parameter determines the modules which must be loaded along with the kernel which we will be setting to 0. More information regarding the function of the flags parameter and bitwise information can be found in the documentation for the [multiboot standard](https://www.gnu.org/software/grub/manual/multiboot/multiboot.html#Header-magic-fields). Finally, the checksum is a constant such that the sum of the above fields is 0. This is merely a validation to ensure that none of the data is corrupted. These three fields are required while the rest of the fields present in the multiboot header can be ignored as they directly deal with the other modules to be loaded and thus governed by the Flag field. As we are setting this field to be 0, we can safely ignore the presence of the other fields.
Now that we know the fields which must be present, we also must know where these fields must be placed in memory. The documentation defines that these constants must be loaded into memory with offsets of 4bytes in the same format as I have described and must be within the first 8192bytes of the Operating System image. Apart from this, we need to know the memory address from which these parameters must be loaded. As GRUB utilizes memory below 1MB, we have to set these constants to be after the 1MB mark.

### Assembly code
Okay, now that we know where what needs to be in place for GRUB to recognize our kernel, we need to write code for the same. In theory, we could use C to implement this, but I'm going to perform this using assembly with the following snippet of code,
```assembly
;set.asm
section .text
global loader

MAGIC_NUMBER equ 0x1BADB002

 align 4
  DD MAGICN MAGIC_NUMBER
  DD FLAGS 0x00000000
  DD CHKSM -MAGIC_NUMBER

extern vmain

loader:
  cli
  call vmain
  hlt
```
### Explanation of the code
The first part of the assembly code loads the constants into memory and is pretty staight forward. Once the header is loaded onto memory, we are in position to call our C code which is the main kernel. In order to do so, we must first inform our assembler regarding the existance of the kernel's main function, the entry point, `vmain` which the line `extern vmain` accomplishes. Before calling `vmain` i.e pass execution controll to it, we need to clear the interrupts which the instruction `cli` does. The instruction `hlt` halts the CPU if ever the kernel stopped execution and passed execution controll back to this piece of assembly code. `align 4` ensures that our header is 4byte (32 bit) aligned.
Before we understand the function of the line `section .text` we must take a look at memory models and memory segments. In general, there are three major and important memory segments namely

1. Code Segment
2. Data Segment
3. Stack Segment

#### Code Segment
The code segment contains the actual code that is to be executed in memory. This is a fixed segment and the start of this section is indicated to the linker using `.text`  attribute.

### Data Segment
The data segment contains the data such as constants, variables etc. which is required for the code to function. This section is indicated to the linker using the `.data` and the `.bss` attributes. The `.bss` section is used to reserve memory which variables that are to be declared later in the program and is a static field which is initialized with 0's in the memory.

### Stack Segment
As the name implies, the stack segment is used for the program stack and must be setup so that we can run non trivial C programms. We will be looking at setting up the stack segment in the following sections.

### Linker script
While we have the header ready, we have not yet defined the memory address where the header must be placed. This is done by the linker and we must define the linker script according to the specification. From the previous section, we know that we must place the `.text` section at the 1MB mark which translates to `0x100000`. For now, we'll set the `.data` and the `.bss` section also to begin at this address.
So translating this to a linker script, we get the following linker script:

```
OUTPUT_FORMAT(elf32-i386)
ENTRY(loader)
SECTIONS
 {
   . = 1M;
   .text : { *(.text) }
   .data : { *(.data) }
   .bss  : { *(.bss)  }
 }
```

## Setting up the Stack Segment
Before we speak about the stack segment, we must understand

1. Memory Models in 32bit modes
2. Segment registers

When operating in 32bit mode, we utilize a memory model known as *flat memory model* in which any 32bit register can be utilized for memory addressing. This is in contrast to the Multi-Segmnent memory model which was primaryly used in the 16bit era and the iconic 8086 microprocessor. Short digression into the 8086 microprocessor, there are primarily two sets of registers
1. General Purpose Registers
2. Segment Registers

### General Purpose Registers
The list of 16bit GPR's are as follows
1. AX: Accumulator register
2. BX: Base register to be be used in conjunction with the segment register(default is DS)
3. CX: Counter register
4. DX: Data register
5. SP: Stack Pointer which points to the top of the stack
6. BP: Base Pointer which points to the base of the stack
7. SI: Source Index which is to be used in conjunction with the segment register(default is DS)
8. DI: Destination Index which is to be used in conjunction with the segment register(default is DS)

The 32bit versions of these registers can be accessed in the 32bit protected mode by adding an E prefix (Ex:AX would be accessed as EAX)

Note: These GPRs are specific to certain instructions, for example CX is implicitly used with loop instructions

### Segment Registers
Segment registers were extensively used in the 16bit era to hold the base address for each of the memory segments. This method allowed the 8086 to address upto 1MB of memory using 20bit address bus. In order to calculate the address of a variable, the content of the segment register was left shifted by 4bits and the contents of the corresponding offset address are summed to determine the physical address. A memory model which utilizes these segment registers is known as Multi-Segmented memory model which is in stark contrast to the flat memory model where no such partitioning exists.

### Initializing Stack Segment
The stack in the x86 architecture is a downward growing stack which means that the stack pointer is decremented everytime a `PUSH` instruction is encountered and is incremented when the `POP` instruction is incremented. Therefore, the base address of the stack must be set in such a way that no memory conflicts occur and data is not overwritten. For our simple OS, we will be leaving the stack memory allocation job to the C compiler(gcc) and trusting the linker. One change that we must do to the linker script is to introduce a command that will combine all the `.bss` sections.
```
.bss:
{
  *(COMMON)
  *(.bss)
}

```

