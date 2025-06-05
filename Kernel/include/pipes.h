#ifndef PIPES_H
#define PIPES_H

#include <memory.h>
#include <utils.h>

#define BUFFER_SIZE 4096

typedef struct {
    char* name;
    char buffer[BUFFER_SIZE];
    int producingIndex;
    int consumingIndex;
    int mutex;
    int empty;
    int full;
    int read;
    int write;
} pipe;

typedef struct {
    int available;
    pipe* pipe;
} PipeSlot;

//Setea todas las posiciones del array como no usadas e inicializa size en 0;
void initPipes();
//Crea un pipe 
//Permite crear un nuevo pipe (se necesita que se envíe un nombre ÚNICO, si no se le quieren adsignar procesos TODAVÍA
//se envía 0 al process que no tengo (si no le mando el pid), con el join se puede agregar
int createPipe(char* name, int process_write, int process_read);

//Se asigna el proc read y/o write a un pipe, si no se envían procesos o ya tengo procesos asignados devuelve -1
int attachToPipe(const char* pipe_name, int process_write, int process_read);
//permite escribir en un pipe, funciona como la función write (en lo posible)
int writeToPipe(int pipe, const char* info, int size);
//permite leer de un pipe (igual que antes funciona como un read)
int readFromPipe(int pipe, char* info, int size);
//permite eliminar un pipe para que vuelva a estar disponible
int deletePipe(int pipe);





#endif //PIPES_H