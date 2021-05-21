.text
.globl tigermain
.type tigermain, @function
tigermain:
pushl %ebp
movl %esp, %ebp
subl $12, %esp
L21:
movl %ebx, -12(%ebp)
movl %esi, -8(%ebp)
movl %edi, -4(%ebp)
movl $4, %eax
pushl %eax
call allocRecord
addl $4, %esp
movl $1, %ecx
movl %ecx, 0(%eax)
movl %eax, %edi
movl $4, %eax
pushl %eax
call allocRecord
addl $4, %esp
movl $1, %ecx
movl %ecx, 0(%eax)
movl %eax, %esi
movl $8, %eax
pushl %eax
call allocRecord
addl $4, %esp
movl $1, %ecx
movl %ecx, 0(%eax)
movl $0, %ecx
movl %ecx, 4(%eax)
movl %eax, %ebx
cmpl %esi, %edi
je L2
L3:
movl $.L1, %eax
pushl %eax
call print
addl $4, %esp
L4:
cmpl %esi, %esi
je L7
L8:
movl $.L6, %eax
pushl %eax
call print
addl $4, %esp
L9:
movl %ebx, 4(%ebx)
movl 4(%ebx), %eax
cmpl %ebx, %eax
je L12
L13:
movl $.L11, %eax
pushl %eax
call print
addl $4, %esp
L14:
movl $8, %eax
pushl %eax
call allocRecord
addl $4, %esp
movl $1, %ecx
movl %ecx, 0(%eax)
movl $0, %ecx
movl %ecx, 4(%eax)
movl 4(%ebx), %ecx
movl %ecx, 4(%eax)
movl 4(%eax), %eax
cmpl %ebx, %eax
je L17
L18:
movl $.L16, %eax
pushl %eax
call print
addl $4, %esp
L19:
movl $0, %eax
movl -4(%ebp), %edi
movl -8(%ebp), %esi
movl -12(%ebp), %ebx
jmp L20
L2:
movl $.L0, %eax
pushl %eax
call print
addl $4, %esp
jmp L4
L7:
movl $.L5, %eax
pushl %eax
call print
addl $4, %esp
jmp L9
L12:
movl $.L10, %eax
pushl %eax
call print
addl $4, %esp
jmp L14
L17:
movl $.L15, %eax
pushl %eax
call print
addl $4, %esp
jmp L19
L20:
leave
ret


.section .rodata
.L16:
.string "\x0A\x00\x00\x00\x78\x78\x2E\x62\x20\x21\x3D\x20\x78\x0A"

.section .rodata
.L15:
.string "\x09\x00\x00\x00\x78\x78\x2E\x62\x20\x3D\x20\x78\x0A"

.section .rodata
.L11:
.string "\x09\x00\x00\x00\x78\x2E\x62\x20\x21\x3D\x20\x78\x0A"

.section .rodata
.L10:
.string "\x08\x00\x00\x00\x78\x2E\x62\x20\x3D\x20\x78\x0A"

.section .rodata
.L6:
.string "\x07\x00\x00\x00\x63\x20\x21\x3D\x20\x62\x0A"

.section .rodata
.L5:
.string "\x06\x00\x00\x00\x63\x20\x3D\x20\x62\x0A"

.section .rodata
.L1:
.string "\x07\x00\x00\x00\x61\x20\x21\x3D\x20\x62\x0A"

.section .rodata
.L0:
.string "\x06\x00\x00\x00\x61\x20\x3D\x20\x62\x0A"

