GLOBAL enterCritical
GLOBAL exitCritical

enterCritical:
  mov eax,1
  xchg eax,[rdi]
  cmp eax,0
  je exit
  int 22h
  jmp enterCritical
  exit:
  ret
    
exitCritical:
   mov dword [rdi], 0
   ret