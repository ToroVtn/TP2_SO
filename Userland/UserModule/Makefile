include ../Makefile.inc

OBJDIR := objs
CDIR := c2
ASMDIR := asm

MODULE=0000-userModule.bin
MODULE_DEBUG=$(MODULE:.bin=.elf)
SOURCES_C := $(wildcard $(CDIR)/*.c)
OBJECTS_C := $(addprefix $(OBJDIR)/,$(SOURCES_C:%.c=%.o))
SOURCES_ASM := $(wildcard $(ASMDIR)/*.asm)
OBJECTS_ASM := $(addprefix $(OBJDIR)/,$(SOURCES_ASM:%.asm=%.o))

LOADERSRC := _loader.c
LOADEROBJECT := $(addprefix $(OBJDIR)/,$(LOADERSRC:.c=.o))

LINK_SCRIPT := linker.ld

ALL_OBJECTS := $(OBJECTS_C) $(OBJECTS_ASM)

all: $(MODULE)

$(MODULE): $(ALL_OBJECTS)
	$(GCC) $(GCCFLAGS) -T $(LINK_SCRIPT) $(LOADERSRC) $(ALL_OBJECTS) -o ../$(MODULE)
	$(GCC) $(GCCFLAGS) -T $(LINK_SCRIPT) -Wl,--oformat=elf64-x86-64,-no-pie $(LOADERSRC) $(ALL_OBJECTS) -o ../$(MODULE_DEBUG)

$(OBJECTS_C) $(OBJECTS_ASM): | $(OBJDIR)

$(OBJDIR): FORCE
	mkdir -p $(OBJDIR)/$(CDIR) $(OBJDIR)/$(ASMDIR)

$(OBJDIR)/%.o : %.c
	$(GCC) $(GCCFLAGS) -I./include -c $< -o $@

$(OBJDIR)/%.o : %.asm
	$(ASM) $(ASMFLAGS) $< -o $@

clean:
	rm -f $(OBJDIR)/*.o $(OBJDIR)/*/*.o *.bin *.elf

FORCE:

.PHONY: all clean
