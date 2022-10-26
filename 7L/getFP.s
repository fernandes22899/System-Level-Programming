  .globl  getFP
  .align  8
  .type   getFP,%function
getFP:
  movq    %rbp, %rax
  ret
