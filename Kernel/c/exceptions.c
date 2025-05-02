#include <exceptions.h>
#include <registers.h>
#include <videoDriver.h>
#include <timer.h>


static void printRegisters();

void zeroDivisionException(){
	clearScreen();
    printNextString("An exception occurred | ExceptionId: 0 - ZeroDivision Error\n\n");
    printRegisters();
}

void invalidOpcodeException(){
	clearScreen();
	printNextString("An exception occurred | ExceptionId: 6 - InvalidOpcode Error\n\n");
	printRegisters();
}

static void printRegisters(){
	Register * exceptionRegisters = getExceptionRegisters();
    for(int i = 0; i < REGISTER_QUANTITY; i++){
        printNextString(exceptionRegisters[i].name); printNextChar(' '); 
        printNextHexWithPadding(exceptionRegisters[i].value); printNextChar(' ');
        if(i % 3 == 0) // para que entren 3 registros por línea
            printNextChar('\n');
    }
    printNextChar('\n');
    printNextString("Returning to shell in 10 seconds\n");
    sleep(10000);
}