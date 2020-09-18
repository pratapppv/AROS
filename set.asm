section         .text
        align   4
        dd      0x1BADB002
        dd      0x00
        dd      - (0x1BADB002)
        
global loader
extern vmain
loader:
        cli             ;clears the interrupts 
        call vmain      ;send processor to continue execution from the kamin funtion in c code
        hlt             ; halt the cpu(pause it from executing from this address