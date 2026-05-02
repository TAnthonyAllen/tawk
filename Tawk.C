#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "StringRoutines.h"
#include "Buffer.h"
#include "PLGparse.h"
#include "BaseHash.h"
#include "Types.h"
#include "Symbol.h"
#include "KeyTable.h"
#include "FormatC.h"
#include "Directive.h"
#include "Stak.h"
#include "SymbolType.h"
#include "Operate.h"
#include "PLGtester.h"
#include "DoubleLinkList.h"
#include "PLGrule.h"
#include "DoubleLink.h"
#include "PLGitem.h"
#include "PLGset.h"
#include "InstanceTable.h"
#include "KeyTableItem.h"
#include "BlockTok.h"
#include "Expression.h"
#include "Instance.h"
#include "Statement.h"
#include "Tawk.h"

int AliasItem2TawkNow(PLGitem *iTEM)
{
PLGitem 	*alias = iTEM->get("alias");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
	p->currentClass->constructor = alias->toString();
	return 1;
}

int AliasItem4TawkNow(PLGitem *iTEM)
{
PLGitem 	*alias = iTEM->get("alias");
PLGitem 	*value = iTEM->get("value");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
char 		*name = 0;
char 		*targetName = 0;
PLGitem 	*body = value->get("body");
PLGitem 	*indirect = value->get("indirect");
PLGitem 	*target = value->get("target");
PLGitem 	*valueType = value->get("type");
	if ( !p->currentClass )
		::fprintf(stderr,"ERROR processing aliases: current class is not set\n");
	else {
		name = alias->toString();
		if ( valueType )
			{
			SymbolType 	*typeAlias = (SymbolType*)SymbolType::types->get(name);
			/*********************************************************************
			Creates a type alias.
			*********************************************************************/
			if ( indirect )
				{
				if ( !typeAlias )
					{
					typeAlias = SymbolType::getType(value->toString());
					typeAlias->structure = 4;
					typeAlias->isDirect = 1;
					typeAlias->isAliasType = 1;
					typeAlias->isGlobal = 1;
					typeAlias->noDotH = 1;
					}
				else
				if ( typeAlias->isAliasType )
					typeAlias->name = value->toString();
				else	::fprintf(stderr,"AliasItem: cannot use a standard type as an alias %s\n",iTEM->toString());
				}
			else	typeAlias = (SymbolType*)valueType->value;
			SymbolType::types->put(name,typeAlias);
			}
		else {
			if ( body )
				{
				PLGitem 	*arguments = p->divertInput(body->string(),"AliasParameters");
				body->unString();
				body = arguments->get("body");
				}
			targetName = target->string();
			p->currentClass->makeAlias(name,targetName,body,p);
			target->unString();
			}
		}
	return 1;
}

int AliasItemTawkNow(PLGitem *iTEM)
{
PLGitem 	*name = iTEM->get("name");
PLGitem 	*target = iTEM->get("target");
Symbol 		*symbol = (Symbol*)name->value;
Symbol 		*method = (Symbol*)target->value;
	if ( method->type == symbol->type && (!method->parameters || (method->parentClass->isC && method->parameters->length == 1)) )
		{
		method->isGetter = 1;
		method->getter = symbol;
		symbol->getter = method;
		}
	else
	if ( method->parameters && (method->parameters->length == 1 || (method->parentClass->isC && method->parameters->length == 2)) )
		{
		method->isSetter = 1;
		method->setter = symbol;
		symbol->setter = method;
		}
	else	::fprintf(stderr,"Could not set getter or setter for %s\n",symbol->name);
	return 1;
}

int AllowShortcutsTawkNow(PLGitem *iTEM)
{
Tawk 	*p = (Tawk*)iTEM->test->testParser;
	if ( !p->assigning || p->noShortcuts )
		return 0;
	return 1;
}

int ArrayInitializerTawkNow(PLGitem *iTEM)
{
PLGitem 	*instance = iTEM->get("instance");
PLGitem 	*item = 0;
Instance 	*entry = 0;
BlockTok 	*block = new BlockTok(2);
	for ( item = instance; item; item = item->next )
		{
		entry = (Instance*)item->value;
		block->add(entry);
		}
	entry = new Instance(block);
	instance->value = (void*)entry;
	return 1;
}

int AssumedStringTawkNow(PLGitem *iTEM)
{
PLGitem 	*instance = iTEM->get("instance");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
PLGitem 	*lastSwitch = 0;
SymbolType 	*switchType = 0;
Instance 	*switcher = 0;
char 		*word = instance->string();
Instance 	*current = 0;
int 		switchDirect = 0;
	if ( p->ReservedWord->find(word) )
		{
		instance->unString();
		return 0;
		}
	instance->unString();
	if ( p->switchStack )
		if ( lastSwitch = (PLGitem*)p->switchStack->top() )
			{
			switcher = (Instance*)lastSwitch->value;
			switchType = switcher->getType();
			switchDirect = switcher->howDirect();
			if ( instance->itemLength == 1 )
				{
				if ( switchType == SymbolType::charType || (switchType == SymbolType::stringType && !switchDirect) )
					{
					word = ::concat(3,"'",instance->toString(),"'");
					current = new Instance(SymbolType::charType);
					}
				else
				if ( switchType == SymbolType::stringType && switchDirect == 1 )
					{
					word = instance->toString();
					current = new Instance(SymbolType::stringType);
					}
				current->prefix = word;
				goto bailAssumed;
				}
			}
	current = p->getInstance(instance->toString());
	current->indirection = 1;
	// constant strings are really pointers
	current->type = SymbolType::stringType;
bailAssumed:
	current->isConstant = 1;
	instance->value = (void*)current;
	return 1;
}

int AssumingTawkNow(PLGitem *iTEM)
{
Tawk 	*p = (Tawk*)iTEM->test->testParser;
	p->assuming = 1;
	return 1;
}

int BlockStartTawkNow(PLGitem *iTEM)
{
Tawk 		*p = (Tawk*)iTEM->test->testParser;
	if ( p->declaredSomething )
		p->declaredSomething = 0;
	p->currentSymbols->push("Block start");
	p->setCurrentType((SymbolType*)0);
	if ( p->declaringMethod && p->currentMethod )
		{
		if ( !p->currentBlock )
			p->currentMethod->pushParameters(p->currentSymbols);
		else
		if ( p->lambdaMethod )
			p->lambdaMethod->pushParameters(p->currentSymbols);
		}
	p->lambdaMethod = 0;
	if ( p->currentBlock )
		p->blockStack->push(p->currentBlock);
	p->currentBlock = new BlockTok(1);
	iTEM->value = (void*)p->currentBlock;
	p->currentBlock->blockMethod = p->currentMethod;
	// virtualStack can get out of whack when an expression fails
	// so it gets cleaned up here if needed
	p->resetVirtuals();
	return 1;
}

/*******************************************************************************
                Rule actions
            *******************************************************************************/
int BlockTawkNow(PLGitem *iTEM)
{
PLGitem 	*start = iTEM->get("start");
PLGitem 	*line = iTEM->get("line");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
Statement 	*statement = 0;
PLGitem 	*item = 0;
BlockTok 	*block = 0;
Directive 	*directive = 0;
	block = (BlockTok*)start->value;
	for ( ; line; line = line->next )
		{
		item = line->get("statement");
		if ( !item )
			continue;
		statement = (Statement*)item->value;
		if ( statement )
			block->add(statement);
		}
	if ( p->declaringMethod && p->currentMethod )
		{
		p->declaringMethod = 0;
		if ( p->currentMethod->directives )
			{
			p->noLoop = 1;
			p->currentMethod->directives->resetIterator();
			while ( directive = (Directive*)p->currentMethod->directives->next() )
				if ( !directive->codeMatch )
					directive->parseDirective();
			p->noLoop = 0;
			}
		}
	p->currentBlock = (BlockTok*)p->blockStack->pop();
	p->currentSymbols->pop("Block end");
	return 1;
}

int Body2TawkNow(PLGitem *iTEM)
{
PLGitem 	*body = iTEM->get("body");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
Instance 	*method = 0;
	body = body->get("method");
	method = (Instance*)body->value;
	if ( p->extending && method && method->symbol )
		method->symbol->extendType();
	return 1;
}

int Body3TawkNow(PLGitem *iTEM)
{
PLGitem 	*body = iTEM->get("body");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
PLGitem 	*declare = body->get("declare");
Instance 	*instance = 0;
Symbol 		*symbol = 0;
	if ( !p->currentClass )
		return 0;
	body->runDeferred();
	for ( ; declare; declare = declare->next )
		{
		instance = (Instance*)declare->value;
		if ( instance && instance->isError && instance->postfix )
			::fprintf(stderr,"%s",instance->postfix);
		if ( !instance || instance->statement || instance->isError )
			continue;
		if ( instance->type )
			continue;
		symbol = instance->getSymbol();
		//cout "Body: declaring " symbol.name:;
		if ( symbol->isMethod && !symbol->isLambda && !symbol->reference )
			{
			if ( p->extending && symbol->parameters )
				symbol->extendType();
			if ( !p->currentClass->getMethod(symbol->methodName) )
				p->currentClass->addMethod(symbol);
			p->currentMethod = 0;
			}
		else
		if ( !p->currentClass->getLocal(symbol->name) )
			p->currentClass->add(symbol);
		}
	return 1;
}

void ButtonArrayTawkAct(PLGitem *iTEM)
{
PLGitem 	*button = iTEM->get("button");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
Symbol 		*symbol = 0;
int 		i = 1;
char 		*text = 0;
	for ( ; button; button = button->next )
		{
		text = button->toString();
		symbol = p->currentClass->getLocal(text);
		if ( !symbol )
			{
			symbol = new Symbol(text,SymbolType::buttonType);
			symbol->isHidden = 1;
			symbol->isItem = 1;
			symbol->array = ::toStringFromInt(i++);
			p->currentClass->add(symbol);
			}
		else
		if ( symbol->type != SymbolType::buttonType )
			::fprintf(stderr,"Expected a button %s\n",text);
		button->value = (void*)symbol;
		}
}

int Case2TawkNow(PLGitem *iTEM)
{
PLGitem 	*instance = iTEM->get("instance");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
PLGitem 	*lastSwitch = 0;
SymbolType 	*labelType = 0;
SymbolType 	*switchType = 0;
Instance 	*switcher = 0;
Instance 	*label = (Instance*)instance->value;
int 		labelDirect = 0;
int 		switchDirect = 0;
	if ( p->switchStack )
		if ( lastSwitch = (PLGitem*)p->switchStack->top() )
			if ( switcher = (Instance*)lastSwitch->value )
				{
				switchType = switcher->getType();
				switchDirect = switcher->howDirect();
				labelDirect = label->howDirect();
				labelType = label->getType();
				if ( switchType != labelType )
					if ( switchType->isNumber && labelType->isNumber && labelDirect != switchDirect )
						label->error("switch/case type mismatch");
					else
					if ( switchType == SymbolType::stringType && labelType == SymbolType::charType && switchDirect != labelDirect )
						lastSwitch->flag4 = 1;
				}
	if ( !label->isLabel )
		{
		label->isLabel = 1;
		label->isCase = 1;
		label->prefix = "case ";
		}
	return 1;
}

int Case3TawkNow(PLGitem *iTEM)
{
PLGitem 	*instance = iTEM->get("instance");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
PLGitem 	*name = instance->get("name");
char 		*text = name->toString();
Instance 	*label = (Instance*)p->currentSymbols->instances->get(text);
Symbol 		*symbol = 0;
	if ( !label )
		{
		symbol = new Symbol(text,SymbolType::voidType);
		label = new Instance(symbol);
		label->isLabel = 1;
		p->currentSymbols->add(label);
		}
	if ( !label->isLabel )
		::fprintf(stderr,"ERROR: %s is not a label\n",name->toString());
	instance->value = (void*)label;
	return 1;
}

int CaseLabel2TawkNow(PLGitem *iTEM)
{
PLGitem 	*instance = iTEM->get("instance");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
PLGitem 	*item = 0;
Instance 	*field = 0;
Instance 	*rangeField = 0;
Instance 	*label = 0;
	rangeField = (Instance*)instance->value;
	if ( p->switchStack )
		if ( item = (PLGitem*)p->switchStack->top() )
			field = (Instance*)item->value;
	if ( field && rangeField )
		label = new Instance(p->convertRangeX(field,rangeField));
	label->isRange = 1;
	field->isRange = 1;
	// set to flag switch as needing conversion
	instance->value = (void*)label;
	return 1;
}

int CaseLabel3TawkNow(PLGitem *iTEM)
{
PLGitem 	*instance = iTEM->get("instance");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
PLGitem 	*field = instance->get("field");
Instance 	*label = (Instance*)field->value;
Symbol 		*symbol = label->getSymbol();
	if ( symbol->type == SymbolType::buttonType || (symbol->structType && isEnumerator(symbol->structType->structure)) )
		{
		label = p->getInstance("case ");
		if ( symbol->type == SymbolType::buttonType )
			label->postfix = symbol->array;
		else	label->postfix = symbol->name;
		label->type = SymbolType::intType;
		label->isConstant = 1;
		}
	else	label->prefix = "case ";
	label->isLabel = 1;
	label->isCase = 1;
	instance->value = (void*)label;
	p->assuming = 0;
	return 1;
}

int CaseLabel5TawkNow(PLGitem *iTEM)
{
PLGitem 	*instance = iTEM->get("instance");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
PLGitem 	*name = instance->get("name");
Instance 	*field = p->getInstance(name->toString());
	instance->value = (void*)field;
	return 1;
}

int CaseLabelTawkNow(PLGitem *iTEM)
{
PLGitem 	*instance = iTEM->get("instance");
Instance 	*label = (Instance*)instance->value;
	label->postfix = label->prefix;
	return 1;
}

int CaseTawkNow(PLGitem *iTEM)
{
PLGitem 	*instance = iTEM->get("instance");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
Instance 	*label = p->getInstance("default");
	label->isLabel = 1;
	label->isCase = 1;
	instance->value = (void*)label;
	return 1;
}

int CastExpressionTawkNow(PLGitem *iTEM)
{
PLGitem 	*direct = iTEM->get("direct");
PLGitem 	*type = iTEM->get("type");
PLGitem 	*rest = iTEM->get("rest");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
Instance 	*instance = (Instance*)type->value;
	if ( rest )
		{
		if ( !instance->isMethod )
			instance->error("Cast Expression method reference expected");
		instance->parameters = (DoubleLinkList*)rest->value;
		}
	instance->isCast = 1;
	if ( direct )
		instance->prefix = direct->string();
	p->setCurrentType((SymbolType*)0);
	if ( direct )
		type->value = (void*)instance->setIndirectItem(direct);
	return 1;
}

int CastTailTawkNow(PLGitem *iTEM)
{
PLGitem 		*rest = iTEM->get("rest");
DoubleLinkList 	*list = new DoubleLinkList();
Instance 		*instance = 0;
PLGitem 		*cast = 0;
PLGitem 		*item = 0;
	for ( item = rest; item; item = item->next )
		{
		cast = item->get("type");
		instance = (Instance*)cast->value;
		list->add((void*)instance);
		}
	rest->value = (void*)list;
	return 1;
}

int CastTypeTawkNow(PLGitem *iTEM)
{
PLGitem 	*type = iTEM->get("type");
PLGitem 	*direct = iTEM->get("direct");
PLGitem 	*array = iTEM->get("array");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
SymbolType 	*symbolType = (SymbolType*)type->value;
Instance 	*instance = new Instance(symbolType);
char 		*atDirect = 0;
	if ( direct )
		for ( atDirect = direct->itemStart + direct->itemLength - 1; atDirect >= direct->itemStart; atDirect-- )
			if ( *atDirect == '*' )
				instance->indirection++;
			else
			if ( *atDirect == '^' )
				p->currentClass->hasLambda = instance->isLambda = 1;
			else
			if ( *atDirect == '&' )
				instance->setReference((unsigned int)1);
	if ( !symbolType->isDirect && !instance->indirection )
		instance->indirection++;
	if ( instance->reference || instance->isLambda )
		instance->isMethod = 1;
	if ( array )
		instance->postfix = array->toString();
	type->value = (void*)instance;
	return 1;
}

int CharacterTawkNow(PLGitem *iTEM)
{
PLGitem 	*instance = iTEM->get("instance");
Instance 	*character = 0;
SymbolType 	*type = SymbolType::getType("char");
	character = new Instance(type);
	character->isConstant = 1;
	character->prefix = instance->toString();
	instance->value = (void*)character;
	return 1;
}

int CheckMacroParametersTawkNow(PLGitem *iTEM)
{
PLGitem 	*braced = iTEM->get("braced");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
PLGitem 	*body = braced->get("body");
	if ( body )
		{
		PLGitem 	*list = p->divertInput(body->string(),"MacroArgumentList");
		body->unString();
		braced->valueItem = list->get("argument");
		}
	return 1;
}

int CheckMacroTawkNow(PLGitem *iTEM)
{
PLGitem 	*statement = iTEM->get("statement");
PLGitem 	*braced = iTEM->get("braced");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
PLGrule 	*rule = 0;
PLGitem 	*item = 0;
PLGitem 	*macro = 0;
Symbol 		*argument = 0;
Symbol 		*symbol = 0;
int 		count = 0;
Buffer 		*buffer = p->tokJunkBuffer;
	if ( !p->macroList )
		return 0;
	symbol = (Symbol*)p->macroList->get(statement);
	if ( !symbol )
		return 0;
	if ( braced )
		{
		braced = item = braced->valueItem;
		for ( ; item; item = item->next )
			count++;
		if ( symbol->parameters )
			{
			if ( !count || count != symbol->parameters->length )
				return 0;
			}
		else
		if ( count )
			return 0;
		symbol->parameters->entry = 0;
		for ( item = braced; item; item = item->next )
			{
			argument = (Symbol*)symbol->parameters->next();
			argument->commentItem = item->get("part");
			}
		}
	buffer->reset();
	macro = symbol->commentItem;
	for ( ; macro; macro = macro->next )
		if ( argument = (Symbol*)macro->value )
			{
			if ( item = argument->commentItem )
				buffer->appendString(item->toString());
			else {
				buffer->appendString("No value supplied for ");
				buffer->appendString(argument->name);
				buffer->appendString("\n");
				}
			}
		else {
			buffer->appendString(macro->string());
			macro->unString();
			}
	//cout "CheckMacro action: " buffer:;
	/*************************************************************************
	Need to convert tokJunkBuffer to string because it might get reset in
	the middle of the diversion.
	*************************************************************************/
	if ( buffer->length() )
		{
		Statement 	*line = new Statement();
		line->pointInCode = iTEM;
		BlockTok 	*saveBlock = p->currentBlock;
		p->currentBlock = new BlockTok();
		rule = (PLGrule*)p->rules->get("MacroBlock");
		if ( item = p->divertInput(buffer->toString(),rule) )
			{
			Instance 	*instance = (Instance*)p->currentBlock->statements->first->value;
			line->add(p->currentBlock);
			statement->value = (void*)line;
			if ( instance && instance->statement )
				instance->statement->indented = 1;
			}
		p->currentBlock = saveBlock;
		}
	return 1;
}

void ClassAttributes2TawkAct(PLGitem *iTEM)
{
PLGitem 	*type = iTEM->get("type");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
SymbolType 	*extended = (SymbolType*)type->value;
	p->currentClass->setParent(extended);
}

void ClassAttributes3TawkAct(PLGitem *iTEM)
{
PLGitem 	*proto = iTEM->get("proto");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
PLGitem 	*type = 0;
SymbolType 	*pType = 0;
	for ( ; proto; proto = proto->next )
		if ( type = proto->get("type") )
			{
			pType = (SymbolType*)type->value;
			p->currentClass->addProtocol(pType);
			}
}

void ClassAttributes4TawkAct(PLGitem *iTEM)
{
PLGitem 	*nSpace = iTEM->get("nSpace");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
	p->currentClass->nameSpace = nSpace->toString();
}

void ClassAttributesTawkAct(PLGitem *iTEM)
{
PLGitem 		*trait = iTEM->get("trait");
Tawk 			*p = (Tawk*)iTEM->test->testParser;
KeyTableItem 	*item = (KeyTableItem*)trait->value;
Symbol 			*symbol = 0;
	switch (item->position)
		{
		case 1:
			// C
			if ( !p->currentClass->isC )
				{
				p->currentClass->isC = 1;
				if ( !p->currentClass->parent )
					{
					if ( p->currentClass->lastOffset > 0 )
						::fprintf(stderr,"ERROR: do not specify class as C after variables are declared\n");
					symbol = new Symbol("tHIS",p->currentClass);
					symbol->isVirtual = 1;
					symbol->isThis = 1;
					p->currentClass->add(symbol);
					symbol = symbol->makeAlias("this");
					symbol->isThis = 1;
					p->currentClass->add(symbol);
					p->currentClass->lastOffset = 0;
					}
				}
			break;
		case 2:
			// isChar
			p->currentClass->isChar = 1;
			break;
		case 3:
			// isNumber
			p->currentClass->isNumber = 1;
			break;
		case 4:
			// local
			p->currentClass->isLocal = 1;
			break;
		case 5:
			// no.h
			p->currentClass->noDotH = 1;
			break;
		case 6:
			// noClassForward
			p->currentClass->noClassForward = 1;
			break;
		case 7:
			// OC
			if ( !p->currentClass->isOC )
				{
				p->currentClass->isOC = 1;
				p->currentClass->isVirtuous = 1;
				}
			break;
		case 8:
			// proper
			p->currentClass->proper = 1;
			break;
		case 9:
			//protocol
			p->currentClass->structure = 3;
			p->currentClass->isOC = 1;
			break;
		case 10:
			// type
			p->currentClass->structure = 4;
			p->currentClass->isDirect = 1;
			break;
		case 11:
			// addClassNameToMethods
			p->currentClass->addClassNameToMethods = 1;
			break;
		}
}

int ClassBlockStartTawkNow(PLGitem *iTEM)
{
Tawk 	*p = (Tawk*)iTEM->test->testParser;
	if ( p->currentClass != SymbolType::globalType )
		p->currentSymbols->push(p->currentClass->name);
	if ( !p->currentClass->isExternal )
		p->declaredSomething = 0;
	return 1;
}

int ClassBlockTawkNow(PLGitem *iTEM)
{
Tawk 	*p = (Tawk*)iTEM->test->testParser;
	if ( p->currentClass != SymbolType::globalType )
		p->currentSymbols->pop("end class");
	p->currentClass->classOK = 1;
	p->declaredSomething = 1;
	//cout "Processed class block for " currentClass.name:;
	return 1;
}

int ClassHeading2TawkNow(PLGitem *iTEM)
{
PLGitem 	*externalRef = iTEM->get("externalRef");
PLGitem 	*structure = iTEM->get("structure");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
PLGitem 	*type = structure->get("body");
PLGitem 	*kind = structure->get("kind");
PLGitem 	*field = 0;
Instance 	*instance = 0;
	p->extending = 0;
	if ( kind->compare("typedef") == 0 || kind->compare("struct") == 0 )
		return 0;
	field = type->next;
	structure->runDeferred();
	instance = (Instance*)type->value;
	p->setCurrentClass(instance->getType());
	p->currentClass->noDotH = 1;
	p->setCurrentType((SymbolType*)0);
	if ( externalRef )
		p->currentClass->isExternal = 1;
	else	p->currentClass->isExternal = 0;
	for ( ; field; field = field->next )
		{
		instance = (Instance*)field->value;
		if ( instance )
			{
			instance->isDeclaration = 1;
			if ( instance->symbol )
				instance->symbol->parentClass = SymbolType::globalType;
			}
		}
	return 1;
}

int ClassHeading3TawkNow(PLGitem *iTEM)
{
PLGitem 	*kind = iTEM->get("kind");
PLGitem 	*nom = iTEM->get("nom");
PLGitem 	*attributes = iTEM->get("attributes");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
SymbolType 	*parent = 0;
char 		*className = 0;
char 		*dotHfile = 0;
PLGitem 	*entry = 0;
PLGitem 	*item = 0;
PLGitem 	*name = 0;
PLGitem 	*dotH = 0;
PLGitem 	*path = 0;
PLGitem 	*temp = 0;
PLGitem 	*type = 0;
Symbol 		*symbol = 0;
	p->currentBlock = 0;
	p->extending = 0;
	if ( !nom )
		{
		p->setCurrentClass(SymbolType::globalType);
		p->extending = 1;
		}
	else
	for ( entry = nom; entry; entry = entry->next )
		{
		name = entry->get("name");
		temp = entry->get("temp");
		path = (PLGitem*)name->value;
		dotH = entry->get("dotH");
		className = name->toString();
		if ( dotH )
			{
			dotHfile = path->toString();
			if ( !nom->next )
				{
				// rename the class since it is not really a class, just the name of a file
				className = ::concat(2,"notType",className);
				p->setCurrentClass(SymbolType::getType(className));
				p->currentClass->isGlobal = 1;
				p->currentClass->isExternal = 1;
				SymbolType::globalList->add((void*)p->currentClass);
				p->extending = 1;
				}
			else	continue;
			}
		else	p->setCurrentClass(SymbolType::getType(className));
		if ( temp )
			p->currentClass->isTemplate = 1;
		for ( item = attributes; item; item = item->next )
			item->runDeferred();
		if ( type )
			parent = (SymbolType*)type->value;
		p->currentClass->isExternal = 1;
		if ( parent )
			{
			p->currentClass->setParent(parent);
			if ( parent->isOC )
				p->currentClass->isOC = 1;
			}
		if ( !p->currentClass->noDotH && !p->currentClass->isAtomic && !p->currentClass->dotHname )
			p->currentClass->dotHname = path->toString();
		if ( p->currentClass->isOC )
			{
			p->nameSet->set((int)':');
			if ( p->currentClass->parent )
				{
				symbol = new Symbol("super",p->currentClass->parent);
				symbol->isVirtual = 1;
				p->currentClass->add(symbol);
				}
			}
		entry->value = (void*)p->currentClass;
		}
	if ( nom && nom->next && dotHfile )
		{
		for ( entry = nom; entry; entry = entry->next )
			{
			if ( !entry->value )
				continue;
			p->setCurrentClass((SymbolType*)entry->value);
			p->currentClass->dotHname = dotHfile;
			}
		p->setCurrentClass((SymbolType*)nom->value);
		}
	if ( kind )
		if ( *kind->itemStart == 't' )
			{
			p->currentClass->isDirect = 1;
			p->currentClass->structure = 4;
			}
		else {
			p->currentClass->isDirect = 1;
			p->currentClass->structure = 5;
			}
	return 1;
}

int ClassHeadingTawkNow(PLGitem *iTEM)
{
PLGitem 	*nom = iTEM->get("nom");
PLGitem 	*attributes = iTEM->get("attributes");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
SymbolType 	*parent = 0;
char 		*className = 0;
PLGitem 	*item = 0;
PLGitem 	*name = 0;
PLGitem 	*path = 0;
PLGitem 	*type = 0;
Symbol 		*symbol = 0;
	p->extending = 0;
	p->referring = 1;
	p->currentBlock = 0;
	name = nom->get("name");
	path = (PLGitem*)name->value;
	className = name->toString();
	p->setCurrentClass(SymbolType::getType(className));
	for ( item = attributes; item; item = item->next )
		{
		if ( !type )
			type = item->get("type");
		item->runDeferred();
		}
	if ( type )
		parent = (SymbolType*)type->value;
	if ( parent )
		{
		p->currentClass->setParent(parent);
		if ( parent->isOC )
			p->currentClass->isOC = 1;
		}
	if ( p->currentComment )
		p->currentClass->comment = p->extractComment();
	if ( p->currentClass->isOC )
		{
		p->nameSet->set((int)':');
		if ( p->currentClass->parent )
			{
			symbol = new Symbol("super",p->currentClass->parent);
			symbol->isVirtual = 1;
			p->currentClass->add(symbol);
			}
		}
	if ( !p->currentClass->noDotH && !p->currentClass->isAtomic && !p->currentClass->dotHname && !isProtocol(p->currentClass->structure) )
		p->currentClass->dotHname = path->toString();
	p->currentClass->isExternal = 0;
	if ( p->directivesFile )
		p->processDirectives();
	return 1;
}

int ClassNameTawkNow(PLGitem *iTEM)
{
PLGitem 	*path = iTEM->get("path");
PLGitem 	*name = iTEM->get("name");
PLGitem 	*temp = iTEM->get("temp");
PLGitem 	*dotH = iTEM->get("dotH");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
char 		*text = 0;
	if ( temp )
		name->itemLength += temp->itemLength;
	if ( name->compare("extends") == 0 || name->compare("external") == 0 )
		{
		name->unString();
		return 0;
		}
	else	name->unString();
	if ( path )
		{
		path->itemLength += name->itemLength;
		if ( dotH )
			path->itemLength += dotH->itemLength;
		}
	else
	if ( dotH )
		{
		path = p->plgItemFactory(name);
		path->itemLength += dotH->itemLength;
		}
	else {
		text = ::concat(2,name->string(),".h");
		path = p->plgItemFactory(text);
		name->unString();
		}
	name->value = (void*)path;
	return 1;
}

int CommentTawkNow(PLGitem *iTEM)
{
PLGitem 	*comment = iTEM->get("comment");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
	p->currentComment = comment;
	return 1;
}

int ConditionLabelTawkNow(PLGitem *iTEM)
{
PLGitem 	*label = iTEM->get("label");
PLGitem 	*text = iTEM->get("text");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
	p->Conditions->add(label->string(),(void*)text->toString());
	return 1;
}

void ConditionWordTawkAct(PLGitem *iTEM)
{
PLGitem 		*list = iTEM->get("list");
Tawk 			*p = (Tawk*)iTEM->test->testParser;
PLGitem 		*item = 0;
PLGitem 		*text = 0;
KeyTableItem 	*conditionItem = 0;
Instance 		*condition = 0;
char 			*conditionText = 0;
	if ( conditionItem = (KeyTableItem*)list->value )
		if ( conditionText = (char*)conditionItem->value )
			if ( item = p->divertInput(conditionText,"Expression") )
				{
				text = item->get("instance");
				if ( text )
					list->value = text->value;
				else {
					condition = new Instance();
					condition->error("Failed parse of condition list");
					list->value = (void*)condition;
					}
				return;
				}
	::fprintf(stderr,"Could not parse Condition: %s\n",list->toString());
}

int Constant4TawkNow(PLGitem *iTEM)
{
PLGitem 	*instance = iTEM->get("instance");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
Instance 	*current = new Instance(p->trueSymbol);
	instance->value = (void*)current;
	current->isConstant = 1;
	return 1;
}

int Constant5TawkNow(PLGitem *iTEM)
{
PLGitem 	*instance = iTEM->get("instance");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
Instance 	*current = new Instance(p->falseSymbol);
	instance->value = (void*)current;
	current->isConstant = 1;
	return 1;
}

void DebugDirectiveTawkAct(PLGitem *iTEM)
{
PLGitem 	*method = iTEM->get("method");
PLGitem 	*body = iTEM->get("body");
PLGitem 	*locate = iTEM->get("locate");
PLGitem 	*active = iTEM->get("active");
PLGitem 	*code = iTEM->get("code");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
	/***************************************************************************
	The directive does not get added unless the matching method is found
	and the directive is marked as active
	***************************************************************************/
	if ( active )
		{
		char 	*text = method->string();
		if ( !body && (!locate || (*locate->itemStart != 'e' && *locate->itemStart != 's')) )
			::fprintf(stderr,"Directive for %s missing location\n",text);
		else {
			Symbol 	*directiveMethod = p->currentType->getMethod(text);
			if ( !directiveMethod && p->currentType->isGlobal )
				directiveMethod = p->currentSymbols->findGlobalMethod(text);
			if ( directiveMethod )
				{
				Directive 	*directive = new Directive();
				directive->type = directiveMethod->parentClass;
				directive->method = directiveMethod;
				if ( body )
					directive->codeMatch = body->toString();
				directive->codeToAdd = code->toString();
				if ( locate )
					{
					switch (*locate->itemStart)
						{
						case 'b':
							directive->comesBefore = 1;
							break;
						case 'e':
							directive->atEnd = 1;
							break;
						case 's':
							directive->atStart = 1;
							break;
						case 'w':
							directive->within = 1;
						}
					}
				if ( !directiveMethod->directives )
					directiveMethod->directives = new DoubleLinkList();
				directiveMethod->directives->add(directive);
				}
			else	::fprintf(stderr,"Could not find directive method: %s in type: %s\n",text,p->currentType->name);
			}
		method->unString();
		}
}

int DebugRuleTawkNow(PLGitem *iTEM)
{
PLGitem 	*name = iTEM->get("name");
PLGitem 	*upcoming = iTEM->get("upcoming");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
PLGrule 	*rule = 0;
char 		*text = name->string();
	rule = (PLGrule*)p->rules->get(text);
	if ( !rule )
		::fprintf(stderr,"Rule not found: %s\n",text);
	else {
		rule->debug = 1;
		if ( upcoming )
			{
			PLGitem 	*body = upcoming->get("body");
			if ( body )
				rule->debugText = body->toString();
			}
		}
	name->unString();
	return 1;
}

int DebugTextTawkNow(PLGitem *iTEM)
{
PLGitem 	*upcoming = iTEM->get("upcoming");
	::printf("DebugText got here %s\n",upcoming->toString());
	return 1;
}

void Declaration2TawkAct(PLGitem *iTEM)
{
PLGitem 	*declare = iTEM->get("declare");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
PLGitem 	*field = declare->get("body");
PLGitem 	*kind = declare->get("kind");
SymbolType 	*structureType = 0;
Instance 	*instance = 0;
	declare->runDeferred();
	if ( kind )
		structureType = (SymbolType*)kind->value;
	if ( !p->currentClass->isExternal && structureType && isType(structureType->structure) )
		::fprintf(stderr,"WARNING typedef ignored: must be declared external\n");
	declare->next = field;
	for ( ; field; field = field->next )
		{
		instance = (Instance*)field->value;
		if ( instance )
			instance->isDeclaration = 1;
		}
}

void DeclarationTawkAct(PLGitem *iTEM)
{
PLGitem 	*outlet = iTEM->get("outlet");
PLGitem 	*modify = iTEM->get("modify");
PLGitem 	*type = iTEM->get("type");
PLGitem 	*declare = iTEM->get("declare");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
Instance 	*instance = 0;
Symbol 		*symbol = 0;
PLGitem 	*entry = 0;
PLGitem 	*item = 0;
	p->setCurrentType((SymbolType*)type->value);
	p->newType = p->methodType = p->currentType;
	for ( item = modify; item; item = item->next )
		if ( *item->itemStart == 'c' )
			p->constDeclare = 1;
		else
		if ( *item->itemStart == 'i' )
			p->linkage = 2;
		else
		if ( *item->itemStart == 's' )
			p->linkage = 3;
		else
		if ( *item->itemStart == 'v' )
			p->linkage = 4;
		else	p->linkage = 1;
	for ( entry = declare; entry; entry = entry->next )
		{
		entry->runDeferred();
		item = entry->get("item");
		instance = (Instance*)item->value;
		instance->isDeclaration = 1;
		entry->value = (void*)instance;
		symbol = instance->getSymbol();
		if ( outlet )
			symbol->isOutlet = 1;
		if ( staticDeclare(p->linkage) )
			{
			symbol->isStatic = 1;
			if ( instance->express )
				{
				Statement 	*statement = new Statement();
				statement->pointInCode = iTEM;
				statement->add(instance);
				p->formatter->staticBlock->add(statement);
				symbol->isInitialized = 1;
				}
			if ( symbol->isMethod )
				p->currentSymbols->addGlobalField(symbol->methodName,symbol);
			p->currentSymbols->addGlobalField(symbol->name,symbol);
			}
		if ( p->constDeclare )
			symbol->isConst = 1;
		if ( inlineDeclare(p->linkage) )
			symbol->isInline = 1;
		if ( staticDeclare(p->linkage) )
			symbol->isStatic = 1;
		if ( virtualDeclare(p->linkage) )
			symbol->isVirtual = 1;
		if ( externDeclare(p->linkage) )
			{
			symbol->isExtern = 1;
			p->currentClass->hasExtern = 1;
			}
		}
	p->setCurrentType((SymbolType*)0);
	if ( modify )
		{
		p->constDeclare = 0;
		p->linkage = 0;
		}
}

void DeclareItem2TawkAct(PLGitem *iTEM)
{
PLGitem 	*item = iTEM->get("item");
PLGitem 	*argument = iTEM->get("argument");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
char 		*text = 0;
Symbol 		*symbol = 0;
PLGitem 	*name = item->get("name");
PLGitem 	*parameter = argument->get("instance");
Instance 	*instance = p->getInstance(p->currentType->name);
Instance 	*part = (Instance*)parameter->value;
	/**********************************************************************
	converting constructor
	**********************************************************************/
	instance->addParameter(part);
	text = instance->mangle();
	if ( symbol = p->currentType->getMethod(text) )
		{
		symbol = new Symbol(name->toString(),p->currentType);
		symbol->indirect = 0;
		symbol->isConstructor = 1;
		instance->symbol = symbol;
		instance->prefix = 0;
		instance->isMethod = 1;
		}
	else	instance->error("Could not find converting constructor");
	item->value = (void*)instance;
}

void DeclareItem3TawkAct(PLGitem *iTEM)
{
PLGitem 	*item = iTEM->get("item");
PLGitem 	*assign = iTEM->get("assign");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
PLGitem 	*direct = item->get("direct");
PLGitem 	*name = item->get("name");
PLGitem 	*array = item->get("array");
PLGitem 	*bits = item->get("bits");
Symbol 		*symbol = 0;
Instance 	*instance = 0;
Instance 	*assigned = 0;
Expression 	*express = 0;
char 		*text = name->toString();
	if ( !p->currentBlock )
		symbol = p->currentClass->getLocal(text);
	if ( p->currentBlock || !symbol )
		{
		symbol = new Symbol(text,p->currentType);
		if ( direct )
			symbol->setIndirection(direct);
		if ( array )
			{
			symbol->indirect += (long)array->value;
			symbol->isArray = (unsigned int)array->amount;
			p->tokJunkBuffer->reset();
			for ( ; array; array = array->next )
				array->copyTo(p->tokJunkBuffer);
			symbol->array = p->tokJunkBuffer->toString();
			}
		else
		if ( bits )
			symbol->array = bits->toString();
		}
	instance = new Instance(symbol);
	instance->isDeclaration = 1;
	instance->block = p->currentBlock;
	if ( !direct && !p->currentType->isDirect )
		symbol->indirect = 1;
	if ( assign )
		{
		PLGitem 	*assignItem = assign->get("instance");
		assigned = (Instance*)assignItem->value;
		express = new Expression(instance,assigned,"=");
		instance = new Instance(express);
		instance->isRange = assigned->isRange;
		}
	item->value = (void*)instance;
}

void DeclareItemTawkAct(PLGitem *iTEM)
{
PLGitem 	*item = iTEM->get("item");
PLGitem 	*instance = iTEM->get("instance");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
PLGitem 	*parameters = item->get("head");
Instance 	*methodD = 0;
Instance 	*methodI = 0;
Expression 	*xpress = 0;
	item->runDeferred();
	methodD = (Instance*)parameters->value;
	methodD->isDeclaration = 1;
	if ( methodD->symbol )
		{
		if ( !p->currentBlock )
			methodD->symbol->parentClass = p->currentClass;
		if ( p->currentMethod && p->currentMethod == methodD->symbol )
			p->currentMethod = 0;
		}
	if ( instance )
		{
		methodI = (Instance*)instance->value;
		xpress = new Expression(methodD,methodI,"=");
		methodD = new Instance(xpress);
		}
	item->value = (void*)methodD;
}

int DeclareTypeTawkNow(PLGitem *iTEM)
{
PLGitem 	*type = iTEM->get("type");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
	p->setCurrentType((SymbolType*)type->value);
	p->newType = p->methodType = p->currentType;
	return 1;
}

int DirectiveTawkNow(PLGitem *iTEM)
{
PLGitem 	*type = iTEM->get("type");
PLGitem 	*directives = iTEM->get("directives");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
	p->setCurrentType((SymbolType*)type->value);
	for ( ; directives; directives = directives->next )
		if ( p->currentType == p->currentClass || p->currentType == SymbolType::globalType )
			directives->runDeferred();
	return 1;
}

int Else2TawkNow(PLGitem *iTEM)
{
PLGitem 	*statement = iTEM->get("statement");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
PLGitem 	*action = statement->get("action");
PLGitem 	*instance = statement->get("instance");
PLGitem 	*item = action->get("statement");
PLGitem 	*otherwise = statement->get("otherwise");
Instance 	*express = (Instance*)instance->value;
Statement 	*line = (Statement*)item->value;
Statement 	*ifStatement = new Statement(IF);
	ifStatement->add(express);
	ifStatement->pointInCode = iTEM;
	if ( p->currentMethod && p->currentMethod->directives && !p->noLoop )
		{
		Directive 	*directive = 0;
		p->currentMethod->directives->resetIterator();
		while ( directive = (Directive*)p->currentMethod->directives->next() )
			if ( directive->isDirected || !directive->codeMatch )
				continue;
			else
			if ( !::strncmp(directive->codeMatch,ifStatement->pointInCode->itemStart,::strlen(directive->codeMatch)) )
				{
				p->noLoop = 1;
				directive->parseDirective();
				p->noLoop = 0;
				break;
				}
		}
	line->indented = 1;
	if ( line )
		ifStatement->add(line);
	if ( otherwise )
		{
		item = otherwise->get("statement");
		line = (Statement*)item->value;
		if ( line )
			ifStatement->add(line);
		}
	statement->value = (void*)ifStatement;
	return 1;
}

int ExpressListTawkNow(PLGitem *iTEM)
{
PLGitem 	*list = iTEM->get("list");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
Instance 	*left = 0;
Instance 	*right = 0;
Expression 	*express = 0;
PLGitem 	*item = 0;
PLGitem 	*part = 0;
	for ( item = list; item; item = item->next )
		{
		part = item->get("instance");
		right = (Instance*)part->value;
		if ( left )
			{
			express = new Expression(left,right,",");
			left = new Instance(express);
			}
		else	left = right;
		}
	list->value = (void*)left;
	p->noShortcuts = 0;
	return 1;
}

int ExpressPartTawkNow(PLGitem *iTEM)
{
PLGitem 	*unaryOp = iTEM->get("unaryOp");
PLGitem 	*instance = iTEM->get("instance");
PLGitem 	*express = iTEM->get("express");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
Expression 	*expression = 0;
Instance 	*secondary = 0;
Instance 	*primary = 0;
PLGitem 	*item = 0;
PLGitem 	*operand = 0;
PLGitem 	*operate = 0;
PLGitem 	*stringPart = 0;
SymbolType 	*type = 0;
SymbolType 	*secondaryType = 0;
	primary = (Instance*)instance->value;
	type = primary->getType();
	p->expressType = 0;
	if ( express )
		{
		if ( operate = express->get("operate") )
			{
			operand = operate->get("operand");
			if ( !operand )
				operand = operate->get("comparator");
			}
		item = express->get("instance");
		if ( item )
			{
			secondary = (Instance*)item->value;
			if ( secondary->isRange )
				{
				expression = p->convertRangeX(primary,secondary);
				primary = new Instance(expression);
				}
			else {
				secondaryType = secondary->getType();
				if ( !operand && secondary && secondary->express && secondary->express->verb->compare(":") == 0 )
					{
					expression = new Expression(primary,secondary,"?");
					primary = new Instance(expression);
					}
				else
				if ( operand && operand->compare("+=") == 0 && type == SymbolType::stringType && secondaryType == SymbolType::stringType && primary->howDirect() == 1 )
					{
					operand->setString("=");
					//plgStart = item.start;
					p->stringing = 1;
					if ( stringPart = p->parse("Strings") )
						{
						item->next = stringPart;
						instance->next = item;
						}
					else	instance->next = item;
					secondary = p->concatenate(instance);
					expression = new Expression(primary,secondary,"=");
					primary = new Instance(expression);
					}
				}
			}
		}
	if ( !expression && (primary->isVirtuous() || operand) )
		{
		expression = p->makeExpress(primary,express);
		primary = new Instance(expression);
		}
	if ( unaryOp )
		{
		if ( unaryOp->compare("&") == 0 && !unaryOp->next )
			primary->setReference((unsigned int)1);
		else {
			expression = new Expression((Instance*)0,primary,unaryOp->toString());
			primary = new Instance(expression);
			}
		}
	primary = primary->checkOverload();
	if ( primary->symbol && primary->symbol->isDefault )
		primary->setDefaults(p);
	instance->value = (void*)primary;
	p->setCurrentType((SymbolType*)0);
	p->popVirtuals();
	return 1;
}

int ExpressTypeTawkNow(PLGitem *iTEM)
{
Tawk 	*p = (Tawk*)iTEM->test->testParser;
	p->expressType = p->currentType;
	return 1;
}

int ExpressionTawkNow(PLGitem *iTEM)
{
PLGitem 	*instance = iTEM->get("instance");
PLGitem 	*express = iTEM->get("express");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
Expression 	*expression = 0;
Instance 	*primary = 0;
	primary = (Instance*)instance->value;
	if ( express )
		{
		expression = p->makeExpress(primary,express);
		primary = new Instance(expression);
		}
	if ( !primary->express )
		{
		expression = new Expression();
		expression->subject = primary;
		primary = new Instance(expression);
		}
	instance->value = (void*)primary;
	p->setCurrentType((SymbolType*)0);
	return 1;
}

int ExtenderTawkNow(PLGitem *iTEM)
{
PLGitem 	*name = iTEM->get("name");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
Symbol 		*method = 0;
char 		*text = name->toString();
	method = p->currentClass->findField(text);
	if ( method )
		method->extendType();
	else	::fprintf(stderr,"Overload could not find extender method: %s\n",iTEM->toString());
	name->unString();
	return 1;
}

int ExtendsTawkNow(PLGitem *iTEM)
{
Tawk 	*p = (Tawk*)iTEM->test->testParser;
	if ( p->currentClass->isOC )
		p->nameSet->reset((int)':');
	SymbolType::types->resetIsFlagged();
	p->formatter->declareClass(p->currentClass);
	p->setCurrentClass(SymbolType::globalType);
	return 1;
}

void FieldBody2TawkAct(PLGitem *iTEM)
{
PLGitem 	*name = iTEM->get("name");
PLGitem 	*item = 0;
	name->runDeferred();
	item = name->get("instance");
	name->value = item->value;
}

int FieldBody3TawkNow(PLGitem *iTEM)
{
PLGitem 	*name = iTEM->get("name");
PLGitem 	*item = name->get("instance");
Instance 	*current = (Instance*)item->value;
Expression 	*express = 0;
	if ( !current->express || current->cast )
		{
		express = new Expression(current,(Instance*)0,(char*)0);
		current = new Instance(express);
		}
	current->express->hasParens = 1;
	name->value = (void*)current;
	return 1;
}

void FieldBodyTawkAct(PLGitem *iTEM)
{
PLGitem 	*prefix = iTEM->get("prefix");
PLGitem 	*part = iTEM->get("part");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
PLGitem 	*item = 0;
PLGitem 	*name = part->get("name");
PLGitem 	*body = part->get("body");
char 		*text = name->toString();
Symbol 		*symbol = 0;
double 		isArray = 0;
Instance 	*current = p->getInstance(text);
Instance 	*parameter = 0;
Instance 	*instance = 0;
Expression 	*expression = 0;
	if ( body )
		{
		item = body->get("array");
		if ( item )
			isArray = item->amount;
		else {
			item = body->get("expression");
			current->isMethod = 1;
			}
		for ( ; item; item = item->next )
			{
			parameter = (Instance*)item->get("instance")->value;
			current->addParameter(parameter);
			}
		if ( !instance && current->isMethod )
			if ( p->currentType )
				instance = p->currentType->findMethodInstance(current);
			else	instance = p->currentSymbols->findMethod(current);
		}
	if ( !instance )
		{
		if ( p->isQualified )
			{
			SymbolType::types->resetIsFlagged();
			instance = p->currentType->findFieldInstance(current->prefix);
			if ( !instance && p->currentType->isOC )
				symbol = (Symbol*)SymbolType::ocSymbols->get(text);
			}
		else
		if ( instance = p->currentSymbols->find(current->prefix) )
			if ( instance->isMethod && !body )
				instance->noBody = 1;
			else
			if ( current->isMethod )
				instance->isMethod = 1;
		if ( symbol )
			{
			current->symbol = symbol;
			current->isMethod = symbol->isMethod;
			current->prefix = 0;
			instance = current;
			}
		}
	if ( *text == '@' )
		{
		instance = current;
		instance->prefix++;
		instance->type = SymbolType::selectorType;
		instance->isSelector = 1;
		instance->isConstant = 1;
		}
	else
	if ( instance )
		{
		if ( instance->isLambda && body && !instance->symbol->isAssigned )
			{
			if ( p->isQualified )
				current = new Instance(instance);
			else {
				delete current;
				current = instance;
				}
			current->error("lambda not assigned");
			}
		else
		if ( !isArray && instance != current && !instance->reference && !current->parameters )
			{
			if ( p->isQualified )
				current = new Instance(instance);
			else {
				delete current;
				current = instance;
				}
			current->parameters = 0;
			current->arrayRef = 0;
			}
		else {
			if ( instance != current )
				{
				if ( instance->isCopy )
					{
					if ( current->isMethod || instance->reference )
						{
						// The following needed when () is overloaded
						if ( current->isMethod && !instance->isMethod )
							instance->isMethod = 1;
						}
					instance->parameters = current->parameters;
					delete current;
					current = instance;
					}
				else {
					current->symbol = instance->symbol;
					current->setParent(instance->parent);
					current->isMethod = instance->isMethod;
					current->prefix = 0;
					current->type = 0;
					current->isConstant = 0;
					current->indirection = 0;
					current->setReference((unsigned int)0);
					current->isDeclaration = 0;
					current->checked = 0;
					current->resolved = 0;
					}
				}
			current->arrayRef = (unsigned int)isArray;
			}
		}
	else
	if ( current->isMethod )
		{
		if ( instance = (Instance*)p->missingMethods->get(current->prefix) )
			{
			current->symbol = instance->symbol;
			current->setParent(instance->parent);
			current->prefix = 0;
			}
		else {
			current->prefix = current->mangle();
			p->missingMethods->put(current->prefix,current);
			current->symbol = new Symbol(text,SymbolType::nullType);
			current->symbol->isMethod = 1;
			current->symbol->methodName = current->prefix;
			current->prefix = 0;
			}
		}
	else
	if ( current->type == SymbolType::stringType && !current->isMethod && ((p->currentClass && p->currentClass->isVirtuous) || (p->currentType && p->currentType->isVirtuous) || p->virtualOp || p->assuming) )
		{
		/***********************************************************************
		We bail here so Qualified will fail because name value does not get
		set, letting AssumedString convert field to a string constant
		***********************************************************************/
		return;
		}
	else
	if ( !current->isVirtuous() )
		if ( *p->plgStart != ':' )
			{
			text = ::concat(2,"FieldBody: could not find ",current->prefix);
			current = p->makeError(text);
			}
		else	current->type = SymbolType::nullType;
	if ( current->isMethod && !current->parameters && !body )
		current->setReference((unsigned int)1);
	if ( prefix )
		{
		expression = new Expression((Instance*)0,current,prefix->string());
		prefix->unString();
		}
	if ( expression )
		current = new Instance(expression);
	name->value = (void*)current;
}

int FieldExpressionTawkNow(PLGitem *iTEM)
{
PLGitem 	*cast = iTEM->get("cast");
PLGitem 	*direct = iTEM->get("direct");
PLGitem 	*instance = iTEM->get("instance");
Instance 	*castInstance = 0;
Instance 	*subject = 0;
PLGitem 	*item = instance->get("field");
Instance 	*current = (Instance*)item->value;
	if ( direct )
		current = current->setIndirectItem(direct);
	if ( cast )
		{
		direct = cast->get("direct");
		item = cast->get("type");
		castInstance = (Instance*)item->value;
		if ( direct )
			castInstance = castInstance->setIndirectItem(direct);
		subject = new Instance(current);
		subject->cast = castInstance;
		current = subject;
		}
	instance->value = (void*)current;
	return 1;
}

int FieldTawkNow(PLGitem *iTEM)
{
PLGitem 	*name = iTEM->get("name");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
Symbol 		*symbol = 0;
	if ( p->currentClass->components )
		{
		symbol = (Symbol*)p->currentClass->components->get(name->string());
		name->unString();
		}
	if ( symbol )
		name->value = (void*)symbol;
	else	return 0;
	return 1;
}

int FieldingTawkNow(PLGitem *iTEM)
{
PLGitem 	*name = iTEM->get("name");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
char 		*text = name->toString();
Symbol 		*symbol = p->currentSymbols->presentClass->findField(text);
	if ( symbol && !p->currentType )
		p->setCurrentType(symbol->type);
	return 1;
}

int FileNameTawkNow(PLGitem *iTEM)
{
PLGitem 	*name = iTEM->get("name");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
	p->formatter->filename = name->toString();
	return 1;
}

int ForOption2TawkNow(PLGitem *iTEM)
{
PLGitem 	*instance = iTEM->get("instance");
PLGitem 	*name = iTEM->get("name");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
Instance 	*initial = 0;
Instance 	*target = 0;
Instance 	*iterator = 0;
SymbolType 	*type = 0;
SymbolType 	*objectType = 0;
Symbol 		*symbol = 0;
Symbol 		*objectIterator = 0;
char 		*error = 0;
Statement 	*statement = new Statement(FOR);
Expression 	*express = 0;
	statement->pointInCode = iTEM;
	initial = (Instance*)instance->value;
	target = initial->express->subject;
	if ( !target )
		target = p->makeError("expression has no subject");
	else
	if ( !target->symbol )
		target = p->makeError("invalid subject");
	else {
		type = target->getType();
		if ( name )
			{
			symbol = type->getLocal(name->string());
			name->unString();
			}
		else
		if ( initial->howDirect() == 2 )
			{
			/***************************************************************
			Build double indirection loop
			***************************************************************/
			target = new Instance(target);
			iterator = new Instance(target);
			express = new Expression(iterator,target,"&&");
			iterator = new Instance(target);
			target->indirection = 1;
			iterator->postfix = "++";
			target = new Instance(express);
			goto endForOption;
			}
		else	symbol = type->getLocal("next");
		if ( !symbol )
			{
			if ( initial->express->object )
				if ( objectType = initial->express->object->getType() )
					if ( objectType != type && (objectIterator = objectType->getLocal("next()")) )
						{
						iterator = new Instance(objectIterator);
						if ( type != objectType )
							{
							iterator->cast = new Instance(type);
							iterator->cast->isCast = 1;
							iterator->cast->indirection = 1;
							}
						iterator->setParent(initial->express->object);
						express = new Expression(target,iterator,"=");
						initial = new Instance(express);
						target = 0;
						iterator = 0;
						statement->statementType = WHILE;
						}
					else
					if ( initial->howDirect() == 1 && type == SymbolType::stringType )
						{
						iterator = new Instance(target);
						target->indirection = 1;
						iterator->postfix = "++";
						}
					else {
						error = ::concat(2,target->symbol->name," has no iterator");
						target = p->makeError(error);
						}
			}
		else
		if ( !target->symbol->typeMatch(symbol->type) )
			{
			error = ::concat(4,"type of ",symbol->name," does not match type of ",target->symbol->name);
			target = p->makeError(error);
			}
		else {
			iterator = new Instance(symbol);
			if ( type != symbol->type )
				{
				iterator->cast = new Instance(type);
				iterator->cast->isCast = 1;
				iterator->cast->indirection = 1;
				}
			iterator->setParent(target);
			express = new Expression(target,iterator,"=");
			iterator = new Instance(express);
			}
		}
endForOption:
	statement->first = initial;
	statement->second = target;
	statement->third = iterator;
	instance->value = (void*)statement;
	return 1;
}

int ForOptionTawkNow(PLGitem *iTEM)
{
PLGitem 	*instance = iTEM->get("instance");
PLGitem 	*initial = iTEM->get("initial");
PLGitem 	*condition = iTEM->get("condition");
PLGitem 	*increment = iTEM->get("increment");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
Statement 	*statement = new Statement(FOR);
Instance 	*express = 0;
PLGitem 	*item = 0;
	statement->pointInCode = iTEM;
	if ( initial )
		{
		item = initial->get("list");
		express = (Instance*)item->value;
		statement->first = express;
		}
	if ( condition )
		{
		item = condition->get("instance");
		express = (Instance*)item->value;
		express->isCondition = 1;
		statement->second = express;
		}
	if ( increment )
		{
		item = increment->get("list");
		express = (Instance*)item->value;
		statement->third = express;
		}
	instance->value = (void*)statement;
	p->noShortcuts = 0;
	return 1;
}

int Include2TawkNow(PLGitem *iTEM)
{
PLGitem 	*include = iTEM->get("include");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
	p->formatter->includeText->appendString(include->string());
	include->unString();
	return 1;
}

int Include3TawkNow(PLGitem *iTEM)
{
PLGitem 	*include = iTEM->get("include");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
char 		*sourceFile = include->toString();
char 		*text = ::getStringFromFile(sourceFile);
	if ( !text )
		::fprintf(stderr,"Include: could not get text from %s\n",sourceFile);
	else
	if ( !p->includedFiles->get(sourceFile) )
		{
		p->includedFiles->add(sourceFile);
		//cout "including file: " sourceFile:;
		p->divertInput(text,"Divert");
		//cout `"Done with " start:;
		}
	else	::fprintf(stderr,"Include: source file already loaded, now ignored: %s\n",sourceFile);
	return 1;
}

int IncludeTawkNow(PLGitem *iTEM)
{
PLGitem 	*include = iTEM->get("include");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
	p->formatter->includeText->appendString(include->string());
	p->formatter->includeText->appendString("\n");
	include->unString();
	return 1;
}

int Inheritance4TawkNow(PLGitem *iTEM)
{
PLGitem 	*error = iTEM->get("error");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
	if ( error )
		{
		::printf("ERROR Inheritance: at ==>%s\n",error->string());
		error->unString();
		}
	else	::printf("ERROR: no idea where\n");
	p->extending = 0;
	return 1;
}

int InitExpressionTawkNow(PLGitem *iTEM)
{
PLGitem 	*instance = iTEM->get("instance");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
Instance 	*subject = 0;
PLGitem 	*last = 0;
PLGitem 	*item = 0;
SymbolType 	*type = 0;
	subject = (Instance*)instance->value;
	type = subject->getType();
	if ( type == SymbolType::stringType && subject->howDirect() == 1 )
		{
		last = instance;
		while ( 1 )
			{
			item = p->parse("StringExpression");
			if ( item )
				{
				PLGitem 	*expressItem = item->get("instance");
				Instance 	*secondary = (Instance*)expressItem->value;
				type = secondary->getType();
				if ( type == SymbolType::stringType )
					{
					last->next = expressItem;
					last = last->next;
					}
				else	break;
				}
			else	break;
			}
		if ( instance->next )
			{
			subject = p->concatenate(instance);
			instance->value = (void*)subject;
			}
		}
	return 1;
}

int InitializerItemTawkNow(PLGitem *iTEM)
{
PLGitem 	*field = iTEM->get("field");
PLGitem 	*function = iTEM->get("function");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
SymbolType 	*type = SymbolType::types->getFromItem(field);
char 		*text = function->string();
char 		*signature = ::concat(2,text,"(char*)");
Symbol 		*method = p->currentClass->findField(signature);
	if ( type )
		if ( method )
			{
			type->hasInitializer = 1;
			type->initializer = method;
			method->isInitializer = 1;
			}
		else	::fprintf(stderr,"InitializerItem rule could not find initialer method: %s\n",signature);
	else {
		Symbol 	*virtualField = p->currentClass->get(field);
		if ( !virtualField )
			if ( method )
				{
				virtualField = new Symbol(field->toString(),method->type);
				virtualField->isHidden = 1;
				virtualField->getter = method;
				p->currentClass->add(virtualField);
				}
			else	::fprintf(stderr,"InitializerItem rule could not find getter method: %s\n",signature);
		else	::fprintf(stderr,"InitializerItem rule: virtual field %s already exists\n",field->toString());
		}
	function->unString();
	return 1;
}

int InitializerTawkNow(PLGitem *iTEM)
{
PLGitem 	*instance = iTEM->get("instance");
PLGitem 	*list = instance->get("list");
	instance->value = list->value;
	return 1;
}

int InstanceTailTawkNow(PLGitem *iTEM)
{
PLGitem 	*array = iTEM->get("array");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
PLGitem 	*item = 0;
double 		i = 0;
	for ( item = array; item; item = item->next )
		i++;
	array->amount = i;
	p->noShortcuts = 0;
	return 1;
}

int ItemHeadTawkNow(PLGitem *iTEM)
{
PLGitem 	*array = iTEM->get("array");
PLGitem 	*item = 0;
double 		i = 0;
	if ( array )
		{
		for ( item = array; item; item = item->next )
			i++;
		array->amount = i;
		}
	return 1;
}

int IteratingTawkNow(PLGitem *iTEM)
{
Tawk 	*p = (Tawk*)iTEM->test->testParser;
	p->iterating++;
	return 1;
}

int Lambda2TawkNow(PLGitem *iTEM)
{
PLGitem 	*function = iTEM->get("function");
PLGitem 	*body = iTEM->get("body");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
Instance 	*instance = 0;
Instance 	*bodyInstance = 0;
PLGitem 	*item = 0;
Statement 	*line = 0;
BlockTok 	*block = 0;
	item = function->find("head");
	instance = (Instance*)item->value;
	if ( !instance->isLambda )
		return 0;
	instance->isDeclaration = 1;
	instance->symbol->isAssigned = 1;
	p->currentSymbols->add(instance);
	line = new Statement(LAMBDA);
	line->pointInCode = iTEM;
	line->add(instance);
	item = body->get("start");
	block = (BlockTok*)item->value;
	p->currentClass->hasLambda = 1;
	block->isLambda = 1;
	bodyInstance = new Instance(block);
	line->add(bodyInstance);
	// replaces prior commented out line
	instance = new Instance(line);
	function->value = (void*)instance;
	return 1;
}

int LambdaNameTawkNow(PLGitem *iTEM)
{
PLGitem 	*name = iTEM->get("name");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
Instance 	*instance = 0;
char 		*text = 0;
	if ( !p->currentClass->hasLambda )
		return 0;
	text = name->string();
	if ( !SymbolType::find(text) )
		{
		instance = p->currentSymbols->find(text);
		name->unString();
		if ( instance && instance->symbol && instance->isLambda )
			{
			// Create a copy of instance w/o isDeclaration set
			instance = new Instance(instance->symbol);
			name->value = (void*)instance;
			p->lambdaMethod = instance->symbol;
			}
		else	return 0;
		}
	else {
		name->unString();
		return 0;
		}
	return 1;
}

int LambdaTawkNow(PLGitem *iTEM)
{
PLGitem 	*function = iTEM->get("function");
PLGitem 	*body = iTEM->get("body");
PLGitem 	*item = function->get("name");
Instance 	*lambda = (Instance*)item->value;
Instance 	*lambdaBody = 0;
Expression 	*express = 0;
BlockTok 	*block = 0;
	lambda->symbol->isAssigned = 1;
	lambda->assigning = 1;
	item = body->get("start");
	block = (BlockTok*)item->value;
	block->isLambda = 1;
	lambdaBody = new Instance(block);
	express = new Expression(lambda,lambdaBody,"=");
	lambda = new Instance(express);
	function->value = (void*)lambda;
	return 1;
}

int Line3TawkNow(PLGitem *iTEM)
{
PLGitem 	*name = iTEM->get("name");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
char 		*text = 0;
Instance 	*label = 0;
PLGitem 	*item = 0;
Symbol 		*symbol = 0;
	for ( item = name; item; item = item->next )
		{
		text = name->string();
		label = (Instance*)p->currentSymbols->instances->get(text);
		if ( !label )
			{
			symbol = new Symbol(name->toString(),SymbolType::voidType);
			label = new Instance(symbol);
			label->isLabel = 1;
			p->currentSymbols->add(label);
			}
		name->unString();
		if ( !label->isLabel )
			::fprintf(stderr,"ERROR: label %s already in scope\n",name->toString());
		}
	return 1;
}

int LineByLineTawkNow(PLGitem *iTEM)
{
PLGitem 	*line = iTEM->get("line");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
Statement 	*statement = 0;
PLGitem 	*entry = 0;
PLGitem 	*method = 0;
	for ( entry = line; entry; entry = entry->next )
		if ( method = entry->get("statement") )
			{
			statement = (Statement*)method->value;
			if ( statement )
				p->currentMethod->block->add(statement);
			}
	return 1;
}

int LineTawkNow(PLGitem *iTEM)
{
PLGitem 	*target = iTEM->get("target");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
Instance 	*instance = (Instance*)target->get("instance")->value;
	instance->level = 1;
	p->currentSymbols->add(instance);
	return 1;
}

int MacroBitTawkNow(PLGitem *iTEM)
{
PLGitem 	*bitpart = iTEM->get("bitpart");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
Symbol 		*symbol = 0;
	if ( symbol = (Symbol*)p->macroHash->get(bitpart) )
		iTEM->value = (void*)symbol;
	else	return 0;
	return 1;
}

int MacroBlockTawkNow(PLGitem *iTEM)
{
PLGitem 	*line = iTEM->get("line");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
Statement 	*statement = 0;
PLGitem 	*item = 0;
int 		indentFlag = 0;
	for ( ; line; line = line->next )
		{
		item = line->get("statement");
		if ( !item )
			continue;
		statement = (Statement*)item->value;
		if ( !indentFlag )
			{
			statement->indented = 0;
			indentFlag = 1;
			}
		if ( statement )
			p->currentBlock->add(statement);
		}
	return 1;
}

int MacroBodyTawkNow(PLGitem *iTEM)
{
PLGitem 	*parts = iTEM->get("parts");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
PLGitem 	*body = 0;
PLGitem 	*last = 0;
PLGitem 	*other = 0;
PLGitem 	*part = 0;
PLGitem 	*rest = 0;
	for ( ; parts; parts = parts->next )
		{
		if ( body && !last )
			last = body;
		// Not sure why we cannot say here: if other = parts["other"]
		if ( other = parts->firstComponent )
			{
			if ( other->itemLength )
				{
				if ( !body )
					body = other;
				if ( !last )
					last = body;
				else {
					last->next = other;
					last = last->next;
					}
				}
			if ( part = other->valueItem )
				{
				if ( !body )
					body = part;
				if ( !last )
					last = body;
				else {
					last->next = part;
					last = last->next;
					}
				other->valueItem = 0;
				}
			}
		else
		if ( rest = parts->get("rest") )
			{
			if ( !body )
				body = rest;
			if ( !last )
				last = body;
			else {
				last->next = rest;
				last = last->next;
				}
			}
		}
	p->currentMethod->commentItem = body;
	return 1;
}

int MacroDefineTawkNow(PLGitem *iTEM)
{
PLGitem 	*name = iTEM->get("name");
PLGitem 	*parameters = iTEM->get("parameters");
PLGitem 	*body = iTEM->get("body");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
Symbol 		*argument = 0;
Symbol 		*symbol = 0;
Buffer 		*buffer = p->tokJunkBuffer;
	symbol = new Symbol(name->string(),SymbolType::voidType);
	buffer->reset();
	buffer->appendString(symbol->name);
	if ( !p->macroHash )
		p->macroHash = new BaseHash();
	else	p->macroHash->clear();
	if ( parameters )
		{
		PLGitem 	*item = parameters;
		symbol->isMethod = 1;
		buffer->appendString("(");
		for ( ; item; item = item->next )
			{
			PLGitem 	*element = item->get("element");
			argument = new Symbol(element->string());
			argument->isAlias = 1;
			symbol->addParameter(argument);
			buffer->appendString(argument->name);
			if ( item->next )
				buffer->appendString(",");
			p->macroHash->add(argument->name,(void*)argument);
			}
		buffer->appendString(")");
		}
	symbol->methodName = buffer->toString();
	symbol->isHidden = 1;
	// currentMethod is reused here temporarily then reset
	argument = p->currentMethod;
	p->currentMethod = symbol;
	if ( parameters )
		p->divertInput(body->string(),p->getRule("MacroBody"));
	else {
		symbol->commentItem = body;
		body->value = (void*)0;
		}
	p->currentMethod = argument;
	if ( !p->macroList )
		p->macroList = new BaseHash();
	p->macroList->add(symbol->name,(void*)symbol);
	return 1;
}

int MacroDelimitTawkNow(PLGitem *iTEM)
{
PLGitem 	*delimiter = iTEM->get("delimiter");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
	p->macroDelimiter = delimiter->toString();
	return 1;
}

int MacroNameTawkNow(PLGitem *iTEM)
{
PLGitem 	*name = iTEM->get("name");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
	if ( !p->macroList || !p->macroList->get(name) )
		return 0;
	return 1;
}

void MethodHeadTawkAct(PLGitem *iTEM)
{
PLGitem 	*direct = iTEM->get("direct");
PLGitem 	*function = iTEM->get("function");
PLGitem 	*head = iTEM->get("head");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
Symbol 		*symbol = 0;
Symbol 		*argument = 0;
Instance 	*instance = 0;
PLGitem 	*item = 0;
PLGitem 	*atItem = 0;
PLGitem 	*ellipsis = head->get("ellipsis");
PLGitem 	*parameter = head->get("parameter");
char 		*name = 0;
char 		*methodName = 0;
int 		i = 0;
int 		construct = 0;
int 		lambda = direct && p->lambdaSet->foundIn(direct);
	if ( function )
		name = function->toString();
	else
	if ( lambda )
		name = "lambda";
	else {
		name = p->methodType->name;
		construct = 1;
		}
	if ( lambda )
		p->currentClass->hasLambda = 1;
	methodName = (char*)::alloca(1000);
	::strcpy(methodName,name);
	::strcat(methodName,"(");
	for ( atItem = parameter; atItem; atItem = atItem->next )
		{
		item = atItem->get("type");
		for ( item = (PLGitem*)item->value; item; item = item->next )
			{
			symbol = (Symbol*)item->value;
			if ( symbol->isLambda || (symbol->reference && symbol->isMethod) )
				::strcat(methodName,symbol->getSignature());
			else {
				::strcat(methodName,symbol->type->name);
				for ( i = 0; i < symbol->indirect; i++ )
					::strcat(methodName,"*");
				//if symbol.reference strcat(methodName,"&");
				}
			if ( item->next )
				::strcat(methodName,",");
			}
		if ( atItem->next )
			::strcat(methodName,",");
		else
		if ( ellipsis )
			symbol->hasEllipsis = 1;
		}
	if ( ellipsis )
		::strcat(methodName,",null");
	::strcat(methodName,")");
	symbol = 0;
	if ( p->currentClass )
		if ( p->currentClass->isGlobal )
			symbol = p->currentSymbols->findGlobalMethod(methodName);
		else	symbol = p->currentClass->getMethod(methodName);
	/***************************************************************************
	If symbol then method was probably declared external and need to make
	sure the parameter names match up.
	***************************************************************************/
	if ( symbol )
		{
		while ( symbol->source )
			symbol = symbol->source;
		if ( !(p->currentClass->isGlobal && symbol->parentClass->isGlobal) && symbol->parentClass != p->currentClass )
			goto newSymbol;
		if ( symbol->type != p->methodType )
			::fprintf(stderr,"Warning: multiple types for %s\n",methodName);
		symbol->checkParameters(parameter);
		}
	else {
newSymbol:
		symbol = new Symbol(name,p->methodType);
		symbol->isMethod = 1;
		if ( p->constDeclare )
			symbol->isConst = 1;
		if ( inlineDeclare(p->linkage) )
			symbol->isInline = 1;
		if ( staticDeclare(p->linkage) )
			symbol->isStatic = 1;
		if ( virtualDeclare(p->linkage) )
			symbol->isVirtual = 1;
		if ( externDeclare(p->linkage) )
			{
			symbol->isExtern = 1;
			p->currentClass->hasExtern = 1;
			}
		if ( direct )
			symbol->setIndirection(direct);
		for ( atItem = parameter; atItem; atItem = atItem->next )
			{
			item = atItem->get("type");
			for ( item = (PLGitem*)item->value; item; item = item->next )
				{
				argument = (Symbol*)item->value;
				symbol->addParameter(argument);
				}
			}
		if ( ellipsis )
			{
			Symbol 	*lips = new Symbol("...",SymbolType::nullType);
			lips->hasEllipsis = 1;
			lips->indirect = 0;
			lips->type->isDirect = 1;
			symbol->addParameter(lips);
			symbol->hasEllipsis = 1;
			}
		symbol->mangle();
		if ( p->currentClass && !p->currentMethod && !p->processingParameters && !lambda && !symbol->reference )
			p->currentClass->addMethod(symbol);
		}
	symbol->isConstructor = construct;
	instance = new Instance(symbol);
	head->value = (void*)instance;
	if ( !p->currentMethod && !p->processingParameters && !symbol->reference )
		p->currentMethod = symbol;
	if ( instance->isLambda )
		p->lambdaMethod = symbol;
}

int MethodNameTawkNow(PLGitem *iTEM)
{
PLGitem 	*name = iTEM->get("name");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
char 		*word = name->string();
SymbolType 	*type = SymbolType::find(word);
	if ( p->ReservedWord->find(word) || (type && !type->isGlobal) )
		{
		name->unString();
		return 0;
		}
	name->unString();
	return 1;
}

int MethodTawkNow(PLGitem *iTEM)
{
PLGitem 	*method = iTEM->get("method");
PLGitem 	*block = iTEM->get("block");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
PLGitem 	*body = block->get("start");
PLGitem 	*head = method->find("head");
Instance 	*instance = 0;
	method->value = head->value;
	instance = (Instance*)method->value;
	instance->checkSymbol();
	instance->symbol->block = (BlockTok*)body->value;
	instance->symbol->block->isMethodBlock = 1;
	if ( p->currentComment )
		instance->symbol->comment = p->extractComment();
	p->missingMethods->remove(instance->symbol->methodName);
	if ( instance->symbol->directives )
		::printf("\t%s has directives\n",instance->symbol->methodName);
	else	::printf("\t%s\n",instance->symbol->methodName);
	p->currentMethod = 0;
	p->produceCodeFile = 1;
	/***************************************************************************
	external methods declared in the current file get added to internalType
	so they get declared in the Start rule. The parentClass of the method
	is reset to globalType (internalType is not for public consumption).
	***************************************************************************/
	if ( instance->symbol->parentClass && instance->symbol->parentClass->isExternal && instance->symbol->parentClass->isGlobal )
		{
		SymbolType::internalType->addMethod(instance->symbol);
		instance->symbol->parentClass = SymbolType::globalType;
		}
	return 1;
}

void MethodTypeTawkAct(PLGitem *iTEM)
{
PLGitem 	*type = iTEM->get("type");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
	p->methodType = (SymbolType*)type->value;
}

int MethodTypeTawkNow(PLGitem *iTEM)
{
PLGitem 	*modify = iTEM->get("modify");
PLGitem 	*type = iTEM->get("type");
PLGitem 	*methodHead = iTEM->get("methodHead");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
Instance 	*instance = 0;
PLGitem 	*head = 0;
PLGitem 	*item = 0;
	// follow is not used but apparently it has to be there. Figure out why.
	p->methodType = (SymbolType*)type->value;
	for ( item = modify; item; item = item->next )
		if ( *item->itemStart == 'c' )
			p->constDeclare = 1;
		else
		if ( *item->itemStart == 'i' )
			p->linkage = 2;
		else
		if ( *item->itemStart == 's' )
			p->linkage = 3;
		else
		if ( *item->itemStart == 'v' )
			p->linkage = 4;
		else	p->linkage = 1;
	methodHead->runDeferred();
	head = methodHead->get("head");
	instance = (Instance*)head->value;
	if ( modify )
		{
		p->constDeclare = 0;
		p->linkage = 0;
		}
	p->declaringMethod = 1;
	return 1;
}

int NameTawkNow(PLGitem *iTEM)
{
PLGitem 	*name = iTEM->get("name");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
char 		*word = name->string();
SymbolType 	*type = SymbolType::find(word);
	if ( p->ReservedWord->find(word) || type )
		{
		name->unString();
		return 0;
		}
	name->unString();
	return 1;
}

void NewTawkAct(PLGitem *iTEM)
{
PLGitem 	*instance = iTEM->get("instance");
PLGitem 	*type = iTEM->get("type");
PLGitem 	*body = iTEM->get("body");
PLGitem 	*initial = iTEM->get("initial");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
	/**********************************************************************
	Need to check that parameters are of the right types
	If type is not provided, it gets set in the expression that
	invokes the new statement.
	**********************************************************************/
SymbolType 	*symbolType = 0;
Symbol 		*symbol = 0;
Expression 	*express = 0;
PLGitem 	*item = 0;
Instance 	*current = 0;
Instance 	*parameter = 0;
char 		*allocator = 0;
int 		skipConstructor = 0;
	if ( type )
		symbolType = (SymbolType*)type->value;
	else	symbolType = p->newType;
	if ( !symbolType )
		symbolType = SymbolType::nullType;
	if ( symbolType == SymbolType::nullType )
		current = p->getInstance("No type found");
	if ( symbolType->constructor )
		{
		if ( p->currentMethod && ::compare(symbolType->constructor,p->currentMethod->name) == 0 )
			{
			//currentMethod.isConstructor = true;
			skipConstructor = p->currentMethod->isInitialized = 1;
			}
		if ( !skipConstructor )
			allocator = symbolType->constructor;
		}
	if ( !allocator )
		allocator = symbolType->name;
	if ( body )
		{
		item = body->get("array");
		if ( item )
			{
			symbol = new Symbol("new",symbolType);
			current = new Instance(symbol);
			current->symbol->isArray = (unsigned int)item->amount;
			}
		else {
			if ( symbolType->isOC )
				{
				current = p->getInstance("init");
				current->type = symbolType;
				}
			else	current = p->getInstance(allocator);
			current->isMethod = 1;
			item = body->get("expression");
			}
		}
	else
	if ( symbolType->isOC )
		{
		current = p->getInstance("init");
		current->isMethod = 1;
		current->type = symbolType;
		}
	else {
		current = p->getInstance(allocator);
		current->isMethod = 1;
		}
	for ( ; item; item = item->next )
		{
		parameter = (Instance*)item->get("instance")->value;
		current->addParameter(parameter);
		}
	if ( symbol && initial )
		{
		parameter = (Instance*)initial->value;
		express = new Expression(current,parameter,"=");
		current = new Instance(express);
		}
	if ( !symbol )
		{
		if ( symbolType->constructor && !skipConstructor )
			{
			parameter = p->currentSymbols->findMethod(current);
			if ( parameter )
				{
				symbol = parameter->symbol;
				current->setParent(parameter->parent);
				}
			else {
				skipConstructor = 1;
				symbol = symbolType->findMethod(current);
				}
			}
		else	symbol = symbolType->findMethod(current);
		if ( !symbol )
			if ( !symbolType->isOC )
				symbol = new Symbol(symbolType->name,symbolType);
			else {
				symbol = symbolType->getLocal("init");
				if ( !symbol )
					{
					symbol = new Symbol("init",symbolType);
					symbol->isMethod = 1;
					symbol->isOCfield = 1;
					symbol->parentClass = symbolType;
					}
				}
		if ( !current )
			current = new Instance(symbol);
		else	current->symbol = symbol;
		current->type = 0;
		current->prefix = 0;
		if ( !symbolType->isOC )
			if ( symbolType->constructor && !skipConstructor )
				current->type = symbolType;
			else	current->prefix = "new ";
		}
	if ( !symbolType->constructor )
		current->isNew = 1;
	current->isConstant = 0;
	instance->value = (void*)current;
}

int NoShortcutsTawkNow(PLGitem *iTEM)
{
Tawk 	*p = (Tawk*)iTEM->test->testParser;
	p->noShortcuts = 1;
	return 1;
}

int Number2TawkNow(PLGitem *iTEM)
{
PLGitem 	*instance = iTEM->get("instance");
PLGitem 	*isLong = iTEM->get("isLong");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
Instance 	*number = 0;
SymbolType 	*type = 0;
	type = isLong ? SymbolType::longType : SymbolType::intType;
	number = p->getInstance(instance->toString());
	number->isConstant = 1;
	number->type = type;
	if ( isLong )
		number->postfix = "LL";
	instance->value = (void*)number;
	return 1;
}

int NumberTawkNow(PLGitem *iTEM)
{
PLGitem 	*instance = iTEM->get("instance");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
Instance 	*number = 0;
SymbolType 	*type = SymbolType::doubleType;
	number = p->getInstance(instance->toString());
	number->isConstant = 1;
	number->type = type;
	instance->value = (void*)number;
	return 1;
}

int OperationTail2TawkNow(PLGitem *iTEM)
{
PLGitem 	*instance = iTEM->get("instance");
PLGitem 	*question = instance->get("question");
	instance->value = question->value;
	return 1;
}

int OperationTailTawkNow(PLGitem *iTEM)
{
PLGitem 	*operate = iTEM->get("operate");
PLGitem 	*instance = iTEM->get("instance");
PLGitem 	*question = iTEM->get("question");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
PLGitem 	*item = 0;
PLGitem 	*operand = 0;
SymbolType 	*type = 0;
Expression 	*expression = 0;
Instance 	*subject = 0;
Instance 	*trueValue = 0;
char 		*flag = 0;
char 		*save = p->plgStart;
	operand = operate->get("operand");
	if ( operand )
		{
		if ( p->stringing && p->expressType == SymbolType::stringType && !p->stringOP->foundIn(operand) )
			return 0;
		if ( *operand->itemStart == '=' && operand->itemLength == 1 )
			flag = operand->itemStart + 1;
		}
	else
	if ( operand = operate->get("comparator") )
		flag = operand->itemStart + 3;
	if ( question )
		{
		subject = (Instance*)instance->value;
		trueValue = (Instance*)question->value;
		expression = new Expression(subject,trueValue,"?");
		subject = new Instance(expression);
		instance->value = (void*)subject;
		flag = 0;
		}
	else	subject = (Instance*)instance->value;
	if ( subject->parent && subject->isConstant )
		instance->value = (void*)subject;
	p->expressType = type = subject->getType();
	if ( flag )
		if ( type == SymbolType::stringType && subject->howDirect() == 1 )
			{
			p->plgStart = flag;
			p->stringing = 1;
			if ( item = p->parse("Strings") )
				{
				subject = p->concatenate(item->get("item"));
				instance->value = (void*)subject;
				}
			else	p->plgStart = save;
			p->stringing = 0;
			}
	((Tawk*)iTEM->test->testParser)->assigning = 0;
	return 1;
}

int Operator2TawkNow(PLGitem *iTEM)
{
PLGitem 	*comparator = iTEM->get("comparator");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
	if ( !p->compareFollow->contains(*(comparator->itemStart + 2)) )
		return 0;
	return 1;
}

int OperatorTawkNow(PLGitem *iTEM)
{
PLGitem 		*operand = iTEM->get("operand");
Tawk 			*p = (Tawk*)iTEM->test->testParser;
KeyTableItem 	*operatorItem = (KeyTableItem*)operand->value;
Operate 		*verb = (Operate*)operatorItem->value;
	if ( verb->overload )
		return 1;
	if ( verb->isRange )
		return 0;
	if ( p->assigning && *operand->itemStart == ',' )
		return 0;
	if ( !verb || verb->conjunction || verb->question )
		return 0;
	if ( p->virtualItem )
		{
		SymbolType 	*type = p->virtualItem->getType();
		if ( type->isVirtuous )
			if ( type->overloaded(verb->op) || (p->virtualItem->arrayRef && *operand->itemStart == '=' && operand->itemLength == 1 && type->overloaded("[]=")) )
				p->virtualOp = operand;
		}
	if ( verb->assign && p->saveType )
		{
		p->newType = p->saveType;
		if ( p->expressType == SymbolType::stringType && !p->assuming )
			p->assigning = 1;
		}
	return 1;
}

int OverLoadItem2TawkNow(PLGitem *iTEM)
{
PLGitem 	*assign = iTEM->get("assign");
PLGitem 	*name = iTEM->get("name");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
char 		*text = name->toString();
	if ( p->currentClass )
		if ( assign )
			p->currentClass->overload("[]=",text);
		else	p->currentClass->overload("[]",text);
	else	::fprintf(stderr,"Overload specification must be within class\n");
	p->currentClass->isVirtuous = 1;
	return 1;
}

int OverLoadItem3TawkNow(PLGitem *iTEM)
{
PLGitem 	*operate = iTEM->get("operate");
PLGitem 	*name = iTEM->get("name");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
char 		*text = name->toString();
	if ( p->currentClass )
		p->currentClass->overload(operate->toString(),text);
	else	::fprintf(stderr,"Overload specification must be within class\n");
	return 1;
}

int OverLoadItem4TawkNow(PLGitem *iTEM)
{
PLGitem 	*newOp = iTEM->get("newOp");
PLGitem 	*name = iTEM->get("name");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
Operate 	*verb = new Operate(newOp->toString());
char 		*text = name->toString();
	if ( p->currentClass )
		p->currentClass->overload(verb->op,text);
	else	::fprintf(stderr,"Overload specification must be within class\n");
	return 1;
}

int OverLoadItem5TawkNow(PLGitem *iTEM)
{
PLGitem 	*name = iTEM->get("name");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
char 		*text = name->toString();
	if ( p->currentClass )
		p->currentClass->overload("()",text);
	else	::fprintf(stderr,"Overload specification must be within class\n");
	return 1;
}

int OverLoadItemTawkNow(PLGitem *iTEM)
{
PLGitem 	*operate = iTEM->get("operate");
PLGitem 	*name = iTEM->get("name");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
PLGitem 	*operand = operate->get("operand");
char 		*text = name->toString();
	p->assigning = 0;
	if ( !operand )
		operand = operate->get("comparator");
	if ( !p->currentClass )
		::fprintf(stderr,"Overload specification must be within class: %s\n",iTEM->toString());
	else	p->currentClass->overload(operand->toString(),text);
	name->unString();
	return 1;
}

void ParameterItem2TawkAct(PLGitem *iTEM)
{
PLGitem 	*direct = iTEM->get("direct");
PLGitem 	*name = iTEM->get("name");
PLGitem 	*array = iTEM->get("array");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
Symbol 		*symbol = 0;
	symbol = new Symbol(name->toString(),p->currentType);
	if ( direct )
		symbol->setIndirection(direct);
	if ( array )
		{
		symbol->array = array->toString();
		for ( ; array; array = array->next )
			{
			symbol->indirect++;
			symbol->isArray++;
			}
		}
	name->value = (void*)symbol;
}

void ParameterItem3TawkAct(PLGitem *iTEM)
{
PLGitem 	*name = iTEM->get("name");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
Symbol 		*symbol = 0;
	symbol = new Symbol("",p->currentType);
	symbol->array = name->toString();
	for ( ; name; name = name->next )
		{
		symbol->indirect++;
		symbol->isArray++;
		}
	name->value = (void*)symbol;
	name->next = 0;
}

void ParameterItem4TawkAct(PLGitem *iTEM)
{
PLGitem 	*name = iTEM->get("name");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
Symbol 		*symbol = 0;
	symbol = new Symbol("",p->currentType);
	symbol->setIndirection(name);
	name->value = (void*)symbol;
}

void ParameterItemTawkAct(PLGitem *iTEM)
{
PLGitem 	*name = iTEM->get("name");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
PLGitem 	*head = name->get("head");
Instance 	*instance = 0;
	p->processingParameters = 1;
	name->runDeferred();
	instance = (Instance*)head->value;
	name->value = (void*)instance->symbol;
	p->processingParameters = 0;
}

int ParameterTawkNow(PLGitem *iTEM)
{
PLGitem 	*type = iTEM->get("type");
PLGitem 	*item = iTEM->get("item");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
Symbol 		*symbol = 0;
PLGitem 	*last = 0;
PLGitem 	*name = 0;
	p->methodType = (SymbolType*)type->value;
	if ( !item )
		{
		symbol = new Symbol("",p->methodType);
		name = p->plgItemFactory("no field specified");
		name->value = (void*)symbol;
		}
	else
	for ( ; item; item = item->next )
		{
		item->runDeferred();
		if ( !last )
			last = name = item->get("name");
		else
		if ( last->next = item->get("name") )
			last = last->next;
		symbol = (Symbol*)last->value;
		if ( type->flag4 )
			symbol->isConst = 1;
		}
	type->value = (void*)name;
	return 1;
}

int PoundCommandTawkNow(PLGitem *iTEM)
{
PLGitem 	*state = iTEM->get("state");
PLGitem 	*type = iTEM->get("type");
PLGitem 	*level = iTEM->get("level");
PLGitem 	*list = iTEM->get("list");
PLGitem 	*field = iTEM->get("field");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
SymbolType 	*symbolType = 0;
PLGitem 	*end = state;
	switch (*state->itemStart)
		{
		case 'a':
			if ( p->currentClass )
				p->currentClass->autoGetSet = !p->currentClass->getAutoGetSet();
			break;
		case 'd':
			if ( *(state->itemStart + 1) == 'E' )
				if ( list )
					{
					iTEM->test->testParser->currentRule->debug = 0;
					end = list;
					}
				else
				if ( iTEM->test->testParser->currentRule->debug )
					iTEM->test->testParser->currentRule->debug = 0;
				else	p->debugRulePLG = 1;
			else {
				if ( type )
					{
					if ( symbolType = (SymbolType*)type->value )
						{
						symbolType->dump();
						if ( field )
							symbolType->dumpFields();
						}
					if ( field )
						end = field;
					else	end = type;
					}
				else
				if ( level )
					{
					p->currentSymbols->dump();
					end = level;
					}
				else {
					p->currentSymbols->dump("Symbol Table");
					p->currentSymbols->dumpGlobals();
					}
				}
			break;
		case 'i':
			if ( p->currentMethod )
				{
				p->currentMethod->isInitialized = 1;
				if ( p->currentMethod->isAlias )
					p->currentMethod->source->isInitialized = 1;
				}
			break;
		case 'm':
			p->debugging = !p->debugging;
			break;
		case 'r':
			p->defaultPrinter = p->currentSymbols->find("printf");
			break;
		case 's':
			p->summaryDebug();
			break;
		case 't':
			::printf("At trace: %d %d\n",Symbol::symbolCount,Instance::instanceCount);
			//debugTest = !debugTest;
		}
resetPointer:
	iTEM->test->current = p->plgStart = end->itemStart + end->itemLength;
	return 1;
}

int PrintCommand2TawkNow(PLGitem *iTEM)
{
PLGitem 	*stdPrint = iTEM->get("stdPrint");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
Instance 	*newPrinter = p->currentSymbols->find("printf");
	stdPrint->value = (void*)newPrinter;
	return 1;
}

int PrintCommand3TawkNow(PLGitem *iTEM)
{
PLGitem 	*stdPrint = iTEM->get("stdPrint");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
Instance 	*target = p->currentSymbols->find("stderr");
Instance 	*newPrinter = p->currentSymbols->find("fprintf");
	newPrinter->addParameter(target);
	stdPrint->value = (void*)newPrinter;
	return 1;
}

int PrintCommandTawkNow(PLGitem *iTEM)
{
PLGitem 	*printer = iTEM->get("printer");
PLGitem 	*target = iTEM->get("target");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
	printer->value = (void*)p->processPrintTarget(target);
	return 1;
}

int PrintItem2TawkNow(PLGitem *iTEM)
{
PLGitem 	*instance = iTEM->get("instance");
PLGitem 	*format = iTEM->get("format");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
Instance 	*item = (Instance*)instance->value;
	if ( format )
		{
		PLGitem 	*width = format->get("width");
		if ( format->itemLength > 1 )
			{
			*format->itemStart = '%';
			}
		item->format = p->getInstance(format->toString());
		item->format->type = SymbolType::stringType;
		item->format->indirection = 1;
		item->format->isConstant = 1;
		if ( width )
			{
			item->format->format = p->getInstance(width->toString());
			item->format->format->type = SymbolType::intType;
			item->format->format->isConstant = 1;
			}
		}
	return 1;
}

int PrintShortcutTawkNow(PLGitem *iTEM)
{
PLGitem 	*instance = iTEM->get("instance");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
Instance 	*item = 0;
	if ( *instance->itemStart == ',' )
		item = p->getInstance(" ");
	else
	if ( *instance->itemStart == '`' )
		item = p->getInstance("\\t");
	else	item = p->getInstance("\\n");
	item->isConstant = 1;
	item->type = SymbolType::stringType;
	item->indirection = 1;
	instance->value = (void*)item;
	return 1;
}

int PrintTawkNow(PLGitem *iTEM)
{
PLGitem 	*start = iTEM->get("start");
PLGitem 	*arguments = iTEM->get("arguments");
PLGitem 	*output = iTEM->get("output");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
PLGitem 	*item = 0;
PLGitem 	*argument = 0;
Instance 	*method = 0;
SymbolType 	*type = 0;
BlockTok 	*block = 0;
Statement 	*statement = 0;
Instance 	*converter = 0;
Instance 	*format = 0;
Instance 	*savePrinter = p->defaultPrinter;
Instance 	*text = 0;
	if ( item = start->get("printer") )
		if ( output )
			method = p->processPrintTarget(output);
		else	method = (Instance*)item->value;
	else {
		item = start->get("stdPrint");
		method = (Instance*)item->value;
		}
	if ( !method->isPrintMethod )
		{
		p->tokJunkBuffer->reset();
		for ( argument = arguments; argument; argument = argument->next )
			{
			format = 0;
			item = argument->get("instance");
			text = (Instance*)item->value;
			format = text->getFormat();
			type = text->getType();
			if ( text->express && !text->express->verb && !text->cast )
				text = text->getSubject();
			if ( text->isConstant && !text->isMethod )
				p->tokJunkBuffer->appendString(text->prefix);
			else
			if ( type != SymbolType::stringType && text->howDirect() <= 1 && type->getMethod("toString") )
				p->tokJunkBuffer->appendString("%s");
			else
			if ( format )
				p->tokJunkBuffer->appendString(format->prefix);
			else {
				text->error("No toString method");
				p->tokJunkBuffer->appendString("%s");
				}
			}
		text = p->getInstance(p->tokJunkBuffer->toString());
		text->type = SymbolType::stringType;
		text->isConstant = 1;
		text->indirection = 1;
		method->addParameter(text);
		for ( argument = arguments; argument; argument = argument->next )
			{
			item = argument->get("instance");
			text = (Instance*)item->value;
			type = text->getType();
			if ( text->express && !text->express->verb && !text->cast )
				text = text->getSubject();
			if ( text->isConstant && !text->isMethod )
				continue;
			if ( type != SymbolType::stringType && text->howDirect() <= 1 )
				{
				Symbol 	*toString = type->getMethod("toString");
				if ( toString )
					{
					converter = new Instance(toString);
					converter->setParent(text);
					text = converter;
					}
				}
			method->addParameter(text);
			}
		}
	else {
		//		Print using the print target object's print methods
		//		Need to add a string concatenation loop
		if ( arguments->next )
			block = new BlockTok();
		for ( argument = arguments; argument; argument = argument->next )
			{
			item = argument->get("instance");
			text = (Instance*)item->value;
			method = p->generatePrint(text);
			if ( block )
				{
				statement = new Statement();
				statement->pointInCode = iTEM;
				statement->add(method);
				block->add(statement);
				}
			}
		if ( block )
			method = new Instance(block);
		}
	start->value = (void*)method;
	if ( output )
		p->defaultPrinter = savePrinter;
	return 1;
}

int QualifiedTawkNow(PLGitem *iTEM)
{
PLGitem 	*type = iTEM->get("type");
PLGitem 	*field = iTEM->get("field");
PLGitem 	*rest = iTEM->get("rest");
PLGitem 	*postfix = iTEM->get("postfix");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
PLGitem 	*item = 0;
PLGitem 	*tail = 0;
PLGitem 	*name = field->find("name");
Instance 	*parent = 0;
Instance 	*child = 0;
	p->isQualified = 0;
	if ( type )
		{
		p->setCurrentType((SymbolType*)type->value);
		child = new Instance(p->currentType);
		}
	if ( child )
		p->isQualified = 1;
	field->runDeferred();
	parent = (Instance*)name->value;
	/*************************************************************************
	If field is an assumed string, parent will be none so we bail
	*************************************************************************/
	if ( !parent )
		return 0;
	// The following inserts the parent class reference
	if ( child )
		parent = parent->copyAndSetParent(child);
	p->setCurrentType(parent->getType());
	if ( rest )
		p->isQualified = 1;
	/*************************************************************************
	If this is a qualifier (has following .) or if there is not symbol
	(unknown field) resolve virtue here
	*************************************************************************/
	if ( rest && p->currentType && p->currentType->isVirtuous && parent->arrayRef && !parent->resolved )
		{
		parent = parent->checkOverload();
		if ( parent->resolved )
			{
			p->setCurrentType(parent->getType());
			// Not sure why I do the following
			if ( parent->reference && p->currentType->isOC )
				parent->setReference((unsigned int)0);
			}
		}
	for ( item = rest; item; item = item->next )
		{
		tail = item->get("field");
		tail->runDeferred();
		name = tail->find("name");
		if ( child = (Instance*)name->value )
			{
			if ( !child->symbol )
				if ( p->currentType->isOC )
					;
				else
				if ( p->currentType->isVirtuous )
					{
					char 	*text = ::concat(2,child->prefix," is not a valid symbol");
					child->error(text);
					}
			child = child->copyAndSetParent(parent);
			}
		parent = child;
		if ( !parent )
			break;
		p->setCurrentType(parent->getType());
		}
	p->saveType = p->currentType;
	p->setCurrentType((SymbolType*)0);
	if ( !parent )
		return 0;
	if ( postfix )
		{
		Expression 	*expression = new Expression(parent,(Instance*)0,postfix->string());
		postfix->unString();
		parent = new Instance(expression);
		}
	/*************************************************************************
	Default parameters handled here because here we know the method parent
	if there is one.
	*************************************************************************/
	if ( parent->symbol && parent->symbol->isDefault && parent->isMethod )
		parent->setDefaults(p);
	field->value = (void*)parent;
	p->isQualified = 0;
	return 1;
}

int QualifyStartTawkNow(PLGitem *iTEM)
{
PLGitem 	*name = iTEM->get("name");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
Symbol 		*symbol = 0;
Instance 	*instance = 0;
	if ( p->currentClass->isC )
		return 0;
	symbol = new Symbol("this",p->currentClass);
	instance = new Instance(symbol);
	name->value = (void*)instance;
	return 1;
}

int QuestionTawkNow(PLGitem *iTEM)
{
PLGitem 	*question = iTEM->get("question");
PLGitem 	*trueExp = iTEM->get("trueExp");
PLGitem 	*falseExp = iTEM->get("falseExp");
PLGitem 	*falseItem = 0;
PLGitem 	*trueItem = 0;
Expression 	*expression = 0;
Instance 	*trueValue = 0;
Instance 	*falseValue = 0;
	falseItem = falseExp->get("instance");
	trueItem = trueExp->get("instance");
	trueValue = (Instance*)trueItem->value;
	falseValue = (Instance*)falseItem->value;
	expression = new Expression(trueValue,falseValue,":");
	trueValue = new Instance(expression);
	question->value = (void*)trueValue;
	return 1;
}

int QuoteTawkNow(PLGitem *iTEM)
{
PLGitem 	*string = iTEM->get("string");
PLGitem 	*instance = iTEM->get("instance");
PLGitem 	*body = iTEM->get("body");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
Buffer 		*buffer = p->tokJunkBuffer;
Instance 	*current = 0;
char 		*mark = 0;
	/**********************************************************************
	Check for multi-line strings
	**********************************************************************/
	buffer->reset();
	if ( !body )
		current = p->getInstance("");
	else {
		mark = body->string();
		while ( *mark )
			{
			if ( *mark == '\n' )
				{
				buffer->appendChar('\\');
				buffer->appendChar('n');
				}
			else	buffer->appendChar(*mark);
			mark++;
			}
		current = p->getInstance(buffer->toString());
		body->unString();
		}
	current->isConstant = 1;
	current->indirection = 1;
	// constant strings are really pointers
	current->type = SymbolType::stringType;
	if ( string )
		current->atString = 1;
	instance->value = (void*)current;
	return 1;
}

int RangeExpressionTawkNow(PLGitem *iTEM)
{
PLGitem 	*instance = iTEM->get("instance");
PLGitem 	*back = iTEM->get("back");
PLGitem 	*item = 0;
Instance 	*front = (Instance*)instance->value;
Instance 	*tail = 0;
Expression 	*rangeX = 0;
	item = back->get("instance");
	tail = (Instance*)item->value;
	item = back->get("operate");
	rangeX = new Expression(front,tail,item->string());
	front = new Instance(rangeX);
	front->isRange = 1;
	instance->value = (void*)front;
	return 1;
}

int RangeFieldTawkNow(PLGitem *iTEM)
{
PLGitem 	*instance = iTEM->get("instance");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
Instance 	*rangeField = (Instance*)p->currentSymbols->instances->get(instance->string());
	instance->unString();
	if ( rangeField && rangeField->isRange && rangeField->express && rangeField->express->object )
		instance->value = (void*)rangeField->express->object;
	else	return 0;
	return 1;
}

int ResetTypeTawkNow(PLGitem *iTEM)
{
Tawk 	*p = (Tawk*)iTEM->test->testParser;
	p->setCurrentType((SymbolType*)0);
	return 1;
}

int SaveVirtualsTawkNow(PLGitem *iTEM)
{
Tawk 	*p = (Tawk*)iTEM->test->testParser;
	p->pushVirtuals();
	return 1;
}

int SecondaryExpression2TawkNow(PLGitem *iTEM)
{
PLGitem 	*instance = iTEM->get("instance");
PLGitem 	*pointer = iTEM->get("pointer");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
char 		*text = 0;
	if ( pointer )
		{
		text = ::concat(5,instance->string(),"(",p->currentType->name,pointer->string(),")");
		pointer->unString();
		}
	else	text = ::concat(4,instance->string(),"(",p->currentType->name,")");
Instance 	*current = p->getInstance(text);
	current->type = SymbolType::intType;
	current->isConstant = 1;
	current->isMethod = 1;
	// so will not screw up as a print argument
	instance->value = (void*)current;
	instance->unString();
	p->currentType->setRefer();
	return 1;
}

int SecondaryExpressionTawkNow(PLGitem *iTEM)
{
PLGitem 	*instance = iTEM->get("instance");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
Instance 	*current = new Instance(p->nullSymbol);
	instance->value = (void*)current;
	current->isConstant = 1;
	return 1;
}

int SetObjectTawkNow(PLGitem *iTEM)
{
Tawk 	*p = (Tawk*)iTEM->test->testParser;
	p->setCurrentType((SymbolType*)0);
	return 1;
}

int StartTawkNow(PLGitem *iTEM)
{
Tawk 		*p = (Tawk*)iTEM->test->testParser;
DoubleLink 	*link = 0;
Instance 	*instance = 0;
SymbolType 	*type = 0;
Buffer 		*saveBuffer = 0;
char 		*codefile = 0;
	SymbolType::types->resetIsFlagged();
	p->setCurrentClass(p->formatter->currentType);
	if ( !p->currentClass || !p->currentClass->classOK )
		{
		::fprintf(stderr,"ERROR outputting class\n");
		return 0;
		}
	p->tokJunkBuffer->reset();
	saveBuffer = p->formatter->buffer;
	p->formatter->buffer = p->formatter->headerBuffer;
	p->formatter->forwardClass(SymbolType::internalType);
	p->formatter->buffer->setMark();
	p->formatter->processingGlobalMethods = 1;
	p->formatter->declareHeaders(SymbolType::internalType);
	if ( SymbolType::internalType->descendentTypes )
		while ( type = (SymbolType*)SymbolType::internalType->descendentTypes->next() )
			if ( !type->isDeclared && !type->isExternal )
				p->formatter->declareStructure(type);
	if ( p->missingMethods->hashList->length > 0 )
		{
		if ( !p->formatter->errorBuffer )
			p->formatter->errorBuffer = ::bufferFactory2("errors");
		p->formatter->errorBuffer->appendString("/*");
		p->formatter->errorBuffer->appendString("\t");
		p->formatter->errorBuffer->appendString("Warning: the following methods were referenced but not declared");
		p->formatter->errorBuffer->appendString("\n");
		p->missingMethods->hashList->entry = 0;
		while ( link = p->missingMethods->hashList->nextLink() )
			{
			instance = (Instance*)link->value;
			p->formatter->errorBuffer->appendString("\t");
			p->formatter->errorBuffer->appendString(link->key);
			p->formatter->errorBuffer->appendString("\n");
			}
		p->formatter->errorBuffer->appendString("*/");
		p->formatter->errorBuffer->appendString("\n");
		}
	p->formatter->buffer = saveBuffer;
	p->formatter->declareBody(SymbolType::internalType);
	p->formatter->processingGlobalMethods = 0;
	p->formatter->printCode();
	if ( p->formatter->errorBuffer )
		{
		p->formatter->buffer->appendString(p->formatter->errorBuffer->string());
		p->formatter->errorBuffer->reset();
		}
	if ( p->currentClass->isOC || p->formatter->makeOCfile )
		codefile = ::concat(2,p->formatter->filename,".mm");
	else	codefile = ::concat(2,p->formatter->filename,".C");
	p->formatter->buffer = p->formatter->includeText;
	if ( p->formatter->currentType )
		if ( p->produceCodeFile || p->formatter->currentType->hasMethods )
			p->formatter->buffer->setFile(codefile);
		else	::printf("Code file not produced for %s because no methods specified\n",p->formatter->currentType->name);
	while ( type = (SymbolType*)SymbolType::types->hashList->next() )
		if ( type->codeBuffer )
			{
			p->formatter->buffer->appendString(type->codeBuffer->string());
			type->codeBuffer = 0;
			}
	p->referring = 0;
	p->formatter->close();
	return 1;
}

int Statement2TawkNow(PLGitem *iTEM)
{
PLGitem 	*statement = iTEM->get("statement");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
Statement 	*line = (Statement*)statement->value;
	if ( line )
		{
		if ( !line->pointInCode )
			line->pointInCode = iTEM;
		line->setIsUsed();
		if ( p->currentMethod && p->currentMethod->directives && !p->noLoop )
			{
			Directive 	*directive = 0;
			p->currentMethod->directives->resetIterator();
			while ( directive = (Directive*)p->currentMethod->directives->next() )
				if ( directive->isDirected || !directive->codeMatch )
					continue;
				else
				if ( !::strncmp(directive->codeMatch,line->pointInCode->itemStart,::strlen(directive->codeMatch)) )
					{
					p->noLoop = 1;
					p->parsingDirective = 1;
					directive->parseDirective();
					p->parsingDirective = 0;
					p->noLoop = 0;
					break;
					}
			}
		}
	return 1;
}

int StatementBody10TawkNow(PLGitem *iTEM)
{
PLGitem 	*statement = iTEM->get("statement");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
Instance 	*instance = p->getInstance("continue");
Statement 	*line = new Statement();
	line->add(instance);
	// continue
	line->branch = 1;
	statement->value = (void*)line;
	return 1;
}

int StatementBody11TawkNow(PLGitem *iTEM)
{
PLGitem 	*statement = iTEM->get("statement");
PLGitem 	*direct = iTEM->get("direct");
PLGitem 	*field = iTEM->get("field");
Instance 	*instance = 0;
Statement 	*line = new Statement(GOTO);
	// goto
	instance = (Instance*)field->value;
	if ( direct )
		instance = instance->setIndirectItem(direct);
	line->add(instance);
	line->branch = 1;
	statement->value = (void*)line;
	return 1;
}

int StatementBody12TawkNow(PLGitem *iTEM)
{
PLGitem 	*statement = iTEM->get("statement");
PLGitem 	*block = iTEM->get("block");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
Statement 	*switchStatement = new Statement(SWITCH);
PLGitem 	*item = block->get("start");
BlockTok 	*body = (BlockTok*)item->value;
Instance 	*trigger = (Instance*)statement->value;
SymbolType 	*type = 0;
	// switch
	body->isSwitch = 1;
	if ( trigger )
		{
		type = trigger->getType();
		switchStatement->add(trigger);
		}
	switchStatement->add(body);
	if ( !trigger )
		{
		// jiggery pokery to put body in right place w/no switch parameter
		switchStatement->second = switchStatement->first;
		switchStatement->first = 0;
		}
	switchStatement->indented = 1;
	switchStatement->noFallThru = !statement->flag1;
	statement->value = (void*)switchStatement;
	if ( !trigger || trigger->isRange || (!type->isNumber && !(type == SymbolType::stringType && trigger->howDirect() != 1)) )
		switchStatement->switching = 1;
	p->switchStack->pop();
	return 1;
}

int StatementBody14TawkNow(PLGitem *iTEM)
{
PLGitem 	*statement = iTEM->get("statement");
Statement 	*line = new Statement();
	// ;
	statement->value = (void*)line;
	return 1;
}

int StatementBody15TawkNow(PLGitem *iTEM)
{
PLGitem 	*statement = iTEM->get("statement");
PLGitem 	*array = iTEM->get("array");
PLGitem 	*instance = iTEM->get("instance");
Statement 	*line = new Statement(DELETE);
	// delete
PLGitem 	*item = instance->get("field");
Instance 	*current = (Instance*)item->value;
	if ( array )
		current->postfix = "[]";
	line->add(current);
	statement->value = (void*)line;
	return 1;
}

int StatementBody16TawkNow(PLGitem *iTEM)
{
PLGitem 	*statement = iTEM->get("statement");
PLGitem 	*instance = iTEM->get("instance");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
Statement 	*doStatement = new Statement(DO);
Instance 	*test = (Instance*)instance->value;
Statement 	*body = (Statement*)statement->value;
	// do
	doStatement->add(body);
	doStatement->add(test);
	doStatement->pointInCode = iTEM;
	test->isCondition = 1;
	statement->value = (void*)doStatement;
	p->iterating--;
	return 1;
}

int StatementBody17TawkNow(PLGitem *iTEM)
{
PLGitem 	*statement = iTEM->get("statement");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
Statement 	*line = new Statement();
	// throw
Instance 	*instance = 0;
	instance = p->getInstance("Saw a throw expression");
	instance->isComment = 1;
	line->add(instance);
	statement->value = (void*)line;
	return 1;
}

int StatementBody18TawkNow(PLGitem *iTEM)
{
PLGitem 	*statement = iTEM->get("statement");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
Statement 	*line = new Statement();
	// try
Instance 	*instance = 0;
	instance = p->getInstance("Saw a try expression");
	instance->isComment = 1;
	line->add(instance);
	statement->value = (void*)line;
	return 1;
}

int StatementBody19TawkNow(PLGitem *iTEM)
{
PLGitem 	*statement = iTEM->get("statement");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
PLGitem 	*declare = statement->get("declare");
PLGitem 	*entry = 0;
Statement 	*line = 0;
Instance 	*stacked = 0;
Instance 	*instance = 0;
Expression 	*express = 0;
	statement->runDeferred();
	// declaration
	p->virtualStack->clear();
	for ( entry = declare; entry; entry = entry->next )
		{
		instance = (Instance*)entry->value;
		if ( !instance || instance->type )
			continue;
		if ( instance->symbol && instance->symbol->type->hasInitializer && entry->get("initialize") )
			{
			Expression 	*initialize = 0;
			/*************************************************************
			The instance being declared does not have an initializing
			expression but its class has an initializer method
			(specified as an initializer in an external type declaration).
			Add the call to the initializer here.
			*************************************************************/
			Instance 	*assigned = p->findInitializer(instance->symbol);
			if ( assigned )
				{
				Instance 	*argument = p->getInstance(instance->symbol->name);
				argument->isConstant = 1;
				argument->indirection = 1;
				argument->type = SymbolType::stringType;
				assigned->addParameter(argument);
				if ( assigned->symbol->isDefault )
					assigned->setDefaults(p);
				initialize = new Expression(instance,assigned,"=");
				instance = new Instance(initialize);
				}
			}
		if ( instance->symbol )
			{
			p->virtualStack->push(instance);
			instance->isLocal = 1;
			}
		else
		if ( instance->express )
			if ( instance->isRange )
				p->virtualStack->push(instance);
			else {
				Instance 	*temp = instance->express->subject;
				p->virtualStack->push(temp);
				temp->isLocal = 1;
				}
		if ( express )
			{
			express->verb = p->commaOp;
			express->object = instance;
			if ( instance->symbol )
				instance->isDeclaration = 0;
			if ( instance->express )
				instance->express->subject->isDeclaration = 0;
			instance = new Instance(express);
			}
		if ( entry->next )
			{
			express = new Expression();
			express->subject = instance;
			}
		}
	if ( p->virtualStack->length )
		{
		p->virtualStack->entry = 0;
		while ( stacked = (Instance*)p->virtualStack->next() )
			p->currentSymbols->add(stacked);
		p->virtualStack->clear();
		}
	if ( !instance->isRange )
		{
		instance->isDeclaration = 1;
		line = new Statement();
		line->add(instance);
		line->indented = 0;
		statement->value = (void*)line;
		}
	return 1;
}

int StatementBody20TawkNow(PLGitem *iTEM)
{
PLGitem 	*statement = iTEM->get("statement");
PLGitem 	*item = statement->get("instance");
Statement 	*line = new Statement();
Instance 	*instance = (Instance*)item->value;
	line->add(instance);
	// expression
	statement->value = (void*)line;
	return 1;
}

int StatementBody21TawkNow(PLGitem *iTEM)
{
PLGitem 	*statement = iTEM->get("statement");
PLGitem 	*item = statement->get("function");
Statement 	*line = 0;
	// Lambda
Instance 	*lambda = (Instance*)item->value;
	if ( lambda->statement )
		statement->value = (void*)lambda->statement;
	else {
		line = new Statement();
		statement->value = (void*)line;
		line->add(lambda);
		}
	return 1;
}

int StatementBody2TawkNow(PLGitem *iTEM)
{
PLGitem 	*statement = iTEM->get("statement");
PLGitem 	*item = statement->get("start");
BlockTok 	*block = (BlockTok*)item->value;
Statement 	*line = new Statement();
	line->add(block);
	// Block
	statement->value = (void*)line;
	return 1;
}

int StatementBody3TawkNow(PLGitem *iTEM)
{
PLGitem 	*statement = iTEM->get("statement");
PLGitem 	*action = statement->get("action");
PLGitem 	*instance = statement->get("instance");
PLGitem 	*item = action->get("statement");
PLGitem 	*otherwise = statement->get("otherwise");
Instance 	*express = (Instance*)instance->value;
Statement 	*ifStatement = new Statement(IF);
Statement 	*line = (Statement*)item->value;
	// if
	ifStatement->add(express);
	express->isCondition = 1;
	line->indented = 1;
	if ( line )
		ifStatement->add(line);
	if ( otherwise )
		{
		item = otherwise->get("statement");
		line = (Statement*)item->value;
		if ( line )
			ifStatement->add(line);
		}
	ifStatement->pointInCode = iTEM;
	statement->value = (void*)ifStatement;
	return 1;
}

int StatementBody4TawkNow(PLGitem *iTEM)
{
PLGitem 	*statement = iTEM->get("statement");
PLGitem 	*instance = iTEM->get("instance");
Instance 	*returnValue = 0;
Statement 	*line = new Statement(RETURN);
	// return
	if ( instance )
		{
		returnValue = (Instance*)instance->value;
		line->add(returnValue);
		}
	line->branch = 1;
	statement->value = (void*)line;
	return 1;
}

int StatementBody5TawkNow(PLGitem *iTEM)
{
PLGitem 	*instance = iTEM->get("instance");
PLGitem 	*statement = iTEM->get("statement");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
Statement 	*forStatement = (Statement*)instance->value;
Statement 	*body = 0;
	body = (Statement*)statement->value;
	body->indented = 1;
	forStatement->fourth = new Instance(body);
	// for
	forStatement->pointInCode = iTEM;
	statement->value = (void*)forStatement;
	p->iterating--;
	return 1;
}

int StatementBody6TawkNow(PLGitem *iTEM)
{
PLGitem 	*statement = iTEM->get("statement");
PLGitem 	*item = statement->get("start");
Statement 	*line = new Statement();
Instance 	*instance = (Instance*)item->value;
	line->add(instance);
	// Print
	if ( instance->block )
		line->indented = 0;
	statement->value = (void*)line;
	return 1;
}

int StatementBody7TawkNow(PLGitem *iTEM)
{
PLGitem 	*instance = iTEM->get("instance");
PLGitem 	*statement = iTEM->get("statement");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
Statement 	*wile = new Statement(WHILE);
Instance 	*test = (Instance*)instance->value;
Statement 	*body = (Statement*)statement->value;
	// while
	wile->add(test);
	test->isCondition = 1;
	body->indented = 1;
	wile->add(body);
	wile->pointInCode = iTEM;
	statement->value = (void*)wile;
	p->iterating--;
	return 1;
}

int StatementBody8TawkNow(PLGitem *iTEM)
{
PLGitem 	*statement = iTEM->get("statement");
PLGitem 	*item = statement->get("instance");
Instance 	*instance = (Instance*)item->value;
Statement 	*line = new Statement(LABEL);
	line->add(instance);
	// label or case
	if ( !instance->prefix )
		line->indented = 0;
	statement->value = (void*)line;
	return 1;
}

int StatementBody9TawkNow(PLGitem *iTEM)
{
PLGitem 	*statement = iTEM->get("statement");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
Instance 	*instance = p->getInstance("break");
Statement 	*line = new Statement();
PLGitem 	*lastSwitch = 0;
	line->add(instance);
	// break
	line->branch = 1;
	statement->value = (void*)line;
	// if break is in a switch case, set switch fall thru status
	if ( p->switchStack )
		lastSwitch = (PLGitem*)p->switchStack->top();
	if ( lastSwitch && p->iterating == lastSwitch->itemLength )
		lastSwitch->flag1 = 1;
	return 1;
}

int StatementBodyTawkNow(PLGitem *iTEM)
{
PLGitem 	*statement = iTEM->get("statement");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
Statement 	*line = new Statement();
PLGitem 	*comment = statement->get("comment");
Instance 	*instance = 0;
	if ( comment )
		instance = p->getInstance(comment->toString());
	else	instance = p->getInstance(statement->toString());
	instance->isComment = 1;
	line->add(instance);
	statement->value = (void*)line;
	return 1;
}

int StopTawkNow(PLGitem *iTEM)
{
	// This is a dummy rule to stick wherever for debugging
	return 1;
}

int StringExpression2TawkNow(PLGitem *iTEM)
{
PLGitem 	*instance = iTEM->get("instance");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
Instance 	*item = (Instance*)instance->value;
Instance 	*target = 0;
SymbolType 	*type = item->getType();
	target = item->getSubject();
	if ( type != SymbolType::stringType )
		{
		target = p->convertToString(item);
		if ( !target )
			return 0;
		instance->value = (void*)target;
		}
	else
	if ( !(target->isConstant || item->howDirect() == 1) )
		return 0;
	else
	if ( item->express && item->express->verb && !(item->express->verb->pointing || item->express->verb->assign) )
		return 0;
	return 1;
}

int StringsTawkNow(PLGitem *iTEM)
{
PLGitem 	*item = iTEM->get("item");
PLGitem 	*instance = 0;
	for ( ; item; item = item->next )
		{
		instance = item->get("instance");
		item->value = instance->value;
		}
	return 1;
}

void StructureBody2TawkAct(PLGitem *iTEM)
{
PLGitem 	*entry = iTEM->get("entry");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
Symbol 		*symbol = 0;
PLGitem 	*name = entry->find("name");
PLGitem 	*item = 0;
	p->saveStruct = p->currentClass;
	if ( p->currentClass )
		{
		symbol = p->currentClass->getLocal(name->string());
		name->unString();
		}
	if ( symbol )
		p->setCurrentClass(symbol->structType);
	else {
		char 	*typeName = ::concat(4,p->currentClass->name,"Struct",::toStringFromInt(p->stringNumber++),"Type");
		p->setCurrentClass(SymbolType::getType(typeName));
		}
	if ( p->currentClass )
		p->currentClass->nameLess = 1;
	for ( item = entry; item; item = item->next )
		{
		item->runDeferred();
		name = item->get("name");
		if ( name )
			item->value = name->value;
		}
	p->setCurrentType(p->currentClass);
	p->setCurrentClass(p->saveStruct);
	p->saveStruct = 0;
}

void StructureBodyTawkAct(PLGitem *iTEM)
{
PLGitem 	*label = iTEM->get("label");
PLGitem 	*entry = iTEM->get("entry");
PLGitem 	*field = iTEM->get("field");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
Instance 	*current = 0;
SymbolType 	*saveClass = p->currentClass;
PLGitem 	*item = 0;
PLGitem 	*name = 0;
	item = label->get("type");
	if ( !item->value )
		{
		label->runDeferred();
		item = label->get("type");
		}
	p->setCurrentClass((SymbolType*)item->value);
	for ( item = entry; item; item = item->next )
		{
		item->runDeferred();
		name = item->get("name");
		if ( name )
			item->value = name->value;
		}
	p->setCurrentType(p->currentClass);
	p->currentType->isDirect = 1;
	for ( ; field; field = field->next )
		{
		field->runDeferred();
		item = field->get("item");
		current = (Instance*)item->value;
		field->value = (void*)current;
		}
	current = new Instance(p->currentClass);
	p->setCurrentClass(saveClass);
	label->value = (void*)current;
}

void StructureItem2TawkAct(PLGitem *iTEM)
{
PLGitem 	*name = iTEM->get("name");
PLGitem 	*bits = iTEM->get("bits");
PLGitem 	*buttons = iTEM->get("buttons");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
char 		*text = name->toString();
Symbol 		*aliasSymbol = 0;
Symbol 		*symbol = 0;
Instance 	*instance = 0;
SymbolType 	*symbolType = 0;
	symbolType = SymbolType::getType("unsigned int");
	symbol = p->currentClass->getLocal(text);
	if ( !symbol )
		{
		symbol = new Symbol(text,symbolType);
		symbol->structType = p->currentClass;
		if ( bits )
			{
			PLGitem 	*length = bits->get("length");
			symbol->array = bits->toString();
			symbol->symbolBitLength = ::atoi(length->string());
			length->unString();
			}
		else	symbol->symbolBitLength = 1;
		symbol->isItem = 1;
		p->currentClass->add(symbol);
		// saveStruct, if set, is the class containing the structure
		if ( p->saveStruct )
			{
			aliasSymbol = new Symbol(symbol);
			aliasSymbol->isHidden = 1;
			aliasSymbol->isItem = 1;
			p->saveStruct->add(aliasSymbol);
			}
		}
	else {
		if ( bits )
			symbol->array = bits->toString();
		symbol->isItem = 1;
		}
	instance = new Instance(symbol);
	name->value = (void*)instance;
	if ( buttons )
		{
		PLGitem 	*button = 0;
		buttons->runDeferred();
		button = buttons->get("button");
		for ( ; button; button = button->next )
			{
			// for button items symbol.source is set to the button container symbol
			symbol = (Symbol*)button->value;
			symbol->isButton = 1;
			symbol->source = instance->symbol;
			if ( p->saveStruct )
				{
				aliasSymbol = new Symbol(symbol);
				p->saveStruct->add(aliasSymbol);
				}
			}
		}
}

void StructureItemTawkAct(PLGitem *iTEM)
{
PLGitem 	*item = iTEM->get("item");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
PLGitem 	*declare = item->get("declare");
PLGitem 	*entry = 0;
Instance 	*instance = 0;
Symbol 		*aliasSymbol = 0;
	item->runDeferred();
	for ( entry = declare; entry; entry = entry->next )
		{
		instance = (Instance*)entry->value;
		if ( instance )
			{
			p->currentClass->add(instance->symbol);
			instance->symbol->structType = p->currentClass;
			if ( p->saveStruct )
				{
				aliasSymbol = new Symbol(instance->symbol);
				aliasSymbol->isHidden = 1;
				aliasSymbol->isItem = 1;
				p->saveStruct->add(aliasSymbol);
				}
			}
		}
}

void StructureTawkAct(PLGitem *iTEM)
{
PLGitem 	*kind = iTEM->get("kind");
PLGitem 	*body = iTEM->get("body");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
Symbol 		*symbol = 0;
Instance 	*instance = 0;
Instance 	*current = 0;
SymbolType 	*structureType = 0;
PLGitem 	*item = body->get("label");
PLGitem 	*field = body->get("field");
PLGitem 	*entry = body->get("entry");
char 		*error = 0;
	p->setCurrentType((SymbolType*)0);
	p->newType = p->methodType = 0;
	if ( !item && kind->compare("typedef") == 0 )
		{
		PLGitem 	*name = entry->get("name");
		structureType = SymbolType::find(name->string());
		if ( structureType && !isType(structureType->structure) )
			{
			error = ::concat(3,"typedef ",name->string()," conflicts with existing class name\n");
			instance = p->makeError(error);
			}
		else {
			if ( !structureType )
				{
				structureType = SymbolType::getType(name->toString());
				structureType->structure = 4;
				structureType->isAtomic = 1;
				}
			instance = new Instance(structureType);
			}
		name->unString();
		if ( p->currentClass->noDotH )
			structureType->noDotH = 1;
		else	structureType->dotHname = p->currentClass->dotHname;
		goto finish;
		}
	p->processingParameters = 1;
	body->runDeferred();
	p->processingParameters = 0;
	structureType = p->currentType;
	structureType->noDotH = 1;
	if ( !p->currentClass )
		p->setCurrentClass(p->currentType);
	if ( !item )
		{
		char 	*name = ::headToString(structureType->name,"Type");
		symbol = p->currentClass->getLocal(name);
		if ( !symbol )
			{
			symbol = new Symbol(name,structureType);
			p->currentClass->add(symbol);
			symbol->isHidden = 1;
			//	Note here, isHidden has to be set after adding
			}
		instance = new Instance(symbol);
		}
	else {
		instance = (Instance*)item->value;
		structureType = instance->type;
		if ( field )
			body->next = field;
		else
		if ( !p->currentClass->isExternal )
			structureType->mustDeclare = 1;
		}
	if ( (kind->compare("boolean") == 0) )
		{
		structureType->structure = 1;
		structureType->isNumber = 1;
		}
	else
	if ( (kind->compare("enumerator") == 0) )
		{
		structureType->structure = 2;
		structureType->isNumber = 1;
		SymbolType::globalList->add((void*)structureType);
		}
	else
	if ( (kind->compare("struct") == 0) )
		structureType->structure = 5;
	else
	if ( (kind->compare("typedef") == 0) )
		structureType->structure = 4;
	else	structureType->structure = 6;
	if ( isBoolean(structureType->structure) || isEnumerator(structureType->structure) )
		for ( item = entry; item; item = item->next )
			{
			current = (Instance*)item->value;
			symbol = current->symbol;
			if ( isEnumerator(structureType->structure) )
				{
				symbol->type = SymbolType::nullType;
				p->currentSymbols->addGlobalField(symbol->name,symbol);
				}
			else
			if ( isBoolean(structureType->structure) )
				if ( !symbol->array )
					symbol->array = ":1";
			}
finish:
	structureType->isExternal = 0;
	structureType->isDirect = 1;
	if ( p->currentClass != SymbolType::globalType )
		structureType->setParent(p->currentClass);
	kind->value = (void*)structureType;
	body->value = (void*)instance;
	p->setCurrentType((SymbolType*)0);
}

void StructureType2TawkAct(PLGitem *iTEM)
{
PLGitem 	*type = iTEM->get("type");
SymbolType 	*symbolType = 0;
char 		*name = type->string();
	symbolType = SymbolType::find(name);
	type->unString();
	if ( !symbolType )
		symbolType = SymbolType::getType(type->toString());
	type->value = (void*)symbolType;
}

int SwitchTawkNow(PLGitem *iTEM)
{
PLGitem 	*statement = iTEM->get("statement");
PLGitem 	*name = iTEM->get("name");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
	// overriding statement length (not otherwise used).
	statement->itemLength = p->iterating;
	if ( name )
		statement->value = name->value;
	if ( !p->switchStack )
		p->switchStack = new Stak();
	p->switchStack->push(statement);
	return 1;
}

int SyntaxExtensions2TawkNow(PLGitem *iTEM)
{
Tawk 	*p = (Tawk*)iTEM->test->testParser;
Symbol 	*symbol = 0;
	if ( p->currentClass->aliasStack && p->currentClass->aliasStack->length )
		{
		p->currentClass->aliasStack->entry = 0;
		while ( symbol = (Symbol*)p->currentClass->aliasStack->next() )
			p->currentClass->add(symbol);
		p->currentClass->aliasStack->clear();
		}
	return 1;
}

int Target2TawkNow(PLGitem *iTEM)
{
PLGitem 	*field = iTEM->get("field");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
PLGitem 	*name = field->get("name");
Instance 	*instance = p->getInstance(name->toString());
	instance->type = 0;
	field->value = (void*)instance;
	return 1;
}

int TargetMethodTawkNow(PLGitem *iTEM)
{
PLGitem 	*target = iTEM->get("target");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
Symbol 		*symbol = 0;
PLGitem 	*name = target->get("name");
	if ( p->currentClass->methods )
		{
		symbol = (Symbol*)p->currentClass->methods->get(name->string());
		name->unString();
		}
	if ( symbol )
		target->value = (void*)symbol;
	else	return 0;
	return 1;
}

int TargetTawkNow(PLGitem *iTEM)
{
PLGitem 	*field = iTEM->get("field");
Instance 	*instance = (Instance*)field->value;
	if ( instance->isError )
		return 0;
	if ( instance->isConstant )
		instance->type = 0;
	return 1;
}

int TypeListTawkNow(PLGitem *iTEM)
{
PLGitem 	*instance = iTEM->get("instance");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
Instance 	*item = new Instance(p->currentType);
	instance->value = (void*)item;
	p->setCurrentType((SymbolType*)0);
	return 1;
}

int TypeNameTawkNow(PLGitem *iTEM)
{
PLGitem 	*type = iTEM->get("type");
SymbolType 	*symbolType = 0;
	symbolType = SymbolType::find(type->string());
	type->unString();
	if ( symbolType )
		{
		type->value = (void*)symbolType;
		return 1;
		}
	return 0;
	return 1;
}

int TypeTawkNow(PLGitem *iTEM)
{
PLGitem 	*hasConst = iTEM->get("hasConst");
PLGitem 	*noSign = iTEM->get("noSign");
PLGitem 	*type = iTEM->get("type");
PLGitem 	*temp = iTEM->get("temp");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
SymbolType 	*symbolType = 0;
char 		*name = 0;
	if ( !noSign && !temp )
		symbolType = (SymbolType*)type->value;
	else {
		if ( temp )
			type->itemLength += temp->itemLength;
		name = type->string();
		if ( noSign )
			name = ::concat(2,"unsigned ",name);
		symbolType = SymbolType::find(name);
		type->unString();
		}
	if ( !symbolType )
		return 0;
	if ( hasConst )
		type->flag4 = 1;
	type->value = (void*)symbolType;
	p->setCurrentType(symbolType);
	return 1;
}

int UnaryExpression2TawkNow(PLGitem *iTEM)
{
PLGitem 	*operate = iTEM->get("operate");
PLGitem 	*cast = iTEM->get("cast");
PLGitem 	*instance = iTEM->get("instance");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
Expression 	*expression = 0;
PLGitem 	*item = 0;
Instance 	*castInstance = 0;
Instance 	*subject = 0;
SymbolType 	*virtualType = 0;
	subject = (Instance*)instance->value;
	if ( operate )
		if ( operate->compare("&") == 0 && !operate->next )
			subject->setReference((unsigned int)1);
		else {
			expression = new Expression((Instance*)0,subject,operate->toString());
			subject = new Instance(expression);
			}
	if ( cast )
		{
		item = cast->get("type");
		castInstance = (Instance*)item->value;
		if ( !subject->express )
			{
			expression = new Expression((Instance*)0,subject,(char*)0);
			subject = new Instance(expression);
			}
		else	subject = new Instance(subject);
		subject->cast = castInstance;
		}
	virtualType = subject->getType();
	if ( virtualType->overloads )
		p->virtualItem = subject;
	instance->value = (void*)subject;
	p->setCurrentType(virtualType);
	return 1;
}

int UnaryExpressionTawkNow(PLGitem *iTEM)
{
PLGitem 	*operate = iTEM->get("operate");
PLGitem 	*instance = iTEM->get("instance");
Tawk 		*p = (Tawk*)iTEM->test->testParser;
Instance 	*subject = 0;
Expression 	*expression = 0;
PLGitem 	*list = instance->get("list");
	instance->runDeferred();
	subject = (Instance*)list->value;
	if ( operate )
		{
		expression = new Expression((Instance*)0,subject,operate->toString());
		subject = new Instance(expression);
		}
	instance->value = (void*)subject;
	p->setCurrentType((SymbolType*)0);
	return 1;
}

int UnaryOperator2TawkNow(PLGitem *iTEM)
{
PLGitem 	*operate = iTEM->get("operate");
	if ( operate->next )
		return 0;
	return 1;
}

void assignFailed(PLGtester *t)
{
Tawk 	*tOK = (Tawk*)t->testParser;
	if ( tOK->assigning )
		{
		tOK->assigning = 0;
		::printf("Assigning turned off\n");
		}
}

void caseLabelFail(PLGtester *t)
{
Tawk 	*tOK = (Tawk*)t->testParser;
	tOK->assuming = 0;
}

void expressPartFailed(PLGtester *t)
{
Tawk 	*tOK = (Tawk*)t->testParser;
	tOK->popVirtuals();
}

void instanceTailFail(PLGtester *t)
{
Tawk 	*tOK = (Tawk*)t->testParser;
	tOK->noShortcuts = 0;
	t->printErrorMessage();
}

/*****************************************************************************
	Constructor
*****************************************************************************/
Tawk::Tawk()
{
	currentClass = 0;
	currentType = 0;
	expressType = 0;
	methodType = 0;
	newType = 0;
	currentMethod = 0;
	lambdaMethod = 0;
	componentCount = 0;
	iterating = 0;
	stringNumber = 0;
	currentBlock = 0;
	macroHash = 0;
	macroList = 0;
	defaultPrinter = 0;
	debugInstance = 0;
	virtualItem = 0;
	commaOp = 0;
	currentComment = 0;
	virtualOp = 0;
	switchStack = 0;
	saveStruct = 0;
	saveType = 0;
	falseSymbol = 0;
	nullSymbol = 0;
	trueSymbol = 0;
	directivesFile = 0;
	macroDelimiter = 0;
	assigning = 0;
	assuming = 0;
	constDeclare = 0;
	debugging = 0;
	declaringMethod = 0;
	declaredSomething = 0;
	extending = 0;
	isQualified = 0;
	linkage = 0;
	noLoop = 0;
	noShortcuts = 0;
	parsingDirective = 0;
	processingParameters = 0;
	produceCodeFile = 0;
	referring = 0;
	stringing = 0;
	externalENV = (void*)0;
	alphaSet = 0;
	nameStartSet = 0;
	commentSet = 0;
	compareSet = 0;
	compareFollow = 0;
	logicSet = 0;
	methodSet = 0;
	methodNameSet = 0;
	nameSet = 0;
	operatorSet = 0;
	space = 0;
	rangeSet = 0;
	stringOP = 0;
	textFollow = 0;
	typesSet = 0;
	Attributes = 0;
	Conditions = 0;
	ReservedWord = 0;
	Comparisons = 0;
	Directives = 0;
	Linkage = 0;
	Operators = 0;
	Ranges = 0;
	State = 0;
	Structures = 0;
	singleQuote = 0;
	// WTF?
	SymbolType::types = new Types();
	blockStack = new Stak();
	virtualStack = new Stak();
	tokJunkBuffer = ::bufferFactory3("tokJunk",1000);
	currentSymbols = new InstanceTable();
	formatter = new FormatC();
	lambdaSet = new PLGset();
	lambdaSet->set((int)'^');
	includedFiles = new BaseHash();
	missingMethods = new BaseHash();
	mainParser = (void*)this;
}

/*******************************************************************************
        Check to see if assignment is an implied conversion
*******************************************************************************/
void Tawk::checkConversion(Expression *express)
{
SymbolType 	*subjectType = express->subject->getType();
SymbolType 	*objectType = express->object->getType();
Instance 	*instance = 0;
int 		subjectDirect = express->subject->howDirect();
int 		objectDirect = express->object->howDirect();
	if ( subjectType == objectType || (express->subject->arrayRef && subjectType->overloads) || subjectDirect > 1 || objectDirect > 1 || (subjectDirect == objectDirect && ((subjectType == SymbolType::stringType || subjectType == SymbolType::charType) && (objectType == SymbolType::charType || objectType == SymbolType::stringType))) )
		return;
	if ( subjectType != SymbolType::voidType && objectType != SymbolType::voidType )
		{
		instance = currentSymbols->getConverter(subjectType,objectType);
		if ( instance )
			{
			if ( instance->symbol->parameters )
				instance->addParameter(express->object);
			else	instance->setParent(express->object);
			express->object = instance;
			return;
			}
		}
}

/*****************************************************************************
	item is a chain of strings. Build and return an instance that
	concatenates those strings into a single item
*****************************************************************************/
Instance *Tawk::concatenate(PLGitem *source)
{
PLGitem 	*item = source;
Instance 	*instance = 0;
Instance 	*parameter = 0;
char 		*text = 0;
int 		count = 0;
	if ( !source->next )
		return (Instance*)source->value;
	/*************************************************************************
	If all items to be concatenated are literals just glom them
	together and go home
	*************************************************************************/
	for ( ; item; item = item->next )
		{
		parameter = (Instance*)item->value;
		if ( parameter )
			{
			if ( !parameter->isConstant || !parameter->prefix )
				break;
			count += (int)::strlen(parameter->prefix);
			}
		else	break;
		}
	if ( !item )
		{
		text = (char*)::malloc(count + 1);
		*text = 0;
		for ( item = source; item; item = item->next )
			{
			parameter = (Instance*)item->value;
			::strcat(text,parameter->prefix);
			}
		instance = getInstance(text);
		instance->isConstant = 1;
		instance->type = SymbolType::stringType;
		return instance;
		}
	count = 0;
	/*************************************************************************
	Otherwise generate a concat method to join them
	*************************************************************************/
	instance = currentSymbols->find("concat");
	// Make sure this is a new instance
	instance = new Instance(instance->symbol);
	for ( item = source; item; item = item->next )
		count++;
	::asprintf(&text,"%d",count);
	parameter = getInstance(text);
	parameter->type = SymbolType::intType;
	instance->addParameter(parameter);
	for ( item = source; item; item = item->next )
		{
		parameter = (Instance*)item->value;
		instance->addParameter(parameter);
		}
	source->next = 0;
	SymbolType::stringRoutines->setRefer();
	return instance;
}

/*******************************************************************************
        Convert a range expression into a valid expression.
*******************************************************************************/
Expression *Tawk::convertRangeX(Instance *field, Instance *rangeField)
{
Expression 	*rangeX = 0;
char 		*frontVerb = 0;
char 		*backVerb = 0;
Instance 	*front = rangeField->express->subject;
Instance 	*back = rangeField->express->object;
	if ( *rangeField->express->verb->op == '>' )
		frontVerb = ">";
	else	frontVerb = ">=";
	if ( *(rangeField->express->verb->op + 1) == '<' )
		backVerb = "<";
	else	backVerb = "<=";
	rangeX = new Expression(field,front,frontVerb);
	front = new Instance(rangeX);
	rangeX = new Expression(field,back,backVerb);
	back = new Instance(rangeX);
	rangeX = new Expression(front,back,"&&");
	return rangeX;
}

/*******************************************************************************
        Convert the instance passed in to a string if there is a toString
        routine that does so.
*******************************************************************************/
Instance *Tawk::convertToString(Instance *source)
{
SymbolType 	*type = 0;
Instance 	*instance = 0;
	type = source->getType();
	if ( type == SymbolType::stringType )
		return source;
	if ( type->isNumber && !source->howDirect() )
		{
		instance = getInstance("toString");
		instance->addParameter(source);
		instance->isMethod = 1;
		// have to addParameter because dealing with a new instance
		if ( instance = currentSymbols->findMethod(instance) )
			instance->addParameter(source);
		else	::fprintf(stderr,"convertToString: failed\n");
		}
	else
	if ( !type->isAtomic )
		{
		Symbol 	*target = type->getMethod("toString()");
		if ( target )
			{
			instance = new Instance(target);
			instance->setParent(source);
			}
		}
	return instance;
}

/*****************************************************************************
	Extract the current comment
*****************************************************************************/
char *Tawk::extractComment()
{
PLGitem 	*item = currentComment;
	tokJunkBuffer->reset();
	for ( ; item; item = item->next )
		{
		tokJunkBuffer->appendString(item->string());
		tokJunkBuffer->appendString("\n");
		item->unString();
		}
	currentComment = 0;
	if ( tokJunkBuffer->length() )
		return tokJunkBuffer->toString();
	return 0;
}

/*******************************************************************************
        Find a qualified initializer matching the method passed in and returns
        a copy w/no parameters (the parameter will be added in the declaration).
*******************************************************************************/
Instance *Tawk::findInitializer(Symbol *symbol)
{
Symbol 		*method = symbol->type->initializer;
Instance 	*instance = 0;
	if ( method )
		if ( instance = currentSymbols->find(method->methodName) )
			if ( instance->isMethod && instance->symbol->isInitializer )
				return instance;
	return 0;
}

/*****************************************************************************
	Generate a print statement to print the passed in instance
*****************************************************************************/
Instance *Tawk::generatePrint(Instance *argument)
{
Instance 	*instance = 0;
char 		*printMethod = 0;
SymbolType 	*poType = 0;
Symbol 		*symbol = 0;
char 		*text = 0;
	if ( defaultPrinter->isMethod )
		poType = defaultPrinter->symbol->parentClass;
	else	poType = defaultPrinter->getType();
	if ( defaultPrinter->isMethod )
		{
		/*********************************************************************
		Use whatever method name is provided and try to match argument type
		*********************************************************************/
		instance = getInstance(defaultPrinter->symbol->name);
		instance->isMethod = 1;
		instance->addParameter(argument);
		text = instance->mangle();
		symbol = poType->findField(text);
		}
	else {
		/*********************************************************************
		Try to find a method named append that matches the argument type
		*********************************************************************/
		instance = getInstance("append");
		instance->isMethod = 1;
		instance->addParameter(argument);
		if ( argument->format )
			{
			instance->addParameter(argument->format);
			// The following if it exists is the format width
			if ( argument->format->format )
				instance->addParameter(argument->format->format);
			}
		instance->setParent(defaultPrinter);
		text = instance->mangle();
		symbol = poType->findField(text);
		}
	if ( !symbol )
		{
		printMethod = "append(char*)";
		if ( symbol = poType->getMethod(printMethod) )
			{
			instance = convertToString(argument);
			if ( !instance )
				{
				text = ::concat(2,"ERROR: Could not find toString method to print ",argument->toString());
				instance = getInstance(text);
				}
			else	return generatePrint(instance);
			}
		else {
			text = ::concat(2,"ERROR: Could not find method to print ",argument->toString());
			instance = getInstance(text);
			}
		}
	else {
		instance->symbol = symbol;
		instance->prefix = 0;
		if ( symbol->isDefault )
			instance->setDefaults(this);
		}
	return instance;
}

/*******************************************************************************
        Return a new Instance from a String making sure the String is not
        a key word.
*******************************************************************************/
Instance *Tawk::getInstance(char *text)
{
Instance 	*instance = new Instance(text);
	if ( ReservedWord )
		if ( ReservedWord->find(text) )
			instance->type = 0;
	return instance;
}

void Tawk::initializeKeyWords()
{
	Attributes = new KeyTable("Attributes");
	Attributes->add("C");
	Attributes->add("isChar");
	Attributes->add("isNumber");
	Attributes->add("local");
	Attributes->add("no.h");
	Attributes->add("noClassForward");
	Attributes->add("OC");
	Attributes->add("proper");
	Attributes->add("protocol");
	Attributes->add("type");
	Attributes->add("addClassNameToMethods");
	Conditions = new KeyTable("Conditions");
	ReservedWord = new KeyTable("ReservedWord");
	ReservedWord->add("boolean");
	ReservedWord->add("break");
	ReservedWord->add("case");
	ReservedWord->add("catch");
	ReservedWord->add("cerr");
	ReservedWord->add("char");
	ReservedWord->add("continue");
	ReservedWord->add("cout");
	ReservedWord->add("default");
	ReservedWord->add("delete");
	ReservedWord->add("do");
	ReservedWord->add("double");
	ReservedWord->add("else");
	ReservedWord->add("enumerator");
	ReservedWord->add("extends");
	ReservedWord->add("external");
	ReservedWord->add("false");
	ReservedWord->add("finally");
	ReservedWord->add("float");
	ReservedWord->add("for");
	ReservedWord->add("goto");
	ReservedWord->add("if");
	ReservedWord->add("int");
	ReservedWord->add("long");
	ReservedWord->add("new");
	ReservedWord->add("null");
	ReservedWord->add("or");
	ReservedWord->add("print");
	ReservedWord->add("protocol");
	ReservedWord->add("return");
	ReservedWord->add("short");
	ReservedWord->add("sizeof");
	ReservedWord->add("static");
	ReservedWord->add("String");
	ReservedWord->add("struct");
	ReservedWord->add("switch");
	ReservedWord->add("throw");
	ReservedWord->add("to");
	ReservedWord->add("true");
	ReservedWord->add("try");
	ReservedWord->add("union");
	ReservedWord->add("unsigned");
	ReservedWord->add("use");
	ReservedWord->add("void");
	ReservedWord->add("while");
	ReservedWord->add("gt");
	ReservedWord->add("ge");
	ReservedWord->add("lt");
	ReservedWord->add("le");
	ReservedWord->add("eq");
	ReservedWord->add("ne");
	Comparisons = new KeyTable("Comparisons");
	Comparisons->add("gt");
	Comparisons->add("ge");
	Comparisons->add("lt");
	Comparisons->add("le");
	Comparisons->add("eq");
	Comparisons->add("ne");
	Directives = new KeyTable("Directives");
	Directives->add("before");
	Directives->add("ending");
	Directives->add("starting");
	Directives->add("within");
	Linkage = new KeyTable("Linkage");
	Linkage->add("const");
	Linkage->add("extern");
	Linkage->add("inline");
	Linkage->add("static");
	Linkage->add("virtual");
	Operators = new KeyTable("Operators");
	Ranges = new KeyTable("Ranges");
	Ranges->add("..");
	Ranges->add(">.");
	Ranges->add(".<");
	Ranges->add("><");
	State = new KeyTable("State");
	State->add("autoGetSet");
	State->add("dEBUG");
	State->add("dUMP");
	State->add("initNOT");
	State->add("mARK");
	State->add("resetPRINT");
	State->add("sUMMARY");
	State->add("tEST");
	Structures = new KeyTable("Structures");
	Structures->add("boolean");
	Structures->add("enumerator");
	Structures->add("struct");
	Structures->add("typedef");
	Structures->add("union");
}

void Tawk::initializeSetTable()
{
	setTable->add("alphaSet",(void*)alphaSet);
	setTable->add("nameStartSet",(void*)nameStartSet);
	setTable->add("commentSet",(void*)commentSet);
	setTable->add("compareSet",(void*)compareSet);
	setTable->add("compareFollow",(void*)compareFollow);
	setTable->add("logicSet",(void*)logicSet);
	setTable->add("methodSet",(void*)methodSet);
	setTable->add("methodNameSet",(void*)methodNameSet);
	setTable->add("nameSet",(void*)nameSet);
	setTable->add("operatorSet",(void*)operatorSet);
	setTable->add("space",(void*)space);
	setTable->add("rangeSet",(void*)rangeSet);
	setTable->add("singleQuote",(void*)singleQuote);
	setTable->add("stringOP",(void*)stringOP);
	setTable->add("textFollow",(void*)textFollow);
	setTable->add("typesSet",(void*)typesSet);
}

/*******************************************************************************
        Create an error instance
*******************************************************************************/
Instance *Tawk::makeError(char *text)
{
Instance 	*instance = new Instance();
	instance->error(text);
	return instance;
}

/*****************************************************************************
	Build an expression out of its parts (recursive)
*****************************************************************************/
Expression *Tawk::makeExpress(Instance *instance, PLGitem *item)
{
PLGitem 	*expressItem = 0;
PLGitem 	*operate = 0;
Expression 	*expression = 0;
Instance 	*primary = 0;
Instance 	*secondary = 0;
SymbolType 	*type = 0;
	if ( !item )
		{
		expression = new Expression(instance,0,(char*)0);
		return expression;
		}
	operate = item->get("operate");
	expressItem = item->get("instance");
	if ( item->next )
		{
		primary = (Instance*)expressItem->value;
		expression = makeExpress(primary,item->next);
		secondary = new Instance(expression);
		}
	else {
		secondary = (Instance*)expressItem->value;
		if ( secondary->resolved )
			{
			expression = secondary->express;
			if ( !expression )
				expression = new Expression(secondary,0,(char*)0);
			}
		}
	/*************************************************************************
	Resolve []= overloads
	*************************************************************************/
	if ( instance->resolved && instance->parameters && operate->compare("=") == 0 && (type = instance->getType()) && type->isVirtuous )
		{
		char 	*text = type->overloaded("[]=");
		if ( text )
			{
			Symbol 	*symbol = instance->symbol;
			instance->symbol = 0;
			instance->prefix = text;
			instance->addParameter(secondary);
			text = instance->mangle();
			if ( instance->symbol = type->getMethod(text) )
				instance->prefix = 0;
			else {
				instance->prefix = 0;
				instance->symbol = symbol;
				instance->parameters->last->remove();
				}
			expression = new Expression(instance,0,(char*)0);
			goto bail;
			}
		}
	expression = new Expression(instance,secondary,operate->toString());
bail:
	return expression;
}

/*******************************************************************************
        Pop off current virtualOp
*******************************************************************************/
void Tawk::popVirtuals()
{
	virtualOp = (PLGitem*)virtualStack->pop();
	virtualItem = (Instance*)virtualStack->pop();
}

/*****************************************************************************
	Process a tawk input file
*****************************************************************************/
void Tawk::process(char *name)
{
char 	*input = 0;
	SymbolType::setTypeTable();
	setCurrentClass(SymbolType::globalType);
	falseSymbol = new Symbol("0",SymbolType::intType);
	nullSymbol = new Symbol("0",SymbolType::nullType);
	trueSymbol = new Symbol("1",SymbolType::intType);
	typesSet = SymbolType::types->typesSet;
	setTable->add("typesSet",typesSet);
	setInput(name);
	run("FileName");
	if ( !Operate::verbs )
		{
		setOperators();
		Tawk::setComparisons();
		}
	input = ::getStringFromFile(name);
	if ( input )
		{
		setInput(input);
		parse("Start");
		}
}

/*****************************************************************************
	Process debugging directives
*****************************************************************************/
void Tawk::processDirectives()
{
char 	*text = ::getStringFromFile(directivesFile);
	if ( !text )
		::fprintf(stderr,"processDirectives: could not get text from %s\n",directivesFile);
	divertInput(text,"Divert");
	directivesFile = 0;
}

/*******************************************************************************
        Process a print target
*******************************************************************************/
Instance *Tawk::processPrintTarget(PLGitem *target)
{
PLGitem 	*field = 0;
Instance 	*printObject = 0;
SymbolType 	*type = 0;
int 		direct = 0;
	if ( target )
		{
		field = target->get("instance");
		if ( !field )
			defaultPrinter = currentSymbols->find("printf");
		else {
			printObject = (Instance*)field->value;
			type = printObject->getType();
			direct = printObject->howDirect();
			if ( type == SymbolType::stringType || type == SymbolType::charType )
				{
				if ( direct == 1 )
					{
					defaultPrinter = currentSymbols->find("sprintf");
					defaultPrinter->addParameter(printObject);
					}
				else
				if ( direct == 2 )
					{
					defaultPrinter = currentSymbols->find("asprintf");
					defaultPrinter->addParameter(printObject);
					}
				else	defaultPrinter = getInstance("Invalid print command");
				printObject = new Instance(defaultPrinter);
				}
			else {
				defaultPrinter = printObject;
				defaultPrinter->isPrintMethod = 1;
				// Used to flag printObject process
				}
			}
		}
	else
	if ( !defaultPrinter )
		defaultPrinter = currentSymbols->find("printf");
	if ( !printObject )
		printObject = new Instance(defaultPrinter);
	return printObject;
}

/*******************************************************************************
        push current virtualOp onto virtuals stack
*******************************************************************************/
void Tawk::pushVirtuals()
{
	virtualStack->push(virtualItem);
	virtualItem = 0;
	virtualStack->push(virtualOp);
	virtualOp = 0;
}

/*******************************************************************************
        reset virtualOp
*******************************************************************************/
void Tawk::resetVirtuals()
{
	if ( virtualStack->length )
		{
		virtualItem = 0;
		virtualOp = 0;
		virtualStack->clear();
		}
}

PLGitem *Tawk::run(char *name)
{
	initializeKeyWords();
	if ( !rules->hashList->length )
		{
		setRules();
		initialize();
		}
	return parse(name);
}

/*****************************************************************************
	Initialize CompareOperators
*****************************************************************************/
void Tawk::setComparisons()
{
	if ( Expression::CompareOperators )
		return;
	Expression::CompareOperators = new BaseHash();
	Expression::CompareOperators->put("lt",(void*)Operate::verbs->find("<"));
	Expression::CompareOperators->put("le",(void*)Operate::verbs->find("<="));
	Expression::CompareOperators->put("gt",(void*)Operate::verbs->find(">"));
	Expression::CompareOperators->put("ge",(void*)Operate::verbs->find(">="));
	Expression::CompareOperators->put("eq",(void*)Operate::verbs->find("=="));
	Expression::CompareOperators->put("ne",(void*)Operate::verbs->find("!="));
}

/******************************************************************************
	Current class setter
******************************************************************************/
void Tawk::setCurrentClass(SymbolType *t)
{
	currentSymbols->presentClass = currentClass = t;
}

/******************************************************************************
	Current type setter
******************************************************************************/
void Tawk::setCurrentType(SymbolType *t)
{
	currentType = t;
}

/*******************************************************************************
        Create operators (without verbs)
*******************************************************************************/
void Tawk::setOperators()
{
Operate 	*verb = 0;
	Operate::verbs = Operators;
	verb = new Operate("~",0);
	verb->unary = 1;
	verb = new Operate("!",0);
	verb->unary = 1;
	verb = new Operate("++",0);
	verb->assign = 1;
	verb->unary = 1;
	verb = new Operate("--",0);
	verb->unary = 1;
	verb->assign = 1;
	verb = new Operate("*",1);
	verb = new Operate("/",1);
	verb = new Operate("%",1);
	verb = new Operate("+",2);
	verb->pointing = 1;
	verb = new Operate("-",2);
	verb->pointing = 1;
	verb = new Operate(">>",3);
	verb = new Operate("<<",3);
	verb = new Operate("<",4);
	verb->comparison = 2;
	verb = new Operate("lt",4);
	verb->comparison = 2;
	verb->call = 1;
	verb = new Operate("<=",4);
	verb->comparison = 3;
	verb = new Operate("le",4);
	verb->comparison = 3;
	verb->call = 1;
	verb = new Operate(">",4);
	verb->comparison = 4;
	verb = new Operate("gt",4);
	verb->comparison = 4;
	verb->call = 1;
	verb = new Operate(">=",4);
	verb->comparison = 5;
	verb = new Operate("ge",4);
	verb->comparison = 5;
	verb->call = 1;
	verb = new Operate("==",5);
	verb->comparison = 1;
	verb = new Operate("eq",5);
	verb->comparison = 1;
	verb->call = 1;
	verb = new Operate("!=",5);
	verb->comparison = 6;
	verb = new Operate("ne",5);
	verb->comparison = 6;
	verb->call = 1;
	verb = new Operate("&",6);
	verb = new Operate("^",6);
	verb = new Operate("|",6);
	verb = new Operate("&&",7);
	verb->conjunction = 1;
	verb = new Operate("||",7);
	verb->conjunction = 1;
	verb = new Operate("?",7);
	verb->question = 1;
	verb = new Operate(":",7);
	verb->question = 1;
	verb = new Operate("=",10);
	verb->assign = 1;
	verb = new Operate("+=",10);
	verb->assign = 1;
	verb = new Operate("-=",10);
	verb->assign = 1;
	verb = new Operate("*=",10);
	verb->assign = 1;
	verb = new Operate("/=",10);
	verb->assign = 1;
	verb = new Operate("%=",10);
	verb->assign = 1;
	verb = new Operate(">>=",10);
	verb->assign = 1;
	verb = new Operate("<<=",10);
	verb->assign = 1;
	verb = new Operate("&=",10);
	verb->assign = 1;
	verb = new Operate("^=",10);
	verb->assign = 1;
	verb = new Operate("|=",10);
	verb->assign = 1;
	commaOp = new Operate(",",11);
	verb = new Operate("..",11);
	verb->isRange = 1;
	verb = new Operate(">.",11);
	verb->isRange = 1;
	verb = new Operate(".<",11);
	verb->isRange = 1;
	verb = new Operate("><",11);
	verb->isRange = 1;
	//verbs.dump(false);
}

void Tawk::setRules()
{
	setSkip();
	alphaSet = getSet("alphaSet","ABCDEFGHIJKLMNOPQRSTUVWXYZA-Zabcdefghijklmnopqrstuvwxyza-z_");
	nameStartSet = getSet("nameStartSet","ABCDEFGHIJKLMNOPQRSTUVWXYZA-Zabcdefghijklmnopqrstuvwxyza-z_@");
	commentSet = getSet("commentSet","-/#");
	compareSet = getSet("compareSet","-!/|%^?:&*<>+=glen");
	compareFollow = getSet("compareFollow","-+r(\"*&!");
	logicSet = getSet("logicSet","glen");
	methodSet = getSet("methodSet","ABCDEFGHIJKLMNOPQRSTUVWXYZA-Zabcdefghijklmnopqrstuvwxyza-z01234567890-9_*&(),@");
	methodNameSet = getSet("methodNameSet","ABCDEFGHIJKLMNOPQRSTUVWXYZA-Zabcdefghijklmnopqrstuvwxyza-z01234567890-9_:");
	nameSet = getSet("nameSet","ABCDEFGHIJKLMNOPQRSTUVWXYZA-Zabcdefghijklmnopqrstuvwxyza-z01234567890-9_");
	operatorSet = getSet("operatorSet","-!/~|%^?:&*<>+=ei");
	space = getSet("space","n");
	rangeSet = getSet("rangeSet","<>.");
	singleQuote = getSet("singleQuote","'");
	stringOP = getSet("stringOP","-+=");
	textFollow = getSet("textFollow","ABCDEFGHIJKLMNOPQRSTUVWXYZA-Zabcdefghijklmnopqrstuvwxyza-z01234567890-9_");
	typesSet = getSet("typesSet","");
	//
	currentRule = getRule("Block");
	currentRule->immediate = ::BlockTawkNow;
	addTest(5,(void*)getRule("BlockStart"),"start",1,1,"defaultSKIP");
	addTest(5,(void*)getRule("Line"),"line",0,268435455,"defaultSKIP");
	addTest(7,(void*)"}",(char*)0,1,1,"defaultSKIP");
	currentTest->errorMessage = "Expected } or statement";
	//
	currentRule = getRule("StatementBody16");
	currentRule->immediate = ::StatementBody16TawkNow;
	addTest(7,(void*)"do",(char*)0,1,1,"defaultSKIP");
	currentSet = textFollow;
	addTest(6,(void*)currentSet,(char*)0,-1,1,(char*)0);
	addTest(5,(void*)getRule("Iterating"),(char*)0,0,1,"defaultSKIP");
	addTest(5,(void*)getRule("Statement"),"statement",1,1,"defaultSKIP");
	addTest(5,(void*)getRule("ResetType"),(char*)0,1,1,"defaultSKIP");
	addTest(7,(void*)"while",(char*)0,1,1,"defaultSKIP");
	currentSet = textFollow;
	addTest(6,(void*)currentSet,(char*)0,-1,1,(char*)0);
	addTest(5,(void*)getRule("Expression"),"instance",1,1,"defaultSKIP");
	addTest(7,(void*)";",(char*)0,1,1,"defaultSKIP");
	currentRule->next = getRule("StatementBody17");
	//
	currentRule = getRule("BlockStart");
	currentRule->immediate = ::BlockStartTawkNow;
	addTest(7,(void*)"{","brace",1,1,"defaultSKIP");
	//
	currentRule = getRule("ClassBlockStart");
	currentRule->immediate = ::ClassBlockStartTawkNow;
	addTest(7,(void*)"{",(char*)0,1,1,"defaultSKIP");
	//
	currentRule = getRule("OperationTail");
	currentRule->immediate = ::OperationTailTawkNow;
	addTest(5,(void*)getRule("Operator"),"operate",1,1,"defaultSKIP");
	currentSet = compareSet;
	currentRule->guardSet = currentSet;
	currentTest->guardSet = currentSet;
	addTest(5,(void*)getRule("UnaryExpression"),"instance",1,1,"defaultSKIP");
	addTest(5,(void*)getRule("Question"),"question",0,1,"defaultSKIP");
	currentRule->next = getRule("OperationTail2");
	currentRule->fail = ::assignFailed;
	//
	currentRule = getRule("MacroBlock");
	currentRule->immediate = ::MacroBlockTawkNow;
	addTest(5,(void*)getRule("Line"),"line",0,268435455,"defaultSKIP");
	//
	currentRule = getRule("OverLoadItem4");
	currentRule->immediate = ::OverLoadItem4TawkNow;
	addTest(5,(void*)getRule("OverLoadItem4Block1"),"newOp",1,1,"defaultSKIP");
	addTest(5,(void*)getRule("Name"),"name",1,1,"defaultSKIP");
	currentRule->next = getRule("OverLoadItem5");
	//
	currentRule = getRule("Body2");
	currentRule->immediate = ::Body2TawkNow;
	addTest(5,(void*)getRule("Method"),"body",1,1,"defaultSKIP");
	currentRule->next = getRule("Body3");
	//
	currentRule = getRule("Statement2");
	currentRule->immediate = ::Statement2TawkNow;
	addTest(5,(void*)getRule("StatementBody"),"statement",1,1,"defaultSKIP");
	//
	currentRule = getRule("Body3");
	currentRule->immediate = ::Body3TawkNow;
	addTest(5,(void*)getRule("Declaration"),"body",1,1,"defaultSKIP");
	addTest(7,(void*)";",(char*)0,1,1,"defaultSKIP");
	currentTest->errorMessage = "Expected a semi-colon";
	currentRule->next = getRule("Body4");
	//
	currentRule = getRule("SecondaryExpression2");
	currentRule->immediate = ::SecondaryExpression2TawkNow;
	addTest(7,(void*)"sizeof","instance",1,1,"defaultSKIP");
	addTest(7,(void*)"(",(char*)0,1,1,"defaultSKIP");
	addTest(5,(void*)getRule("Type"),(char*)0,1,1,"defaultSKIP");
	addTest(7,(void*)"*","pointer",0,268435455,"defaultSKIP");
	addTest(7,(void*)")",(char*)0,1,1,"defaultSKIP");
	//
	currentRule = getRule("PrimaryExpression2");
	addTest(5,(void*)getRule("SecondaryExpression"),"instance",1,1,"defaultSKIP");
	currentRule->next = getRule("PrimaryExpression3");
	//
	currentRule = getRule("InitExpression2");
	addTest(5,(void*)getRule("RangeField"),"instance",1,1,"defaultSKIP");
	//
	currentRule = getRule("StringExpression2");
	currentRule->immediate = ::StringExpression2TawkNow;
	addTest(5,(void*)getRule("Expression"),"instance",1,1,"defaultSKIP");
	//
	currentRule = getRule("UnaryExpression2");
	currentRule->immediate = ::UnaryExpression2TawkNow;
	addTest(5,(void*)getRule("UnaryOperator"),"operate",0,1,"defaultSKIP");
	addTest(5,(void*)getRule("CastExpression"),"cast",0,1,"defaultSKIP");
	addTest(5,(void*)getRule("PrimaryExpression"),"instance",1,1,"defaultSKIP");
	currentRule->next = getRule("UnaryExpression3");
	//
	currentRule = getRule("ClassBlock");
	currentRule->immediate = ::ClassBlockTawkNow;
	addTest(5,(void*)getRule("ClassBlockStart"),(char*)0,1,1,"defaultSKIP");
	addTest(5,(void*)getRule("Body"),(char*)0,0,268435455,"defaultSKIP");
	addTest(7,(void*)"}",(char*)0,1,1,"defaultSKIP");
	currentTest->errorMessage = "Expected }, static, or type";
	//
	currentRule = getRule("ClassHeading");
	currentRule->immediate = ::ClassHeadingTawkNow;
	addTest(7,(void*)"class",(char*)0,1,1,"defaultSKIP");
	addTest(5,(void*)getRule("ClassName"),"nom",1,1,"defaultSKIP");
	addTest(5,(void*)getRule("ClassAttributes"),"attributes",0,268435455,"defaultSKIP");
	currentRule->next = getRule("ClassHeading2");
	//
	currentRule = getRule("StatementBody14");
	currentRule->immediate = ::StatementBody14TawkNow;
	addTest(7,(void*)";","statement",1,1,"defaultSKIP");
	currentRule->next = getRule("StatementBody15");
	//
	currentRule = getRule("RangeTail");
	addTest(4,(void*)Ranges,"operate",1,1,"defaultSKIP");
	currentSet = rangeSet;
	currentRule->guardSet = currentSet;
	currentTest->guardSet = currentSet;
	addTest(5,(void*)getRule("Expression"),"instance",1,1,"defaultSKIP");
	//
	currentRule = getRule("ExpressItem");
	addTest(5,(void*)getRule("Expression"),"instance",1,1,"defaultSKIP");
	addTest(7,(void*)",",(char*)0,0,1,"defaultSKIP");
	//
	currentRule = getRule("ClassHeading2");
	currentRule->immediate = ::ClassHeading2TawkNow;
	addTest(7,(void*)"external","externalRef",0,1,"defaultSKIP");
	addTest(5,(void*)getRule("Structure"),"structure",1,1,"defaultSKIP");
	addTest(7,(void*)";",(char*)0,0,1,"defaultSKIP");
	currentRule->next = getRule("ClassHeading3");
	//
	currentRule = getRule("Constant");
	addTest(5,(void*)getRule("Number"),"instance",1,1,"defaultSKIP");
	currentRule->next = getRule("Constant2");
	//
	currentRule = getRule("Inheritance");
	addTest(5,(void*)getRule("Extends"),(char*)0,1,1,"defaultSKIP");
	currentRule->next = getRule("Inheritance2");
	//
	currentRule = getRule("ClassHeading3");
	currentRule->immediate = ::ClassHeading3TawkNow;
	addTest(7,(void*)"external",(char*)0,1,1,"defaultSKIP");
	addTest(5,(void*)getRule("ClassHeading3Block15"),"kind",0,1,"defaultSKIP");
	addTest(5,(void*)getRule("ClassName"),"nom",0,268435455,"defaultSKIP");
	addTest(5,(void*)getRule("ClassAttributes"),"attributes",0,268435455,"defaultSKIP");
	addTest(7,(void*)";",(char*)0,0,1,"defaultSKIP");
	//
	currentRule = getRule("ItemArray");
	addTest(7,(void*)"[",(char*)0,1,1,"defaultSKIP");
	currentSet = getSet("01234567890-9");
	addTest(6,(void*)currentSet,(char*)0,0,268435455,"defaultSKIP");
	addTest(7,(void*)"]",(char*)0,1,1,"defaultSKIP");
	//
	currentRule = getRule("Else2");
	currentRule->immediate = ::Else2TawkNow;
	addTest(7,(void*)"or",(char*)0,1,1,"defaultSKIP");
	currentSet = textFollow;
	addTest(6,(void*)currentSet,(char*)0,-1,1,(char*)0);
	addTest(5,(void*)getRule("IfBody"),"statement",1,1,"defaultSKIP");
	//
	currentRule = getRule("CommentBody2");
	addTest(7,(void*)"//",(char*)0,1,1,"defaultSKIP");
	addTest(5,(void*)getRule("EndComment"),"end",1,1,"defaultSKIP");
	currentRule->next = getRule("CommentBody3");
	//
	currentRule = getRule("StatementBody2");
	currentRule->immediate = ::StatementBody2TawkNow;
	addTest(5,(void*)getRule("Block"),"statement",1,1,"defaultSKIP");
	currentRule->next = getRule("StatementBody3");
	//
	currentRule = getRule("Number2Block8Block9");
	addTest(7,(void*)"0",(char*)0,1,1,(char*)0);
	currentSet = getSet("xX");
	addTest(6,(void*)currentSet,(char*)0,1,1,(char*)0);
	currentSet = getSet("01234567890-9abcdefa-fABCDEFA-F");
	addTest(6,(void*)currentSet,(char*)0,1,268435455,(char*)0);
	//
	currentRule = getRule("Extends");
	currentRule->immediate = ::ExtendsTawkNow;
	addTest(5,(void*)getRule("ClassHeading"),(char*)0,1,1,"defaultSKIP");
	addTest(5,(void*)getRule("ClassBlock"),(char*)0,0,1,"defaultSKIP");
	currentRule->next = getRule("Extends2");
	//
	currentRule = getRule("Inheritance4");
	currentRule->immediate = ::Inheritance4TawkNow;
	addTest(5,(void*)getRule("EndComment"),"error",1,1,"defaultSKIP");
	//
	currentRule = getRule("PrintCommand");
	currentRule->immediate = ::PrintCommandTawkNow;
	addTest(7,(void*)"print","printer",1,1,"defaultSKIP");
	currentSet = textFollow;
	addTest(6,(void*)currentSet,(char*)0,-1,1,(char*)0);
	addTest(5,(void*)getRule("PrintTarget"),"target",0,1,(char*)0);
	currentRule->next = getRule("PrintCommand2");
	//
	currentRule = getRule("PoundCommand");
	currentRule->immediate = ::PoundCommandTawkNow;
	addTest(4,(void*)State,"state",1,1,"defaultSKIP");
	addTest(5,(void*)getRule("Type"),"type",0,1,"defaultSKIP");
	addTest(5,(void*)getRule("Count"),"level",0,1,"defaultSKIP");
	addTest(5,(void*)getRule("RuleList"),"list",0,1,"defaultSKIP");
	addTest(5,(void*)getRule("FieldList"),"field",0,1,"defaultSKIP");
	currentRule->next = getRule("PoundCommand2");
	//
	currentRule = getRule("Template2Any");
	currentRule->immediate = ::balancEbail;
	addTest(1,(void*)0,(char*)0,1,1,(char*)0);
	//
	currentRule = getRule("FieldBody");
	currentRule->defer = ::FieldBodyTawkAct;
	addTest(5,(void*)getRule("Bump"),"prefix",0,1,"defaultSKIP");
	addTest(5,(void*)getRule("FieldComponent"),"part",1,1,"defaultSKIP");
	currentRule->next = getRule("FieldBody2");
	//
	currentRule = getRule("FieldBody2");
	currentRule->defer = ::FieldBody2TawkAct;
	addTest(5,(void*)getRule("New"),"name",1,1,"defaultSKIP");
	currentRule->next = getRule("FieldBody3");
	//
	currentRule = getRule("FieldBody3");
	currentRule->immediate = ::FieldBody3TawkNow;
	addTest(7,(void*)"(",(char*)0,1,1,"defaultSKIP");
	addTest(5,(void*)getRule("Expression"),"name",1,1,"defaultSKIP");
	addTest(7,(void*)")",(char*)0,1,1,"defaultSKIP");
	//
	currentRule = getRule("Qualified");
	currentRule->immediate = ::QualifiedTawkNow;
	addTest(5,(void*)getRule("QualifyType"),"type",0,1,"defaultSKIP");
	addTest(5,(void*)getRule("QualifyStart"),"field",1,1,"defaultSKIP");
	addTest(5,(void*)getRule("QualifyTail"),"rest",0,268435455,"defaultSKIP");
	addTest(5,(void*)getRule("Bump"),"postfix",0,1,"defaultSKIP");
	//
	currentRule = getRule("FieldExpression");
	currentRule->immediate = ::FieldExpressionTawkNow;
	addTest(5,(void*)getRule("CastExpression"),"cast",0,1,"defaultSKIP");
	addTest(5,(void*)getRule("Indirection"),"direct",0,1,"defaultSKIP");
	addTest(5,(void*)getRule("Qualified"),"instance",1,1,"defaultSKIP");
	//
	currentRule = getRule("Expression");
	currentRule->immediate = ::ExpressionTawkNow;
	addTest(5,(void*)getRule("ExpressPart"),"instance",1,1,"defaultSKIP");
	addTest(5,(void*)getRule("ExpressTail"),"express",0,268435455,"defaultSKIP");
	currentRule->fail = ::expressPartFailed;
	//
	currentRule = getRule("CastExpression");
	currentRule->immediate = ::CastExpressionTawkNow;
	addTest(5,(void*)getRule("Indirection"),"direct",0,1,"defaultSKIP");
	addTest(7,(void*)"(",(char*)0,1,1,"defaultSKIP");
	addTest(5,(void*)getRule("CastType"),"type",1,1,"defaultSKIP");
	addTest(5,(void*)getRule("CastTail"),"rest",0,1,"defaultSKIP");
	addTest(7,(void*)")",(char*)0,1,1,"defaultSKIP");
	//
	currentRule = getRule("SecondaryExpression");
	currentRule->immediate = ::SecondaryExpressionTawkNow;
	addTest(7,(void*)"null","instance",1,1,"defaultSKIP");
	currentSet = textFollow;
	addTest(6,(void*)currentSet,(char*)0,-1,1,(char*)0);
	currentRule->next = getRule("SecondaryExpression2");
	//
	currentRule = getRule("InitExpression");
	currentRule->immediate = ::InitExpressionTawkNow;
	addTest(5,(void*)getRule("Expression"),"instance",1,1,"defaultSKIP");
	currentRule->next = getRule("InitExpression2");
	//
	currentRule = getRule("RangeExpression");
	currentRule->immediate = ::RangeExpressionTawkNow;
	addTest(5,(void*)getRule("UnaryExpression"),"instance",1,1,"defaultSKIP");
	addTest(5,(void*)getRule("RangeTail"),"back",1,1,"defaultSKIP");
	//
	currentRule = getRule("PrimaryExpression");
	addTest(5,(void*)getRule("Constant"),"instance",1,1,"defaultSKIP");
	currentRule->next = getRule("PrimaryExpression2");
	//
	currentRule = getRule("StatementBody8");
	currentRule->immediate = ::StatementBody8TawkNow;
	addTest(5,(void*)getRule("Case"),"statement",1,1,"defaultSKIP");
	currentRule->next = getRule("StatementBody9");
	//
	currentRule = getRule("StringExpression");
	addTest(5,(void*)getRule("AllowShortcuts"),(char*)0,1,1,"defaultSKIP");
	addTest(5,(void*)getRule("PrintShortcut"),"instance",1,1,"defaultSKIP");
	currentRule->next = getRule("StringExpression2");
	//
	currentRule = getRule("UnaryExpression");
	currentRule->immediate = ::UnaryExpressionTawkNow;
	addTest(5,(void*)getRule("UnaryOperator"),"operate",0,1,"defaultSKIP");
	addTest(5,(void*)getRule("ConditionWord"),"instance",1,1,"defaultSKIP");
	currentRule->next = getRule("UnaryExpression2");
	//
	currentRule = getRule("Fielding");
	currentRule->immediate = ::FieldingTawkNow;
	addTest(5,(void*)getRule("Name"),"name",1,1,"defaultSKIP");
	//
	currentRule = getRule("IfBody");
	addTest(5,(void*)getRule("Expression"),"instance",1,1,"defaultSKIP");
	addTest(5,(void*)getRule("Comment"),(char*)0,0,1,"defaultSKIP");
	addTest(5,(void*)getRule("Statement"),"action",1,1,"defaultSKIP");
	addTest(5,(void*)getRule("ResetType"),(char*)0,0,1,"defaultSKIP");
	addTest(5,(void*)getRule("Else"),"otherwise",0,1,"defaultSKIP");
	//
	currentRule = getRule("ForOption");
	currentRule->immediate = ::ForOptionTawkNow;
	addTest(7,(void*)"(","instance",1,1,"defaultSKIP");
	addTest(5,(void*)getRule("ExpressList"),"initial",0,1,"defaultSKIP");
	addTest(7,(void*)";",(char*)0,1,1,"defaultSKIP");
	currentTest->errorMessage = "Expected expression list or ;";
	addTest(5,(void*)getRule("Expression"),"condition",0,1,"defaultSKIP");
	addTest(7,(void*)";",(char*)0,1,1,"defaultSKIP");
	currentTest->errorMessage = "Expected expression or ;";
	addTest(5,(void*)getRule("ExpressList"),"increment",0,1,"defaultSKIP");
	addTest(7,(void*)")",(char*)0,1,1,"defaultSKIP");
	currentTest->errorMessage = "Expected expression list or )";
	currentRule->next = getRule("ForOption2");
	//
	currentRule = getRule("ForOption2");
	currentRule->immediate = ::ForOption2TawkNow;
	addTest(5,(void*)getRule("Expression"),"instance",1,1,"defaultSKIP");
	addTest(5,(void*)getRule("ForOption2Block16"),"name",0,1,"defaultSKIP");
	//
	currentRule = getRule("Include");
	currentRule->immediate = ::IncludeTawkNow;
	addTest(5,(void*)getRule("IncludeBalancE"),"include",1,1,(char*)0);
	currentRule->next = getRule("Include2");
	//
	currentRule = getRule("Include2");
	currentRule->immediate = ::Include2TawkNow;
	addTest(5,(void*)getRule("Include2BalancE"),"include",1,1,(char*)0);
	currentRule->next = getRule("Include3");
	//
	currentRule = getRule("CommentBody3");
	addTest(5,(void*)getRule("CommentBody3BalancE"),(char*)0,1,1,"defaultSKIP");
	currentRule->next = getRule("CommentBody4");
	//
	currentRule = getRule("StatementBody3");
	currentRule->immediate = ::StatementBody3TawkNow;
	addTest(7,(void*)"if",(char*)0,1,1,"defaultSKIP");
	currentSet = textFollow;
	addTest(6,(void*)currentSet,(char*)0,-1,1,(char*)0);
	addTest(5,(void*)getRule("IfBody"),"statement",1,1,"defaultSKIP");
	currentRule->next = getRule("StatementBody4");
	//
	currentRule = getRule("Include3");
	currentRule->immediate = ::Include3TawkNow;
	addTest(5,(void*)getRule("Include3Block17"),(char*)0,1,1,"defaultSKIP");
	currentSet = getSet("t");
	addTest(6,(void*)currentSet,(char*)0,1,268435455,(char*)0);
	addTest(7,(void*)"\n","include",0,1,(char*)0);
	currentTest->skipOverMatch = 1;
	currentTest->processUpTo = 1;
	//
	currentRule = getRule("CommentBody4");
	addTest(5,(void*)getRule("CommentBody4BalancE"),(char*)0,1,1,"defaultSKIP");
	currentRule->next = getRule("CommentBody5");
	//
	currentRule = getRule("StatementBody4");
	currentRule->immediate = ::StatementBody4TawkNow;
	addTest(7,(void*)"return","statement",1,1,"defaultSKIP");
	currentSet = textFollow;
	addTest(6,(void*)currentSet,(char*)0,-1,1,(char*)0);
	addTest(5,(void*)getRule("Expression"),"instance",0,1,"defaultSKIP");
	addTest(7,(void*)";",(char*)0,1,1,"defaultSKIP");
	currentRule->next = getRule("StatementBody5");
	//
	currentRule = getRule("InstanceTail");
	currentRule->immediate = ::InstanceTailTawkNow;
	addTest(5,(void*)getRule("NewArray"),"array",1,268435455,"defaultSKIP");
	currentRule->next = getRule("InstanceTail2");
	//
	currentRule = getRule("InstanceTail2");
	addTest(7,(void*)"(",(char*)0,1,1,"defaultSKIP");
	addTest(5,(void*)getRule("ResetType"),(char*)0,1,1,"defaultSKIP");
	addTest(5,(void*)getRule("NoShortcuts"),(char*)0,1,1,"defaultSKIP");
	addTest(5,(void*)getRule("ParameterList"),"expression",0,1,"defaultSKIP");
	addTest(7,(void*)")",(char*)0,1,1,"defaultSKIP");
	currentRule->fail = ::instanceTailFail;
	//
	currentRule = getRule("Iterating");
	currentRule->immediate = ::IteratingTawkNow;
	//
	currentRule = getRule("OverLoadItem5");
	currentRule->immediate = ::OverLoadItem5TawkNow;
	addTest(7,(void*)"()",(char*)0,1,1,"defaultSKIP");
	addTest(5,(void*)getRule("Name"),"name",1,1,"defaultSKIP");
	//
	currentRule = getRule("Lambda");
	currentRule->immediate = ::LambdaTawkNow;
	addTest(5,(void*)getRule("LambdaName"),"function",1,1,"defaultSKIP");
	addTest(7,(void*)"=",(char*)0,1,1,"defaultSKIP");
	addTest(5,(void*)getRule("Block"),"body",1,1,"defaultSKIP");
	currentRule->next = getRule("Lambda2");
	//
	currentRule = getRule("Character");
	currentRule->immediate = ::CharacterTawkNow;
	addTest(5,(void*)getRule("CharacterBlock4"),"instance",1,1,(char*)0);
	//
	currentRule = getRule("Lambda2");
	currentRule->immediate = ::Lambda2TawkNow;
	addTest(5,(void*)getRule("MethodType"),"function",1,1,"defaultSKIP");
	addTest(5,(void*)getRule("Block"),"body",1,1,"defaultSKIP");
	//
	currentRule = getRule("Line");
	currentRule->immediate = ::LineTawkNow;
	addTest(7,(void*)"use",(char*)0,1,1,"defaultSKIP");
	currentSet = textFollow;
	addTest(6,(void*)currentSet,(char*)0,-1,1,(char*)0);
	addTest(5,(void*)getRule("FieldExpression"),"target",1,1,"defaultSKIP");
	currentRule->next = getRule("Line2");
	//
	currentRule = getRule("Line3");
	currentRule->immediate = ::Line3TawkNow;
	addTest(7,(void*)"label",(char*)0,1,1,"defaultSKIP");
	addTest(5,(void*)getRule("Name"),"name",1,268435455,"defaultSKIP");
	addTest(7,(void*)";",(char*)0,1,1,"defaultSKIP");
	currentTest->errorMessage = "Label declaration expected an ending ;";
	//
	currentRule = getRule("Format");
	addTest(7,(void*)"#",(char*)0,1,1,(char*)0);
	currentSet = getSet("- 0+");
	addTest(6,(void*)currentSet,(char*)0,0,1,(char*)0);
	currentSet = getSet("01234567890-9");
	addTest(6,(void*)currentSet,"width",0,268435455,(char*)0);
	currentSet = getSet("*%.01234567890-9abcdefghijklmnopqrstuvwxyza-zABCDEFGHIJKLMNOPQRSTUVWXYZA-Z");
	addTest(6,(void*)currentSet,(char*)0,0,268435455,(char*)0);
	//
	currentRule = getRule("LineByLine");
	currentRule->immediate = ::LineByLineTawkNow;
	addTest(5,(void*)getRule("Statement"),"line",1,268435455,"defaultSKIP");
	currentSet = getSet("ABCDEFGHIJKLMNOPQRSTUVWXYZA-Zabcdefghijklmnopqrstuvwxyza-z01234567890-9/(*");
	currentRule->guardSet = currentSet;
	currentTest->guardSet = currentSet;
	//
	currentRule = getRule("MacroName");
	currentRule->immediate = ::MacroNameTawkNow;
	addTest(5,(void*)getRule("NameSet"),"name",1,1,"defaultSKIP");
	//
	currentRule = getRule("Method");
	currentRule->immediate = ::MethodTawkNow;
	addTest(5,(void*)getRule("MethodType"),"method",1,1,"defaultSKIP");
	addTest(5,(void*)getRule("Block"),"block",1,1,"defaultSKIP");
	//
	currentRule = getRule("IncludeBoDY");
	currentRule->immediate = ::balancEbody;
	addTest(7,(void*)"#include","begin",0,1,(char*)0);
	addTest(7,(void*)"\n","end",0,1,(char*)0);
	currentRule->next = getRule("IncludeAny");
	//
	currentRule = getRule("TargetMethod");
	currentRule->immediate = ::TargetMethodTawkNow;
	addTest(5,(void*)getRule("Name"),"target",1,1,"defaultSKIP");
	//
	currentRule = getRule("MethodHead");
	currentRule->defer = ::MethodHeadTawkAct;
	addTest(5,(void*)getRule("Indirection"),"direct",0,1,"defaultSKIP");
	addTest(5,(void*)getRule("MethodName"),"function",0,1,"defaultSKIP");
	addTest(5,(void*)getRule("MethodParameters"),"head",1,1,"defaultSKIP");
	//
	currentRule = getRule("MethodType");
	currentRule->immediate = ::MethodTypeTawkNow;
	currentRule->defer = ::MethodTypeTawkAct;
	addTest(4,(void*)Linkage,"modify",0,268435455,"defaultSKIP");
	addTest(5,(void*)getRule("Type"),"type",1,1,"defaultSKIP");
	addTest(5,(void*)getRule("MethodHead"),"methodHead",1,1,"defaultSKIP");
	addTest(7,(void*)"{",(char*)0,1,1,"defaultSKIP");
	currentTest->isIgnored = 1;
	//
	currentRule = getRule("Extends2");
	addTest(5,(void*)getRule("Comment"),(char*)0,1,1,"defaultSKIP");
	currentSet = commentSet;
	currentRule->guardSet = currentSet;
	currentTest->guardSet = currentSet;
	currentRule->next = getRule("Extends3");
	//
	currentRule = getRule("Throw");
	addTest(7,(void*)"throw",(char*)0,1,1,"defaultSKIP");
	addTest(5,(void*)getRule("Expression"),"express",1,1,"defaultSKIP");
	//
	currentRule = getRule("New");
	currentRule->defer = ::NewTawkAct;
	addTest(7,(void*)"new","instance",1,1,"defaultSKIP");
	currentSet = textFollow;
	addTest(6,(void*)currentSet,(char*)0,-1,1,(char*)0);
	addTest(5,(void*)getRule("Type"),"type",0,1,"defaultSKIP");
	addTest(5,(void*)getRule("InstanceBody"),"body",0,1,"defaultSKIP");
	addTest(5,(void*)getRule("ArrayInitializer"),"initial",0,1,"defaultSKIP");
	//
	currentRule = getRule("NoShortcuts");
	currentRule->immediate = ::NoShortcutsTawkNow;
	//
	currentRule = getRule("AllowShortcuts");
	currentRule->immediate = ::AllowShortcutsTawkNow;
	//
	currentRule = getRule("CaseLabel2");
	currentRule->immediate = ::CaseLabel2TawkNow;
	addTest(5,(void*)getRule("RangeField"),"instance",1,1,"defaultSKIP");
	currentRule->next = getRule("CaseLabel3");
	//
	currentRule = getRule("Parameter");
	currentRule->immediate = ::ParameterTawkNow;
	addTest(5,(void*)getRule("Type"),"type",1,1,"defaultSKIP");
	addTest(5,(void*)getRule("ParameterItem"),"item",0,268435455,"defaultSKIP");
	addTest(7,(void*)",",(char*)0,0,1,"defaultSKIP");
	//
	currentRule = getRule("AliasParameter");
	addTest(5,(void*)getRule("Name"),"parameter",1,1,"defaultSKIP");
	addTest(5,(void*)getRule("AliasParameterBlock0"),"replacedBy",0,1,"defaultSKIP");
	addTest(7,(void*)",",(char*)0,0,1,"defaultSKIP");
	currentRule->next = getRule("AliasParameter2");
	//
	currentRule = getRule("Indirection");
	currentSet = getSet("*&^");
	addTest(6,(void*)currentSet,"direct",1,268435455,"defaultSKIP");
	//
	currentRule = getRule("ParameterItem");
	currentRule->defer = ::ParameterItemTawkAct;
	addTest(5,(void*)getRule("MethodHead"),"name",1,1,"defaultSKIP");
	addTest(7,(void*)",",(char*)0,0,1,"defaultSKIP");
	currentRule->next = getRule("ParameterItem2");
	//
	currentRule = getRule("ParameterItem2");
	currentRule->defer = ::ParameterItem2TawkAct;
	addTest(5,(void*)getRule("Indirection"),"direct",0,1,"defaultSKIP");
	addTest(5,(void*)getRule("Name"),"name",1,1,"defaultSKIP");
	addTest(7,(void*)"[]","array",0,268435455,"defaultSKIP");
	addTest(7,(void*)",",(char*)0,0,1,"defaultSKIP");
	currentRule->next = getRule("ParameterItem3");
	//
	currentRule = getRule("ParameterItem3");
	currentRule->defer = ::ParameterItem3TawkAct;
	addTest(7,(void*)"[]","name",1,268435455,"defaultSKIP");
	addTest(7,(void*)",",(char*)0,0,1,"defaultSKIP");
	currentRule->next = getRule("ParameterItem4");
	//
	currentRule = getRule("QualifyStart2");
	addTest(5,(void*)getRule("FieldBody"),"field",1,1,"defaultSKIP");
	//
	currentRule = getRule("ParameterItem4");
	currentRule->defer = ::ParameterItem4TawkAct;
	addTest(5,(void*)getRule("Indirection"),"name",1,1,"defaultSKIP");
	addTest(7,(void*)",",(char*)0,0,1,"defaultSKIP");
	//
	currentRule = getRule("EscapeCharacters2");
	addTest(7,(void*)"u",(char*)0,1,268435455,(char*)0);
	currentSet = getSet("01234567890-9abcdefa-fABCDEFA-F");
	addTest(6,(void*)currentSet,(char*)0,4,4,(char*)0);
	currentRule->next = getRule("EscapeCharacters3");
	//
	currentRule = getRule("AliasItem2");
	currentRule->immediate = ::AliasItem2TawkNow;
	addTest(7,(void*)"new",(char*)0,1,1,"defaultSKIP");
	addTest(5,(void*)getRule("Name"),"alias",1,1,"defaultSKIP");
	currentRule->next = getRule("AliasItem3");
	//
	currentRule = getRule("Print");
	currentRule->immediate = ::PrintTawkNow;
	addTest(5,(void*)getRule("PrintCommand"),"start",1,1,"defaultSKIP");
	addTest(5,(void*)getRule("PrintItem"),"arguments",0,268435455,"defaultSKIP");
	addTest(5,(void*)getRule("PrintTo"),"output",0,1,"defaultSKIP");
	//
	currentRule = getRule("FieldComponent");
	addTest(5,(void*)getRule("Fielding"),"name",1,1,"defaultSKIP");
	addTest(5,(void*)getRule("InstanceBody"),"body",0,1,"defaultSKIP");
	currentRule->next = getRule("FieldComponent2");
	//
	currentRule = getRule("PrintCommand2");
	currentRule->immediate = ::PrintCommand2TawkNow;
	addTest(7,(void*)"cout","stdPrint",1,1,"defaultSKIP");
	currentSet = textFollow;
	addTest(6,(void*)currentSet,(char*)0,-1,1,(char*)0);
	currentRule->next = getRule("PrintCommand3");
	//
	currentRule = getRule("PoundCommand2");
	addTest(5,(void*)getRule("MacroDefine"),(char*)0,1,1,"defaultSKIP");
	currentRule->next = getRule("PoundCommand3");
	//
	currentRule = getRule("PrintCommand3");
	currentRule->immediate = ::PrintCommand3TawkNow;
	addTest(7,(void*)"cerr","stdPrint",1,1,"defaultSKIP");
	currentSet = textFollow;
	addTest(6,(void*)currentSet,(char*)0,-1,1,(char*)0);
	//
	currentRule = getRule("PoundCommand3");
	addTest(5,(void*)getRule("Directive"),(char*)0,1,1,"defaultSKIP");
	//
	currentRule = getRule("PrintItem2");
	currentRule->immediate = ::PrintItem2TawkNow;
	addTest(5,(void*)getRule("Expression"),"instance",1,1,"defaultSKIP");
	addTest(5,(void*)getRule("Format"),"format",0,1,"defaultSKIP");
	//
	currentRule = getRule("SaveVirtuals");
	currentRule->immediate = ::SaveVirtualsTawkNow;
	//
	currentRule = getRule("PrintShortcut");
	currentRule->immediate = ::PrintShortcutTawkNow;
	currentSet = getSet(",:`");
	addTest(6,(void*)currentSet,"instance",1,1,"defaultSKIP");
	//
	currentRule = getRule("CastType");
	currentRule->immediate = ::CastTypeTawkNow;
	addTest(5,(void*)getRule("Type"),"type",1,1,"defaultSKIP");
	addTest(5,(void*)getRule("Indirection"),"direct",0,1,"defaultSKIP");
	addTest(5,(void*)getRule("CastTypeBlock13"),"array",0,1,"defaultSKIP");
	addTest(7,(void*)",",(char*)0,0,1,"defaultSKIP");
	//
	currentRule = getRule("StatementBody17");
	currentRule->immediate = ::StatementBody17TawkNow;
	addTest(5,(void*)getRule("Throw"),"statement",1,1,"defaultSKIP");
	addTest(7,(void*)";",(char*)0,1,1,"defaultSKIP");
	currentRule->next = getRule("StatementBody18");
	//
	currentRule = getRule("QualifyStart");
	currentRule->immediate = ::QualifyStartTawkNow;
	addTest(7,(void*)"this","name",1,1,"defaultSKIP");
	currentSet = textFollow;
	addTest(6,(void*)currentSet,(char*)0,-1,1,(char*)0);
	currentRule->next = getRule("QualifyStart2");
	//
	currentRule = getRule("NotQuote");
	//
	currentRule = getRule("ResetType");
	currentRule->immediate = ::ResetTypeTawkNow;
	//
	currentRule = getRule("Start");
	currentRule->immediate = ::StartTawkNow;
	addTest(5,(void*)getRule("Inheritance"),(char*)0,1,268435455,"defaultSKIP");
	//
	currentRule = getRule("StatementBody");
	currentRule->immediate = ::StatementBodyTawkNow;
	addTest(5,(void*)getRule("CommentBody"),"statement",1,1,"defaultSKIP");
	currentSet = commentSet;
	currentRule->guardSet = currentSet;
	currentTest->guardSet = currentSet;
	currentRule->next = getRule("StatementBody2");
	//
	currentRule = getRule("CommentBody");
	addTest(5,(void*)getRule("CodePass"),"comment",1,1,"defaultSKIP");
	currentRule->next = getRule("CommentBody2");
	//
	currentRule = getRule("Template2");
	addTest(5,(void*)getRule("Template2BalancE"),(char*)0,1,1,"defaultSKIP");
	//
	currentRule = getRule("StatementBody5");
	currentRule->immediate = ::StatementBody5TawkNow;
	addTest(7,(void*)"for",(char*)0,1,1,"defaultSKIP");
	currentSet = textFollow;
	addTest(6,(void*)currentSet,(char*)0,-1,1,(char*)0);
	addTest(5,(void*)getRule("Iterating"),(char*)0,0,1,"defaultSKIP");
	addTest(5,(void*)getRule("ForOption"),"instance",1,1,"defaultSKIP");
	addTest(5,(void*)getRule("Statement"),"statement",1,1,"defaultSKIP");
	currentRule->next = getRule("StatementBody6");
	//
	currentRule = getRule("MethodNameSet");
	currentSet = nameStartSet;
	addTest(6,(void*)currentSet,(char*)0,1,1,(char*)0);
	currentSet = methodNameSet;
	addTest(6,(void*)currentSet,(char*)0,0,268435455,(char*)0);
	//
	currentRule = getRule("CommentBody5");
	addTest(7,(void*)"#define",(char*)0,1,1,"defaultSKIP");
	addTest(5,(void*)getRule("EndComment"),"end",1,1,"defaultSKIP");
	//
	currentRule = getRule("NameSet");
	currentSet = nameStartSet;
	addTest(6,(void*)currentSet,(char*)0,1,1,(char*)0);
	currentSet = nameSet;
	addTest(6,(void*)currentSet,(char*)0,0,268435455,(char*)0);
	//
	currentRule = getRule("StatementBody6");
	currentRule->immediate = ::StatementBody6TawkNow;
	addTest(5,(void*)getRule("Print"),"statement",1,1,"defaultSKIP");
	addTest(7,(void*)";",(char*)0,1,1,"defaultSKIP");
	currentRule->next = getRule("StatementBody7");
	//
	currentRule = getRule("StatementBody7");
	currentRule->immediate = ::StatementBody7TawkNow;
	addTest(7,(void*)"while",(char*)0,1,1,"defaultSKIP");
	currentSet = textFollow;
	addTest(6,(void*)currentSet,(char*)0,-1,1,(char*)0);
	addTest(5,(void*)getRule("Iterating"),(char*)0,0,1,"defaultSKIP");
	addTest(5,(void*)getRule("Expression"),"instance",1,1,"defaultSKIP");
	addTest(5,(void*)getRule("Comment"),(char*)0,0,1,"defaultSKIP");
	addTest(5,(void*)getRule("Statement"),"statement",1,1,"defaultSKIP");
	currentRule->next = getRule("StatementBody8");
	//
	currentRule = getRule("StatementBody9");
	currentRule->immediate = ::StatementBody9TawkNow;
	addTest(7,(void*)"break","statement",1,1,"defaultSKIP");
	currentSet = textFollow;
	addTest(6,(void*)currentSet,(char*)0,-1,1,(char*)0);
	addTest(7,(void*)";",(char*)0,1,1,"defaultSKIP");
	currentRule->next = getRule("StatementBody10");
	//
	currentRule = getRule("StatementBody10");
	currentRule->immediate = ::StatementBody10TawkNow;
	addTest(7,(void*)"continue","statement",1,1,"defaultSKIP");
	currentSet = textFollow;
	addTest(6,(void*)currentSet,(char*)0,-1,1,(char*)0);
	addTest(7,(void*)";",(char*)0,1,1,"defaultSKIP");
	currentRule->next = getRule("StatementBody11");
	//
	currentRule = getRule("StatementBody11");
	currentRule->immediate = ::StatementBody11TawkNow;
	addTest(7,(void*)"goto","statement",1,1,"defaultSKIP");
	currentSet = textFollow;
	addTest(6,(void*)currentSet,(char*)0,-1,1,(char*)0);
	addTest(5,(void*)getRule("Indirection"),"direct",0,1,"defaultSKIP");
	addTest(5,(void*)getRule("Target"),"field",1,1,"defaultSKIP");
	addTest(7,(void*)";",(char*)0,1,1,"defaultSKIP");
	currentRule->next = getRule("StatementBody12");
	//
	currentRule = getRule("Replacement");
	addTest(5,(void*)getRule("Quote"),(char*)0,1,1,"defaultSKIP");
	currentRule->next = getRule("Replacement2");
	//
	currentRule = getRule("NewArray");
	addTest(7,(void*)"[",(char*)0,1,1,"defaultSKIP");
	addTest(5,(void*)getRule("Expression"),"instance",0,1,"defaultSKIP");
	addTest(7,(void*)"]",(char*)0,1,1,"defaultSKIP");
	//
	currentRule = getRule("StatementBody12");
	currentRule->immediate = ::StatementBody12TawkNow;
	addTest(5,(void*)getRule("Switch"),"statement",1,1,"defaultSKIP");
	addTest(5,(void*)getRule("Comment"),(char*)0,0,1,"defaultSKIP");
	addTest(5,(void*)getRule("Block"),"block",1,1,"defaultSKIP");
	currentTest->errorMessage = "Switch expected a block";
	currentRule->next = getRule("StatementBody13");
	//
	currentRule = getRule("ItemHead");
	currentRule->immediate = ::ItemHeadTawkNow;
	addTest(5,(void*)getRule("Indirection"),"direct",0,1,"defaultSKIP");
	addTest(5,(void*)getRule("Name"),"name",1,1,"defaultSKIP");
	addTest(5,(void*)getRule("ItemArray"),"array",0,268435455,"defaultSKIP");
	addTest(5,(void*)getRule("Bits"),"bits",0,1,"defaultSKIP");
	//
	currentRule = getRule("StatementBody15");
	currentRule->immediate = ::StatementBody15TawkNow;
	addTest(7,(void*)"delete","statement",1,1,"defaultSKIP");
	addTest(7,(void*)"[]","array",0,1,"defaultSKIP");
	addTest(5,(void*)getRule("Qualified"),"instance",1,1,"defaultSKIP");
	addTest(7,(void*)";",(char*)0,1,1,"defaultSKIP");
	currentRule->next = getRule("StatementBody16");
	//
	currentRule = getRule("StatementBody18");
	currentRule->immediate = ::StatementBody18TawkNow;
	addTest(5,(void*)getRule("Try"),"statement",1,1,"defaultSKIP");
	currentRule->next = getRule("StatementBody19");
	//
	currentRule = getRule("StatementBody19");
	currentRule->immediate = ::StatementBody19TawkNow;
	addTest(5,(void*)getRule("Declaration"),"statement",1,1,"defaultSKIP");
	addTest(7,(void*)";",(char*)0,1,1,"defaultSKIP");
	currentRule->next = getRule("StatementBody20");
	//
	currentRule = getRule("FieldComponent2");
	addTest(5,(void*)getRule("TypeName"),"name",1,1,"defaultSKIP");
	addTest(5,(void*)getRule("InstanceBody"),"body",1,1,"defaultSKIP");
	//
	currentRule = getRule("StatementBody20");
	currentRule->immediate = ::StatementBody20TawkNow;
	addTest(5,(void*)getRule("Expression"),"statement",1,1,"defaultSKIP");
	addTest(7,(void*)";",(char*)0,1,1,"defaultSKIP");
	currentRule->next = getRule("StatementBody21");
	//
	currentRule = getRule("ClassHeading3Block15");
	addTest(4,(void*)Structures,"kind",1,1,"defaultSKIP");
	currentSet = textFollow;
	addTest(6,(void*)currentSet,(char*)0,-1,1,(char*)0);
	//
	currentRule = getRule("EscapeCharacters");
	currentSet = getSet("nrtb\"\\");
	addTest(6,(void*)currentSet,(char*)0,1,1,(char*)0);
	currentRule->next = getRule("EscapeCharacters2");
	//
	currentRule = getRule("StatementBody21");
	currentRule->immediate = ::StatementBody21TawkNow;
	addTest(5,(void*)getRule("Lambda"),"statement",1,1,"defaultSKIP");
	//
	currentRule = getRule("ForOption2Block16");
	addTest(7,(void*)"on",(char*)0,1,1,"defaultSKIP");
	addTest(5,(void*)getRule("Name"),"name",1,1,"defaultSKIP");
	//
	currentRule = getRule("DeclareItem");
	currentRule->defer = ::DeclareItemTawkAct;
	addTest(5,(void*)getRule("MethodHead"),"item",1,1,"defaultSKIP");
	addTest(5,(void*)getRule("MethodInitializer"),"instance",0,1,"defaultSKIP");
	addTest(7,(void*)",",(char*)0,0,1,"defaultSKIP");
	addTest(5,(void*)getRule("Comment"),(char*)0,0,1,"defaultSKIP");
	currentRule->next = getRule("DeclareItem2");
	//
	currentRule = getRule("StructureItem");
	currentRule->defer = ::StructureItemTawkAct;
	addTest(5,(void*)getRule("Declaration"),"item",1,1,"defaultSKIP");
	addTest(7,(void*)";",(char*)0,0,1,"defaultSKIP");
	currentRule->next = getRule("StructureItem2");
	//
	currentRule = getRule("Switch");
	currentRule->immediate = ::SwitchTawkNow;
	addTest(7,(void*)"switch","statement",1,1,"defaultSKIP");
	currentSet = textFollow;
	addTest(6,(void*)currentSet,(char*)0,-1,1,(char*)0);
	addTest(5,(void*)getRule("FieldBody3"),"name",0,1,"defaultSKIP");
	currentTest->errorMessage = "Switch expected expression in parentheses";
	//
	currentRule = getRule("Target");
	currentRule->immediate = ::TargetTawkNow;
	addTest(5,(void*)getRule("Qualified"),"field",1,1,"defaultSKIP");
	currentRule->next = getRule("Target2");
	//
	currentRule = getRule("PrintTarget");
	addTest(7,(void*)"(",(char*)0,1,1,(char*)0);
	addTest(5,(void*)getRule("FieldExpression"),"instance",0,1,"defaultSKIP");
	addTest(7,(void*)")",(char*)0,1,1,"defaultSKIP");
	//
	currentRule = getRule("CastTail");
	currentRule->immediate = ::CastTailTawkNow;
	addTest(7,(void*)"(",(char*)0,1,1,"defaultSKIP");
	addTest(5,(void*)getRule("CastType"),"rest",1,268435455,"defaultSKIP");
	addTest(7,(void*)")",(char*)0,1,1,"defaultSKIP");
	//
	currentRule = getRule("DotH");
	//
	currentRule = getRule("Target2");
	currentRule->immediate = ::Target2TawkNow;
	addTest(5,(void*)getRule("Name"),"field",1,1,"defaultSKIP");
	//
	currentRule = getRule("Directivise");
	addTest(5,(void*)getRule("Line"),"line",1,268435455,"defaultSKIP");
	//
	currentRule = getRule("AliasTarget2");
	addTest(5,(void*)getRule("Name"),"target",1,1,"defaultSKIP");
	addTest(5,(void*)getRule("AliasBody"),"body",0,1,"defaultSKIP");
	//
	currentRule = getRule("PrintItem");
	addTest(5,(void*)getRule("PrintShortcut"),"instance",1,1,"defaultSKIP");
	currentRule->next = getRule("PrintItem2");
	//
	currentRule = getRule("Body");
	addTest(5,(void*)getRule("Commands"),(char*)0,1,1,"defaultSKIP");
	currentRule->next = getRule("Body2");
	//
	currentRule = getRule("MethodName");
	currentRule->immediate = ::MethodNameTawkNow;
	addTest(5,(void*)getRule("MethodNameSet"),"name",1,1,"defaultSKIP");
	//
	currentRule = getRule("Bump");
	addTest(5,(void*)getRule("BumpBlock3"),"bump",1,1,(char*)0);
	//
	currentRule = getRule("CodePass");
	addTest(7,(void*)"-%",(char*)0,1,1,"defaultSKIP");
	addTest(7,(void*)"%-","comment",0,1,(char*)0);
	currentTest->skipOverMatch = 1;
	currentTest->processUpTo = 1;
	//
	currentRule = getRule("Comment");
	currentRule->immediate = ::CommentTawkNow;
	addTest(5,(void*)getRule("CommentBody"),"comment",1,268435455,"defaultSKIP");
	//
	currentRule = getRule("EndComment");
	currentSet = getSet("\n");
	addTest(6,(void*)currentSet,(char*)0,1,268435455,(char*)0);
	//
	currentRule = getRule("Count");
	currentSet = getSet("01234567890-9");
	addTest(6,(void*)currentSet,(char*)0,1,268435455,(char*)0);
	//
	currentRule = getRule("ConditionList");
	//
	currentRule = getRule("NumberBlock6");
	currentSet = getSet("01234567890-9");
	addTest(6,(void*)currentSet,(char*)0,1,268435455,(char*)0);
	addTest(7,(void*)".",(char*)0,1,1,(char*)0);
	currentSet = getSet("01234567890-9");
	addTest(6,(void*)currentSet,(char*)0,1,268435455,(char*)0);
	addTest(5,(void*)getRule("NumberBlock6Block7"),(char*)0,0,1,(char*)0);
	//
	currentRule = getRule("Declaration");
	currentRule->defer = ::DeclarationTawkAct;
	currentRule->doNotGuard = 1;
	addTest(7,(void*)"outlet","outlet",0,1,"defaultSKIP");
	addTest(4,(void*)Linkage,"modify",0,268435455,"defaultSKIP");
	addTest(5,(void*)getRule("DeclareType"),"type",1,1,"defaultSKIP");
	addTest(5,(void*)getRule("DeclareItem"),"declare",1,268435455,"defaultSKIP");
	currentRule->next = getRule("Declaration2");
	//
	currentRule = getRule("CaseLabel3");
	currentRule->immediate = ::CaseLabel3TawkNow;
	addTest(5,(void*)getRule("Qualified"),"instance",1,1,"defaultSKIP");
	addTest(7,(void*)":",(char*)0,1,1,"defaultSKIP");
	currentTest->isIgnored = 1;
	currentRule->next = getRule("CaseLabel4");
	//
	currentRule = getRule("AliasTarget");
	addTest(5,(void*)getRule("Type"),"type",1,1,"defaultSKIP");
	addTest(5,(void*)getRule("Indirection"),"indirect",0,1,"defaultSKIP");
	currentRule->next = getRule("AliasTarget2");
	//
	currentRule = getRule("Escape");
	addTest(7,(void*)"\\",(char*)0,1,1,(char*)0);
	addTest(5,(void*)getRule("EscapeCharacters"),(char*)0,1,1,(char*)0);
	//
	currentRule = getRule("AliasItem4");
	currentRule->immediate = ::AliasItem4TawkNow;
	addTest(5,(void*)getRule("NameSet"),"alias",1,1,"defaultSKIP");
	addTest(5,(void*)getRule("AliasTarget"),"value",1,1,"defaultSKIP");
	//
	currentRule = getRule("EscapeCharacters4");
	currentSet = getSet("45674-7");
	addTest(6,(void*)currentSet,(char*)0,1,1,(char*)0);
	currentSet = getSet("01234567890-9");
	addTest(6,(void*)currentSet,(char*)0,0,1,(char*)0);
	//
	currentRule = getRule("ExpressList");
	currentRule->immediate = ::ExpressListTawkNow;
	addTest(5,(void*)getRule("NoShortcuts"),(char*)0,1,1,"defaultSKIP");
	addTest(5,(void*)getRule("ExpressItem"),"list",1,268435455,"defaultSKIP");
	currentRule->fail = ::instanceTailFail;
	//
	currentRule = getRule("MacroPart");
	currentSet = getSet(",(");
	addTest(6,(void*)currentSet,(char*)0,1,268435455,"defaultSKIP");
	currentRule->next = getRule("MacroPart2");
	//
	currentRule = getRule("Include3Block17");
	addTest(7,(void*)"include",(char*)0,1,1,"defaultSKIP");
	saveTest = currentTest;
	addTest(7,(void*)"import",(char*)0,1,1,"defaultSKIP");
	saveTest->setAlternate(currentTest);
	//
	currentRule = getRule("ExpressPart");
	currentRule->immediate = ::ExpressPartTawkNow;
	addTest(5,(void*)getRule("SaveVirtuals"),(char*)0,1,1,"defaultSKIP");
	addTest(5,(void*)getRule("UnaryOperator"),"unaryOp",0,1,"defaultSKIP");
	addTest(5,(void*)getRule("UnaryExpression"),"instance",1,1,"defaultSKIP");
	addTest(5,(void*)getRule("RangeTail"),(char*)0,-1,1,"defaultSKIP");
	addTest(5,(void*)getRule("ExpressType"),(char*)0,0,1,"defaultSKIP");
	addTest(5,(void*)getRule("OperationTail"),"express",0,268435455,"defaultSKIP");
	//
	currentRule = getRule("ClassAttributes4");
	currentRule->defer = ::ClassAttributes4TawkAct;
	addTest(7,(void*)"namespace",(char*)0,1,1,"defaultSKIP");
	addTest(5,(void*)getRule("NameSet"),"nSpace",1,1,"defaultSKIP");
	//
	currentRule = getRule("DeclareConditions");
	addTest(7,(void*)"Conditions",(char*)0,1,1,"defaultSKIP");
	addTest(5,(void*)getRule("ConditionLabel"),(char*)0,1,268435455,"defaultSKIP");
	//
	currentRule = getRule("Field");
	currentRule->immediate = ::FieldTawkNow;
	addTest(5,(void*)getRule("Name"),"name",1,1,"defaultSKIP");
	//
	currentRule = getRule("ParameterList");
	addTest(5,(void*)getRule("ExpressItem"),"expression",1,268435455,"defaultSKIP");
	currentRule->next = getRule("ParameterList2");
	//
	currentRule = getRule("Initializer");
	currentRule->immediate = ::InitializerTawkNow;
	addTest(5,(void*)getRule("ExpressList"),"instance",1,1,"defaultSKIP");
	addTest(7,(void*)",",(char*)0,0,1,"defaultSKIP");
	currentRule->next = getRule("Initializer2");
	//
	currentRule = getRule("ArrayInitializer");
	currentRule->immediate = ::ArrayInitializerTawkNow;
	addTest(7,(void*)"{",(char*)0,1,1,"defaultSKIP");
	addTest(5,(void*)getRule("Initializer"),"instance",0,268435455,"defaultSKIP");
	addTest(7,(void*)"}",(char*)0,1,1,"defaultSKIP");
	//
	currentRule = getRule("CommentBody4Any");
	currentRule->immediate = ::balancEbail;
	addTest(1,(void*)0,(char*)0,1,1,(char*)0);
	//
	currentRule = getRule("ItemInitializer");
	addTest(7,(void*)"=",(char*)0,1,1,"defaultSKIP");
	addTest(5,(void*)getRule("SetObject"),(char*)0,0,1,"defaultSKIP");
	addTest(5,(void*)getRule("ItemInitializerBlock12"),"instance",1,1,"defaultSKIP");
	//
	currentRule = getRule("MethodInitializer");
	addTest(7,(void*)"=",(char*)0,1,1,"defaultSKIP");
	addTest(5,(void*)getRule("FieldExpression"),"instance",1,1,"defaultSKIP");
	//
	currentRule = getRule("RangeField2");
	addTest(5,(void*)getRule("RangeExpression"),"instance",1,1,"defaultSKIP");
	//
	currentRule = getRule("InstanceBody");
	addTest(5,(void*)getRule("InstanceTail"),"body",1,1,"defaultSKIP");
	//
	currentRule = getRule("Quote");
	currentRule->immediate = ::QuoteTawkNow;
	addTest(7,(void*)"@","string",0,1,(char*)0);
	addTest(7,(void*)"\"","instance",1,1,(char*)0);
	addTest(7,(void*)"\"","body",0,1,(char*)0);
	currentTest->skipOverMatch = 1;
	currentTest->processUpTo = 1;
	//
	currentRule = getRule("Label");
	addTest(5,(void*)getRule("Name"),"name",1,1,(char*)0);
	addTest(7,(void*)":",(char*)0,1,1,(char*)0);
	//
	currentRule = getRule("MethodParameters");
	addTest(7,(void*)"(",(char*)0,1,1,"defaultSKIP");
	addTest(5,(void*)getRule("Parameter"),"parameter",0,268435455,"defaultSKIP");
	addTest(7,(void*)"...","ellipsis",0,1,"defaultSKIP");
	addTest(7,(void*)")",(char*)0,1,1,"defaultSKIP");
	//
	currentRule = getRule("Initializer2");
	addTest(5,(void*)getRule("ArrayInitializer"),"instance",1,1,"defaultSKIP");
	addTest(7,(void*)",",(char*)0,0,1,"defaultSKIP");
	//
	currentRule = getRule("AssumedString");
	currentRule->immediate = ::AssumedStringTawkNow;
	addTest(5,(void*)getRule("NameSet"),"instance",1,1,(char*)0);
	//
	currentRule = getRule("MacroParameters");
	addTest(7,(void*)"(",(char*)0,1,1,"defaultSKIP");
	addTest(5,(void*)getRule("MacroElement"),"parameters",1,268435455,"defaultSKIP");
	addTest(7,(void*)")",(char*)0,1,1,"defaultSKIP");
	//
	currentRule = getRule("AliasParameters");
	addTest(5,(void*)getRule("AliasParameter"),"body",0,268435455,"defaultSKIP");
	//
	currentRule = getRule("InitializerItem");
	currentRule->immediate = ::InitializerItemTawkNow;
	addTest(5,(void*)getRule("NameSet"),"field",1,1,"defaultSKIP");
	addTest(5,(void*)getRule("NameSet"),"function",1,1,"defaultSKIP");
	//
	currentRule = getRule("CheckMacroParameters");
	currentRule->immediate = ::CheckMacroParametersTawkNow;
	addTest(5,(void*)getRule("CheckMacroParametersBalancE"),"braced",1,1,"defaultSKIP");
	//
	currentRule = getRule("Modify");
	//
	currentRule = getRule("Name");
	currentRule->immediate = ::NameTawkNow;
	addTest(5,(void*)getRule("NameSet"),"name",1,1,"defaultSKIP");
	//
	currentRule = getRule("NotQuote2");
	//
	currentRule = getRule("Number");
	currentRule->immediate = ::NumberTawkNow;
	addTest(5,(void*)getRule("NumberBlock6"),"instance",1,1,(char*)0);
	currentRule->next = getRule("Number2");
	//
	currentRule = getRule("Operator");
	currentRule->immediate = ::OperatorTawkNow;
	addTest(4,(void*)Operators,"operand",1,1,"defaultSKIP");
	currentSet = operatorSet;
	currentRule->guardSet = currentSet;
	currentTest->guardSet = currentSet;
	currentRule->next = getRule("Operator2");
	//
	currentRule = getRule("UnaryOperator");
	addTest(5,(void*)getRule("Bump"),"operate",1,1,"defaultSKIP");
	currentRule->next = getRule("UnaryOperator2");
	//
	currentRule = getRule("Path");
	addTest(7,(void*)"/",(char*)0,0,1,(char*)0);
	addTest(5,(void*)getRule("PathBlock11"),(char*)0,1,268435455,(char*)0);
	//
	currentRule = getRule("Imports");
	addTest(5,(void*)getRule("Include"),(char*)0,1,1,"defaultSKIP");
	currentRule->next = getRule("Imports2");
	//
	currentRule = getRule("PrintSet");
	//
	currentRule = getRule("Inheritance2");
	addTest(5,(void*)getRule("Imports"),(char*)0,1,1,"defaultSKIP");
	currentRule->next = getRule("Inheritance3");
	//
	currentRule = getRule("Question");
	currentRule->immediate = ::QuestionTawkNow;
	addTest(7,(void*)"?","question",1,1,"defaultSKIP");
	addTest(5,(void*)getRule("Expression"),"trueExp",1,1,"defaultSKIP");
	addTest(7,(void*)":",(char*)0,1,1,"defaultSKIP");
	addTest(5,(void*)getRule("Expression"),"falseExp",1,1,"defaultSKIP");
	//
	currentRule = getRule("RangeField");
	currentRule->immediate = ::RangeFieldTawkNow;
	addTest(5,(void*)getRule("Name"),"instance",1,1,"defaultSKIP");
	currentRule->next = getRule("RangeField2");
	//
	currentRule = getRule("RuleList");
	addTest(7,(void*)"Rule",(char*)0,1,1,"defaultSKIP");
	currentSet = textFollow;
	addTest(6,(void*)currentSet,(char*)0,-1,1,(char*)0);
	addTest(5,(void*)getRule("DebugRule"),"debugRULE",1,268435455,"defaultSKIP");
	addTest(7,(void*)";",(char*)0,1,1,"defaultSKIP");
	//
	currentRule = getRule("ClassName");
	currentRule->immediate = ::ClassNameTawkNow;
	addTest(5,(void*)getRule("ClassAttributes"),(char*)0,-1,1,"defaultSKIP");
	addTest(5,(void*)getRule("Path"),"path",0,1,"defaultSKIP");
	addTest(5,(void*)getRule("NameSet"),"name",1,1,(char*)0);
	addTest(5,(void*)getRule("Template"),"temp",0,1,"defaultSKIP");
	addTest(7,(void*)".h","dotH",0,1,(char*)0);
	//
	currentRule = getRule("Statement");
	addTest(5,(void*)getRule("CheckMacro"),"statement",1,1,"defaultSKIP");
	currentRule->next = getRule("Statement2");
	//
	currentRule = getRule("FieldList");
	addTest(7,(void*)"Field",(char*)0,1,1,"defaultSKIP");
	currentSet = textFollow;
	addTest(6,(void*)currentSet,(char*)0,-1,1,(char*)0);
	currentSet = getSet("ABCDEFGHIJKLMNOPQRSTUVWXYZA-Zabcdefghijklmnopqrstuvwxyza-z01234567890-9_()*");
	addTest(6,(void*)currentSet,"name",0,268435455,"defaultSKIP");
	addTest(7,(void*)";",(char*)0,1,1,"defaultSKIP");
	currentRule->next = getRule("FieldList2");
	//
	currentRule = getRule("Strings");
	currentRule->immediate = ::StringsTawkNow;
	addTest(5,(void*)getRule("StringExpression"),"item",1,268435455,"defaultSKIP");
	currentSet = getSet(";");
	currentRule->guardSet = currentSet;
	currentTest->guardSet = currentSet;
	//
	currentRule = getRule("MacroPart2");
	addTest(5,(void*)getRule("Braced"),(char*)0,1,1,"defaultSKIP");
	//
	currentRule = getRule("SyntaxExtensions");
	addTest(7,(void*)"overload",(char*)0,1,1,"defaultSKIP");
	currentSet = textFollow;
	addTest(6,(void*)currentSet,(char*)0,-1,1,(char*)0);
	addTest(5,(void*)getRule("OverLoadItem"),(char*)0,1,268435455,"defaultSKIP");
	addTest(7,(void*)";",(char*)0,1,1,"defaultSKIP");
	currentRule->next = getRule("SyntaxExtensions2");
	//
	currentRule = getRule("ItemInitializerBlock12");
	addTest(5,(void*)getRule("ArrayInitializer"),"instance",1,1,"defaultSKIP");
	saveTest = currentTest;
	addTest(5,(void*)getRule("InitExpression"),"instance",1,1,"defaultSKIP");
	saveTest->setAlternate(currentTest);
	//
	currentRule = getRule("Template");
	addTest(7,(void*)"<>",(char*)0,1,1,(char*)0);
	currentRule->next = getRule("Template2");
	//
	currentRule = getRule("PrimaryExpression3");
	addTest(5,(void*)getRule("FieldExpression"),"instance",1,1,"defaultSKIP");
	currentRule->next = getRule("PrimaryExpression4");
	//
	currentRule = getRule("Body4");
	addTest(5,(void*)getRule("Comment"),(char*)0,1,1,"defaultSKIP");
	currentSet = commentSet;
	currentRule->guardSet = currentSet;
	currentTest->guardSet = currentSet;
	currentRule->next = getRule("Body5");
	//
	currentRule = getRule("FieldList2");
	addTest(7,(void*)"Map;",(char*)0,1,1,"defaultSKIP");
	//
	currentRule = getRule("UnaryExpression3");
	addTest(5,(void*)getRule("AllowShortcuts"),(char*)0,1,1,"defaultSKIP");
	addTest(5,(void*)getRule("PrintShortcut"),"instance",1,1,"defaultSKIP");
	//
	currentRule = getRule("Try");
	addTest(7,(void*)"try",(char*)0,1,1,"defaultSKIP");
	addTest(5,(void*)getRule("Statement"),"statement",1,1,"defaultSKIP");
	addTest(5,(void*)getRule("ResetType"),(char*)0,1,1,"defaultSKIP");
	addTest(5,(void*)getRule("Catch"),"catch",0,268435455,"defaultSKIP");
	addTest(5,(void*)getRule("Final"),"end",0,1,"defaultSKIP");
	//
	currentRule = getRule("StructureBody2");
	currentRule->defer = ::StructureBody2TawkAct;
	addTest(5,(void*)getRule("StructureItem"),"entry",1,268435455,"defaultSKIP");
	//
	currentRule = getRule("Type");
	currentRule->immediate = ::TypeTawkNow;
	currentRule->doNotGuard = 1;
	addTest(7,(void*)"const","hasConst",0,1,"defaultSKIP");
	addTest(7,(void*)"unsigned","noSign",0,1,"defaultSKIP");
	addTest(5,(void*)getRule("TypeName"),"type",1,1,"defaultSKIP");
	addTest(5,(void*)getRule("Template"),"temp",0,1,"defaultSKIP");
	//
	currentRule = getRule("TypeList");
	currentRule->immediate = ::TypeListTawkNow;
	addTest(5,(void*)getRule("Type"),"instance",1,1,"defaultSKIP");
	addTest(7,(void*)",",(char*)0,0,1,"defaultSKIP");
	//
	currentRule = getRule("Include2BalancE");
	currentRule->immediate = ::balancE;
	addTest(7,(void*)"#import","start",1,1,(char*)0);
	currentTest->leftBalance = 1;
	addTest(5,(void*)getRule("Include2BoDY"),"body",0,268435455,(char*)0);
	addTest(7,(void*)"\n","finish",1,1,(char*)0);
	currentTest->rightBalance = 1;
	//
	currentRule = getRule("IncludeBalancE");
	currentRule->immediate = ::balancE;
	addTest(7,(void*)"#include","start",1,1,(char*)0);
	currentTest->leftBalance = 1;
	addTest(5,(void*)getRule("IncludeBoDY"),"body",0,268435455,(char*)0);
	addTest(7,(void*)"\n","finish",1,1,(char*)0);
	currentTest->rightBalance = 1;
	//
	currentRule = getRule("Template2BalancE");
	currentRule->immediate = ::balancE;
	addTest(7,(void*)"<","start",1,1,"defaultSKIP");
	currentTest->leftBalance = 1;
	addTest(5,(void*)getRule("Template2BoDY"),"body",0,268435455,"defaultSKIP");
	addTest(7,(void*)">","finish",1,1,(char*)0);
	currentTest->rightBalance = 1;
	//
	currentRule = getRule("CommentBody4BalancE");
	currentRule->immediate = ::balancE;
	addTest(7,(void*)"#ifdef","start",1,1,"defaultSKIP");
	currentTest->leftBalance = 1;
	addTest(5,(void*)getRule("CommentBody4BoDY"),"body",0,268435455,"defaultSKIP");
	addTest(7,(void*)"#endif","finish",1,1,(char*)0);
	currentTest->rightBalance = 1;
	//
	currentRule = getRule("CommentBody3BalancE");
	currentRule->immediate = ::balancE;
	addTest(7,(void*)"/*","start",1,1,"defaultSKIP");
	currentTest->leftBalance = 1;
	addTest(5,(void*)getRule("CommentBody3BoDY"),"body",0,268435455,"defaultSKIP");
	addTest(7,(void*)"*/","finish",1,1,(char*)0);
	currentTest->rightBalance = 1;
	//
	currentRule = getRule("ClassAttributes");
	currentRule->defer = ::ClassAttributesTawkAct;
	addTest(4,(void*)Attributes,"trait",1,1,"defaultSKIP");
	currentSet = textFollow;
	addTest(6,(void*)currentSet,(char*)0,-1,1,(char*)0);
	currentRule->next = getRule("ClassAttributes2");
	//
	currentRule = getRule("CheckMacroParametersBalancE");
	currentRule->immediate = ::balancE;
	addTest(7,(void*)"(","start",1,1,"defaultSKIP");
	currentTest->leftBalance = 1;
	addTest(5,(void*)getRule("CheckMacroParametersBoDY"),"body",0,268435455,"defaultSKIP");
	addTest(7,(void*)")","finish",1,1,(char*)0);
	currentTest->rightBalance = 1;
	//
	currentRule = getRule("BracedBalancE");
	currentRule->immediate = ::balancE;
	addTest(7,(void*)"(","start",1,1,"defaultSKIP");
	currentTest->leftBalance = 1;
	addTest(5,(void*)getRule("BracedBoDY"),"body",0,268435455,"defaultSKIP");
	addTest(7,(void*)")","finish",1,1,(char*)0);
	currentTest->rightBalance = 1;
	//
	currentRule = getRule("AliasBodyBalancE");
	currentRule->immediate = ::balancE;
	addTest(7,(void*)"(","start",1,1,"defaultSKIP");
	currentTest->leftBalance = 1;
	addTest(5,(void*)getRule("AliasBodyBoDY"),"body",0,268435455,"defaultSKIP");
	addTest(7,(void*)")","finish",1,1,(char*)0);
	currentTest->rightBalance = 1;
	//
	currentRule = getRule("AliasItem");
	currentRule->immediate = ::AliasItemTawkNow;
	addTest(5,(void*)getRule("Field"),"name",1,1,"defaultSKIP");
	addTest(5,(void*)getRule("TargetMethod"),"target",1,1,"defaultSKIP");
	currentRule->next = getRule("AliasItem2");
	//
	currentRule = getRule("CheckMacro");
	currentRule->immediate = ::CheckMacroTawkNow;
	addTest(5,(void*)getRule("MacroName"),"statement",1,1,"defaultSKIP");
	addTest(5,(void*)getRule("CheckMacroParameters"),"braced",0,1,"defaultSKIP");
	addTest(7,(void*)";",(char*)0,0,1,"defaultSKIP");
	//
	currentRule = getRule("ConditionLabel");
	currentRule->immediate = ::ConditionLabelTawkNow;
	addTest(5,(void*)getRule("Name"),"label",1,1,"defaultSKIP");
	currentSet = getSet("t");
	addTest(6,(void*)currentSet,(char*)0,0,1,"defaultSKIP");
	addTest(7,(void*)"\n","text",0,1,(char*)0);
	currentTest->skipOverMatch = 1;
	currentTest->processUpTo = 1;
	//
	currentRule = getRule("NumberBlock6Block7");
	currentSet = getSet("eE");
	addTest(6,(void*)currentSet,(char*)0,1,1,(char*)0);
	currentSet = getSet("-+");
	addTest(6,(void*)currentSet,(char*)0,0,1,(char*)0);
	currentSet = getSet("01234567890-9");
	addTest(6,(void*)currentSet,(char*)0,1,268435455,(char*)0);
	//
	currentRule = getRule("Extender");
	currentRule->immediate = ::ExtenderTawkNow;
	addTest(5,(void*)getRule("Name"),"name",1,1,"defaultSKIP");
	//
	currentRule = getRule("QualifyType");
	addTest(5,(void*)getRule("Type"),"type",1,1,"defaultSKIP");
	addTest(7,(void*)".",(char*)0,1,1,"defaultSKIP");
	//
	currentRule = getRule("OverLoadItem");
	currentRule->immediate = ::OverLoadItemTawkNow;
	addTest(5,(void*)getRule("Operator"),"operate",1,1,"defaultSKIP");
	addTest(5,(void*)getRule("Name"),"name",1,1,"defaultSKIP");
	currentRule->next = getRule("OverLoadItem2");
	//
	currentRule = getRule("ButtonArray");
	currentRule->defer = ::ButtonArrayTawkAct;
	addTest(7,(void*)"[",(char*)0,1,1,"defaultSKIP");
	addTest(5,(void*)getRule("Name"),"button",1,268435455,"defaultSKIP");
	addTest(7,(void*)"]",(char*)0,1,1,"defaultSKIP");
	//
	currentRule = getRule("OverLoadItem2");
	currentRule->immediate = ::OverLoadItem2TawkNow;
	addTest(7,(void*)"[]",(char*)0,1,1,"defaultSKIP");
	addTest(7,(void*)"=","assign",0,1,"defaultSKIP");
	addTest(5,(void*)getRule("Name"),"name",1,1,"defaultSKIP");
	currentRule->next = getRule("OverLoadItem3");
	//
	currentRule = getRule("OverLoadItem3");
	currentRule->immediate = ::OverLoadItem3TawkNow;
	addTest(5,(void*)getRule("Bump"),"operate",1,1,"defaultSKIP");
	addTest(5,(void*)getRule("Name"),"name",1,1,"defaultSKIP");
	currentRule->next = getRule("OverLoadItem4");
	//
	currentRule = getRule("MacroBit");
	currentRule->immediate = ::MacroBitTawkNow;
	currentSet = getSet("abcdefghijklmnopqrstuvwxyza-zABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`A-z01234567890-9");
	addTest(6,(void*)currentSet,"bitpart",1,268435455,(char*)0);
	//
	currentRule = getRule("DeclareItem3");
	currentRule->defer = ::DeclareItem3TawkAct;
	addTest(5,(void*)getRule("ItemHead"),"item",1,1,"defaultSKIP");
	addTest(5,(void*)getRule("ItemInitializer"),"assign",0,1,"defaultSKIP");
	addTest(7,(void*)":","initialize",0,1,"defaultSKIP");
	addTest(7,(void*)",",(char*)0,0,1,"defaultSKIP");
	addTest(5,(void*)getRule("Comment"),(char*)0,0,1,"defaultSKIP");
	//
	currentRule = getRule("Assuming");
	currentRule->immediate = ::AssumingTawkNow;
	//
	currentRule = getRule("MacroBody");
	currentRule->immediate = ::MacroBodyTawkNow;
	addTest(5,(void*)getRule("MacroBodyPart"),"parts",1,268435455,(char*)0);
	//
	currentRule = getRule("MacroDelimit");
	currentRule->immediate = ::MacroDelimitTawkNow;
	currentSet = getSet("abcdefghijklmnopqrstuvwxyza-zABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`A-z01234567890-9;");
	addTest(6,(void*)currentSet,"delimiter",1,1,"defaultSKIP");
	//
	currentRule = getRule("MacroDefine");
	currentRule->immediate = ::MacroDefineTawkNow;
	addTest(5,(void*)getRule("Name"),"name",1,1,"defaultSKIP");
	addTest(5,(void*)getRule("MacroParameters"),"parameters",0,1,"defaultSKIP");
	addTest(5,(void*)getRule("MacroDelimit"),(char*)0,1,1,"defaultSKIP");
	addTest(7,(void*)&macroDelimiter,"body",0,1,(char*)0);
	currentTest->aVariable = 1;
	currentTest->skipOverMatch = 1;
	currentTest->processUpTo = 1;
	//
	currentRule = getRule("Replacement2");
	currentSet = getSet(",n");
	addTest(6,(void*)currentSet,(char*)0,1,268435455,"defaultSKIP");
	//
	currentRule = getRule("DeclareItem2");
	currentRule->defer = ::DeclareItem2TawkAct;
	addTest(5,(void*)getRule("Name"),"item",1,1,"defaultSKIP");
	addTest(7,(void*)"(",(char*)0,1,1,"defaultSKIP");
	addTest(5,(void*)getRule("Expression"),"argument",1,1,"defaultSKIP");
	addTest(7,(void*)")",(char*)0,1,1,"defaultSKIP");
	currentRule->next = getRule("DeclareItem3");
	//
	currentRule = getRule("StructureItem2");
	currentRule->defer = ::StructureItem2TawkAct;
	addTest(5,(void*)getRule("Name"),"name",1,1,"defaultSKIP");
	addTest(5,(void*)getRule("Bits"),"bits",0,1,"defaultSKIP");
	addTest(5,(void*)getRule("ButtonArray"),"buttons",0,1,"defaultSKIP");
	addTest(7,(void*)",",(char*)0,0,1,"defaultSKIP");
	//
	currentRule = getRule("AliasParameterBlock0");
	addTest(7,(void*)"=",(char*)0,1,1,"defaultSKIP");
	addTest(5,(void*)getRule("Replacement"),"replacedBy",1,1,"defaultSKIP");
	//
	currentRule = getRule("AliasBodyAny");
	currentRule->immediate = ::balancEbail;
	addTest(1,(void*)0,(char*)0,1,1,(char*)0);
	//
	currentRule = getRule("AliasParameter2");
	addTest(5,(void*)getRule("Replacement"),"parameter",1,1,"defaultSKIP");
	addTest(7,(void*)",",(char*)0,0,1,"defaultSKIP");
	//
	currentRule = getRule("AliasBody");
	addTest(5,(void*)getRule("AliasBodyBalancE"),"body",1,1,"defaultSKIP");
	//
	currentRule = getRule("LambdaName");
	currentRule->immediate = ::LambdaNameTawkNow;
	addTest(5,(void*)getRule("NameSet"),"name",1,1,"defaultSKIP");
	//
	currentRule = getRule("AliasBodyBoDY");
	currentRule->immediate = ::balancEbody;
	addTest(7,(void*)"(","begin",0,1,(char*)0);
	addTest(7,(void*)")","end",0,1,(char*)0);
	currentRule->next = getRule("AliasBodyAny");
	//
	currentRule = getRule("ExpressType");
	currentRule->immediate = ::ExpressTypeTawkNow;
	//
	currentRule = getRule("AliasItem3");
	addTest(5,(void*)getRule("Comment"),(char*)0,1,1,"defaultSKIP");
	currentRule->next = getRule("AliasItem4");
	//
	currentRule = getRule("EscapeCharacters3");
	currentSet = getSet("01230-3");
	addTest(6,(void*)currentSet,(char*)0,1,1,(char*)0);
	currentSet = getSet("01234567890-9");
	addTest(6,(void*)currentSet,(char*)0,0,1,(char*)0);
	currentSet = getSet("01234567890-9");
	addTest(6,(void*)currentSet,(char*)0,0,1,(char*)0);
	currentRule->next = getRule("EscapeCharacters4");
	//
	currentRule = getRule("Final");
	addTest(7,(void*)"finally",(char*)0,1,1,"defaultSKIP");
	addTest(5,(void*)getRule("Statement"),"statement",1,1,"defaultSKIP");
	addTest(5,(void*)getRule("ResetType"),(char*)0,0,1,"defaultSKIP");
	//
	currentRule = getRule("OverLoadItem4Block1");
	currentSet = operatorSet;
	addTest(6,(void*)currentSet,(char*)0,1,268435455,"defaultSKIP");
	//
	currentRule = getRule("MacroElement");
	currentSet = getSet(",)");
	addTest(6,(void*)currentSet,"element",1,268435455,"defaultSKIP");
	addTest(7,(void*)",",(char*)0,0,1,"defaultSKIP");
	//
	currentRule = getRule("Declaration3");
	addTest(5,(void*)getRule("DeclareConditions"),(char*)0,1,1,"defaultSKIP");
	//
	currentRule = getRule("MacroBodyPart");
	currentRule->doNotGuard = 1;
	addTest(5,(void*)getRule("MacroBit"),"other",0,1,(char*)0);
	currentTest->skipOverMatch = 1;
	currentTest->processUpTo = 1;
	currentRule->next = getRule("MacroBodyPart2");
	//
	currentRule = getRule("MacroBodyPart2");
	addTest(1,(void*)0,"rest",1,268435455,(char*)0);
	//
	currentRule = getRule("CharacterBlock4Block5");
	addTest(5,(void*)getRule("Escape"),(char*)0,1,1,(char*)0);
	saveTest = currentTest;
	currentSet = getSet("'");
	addTest(6,(void*)currentSet,(char*)0,1,1,(char*)0);
	saveTest->setAlternate(currentTest);
	//
	currentRule = getRule("Braced");
	addTest(5,(void*)getRule("BracedBalancE"),(char*)0,1,1,"defaultSKIP");
	//
	currentRule = getRule("Case");
	currentRule->immediate = ::CaseTawkNow;
	addTest(7,(void*)"default:","instance",1,1,"defaultSKIP");
	currentRule->next = getRule("Case2");
	//
	currentRule = getRule("BracedBoDY");
	currentRule->immediate = ::balancEbody;
	addTest(7,(void*)"(","begin",0,1,(char*)0);
	addTest(7,(void*)")","end",0,1,(char*)0);
	currentRule->next = getRule("BracedAny");
	//
	currentRule = getRule("Case2");
	currentRule->immediate = ::Case2TawkNow;
	addTest(7,(void*)"case",(char*)0,1,1,"defaultSKIP");
	currentSet = textFollow;
	addTest(6,(void*)currentSet,(char*)0,-1,1,(char*)0);
	addTest(5,(void*)getRule("Assuming"),(char*)0,1,1,"defaultSKIP");
	addTest(5,(void*)getRule("CaseLabel"),"instance",1,1,"defaultSKIP");
	addTest(7,(void*)":",(char*)0,1,1,"defaultSKIP");
	currentRule->next = getRule("Case3");
	currentRule->fail = ::caseLabelFail;
	//
	currentRule = getRule("BracedAny");
	currentRule->immediate = ::balancEbail;
	addTest(1,(void*)0,(char*)0,1,1,(char*)0);
	//
	currentRule = getRule("Constant4");
	currentRule->immediate = ::Constant4TawkNow;
	addTest(7,(void*)"true","instance",1,1,"defaultSKIP");
	currentSet = textFollow;
	addTest(6,(void*)currentSet,(char*)0,-1,1,(char*)0);
	currentRule->next = getRule("Constant5");
	//
	currentRule = getRule("MacroArgument");
	addTest(5,(void*)getRule("MacroArgumentBlock2"),"part",1,1,"defaultSKIP");
	addTest(7,(void*)",",(char*)0,0,1,"defaultSKIP");
	//
	currentRule = getRule("MacroArgumentBlock2");
	addTest(5,(void*)getRule("MacroPart"),(char*)0,1,268435455,"defaultSKIP");
	//
	currentRule = getRule("IncludeAny");
	currentRule->immediate = ::balancEbail;
	addTest(1,(void*)0,(char*)0,1,1,(char*)0);
	//
	currentRule = getRule("MacroArgumentList");
	addTest(5,(void*)getRule("MacroArgument"),"argument",1,268435455,"defaultSKIP");
	//
	currentRule = getRule("Line2");
	addTest(7,(void*)"}",(char*)0,-1,1,(char*)0);
	addTest(5,(void*)getRule("Statement"),"statement",1,1,"defaultSKIP");
	addTest(5,(void*)getRule("ResetType"),(char*)0,0,1,"defaultSKIP");
	currentRule->next = getRule("Line3");
	//
	currentRule = getRule("CheckMacroParametersBoDY");
	currentRule->immediate = ::balancEbody;
	addTest(7,(void*)"(","begin",0,1,(char*)0);
	addTest(7,(void*)")","end",0,1,(char*)0);
	currentRule->next = getRule("CheckMacroParametersAny");
	//
	currentRule = getRule("CheckMacroParametersAny");
	currentRule->immediate = ::balancEbail;
	addTest(1,(void*)0,(char*)0,1,1,(char*)0);
	//
	currentRule = getRule("CodeMatch2");
	addTest(4,(void*)Directives,(char*)0,-1,1,"defaultSKIP");
	currentSet = space;
	addTest(6,(void*)currentSet,"body",0,1,(char*)0);
	currentTest->processUpTo = 1;
	//
	currentRule = getRule("OperationTail2");
	currentRule->immediate = ::OperationTail2TawkNow;
	addTest(5,(void*)getRule("Question"),"instance",1,1,"defaultSKIP");
	currentRule->next = getRule("OperationTail3");
	//
	currentRule = getRule("Case3");
	currentRule->immediate = ::Case3TawkNow;
	addTest(5,(void*)getRule("Label"),"instance",1,1,"defaultSKIP");
	//
	currentRule = getRule("CaseLabel");
	currentRule->immediate = ::CaseLabelTawkNow;
	addTest(5,(void*)getRule("Constant"),"instance",1,1,"defaultSKIP");
	addTest(7,(void*)":",(char*)0,1,1,"defaultSKIP");
	currentTest->isIgnored = 1;
	currentRule->next = getRule("CaseLabel2");
	//
	currentRule = getRule("CaseLabel5");
	currentRule->immediate = ::CaseLabel5TawkNow;
	addTest(5,(void*)getRule("Name"),"instance",1,1,"defaultSKIP");
	//
	currentRule = getRule("ClassAttributes2");
	currentRule->defer = ::ClassAttributes2TawkAct;
	addTest(7,(void*)"extends",(char*)0,1,1,"defaultSKIP");
	addTest(5,(void*)getRule("Type"),"type",1,1,"defaultSKIP");
	currentRule->next = getRule("ClassAttributes3");
	//
	currentRule = getRule("ClassAttributes3");
	currentRule->defer = ::ClassAttributes3TawkAct;
	addTest(7,(void*)"implements",(char*)0,1,1,"defaultSKIP");
	addTest(5,(void*)getRule("Type"),"proto",1,268435455,"defaultSKIP");
	currentRule->next = getRule("ClassAttributes4");
	//
	currentRule = getRule("Constant5");
	currentRule->immediate = ::Constant5TawkNow;
	addTest(7,(void*)"false","instance",1,1,"defaultSKIP");
	currentSet = textFollow;
	addTest(6,(void*)currentSet,(char*)0,-1,1,(char*)0);
	//
	currentRule = getRule("Commands");
	addTest(7,(void*)"#",(char*)0,1,1,"defaultSKIP");
	addTest(5,(void*)getRule("PoundCommand"),(char*)0,1,1,"defaultSKIP");
	currentRule->next = getRule("Commands2");
	//
	currentRule = getRule("DebugDirective");
	currentRule->defer = ::DebugDirectiveTawkAct;
	addTest(7,(void*)"#",(char*)0,-1,1,"defaultSKIP");
	addTest(5,(void*)getRule("Comment"),(char*)0,0,1,"defaultSKIP");
	addTest(5,(void*)getRule("Name"),"method",1,1,"defaultSKIP");
	addTest(5,(void*)getRule("CodeMatch"),"body",0,1,"defaultSKIP");
	addTest(4,(void*)Directives,"locate",0,1,"defaultSKIP");
	addTest(7,(void*)"active","active",0,1,"defaultSKIP");
	addTest(7,(void*)"#;","code",0,1,(char*)0);
	currentTest->skipOverMatch = 1;
	currentTest->processUpTo = 1;
	//
	currentRule = getRule("Directive");
	currentRule->immediate = ::DirectiveTawkNow;
	addTest(5,(void*)getRule("Type"),"type",1,1,"defaultSKIP");
	addTest(5,(void*)getRule("DebugDirective"),"directives",0,268435455,"defaultSKIP");
	//
	currentRule = getRule("FileName");
	currentRule->immediate = ::FileNameTawkNow;
	addTest(5,(void*)getRule("Path"),"path",0,1,(char*)0);
	addTest(5,(void*)getRule("NameSet"),"name",1,1,(char*)0);
	addTest(7,(void*)".twk",(char*)0,1,1,(char*)0);
	//
	currentRule = getRule("Number2");
	currentRule->immediate = ::Number2TawkNow;
	addTest(5,(void*)getRule("Number2Block8"),"instance",1,1,(char*)0);
	currentSet = getSet("lL");
	addTest(6,(void*)currentSet,"isLong",0,1,(char*)0);
	//
	currentRule = getRule("DebugText");
	currentRule->immediate = ::DebugTextTawkNow;
	addTest(7,(void*)"=",(char*)0,1,1,"defaultSKIP");
	addTest(5,(void*)getRule("Quote"),"upcoming",1,1,"defaultSKIP");
	//
	currentRule = getRule("DebugRule");
	currentRule->immediate = ::DebugRuleTawkNow;
	addTest(5,(void*)getRule("NameSet"),"name",1,268435455,"defaultSKIP");
	addTest(5,(void*)getRule("DebugText"),"upcoming",0,1,"defaultSKIP");
	//
	currentRule = getRule("Stop");
	currentRule->immediate = ::StopTawkNow;
	//
	currentRule = getRule("Constant3");
	addTest(5,(void*)getRule("Quote"),"instance",1,1,"defaultSKIP");
	currentRule->next = getRule("Constant4");
	//
	currentRule = getRule("SyntaxExtensions2");
	currentRule->immediate = ::SyntaxExtensions2TawkNow;
	addTest(7,(void*)"alias",(char*)0,1,1,"defaultSKIP");
	currentSet = textFollow;
	addTest(6,(void*)currentSet,(char*)0,-1,1,(char*)0);
	addTest(5,(void*)getRule("AliasItem"),(char*)0,1,268435455,"defaultSKIP");
	addTest(7,(void*)";",(char*)0,1,1,"defaultSKIP");
	currentRule->next = getRule("SyntaxExtensions3");
	//
	currentRule = getRule("Alpha");
	currentSet = getSet("abcdefghijklmnopqrstuvwxyza-zABCDEFGHIJKLMNOPQRSTUVWXYZA-Z01234567890-9_.");
	addTest(6,(void*)currentSet,(char*)0,1,268435455,(char*)0);
	//
	currentRule = getRule("BumpBlock3");
	addTest(7,(void*)"++","bump",1,1,(char*)0);
	saveTest = currentTest;
	addTest(7,(void*)"--","bump",1,1,(char*)0);
	saveTest->setAlternate(currentTest);
	//
	currentRule = getRule("CharacterBlock4");
	currentSet = getSet("'");
	addTest(6,(void*)currentSet,(char*)0,1,1,(char*)0);
	addTest(5,(void*)getRule("CharacterBlock4Block5"),(char*)0,1,1,(char*)0);
	currentSet = getSet("'");
	addTest(6,(void*)currentSet,(char*)0,1,1,(char*)0);
	//
	currentRule = getRule("CaseLabel4");
	addTest(5,(void*)getRule("Expression"),"instance",1,1,"defaultSKIP");
	currentRule->next = getRule("CaseLabel5");
	//
	currentRule = getRule("Commands2");
	addTest(5,(void*)getRule("SyntaxExtensions"),(char*)0,1,1,"defaultSKIP");
	//
	currentRule = getRule("CommentBody3BoDY");
	currentRule->immediate = ::balancEbody;
	addTest(7,(void*)"/*","begin",0,1,(char*)0);
	addTest(7,(void*)"*/","end",0,1,(char*)0);
	currentRule->next = getRule("CommentBody3Any");
	//
	currentRule = getRule("CommentBody3Any");
	currentRule->immediate = ::balancEbail;
	addTest(1,(void*)0,(char*)0,1,1,(char*)0);
	//
	currentRule = getRule("PathBlock11");
	addTest(5,(void*)getRule("Alpha"),(char*)0,1,1,(char*)0);
	addTest(7,(void*)"/",(char*)0,1,1,(char*)0);
	//
	currentRule = getRule("CommentBody4BoDY");
	currentRule->immediate = ::balancEbody;
	addTest(7,(void*)"#ifdef","begin",0,1,(char*)0);
	addTest(7,(void*)"#endif","end",0,1,(char*)0);
	currentRule->next = getRule("CommentBody4Any");
	//
	currentRule = getRule("CodeMatch");
	addTest(5,(void*)getRule("Quote"),"body",1,1,"defaultSKIP");
	currentRule->next = getRule("CodeMatch2");
	//
	currentRule = getRule("Constant2");
	addTest(5,(void*)getRule("Character"),"instance",1,1,"defaultSKIP");
	currentRule->next = getRule("Constant3");
	//
	currentRule = getRule("SyntaxExtensions3");
	addTest(7,(void*)"extender",(char*)0,1,1,"defaultSKIP");
	currentSet = textFollow;
	addTest(6,(void*)currentSet,(char*)0,-1,1,(char*)0);
	addTest(5,(void*)getRule("Extender"),(char*)0,1,268435455,"defaultSKIP");
	addTest(7,(void*)";",(char*)0,1,1,"defaultSKIP");
	currentRule->next = getRule("SyntaxExtensions4");
	//
	currentRule = getRule("SyntaxExtensions4");
	addTest(7,(void*)"initializer",(char*)0,1,1,"defaultSKIP");
	currentSet = textFollow;
	addTest(6,(void*)currentSet,(char*)0,-1,1,(char*)0);
	addTest(5,(void*)getRule("InitializerItem"),(char*)0,1,268435455,"defaultSKIP");
	addTest(7,(void*)";",(char*)0,1,1,"defaultSKIP");
	//
	currentRule = getRule("Number2Block8");
	addTest(5,(void*)getRule("Number2Block8Block9"),"instance",1,1,(char*)0);
	saveTest = currentTest;
	addTest(5,(void*)getRule("Number2Block8Block10"),"instance",1,1,(char*)0);
	saveTest->setAlternate(currentTest);
	//
	currentRule = getRule("Number2Block8Block10");
	currentSet = getSet("01234567890-9");
	addTest(6,(void*)currentSet,(char*)0,1,268435455,(char*)0);
	//
	currentRule = getRule("DeclareType");
	currentRule->immediate = ::DeclareTypeTawkNow;
	currentRule->doNotGuard = 1;
	addTest(5,(void*)getRule("Type"),"type",1,1,"defaultSKIP");
	//
	currentRule = getRule("StructureType");
	addTest(5,(void*)getRule("Type"),"type",1,1,"defaultSKIP");
	currentRule->next = getRule("StructureType2");
	//
	currentRule = getRule("Declaration2");
	currentRule->defer = ::Declaration2TawkAct;
	addTest(5,(void*)getRule("Structure"),"declare",1,1,"defaultSKIP");
	currentRule->next = getRule("Declaration3");
	//
	currentRule = getRule("SetObject");
	currentRule->immediate = ::SetObjectTawkNow;
	//
	currentRule = getRule("Structure");
	currentRule->defer = ::StructureTawkAct;
	addTest(4,(void*)Structures,"kind",1,1,"defaultSKIP");
	currentSet = textFollow;
	addTest(6,(void*)currentSet,(char*)0,-1,1,(char*)0);
	addTest(5,(void*)getRule("StructureBody"),"body",1,1,"defaultSKIP");
	//
	currentRule = getRule("StructureBody");
	currentRule->defer = ::StructureBodyTawkAct;
	addTest(5,(void*)getRule("StructureType"),"label",1,1,"defaultSKIP");
	addTest(7,(void*)"{",(char*)0,1,1,"defaultSKIP");
	addTest(5,(void*)getRule("StructureItem"),"entry",1,268435455,"defaultSKIP");
	addTest(7,(void*)"}",(char*)0,1,1,"defaultSKIP");
	addTest(5,(void*)getRule("DeclareItem"),"field",0,268435455,"defaultSKIP");
	currentRule->next = getRule("StructureBody2");
	//
	currentRule = getRule("StructureType2");
	currentRule->defer = ::StructureType2TawkAct;
	addTest(5,(void*)getRule("Name"),"type",1,1,"defaultSKIP");
	//
	currentRule = getRule("StatementBody13");
	addTest(5,(void*)getRule("Commands"),(char*)0,1,1,"defaultSKIP");
	currentRule->next = getRule("StatementBody14");
	//
	currentRule = getRule("QualifyTail");
	addTest(7,(void*)".",(char*)0,1,1,"defaultSKIP");
	addTest(5,(void*)getRule("FieldBody"),"field",1,1,"defaultSKIP");
	//
	currentRule = getRule("TypeName");
	currentRule->immediate = ::TypeNameTawkNow;
	addTest(5,(void*)getRule("NameSet"),"type",1,1,"defaultSKIP");
	//
	currentRule = getRule("Bits");
	addTest(7,(void*)":",(char*)0,1,1,"defaultSKIP");
	addTest(5,(void*)getRule("Number"),"length",1,1,"defaultSKIP");
	//
	currentRule = getRule("Template2BoDY");
	currentRule->immediate = ::balancEbody;
	addTest(7,(void*)"<","begin",0,1,(char*)0);
	addTest(7,(void*)">","end",0,1,(char*)0);
	currentRule->next = getRule("Template2Any");
	//
	currentRule = getRule("Include2BoDY");
	currentRule->immediate = ::balancEbody;
	addTest(7,(void*)"#import","begin",0,1,(char*)0);
	addTest(7,(void*)"\n","end",0,1,(char*)0);
	currentRule->next = getRule("Include2Any");
	//
	currentRule = getRule("ConditionWord");
	currentRule->defer = ::ConditionWordTawkAct;
	currentRule->doNotGuard = 1;
	addTest(4,(void*)Conditions,"list",1,1,"defaultSKIP");
	currentSet = textFollow;
	addTest(6,(void*)currentSet,(char*)0,-1,1,(char*)0);
	//
	currentRule = getRule("Operator2");
	currentRule->immediate = ::Operator2TawkNow;
	addTest(4,(void*)Comparisons,"comparator",1,1,"defaultSKIP");
	currentSet = logicSet;
	currentRule->guardSet = currentSet;
	currentTest->guardSet = currentSet;
	currentSet = alphaSet;
	addTest(6,(void*)currentSet,(char*)0,-1,1,(char*)0);
	//
	currentRule = getRule("Divert");
	addTest(5,(void*)getRule("Inheritance"),(char*)0,1,268435455,"defaultSKIP");
	//
	currentRule = getRule("UnaryOperator2");
	currentRule->immediate = ::UnaryOperator2TawkNow;
	currentSet = getSet("-+!~");
	addTest(6,(void*)currentSet,"operate",1,268435455,"defaultSKIP");
	//
	currentRule = getRule("CastTypeBlock13");
	addTest(7,(void*)"[]",(char*)0,1,268435455,"defaultSKIP");
	//
	currentRule = getRule("ExpressTail");
	addTest(5,(void*)getRule("ExpressTailBlock14"),"operate",1,1,"defaultSKIP");
	addTest(5,(void*)getRule("ExpressPart"),"instance",1,1,"defaultSKIP");
	currentRule->fail = ::expressPartFailed;
	//
	currentRule = getRule("ExpressTailBlock14");
	addTest(7,(void*)"&&","operate",1,1,"defaultSKIP");
	saveTest = currentTest;
	addTest(7,(void*)"||","operate",1,1,"defaultSKIP");
	saveTest->setAlternate(currentTest);
	//
	currentRule = getRule("OperationTail3");
	addTest(7,(void*)"in","in",1,1,"defaultSKIP");
	currentSet = textFollow;
	addTest(6,(void*)currentSet,(char*)0,-1,1,(char*)0);
	addTest(5,(void*)getRule("RangeField"),"range",1,1,"defaultSKIP");
	//
	currentRule = getRule("Body5");
	addTest(5,(void*)getRule("Include3"),(char*)0,1,1,"defaultSKIP");
	//
	currentRule = getRule("PrimaryExpression4");
	addTest(5,(void*)getRule("AssumedString"),"instance",1,1,"defaultSKIP");
	//
	currentRule = getRule("Catch");
	addTest(7,(void*)"catch",(char*)0,1,1,"defaultSKIP");
	addTest(5,(void*)getRule("Parameter"),"except",0,1,"defaultSKIP");
	addTest(5,(void*)getRule("Statement"),"statement",1,1,"defaultSKIP");
	addTest(5,(void*)getRule("ResetType"),(char*)0,0,1,"defaultSKIP");
	//
	currentRule = getRule("Else");
	addTest(7,(void*)"else",(char*)0,1,1,"defaultSKIP");
	currentSet = textFollow;
	addTest(6,(void*)currentSet,(char*)0,-1,1,(char*)0);
	addTest(5,(void*)getRule("Statement"),"statement",1,1,"defaultSKIP");
	addTest(5,(void*)getRule("ResetType"),(char*)0,0,1,"defaultSKIP");
	currentRule->next = getRule("Else2");
	//
	currentRule = getRule("Extends3");
	addTest(5,(void*)getRule("DeclareConditions"),(char*)0,1,1,"defaultSKIP");
	addTest(7,(void*)";",(char*)0,1,1,"defaultSKIP");
	//
	currentRule = getRule("Include2Any");
	currentRule->immediate = ::balancEbail;
	addTest(1,(void*)0,(char*)0,1,1,(char*)0);
	//
	currentRule = getRule("Imports2");
	addTest(5,(void*)getRule("Commands"),(char*)0,1,1,"defaultSKIP");
	//
	currentRule = getRule("Inheritance3");
	addTest(5,(void*)getRule("Body"),"method",1,268435455,"defaultSKIP");
	currentRule->next = getRule("Inheritance4");
	//
	currentRule = getRule("ParameterList2");
	addTest(5,(void*)getRule("TypeList"),"expression",1,268435455,"defaultSKIP");
	//
	currentRule = getRule("PrintTo");
	addTest(7,(void*)"to",(char*)0,1,1,"defaultSKIP");
	addTest(5,(void*)getRule("FieldExpression"),"instance",1,1,"defaultSKIP");
}

/*****************************************************************************
	Summary debugging data
*****************************************************************************/
void Tawk::summaryDebug()
{
SymbolType 	*type = 0;
int 		t = 0;
int 		c = 0;
int 		m = 0;
	::printf("Summary of Types\n");
	::printf("\t\t\t  Name\t\t\tTypes\tComponents\t\tMethods\t\tFields\n");
	while ( type = (SymbolType*)SymbolType::types->hashList->next() )
		if ( !type->isAtomic && (type->components || type->methods) )
			{
			if ( type->componentTypes )
				t += type->componentTypes->length;
			if ( type->components )
				c += type->components->hashList->length;
			if ( type->methods )
				m += type->methods->hashList->length;
			::printf("%20s\t",type->name);
			if ( type->componentTypes )
				::printf("%10d\t",type->componentTypes->length);
			else	::printf("         0\t");
			if ( type->components )
				::printf("%10d\t",type->components->hashList->length);
			else	::printf("         0\t");
			if ( type->methods )
				::printf("%10d\t",type->methods->hashList->length);
			else	::printf("         0\n");
			}
	::printf("\t\t\t Total\t%10d\t%10d\t%10d\n",t,c,m);
	summary(10);
}
// Ignoring declaration of unused variable type in method: AliasItem4TawkNow(PLGitem*)
// Ignoring declaration of unused variable p in method: AliasItemTawkNow(PLGitem*)
// Ignoring declaration of unused variable p in method: ArrayInitializerTawkNow(PLGitem*)
// Ignoring declaration of unused variable brace in method: BlockStartTawkNow(PLGitem*)
// Ignoring declaration of unused variable p in method: CaseLabelTawkNow(PLGitem*)
// Ignoring declaration of unused variable p in method: CastTailTawkNow(PLGitem*)
// Ignoring declaration of unused variable p in method: CharacterTawkNow(PLGitem*)
// Ignoring declaration of unused variable p in method: DebugTextTawkNow(PLGitem*)
// Ignoring declaration of unused variable staticCopy in method: DeclarationTawkAct(PLGitem*)
// Ignoring declaration of unused variable initialize in method: DeclareItem3TawkAct(PLGitem*)
// Ignoring declaration of unused variable p in method: FieldBody2TawkAct(PLGitem*)
// Ignoring declaration of unused variable p in method: FieldBody3TawkNow(PLGitem*)
// Ignoring declaration of unused variable p in method: FieldExpressionTawkNow(PLGitem*)
// Ignoring declaration of unused variable path in method: FileNameTawkNow(PLGitem*)
// Ignoring declaration of unused variable p in method: InitializerTawkNow(PLGitem*)
// Ignoring declaration of unused variable direct in method: ItemHeadTawkNow(PLGitem*)
// Ignoring declaration of unused variable name in method: ItemHeadTawkNow(PLGitem*)
// Ignoring declaration of unused variable bits in method: ItemHeadTawkNow(PLGitem*)
// Ignoring declaration of unused variable p in method: ItemHeadTawkNow(PLGitem*)
// Ignoring declaration of unused variable p in method: LambdaTawkNow(PLGitem*)
// Ignoring declaration of unused variable modify in method: MethodTypeTawkAct(PLGitem*)
// Ignoring declaration of unused variable methodHead in method: MethodTypeTawkAct(PLGitem*)
// Ignoring declaration of unused variable p in method: OperationTail2TawkNow(PLGitem*)
// Ignoring declaration of unused variable selectedField in method: PoundCommandTawkNow(PLGitem*)
// Ignoring declaration of unused variable p in method: QuestionTawkNow(PLGitem*)
// Ignoring declaration of unused variable p in method: RangeExpressionTawkNow(PLGitem*)
// Ignoring declaration of unused variable p in method: StatementBody11TawkNow(PLGitem*)
// Ignoring declaration of unused variable p in method: StatementBody14TawkNow(PLGitem*)
// Ignoring declaration of unused variable p in method: StatementBody15TawkNow(PLGitem*)
// Ignoring declaration of unused variable p in method: StatementBody20TawkNow(PLGitem*)
// Ignoring declaration of unused variable p in method: StatementBody21TawkNow(PLGitem*)
// Ignoring declaration of unused variable p in method: StatementBody2TawkNow(PLGitem*)
// Ignoring declaration of unused variable p in method: StatementBody3TawkNow(PLGitem*)
// Ignoring declaration of unused variable p in method: StatementBody4TawkNow(PLGitem*)
// Ignoring declaration of unused variable p in method: StatementBody6TawkNow(PLGitem*)
// Ignoring declaration of unused variable p in method: StatementBody8TawkNow(PLGitem*)
// Ignoring declaration of unused variable p in method: StopTawkNow(PLGitem*)
// Ignoring declaration of unused variable p in method: StringsTawkNow(PLGitem*)
// Ignoring declaration of unused variable p in method: StructureType2TawkAct(PLGitem*)
// Ignoring declaration of unused variable p in method: TargetTawkNow(PLGitem*)
// Ignoring declaration of unused variable p in method: TypeNameTawkNow(PLGitem*)
// Ignoring declaration of unused variable p in method: UnaryOperator2TawkNow(PLGitem*)
