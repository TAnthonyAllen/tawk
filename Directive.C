#include <string.h>
#include <stdio.h>
#include "PLGparse.h"
#include "Symbol.h"
#include "FormatC.h"
#include "SymbolType.h"
#include "DoubleLinkList.h"
#include "BaseHash.h"
#include "PLGitem.h"
#include "Statement.h"
#include "Tawk.h"
#include "Tok.h"
#include "Directive.h"

/*******************************************************************************
    Emit the directive statements into the output code stream.
*******************************************************************************/
void Directive::emitDirective()
{
PLGitem 	*item = 0;
Statement 	*statement = 0;
	for ( ; line; line = line->itemNext )
		{
		if ( !(item = (PLGitem*)line->children->get("statement")) )
			continue;
		if ( statement = (Statement*)item->itemValue )
			Tok::tawking->formatter->writeStatement(statement);
		}
	isDirected = 1;
	method->directives->next();
}

/*******************************************************************************
    Parse the codeToAdd into statements and add them to the block passed in.
*******************************************************************************/
void Directive::parseDirective()
{
PLGitem 	*item = 0;
	isDirected = 1;
	if ( item = Tok::tawking->divertInput(codeToAdd,"Directivise") )
		line = (PLGitem*)item->children->get("line");
	if ( !line )
		::printf("parseDirective: failed for %s %s\n",method->name,codeMatch);
}
