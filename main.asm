%include "words.inc"
%include "lib.h"

%define BUFF_SIZE 255
%define NODE_SIZE 8

extern find_word

global _start

section .data

key_not_found:
	db "this key not found", 0
buffer_overflow:
	db "overflow of buffer", 0
string_buffer:
	times BUFF_SIZE db 0

section .text

_start:
	xor rax, rax
	mov rdi, string_buffer
	mov rsi, BUFF_SIZE
	call read_word
	test rax, rax			; 0 means buffer error
	jne .buffer_ok			; address -> rax, length -> rdx
	mov rdi, buffer_overflow
	call print_err
	call print_newline
	call exit

; branch if name of key is valid
; checking this key in a set of all keys
.buffer_ok:
	mov rdi, rax
	mov rsi, x0
	push rdx			; store length
	call find_word
	pop rdx				; restore length
	test rax, rax			; 0 is error with founding
	jne .key_ok
	jmp .key_err			; call to err of searching key

; search a needed key successfully		
.key_ok:				
	add rax, NODE_SIZE		; +offset to point to the next element
	add rax, rdx			; +key length
	add rax, 1			; +null-terminator
	mov rdi, rax
	call print_string
	call print_newline
	call exit

; error with search needed key
.key_err:				
	mov rdi, key_not_found
	call print_err
	call print_newline
	call exit

print_err:
	xor rax, rax
	mov rsi, rdi
	call string_length
	mov rdx, rax
	mov rdi, 2
	mov rax, 1
	syscall
	ret

