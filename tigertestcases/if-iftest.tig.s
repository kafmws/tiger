.text
.globl tigermain
.type tigermain, @function
tigermain:
pushl %ebp
movl %esp, %ebp
L10:
movl $1, %eax
cmpl $3, %eax
jg L6
L7:
movl $.L5, %eax
pushl %eax
call print
addl $4, %esp
L8:
movl $0, %eax
jmp L9
L6:
movl $2, %eax
cmpl $3, %eax
jl L2
L3:
movl $.L1, %eax
pushl %eax
call print
addl $4, %esp
L4:
jmp L8
L2:
movl $.L0, %eax
pushl %eax
call print
addl $4, %esp
jmp L4
L9:
leave
ret


.section .rodata
.L5:
.string "\x05\x00\x00\x00\x31\x3C\x3D\x33\x0A"

.section .rodata
.L1:
.string "\x0C\x00\x00\x00\x31\x3E\x33\x20\x26\x26\x20\x32\x3E\x3D\x33\x0A"

.section .rodata
.L0:
.string "\x0B\x00\x00\x00\x31\x3E\x33\x20\x26\x26\x20\x32\x3C\x33\x0A"

