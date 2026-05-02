#include <string.h>
#include <stdio.h>
#include "PLGparse.h"
#include "Symbol.h"
#include "SymbolType.h"
#include "DoubleLinkList.h"
#include "DoubleLink.h"
#include "Instance.h"
#include "Statement.h"
#include "Tawk.h"
#include "Tok.h"
#include "BlockTok.h"
int BlockTok::indentCount;

// Used in formatting declarations
BlockTok::BlockTok()
{
	blockMethod = 0;
	modified = 0;
	width = 0;
	hasBreak = 0;
	indenting = 0;
	isArrayInitializer = 0;
	isBlock = 0;
	isLambda = 0;
	isMethodBlock = 0;
	isSwitch = 0;
	statements = new DoubleLinkList();
}

BlockTok::BlockTok(int i)
{
	blockMethod = 0;
	modified = 0;
	width = 0;
	hasBreak = 0;
	indenting = 0;
	isArrayInitializer = 0;
	isBlock = 0;
	isLambda = 0;
	isMethodBlock = 0;
	isSwitch = 0;
	statements = new DoubleLinkList();
	if ( i == 1 )
		isBlock = 1;
	if ( i == 2 )
		isArrayInitializer = 1;
}

/*******************************************************************************
        add an instance (to the statement list)
*******************************************************************************/
void BlockTok::add(Instance *i)
{
Instance 	*instance = i;
	if ( !instance )
		return;
	if ( i->statement )
		i->statement->block = this;
	else {
		Statement 	*line = new Statement();
		line->add(i);
		line->block = this;
		instance = new Instance(line);
		}
	statements->add((void*)instance);
}

/*******************************************************************************
        add a comment (as a statement)
*******************************************************************************/
void BlockTok::add(char *s)
{
Instance 	*i = Tok::tawking->getInstance(s);
Statement 	*statement = new Statement();
	statement->block = this;
	statement->add(i);
	add(statement);
}

/*******************************************************************************
        add a statement
*******************************************************************************/
void BlockTok::add(Statement *s)
{
Instance 	*i = new Instance(s);
	s->block = this;
	add(i);
}

/*******************************************************************************
        Runs all statements thru a sanity check
*******************************************************************************/
void BlockTok::check()
{
Instance 	*instance = 0;
int 		i = 0;
	statements->resetIterator();
	while ( instance = (Instance*)statements->next() )
		{
		instance->check();
		i++;
		}
}

/*******************************************************************************
        get the width in tabs to indent field names being declared
*******************************************************************************/
int BlockTok::getWidth()
{
Instance 	*instance = 0;
SymbolType 	*type = 0;
int 		length = 0;
	if ( !width && statements->length )
		{
		while ( instance = (Instance*)statements->next() )
			{
			instance = instance->getSubject();
			if ( !instance->isDeclaration )
				break;
			type = instance->getType();
			length = (int)::strlen(type->name) + 1;
			if ( length > width )
				width = length;
			}
		width = width / 4 + 1;
		}
	return width;
}

/*******************************************************************************
        insert an instance at the beginning of the statement list
*******************************************************************************/
void BlockTok::insert(Instance *i)
{
	if ( i->statement )
		i->statement->block = this;
	else	i->error("Expected a statement");
	statements->insert((void*)i);
}

/*******************************************************************************
        Return the number of statements added to this block
*******************************************************************************/
int BlockTok::length()
{
	if ( statements )
		return statements->length;
	return 0;
}
