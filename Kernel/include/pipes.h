#ifndef PIPES_H
#define PIPES_H

#include <memory.h>
#include <utils.h>
#include <scheduler.h>
#include <arrayUtils.h>

#define BUFFER_SIZE 4096

#define STDIN 0
#define STDOUT 1
#define STDERR 2

typedef struct {
  char buffer[BUFFER_SIZE];
  int32_t producingIndex;
  int32_t consumingIndex;
  int mutex;
  int empty;
  int full;
  bool deleted;
  // PCB* readerPcb;
  // PCB* writerPcb;
} Pipe;

//Setea todas las posiciones del array como no usadas e inicializa size en 0;
void initPipes();
//Crea un pipe 
//Permite crear un nuevo pipe (se necesita que se envíe un nombre ÚNICO, si no se le quieren adsignar procesos TODAVÍA
//se envía 0 al process que no tengo (si no le mando el pid), con el join se puede agregar
int64_t createPipe();
//permite escribir en un pipe, funciona como la función write (en lo posible)
int64_t writeToPipe(int pipe, const char* info, int size);
//permite leer de un pipe (igual que antes funciona como un read)
int64_t readFromPipe(int pipe, char* info, int size);
//permite eliminar un pipe para que vuelva a estar disponible
bool deletePipe(int pipe);

void writeStdin(char c);
int64_t readStdin(char* buf, int len);





#endif //PIPES_H