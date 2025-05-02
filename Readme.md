## Docker
docker pull agodio/itba-so:1.0

docker run -d -v ${PWD}:/root --security-opt seccomp:unconfined -ti --name arqui_builder agodio/itba-so:1.0

## Compile and run
./compile.sh

./run.sh (-d)

## gdb cmds
source tgdbinit

b initializeKernelBinary
b getKeyCode
b irq01Handler

n -> ir linea por linea

finish -> ejecuta hasta el final de la funcion

si -> instruccion por instruccion

# old readme ------

x64BareBones is a basic setup to develop operating systems for the Intel 64 bits architecture.

The final goal of the project is to provide an entry point for a kernel and the possibility to load extra binary modules separated from the main kernel.

Environment setup:
1- Install the following packages before building the Toolchain and Kernel:

nasm qemu gcc make

2- Build the Toolchain

Execute the following commands on the x64BareBones project directory:

  user@linux:$ cd Toolchain
  user@linux:$ make all

3- Build the Kernel

From the x64BareBones project directory run:

  user@linux:$ make all

4- Run the kernel

From the x64BareBones project directory run:

  user@linux:$ ./run.sh


Author: Rodrigo Rearden (RowDaBoat)
Collaborator: Augusto Nizzo McIntosh