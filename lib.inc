section .text 

global string_length
global print_string
global print_char
global print_newline
global print_uint
global print_int
global string_equals
global read_char
global read_word
global parse_uint
global parse_int
global string_copy
global exit

; Принимает код возврата и завершает текущий процесс
exit: 
	mov rax, 60
	syscall

; Принимает указатель на нуль-терминированную строку, возвращает её длину
string_length:
	xor rax, rax		; 0 -> rax(counter)

  .loop:			; cycle to search null-terminated symbol
	mov cl, [rdi + rax]	; fetch symbol
	test cl, cl		; maybe this symbol is 0 ->
	jz .end			; -> and logical exit
				; next symbol and
	inc rax			; counter++
	jmp .loop		; next iteration
  .end:
	ret

; Принимает указатель на нуль-терминированную строку, выводит её в stdout
print_string:
  .execute:	
	call string_length	; find a length of string
	mov rdx, rax		; prepare length to a syscall
	mov rsi, rdi		; adress of str -> rsi	
	mov rax, 1		; "write" state to syscall
	mov rdi, 1		; "stdout" description
	syscall			; output the string
  .end: 
	ret

; Принимает код символа и выводит его в stdout
print_char:
	push rdi		; saving a value to stack
	mov rsi, rsp		; link to stack value
	mov rax, 1		; "write" state to syscall
	mov rdx, 1		; length of char = 1
	mov rdi, 1		; "stdout" description
	syscall
	pop rdi			; "pop" to reset rsp
  .end:    
	ret

; Переводит строку (выводит символ с кодом 0xA)
print_newline:
	push rdi
	mov rdi, 0x0A
	call print_char
	pop rdi
	ret

; Выводит беззнаковое 8-байтовое число в десятичном формате 
; Совет: выделите место в стеке и храните там результаты деления
; Не забудьте перевести цифры в их ASCII коды.
print_uint:
	push r10		; 0->r10 
	xor r10, r10		; r10 is counter
	push r11		; 0->r11, r11 is local CONST
	mov r11, 10		; number / 10(r11)
	mov rax, rdi		; number to rax

  .divide_number:
	inc r10
	xor rdx, rdx		; 
	div r11			;
	add rdx, 30h		; number to ASCII code
	push rdx		; char to stack

	cmp rax, 0		; checking for next iteration
	jz .out_number		
	jmp .divide_number

  .out_number:
	pop rdi			; get char from stack
	call print_char

	dec r10
	test r10, r10
	jnz .out_number

  .end:
	pop r11		; return a value to r11
	pop r10		; return a value to r10
	ret

; Выводит знаковое 8-байтовое число в десятичном формате 
print_int:			; just try to use print_Uint function
  .print_sign:
	mov rax, rdi		; rdi -> rax, rax to check sign
	sar rax, 63
	and rax, 0x1
	cmp rax, 0
	je .print_num
  .prepare_negative_num:
	push rdi
	mov rdi, 2Dh
	call print_char
	pop rdi
	neg rdi
  .print_num:
	call print_uint
  .end:
	ret

; Принимает два указателя на нуль-терминированные строки, возвращает 1 если они равны, 0 иначе
string_equals:
	push rbx
	xor rbx, rbx			; counter of symb in strings
	xor rax, rax			; local data to equals
	
  .loop_equal:
	mov cl, byte [rdi+rbx]
	cmp cl, byte [rsi+rbx]
	jnz .eq_err
	
	cmp byte [rdi+rbx], 0
	jz .eq_ok

	inc rbx
	jmp .loop_equal

  .eq_ok:
	mov rax, 1
	jmp .end
  .eq_err:
	mov rax, 0
	jmp .end

  .end:
	pop rbx
	ret

; Читает один символ из stdin и возвращает его. Возвращает 0 если достигнут конец потока
read_char:
	
	push 0			; decrement rsp to not lose rdx	

	xor rax, rax
	mov rdi, 0
	mov rsi, rsp 
	mov rdx, 1
	syscall			; value -> rax

	cmp rax, 0
	je .end
	mov rax, [rsp]			
  .end:	
	pop rdx
	ret 

; Принимает: адрес начала буфера(rdi), размер буфера(rsi)
; Читает в буфер слово из stdin, пропуская пробельные символы в начале, .
; Пробельные символы это пробел 0x20, табуляция 0x9 и перевод строки 0xA.
; Останавливается и возвращает 0 если слово слишком большое для буфера
; При успехе возвращает адрес буфера в rax, длину слова в rdx.
; При неудаче возвращает 0 в rax
; Эта функция должна дописывать к слову нуль-терминатор

read_word:
  .delete_spaces:			; delete all start spaces symb
	push rdi
	push rsi
	push rdx	
	call read_char
	pop rdx
	pop rsi
	pop rdi	

	cmp rax, 0x09
	je .delete_spaces
	cmp rax, 0x0A
	je .delete_spaces
	cmp rax, 0x20
	je .delete_spaces
	
  .before_loop:
	xor rdx, rdx			; 0 -> counter
  .read_char_loop:
	cmp rdx, rsi
	je .str_error
	
	cmp rax, 0x00			; checking the end of word
	je .str_ok
	cmp rax, 0x09
	je .str_ok
	cmp rax, 0x0A
	je .str_ok
	cmp rax, 0x20
	je .str_ok

	mov byte [rdi + rdx], al
	inc rdx
	
	push rdi
	push rsi
	push rdx
	call read_char
	pop rdx
	pop rsi
	pop rdi	

	jmp .read_char_loop

  .str_ok:
	mov byte [rdi + rdx], 0		; null-term
	mov rax, rdi			; returning adress of buf
	jmp .end
  .str_error:
	xor rax, rax			; 0 -> result
	jmp .end
  .end:	
 	ret

; Принимает указатель на строку, пытается
; прочитать из её начала беззнаковое число.
; Возвращает в rax: число, rdx : его длину в символах
; rdx = 0 если число прочитать не удалось
parse_uint:
	push rbx

	xor rdx, rdx
	xor rax, rax
	xor rbx, rbx
  .loop:				
	mov bl, byte [rdi + rdx]	; taking a char   
	cmp bl, 0x30			; checking a char
	jl .end				; to into digit
	cmp bl, 0x39			; interval into
	jg .end				; ASCII table
	
	sub bl, 0x30			; chat_to_number

	push rdx
	mov rdx, 10
	mul rdx				; shift to next rank
	pop rdx
	add rax, rbx			; put next digit in the res

	inc rdx				; length++
	jmp .loop			

  .end:
	pop rbx
	ret



; Принимает указатель на строку, пытается
; прочитать из её начала знаковое число.
; Если есть знак, пробелы между ним и числом не разрешены.
; Возвращает в rax: число, rdx : его длину в символах (включая знак, если он был) 
; rdx = 0 если число прочитать не удалось
parse_int:
	push rdi	
	
  .first_symb:
	mov rax, rdi			; check first symbol
	
	cmp byte [rax], '-'		; check negative
	jz .neg_number

	cmp byte [rax], '+'		; check plus
	jz .pos_number

	call parse_uint			; without sign
	jmp .end	

  .pos_number:
	inc rdi
	call parse_uint
	cmp rdx, 0
	jz .end

	inc rdx
	jmp .end

  .neg_number:
	inc rdi
	call parse_uint
	cmp rdx, 0
	jz .end

	inc rdx
	neg rax
	jmp .end

  .end:
	pop rdi
	ret 

; Принимает указатель на строку, указатель на буфер и длину буфера
; Копирует строку в буфер
; Возвращает длину строки если она умещается в буфер, иначе 0
; rdi -> на строку, rsi -> на буфер, rdx -> на длину буфера
string_copy:
	xor rax, rax		; 0 -> rax

	push rdi		; saving all arguments 
	push rsi		; to a stack
	push rdx		;
	call string_length	; calling a string_length function
	pop rdx			; to find a length of string
	pop rsi			; then length(of str) into a rax
	pop rdi			; then restore our arguments
	
	cmp rax, rdx		; check the buffer size for string placement 
	jg .err_size		; exit if str_size > buf_size
	
	push rax		; saving str size
	push r12		; r12 now is buffer

  .loop_copy:
	cmp rax, 0
	jl .end
	dec rax
	mov r12, [rdi+rax]	; symb_str ->	
	mov [rsi+rax], r12	; -> symb_buf
	cmp rax, 0		; is it last(first) symb?
	jz .end
	jmp .loop_copy

  .end:
	pop r12
	pop rax
  .err_size:
	mov rax, 0
	ret


