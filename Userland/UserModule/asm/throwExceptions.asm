global commandInvalidOpcodeError

section .text

commandInvalidOpcodeError:
    ud2
    ret