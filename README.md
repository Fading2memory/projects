
Eductional Purposes only!

Not the full source material for dreadmistfall, aka dreadmist.
But proof that I can program metamorphic code.
Infect.inc and Permut.inc are not included, sorry. 


Dreadmistfall includes ~

1.) Infect.inc --- this is our file infector, plus our MBR infector it will save the virus to the MBR at an offset. It is then my job
  to find that offset in which the virus is saved too, and clear it so it does not spread. 

2.) Permut.inc --- the polymorphic/metamorphic engine in which the virus generates new copies of itself upon each new infection. 
  It is greatly difficult to spot this without heuristics analysis, but each and every day virus/malware creators finds new ways to 
  overcome and beat heuristics analysis.

 3.) Shrinker & Xpander.inc --- The shrinker acts as a compression engine, and the xpander (aka expander) is only called to either make 
  the code size either bigger or smaller on each generation (meaning, on each new infection, the virus size is either bigger or smaller).

Morph32 Assembler, DreadDebugger.asm, and DreadDebuggerInst.asm
-----------------------------------------------------------------------

Morhp32 Assembler ~
------------------------
Features: 
 - masm's and tasm's syntax, cpu directives!
 - cpu directives include: .386/.386p through .686/.686p and p386-p686
 - customized macros are allowed
 -  is writable in the .data section, do not need to active
the write flag
 - written in pure assembly.
future updates: 
 - will be a complete IDE assembler that is able to work within the command line, like td32

DreadDebbuger.asm ~
-----------------------
Features:
  - ability to debug 16-bit, 32-bit and 64-bit applications
  - will have all opcodes, registers and Flags such as EFLAGS, RFALGS, IP, EIP, and RIP
  - ability to debug .dll files, .exe files and .sys files

 DreadDebuggerInst.asm ~
 -------------------------
 Features:
  - have different color schemes for the background, text, and border colours.

Mlink32.asm ~
-----------------------
Features:
  - Compiles .obj to .exe, and also links libs to import to the exe file 
  - Updated version from tlink32.exe (borland product)
  - Changed "MZP" which is Borland signature to signify 32-bit application, and changed "This program must run under Win32"
    to "This program cannot be run in DOS mode", and added in the "Rich" section. 


As you can tell, I am just an amatuer, just trying to imrpove my skills as a programmer in assembly and other languages!
