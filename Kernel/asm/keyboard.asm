global readKeyCode

section .text

readKeyCode:
  in al, 0x60
  ret
