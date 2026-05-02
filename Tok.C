#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "PLGparse.h"
#include "Symbol.h"
#include "Tape.h"
#include "Instance.h"
#include "Tawk.h"
#include "Tok.h"

/*****************************************************************************
	main program
*****************************************************************************/
int main(int argc, char **argv)
{
char 	*sourceFile = argv[1];
	Tok::tawking = new Tawk();
	if ( argc == 3 )
		Tok::tawking->directivesFile = argv[2];
	if ( sourceFile )
		{
		Tok::tawking->process(sourceFile);
		::printf("Parsed %s %d %d %d\n",sourceFile,Symbol::symbolCount,Instance::instanceCount,Tok::tawking->componentCount);
		//tawking.summary(5);
		}
	else {
		::fprintf(stderr,"No source file provided. Usage: tok sourceFile directivesFile(optional)\n");
		::exit(1);
		}
	::exit(0);
}
Tawk *Tok::tawking;
Tape *Tok::globalLinkTape;

void Tok::run()
{
	return;
}
