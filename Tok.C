#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "PLGparse.h"
#include "Symbol.h"
#include "Tape.h"
#include "PLGitem.h"
#include "Instance.h"
#include "Tawk.h"
#include "Tok.h"

/*****************************************************************************
	Rule FAIL handlers. These were free functions in Tawk.g's %% epilogue
	(which plg's regen does not carry). Moved here as file-scope externals;
	wired by name in setRules via the grammar's FAIL directives. The parser
	arrives as iTEM.testParser (was PLGtester in the old model).
*****************************************************************************/
void assignFailed(PLGitem *iTEM)
{
Tawk 	*tOK = Tok::testParser;
	if ( tOK->assigning )
		{
		tOK->assigning = 0;
		::printf("Assigning turned off\n");
		}
}

void caseLabelFail(PLGitem *iTEM)
{
Tawk 	*tOK = Tok::testParser;
	tOK->assuming = 0;
}

void expressPartFailed(PLGitem *iTEM)
{
Tawk 	*tOK = Tok::testParser;
	tOK->popVirtuals();
}

void instanceTailFail(PLGitem *iTEM)
{
Tawk 	*tOK = Tok::testParser;
	tOK->noShortcuts = 0;
	tOK->printErrorMessage();
}

/*****************************************************************************
	main program
*****************************************************************************/
int main(int argc, char **argv)
{
char 	*sourceFile = argv[1];
	Tok::tawking = new Tawk();
	Tok::testParser = Tok::tawking;
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
Tawk *Tok::testParser;

// set-and-forget parser handle for the FAIL handlers
void Tok::run()
{
	return;
}
/*	Warning: the following methods were referenced but not declared
	printErrorMessage()
*/
