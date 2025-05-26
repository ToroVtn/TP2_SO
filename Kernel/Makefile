include Makefile.inc

OBJDIR := objs
CDIR := c
ASMDIR := asm

KERNEL := kernel.bin
KERNEL_DEBUG := $(KERNEL:.bin=.elf)

SOURCES_C := $(wildcard $(CDIR)/*.c)
OBJECTS_C := $(addprefix $(OBJDIR)/,$(SOURCES_C:%.c=%.o))
SOURCES_ASM := $(wildcard $(ASMDIR)/*.asm)
OBJECTS_ASM := $(addprefix $(OBJDIR)/,$(SOURCES_ASM:%.asm=%.o))

LOADERSRC := loader.asm
LOADEROBJECT := $(addprefix $(OBJDIR)/,$(LOADERSRC:.asm=.o))

LINK_SCRIPT := kernel.ld

STATICLIBS := 

ALL_OBJECTS := $(LOADEROBJECT) $(OBJECTS_C) $(OBJECTS_ASM) $(STATICLIBS)

all: $(KERNEL)

$(KERNEL): $(ALL_OBJECTS)
	$(LD) $(LDFLAGS) -T $(LINK_SCRIPT) -o $(KERNEL) $(ALL_OBJECTS)
	$(LD) $(LDFLAGS) -T $(LINK_SCRIPT) -no-pie --oformat=elf64-x86-64 -o $(KERNEL_DEBUG) $(ALL_OBJECTS)

$(OBJECTS_C) $(OBJECTS_ASM) $(LOADEROBJECT): | $(OBJDIR)

$(OBJDIR): FORCE
	mkdir -p $(OBJDIR)/$(CDIR) $(OBJDIR)/$(ASMDIR)

$(OBJDIR)/%.o : %.c
	$(GCC) $(GCCFLAGS) -I./include -c $< -o $@

$(OBJDIR)/%.o : %.asm
	$(ASM) $(ASMFLAGS) $< -o $@

$(LOADEROBJECT):
	$(ASM) $(ASMFLAGS) $(LOADERSRC) -o $(LOADEROBJECT)

clean:
	rm -f $(OBJDIR)/*.o $(OBJDIR)/*/*.o *.bin *.elf

FORCE:

.PHONY: all clean
