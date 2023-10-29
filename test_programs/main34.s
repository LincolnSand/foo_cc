.bss
.align 4
.globl c
c:
.zero 4
.data
.align 4
.globl g
g:
.long 6
.bss
.align 1
.globl l
l:
.zero 1
.data
.align 1
.globl t
t:
.byte 99
.bss
.align 8
.globl f
f:
.zero 8
.data
.align 4
.globl nt
nt:
.long 7009
.data
.align 8
.globl dob
dob:
.quad 4612512428959010390
.data
.align 4
.globl flo
flo:
.long 1129839788
.data
.align 4
.globl in
in:
.long 9
.text
.globl main
main:
pushq %rbp
movq %rsp, %rbp
pushq %rdi
pushq %rsi
pushq %rdx
pushq %rcx
pushq %r8
pushq %r9
subq $16, %rsp
movdqu %xmm0, (%rsp)
subq $16, %rsp
movdqu %xmm1, (%rsp)
subq $16, %rsp
movdqu %xmm2, (%rsp)
subq $16, %rsp
movdqu %xmm3, (%rsp)
subq $16, %rsp
movdqu %xmm4, (%rsp)
subq $16, %rsp
movdqu %xmm5, (%rsp)
subq $16, %rsp
movdqu %xmm6, (%rsp)
subq $16, %rsp
movdqu %xmm7, (%rsp)
pushq $9
popq %rax
movdqu (%rsp), %xmm7
addq $16, %rsp
movdqu (%rsp), %xmm6
addq $16, %rsp
movdqu (%rsp), %xmm5
addq $16, %rsp
movdqu (%rsp), %xmm4
addq $16, %rsp
movdqu (%rsp), %xmm3
addq $16, %rsp
movdqu (%rsp), %xmm2
addq $16, %rsp
movdqu (%rsp), %xmm1
addq $16, %rsp
movdqu (%rsp), %xmm0
addq $16, %rsp
popq %r9
popq %r8
popq %rcx
popq %rdx
popq %rsi
popq %rdi
movq %rbp, %rsp
popq %rbp
ret
