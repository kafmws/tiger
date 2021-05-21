.text
.globl tigermain
.type tigermain, @function
tigermain:
pushl %ebp
movl %esp, %ebp
L9:
movl $1, %eax
movl $1, %eax
cmpl $3, %eax
jg L0
L1:
movl $1, %eax
movl $3, %ecx
cmpl $2, %ecx
jg L3
L4:
movl $0, %eax
L3:
L2:
addl $48, %eax
pushl %eax
call chr
addl $4, %esp
pushl %eax
call print
addl $4, %esp
movl $.L7, %eax
pushl %eax
call print
addl $4, %esp
jmp L8
L0:
movl $1, %eax
movl $2, %ecx
cmpl $3, %ecx
jg L5
L6:
movl $0, %eax
L5:
jmp L2
L8:
leave
ret


.section .rodata
.L7:
.string "\x01\x00\x00\x00\x0A"

