%define NODE_SIZE 8

global find_word
extern string_length
extern string_equals

; this function is searching selected key to output word(value) with this key
find_word:
	xor rax, rax
	mov r8, rdi				; key pointer
	mov r9, rsi				; start address
	.loop:
		add r9, NODE_SIZE		; set pointer to the next element
		mov rsi, r9
		mov rdi, r8
		push r8
		push r9
		call string_equals		; compare keys
		pop r9
		pop r8
		cmp rax, 1
		je .got_key			; get if keys are equal
		mov r9, [r9 - NODE_SIZE]	; pointer to the next element
		cmp r9, 0			; pointer -> 0 means no elements 								remain. Exit.
		je .no_key
		jmp .loop
	.got_key:
		sub r9, NODE_SIZE		; set pointer at the beginning 								of needed element
		mov rax, r9
		ret
	.no_key:
		xor rax, rax			; 0 -> rax = error with finding
		ret

