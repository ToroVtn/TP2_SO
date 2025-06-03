GLOBAL enterCritical
GLOBAL exitCritical

enterCritical:
  mov rax,1
  xchg rax,[rdi]
  cmp rax,0
  je exit
  int 22h
  jmp enterCritical
  exit:
  ret
    
exitCritical:
   mov qword [rdi], 0
   ret