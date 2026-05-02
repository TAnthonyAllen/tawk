#include <string.h>
#include <stdio.h>
#include "StringRoutines.h"
#include "PLGparse.h"
#include "Stak.h"
#include "SymbolType.h"
#include "Operate.h"
#include "DoubleLinkList.h"
#include "DoubleLink.h"
#include "PLGitem.h"
#include "BlockTok.h"
#include "Expression.h"
#include "Instance.h"
#include "Tawk.h"
#include "Tok.h"
#include "Statement.h"

Statement::Statement()
{
	pointInCode = 0;
	block = 0;
	first = 0;
	second = 0;
	third = 0;
	fourth = 0;
	branch = 0;
	debug = 0;
	noFallThru = 0;
	switching = 0;
	fallThruStack = 0;
	phiStack = 0;
	lvl = 0;
	statementType = NOTSPECIFIED;
	indented = 1;
}

Statement::Statement(sType type)
{
	pointInCode = 0;
	block = 0;
	first = 0;
	second = 0;
	third = 0;
	fourth = 0;
	branch = 0;
	debug = 0;
	noFallThru = 0;
	switching = 0;
	fallThruStack = 0;
	phiStack = 0;
	lvl = 0;
	statementType = type;
	indented = 1;
}

/*******************************************************************************
        Add a block to this statement
*******************************************************************************/
void Statement::add(BlockTok *b)
{
Instance 	*i = new Instance(b);
	indented = 0;
	add(i);
}

/*******************************************************************************
        Add a simple statements (break or continue)
*******************************************************************************/
void Statement::add(char *s)
{
Instance 	*i = Tok::tawking->getInstance(s);
	i->isComment = 0;
	add(i);
}

/*******************************************************************************
        Add a statement to this statement
*******************************************************************************/
void Statement::add(Statement *s)
{
Instance 	*i = new Instance(s);
	s->block = block;
	add(i);
}

/*******************************************************************************
        Add an instance to this statement
*******************************************************************************/
void Statement::add(Instance *i)
{
	if ( !first )
		first = i;
	else
	if ( !second )
		second = i;
	else
	if ( !third )
		third = i;
	else
	if ( !fourth )
		fourth = i;
}

/*******************************************************************************
        Add an expression to this statement
*******************************************************************************/
void Statement::add(Expression *e)
{
Instance 	*i = new Instance(e);
	add(i);
}

/*******************************************************************************
        Add breaks to a switch to avoid fall thrus (no fall thru is the default
        for a switch that has no break statements).
*******************************************************************************/
void Statement::addBreaks()
{
Instance 	*breakInstance = Tok::tawking->getInstance("break");
Instance 	*line = 0;
Instance 	*instance = 0;
BlockTok 	*switchBody = second->block;
int 		branched = 0;
int 		sawAction = 0;
Statement 	*breakLine = new Statement();
	// remove type so that break not treated as a string
	breakInstance->type = 0;
	// A statement line is an instance wrapping a statement
	breakLine->add(breakInstance);
	breakInstance = new Instance(breakLine);
	while ( instance = (Instance*)switchBody->statements->next() )
		{
		line = instance->statement->first;
		if ( !line || !line->isCase )
			sawAction = 1;
		else
		if ( sawAction )
			{
			if ( !branched )
				switchBody->statements->entry->insert(breakInstance);
			sawAction = 0;
			}
		branched = instance->statement->branch;
		}
}

/*******************************************************************************
        Runs all fields thru a sanity check
*******************************************************************************/
void Statement::check()
{
	if ( first )
		first->check();
	if ( second )
		second->check();
	if ( third )
		third->check();
	if ( fourth )
		fourth->check();
}

/*******************************************************************************
        Check if we need to decorate a block w/{}
*******************************************************************************/
void Statement::checkBlock(Instance *i)
{
BlockTok 	*block = 0;
	if ( i->block )
		block = i->block;
	if ( i->statement && i->statement->first && i->statement->first->block )
		block = i->statement->first->block;
	if ( block )
		if ( block->statements->length > 1 )
			block->isBlock = 1;
}

/*******************************************************************************
        Convert this switch to nested ifs
*******************************************************************************/
void Statement::convertSwitch()
{
Instance 	*switchExpress = 0;
Instance 	*ifExpress = 0;
Instance 	*current = 0;
Instance 	*falling = 0;
Instance 	*instance = 0;
BlockTok 	*switchBody = second->block;
BlockTok 	*defaultBody = 0;
BlockTok 	*ifBody = 0;
int 		broke = 0;
int 		firstTime = 1;
	if ( !fallThruStack )
		fallThruStack = new Stak();
	else	fallThruStack->clear();
	switchExpress = first;
	while ( instance = (Instance*)switchBody->statements->next() )
		if ( current = instance->statement->first )
			if ( current->isLabel )
				{
				if ( ifBody )
					{
					if ( firstTime )
						firstTime = 0;
					else {
						fallThruStack->entry = 0;
						while ( falling = (Instance*)fallThruStack->prior() )
							{
							ifBody->insert(falling);
							}
						fallThruStack->clear();
						}
					makeIfStatement(ifExpress,ifBody);
					ifExpress = 0;
					if ( broke )
						{
						broke = 0;
						fallThruStack->clear();
						}
					else {
						while ( falling = (Instance*)ifBody->statements->next() )
							fallThruStack->push(falling);
						}
					ifBody = 0;
					}
				if (::compare(current->prefix,"default") == 0)
					{
					ifExpress = 0;
					}
				else
				if ( switchExpress )
					ifExpress = makeIfExpression(instance->statement,switchExpress,ifExpress);
				else
				if ( !current->express && !current->isLabel )
					::fprintf(stderr,"Expected an expression %s\n",current->toString());
				else
				if ( ifExpress )
					{
					Expression 	*xp = new Expression(ifExpress,current,"||");
					ifExpress = new Instance(xp);
					}
				else	ifExpress = current;
				}
			else
			if ( ::compare(current->prefix,"break") == 0 )
				broke = 1;
			else {
addStatement:
				if ( !ifBody )
					{
					ifBody = new BlockTok();
					ifBody->indenting = 1;
					if ( !ifExpress )
						defaultBody = ifBody;
					}
				ifBody->add(instance);
				}
		else	goto addStatement;
	if ( ifBody )
		{
		fallThruStack->entry = 0;
		while ( falling = (Instance*)fallThruStack->prior() )
			{
			ifBody->insert(falling);
			}
		makeIfStatement(ifExpress,ifBody);
		}
	if ( defaultBody )
		{
		makeIfStatement(ifExpress = 0,defaultBody);
		}
	indented = 1;
}

/*******************************************************************************
        Get the block associated with this statement (if it has one)
*******************************************************************************/
BlockTok *Statement::getBlock()
{
	if ( block )
		return block;
	if ( first )
		return first->getBlock();
	return 0;
}

/*******************************************************************************
        Build the comparison part of an if statement
*******************************************************************************/
Instance *Statement::makeIfExpression(Statement *line, Instance *switcher, Instance *lastCase)
{
Expression 	*expression = 0;
Instance 	*thisCase = 0;
	line->first->isCase = 0;
	line->first->isLabel = 0;
	if ( line->first->postfix )
		{
		line->first->prefix = line->first->postfix;
		line->first->postfix = 0;
		}
	else	line->first->prefix = 0;
	// clean up unwanted parens around switch expression
	if ( first->express->hasParens && !first->express->verb )
		first->express->hasParens = 0;
	if ( line->first->isRange )
		thisCase = line->first;
	else {
		SymbolType 	*switcherType = switcher->getType();
		SymbolType 	*caseType = line->first->getType();
		if ( switcherType == SymbolType::stringType && caseType == SymbolType::charType )
			expression = new Expression(switcher,line->first,"==");
		else	expression = new Expression(switcher,line->first,"eq");
		thisCase = new Instance(expression);
		}
	if ( lastCase )
		{
		expression = new Expression(lastCase,thisCase,"||");
		lastCase = new Instance(expression);
		}
	else	lastCase = thisCase;
	return lastCase;
}

/*******************************************************************************
        Construct an if statement and append it to the old if statement
		passed in (used  when converting a switch to nested ifs)
*******************************************************************************/
void Statement::makeIfStatement(Instance *lastCase, BlockTok *body)
{
Instance 	*instance = 0;
Statement 	*current = 0;
Statement 	*newBody = 0;
Statement 	*newIf = 0;
	newBody = new Statement();
	newBody->add(body);
	newBody->indented = 1;
	if ( !lastCase )
		{
		current = this;
		while ( current->third )
			{
			instance = current->third;
			current = instance->statement;
			}
		current->add(newBody);
		return;
		}
	if ( statementType == SWITCH )
		{
		indented = 0;
		statementType = IF;
		first = lastCase;
		second = 0;
		add(newBody);
		return;
		}
	newIf = new Statement(IF);
	newIf->add(lastCase);
	newIf->add(newBody);
	if ( !third )
		add(newIf);
	else {
		current = this;
		while ( current->third )
			{
			instance = current->third;
			current = instance->statement;
			}
		current->add(newIf);
		}
}

/******************************************************************************
	Sets the isUsed flag
******************************************************************************/
void Statement::setIsUsed()
{
	if ( first )
		first->setIsUsed();
	if ( second )
		second->setIsUsed();
	if ( third )
		third->setIsUsed();
	if ( fourth )
		fourth->setIsUsed();
}

/*******************************************************************************
        Create a string representation of a statement type
*******************************************************************************/
char *Statement::toString(sType t)
{
char 	*text = 0;
	switch (t)
		{
		case CATCH:
			text = "CATCH";
			break;
		case DELETE:
			text = "DELETE";
			break;
		case DO:
			text = "DO";
			break;
		case FINAL:
			text = "FINAL";
			break;
		case FOR:
			text = "FOR";
			break;
		case IF:
			text = "IF";
			break;
		case LABEL:
			text = "LABEL";
			break;
		case LAMBDA:
			text = "LAMBDA";
			break;
		case RETURN:
			text = "RETURN";
			break;
		case SWITCH:
			text = "SWITCH";
			break;
		case WHILE:
			text = "WHILE";
			break;
		case GOTO:
			text = "GOTO";
			break;
		case THROW:
			text = "THROW";
			break;
		case TRY:
			text = "TRY";
			break;
		case NOTSPECIFIED:
			text = "NOTSPECIFIED";
		}
	return text;
}

/*******************************************************************************
        Create a string representation of this statement best we can
*******************************************************************************/
char *Statement::toString()
{
char 	*text = 0;
	if ( first )
		text = ::concat(3,toString(statementType),"\n\t\t",first->toString());
	if ( third )
		text = ::concat(3,text,second->toString(),third->toString());
	else
	if ( second )
		text = ::concat(2,text,second->toString());
	return text;
}
