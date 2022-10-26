.align 8

.globl loadRBP
.type  loadRBP,%function
loadRBP:
movq %rbp, %rax
ret

.globl loadRBX
.type  loadRBX,%function
loadRBX:
movq %rbx, %rax
ret

.globl loadR12
.type  loadR12,%function
loadR12:
movq %r12, %rax
ret

.globl loadR13
.type  loadR13,%function
loadR13:
movq %r13, %rax
ret

.globl loadR14
.type  loadR14,%function
loadR14:
movq %r14, %rax
ret

.globl loadR15
.type  loadR15,%function
loadR15:
movq %r15, %rax
ret

.globl makeRBX
.type  makeRBX,%function
makeRBX:
movq %rdi, %rbx
ret

.globl makeR12
.type  makeR12,%function
makeR12:
movq %rdi, %r12
ret

.globl makeR13
.type  makeR13,%function
makeR13:
movq %rdi, %r13
ret

.globl makeR14
.type  makeR14,%function
makeR14:
movq %rdi, %r14
ret

.globl makeR15
.type  makeR15,%function
makeR15:
movq %rdi, %r15
ret

.globl pare
.type  pare,%function
pare:
movq %rdi, %rsp
movq %rsi, %rax
popq %rbp
ret
