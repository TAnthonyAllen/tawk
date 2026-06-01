#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "StringRoutines.h"
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
#include "DoubleLinkList.h"
#include "PLGrule.h"
#include "Alternative.h"
#include "DoubleLink.h"
#include "InstanceTable.h"
#include "Element.h"
#include "Buffer.h"
#include "KeyTableItem.h"
#include "PLGitem.h"
#include "PLGset.h"
#include "BlockTok.h"
#include "Expression.h"
#include "Instance.h"
#include "Statement.h"
#include "Tawk.h"

int Tawk::AliasItem2TawkNow(PLGitem *iTEM)
{
PLGitem 	*alias = (PLGitem*)iTEM->children->get("alias");
	currentClass->constructor = alias->toString();
	return 1;
}

int Tawk::AliasItem3TawkNow(PLGitem *iTEM)
{
PLGitem 	*alias = (PLGitem*)iTEM->children->get("alias");
PLGitem 	*value = (PLGitem*)iTEM->children->get("value");
char 		*name = 0;
char 		*targetName = 0;
PLGitem 	*body = value->getLabel("body");
PLGitem 	*indirect = value->getLabel("indirect");
PLGitem 	*target = value->getLabel("target");
PLGitem 	*valueType = value->getLabel("type");
	if ( !currentClass )
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
			else	typeAlias = (SymbolType*)valueType->itemValue;
			SymbolType::types->put(name,typeAlias);
			}
		else {
			if ( body )
				{
				PLGitem 	*arguments = divertInput(body->string(),"AliasParameters");
				body->unString();
				body = arguments->getLabel("body");
				}
			targetName = target->string();
			currentClass->makeAlias(name,targetName,body,"p");
			target->unString();
			}
		}
	return 1;
}

int Tawk::AliasItemTawkNow(PLGitem *iTEM)
{
PLGitem 	*name = (PLGitem*)iTEM->children->get("name");
PLGitem 	*target = (PLGitem*)iTEM->children->get("target");
Symbol 		*symbol = (Symbol*)name->itemValue;
Symbol 		*method = (Symbol*)target->itemValue;
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

int Tawk::ArrayInitializerTawkNow(PLGitem *iTEM)
{
PLGitem 	*instance = (PLGitem*)iTEM->children->get("instance");
PLGitem 	*item = 0;
Instance 	*entry = 0;
BlockTok 	*block = new BlockTok(2);
	for ( item = instance; item; item = item->itemNext )
		{
		entry = (Instance*)item->itemValue;
		block->add(entry);
		}
	entry = new Instance(block);
	instance->itemValue = (void*)entry;
	return 1;
}

int Tawk::AssumedStringTawkNow(PLGitem *iTEM)
{
PLGitem 	*instance = (PLGitem*)iTEM->children->get("instance");
PLGitem 	*lastSwitch = 0;
SymbolType 	*switchType = 0;
Instance 	*switcher = 0;
char 		*word = instance->string();
Instance 	*current = 0;
int 		switchDirect = 0;
	if ( ReservedWord->find(word) )
		{
		instance->unString();
		return 0;
		}
	instance->unString();
	if ( switchStack )
		if ( lastSwitch = (PLGitem*)switchStack->top() )
			{
			switcher = (Instance*)lastSwitch->itemValue;
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
	current = getInstance(instance->toString());
	current->indirection = 1;
	// constant strings are really pointers
	current->type = SymbolType::stringType;
bailAssumed:
	current->isConstant = 1;
	instance->itemValue = (void*)current;
	return 1;
}

int Tawk::BlockStartTawkNow(PLGitem *iTEM)
{
	if ( declaredSomething )
		declaredSomething = 0;
	currentSymbols->push("Block start");
	setCurrentType((SymbolType*)0);
	if ( declaringMethod && currentMethod )
		{
		if ( !currentBlock )
			currentMethod->pushParameters(currentSymbols);
		else
		if ( lambdaMethod )
			lambdaMethod->pushParameters(currentSymbols);
		}
	lambdaMethod = 0;
	if ( currentBlock )
		blockStack->push(currentBlock);
	currentBlock = new BlockTok(1);
	iTEM->itemValue = (void*)currentBlock;
	currentBlock->blockMethod = currentMethod;
	// virtualStack can get out of whack when an expression fails
	// so it gets cleaned up here if needed
	resetVirtuals();
	return 1;
}

int Tawk::BlockTawkNow(PLGitem *iTEM)
{
PLGitem 	*start = (PLGitem*)iTEM->children->get("start");
PLGitem 	*line = (PLGitem*)iTEM->children->get("line");
Statement 	*statement = 0;
PLGitem 	*item = 0;
BlockTok 	*block = 0;
Directive 	*directive = 0;
	block = (BlockTok*)start->itemValue;
	for ( ; line; line = line->itemNext )
		{
		item = line->getLabel("statement");
		if ( !item )
			continue;
		statement = (Statement*)item->itemValue;
		if ( statement )
			block->add(statement);
		}
	if ( declaringMethod && currentMethod )
		{
		declaringMethod = 0;
		if ( currentMethod->directives )
			{
			noLoop = 1;
			currentMethod->directives->resetIterator();
			while ( directive = (Directive*)currentMethod->directives->next() )
				if ( !directive->codeMatch )
					directive->parseDirective();
			noLoop = 0;
			}
		}
	currentBlock = (BlockTok*)blockStack->pop();
	currentSymbols->pop("Block end");
	return 1;
}

int Tawk::Body2TawkNow(PLGitem *iTEM)
{
PLGitem 	*body = (PLGitem*)iTEM->children->get("body");
Instance 	*method = 0;
	body = body->getLabel("method");
	method = (Instance*)body->itemValue;
	if ( extending && method && method->symbol )
		method->symbol->extendType();
	return 1;
}

void Tawk::BodyTawkAct(PLGitem *iTEM)
{
}

void Tawk::ButtonArrayTawkAct(PLGitem *iTEM)
{
PLGitem 	*button = (PLGitem*)iTEM->children->get("button");
Symbol 		*symbol = 0;
int 		i = 1;
char 		*text = 0;
	for ( ; button; button = button->itemNext )
		{
		text = button->toString();
		symbol = currentClass->getLocal(text);
		if ( !symbol )
			{
			symbol = new Symbol(text,SymbolType::buttonType);
			symbol->isHidden = 1;
			symbol->isItem = 1;
			symbol->array = ::toStringFromInt(i++);
			currentClass->add(symbol);
			}
		else
		if ( symbol->type != SymbolType::buttonType )
			::fprintf(stderr,"Expected a button %s\n",text);
		button->itemValue = (void*)symbol;
		}
}

int Tawk::Case2TawkNow(PLGitem *iTEM)
{
PLGitem 	*instance = (PLGitem*)iTEM->children->get("instance");
PLGitem 	*lastSwitch = 0;
SymbolType 	*labelType = 0;
SymbolType 	*switchType = 0;
Instance 	*switcher = 0;
Instance 	*label = (Instance*)instance->itemValue;
int 		labelDirect = 0;
int 		switchDirect = 0;
	if ( switchStack )
		if ( lastSwitch = (PLGitem*)switchStack->top() )
			if ( switcher = (Instance*)lastSwitch->itemValue )
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

int Tawk::Case3TawkNow(PLGitem *iTEM)
{
PLGitem 	*instance = (PLGitem*)iTEM->children->get("instance");
PLGitem 	*name = instance->getLabel("name");
char 		*text = name->toString();
Instance 	*label = (Instance*)currentSymbols->instances->get(text);
Symbol 		*symbol = 0;
	if ( !label )
		{
		symbol = new Symbol(text,SymbolType::voidType);
		label = new Instance(symbol);
		label->isLabel = 1;
		currentSymbols->add(label);
		}
	if ( !label->isLabel )
		::fprintf(stderr,"ERROR: %s is not a label\n",name->toString());
	instance->itemValue = (void*)label;
	return 1;
}

int Tawk::CaseLabel2TawkNow(PLGitem *iTEM)
{
PLGitem 	*instance = (PLGitem*)iTEM->children->get("instance");
PLGitem 	*item = 0;
Instance 	*field = 0;
Instance 	*rangeField = 0;
Instance 	*label = 0;
	rangeField = (Instance*)instance->itemValue;
	if ( switchStack )
		if ( item = (PLGitem*)switchStack->top() )
			field = (Instance*)item->itemValue;
	if ( field && rangeField )
		label = new Instance(convertRangeX(field,rangeField));
	label->isRange = 1;
	field->isRange = 1;
	// set to flag switch as needing conversion
	instance->itemValue = (void*)label;
	return 1;
}

int Tawk::CaseLabel3TawkNow(PLGitem *iTEM)
{
PLGitem 	*instance = (PLGitem*)iTEM->children->get("instance");
PLGitem 	*field = instance->getLabel("field");
Instance 	*label = (Instance*)field->itemValue;
Symbol 		*symbol = label->getSymbol();
	if ( symbol->type == SymbolType::buttonType || (symbol->structType && isEnumerator(symbol->structType->structure)) )
		{
		label = getInstance("case ");
		if ( symbol->type == SymbolType::buttonType )
			label->postfix = symbol->array;
		else	label->postfix = symbol->name;
		label->type = SymbolType::intType;
		label->isConstant = 1;
		}
	else	label->prefix = "case ";
	label->isLabel = 1;
	label->isCase = 1;
	instance->itemValue = (void*)label;
	assuming = 0;
	return 1;
}

int Tawk::CaseLabel4TawkNow(PLGitem *iTEM)
{
PLGitem 	*instance = (PLGitem*)iTEM->children->get("instance");
PLGitem 	*name = instance->getLabel("name");
Instance 	*field = getInstance(name->toString());
	instance->itemValue = (void*)field;
	return 1;
}

int Tawk::CaseLabelTawkNow(PLGitem *iTEM)
{
PLGitem 	*instance = (PLGitem*)iTEM->children->get("instance");
Instance 	*label = (Instance*)instance->itemValue;
	label->postfix = label->prefix;
	return 1;
}

int Tawk::CaseTawkNow(PLGitem *iTEM)
{
PLGitem 	*instance = (PLGitem*)iTEM->children->get("instance");
Instance 	*label = getInstance("default");
	label->isLabel = 1;
	label->isCase = 1;
	instance->itemValue = (void*)label;
	return 1;
}

int Tawk::CastExpressionTawkNow(PLGitem *iTEM)
{
PLGitem 	*direct = (PLGitem*)iTEM->children->get("direct");
PLGitem 	*type = (PLGitem*)iTEM->children->get("type");
PLGitem 	*rest = (PLGitem*)iTEM->children->get("rest");
Instance 	*instance = (Instance*)type->itemValue;
	if ( rest )
		{
		if ( !instance->isMethod )
			instance->error("Cast Expression method reference expected");
		instance->parameters = (DoubleLinkList*)rest->itemValue;
		}
	instance->isCast = 1;
	if ( direct )
		instance->prefix = direct->string();
	setCurrentType((SymbolType*)0);
	if ( direct )
		type->itemValue = (void*)instance->setIndirectItem(direct);
	return 1;
}

int Tawk::CastTailTawkNow(PLGitem *iTEM)
{
PLGitem 		*rest = (PLGitem*)iTEM->children->get("rest");
DoubleLinkList 	*list = new DoubleLinkList();
Instance 		*instance = 0;
PLGitem 		*cast = 0;
PLGitem 		*item = 0;
	for ( item = rest; item; item = item->itemNext )
		{
		cast = item->getLabel("type");
		instance = (Instance*)cast->itemValue;
		list->add((void*)instance);
		}
	rest->itemValue = (void*)list;
	return 1;
}

int Tawk::CastTypeTawkNow(PLGitem *iTEM)
{
PLGitem 	*type = (PLGitem*)iTEM->children->get("type");
PLGitem 	*direct = (PLGitem*)iTEM->children->get("direct");
PLGitem 	*array = (PLGitem*)iTEM->children->get("array");
SymbolType 	*symbolType = (SymbolType*)type->itemValue;
Instance 	*instance = new Instance(symbolType);
char 		*atDirect = 0;
	if ( direct )
		for ( atDirect = direct->itemStart + direct->itemLength - 1; atDirect >= direct->itemStart; atDirect-- )
			if ( *atDirect == '*' )
				instance->indirection++;
			else
			if ( *atDirect == '^' )
				currentClass->hasLambda = instance->isLambda = 1;
			else
			if ( *atDirect == '&' )
				instance->setReference((unsigned int)1);
	if ( !symbolType->isDirect && !instance->indirection )
		instance->indirection++;
	if ( instance->reference || instance->isLambda )
		instance->isMethod = 1;
	if ( array )
		instance->postfix = array->toString();
	type->itemValue = (void*)instance;
	return 1;
}

int Tawk::CharacterTawkNow(PLGitem *iTEM)
{
PLGitem 	*instance = (PLGitem*)iTEM->children->get("instance");
Instance 	*character = 0;
SymbolType 	*type = SymbolType::getType("char");
	character = new Instance(type);
	character->isConstant = 1;
	character->prefix = instance->toString();
	instance->itemValue = (void*)character;
	return 1;
}

int Tawk::CheckMacroParametersTawkNow(PLGitem *iTEM)
{
PLGitem 	*braced = (PLGitem*)iTEM->children->get("braced");
PLGitem 	*body = braced->getLabel("body");
	if ( body )
		{
		PLGitem 	*list = divertInput(body->string(),"MacroArgumentList");
		body->unString();
		braced->itemValue = (void*)list->getLabel("argument");
		}
	return 1;
}

int Tawk::CheckMacroTawkNow(PLGitem *iTEM)
{
PLGitem 	*statement = (PLGitem*)iTEM->children->get("statement");
PLGitem 	*braced = (PLGitem*)iTEM->children->get("braced");
PLGrule 	*rule = 0;
PLGitem 	*item = 0;
PLGitem 	*macro = 0;
Symbol 		*argument = 0;
Symbol 		*symbol = 0;
int 		count = 0;
Buffer 		*buffer = tokJunkBuffer;
	if ( !macroList )
		return 0;
	symbol = (Symbol*)macroList->get(statement->toString());
	if ( !symbol )
		return 0;
	if ( braced )
		{
		braced = item = (PLGitem*)braced->itemValue;
		for ( ; item; item = item->itemNext )
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
		for ( item = braced; item; item = item->itemNext )
			{
			argument = (Symbol*)symbol->parameters->next();
			argument->commentItem = item->getLabel("part");
			}
		}
	buffer->reset();
	macro = symbol->commentItem;
	for ( ; macro; macro = macro->itemNext )
		if ( argument = (Symbol*)macro->itemValue )
			{
			if ( item = argument->commentItem )
				buffer->appendString(item->toString(),0,0);
			else {
				buffer->appendString("No value supplied for ",0,0);
				buffer->appendString(argument->name,0,0);
				buffer->appendString("\n",0,0);
				}
			}
		else {
			buffer->appendString(macro->string(),0,0);
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
		BlockTok 	*saveBlock = currentBlock;
		currentBlock = new BlockTok();
		rule = (PLGrule*)rules->get("MacroBlock");
		if ( item = divertInput(buffer->toString(),rule) )
			{
			Instance 	*instance = (Instance*)currentBlock->statements->first->value;
			line->add(currentBlock);
			statement->itemValue = (void*)line;
			if ( instance && instance->statement )
				instance->statement->indented = 1;
			}
		currentBlock = saveBlock;
		}
	return 1;
}

void Tawk::ClassAttributes2TawkAct(PLGitem *iTEM)
{
PLGitem 	*type = (PLGitem*)iTEM->children->get("type");
SymbolType 	*extended = (SymbolType*)type->itemValue;
	currentClass->setParent(extended);
}

void Tawk::ClassAttributes3TawkAct(PLGitem *iTEM)
{
PLGitem 	*proto = (PLGitem*)iTEM->children->get("proto");
PLGitem 	*type = 0;
SymbolType 	*pType = 0;
	for ( ; proto; proto = proto->itemNext )
		if ( type = proto->getLabel("type") )
			{
			pType = (SymbolType*)type->itemValue;
			currentClass->addProtocol(pType);
			}
}

void Tawk::ClassAttributes4TawkAct(PLGitem *iTEM)
{
PLGitem 	*nSpace = (PLGitem*)iTEM->children->get("nSpace");
	currentClass->nameSpace = nSpace->toString();
}

void Tawk::ClassAttributesTawkAct(PLGitem *iTEM)
{
PLGitem 		*trait = (PLGitem*)iTEM->children->get("trait");
KeyTableItem 	*item = (KeyTableItem*)trait->itemValue;
Symbol 			*symbol = 0;
	switch (item->position)
		{
		case 1:
			// C
			if ( !currentClass->isC )
				{
				currentClass->isC = 1;
				if ( !currentClass->parent )
					{
					if ( currentClass->lastOffset > 0 )
						::fprintf(stderr,"ERROR: do not specify class as C after variables are declared\n");
					symbol = new Symbol("tHIS",currentClass);
					symbol->isVirtual = 1;
					symbol->isThis = 1;
					currentClass->add(symbol);
					symbol = symbol->makeAlias("this");
					symbol->isThis = 1;
					currentClass->add(symbol);
					currentClass->lastOffset = 0;
					}
				}
			break;
		case 2:
			// isChar
			currentClass->isChar = 1;
			break;
		case 3:
			// isNumber
			currentClass->isNumber = 1;
			break;
		case 4:
			// local
			currentClass->isLocal = 1;
			break;
		case 5:
			// no.h
			currentClass->noDotH = 1;
			break;
		case 6:
			// noClassForward
			currentClass->noClassForward = 1;
			break;
		case 7:
			// OC
			if ( !currentClass->isOC )
				{
				currentClass->isOC = 1;
				currentClass->isVirtuous = 1;
				}
			break;
		case 8:
			// proper
			currentClass->proper = 1;
			break;
		case 9:
			//protocol
			currentClass->structure = 3;
			currentClass->isOC = 1;
			break;
		case 10:
			// type
			currentClass->structure = 4;
			currentClass->isDirect = 1;
			break;
		case 11:
			// addClassNameToMethods
			currentClass->addClassNameToMethods = 1;
			break;
		}
}

int Tawk::ClassHeading2TawkNow(PLGitem *iTEM)
{
PLGitem 	*externalRef = (PLGitem*)iTEM->children->get("externalRef");
PLGitem 	*structure = (PLGitem*)iTEM->children->get("structure");
PLGitem 	*type = structure->getLabel("body");
PLGitem 	*kind = structure->getLabel("kind");
PLGitem 	*field = 0;
Instance 	*instance = 0;
	extending = 0;
	if ( kind->toString() == "typedef" || kind->toString() == "struct" )
		return 0;
	field = type->itemNext;
	structure->runDeferred(this);
	instance = (Instance*)type->itemValue;
	setCurrentClass(instance->getType());
	currentClass->noDotH = 1;
	setCurrentType((SymbolType*)0);
	if ( externalRef )
		currentClass->isExternal = 1;
	else	currentClass->isExternal = 0;
	for ( ; field; field = field->itemNext )
		{
		instance = (Instance*)field->itemValue;
		if ( instance )
			{
			instance->isDeclaration = 1;
			if ( instance->symbol )
				instance->symbol->parentClass = SymbolType::globalType;
			}
		}
	return 1;
}

int Tawk::ClassHeading3TawkNow(PLGitem *iTEM)
{
PLGitem 	*kind = (PLGitem*)iTEM->children->get("kind");
PLGitem 	*nom = (PLGitem*)iTEM->children->get("nom");
PLGitem 	*attributes = (PLGitem*)iTEM->children->get("attributes");
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
	currentBlock = 0;
	extending = 0;
	if ( !nom )
		{
		setCurrentClass(SymbolType::globalType);
		extending = 1;
		}
	else
	for ( entry = nom; entry; entry = entry->itemNext )
		{
		name = entry->getLabel("name");
		temp = entry->getLabel("temp");
		path = (PLGitem*)name->itemValue;
		dotH = entry->getLabel("dotH");
		className = name->toString();
		if ( dotH )
			{
			dotHfile = path->toString();
			if ( !nom->itemNext )
				{
				// rename the class since it is not really a class, just the name of a file
				className = ::concat(2,"notType",className);
				setCurrentClass(SymbolType::getType(className));
				currentClass->isGlobal = 1;
				currentClass->isExternal = 1;
				SymbolType::globalList->add((void*)currentClass);
				extending = 1;
				}
			else	continue;
			}
		else	setCurrentClass(SymbolType::getType(className));
		if ( temp )
			currentClass->isTemplate = 1;
		for ( item = attributes; item; item = item->itemNext )
			item->runDeferred(this);
		if ( type )
			parent = (SymbolType*)type->itemValue;
		currentClass->isExternal = 1;
		if ( parent )
			{
			currentClass->setParent(parent);
			if ( parent->isOC )
				currentClass->isOC = 1;
			}
		if ( !currentClass->noDotH && !currentClass->isAtomic && !currentClass->dotHname )
			currentClass->dotHname = path->toString();
		if ( currentClass->isOC )
			{
			nameSet->set((int)':');
			if ( currentClass->parent )
				{
				symbol = new Symbol("super",currentClass->parent);
				symbol->isVirtual = 1;
				currentClass->add(symbol);
				}
			}
		entry->itemValue = (void*)currentClass;
		}
	if ( nom && nom->itemNext && dotHfile )
		{
		for ( entry = nom; entry; entry = entry->itemNext )
			{
			if ( !entry->itemValue )
				continue;
			setCurrentClass((SymbolType*)entry->itemValue);
			currentClass->dotHname = dotHfile;
			}
		setCurrentClass((SymbolType*)nom->itemValue);
		}
	if ( kind )
		if ( *kind->itemStart == 't' )
			{
			currentClass->isDirect = 1;
			currentClass->structure = 4;
			}
		else {
			currentClass->isDirect = 1;
			currentClass->structure = 5;
			}
	return 1;
}

int Tawk::ClassHeadingTawkNow(PLGitem *iTEM)
{
PLGitem 	*nom = (PLGitem*)iTEM->children->get("nom");
PLGitem 	*attributes = (PLGitem*)iTEM->children->get("attributes");
SymbolType 	*parent = 0;
char 		*className = 0;
PLGitem 	*item = 0;
PLGitem 	*name = 0;
PLGitem 	*path = 0;
PLGitem 	*type = 0;
Symbol 		*symbol = 0;
	extending = 0;
	referring = 1;
	currentBlock = 0;
	name = nom->getLabel("name");
	path = (PLGitem*)name->itemValue;
	className = name->toString();
	setCurrentClass(SymbolType::getType(className));
	for ( item = attributes; item; item = item->itemNext )
		{
		if ( !type )
			type = item->getLabel("type");
		item->runDeferred(this);
		}
	if ( type )
		parent = (SymbolType*)type->itemValue;
	if ( parent )
		{
		currentClass->setParent(parent);
		if ( parent->isOC )
			currentClass->isOC = 1;
		}
	if ( currentComment )
		currentClass->comment = extractComment();
	if ( currentClass->isOC )
		{
		nameSet->set((int)':');
		if ( currentClass->parent )
			{
			symbol = new Symbol("super",currentClass->parent);
			symbol->isVirtual = 1;
			currentClass->add(symbol);
			}
		}
	if ( !currentClass->noDotH && !currentClass->isAtomic && !currentClass->dotHname && !isProtocol(currentClass->structure) )
		currentClass->dotHname = path->toString();
	currentClass->isExternal = 0;
	if ( directivesFile )
		processDirectives();
	return 1;
}

int Tawk::ClassNameTawkNow(PLGitem *iTEM)
{
PLGitem 	*path = (PLGitem*)iTEM->children->get("path");
PLGitem 	*name = (PLGitem*)iTEM->children->get("name");
PLGitem 	*temp = (PLGitem*)iTEM->children->get("temp");
PLGitem 	*dotH = (PLGitem*)iTEM->children->get("dotH");
char 		*text = 0;
	if ( temp )
		name->itemLength += temp->itemLength;
	if ( name->toString() == "extends" || name->toString() == "external" )
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
		path = new PLGitem(name);
		path->itemLength += dotH->itemLength;
		}
	else {
		text = ::concat(2,name->string(),".h");
		path = new PLGitem(text);
		name->unString();
		}
	name->itemValue = (void*)path;
	return 1;
}

int Tawk::CommentTawkNow(PLGitem *iTEM)
{
PLGitem 	*comment = (PLGitem*)iTEM->children->get("comment");
	currentComment = comment;
	return 1;
}

int Tawk::ConditionLabelTawkNow(PLGitem *iTEM)
{
PLGitem 	*label = (PLGitem*)iTEM->children->get("label");
PLGitem 	*text = (PLGitem*)iTEM->children->get("text");
	Conditions->add(label->string(),(void*)text->toString());
	return 1;
}

void Tawk::ConditionWordTawkAct(PLGitem *iTEM)
{
PLGitem 		*list = (PLGitem*)iTEM->children->get("list");
PLGitem 		*item = 0;
PLGitem 		*text = 0;
KeyTableItem 	*conditionItem = 0;
Instance 		*condition = 0;
char 			*conditionText = 0;
	if ( conditionItem = (KeyTableItem*)list->itemValue )
		if ( conditionText = (char*)conditionItem->value )
			if ( item = divertInput(conditionText,"Expression") )
				{
				text = item->getLabel("instance");
				if ( text )
					list->itemValue = text->itemValue;
				else {
					condition = new Instance();
					condition->error("Failed parse of condition list");
					list->itemValue = (void*)condition;
					}
				return;
				}
	::fprintf(stderr,"Could not parse Condition: %s\n",list->toString());
}

int Tawk::Constant2TawkNow(PLGitem *iTEM)
{
PLGitem 	*instance = (PLGitem*)iTEM->children->get("instance");
Instance 	*current = new Instance(trueSymbol);
	instance->itemValue = (void*)current;
	current->isConstant = 1;
	return 1;
}

int Tawk::Constant3TawkNow(PLGitem *iTEM)
{
PLGitem 	*instance = (PLGitem*)iTEM->children->get("instance");
Instance 	*current = new Instance(falseSymbol);
	instance->itemValue = (void*)current;
	current->isConstant = 1;
	return 1;
}

void Tawk::ConstantTawkAct(PLGitem *iTEM)
{
}

void Tawk::DebugDirectiveTawkAct(PLGitem *iTEM)
{
PLGitem 	*method = (PLGitem*)iTEM->children->get("method");
PLGitem 	*body = (PLGitem*)iTEM->children->get("body");
PLGitem 	*locate = (PLGitem*)iTEM->children->get("locate");
PLGitem 	*active = (PLGitem*)iTEM->children->get("active");
PLGitem 	*code = (PLGitem*)iTEM->children->get("code");
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
			Symbol 	*directiveMethod = currentType->getMethod(text);
			if ( !directiveMethod && currentType->isGlobal )
				directiveMethod = currentSymbols->findGlobalMethod(text);
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
			else	::fprintf(stderr,"Could not find directive method: %s in type: %s\n",text,currentType->name);
			}
		method->unString();
		}
}

void Tawk::Declaration2TawkAct(PLGitem *iTEM)
{
PLGitem 	*declare = (PLGitem*)iTEM->children->get("declare");
PLGitem 	*field = declare->getLabel("body");
PLGitem 	*kind = declare->getLabel("kind");
SymbolType 	*structureType = 0;
Instance 	*instance = 0;
	declare->runDeferred(this);
	//dealWith next
	if ( kind )
		structureType = (SymbolType*)kind->itemValue;
	if ( !currentClass->isExternal && structureType && isType(structureType->structure) )
		::fprintf(stderr,"WARNING typedef ignored: must be declared external\n");
	declare->itemNext = field;
	for ( ; field; field = field->itemNext )
		{
		instance = (Instance*)field->itemValue;
		if ( instance )
			instance->isDeclaration = 1;
		}
}

void Tawk::DeclarationTawkAct(PLGitem *iTEM)
{
PLGitem 	*outlet = (PLGitem*)iTEM->children->get("outlet");
PLGitem 	*modify = (PLGitem*)iTEM->children->get("modify");
PLGitem 	*type = (PLGitem*)iTEM->children->get("type");
PLGitem 	*declare = (PLGitem*)iTEM->children->get("declare");
Instance 	*instance = 0;
Symbol 		*symbol = 0;
PLGitem 	*entry = 0;
PLGitem 	*item = 0;
	setCurrentType((SymbolType*)type->itemValue);
	newType = methodType = currentType;
	for ( item = modify; item; item = item->itemNext )
		if ( *item->itemStart == 'c' )
			constDeclare = 1;
		else
		if ( *item->itemStart == 'i' )
			linkage = 2;
		else
		if ( *item->itemStart == 's' )
			linkage = 3;
		else
		if ( *item->itemStart == 'v' )
			linkage = 4;
		else	linkage = 1;
	for ( entry = declare; entry; entry = entry->itemNext )
		{
		entry->runDeferred(this);
		item = entry->getLabel("item");
		instance = (Instance*)item->itemValue;
		instance->isDeclaration = 1;
		entry->itemValue = (void*)instance;
		symbol = instance->getSymbol();
		if ( outlet )
			symbol->isOutlet = 1;
		if ( staticDeclare(linkage) )
			{
			symbol->isStatic = 1;
			if ( instance->express )
				{
				Statement 	*statement = new Statement();
				statement->pointInCode = iTEM;
				statement->add(instance);
				formatter->staticBlock->add(statement);
				symbol->isInitialized = 1;
				}
			if ( symbol->isMethod )
				currentSymbols->addGlobalField(symbol->methodName,symbol);
			currentSymbols->addGlobalField(symbol->name,symbol);
			}
		if ( constDeclare )
			symbol->isConst = 1;
		if ( inlineDeclare(linkage) )
			symbol->isInline = 1;
		if ( staticDeclare(linkage) )
			symbol->isStatic = 1;
		if ( virtualDeclare(linkage) )
			symbol->isVirtual = 1;
		if ( externDeclare(linkage) )
			{
			symbol->isExtern = 1;
			currentClass->hasExtern = 1;
			}
		}
	setCurrentType((SymbolType*)0);
	if ( modify )
		{
		constDeclare = 0;
		linkage = 0;
		}
}

void Tawk::DeclareItem2TawkAct(PLGitem *iTEM)
{
PLGitem 	*item = (PLGitem*)iTEM->children->get("item");
PLGitem 	*argument = (PLGitem*)iTEM->children->get("argument");
char 		*text = 0;
Symbol 		*symbol = 0;
PLGitem 	*name = item->getLabel("name");
PLGitem 	*parameter = argument->getLabel("instance");
Instance 	*instance = getInstance(currentType->name);
Instance 	*part = (Instance*)parameter->itemValue;
	/**********************************************************************
	converting constructor
	**********************************************************************/
	instance->addParameter(part);
	text = instance->mangle();
	if ( symbol = currentType->getMethod(text) )
		{
		symbol = new Symbol(name->toString(),currentType);
		symbol->indirect = 0;
		symbol->isConstructor = 1;
		instance->symbol = symbol;
		instance->prefix = 0;
		instance->isMethod = 1;
		}
	else	instance->error("Could not find converting constructor");
	item->itemValue = (void*)instance;
}

void Tawk::DeclareItem3TawkAct(PLGitem *iTEM)
{
PLGitem 	*item = (PLGitem*)iTEM->children->get("item");
PLGitem 	*assign = (PLGitem*)iTEM->children->get("assign");
PLGitem 	*direct = item->getLabel("direct");
PLGitem 	*name = item->getLabel("name");
PLGitem 	*array = item->getLabel("array");
PLGitem 	*bits = item->getLabel("bits");
Symbol 		*symbol = 0;
Instance 	*instance = 0;
Instance 	*assigned = 0;
Expression 	*express = 0;
char 		*text = name->toString();
	if ( !currentBlock )
		symbol = currentClass->getLocal(text);
	if ( currentBlock || !symbol )
		{
		symbol = new Symbol(text,currentType);
		if ( direct )
			symbol->setIndirection(direct);
		if ( array )
			{
			symbol->indirect += (long)array->itemValue;
			symbol->isArray = (unsigned int)array->getAmount();
			tokJunkBuffer->reset();
			for ( ; array; array = array->itemNext )
				array->copyTo(tokJunkBuffer);
			symbol->array = tokJunkBuffer->toString();
			}
		else
		if ( bits )
			symbol->array = bits->toString();
		}
	instance = new Instance(symbol);
	instance->isDeclaration = 1;
	instance->block = currentBlock;
	if ( !direct && !currentType->isDirect )
		symbol->indirect = 1;
	if ( assign )
		{
		PLGitem 	*assignItem = assign->getLabel("instance");
		assigned = (Instance*)assignItem->itemValue;
		express = new Expression(instance,assigned,"=");
		instance = new Instance(express);
		instance->isRange = assigned->isRange;
		}
	item->itemValue = (void*)instance;
}

void Tawk::DeclareItemTawkAct(PLGitem *iTEM)
{
PLGitem 	*item = (PLGitem*)iTEM->children->get("item");
PLGitem 	*instance = (PLGitem*)iTEM->children->get("instance");
PLGitem 	*parameters = item->getLabel("head");
Instance 	*methodD = 0;
Instance 	*methodI = 0;
Expression 	*xpress = 0;
	item->runDeferred(this);
	methodD = (Instance*)parameters->itemValue;
	methodD->isDeclaration = 1;
	if ( methodD->symbol )
		{
		if ( !currentBlock )
			methodD->symbol->parentClass = currentClass;
		if ( currentMethod && currentMethod == methodD->symbol )
			currentMethod = 0;
		}
	if ( instance )
		{
		methodI = (Instance*)instance->itemValue;
		xpress = new Expression(methodD,methodI,"=");
		methodD = new Instance(xpress);
		}
	item->itemValue = (void*)methodD;
}

int Tawk::DeclareTypeTawkNow(PLGitem *iTEM)
{
PLGitem 	*type = (PLGitem*)iTEM->children->get("type");
	setCurrentType((SymbolType*)type->itemValue);
	newType = methodType = currentType;
	return 1;
}

int Tawk::DirectiveTawkNow(PLGitem *iTEM)
{
PLGitem 	*type = (PLGitem*)iTEM->children->get("type");
PLGitem 	*directives = (PLGitem*)iTEM->children->get("directives");
	setCurrentType((SymbolType*)type->itemValue);
	for ( ; directives; directives = directives->itemNext )
		if ( currentType == currentClass || currentType == SymbolType::globalType )
			directives->runDeferred(this);
	return 1;
}

int Tawk::Else2TawkNow(PLGitem *iTEM)
{
PLGitem 	*statement = (PLGitem*)iTEM->children->get("statement");
PLGitem 	*action = statement->getLabel("action");
PLGitem 	*instance = statement->getLabel("instance");
PLGitem 	*item = action->getLabel("statement");
PLGitem 	*otherwise = statement->getLabel("otherwise");
Instance 	*express = (Instance*)instance->itemValue;
Statement 	*line = (Statement*)item->itemValue;
Statement 	*ifStatement = new Statement(IF);
	ifStatement->add(express);
	ifStatement->pointInCode = iTEM;
	if ( currentMethod && currentMethod->directives && !noLoop )
		{
		Directive 	*directive = 0;
		currentMethod->directives->resetIterator();
		while ( directive = (Directive*)currentMethod->directives->next() )
			if ( directive->isDirected || !directive->codeMatch )
				continue;
			else
			if ( !::strncmp(directive->codeMatch,ifStatement->pointInCode->itemStart,::strlen(directive->codeMatch)) )
				{
				noLoop = 1;
				directive->parseDirective();
				noLoop = 0;
				break;
				}
		}
	line->indented = 1;
	if ( line )
		ifStatement->add(line);
	if ( otherwise )
		{
		item = otherwise->getLabel("statement");
		line = (Statement*)item->itemValue;
		if ( line )
			ifStatement->add(line);
		}
	statement->itemValue = (void*)ifStatement;
	return 1;
}

void Tawk::ElseTawkAct(PLGitem *iTEM)
{
}

int Tawk::ExpressListTawkNow(PLGitem *iTEM)
{
PLGitem 	*list = (PLGitem*)iTEM->children->get("list");
Instance 	*left = 0;
Instance 	*right = 0;
Expression 	*express = 0;
PLGitem 	*item = 0;
PLGitem 	*part = 0;
	for ( item = list; item; item = item->itemNext )
		{
		part = item->getLabel("instance");
		right = (Instance*)part->itemValue;
		if ( left )
			{
			express = new Expression(left,right,",");
			left = new Instance(express);
			}
		else	left = right;
		}
	list->itemValue = (void*)left;
	noShortcuts = 0;
	return 1;
}

int Tawk::ExpressPartTawkNow(PLGitem *iTEM)
{
PLGitem 	*unaryOp = (PLGitem*)iTEM->children->get("unaryOp");
PLGitem 	*instance = (PLGitem*)iTEM->children->get("instance");
PLGitem 	*express = (PLGitem*)iTEM->children->get("express");
Expression 	*expression = 0;
Instance 	*secondary = 0;
Instance 	*primary = 0;
PLGitem 	*item = 0;
PLGitem 	*operand = 0;
PLGitem 	*operate = 0;
PLGitem 	*stringPart = 0;
SymbolType 	*type = 0;
SymbolType 	*secondaryType = 0;
	primary = (Instance*)instance->itemValue;
	type = primary->getType();
	expressType = 0;
	if ( express )
		{
		if ( operate = express->getLabel("operate") )
			{
			operand = operate->getLabel("operand");
			if ( !operand )
				operand = operate->getLabel("comparator");
			}
		item = express->getLabel("instance");
		if ( item )
			{
			secondary = (Instance*)item->itemValue;
			if ( secondary->isRange )
				{
				expression = convertRangeX(primary,secondary);
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
				if ( operand && operand->toString() == "+=" && type == SymbolType::stringType && secondaryType == SymbolType::stringType && primary->howDirect() == 1 )
					{
					operand->setString("=");
					//cursor = item.start;
					stringing = 1;
					if ( stringPart = parse("Strings") )
						{
						//dealWith next
						item->itemNext = stringPart;
						instance->itemNext = item;
						}
					else	instance->itemNext = item;
					secondary = concatenate(instance);
					expression = new Expression(primary,secondary,"=");
					primary = new Instance(expression);
					}
				}
			}
		}
	if ( !expression && (primary->isVirtuous() || operand) )
		{
		expression = makeExpress(primary,express);
		primary = new Instance(expression);
		}
	if ( unaryOp )
		{
		//dealWith next
		if ( unaryOp->toString() == "&" && !unaryOp->itemNext )
			primary->setReference((unsigned int)1);
		else {
			expression = new Expression((Instance*)0,primary,unaryOp->toString());
			primary = new Instance(expression);
			}
		}
	primary = primary->checkOverload();
	if ( primary->symbol && primary->symbol->isDefault )
		primary->setDefaults("p");
	instance->itemValue = (void*)primary;
	setCurrentType((SymbolType*)0);
	popVirtuals();
	return 1;
}

int Tawk::ExpressionTawkNow(PLGitem *iTEM)
{
PLGitem 	*instance = (PLGitem*)iTEM->children->get("instance");
PLGitem 	*express = (PLGitem*)iTEM->children->get("express");
Expression 	*expression = 0;
Instance 	*primary = 0;
	primary = (Instance*)instance->itemValue;
	if ( express )
		{
		expression = makeExpress(primary,express);
		primary = new Instance(expression);
		}
	if ( !primary->express )
		{
		expression = new Expression();
		expression->subject = primary;
		primary = new Instance(expression);
		}
	instance->itemValue = (void*)primary;
	setCurrentType((SymbolType*)0);
	return 1;
}

int Tawk::ExtenderTawkNow(PLGitem *iTEM)
{
PLGitem 	*name = (PLGitem*)iTEM->children->get("name");
Symbol 		*method = 0;
char 		*text = name->toString();
	method = currentClass->findField(text);
	if ( method )
		method->extendType();
	else	::fprintf(stderr,"Overload could not find extender method: %s\n",iTEM->toString());
	name->unString();
	return 1;
}

void Tawk::FieldBody2TawkAct(PLGitem *iTEM)
{
PLGitem 	*name = (PLGitem*)iTEM->children->get("name");
PLGitem 	*item = 0;
	name->runDeferred(this);
	item = name->getLabel("instance");
	name->itemValue = item->itemValue;
}

int Tawk::FieldBody3TawkNow(PLGitem *iTEM)
{
PLGitem 	*name = (PLGitem*)iTEM->children->get("name");
PLGitem 	*item = name->getLabel("instance");
Instance 	*current = (Instance*)item->itemValue;
Expression 	*express = 0;
	if ( !current->express || current->cast )
		{
		express = new Expression(current,(Instance*)0,(char*)0);
		current = new Instance(express);
		}
	current->express->hasParens = 1;
	name->itemValue = (void*)current;
	return 1;
}

void Tawk::FieldBodyTawkAct(PLGitem *iTEM)
{
PLGitem 	*prefix = (PLGitem*)iTEM->children->get("prefix");
PLGitem 	*part = (PLGitem*)iTEM->children->get("part");
PLGitem 	*item = 0;
PLGitem 	*name = part->getLabel("name");
PLGitem 	*body = part->getLabel("body");
char 		*text = name->toString();
Symbol 		*symbol = 0;
double 		isArray = 0;
Instance 	*current = getInstance(text);
Instance 	*parameter = 0;
Instance 	*instance = 0;
Expression 	*expression = 0;
	if ( body )
		{
		item = body->getLabel("array");
		if ( item )
			isArray = item->getAmount();
		else {
			item = body->getLabel("expression");
			current->isMethod = 1;
			}
		for ( ; item; item = item->itemNext )
			{
			parameter = (Instance*)item->getLabel("instance")->itemValue;
			current->addParameter(parameter);
			}
		if ( !instance && current->isMethod )
			if ( currentType )
				instance = currentType->findMethodInstance(current);
			else	instance = currentSymbols->findMethod(current);
		}
	if ( !instance )
		{
		if ( isQualified )
			{
			SymbolType::types->resetIsFlagged();
			instance = currentType->findFieldInstance(current->prefix);
			if ( !instance && currentType->isOC )
				symbol = (Symbol*)SymbolType::ocSymbols->get(text);
			}
		else
		if ( instance = currentSymbols->find(current->prefix) )
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
			if ( isQualified )
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
			if ( isQualified )
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
		if ( instance = (Instance*)missingMethods->get(current->prefix) )
			{
			current->symbol = instance->symbol;
			current->setParent(instance->parent);
			current->prefix = 0;
			}
		else {
			current->prefix = current->mangle();
			missingMethods->put(current->prefix,current);
			current->symbol = new Symbol(text,SymbolType::nullType);
			current->symbol->isMethod = 1;
			current->symbol->methodName = current->prefix;
			current->prefix = 0;
			}
		}
	else
	if ( current->type == SymbolType::stringType && !current->isMethod && ((currentClass && currentClass->isVirtuous) || (currentType && currentType->isVirtuous) || virtualOp || assuming) )
		{
		/***********************************************************************
		We bail here so Qualified will fail because name value does not get
		set, letting AssumedString convert field to a string constant
		***********************************************************************/
		return;
		}
	else
	if ( !current->isVirtuous() )
		if ( *cursor != ':' )
			{
			text = ::concat(2,"FieldBody: could not find ",current->prefix);
			current = makeError(text);
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
	name->itemValue = (void*)current;
}

int Tawk::FieldExpressionTawkNow(PLGitem *iTEM)
{
PLGitem 	*cast = (PLGitem*)iTEM->children->get("cast");
PLGitem 	*direct = (PLGitem*)iTEM->children->get("direct");
PLGitem 	*instance = (PLGitem*)iTEM->children->get("instance");
Instance 	*castInstance = 0;
Instance 	*subject = 0;
PLGitem 	*item = instance->getLabel("field");
Instance 	*current = (Instance*)item->itemValue;
	if ( direct )
		current = current->setIndirectItem(direct);
	if ( cast )
		{
		direct = cast->getLabel("direct");
		item = cast->getLabel("type");
		castInstance = (Instance*)item->itemValue;
		if ( direct )
			castInstance = castInstance->setIndirectItem(direct);
		subject = new Instance(current);
		subject->cast = castInstance;
		current = subject;
		}
	instance->itemValue = (void*)current;
	return 1;
}

int Tawk::FieldTawkNow(PLGitem *iTEM)
{
PLGitem 	*name = (PLGitem*)iTEM->children->get("name");
Symbol 		*symbol = 0;
	if ( currentClass->components )
		{
		symbol = (Symbol*)currentClass->components->get(name->string());
		name->unString();
		}
	if ( symbol )
		name->itemValue = (void*)symbol;
	else	return 0;
	return 1;
}

int Tawk::FieldingTawkNow(PLGitem *iTEM)
{
PLGitem 	*name = (PLGitem*)iTEM->children->get("name");
char 		*text = name->toString();
Symbol 		*symbol = currentSymbols->presentClass->findField(text);
	if ( symbol && !currentType )
		setCurrentType(symbol->type);
	return 1;
}

int Tawk::FileNameTawkNow(PLGitem *iTEM)
{
PLGitem 	*name = (PLGitem*)iTEM->children->get("name");
	formatter->filename = name->toString();
	return 1;
}

int Tawk::ForOption2TawkNow(PLGitem *iTEM)
{
PLGitem 	*instance = (PLGitem*)iTEM->children->get("instance");
PLGitem 	*name = (PLGitem*)iTEM->children->get("name");
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
	initial = (Instance*)instance->itemValue;
	target = initial->express->subject;
	if ( !target )
		target = makeError("expression has no subject");
	else
	if ( !target->symbol )
		target = makeError("invalid subject");
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
						target = makeError(error);
						}
			}
		else
		if ( !target->symbol->typeMatch(symbol->type) )
			{
			error = ::concat(4,"type of ",symbol->name," does not match type of ",target->symbol->name);
			target = makeError(error);
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
	instance->itemValue = (void*)statement;
	return 1;
}

int Tawk::ForOptionTawkNow(PLGitem *iTEM)
{
PLGitem 	*instance = (PLGitem*)iTEM->children->get("instance");
PLGitem 	*initial = (PLGitem*)iTEM->children->get("initial");
PLGitem 	*condition = (PLGitem*)iTEM->children->get("condition");
PLGitem 	*increment = (PLGitem*)iTEM->children->get("increment");
Statement 	*statement = new Statement(FOR);
Instance 	*express = 0;
PLGitem 	*item = 0;
	statement->pointInCode = iTEM;
	if ( initial )
		{
		item = initial->getLabel("list");
		express = (Instance*)item->itemValue;
		statement->first = express;
		}
	if ( condition )
		{
		item = condition->getLabel("instance");
		express = (Instance*)item->itemValue;
		express->isCondition = 1;
		statement->second = express;
		}
	if ( increment )
		{
		item = increment->getLabel("list");
		express = (Instance*)item->itemValue;
		statement->third = express;
		}
	instance->itemValue = (void*)statement;
	noShortcuts = 0;
	return 1;
}

int Tawk::Include2TawkNow(PLGitem *iTEM)
{
PLGitem 	*include = (PLGitem*)iTEM->children->get("include");
	formatter->includeText->appendString(include->string(),0,0);
	include->unString();
	return 1;
}

int Tawk::Include3TawkNow(PLGitem *iTEM)
{
PLGitem 	*include = (PLGitem*)iTEM->children->get("include");
char 		*sourceFile = include->toString();
char 		*text = ::getStringFromFile(sourceFile);
	if ( !text )
		::fprintf(stderr,"Include: could not get text from %s\n",sourceFile);
	else
	if ( !includedFiles->get(sourceFile) )
		{
		includedFiles->add(sourceFile);
		//cout "including file: " sourceFile:;
		divertInput(text,"Divert");
		//cout `"Done with " start:;
		}
	else	::fprintf(stderr,"Include: source file already loaded, now ignored: %s\n",sourceFile);
	return 1;
}

int Tawk::IncludeTawkNow(PLGitem *iTEM)
{
PLGitem 	*include = (PLGitem*)iTEM->children->get("include");
	formatter->includeText->appendString(include->string(),0,0);
	formatter->includeText->appendString("\n",0,0);
	include->unString();
	return 1;
}

int Tawk::Inheritance2TawkNow(PLGitem *iTEM)
{
PLGitem 	*error = (PLGitem*)iTEM->children->get("error");
	if ( error )
		{
		::printf("ERROR Inheritance: at ==>%s\n",error->string());
		error->unString();
		}
	else	::printf("ERROR: no idea where\n");
	extending = 0;
	return 1;
}

void Tawk::InheritanceTawkAct(PLGitem *iTEM)
{
}

int Tawk::InitExpressionTawkNow(PLGitem *iTEM)
{
PLGitem 	*instance = (PLGitem*)iTEM->children->get("instance");
Instance 	*subject = 0;
PLGitem 	*last = 0;
PLGitem 	*item = 0;
SymbolType 	*type = 0;
	subject = (Instance*)instance->itemValue;
	type = subject->getType();
	if ( type == SymbolType::stringType && subject->howDirect() == 1 )
		{
		last = instance;
		while ( 1 )
			{
			item = parse("StringExpression");
			if ( item )
				{
				PLGitem 	*expressItem = item->getLabel("instance");
				Instance 	*secondary = (Instance*)expressItem->itemValue;
				type = secondary->getType();
				//dealWith
				/*
				if type == stringType
				{
				last.next = expressItem;
				last = last.next;
				}
				else break;
				*/
				}
			else	break;
			}
		if ( instance->itemNext )
			{
			subject = concatenate(instance);
			instance->itemValue = (void*)subject;
			}
		}
	return 1;
}

int Tawk::InitializerItemTawkNow(PLGitem *iTEM)
{
PLGitem 	*field = (PLGitem*)iTEM->children->get("field");
PLGitem 	*function = (PLGitem*)iTEM->children->get("function");
SymbolType 	*type = SymbolType::types->getFromItem(field);
char 		*text = function->string();
char 		*signature = ::concat(2,text,"(char*)");
Symbol 		*method = currentClass->findField(signature);
	if ( type )
		if ( method )
			{
			type->hasInitializer = 1;
			type->initializer = method;
			method->isInitializer = 1;
			}
		else	::fprintf(stderr,"InitializerItem rule could not find initialer method: %s\n",signature);
	else {
		Symbol 	*virtualField = currentClass->get(field);
		if ( !virtualField )
			if ( method )
				{
				virtualField = new Symbol(field->toString(),method->type);
				virtualField->isHidden = 1;
				virtualField->getter = method;
				currentClass->add(virtualField);
				}
			else	::fprintf(stderr,"InitializerItem rule could not find getter method: %s\n",signature);
		else	::fprintf(stderr,"InitializerItem rule: virtual field %s already exists\n",field->toString());
		}
	function->unString();
	return 1;
}

int Tawk::InitializerTawkNow(PLGitem *iTEM)
{
PLGitem 	*instance = (PLGitem*)iTEM->children->get("instance");
PLGitem 	*list = instance->getLabel("list");
	instance->itemValue = list->itemValue;
	return 1;
}

int Tawk::InstanceTailTawkNow(PLGitem *iTEM)
{
PLGitem 	*array = (PLGitem*)iTEM->children->get("array");
PLGitem 	*item = 0;
double 		i = 0;
	for ( item = array; item; item = item->itemNext )
		i++;
	array->amount = i;
	noShortcuts = 0;
	return 1;
}

int Tawk::ItemHeadTawkNow(PLGitem *iTEM)
{
PLGitem 	*array = (PLGitem*)iTEM->children->get("array");
PLGitem 	*item = 0;
double 		i = 0;
	if ( array )
		{
		for ( item = array; item; item = item->itemNext )
			i++;
		array->amount = i;
		}
	return 1;
}

int Tawk::Lambda2TawkNow(PLGitem *iTEM)
{
PLGitem 	*function = (PLGitem*)iTEM->children->get("function");
PLGitem 	*body = (PLGitem*)iTEM->children->get("body");
Instance 	*instance = 0;
Instance 	*bodyInstance = 0;
PLGitem 	*item = 0;
Statement 	*line = 0;
BlockTok 	*block = 0;
	item = function->deferred->find("head");
	instance = (Instance*)item->itemValue;
	if ( !instance->isLambda )
		return 0;
	instance->isDeclaration = 1;
	instance->symbol->isAssigned = 1;
	currentSymbols->add(instance);
	line = new Statement(LAMBDA);
	line->pointInCode = iTEM;
	line->add(instance);
	item = body->getLabel("start");
	block = (BlockTok*)item->itemValue;
	currentClass->hasLambda = 1;
	block->isLambda = 1;
	bodyInstance = new Instance(block);
	line->add(bodyInstance);
	// replaces prior commented out line
	instance = new Instance(line);
	function->itemValue = (void*)instance;
	return 1;
}

int Tawk::LambdaNameTawkNow(PLGitem *iTEM)
{
PLGitem 	*name = (PLGitem*)iTEM->children->get("name");
Instance 	*instance = 0;
char 		*text = 0;
	if ( !currentClass->hasLambda )
		return 0;
	text = name->string();
	if ( !SymbolType::find(text) )
		{
		instance = currentSymbols->find(text);
		name->unString();
		if ( instance && instance->symbol && instance->isLambda )
			{
			// Create a copy of instance w/o isDeclaration set
			instance = new Instance(instance->symbol);
			name->itemValue = (void*)instance;
			lambdaMethod = instance->symbol;
			}
		else	return 0;
		}
	else {
		name->unString();
		return 0;
		}
	return 1;
}

int Tawk::LambdaTawkNow(PLGitem *iTEM)
{
PLGitem 	*function = (PLGitem*)iTEM->children->get("function");
PLGitem 	*body = (PLGitem*)iTEM->children->get("body");
PLGitem 	*item = function->getLabel("name");
Instance 	*lambda = (Instance*)item->itemValue;
Instance 	*lambdaBody = 0;
Expression 	*express = 0;
BlockTok 	*block = 0;
	lambda->symbol->isAssigned = 1;
	lambda->assigning = 1;
	item = body->getLabel("start");
	block = (BlockTok*)item->itemValue;
	block->isLambda = 1;
	lambdaBody = new Instance(block);
	express = new Expression(lambda,lambdaBody,"=");
	lambda = new Instance(express);
	function->itemValue = (void*)lambda;
	return 1;
}

int Tawk::Line2TawkNow(PLGitem *iTEM)
{
char 		*text = 0;
Instance 	*label = 0;
PLGitem 	*item = 0;
Symbol 		*symbol = 0;
	for ( item = symbol->name; item; item = item->itemNext )
		{
		text = symbol->name->string();
		label = (Instance*)currentSymbols->instances->get(text);
		if ( !label )
			{
			symbol = new Symbol(symbol->name->toString(),SymbolType::voidType);
			label = new Instance(symbol);
			label->isLabel = 1;
			currentSymbols->add(label);
			}
		symbol->name->unString();
		if ( !label->isLabel )
			::fprintf(stderr,"ERROR: label %s already in scope\n",symbol->name);
		}
	return 1;
}

int Tawk::LineByLineTawkNow(PLGitem *iTEM)
{
PLGitem 	*line = (PLGitem*)iTEM->children->get("line");
Statement 	*statement = 0;
PLGitem 	*entry = 0;
PLGitem 	*method = 0;
	for ( entry = line; entry; entry = entry->itemNext )
		if ( method = entry->getLabel("statement") )
			{
			statement = (Statement*)method->itemValue;
			if ( statement )
				currentMethod->block->add(statement);
			}
	return 1;
}

int Tawk::LineTawkNow(PLGitem *iTEM)
{
PLGitem 	*target = (PLGitem*)iTEM->children->get("target");
Instance 	*instance = (Instance*)target->getLabel("instance")->itemValue;
	instance->level = 1;
	currentSymbols->add(instance);
	return 1;
}

int Tawk::MacroBlockTawkNow(PLGitem *iTEM)
{
PLGitem 	*line = (PLGitem*)iTEM->children->get("line");
Statement 	*statement = 0;
PLGitem 	*item = 0;
int 		indentFlag = 0;
	for ( ; line; line = line->itemNext )
		{
		item = line->getLabel("statement");
		if ( !item )
			continue;
		statement = (Statement*)item->itemValue;
		if ( !indentFlag )
			{
			statement->indented = 0;
			indentFlag = 1;
			}
		if ( statement )
			currentBlock->add(statement);
		}
	return 1;
}

int Tawk::MacroBodyTawkNow(PLGitem *iTEM)
{
PLGitem 	*parts = (PLGitem*)iTEM->children->get("parts");
PLGitem 	*body = 0;
PLGitem 	*last = 0;
PLGitem 	*other = 0;
PLGitem 	*part = 0;
PLGitem 	*rest = 0;
	for ( ; parts; parts = parts->itemNext )
		{
		if ( body && !last )
			last = body;
		if ( other = parts->getLabel("other") )
			{
			if ( other->itemLength )
				{
				if ( !body )
					body = other;
				if ( !last )
					last = body;
				else {
					//dealWith
					last->itemNext = other;
					last = last->itemNext;
					}
				}
			if ( part = (PLGitem*)other->itemValue )
				{
				if ( !body )
					body = part;
				if ( !last )
					last = body;
				else {
					//dealWith
					last->itemNext = part;
					last = last->itemNext;
					}
				other->itemValue = (void*)0;
				}
			}
		else
		if ( rest = parts->getLabel("rest") )
			{
			if ( !body )
				body = rest;
			if ( !last )
				last = body;
			else {
				//dealWith
				last->itemNext = rest;
				last = last->itemNext;
				}
			}
		}
	currentMethod->commentItem = body;
	return 1;
}

int Tawk::MacroDefineTawkNow(PLGitem *iTEM)
{
PLGitem 	*name = (PLGitem*)iTEM->children->get("name");
PLGitem 	*parameters = (PLGitem*)iTEM->children->get("parameters");
PLGitem 	*body = (PLGitem*)iTEM->children->get("body");
Symbol 		*argument = 0;
Symbol 		*symbol = 0;
Buffer 		*buffer = tokJunkBuffer;
	symbol = new Symbol(name->string(),SymbolType::voidType);
	buffer->reset();
	buffer->appendString(symbol->name,0,0);
	if ( !macroHash )
		macroHash = new BaseHash();
	else	macroHash->clear();
	if ( parameters )
		{
		PLGitem 	*item = parameters;
		symbol->isMethod = 1;
		buffer->appendString("(",0,0);
		for ( ; item; item = item->itemNext )
			{
			PLGitem 	*element = item->getLabel("element");
			argument = new Symbol(element->string());
			argument->isAlias = 1;
			symbol->addParameter(argument);
			buffer->appendString(argument->name,0,0);
			//dealWith
			if ( item->itemNext )
				buffer->appendString(",",0,0);
			macroHash->add(argument->name,(void*)argument);
			}
		buffer->appendString(")",0,0);
		}
	symbol->methodName = buffer->toString();
	symbol->isHidden = 1;
	// currentMethod is reused here temporarily then reset
	argument = currentMethod;
	currentMethod = symbol;
	if ( parameters )
		divertInput(body->string(),getRule("MacroBody"));
	else {
		symbol->commentItem = body;
		body->itemValue = (void*)0;
		}
	currentMethod = argument;
	if ( !macroList )
		macroList = new BaseHash();
	macroList->add(symbol->name,(void*)symbol);
	return 1;
}

int Tawk::MacroDelimitTawkNow(PLGitem *iTEM)
{
PLGitem 	*delimiter = (PLGitem*)iTEM->children->get("delimiter");
	macroDelimiter = delimiter->toString();
	return 1;
}

int Tawk::MacroNameTawkNow(PLGitem *iTEM)
{
PLGitem 	*name = (PLGitem*)iTEM->children->get("name");
	if ( !macroList || !macroList->get(name->toString()) )
		return 0;
	return 1;
}

void Tawk::MethodHeadTawkAct(PLGitem *iTEM)
{
PLGitem 	*direct = (PLGitem*)iTEM->children->get("direct");
PLGitem 	*function = (PLGitem*)iTEM->children->get("function");
PLGitem 	*head = (PLGitem*)iTEM->children->get("head");
Symbol 		*symbol = 0;
Symbol 		*argument = 0;
Instance 	*instance = 0;
PLGitem 	*item = 0;
PLGitem 	*atItem = 0;
PLGitem 	*ellipsis = head->getLabel("ellipsis");
PLGitem 	*parameter = head->getLabel("parameter");
char 		*name = 0;
char 		*methodName = 0;
int 		i = 0;
int 		construct = 0;
int 		lambda = direct && ::foundIn(lambdaSet,direct);
	if ( function )
		name = function->toString();
	else
	if ( lambda )
		name = "lambda";
	else {
		name = methodType->name;
		construct = 1;
		}
	if ( lambda )
		currentClass->hasLambda = 1;
	methodName = (char*)::alloca(1000);
	::strcpy(methodName,name);
	::strcat(methodName,"(");
	for ( atItem = parameter; atItem; atItem = atItem->itemNext )
		{
		item = atItem->getLabel("type");
		for ( item = (PLGitem*)item->itemValue; item; item = item->itemNext )
			{
			symbol = (Symbol*)item->itemValue;
			if ( symbol->isLambda || (symbol->reference && symbol->isMethod) )
				::strcat(methodName,symbol->getSignature());
			else {
				::strcat(methodName,symbol->type->name);
				for ( i = 0; i < symbol->indirect; i++ )
					::strcat(methodName,"*");
				//if symbol.reference strcat(methodName,"&");
				}
			if ( item->itemNext )
				::strcat(methodName,",");
			}
		if ( atItem->itemNext )
			::strcat(methodName,",");
		else
		if ( ellipsis )
			symbol->hasEllipsis = 1;
		}
	if ( ellipsis )
		::strcat(methodName,",null");
	::strcat(methodName,")");
	symbol = 0;
	if ( currentClass )
		if ( currentClass->isGlobal )
			symbol = currentSymbols->findGlobalMethod(methodName);
		else	symbol = currentClass->getMethod(methodName);
	/***************************************************************************
	If symbol then method was probably declared external and need to make
	sure the parameter names match up.
	***************************************************************************/
	if ( symbol )
		{
		while ( symbol->source )
			symbol = symbol->source;
		if ( !(currentClass->isGlobal && symbol->parentClass->isGlobal) && symbol->parentClass != currentClass )
			goto newSymbol;
		if ( symbol->type != methodType )
			::fprintf(stderr,"Warning: multiple types for %s\n",methodName);
		symbol->checkParameters(parameter);
		}
	else {
newSymbol:
		symbol = new Symbol(name,methodType);
		symbol->isMethod = 1;
		if ( constDeclare )
			symbol->isConst = 1;
		if ( inlineDeclare(linkage) )
			symbol->isInline = 1;
		if ( staticDeclare(linkage) )
			symbol->isStatic = 1;
		if ( virtualDeclare(linkage) )
			symbol->isVirtual = 1;
		if ( externDeclare(linkage) )
			{
			symbol->isExtern = 1;
			currentClass->hasExtern = 1;
			}
		if ( direct )
			symbol->setIndirection(direct);
		for ( atItem = parameter; atItem; atItem = atItem->itemNext )
			{
			item = atItem->getLabel("type");
			for ( item = (PLGitem*)item->itemValue; item; item = item->itemNext )
				{
				argument = (Symbol*)item->itemValue;
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
		if ( currentClass && !currentMethod && !processingParameters && !lambda && !symbol->reference )
			currentClass->addMethod(symbol);
		}
	symbol->isConstructor = construct;
	instance = new Instance(symbol);
	head->itemValue = (void*)instance;
	if ( !currentMethod && !processingParameters && !symbol->reference )
		currentMethod = symbol;
	if ( instance->isLambda )
		lambdaMethod = symbol;
}

int Tawk::MethodNameTawkNow(PLGitem *iTEM)
{
PLGitem 	*name = (PLGitem*)iTEM->children->get("name");
char 		*word = name->string();
SymbolType 	*type = SymbolType::find(word);
	if ( ReservedWord->find(word) || (type && !type->isGlobal) )
		{
		name->unString();
		return 0;
		}
	name->unString();
	return 1;
}

int Tawk::MethodTawkNow(PLGitem *iTEM)
{
PLGitem 	*method = (PLGitem*)iTEM->children->get("method");
PLGitem 	*block = (PLGitem*)iTEM->children->get("block");
PLGitem 	*body = block->getLabel("start");
PLGitem 	*head = method->deferred->find("head");
Instance 	*instance = 0;
	method->itemValue = head->itemValue;
	instance = (Instance*)method->itemValue;
	instance->checkSymbol();
	instance->symbol->block = (BlockTok*)body->itemValue;
	instance->symbol->block->isMethodBlock = 1;
	if ( currentComment )
		instance->symbol->comment = extractComment();
	missingMethods->remove(instance->symbol->methodName);
	if ( trueSymbol->directives )
		::printf("\t%s has directives\n",trueSymbol->methodName);
	else	::printf("\t%s\n",trueSymbol->methodName);
	currentMethod = 0;
	produceCodeFile = 1;
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

void Tawk::MethodTypeTawkAct(PLGitem *iTEM)
{
PLGitem 	*type = (PLGitem*)iTEM->children->get("type");
	methodType = (SymbolType*)type->itemValue;
}

int Tawk::MethodTypeTawkNow(PLGitem *iTEM)
{
PLGitem 	*modify = (PLGitem*)iTEM->children->get("modify");
PLGitem 	*type = (PLGitem*)iTEM->children->get("type");
PLGitem 	*methodHead = (PLGitem*)iTEM->children->get("methodHead");
Instance 	*instance = 0;
PLGitem 	*head = 0;
PLGitem 	*item = 0;
	// follow is not used but apparently it has to be there. Figure out why.
	methodType = (SymbolType*)type->itemValue;
	for ( item = modify; item; item = item->itemNext )
		if ( *item->itemStart == 'c' )
			constDeclare = 1;
		else
		if ( *item->itemStart == 'i' )
			linkage = 2;
		else
		if ( *item->itemStart == 's' )
			linkage = 3;
		else
		if ( *item->itemStart == 'v' )
			linkage = 4;
		else	linkage = 1;
	methodHead->runDeferred(this);
	head = methodHead->getLabel("head");
	instance = (Instance*)head->itemValue;
	if ( modify )
		{
		constDeclare = 0;
		linkage = 0;
		}
	declaringMethod = 1;
	return 1;
}

int Tawk::NameTawkNow(PLGitem *iTEM)
{
PLGitem 	*name = (PLGitem*)iTEM->children->get("name");
char 		*word = name->string();
SymbolType 	*type = SymbolType::find(word);
	if ( ReservedWord->find(word) || type )
		{
		name->unString();
		return 0;
		}
	name->unString();
	return 1;
}

void Tawk::NewTawkAct(PLGitem *iTEM)
{
PLGitem 	*instance = (PLGitem*)iTEM->children->get("instance");
PLGitem 	*type = (PLGitem*)iTEM->children->get("type");
PLGitem 	*body = (PLGitem*)iTEM->children->get("body");
PLGitem 	*initial = (PLGitem*)iTEM->children->get("initial");
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
		symbolType = (SymbolType*)type->itemValue;
	else	symbolType = newType;
	if ( !symbolType )
		symbolType = SymbolType::nullType;
	if ( symbolType == SymbolType::nullType )
		current = getInstance("No type found");
	if ( symbolType->constructor )
		{
		if ( currentMethod && ::compare(symbolType->constructor,currentMethod->name) == 0 )
			{
			//currentMethod.isConstructor = true;
			skipConstructor = currentMethod->isInitialized = 1;
			}
		if ( !skipConstructor )
			allocator = symbolType->constructor;
		}
	if ( !allocator )
		allocator = symbolType->name;
	if ( body )
		{
		item = body->getLabel("array");
		if ( item )
			{
			symbol = new Symbol("new",symbolType);
			current = new Instance(symbol);
			current->symbol->isArray = (unsigned int)item->getAmount();
			}
		else {
			if ( symbolType->isOC )
				{
				current = getInstance("init");
				current->type = symbolType;
				}
			else	current = getInstance(allocator);
			current->isMethod = 1;
			item = body->getLabel("expression");
			}
		}
	else
	if ( symbolType->isOC )
		{
		current = getInstance("init");
		current->isMethod = 1;
		current->type = symbolType;
		}
	else {
		current = getInstance(allocator);
		current->isMethod = 1;
		}
	for ( ; item; item = item->itemNext )
		{
		parameter = (Instance*)item->getLabel("instance")->itemValue;
		current->addParameter(parameter);
		}
	if ( symbol && initial )
		{
		parameter = (Instance*)initial->itemValue;
		express = new Expression(current,parameter,"=");
		current = new Instance(express);
		}
	if ( !symbol )
		{
		if ( symbolType->constructor && !skipConstructor )
			{
			parameter = currentSymbols->findMethod(current);
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
	instance->itemValue = (void*)current;
}

int Tawk::Number2TawkNow(PLGitem *iTEM)
{
PLGitem 	*instance = (PLGitem*)iTEM->children->get("instance");
PLGitem 	*isLong = (PLGitem*)iTEM->children->get("isLong");
Instance 	*number = 0;
SymbolType 	*type = 0;
	type = isLong ? SymbolType::longType : SymbolType::intType;
	number = getInstance(instance->toString());
	number->isConstant = 1;
	number->type = type;
	if ( isLong )
		number->postfix = "LL";
	instance->itemValue = (void*)number;
	return 1;
}

int Tawk::NumberTawkNow(PLGitem *iTEM)
{
PLGitem 	*instance = (PLGitem*)iTEM->children->get("instance");
Instance 	*number = 0;
SymbolType 	*type = SymbolType::doubleType;
	number = getInstance(instance->toString());
	number->isConstant = 1;
	number->type = type;
	instance->itemValue = (void*)number;
	return 1;
}

int Tawk::OperationTail2TawkNow(PLGitem *iTEM)
{
PLGitem 	*instance = (PLGitem*)iTEM->children->get("instance");
PLGitem 	*question = instance->getLabel("question");
	instance->itemValue = question->itemValue;
	return 1;
}

int Tawk::OperationTailTawkNow(PLGitem *iTEM)
{
PLGitem 	*operate = (PLGitem*)iTEM->children->get("operate");
PLGitem 	*instance = (PLGitem*)iTEM->children->get("instance");
PLGitem 	*question = (PLGitem*)iTEM->children->get("question");
PLGitem 	*item = 0;
PLGitem 	*operand = 0;
SymbolType 	*type = 0;
Expression 	*expression = 0;
Instance 	*subject = 0;
Instance 	*trueValue = 0;
char 		*flag = 0;
char 		*save = cursor;
	operand = operate->getLabel("operand");
	if ( operand )
		{
		if ( stringing && expressType == SymbolType::stringType && !::foundIn(stringOP,operand) )
			return 0;
		if ( *operand->itemStart == '=' && operand->itemLength == 1 )
			flag = operand->itemStart + 1;
		}
	else
	if ( operand = operate->getLabel("comparator") )
		flag = operand->itemStart + 3;
	if ( question )
		{
		subject = (Instance*)instance->itemValue;
		trueValue = (Instance*)question->itemValue;
		expression = new Expression(subject,trueValue,"?");
		subject = new Instance(expression);
		instance->itemValue = (void*)subject;
		flag = 0;
		}
	else	subject = (Instance*)instance->itemValue;
	if ( subject->parent && subject->isConstant )
		instance->itemValue = (void*)subject;
	expressType = type = subject->getType();
	if ( flag )
		if ( type == SymbolType::stringType && subject->howDirect() == 1 )
			{
			cursor = flag;
			stringing = 1;
			if ( item = parse("Strings") )
				{
				subject = concatenate(item->getLabel("item"));
				instance->itemValue = (void*)subject;
				}
			else	cursor = save;
			stringing = 0;
			}
	assigning = 0;
	return 1;
}

int Tawk::Operator2TawkNow(PLGitem *iTEM)
{
PLGitem 	*comparator = (PLGitem*)iTEM->children->get("comparator");
	if ( !compareFollow->contains(*(comparator->itemStart + 2)) )
		return 0;
	return 1;
}

int Tawk::OperatorTawkNow(PLGitem *iTEM)
{
PLGitem 		*operand = (PLGitem*)iTEM->children->get("operand");
KeyTableItem 	*operatorItem = (KeyTableItem*)operand->itemValue;
Operate 		*verb = (Operate*)operatorItem->value;
	if ( verb->overload )
		return 1;
	if ( verb->isRange )
		return 0;
	if ( assigning && *operand->itemStart == ',' )
		return 0;
	if ( !verb || verb->conjunction || verb->question )
		return 0;
	if ( virtualItem )
		{
		SymbolType 	*type = virtualItem->getType();
		if ( type->isVirtuous )
			if ( type->overloaded(verb->op) || (virtualItem->arrayRef && *operand->itemStart == '=' && operand->itemLength == 1 && type->overloaded("[]=")) )
				virtualOp = operand;
		}
	if ( verb->assign && saveType )
		{
		newType = saveType;
		if ( expressType == SymbolType::stringType && !assuming )
			assigning = 1;
		}
	return 1;
}

int Tawk::OverLoadItem2TawkNow(PLGitem *iTEM)
{
PLGitem 	*assign = (PLGitem*)iTEM->children->get("assign");
PLGitem 	*name = (PLGitem*)iTEM->children->get("name");
char 		*text = name->toString();
	if ( currentClass )
		if ( assign )
			currentClass->overload("[]=",text);
		else	currentClass->overload("[]",text);
	else	::fprintf(stderr,"Overload specification must be within class\n");
	currentClass->isVirtuous = 1;
	return 1;
}

int Tawk::OverLoadItem3TawkNow(PLGitem *iTEM)
{
PLGitem 	*operate = (PLGitem*)iTEM->children->get("operate");
PLGitem 	*name = (PLGitem*)iTEM->children->get("name");
char 		*text = name->toString();
	if ( currentClass )
		currentClass->overload(operate->toString(),text);
	else	::fprintf(stderr,"Overload specification must be within class\n");
	return 1;
}

int Tawk::OverLoadItem4TawkNow(PLGitem *iTEM)
{
PLGitem 	*newOp = (PLGitem*)iTEM->children->get("newOp");
PLGitem 	*name = (PLGitem*)iTEM->children->get("name");
Operate 	*verb = new Operate(newOp->toString());
char 		*text = name->toString();
	if ( currentClass )
		currentClass->overload(verb->op,text);
	else	::fprintf(stderr,"Overload specification must be within class\n");
	return 1;
}

int Tawk::OverLoadItem5TawkNow(PLGitem *iTEM)
{
PLGitem 	*name = (PLGitem*)iTEM->children->get("name");
char 		*text = name->toString();
	if ( currentClass )
		currentClass->overload("()",text);
	else	::fprintf(stderr,"Overload specification must be within class\n");
	return 1;
}

int Tawk::OverLoadItemTawkNow(PLGitem *iTEM)
{
PLGitem 	*operate = (PLGitem*)iTEM->children->get("operate");
PLGitem 	*name = (PLGitem*)iTEM->children->get("name");
PLGitem 	*operand = operate->getLabel("operand");
char 		*text = name->toString();
	assigning = 0;
	if ( !operand )
		operand = operate->getLabel("comparator");
	if ( !currentClass )
		::fprintf(stderr,"Overload specification must be within class: %s\n",iTEM->toString());
	else	currentClass->overload(operand->toString(),text);
	name->unString();
	return 1;
}

void Tawk::ParameterItem2TawkAct(PLGitem *iTEM)
{
PLGitem 	*direct = (PLGitem*)iTEM->children->get("direct");
PLGitem 	*name = (PLGitem*)iTEM->children->get("name");
PLGitem 	*array = (PLGitem*)iTEM->children->get("array");
Symbol 		*symbol = 0;
	symbol = new Symbol(name->toString(),currentType);
	if ( direct )
		symbol->setIndirection(direct);
	if ( array )
		{
		symbol->array = array->toString();
		for ( ; array; array = array->itemNext )
			{
			symbol->indirect++;
			symbol->isArray++;
			}
		}
	name->itemValue = (void*)symbol;
}

void Tawk::ParameterItem3TawkAct(PLGitem *iTEM)
{
PLGitem 	*name = (PLGitem*)iTEM->children->get("name");
Symbol 		*symbol = 0;
	symbol = new Symbol("",currentType);
	symbol->array = name->toString();
	for ( ; name; name = name->itemNext )
		{
		symbol->indirect++;
		symbol->isArray++;
		}
	name->itemValue = (void*)symbol;
	name->itemNext = 0;
}

void Tawk::ParameterItem4TawkAct(PLGitem *iTEM)
{
PLGitem 	*name = (PLGitem*)iTEM->children->get("name");
Symbol 		*symbol = 0;
	symbol = new Symbol("",currentType);
	symbol->setIndirection(name);
	name->itemValue = (void*)symbol;
}

void Tawk::ParameterItemTawkAct(PLGitem *iTEM)
{
PLGitem 	*name = (PLGitem*)iTEM->children->get("name");
PLGitem 	*head = name->getLabel("head");
Instance 	*instance = 0;
	processingParameters = 1;
	name->runDeferred(this);
	instance = (Instance*)head->itemValue;
	name->itemValue = (void*)instance->symbol;
	processingParameters = 0;
}

int Tawk::ParameterTawkNow(PLGitem *iTEM)
{
PLGitem 	*type = (PLGitem*)iTEM->children->get("type");
PLGitem 	*item = (PLGitem*)iTEM->children->get("item");
Symbol 		*symbol = 0;
PLGitem 	*last = 0;
PLGitem 	*name = 0;
	methodType = (SymbolType*)type->itemValue;
	if ( !item )
		{
		symbol = new Symbol("",methodType);
		name = new PLGitem("no field specified");
		name->itemValue = (void*)symbol;
		}
	else
	for ( ; item; item = item->itemNext )
		{
		item->runDeferred(this);
		if ( !last )
			last = name = item->getLabel("name");
		else
		if ( last->itemNext = item->getLabel("name") )
			last = last->itemNext;
		symbol = (Symbol*)last->itemValue;
		if ( type->flag4 )
			symbol->isConst = 1;
		}
	type->itemValue = (void*)name;
	return 1;
}

int Tawk::PoundCommandTawkNow(PLGitem *iTEM)
{
PLGitem 	*state = (PLGitem*)iTEM->children->get("state");
PLGitem 	*type = (PLGitem*)iTEM->children->get("type");
PLGitem 	*level = (PLGitem*)iTEM->children->get("level");
PLGitem 	*list = (PLGitem*)iTEM->children->get("list");
PLGitem 	*field = (PLGitem*)iTEM->children->get("field");
SymbolType 	*symbolType = 0;
PLGitem 	*end = state;
	switch (*state->itemStart)
		{
		case 'a':
			if ( currentClass )
				currentClass->autoGetSet = !currentClass->getAutoGetSet();
			break;
		case 'd':
			if ( *(state->itemStart + 1) == 'E' )
				if ( list )
					{
					symbolType->debug = 0;
					end = list;
					}
				else
				if ( symbolType->debug )
					symbolType->debug = 0;
				else	debugRulePLG = 1;
			else {
				if ( type )
					{
					if ( symbolType = (SymbolType*)type->itemValue )
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
					currentSymbols->dump();
					end = level;
					}
				else {
					currentSymbols->dump("Symbol Table");
					currentSymbols->dumpGlobals();
					}
				}
			break;
		case 'i':
			if ( currentMethod )
				{
				currentMethod->isInitialized = 1;
				if ( currentMethod->isAlias )
					currentMethod->source->isInitialized = 1;
				}
			break;
		case 'm':
			debugging = !debugging;
			break;
		case 'r':
			defaultPrinter = currentSymbols->find("printf");
			break;
		case 's':
			summaryDebug();
			break;
		case 't':
			::printf("At trace: %d %d\n",Symbol::symbolCount,Instance::instanceCount);
			//debugTest = !debugTest;
		}
	cursor = end->itemStart + end->itemLength;
	return 1;
}

int Tawk::PrintCommand2TawkNow(PLGitem *iTEM)
{
PLGitem 	*stdPrint = (PLGitem*)iTEM->children->get("stdPrint");
Instance 	*newPrinter = currentSymbols->find("printf");
	stdPrint->itemValue = (void*)newPrinter;
	return 1;
}

int Tawk::PrintCommand3TawkNow(PLGitem *iTEM)
{
PLGitem 	*stdPrint = (PLGitem*)iTEM->children->get("stdPrint");
Instance 	*target = currentSymbols->find("stderr");
Instance 	*newPrinter = currentSymbols->find("fprintf");
	newPrinter->addParameter(target);
	stdPrint->itemValue = (void*)newPrinter;
	return 1;
}

int Tawk::PrintCommandTawkNow(PLGitem *iTEM)
{
PLGitem 	*printer = (PLGitem*)iTEM->children->get("printer");
PLGitem 	*target = (PLGitem*)iTEM->children->get("target");
	printer->itemValue = (void*)processPrintTarget(target);
	return 1;
}

int Tawk::PrintItem2TawkNow(PLGitem *iTEM)
{
PLGitem 	*instance = (PLGitem*)iTEM->children->get("instance");
PLGitem 	*format = (PLGitem*)iTEM->children->get("format");
Instance 	*item = (Instance*)instance->itemValue;
	if ( format )
		{
		PLGitem 	*width = format->getLabel("width");
		if ( format->itemLength > 1 )
			{
			*format->itemStart = '%';
			}
		item->format = getInstance(format->toString());
		item->format->type = SymbolType::stringType;
		item->format->indirection = 1;
		item->format->isConstant = 1;
		if ( width )
			{
			item->format->format = getInstance(width->toString());
			item->format->format->type = SymbolType::intType;
			item->format->format->isConstant = 1;
			}
		}
	return 1;
}

void Tawk::PrintItemTawkAct(PLGitem *iTEM)
{
}

int Tawk::PrintShortcutTawkNow(PLGitem *iTEM)
{
PLGitem 	*instance = (PLGitem*)iTEM->children->get("instance");
Instance 	*item = 0;
	if ( *instance->itemStart == ',' )
		item = getInstance(" ");
	else
	if ( *instance->itemStart == '`' )
		item = getInstance("\\t");
	else	item = getInstance("\\n");
	item->isConstant = 1;
	item->type = SymbolType::stringType;
	item->indirection = 1;
	instance->itemValue = (void*)item;
	return 1;
}

int Tawk::PrintTawkNow(PLGitem *iTEM)
{
PLGitem 	*start = (PLGitem*)iTEM->children->get("start");
PLGitem 	*arguments = (PLGitem*)iTEM->children->get("arguments");
PLGitem 	*output = (PLGitem*)iTEM->children->get("output");
PLGitem 	*item = 0;
PLGitem 	*argument = 0;
Instance 	*method = 0;
SymbolType 	*type = 0;
BlockTok 	*block = 0;
Statement 	*statement = 0;
Instance 	*converter = 0;
Instance 	*format = 0;
Instance 	*savePrinter = defaultPrinter;
Instance 	*text = 0;
	if ( item = start->getLabel("printer") )
		if ( output )
			method = processPrintTarget(output);
		else	method = (Instance*)item->itemValue;
	else {
		item = start->getLabel("stdPrint");
		method = (Instance*)item->itemValue;
		}
	if ( !method->isPrintMethod )
		{
		tokJunkBuffer->reset();
		for ( argument = arguments; argument; argument = argument->itemNext )
			{
			format = 0;
			item = argument->getLabel("instance");
			text = (Instance*)item->itemValue;
			format = text->getFormat();
			type = text->getType();
			if ( text->express && !text->express->verb && !text->cast )
				text = text->getSubject();
			if ( text->isConstant && !text->isMethod )
				tokJunkBuffer->appendString(text->prefix,0,0);
			else
			if ( type != SymbolType::stringType && text->howDirect() <= 1 && type->getMethod("toString") )
				tokJunkBuffer->appendString("%s",0,0);
			else
			if ( format )
				tokJunkBuffer->appendString(format->prefix,0,0);
			else {
				text->error("No toString method");
				tokJunkBuffer->appendString("%s",0,0);
				}
			}
		text = getInstance(tokJunkBuffer->toString());
		text->type = SymbolType::stringType;
		text->isConstant = 1;
		text->indirection = 1;
		method->addParameter(text);
		for ( argument = arguments; argument; argument = argument->itemNext )
			{
			item = argument->getLabel("instance");
			text = (Instance*)item->itemValue;
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
		if ( arguments->itemNext )
			block = new BlockTok();
		for ( argument = arguments; argument; argument = argument->itemNext )
			{
			item = argument->getLabel("instance");
			text = (Instance*)item->itemValue;
			method = generatePrint(text);
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
	start->itemValue = (void*)method;
	if ( output )
		defaultPrinter = savePrinter;
	return 1;
}

int Tawk::QualifiedTawkNow(PLGitem *iTEM)
{
PLGitem 	*type = (PLGitem*)iTEM->children->get("type");
PLGitem 	*field = (PLGitem*)iTEM->children->get("field");
PLGitem 	*rest = (PLGitem*)iTEM->children->get("rest");
PLGitem 	*postfix = (PLGitem*)iTEM->children->get("postfix");
PLGitem 	*item = 0;
PLGitem 	*tail = 0;
PLGitem 	*name = field->deferred->find("name");
Instance 	*parent = 0;
Instance 	*child = 0;
	isQualified = 0;
	if ( type )
		{
		setCurrentType((SymbolType*)type->itemValue);
		child = new Instance(currentType);
		}
	if ( child )
		isQualified = 1;
	field->runDeferred(this);
	parent = (Instance*)name->itemValue;
	/*************************************************************************
	If field is an assumed string, parent will be none so we bail
	*************************************************************************/
	if ( !parent )
		return 0;
	// The following inserts the parent class reference
	if ( child )
		parent = parent->copyAndSetParent(child);
	setCurrentType(parent->getType());
	if ( rest )
		isQualified = 1;
	/*************************************************************************
	If this is a qualifier (has following .) or if there is not symbol
	(unknown field) resolve virtue here
	*************************************************************************/
	if ( rest && currentType && currentType->isVirtuous && parent->arrayRef && !parent->resolved )
		{
		parent = parent->checkOverload();
		if ( parent->resolved )
			{
			setCurrentType(parent->getType());
			// Not sure why I do the following
			if ( parent->reference && currentType->isOC )
				parent->setReference((unsigned int)0);
			}
		}
	for ( item = rest; item; item = item->itemNext )
		{
		tail = item->getLabel("field");
		tail->runDeferred(this);
		name = tail->deferred->find("name");
		if ( child = (Instance*)name->itemValue )
			{
			if ( !child->symbol )
				if ( currentType->isOC )
					;
				else
				if ( currentType->isVirtuous )
					{
					char 	*text = ::concat(2,child->prefix," is not a valid symbol");
					child->error(text);
					}
			child = child->copyAndSetParent(parent);
			}
		parent = child;
		if ( !parent )
			break;
		setCurrentType(parent->getType());
		}
	saveType = currentType;
	setCurrentType((SymbolType*)0);
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
		parent->setDefaults("p");
	field->itemValue = (void*)parent;
	isQualified = 0;
	return 1;
}

int Tawk::QualifyStartTawkNow(PLGitem *iTEM)
{
PLGitem 	*name = (PLGitem*)iTEM->children->get("name");
Symbol 		*symbol = 0;
Instance 	*instance = 0;
	if ( currentClass->isC )
		return 0;
	symbol = new Symbol("this",currentClass);
	instance = new Instance(symbol);
	name->itemValue = (void*)instance;
	return 1;
}

int Tawk::QuestionTawkNow(PLGitem *iTEM)
{
PLGitem 	*question = (PLGitem*)iTEM->children->get("question");
PLGitem 	*trueExp = (PLGitem*)iTEM->children->get("trueExp");
PLGitem 	*falseExp = (PLGitem*)iTEM->children->get("falseExp");
PLGitem 	*falseItem = 0;
PLGitem 	*trueItem = 0;
Expression 	*expression = 0;
Instance 	*trueValue = 0;
Instance 	*falseValue = 0;
	falseItem = falseExp->getLabel("instance");
	trueItem = trueExp->getLabel("instance");
	trueValue = (Instance*)trueItem->itemValue;
	falseValue = (Instance*)falseItem->itemValue;
	expression = new Expression(trueValue,falseValue,":");
	trueValue = new Instance(expression);
	question->itemValue = (void*)trueValue;
	return 1;
}

int Tawk::QuoteTawkNow(PLGitem *iTEM)
{
PLGitem 	*string = (PLGitem*)iTEM->children->get("string");
PLGitem 	*instance = (PLGitem*)iTEM->children->get("instance");
PLGitem 	*body = (PLGitem*)iTEM->children->get("body");
Buffer 		*buffer = tokJunkBuffer;
Instance 	*current = 0;
char 		*mark = 0;
	/**********************************************************************
	Check for multi-line strings
	**********************************************************************/
	buffer->reset();
	if ( !body )
		current = getInstance("");
	else {
		mark = body->string();
		while ( *mark )
			{
			if ( *mark == '\n' )
				{
				buffer->appendChar('\\',0,0);
				buffer->appendChar('n',0,0);
				}
			else	buffer->appendChar(*mark,0,0);
			mark++;
			}
		current = getInstance(buffer->toString());
		body->unString();
		}
	current->isConstant = 1;
	current->indirection = 1;
	// constant strings are really pointers
	current->type = SymbolType::stringType;
	if ( string )
		current->atString = 1;
	instance->itemValue = (void*)current;
	return 1;
}

int Tawk::RangeExpressionTawkNow(PLGitem *iTEM)
{
PLGitem 	*instance = (PLGitem*)iTEM->children->get("instance");
PLGitem 	*back = (PLGitem*)iTEM->children->get("back");
PLGitem 	*item = 0;
Instance 	*front = (Instance*)instance->itemValue;
Instance 	*tail = 0;
Expression 	*rangeX = 0;
	item = back->getLabel("instance");
	tail = (Instance*)item->itemValue;
	item = back->getLabel("operate");
	rangeX = new Expression(front,tail,item->string());
	front = new Instance(rangeX);
	front->isRange = 1;
	instance->itemValue = (void*)front;
	return 1;
}

int Tawk::RangeFieldTawkNow(PLGitem *iTEM)
{
PLGitem 	*instance = (PLGitem*)iTEM->children->get("instance");
Instance 	*rangeField = (Instance*)currentSymbols->instances->get(instance->string());
	instance->unString();
	if ( rangeField && rangeField->isRange && rangeField->express && rangeField->express->object )
		instance->itemValue = (void*)rangeField->express->object;
	else	return 0;
	return 1;
}

int Tawk::SecondaryExpression2TawkNow(PLGitem *iTEM)
{
PLGitem 	*instance = (PLGitem*)iTEM->children->get("instance");
PLGitem 	*pointer = (PLGitem*)iTEM->children->get("pointer");
char 		*text = 0;
	if ( pointer )
		{
		text = ::concat(5,instance->string(),"(",currentType->name,pointer->string(),")");
		pointer->unString();
		}
	else	text = ::concat(4,instance->string(),"(",currentType->name,")");
Instance 	*current = getInstance(text);
	current->type = SymbolType::intType;
	current->isConstant = 1;
	current->isMethod = 1;
	// so will not screw up as a print argument
	instance->itemValue = (void*)current;
	instance->unString();
	currentType->setRefer();
	return 1;
}

int Tawk::SecondaryExpressionTawkNow(PLGitem *iTEM)
{
PLGitem 	*instance = (PLGitem*)iTEM->children->get("instance");
Instance 	*current = new Instance(nullSymbol);
	instance->itemValue = (void*)current;
	current->isConstant = 1;
	return 1;
}

int Tawk::Statement2TawkNow(PLGitem *iTEM)
{
PLGitem 	*statement = (PLGitem*)iTEM->children->get("statement");
Statement 	*line = (Statement*)statement->itemValue;
	if ( line )
		{
		if ( !line->pointInCode )
			line->pointInCode = iTEM;
		line->setIsUsed();
		if ( currentMethod && currentMethod->directives && !noLoop )
			{
			Directive 	*directive = 0;
			currentMethod->directives->resetIterator();
			while ( directive = (Directive*)currentMethod->directives->next() )
				if ( directive->isDirected || !directive->codeMatch )
					continue;
				else
				if ( !::strncmp(directive->codeMatch,line->pointInCode->itemStart,::strlen(directive->codeMatch)) )
					{
					noLoop = 1;
					parsingDirective = 1;
					directive->parseDirective();
					parsingDirective = 0;
					noLoop = 0;
					break;
					}
			}
		}
	return 1;
}

int Tawk::StatementBody10TawkNow(PLGitem *iTEM)
{
PLGitem 	*statement = (PLGitem*)iTEM->children->get("statement");
Instance 	*instance = getInstance("continue");
Statement 	*line = new Statement();
	line->add(instance);
	// continue
	line->branch = 1;
	statement->itemValue = (void*)line;
	return 1;
}

int Tawk::StatementBody11TawkNow(PLGitem *iTEM)
{
PLGitem 	*statement = (PLGitem*)iTEM->children->get("statement");
PLGitem 	*direct = (PLGitem*)iTEM->children->get("direct");
PLGitem 	*field = (PLGitem*)iTEM->children->get("field");
Instance 	*instance = 0;
Statement 	*line = new Statement(GOTO);
	// goto
	instance = (Instance*)field->itemValue;
	if ( direct )
		instance = instance->setIndirectItem(direct);
	line->add(instance);
	line->branch = 1;
	statement->itemValue = (void*)line;
	return 1;
}

int Tawk::StatementBody12TawkNow(PLGitem *iTEM)
{
PLGitem 	*statement = (PLGitem*)iTEM->children->get("statement");
PLGitem 	*block = (PLGitem*)iTEM->children->get("block");
Statement 	*switchStatement = new Statement(SWITCH);
PLGitem 	*item = block->getLabel("start");
BlockTok 	*body = (BlockTok*)item->itemValue;
Instance 	*trigger = (Instance*)statement->itemValue;
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
	statement->itemValue = (void*)switchStatement;
	if ( !trigger || trigger->isRange || (!type->isNumber && !(type == SymbolType::stringType && trigger->howDirect() != 1)) )
		switchStatement->switching = 1;
	switchStack->pop();
	return 1;
}

int Tawk::StatementBody13TawkNow(PLGitem *iTEM)
{
PLGitem 	*statement = (PLGitem*)iTEM->children->get("statement");
Statement 	*line = new Statement();
	// ;
	statement->itemValue = (void*)line;
	return 1;
}

int Tawk::StatementBody14TawkNow(PLGitem *iTEM)
{
PLGitem 	*statement = (PLGitem*)iTEM->children->get("statement");
PLGitem 	*array = (PLGitem*)iTEM->children->get("array");
PLGitem 	*instance = (PLGitem*)iTEM->children->get("instance");
Statement 	*line = new Statement(DELETE);
	// delete
PLGitem 	*item = instance->getLabel("field");
Instance 	*current = (Instance*)item->itemValue;
	if ( array )
		current->postfix = "[]";
	line->add(current);
	statement->itemValue = (void*)line;
	return 1;
}

int Tawk::StatementBody15TawkNow(PLGitem *iTEM)
{
PLGitem 	*statement = (PLGitem*)iTEM->children->get("statement");
PLGitem 	*instance = (PLGitem*)iTEM->children->get("instance");
Statement 	*doStatement = new Statement(DO);
Instance 	*test = (Instance*)instance->itemValue;
Statement 	*body = (Statement*)statement->itemValue;
	// do
	doStatement->add(body);
	doStatement->add(test);
	doStatement->pointInCode = iTEM;
	test->isCondition = 1;
	statement->itemValue = (void*)doStatement;
	iterating--;
	return 1;
}

int Tawk::StatementBody16TawkNow(PLGitem *iTEM)
{
PLGitem 	*statement = (PLGitem*)iTEM->children->get("statement");
Statement 	*line = new Statement();
	// throw
Instance 	*instance = 0;
	instance = getInstance("Saw a throw expression");
	instance->isComment = 1;
	line->add(instance);
	statement->itemValue = (void*)line;
	return 1;
}

int Tawk::StatementBody17TawkNow(PLGitem *iTEM)
{
PLGitem 	*statement = (PLGitem*)iTEM->children->get("statement");
Statement 	*line = new Statement();
	// try
Instance 	*instance = 0;
	instance = getInstance("Saw a try expression");
	instance->isComment = 1;
	line->add(instance);
	statement->itemValue = (void*)line;
	return 1;
}

int Tawk::StatementBody18TawkNow(PLGitem *iTEM)
{
PLGitem 	*statement = (PLGitem*)iTEM->children->get("statement");
PLGitem 	*declare = statement->getLabel("declare");
PLGitem 	*entry = 0;
Statement 	*line = 0;
Instance 	*stacked = 0;
Instance 	*instance = 0;
Expression 	*express = 0;
	statement->runDeferred(this);
	// declaration
	virtualStack->clear();
	for ( entry = declare; entry; entry = entry->itemNext )
		{
		instance = (Instance*)entry->itemValue;
		if ( !instance || instance->type )
			continue;
		if ( instance->symbol && instance->symbol->type->hasInitializer && entry->getLabel("initialize") )
			{
			Expression 	*initialize = 0;
			/*************************************************************
			The instance being declared does not have an initializing
			expression but its class has an initializer method
			(specified as an initializer in an external type declaration).
			Add the call to the initializer here.
			*************************************************************/
			Instance 	*assigned = findInitializer(instance->symbol);
			if ( assigned )
				{
				Instance 	*argument = getInstance(instance->symbol->name);
				argument->isConstant = 1;
				argument->indirection = 1;
				argument->type = SymbolType::stringType;
				assigned->addParameter(argument);
				if ( assigned->symbol->isDefault )
					assigned->setDefaults("p");
				initialize = new Expression(instance,assigned,"=");
				instance = new Instance(initialize);
				}
			}
		if ( instance->symbol )
			{
			virtualStack->push(instance);
			instance->isLocal = 1;
			}
		else
		if ( instance->express )
			if ( instance->isRange )
				virtualStack->push(instance);
			else {
				Instance 	*temp = instance->express->subject;
				virtualStack->push(temp);
				temp->isLocal = 1;
				}
		if ( express )
			{
			express->verb = commaOp;
			express->object = instance;
			if ( instance->symbol )
				instance->isDeclaration = 0;
			if ( instance->express )
				instance->express->subject->isDeclaration = 0;
			instance = new Instance(express);
			}
		if ( entry->itemNext )
			{
			express = new Expression();
			express->subject = instance;
			}
		}
	if ( virtualStack->length )
		{
		virtualStack->entry = 0;
		while ( stacked = (Instance*)virtualStack->next() )
			currentSymbols->add(stacked);
		virtualStack->clear();
		}
	if ( !instance->isRange )
		{
		instance->isDeclaration = 1;
		line = new Statement();
		line->add(instance);
		line->indented = 0;
		statement->itemValue = (void*)line;
		}
	return 1;
}

int Tawk::StatementBody19TawkNow(PLGitem *iTEM)
{
PLGitem 	*statement = (PLGitem*)iTEM->children->get("statement");
PLGitem 	*item = statement->getLabel("instance");
Statement 	*line = new Statement();
Instance 	*instance = (Instance*)item->itemValue;
	line->add(instance);
	// expression
	statement->itemValue = (void*)line;
	return 1;
}

int Tawk::StatementBody20TawkNow(PLGitem *iTEM)
{
PLGitem 	*statement = (PLGitem*)iTEM->children->get("statement");
PLGitem 	*item = statement->getLabel("function");
Statement 	*line = 0;
	// Lambda
Instance 	*lambda = (Instance*)item->itemValue;
	if ( lambda->statement )
		statement->itemValue = (void*)lambda->statement;
	else {
		line = new Statement();
		statement->itemValue = (void*)line;
		line->add(lambda);
		}
	return 1;
}

int Tawk::StatementBody2TawkNow(PLGitem *iTEM)
{
PLGitem 	*statement = (PLGitem*)iTEM->children->get("statement");
PLGitem 	*item = statement->getLabel("start");
BlockTok 	*block = (BlockTok*)item->itemValue;
Statement 	*line = new Statement();
	line->add(block);
	// Block
	statement->itemValue = (void*)line;
	return 1;
}

int Tawk::StatementBody3TawkNow(PLGitem *iTEM)
{
PLGitem 	*statement = (PLGitem*)iTEM->children->get("statement");
PLGitem 	*action = statement->getLabel("action");
PLGitem 	*instance = statement->getLabel("instance");
PLGitem 	*item = action->getLabel("statement");
PLGitem 	*otherwise = statement->getLabel("otherwise");
Instance 	*express = (Instance*)instance->itemValue;
Statement 	*ifStatement = new Statement(IF);
Statement 	*line = (Statement*)item->itemValue;
	// if
	ifStatement->add(express);
	express->isCondition = 1;
	line->indented = 1;
	if ( line )
		ifStatement->add(line);
	if ( otherwise )
		{
		item = otherwise->getLabel("statement");
		line = (Statement*)item->itemValue;
		if ( line )
			ifStatement->add(line);
		}
	ifStatement->pointInCode = iTEM;
	statement->itemValue = (void*)ifStatement;
	return 1;
}

int Tawk::StatementBody4TawkNow(PLGitem *iTEM)
{
PLGitem 	*statement = (PLGitem*)iTEM->children->get("statement");
PLGitem 	*instance = (PLGitem*)iTEM->children->get("instance");
Instance 	*returnValue = 0;
Statement 	*line = new Statement(RETURN);
	// return
	if ( instance )
		{
		returnValue = (Instance*)instance->itemValue;
		line->add(returnValue);
		}
	line->branch = 1;
	statement->itemValue = (void*)line;
	return 1;
}

int Tawk::StatementBody5TawkNow(PLGitem *iTEM)
{
PLGitem 	*instance = (PLGitem*)iTEM->children->get("instance");
PLGitem 	*statement = (PLGitem*)iTEM->children->get("statement");
Statement 	*forStatement = (Statement*)instance->itemValue;
Statement 	*body = 0;
	body = (Statement*)statement->itemValue;
	body->indented = 1;
	forStatement->fourth = new Instance(body);
	// for
	forStatement->pointInCode = iTEM;
	statement->itemValue = (void*)forStatement;
	iterating--;
	return 1;
}

int Tawk::StatementBody6TawkNow(PLGitem *iTEM)
{
PLGitem 	*statement = (PLGitem*)iTEM->children->get("statement");
PLGitem 	*item = statement->getLabel("start");
Statement 	*line = new Statement();
Instance 	*instance = (Instance*)item->itemValue;
	line->add(instance);
	// Print
	if ( instance->block )
		line->indented = 0;
	statement->itemValue = (void*)line;
	return 1;
}

int Tawk::StatementBody7TawkNow(PLGitem *iTEM)
{
PLGitem 	*instance = (PLGitem*)iTEM->children->get("instance");
PLGitem 	*statement = (PLGitem*)iTEM->children->get("statement");
Statement 	*wile = new Statement(WHILE);
Instance 	*test = (Instance*)instance->itemValue;
Statement 	*body = (Statement*)statement->itemValue;
	// while
	wile->add(test);
	test->isCondition = 1;
	body->indented = 1;
	wile->add(body);
	wile->pointInCode = iTEM;
	statement->itemValue = (void*)wile;
	iterating--;
	return 1;
}

int Tawk::StatementBody8TawkNow(PLGitem *iTEM)
{
PLGitem 	*statement = (PLGitem*)iTEM->children->get("statement");
PLGitem 	*item = statement->getLabel("instance");
Instance 	*instance = (Instance*)item->itemValue;
Statement 	*line = new Statement(LABEL);
	line->add(instance);
	// label or case
	if ( !instance->prefix )
		line->indented = 0;
	statement->itemValue = (void*)line;
	return 1;
}

int Tawk::StatementBody9TawkNow(PLGitem *iTEM)
{
PLGitem 	*statement = (PLGitem*)iTEM->children->get("statement");
Instance 	*instance = getInstance("break");
Statement 	*line = new Statement();
PLGitem 	*lastSwitch = 0;
	line->add(instance);
	// break
	line->branch = 1;
	statement->itemValue = (void*)line;
	// if break is in a switch case, set switch fall thru status
	if ( switchStack )
		lastSwitch = (PLGitem*)switchStack->top();
	if ( lastSwitch && iterating == lastSwitch->itemLength )
		lastSwitch->flag1 = 1;
	return 1;
}

int Tawk::StatementBodyTawkNow(PLGitem *iTEM)
{
PLGitem 	*statement = (PLGitem*)iTEM->children->get("statement");
Statement 	*line = new Statement();
PLGitem 	*comment = statement->getLabel("comment");
Instance 	*instance = 0;
	if ( comment )
		instance = getInstance(comment->toString());
	else	instance = getInstance(statement->toString());
	instance->isComment = 1;
	line->add(instance);
	statement->itemValue = (void*)line;
	return 1;
}

void Tawk::StatementTawkAct(PLGitem *iTEM)
{
}

int Tawk::StringExpression2TawkNow(PLGitem *iTEM)
{
PLGitem 	*instance = (PLGitem*)iTEM->children->get("instance");
Instance 	*item = (Instance*)instance->itemValue;
Instance 	*target = 0;
SymbolType 	*type = item->getType();
	target = item->getSubject();
	if ( type != SymbolType::stringType )
		{
		target = convertToString(item);
		if ( !target )
			return 0;
		instance->itemValue = (void*)target;
		}
	else
	if ( !(target->isConstant || item->howDirect() == 1) )
		return 0;
	else
	if ( item->express && item->express->verb && !(item->express->verb->pointing || item->express->verb->assign) )
		return 0;
	return 1;
}

void Tawk::StringExpressionTawkAct(PLGitem *iTEM)
{
}

int Tawk::StringsTawkNow(PLGitem *iTEM)
{
PLGitem 	*item = (PLGitem*)iTEM->children->get("item");
PLGitem 	*instance = 0;
	for ( ; item; item = item->itemNext )
		{
		instance = item->getLabel("instance");
		item->itemValue = instance->itemValue;
		}
	return 1;
}

void Tawk::StructureBody2TawkAct(PLGitem *iTEM)
{
PLGitem 	*entry = (PLGitem*)iTEM->children->get("entry");
Symbol 		*symbol = 0;
PLGitem 	*name = entry->deferred->find("name");
PLGitem 	*item = 0;
	saveStruct = currentClass;
	if ( currentClass )
		{
		symbol = currentClass->getLocal(name->string());
		name->unString();
		}
	if ( symbol )
		setCurrentClass(symbol->structType);
	else {
		char 	*typeName = ::concat(4,currentClass->name,"Struct",::toStringFromInt(stringNumber++),"Type");
		setCurrentClass(SymbolType::getType(typeName));
		}
	if ( currentClass )
		currentClass->nameLess = 1;
	for ( item = entry; item; item = item->itemNext )
		{
		item->runDeferred(this);
		name = item->getLabel("name");
		if ( name )
			item->itemValue = name->itemValue;
		}
	setCurrentType(currentClass);
	setCurrentClass(saveStruct);
	saveStruct = 0;
}

void Tawk::StructureBodyTawkAct(PLGitem *iTEM)
{
PLGitem 	*label = (PLGitem*)iTEM->children->get("label");
PLGitem 	*entry = (PLGitem*)iTEM->children->get("entry");
PLGitem 	*field = (PLGitem*)iTEM->children->get("field");
Instance 	*current = 0;
SymbolType 	*saveClass = currentClass;
PLGitem 	*item = 0;
PLGitem 	*name = 0;
	item = label->getLabel("type");
	if ( !item->itemValue )
		{
		label->runDeferred(this);
		item = label->getLabel("type");
		}
	setCurrentClass((SymbolType*)item->itemValue);
	for ( item = entry; item; item = item->itemNext )
		{
		item->runDeferred(this);
		name = item->getLabel("name");
		if ( name )
			item->itemValue = name->itemValue;
		}
	setCurrentType(currentClass);
	currentType->isDirect = 1;
	for ( ; field; field = field->itemNext )
		{
		field->runDeferred(this);
		item = field->getLabel("item");
		current = (Instance*)item->itemValue;
		field->itemValue = (void*)current;
		}
	current = new Instance(currentClass);
	setCurrentClass(saveClass);
	label->itemValue = (void*)current;
}

void Tawk::StructureItem2TawkAct(PLGitem *iTEM)
{
PLGitem 	*name = (PLGitem*)iTEM->children->get("name");
PLGitem 	*bits = (PLGitem*)iTEM->children->get("bits");
PLGitem 	*buttons = (PLGitem*)iTEM->children->get("buttons");
char 		*text = name->toString();
Symbol 		*aliasSymbol = 0;
Symbol 		*symbol = 0;
Instance 	*instance = 0;
SymbolType 	*symbolType = 0;
	symbolType = SymbolType::getType("unsigned int");
	symbol = currentClass->getLocal(text);
	if ( !symbol )
		{
		symbol = new Symbol(text,symbolType);
		symbol->structType = currentClass;
		if ( bits )
			{
			PLGitem 	*length = bits->getLabel("length");
			symbol->array = bits->toString();
			symbol->symbolBitLength = ::atoi(length->string());
			length->unString();
			}
		else	symbol->symbolBitLength = 1;
		symbol->isItem = 1;
		currentClass->add(symbol);
		// saveStruct, if set, is the class containing the structure
		if ( saveStruct )
			{
			aliasSymbol = new Symbol(symbol);
			aliasSymbol->isHidden = 1;
			aliasSymbol->isItem = 1;
			saveStruct->add(aliasSymbol);
			}
		}
	else {
		if ( bits )
			symbol->array = bits->toString();
		symbol->isItem = 1;
		}
	instance = new Instance(symbol);
	name->itemValue = (void*)instance;
	if ( buttons )
		{
		PLGitem 	*button = 0;
		buttons->runDeferred(this);
		button = buttons->getLabel("button");
		for ( ; button; button = button->itemNext )
			{
			// for button items symbol.source is set to the button container symbol
			symbol = (Symbol*)button->itemValue;
			symbol->isButton = 1;
			symbol->source = instance->symbol;
			if ( saveStruct )
				{
				aliasSymbol = new Symbol(symbol);
				saveStruct->add(aliasSymbol);
				}
			}
		}
}

void Tawk::StructureItemTawkAct(PLGitem *iTEM)
{
PLGitem 	*item = (PLGitem*)iTEM->children->get("item");
PLGitem 	*declare = item->getLabel("declare");
PLGitem 	*entry = 0;
Instance 	*instance = 0;
Symbol 		*aliasSymbol = 0;
	item->runDeferred(this);
	for ( entry = declare; entry; entry = entry->itemNext )
		{
		instance = (Instance*)entry->itemValue;
		if ( instance )
			{
			currentClass->add(instance->symbol);
			instance->symbol->structType = currentClass;
			if ( saveStruct )
				{
				aliasSymbol = new Symbol(instance->symbol);
				aliasSymbol->isHidden = 1;
				aliasSymbol->isItem = 1;
				saveStruct->add(aliasSymbol);
				}
			}
		}
}

void Tawk::StructureTawkAct(PLGitem *iTEM)
{
PLGitem 	*kind = (PLGitem*)iTEM->children->get("kind");
PLGitem 	*body = (PLGitem*)iTEM->children->get("body");
Symbol 		*symbol = 0;
Instance 	*instance = 0;
Instance 	*current = 0;
SymbolType 	*structureType = 0;
PLGitem 	*item = body->getLabel("label");
PLGitem 	*field = body->getLabel("field");
PLGitem 	*entry = body->getLabel("entry");
char 		*error = 0;
	setCurrentType((SymbolType*)0);
	newType = methodType = 0;
	if ( !item && kind->toString() == "typedef" )
		{
		PLGitem 	*name = entry->getLabel("name");
		structureType = SymbolType::find(name->string());
		if ( structureType && !isType(structureType->structure) )
			{
			error = ::concat(3,"typedef ",name->string()," conflicts with existing class name\n");
			instance = makeError(error);
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
		if ( currentClass->noDotH )
			structureType->noDotH = 1;
		else	structureType->dotHname = currentClass->dotHname;
		goto finish;
		}
	processingParameters = 1;
	body->runDeferred(this);
	processingParameters = 0;
	structureType = currentType;
	structureType->noDotH = 1;
	if ( !currentClass )
		setCurrentClass(currentType);
	if ( !item )
		{
		char 	*name = ::headToString(structureType->name,"Type");
		symbol = currentClass->getLocal(name);
		if ( !symbol )
			{
			symbol = new Symbol(name,structureType);
			currentClass->add(symbol);
			symbol->isHidden = 1;
			//	Note here, isHidden has to be set after adding
			}
		instance = new Instance(symbol);
		}
	else {
		instance = (Instance*)item->itemValue;
		structureType = instance->type;
		//dealWith next
		if ( field )
			body->itemNext = field;
		else
		if ( !currentClass->isExternal )
			structureType->mustDeclare = 1;
		}
	if ( (kind->toString() == "boolean") )
		{
		structureType->structure = 1;
		structureType->isNumber = 1;
		}
	else
	if ( (kind->toString() == "enumerator") )
		{
		structureType->structure = 2;
		structureType->isNumber = 1;
		SymbolType::globalList->add((void*)structureType);
		}
	else
	if ( (kind->toString() == "struct") )
		structureType->structure = 5;
	else
	if ( (kind->toString() == "typedef") )
		structureType->structure = 4;
	else	structureType->structure = 6;
	if ( isBoolean(structureType->structure) || isEnumerator(structureType->structure) )
		for ( item = entry; item; item = item->itemNext )
			{
			current = (Instance*)item->itemValue;
			symbol = current->symbol;
			if ( isEnumerator(structureType->structure) )
				{
				symbol->type = SymbolType::nullType;
				currentSymbols->addGlobalField(symbol->name,symbol);
				}
			else
			if ( isBoolean(structureType->structure) )
				if ( !symbol->array )
					symbol->array = ":1";
			}
finish:
	structureType->isExternal = 0;
	structureType->isDirect = 1;
	if ( currentClass != SymbolType::globalType )
		structureType->setParent(currentClass);
	kind->itemValue = (void*)structureType;
	body->itemValue = (void*)instance;
	setCurrentType((SymbolType*)0);
}

void Tawk::StructureType2TawkAct(PLGitem *iTEM)
{
PLGitem 	*type = (PLGitem*)iTEM->children->get("type");
SymbolType 	*symbolType = 0;
char 		*name = type->string();
	symbolType = SymbolType::find(name);
	type->unString();
	if ( !symbolType )
		symbolType = SymbolType::getType(type->toString());
	type->itemValue = (void*)symbolType;
}

void Tawk::StructureTypeTawkAct(PLGitem *iTEM)
{
}

int Tawk::SwitchTawkNow(PLGitem *iTEM)
{
PLGitem 	*statement = (PLGitem*)iTEM->children->get("statement");
PLGitem 	*name = (PLGitem*)iTEM->children->get("name");
	// overriding statement length (not otherwise used).
	statement->itemLength = iterating;
	if ( name )
		statement->itemValue = name->itemValue;
	if ( !switchStack )
		switchStack = new Stak();
	switchStack->push(statement);
	return 1;
}

int Tawk::Target2TawkNow(PLGitem *iTEM)
{
PLGitem 	*field = (PLGitem*)iTEM->children->get("field");
PLGitem 	*name = field->getLabel("name");
Instance 	*instance = getInstance(name->toString());
	instance->type = 0;
	field->itemValue = (void*)instance;
	return 1;
}

int Tawk::TargetMethodTawkNow(PLGitem *iTEM)
{
PLGitem 	*target = (PLGitem*)iTEM->children->get("target");
Symbol 		*symbol = 0;
PLGitem 	*name = target->getLabel("name");
	if ( currentClass->methods )
		{
		symbol = (Symbol*)currentClass->methods->get(name->string());
		name->unString();
		}
	if ( symbol )
		target->itemValue = (void*)symbol;
	else	return 0;
	return 1;
}

int Tawk::TargetTawkNow(PLGitem *iTEM)
{
PLGitem 	*field = (PLGitem*)iTEM->children->get("field");
Instance 	*instance = (Instance*)field->itemValue;
	if ( instance->isError )
		return 0;
	if ( instance->isConstant )
		instance->type = 0;
	return 1;
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
	compareSet = 0;
	compareFollow = 0;
	logicSet = 0;
	methodSet = 0;
	methodNameSet = 0;
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
	// WTF?
	SymbolType::types = new Types();
	blockStack = new Stak();
	virtualStack = new Stak();
	tokJunkBuffer = new Buffer("tokJunk",1000);
	currentSymbols = new InstanceTable();
	formatter = new FormatC();
	lambdaSet = new PLGset();
	lambdaSet->set((int)'^');
	includedFiles = new BaseHash();
	missingMethods = new BaseHash();
}

int Tawk::TypeListTawkNow(PLGitem *iTEM)
{
PLGitem 	*instance = (PLGitem*)iTEM->children->get("instance");
Instance 	*item = new Instance(currentType);
	instance->itemValue = (void*)item;
	setCurrentType((SymbolType*)0);
	return 1;
}

int Tawk::TypeNameTawkNow(PLGitem *iTEM)
{
PLGitem 	*type = (PLGitem*)iTEM->children->get("type");
SymbolType 	*symbolType = 0;
	symbolType = SymbolType::find(type->string());
	type->unString();
	if ( symbolType )
		{
		type->itemValue = (void*)symbolType;
		return 1;
		}
	return 0;
	return 1;
}

int Tawk::TypeTawkNow(PLGitem *iTEM)
{
PLGitem 	*hasConst = (PLGitem*)iTEM->children->get("hasConst");
PLGitem 	*noSign = (PLGitem*)iTEM->children->get("noSign");
PLGitem 	*type = (PLGitem*)iTEM->children->get("type");
PLGitem 	*temp = (PLGitem*)iTEM->children->get("temp");
SymbolType 	*symbolType = 0;
char 		*name = 0;
	if ( !noSign && !temp )
		symbolType = (SymbolType*)type->itemValue;
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
	type->itemValue = (void*)symbolType;
	setCurrentType(symbolType);
	return 1;
}

int Tawk::UnaryExpression2TawkNow(PLGitem *iTEM)
{
PLGitem 	*operate = (PLGitem*)iTEM->children->get("operate");
PLGitem 	*cast = (PLGitem*)iTEM->children->get("cast");
PLGitem 	*instance = (PLGitem*)iTEM->children->get("instance");
Expression 	*expression = 0;
PLGitem 	*item = 0;
Instance 	*castInstance = 0;
Instance 	*subject = 0;
SymbolType 	*virtualType = 0;
	subject = (Instance*)instance->itemValue;
	//dealWith next not sure what !operate.next is guarding against
	if ( operate )
		if ( operate->toString() == "&" && !operate->itemNext )
			subject->setReference((unsigned int)1);
		else {
			expression = new Expression((Instance*)0,subject,operate->toString());
			subject = new Instance(expression);
			}
	if ( cast )
		{
		item = cast->getLabel("type");
		castInstance = (Instance*)item->itemValue;
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
		virtualItem = subject;
	instance->itemValue = (void*)subject;
	setCurrentType(virtualType);
	return 1;
}

int Tawk::UnaryExpressionTawkNow(PLGitem *iTEM)
{
PLGitem 	*operate = (PLGitem*)iTEM->children->get("operate");
PLGitem 	*instance = (PLGitem*)iTEM->children->get("instance");
Instance 	*subject = 0;
Expression 	*expression = 0;
PLGitem 	*list = instance->getLabel("list");
	instance->runDeferred(this);
	subject = (Instance*)list->itemValue;
	if ( operate )
		{
		expression = new Expression((Instance*)0,subject,operate->toString());
		subject = new Instance(expression);
		}
	instance->itemValue = (void*)subject;
	setCurrentType((SymbolType*)0);
	return 1;
}

int Tawk::UnaryOperator2TawkNow(PLGitem *iTEM)
{
PLGitem 	*operate = (PLGitem*)iTEM->children->get("operate");
	//dealWith thinking we can drop this action; not sure next is ever set
	if ( operate->itemNext )
		return 0;
	return 1;
}

void Tawk::UnaryOperatorTawkAct(PLGitem *iTEM)
{
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
	//dealWith next
	if ( !source->itemNext )
		return (Instance*)source->itemValue;
	/*************************************************************************
	If all items to be concatenated are literals just glom them
	together and go home
	*************************************************************************/
	for ( ; item; item = item->itemNext )
		{
		parameter = (Instance*)item->itemValue;
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
		for ( item = source; item; item = item->itemNext )
			{
			parameter = (Instance*)item->itemValue;
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
	for ( item = source; item; item = item->itemNext )
		count++;
	::asprintf(&text,"%d",count);
	parameter = getInstance(text);
	parameter->type = SymbolType::intType;
	instance->addParameter(parameter);
	for ( item = source; item; item = item->itemNext )
		{
		parameter = (Instance*)item->itemValue;
		instance->addParameter(parameter);
		}
	//dealWith next
	source->itemNext = 0;
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
	for ( ; item; item = item->itemNext )
		{
		tokJunkBuffer->appendString(item->string(),0,0);
		tokJunkBuffer->appendString("\n",0,0);
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
	operate = item->getLabel("operate");
	expressItem = item->getLabel("instance");
	//dealWith next
	if ( item->itemNext )
		{
		primary = (Instance*)expressItem->itemValue;
		expression = makeExpress(primary,item->itemNext);
		secondary = new Instance(expression);
		}
	else {
		secondary = (Instance*)expressItem->itemValue;
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
	if ( instance->resolved && instance->parameters && operate->toString() == "=" && (type = instance->getType()) && type->isVirtuous )
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
		field = target->getLabel("instance");
		if ( !field )
			defaultPrinter = currentSymbols->find("printf");
		else {
			printObject = (Instance*)field->itemValue;
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
	alphaSet = getSet("alphaSet","ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz");
	nameStartSet = getSet("nameStartSet","@ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz");
	commentSet = getSet("commentSet","#-/");
	compareSet = getSet("compareSet","!%&*+-/:<=>?^egln|");
	compareFollow = getSet("compareFollow","\t\n\r !\"&'(*+-");
	logicSet = getSet("logicSet","egln");
	methodSet = getSet("methodSet","&()*,0123456789@ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstu01234");
	methodNameSet = getSet("methodNameSet","0123456789:ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz");
	nameSet = getSet("nameSet","0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz");
	operatorSet = getSet("operatorSet","!%&*+-/:<=>?^ei|~");
	space = getSet("space","\t\n");
	rangeSet = getSet("rangeSet",".<>");
	singleQuote = getSet("singleQuote","'");
	stringOP = getSet("stringOP","+-=");
	textFollow = getSet("textFollow","0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz");
	typesSet = getSet("typesSet","");
	currentRule = getRule("Block");
	currentAlt = new Alternative();
	addTest(6,"BlockStart","start",1,1,"");
	addTest(6,"Line","line",0,999999,"");
	addTest(1,"}","",1,1,"");
	currentAlt->immediate = BlockTawkNow;
	currentRule->alternatives->add(currentAlt);
	currentRule = getRule("StatementBody16");
	currentRule = getRule("BlockStart");
	currentAlt = new Alternative();
	addTest(1,"{","brace",1,1,"");
	currentAlt->immediate = BlockStartTawkNow;
	currentRule->alternatives->add(currentAlt);
	currentRule = getRule("ClassBlockStart");
	currentAlt = new Alternative();
	addTest(1,"{","",1,1,"");
	currentRule->alternatives->add(currentAlt);
	currentRule = getRule("OperationTail");
	currentAlt = new Alternative();
	addTest(6,"Operator","operate",1,1,"");
	addTest(6,"UnaryExpression","instance",1,1,"");
	addTest(6,"Question","question",0,1,"");
	currentAlt->immediate = OperationTailTawkNow;
	currentRule->alternatives->add(currentAlt);
	currentAlt = new Alternative();
	addTest(6,"Question","instance",1,1,"");
	currentAlt->immediate = OperationTail2TawkNow;
	currentRule->alternatives->add(currentAlt);
	currentAlt = new Alternative();
	addTest(1,"in","in",1,1,"");
	addTest(3,"textFollow","",-1,1,"");
	setNoSkip();
	setBanged();
	addTest(6,"RangeField","range",1,1,"");
	currentRule->alternatives->add(currentAlt);
	currentRule = getRule("MacroBlock");
	currentAlt = new Alternative();
	addTest(6,"Line","line",0,999999,"");
	currentAlt->immediate = MacroBlockTawkNow;
	currentRule->alternatives->add(currentAlt);
	currentRule = getRule("OverLoadItem4");
	currentRule = getRule("Body");
	currentAlt = new Alternative();
	addTest(6,"Commands","",1,1,"");
	currentRule->alternatives->add(currentAlt);
	currentAlt = new Alternative();
	addTest(6,"Method","body",1,1,"");
	currentAlt->defer = BodyTawkAct;
	currentRule->alternatives->add(currentAlt);
	currentAlt = new Alternative();
	addTest(6,"Declaration","body",1,1,"");
	addTest(1,";","",1,1,"");
	currentAlt->immediate = Body2TawkNow;
	currentRule->alternatives->add(currentAlt);
	currentAlt = new Alternative();
	addTest(6,"Comment","",1,1,"");
	currentRule->alternatives->add(currentAlt);
	currentAlt = new Alternative();
	addTest(6,"Include3","",1,1,"");
	currentRule->alternatives->add(currentAlt);
	currentRule = getRule("MethodName");
	currentAlt = new Alternative();
	addTest(6,"MethodNameSet","name",1,1,"");
	currentAlt->immediate = MethodNameTawkNow;
	currentRule->alternatives->add(currentAlt);
	currentRule = getRule("Body2");
	currentRule = getRule("Statement2");
	currentRule = getRule("Body3");
	currentRule = getRule("SecondaryExpression2");
	currentRule = getRule("StringExpression2");
	currentRule = getRule("UnaryExpression2");
	currentRule = getRule("ClassBlock");
	currentAlt = new Alternative();
	addTest(6,"ClassBlockStart","",1,1,"");
	addTest(6,"Body","",0,999999,"");
	addTest(1,"}","",1,1,"");
	currentRule->alternatives->add(currentAlt);
	currentRule = getRule("ClassHeading");
	currentAlt = new Alternative();
	addTest(1,"class","",1,1,"");
	addTest(6,"ClassName","nom",1,1,"");
	addTest(6,"ClassAttributes","attributes",0,999999,"");
	currentAlt->immediate = ClassHeadingTawkNow;
	currentRule->alternatives->add(currentAlt);
	currentAlt = new Alternative();
	addTest(1,"external","externalRef",0,1,"");
	addTest(6,"Structure","structure",1,1,"");
	addTest(1,";","",0,1,"");
	currentAlt->immediate = ClassHeading2TawkNow;
	currentRule->alternatives->add(currentAlt);
	currentAlt = new Alternative();
	addTest(1,"external","",1,1,"");
	addTest(6,"ClassHeadingBlock0","kind",0,1,"");
	addTest(6,"ClassName","nom",0,999999,"");
	addTest(6,"ClassAttributes","attributes",0,999999,"");
	addTest(1,";","",0,1,"");
	currentAlt->immediate = ClassHeading3TawkNow;
	currentRule->alternatives->add(currentAlt);
	currentRule = getRule("StatementBody14");
	currentRule = getRule("ClassHeadingBlock0");
	currentAlt = new Alternative();
{
Element 	*elem = new Element();
	elem->minimum = 1;
	elem->maximum = 1;
	elem->kind = 8;
	elem->label = "kind";
	currentAlt->elements->add((void*)elem);
}
addTest(3,"textFollow","",-1,1,"");
setNoSkip();
setBanged();
currentRule->alternatives->add(currentAlt);
currentRule = getRule("RangeTail");
currentAlt = new Alternative();
{
Element 	*elem = new Element();
	elem->minimum = 1;
	elem->maximum = 1;
	elem->kind = 8;
	elem->label = "operate";
	currentAlt->elements->add((void*)elem);
}
addTest(6,"Expression","instance",1,1,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("ExpressItem");
currentAlt = new Alternative();
addTest(6,"Expression","instance",1,1,"");
addTest(1,",","",0,1,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("ClassHeading2");
currentRule = getRule("Inheritance");
currentAlt = new Alternative();
addTest(6,"Extends","",1,1,"");
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(6,"Imports","",1,1,"");
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(6,"Body","method",1,999999,"");
currentAlt->defer = InheritanceTawkAct;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(6,"EndComment","error",1,1,"");
currentAlt->immediate = Inheritance2TawkNow;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("Constant");
currentAlt = new Alternative();
addTest(6,"Number","instance",1,1,"");
currentAlt->defer = ConstantTawkAct;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(6,"Character","instance",1,1,"");
currentAlt->immediate = Constant2TawkNow;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(6,"Quote","instance",1,1,"");
currentAlt->immediate = Constant3TawkNow;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(1,"true","instance",1,1,"");
addTest(3,"textFollow","",-1,1,"");
setNoSkip();
setBanged();
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(1,"false","instance",1,1,"");
addTest(3,"textFollow","",-1,1,"");
setNoSkip();
setBanged();
currentRule->alternatives->add(currentAlt);
currentRule = getRule("ClassHeading3");
currentRule = getRule("ItemArray");
currentAlt = new Alternative();
addTest(1,"[","",1,1,"");
addTest(3,"0-9","",0,999999,"");
addTest(1,"]","",1,1,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("Else");
currentAlt = new Alternative();
addTest(1,"else","",1,1,"");
addTest(3,"textFollow","",-1,1,"");
setNoSkip();
setBanged();
addTest(6,"Statement","statement",1,1,"");
addTest(6,"ResetType","",0,1,"");
currentAlt->defer = ElseTawkAct;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(1,"or","",1,1,"");
addTest(3,"textFollow","",-1,1,"");
setNoSkip();
setBanged();
addTest(6,"IfBody","statement",1,1,"");
currentAlt->immediate = Else2TawkNow;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("Else2");
currentRule = getRule("StatementBody2");
currentRule = getRule("Extends");
currentAlt = new Alternative();
addTest(6,"ClassHeading","",1,1,"");
addTest(6,"ClassBlock","",0,1,"");
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(6,"Comment","",1,1,"");
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(6,"DeclareConditions","",1,1,"");
addTest(1,";","",1,1,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("PrintCommand");
currentAlt = new Alternative();
addTest(1,"print","printer",1,1,"");
addTest(3,"textFollow","",-1,1,"");
setNoSkip();
setBanged();
addTest(6,"PrintTarget","target",0,1,"");
setNoSkip();
currentAlt->immediate = PrintCommandTawkNow;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(1,"cout","stdPrint",1,1,"");
addTest(3,"textFollow","",-1,1,"");
setNoSkip();
setBanged();
currentAlt->immediate = PrintCommand2TawkNow;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(1,"cerr","stdPrint",1,1,"");
addTest(3,"textFollow","",-1,1,"");
setNoSkip();
setBanged();
currentAlt->immediate = PrintCommand3TawkNow;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("PoundCommand");
currentAlt = new Alternative();
{
Element 	*elem = new Element();
	elem->minimum = 1;
	elem->maximum = 1;
	elem->kind = 8;
	elem->label = "state";
	currentAlt->elements->add((void*)elem);
}
addTest(6,"Type","type",0,1,"");
addTest(6,"Count","level",0,1,"");
addTest(6,"RuleList","list",0,1,"");
addTest(6,"FieldList","field",0,1,"");
currentAlt->immediate = PoundCommandTawkNow;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(6,"MacroDefine","",1,1,"");
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(6,"Directive","",1,1,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("FieldBody");
currentAlt = new Alternative();
addTest(6,"Bump","prefix",0,1,"");
addTest(6,"FieldComponent","part",1,1,"");
currentAlt->defer = FieldBodyTawkAct;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(6,"New","name",1,1,"");
currentAlt->defer = FieldBody2TawkAct;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(1,"(","",1,1,"");
addTest(6,"Expression","name",1,1,"");
addTest(1,")","",1,1,"");
currentAlt->immediate = FieldBody3TawkNow;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("FieldBody2");
currentRule = getRule("FieldBody3");
currentRule = getRule("Qualified");
currentAlt = new Alternative();
addTest(6,"QualifyType","type",0,1,"");
addTest(6,"QualifyStart","field",1,1,"");
addTest(6,"QualifyTail","rest",0,999999,"");
addTest(6,"Bump","postfix",0,1,"");
currentAlt->immediate = QualifiedTawkNow;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("FieldExpression");
currentAlt = new Alternative();
addTest(6,"CastExpression","cast",0,1,"");
addTest(6,"Indirection","direct",0,1,"");
addTest(6,"Qualified","instance",1,1,"");
currentAlt->immediate = FieldExpressionTawkNow;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("Expression");
currentAlt = new Alternative();
addTest(6,"ExpressPart","instance",1,1,"");
addTest(6,"ExpressTail","express",0,999999,"");
currentAlt->immediate = ExpressionTawkNow;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("CastExpression");
currentAlt = new Alternative();
addTest(6,"Indirection","direct",0,1,"");
addTest(1,"(","",1,1,"");
addTest(6,"CastType","type",1,1,"");
addTest(6,"CastTail","rest",0,1,"");
addTest(1,")","",1,1,"");
currentAlt->immediate = CastExpressionTawkNow;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("SecondaryExpression");
currentAlt = new Alternative();
addTest(1,"null","instance",1,1,"");
addTest(3,"textFollow","",-1,1,"");
setNoSkip();
setBanged();
currentAlt->immediate = SecondaryExpressionTawkNow;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(1,"sizeof","instance",1,1,"");
addTest(1,"(","",1,1,"");
addTest(6,"Type","",1,1,"");
addTest(1,"*","pointer",0,999999,"");
addTest(1,")","",1,1,"");
currentAlt->immediate = SecondaryExpression2TawkNow;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("InitExpression");
currentAlt = new Alternative();
addTest(6,"Expression","instance",1,1,"");
currentAlt->immediate = InitExpressionTawkNow;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(6,"RangeField","instance",1,1,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("RangeExpression");
currentAlt = new Alternative();
addTest(6,"UnaryExpression","instance",1,1,"");
addTest(6,"RangeTail","back",1,1,"");
currentAlt->immediate = RangeExpressionTawkNow;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("PrimaryExpression");
currentAlt = new Alternative();
addTest(6,"Constant","instance",1,1,"");
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(6,"SecondaryExpression","instance",1,1,"");
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(6,"FieldExpression","instance",1,1,"");
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(6,"AssumedString","instance",1,1,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("StatementBody8");
currentRule = getRule("StringExpression");
currentAlt = new Alternative();
addTest(6,"AllowShortcuts","",1,1,"");
addTest(6,"PrintShortcut","instance",1,1,"");
currentAlt->defer = StringExpressionTawkAct;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(6,"Expression","instance",1,1,"");
currentAlt->immediate = StringExpression2TawkNow;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("UnaryExpression");
currentAlt = new Alternative();
addTest(6,"UnaryOperator","operate",0,1,"");
addTest(6,"ConditionWord","instance",1,1,"");
currentAlt->immediate = UnaryExpressionTawkNow;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(6,"UnaryOperator","operate",0,1,"");
addTest(6,"CastExpression","cast",0,1,"");
addTest(6,"PrimaryExpression","instance",1,1,"");
currentAlt->immediate = UnaryExpression2TawkNow;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(6,"AllowShortcuts","",1,1,"");
addTest(6,"PrintShortcut","instance",1,1,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("Fielding");
currentAlt = new Alternative();
addTest(6,"Name","name",1,1,"");
currentAlt->immediate = FieldingTawkNow;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("IfBody");
currentAlt = new Alternative();
addTest(6,"Expression","instance",1,1,"");
addTest(6,"Comment","",0,1,"");
addTest(6,"Statement","action",1,1,"");
addTest(6,"ResetType","",0,1,"");
addTest(6,"Else","otherwise",0,1,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("ForOption");
currentAlt = new Alternative();
addTest(1,"(","instance",1,1,"");
addTest(6,"ExpressList","initial",0,1,"");
addTest(1,";","",1,1,"");
addTest(6,"Expression","condition",0,1,"");
addTest(1,";","",1,1,"");
addTest(6,"ExpressList","increment",0,1,"");
addTest(1,")","",1,1,"");
currentAlt->immediate = ForOptionTawkNow;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(6,"Expression","instance",1,1,"");
addTest(6,"ForOptionBlock0","name",0,1,"");
currentAlt->immediate = ForOption2TawkNow;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("ForOption2");
currentRule = getRule("Include");
currentAlt = new Alternative();
addTest(1,"#include","include",1,1,"");
addTest(1,"\n","",1,1,"");
currentAlt->immediate = IncludeTawkNow;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(1,"#import","include",1,1,"");
addTest(1,"\n","",1,1,"");
currentAlt->immediate = Include2TawkNow;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(6,"IncludeBlock0","",1,1,"");
addTest(3," \t","",1,999999,"");
setNoSkip();
addTest(3,"^\n","include",1,999999,"");
setNoSkip();
addTest(1,"\n","",1,1,"");
setNoSkip();
currentAlt->immediate = Include3TawkNow;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("Include2");
currentRule = getRule("StatementBody3");
currentRule = getRule("Include3");
currentRule = getRule("StatementBody4");
currentRule = getRule("Inheritance2");
currentRule = getRule("PrintSet");
currentRule = getRule("InstanceTail");
currentAlt = new Alternative();
addTest(6,"NewArray","array",1,999999,"");
currentAlt->immediate = InstanceTailTawkNow;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(1,"(","",1,1,"");
addTest(6,"ResetType","",1,1,"");
addTest(6,"NoShortcuts","",1,1,"");
addTest(6,"ParameterList","expression",0,1,"");
addTest(1,")","",1,1,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("Iterating");
currentRule = getRule("OverLoadItem5");
currentRule = getRule("macroDelimiter");
currentRule = getRule("Lambda");
currentAlt = new Alternative();
addTest(6,"LambdaName","function",1,1,"");
addTest(1,"=","",1,1,"");
addTest(6,"Block","body",1,1,"");
currentAlt->immediate = LambdaTawkNow;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(6,"MethodType","function",1,1,"");
addTest(6,"Block","body",1,1,"");
currentAlt->immediate = Lambda2TawkNow;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("Character");
currentAlt = new Alternative();
addTest(6,"CharacterBlock0","instance",1,1,"");
currentAlt->immediate = CharacterTawkNow;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("Lambda2");
currentRule = getRule("Line");
currentAlt = new Alternative();
addTest(1,"use","",1,1,"");
addTest(3,"textFollow","",-1,1,"");
setNoSkip();
setBanged();
addTest(6,"FieldExpression","target",1,1,"");
currentAlt->immediate = LineTawkNow;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(1,"}","",-1,1,"");
setNoSkip();
setBanged();
addTest(6,"Statement","statement",1,1,"");
addTest(6,"ResetType","",0,1,"");
currentAlt->immediate = Line2TawkNow;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(1,"label","",1,1,"");
addTest(6,"Name","name",1,999999,"");
addTest(1,";","",1,1,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("Line2");
currentRule = getRule("MacroArgumentList");
currentAlt = new Alternative();
addTest(6,"MacroArgument","argument",1,999999,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("LineByLine");
currentAlt = new Alternative();
addTest(6,"Statement","line",1,999999,"");
currentAlt->immediate = LineByLineTawkNow;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("MacroName");
currentAlt = new Alternative();
addTest(6,"NameSet","name",1,1,"");
currentAlt->immediate = MacroNameTawkNow;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("Method");
currentAlt = new Alternative();
addTest(6,"MethodType","method",1,1,"");
addTest(6,"Block","block",1,1,"");
currentAlt->immediate = MethodTawkNow;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("TargetMethod");
currentAlt = new Alternative();
addTest(6,"Name","target",1,1,"");
currentAlt->immediate = TargetMethodTawkNow;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("MethodHead");
currentAlt = new Alternative();
addTest(6,"Indirection","direct",0,1,"");
addTest(6,"MethodName","function",0,1,"");
addTest(6,"MethodParameters","head",1,1,"");
currentAlt->defer = MethodHeadTawkAct;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("MethodType");
currentAlt = new Alternative();
{
Element 	*elem = new Element();
	elem->minimum = 0;
	elem->maximum = 999999;
	elem->kind = 8;
	elem->label = "modify";
	currentAlt->elements->add((void*)elem);
}
addTest(6,"Type","type",1,1,"");
addTest(6,"MethodHead","methodHead",1,1,"");
addTest(1,"{","",1,1,"");
setIgnored();
currentAlt->immediate = MethodTypeTawkNow;
currentAlt->defer = MethodTypeTawkAct;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("Throw");
currentAlt = new Alternative();
addTest(1,"throw","",1,1,"");
addTest(6,"Expression","express",1,1,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("New");
currentAlt = new Alternative();
addTest(1,"new","instance",1,1,"");
addTest(3,"textFollow","",-1,1,"");
setNoSkip();
setBanged();
addTest(6,"Type","type",0,1,"");
addTest(6,"InstanceBody","body",0,1,"");
addTest(6,"ArrayInitializer","initial",0,1,"");
currentAlt->defer = NewTawkAct;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("NoShortcuts");
currentRule = getRule("AllowShortcuts");
currentRule = getRule("CaseLabel2");
currentRule = getRule("Parameter");
currentAlt = new Alternative();
addTest(6,"Type","type",1,1,"");
addTest(6,"ParameterItem","item",0,999999,"");
addTest(1,",","",0,1,"");
currentAlt->immediate = ParameterTawkNow;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("AliasParameter");
currentAlt = new Alternative();
addTest(6,"Name","parameter",1,1,"");
addTest(6,"AliasParameterBlock0","replacedBy",0,1,"");
addTest(1,",","",0,1,"");
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(6,"Replacement","parameter",1,1,"");
addTest(1,",","",0,1,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("Indirection");
currentAlt = new Alternative();
addTest(3,"*&^","direct",1,999999,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("ParameterItem");
currentAlt = new Alternative();
addTest(6,"MethodHead","name",1,1,"");
addTest(1,",","",0,1,"");
currentAlt->defer = ParameterItemTawkAct;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(6,"Indirection","direct",0,1,"");
addTest(6,"Name","name",1,1,"");
addTest(1,"[]","array",0,999999,"");
addTest(1,",","",0,1,"");
currentAlt->defer = ParameterItem2TawkAct;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(1,"[]","name",1,999999,"");
addTest(1,",","",0,1,"");
currentAlt->defer = ParameterItem3TawkAct;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(6,"Indirection","name",1,1,"");
addTest(1,",","",0,1,"");
currentAlt->defer = ParameterItem4TawkAct;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("ParameterItem2");
currentRule = getRule("ParameterItem3");
currentRule = getRule("ParameterItem4");
currentRule = getRule("AliasItem2");
currentRule = getRule("Print");
currentAlt = new Alternative();
addTest(6,"PrintCommand","start",1,1,"");
addTest(6,"PrintItem","arguments",0,999999,"");
addTest(6,"PrintTo","output",0,1,"");
currentAlt->immediate = PrintTawkNow;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("FieldComponent");
currentAlt = new Alternative();
addTest(6,"Fielding","name",1,1,"");
addTest(6,"InstanceBody","body",0,1,"");
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(6,"TypeName","name",1,1,"");
addTest(6,"InstanceBody","body",1,1,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("PrintCommand2");
currentRule = getRule("PrintCommand3");
currentRule = getRule("PrintItem");
currentAlt = new Alternative();
addTest(6,"PrintShortcut","instance",1,1,"");
currentAlt->defer = PrintItemTawkAct;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(6,"Expression","instance",1,1,"");
addTest(6,"Format","format",0,1,"");
currentAlt->immediate = PrintItem2TawkNow;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("Directivise");
currentAlt = new Alternative();
addTest(6,"Line","line",1,999999,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("Target2");
currentRule = getRule("PrintItem2");
currentRule = getRule("SaveVirtuals");
currentRule = getRule("PrintShortcut");
currentAlt = new Alternative();
addTest(3,",:`","instance",1,1,"");
currentAlt->immediate = PrintShortcutTawkNow;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("CastType");
currentAlt = new Alternative();
addTest(6,"Type","type",1,1,"");
addTest(6,"Indirection","direct",0,1,"");
addTest(6,"CastTypeBlock0","array",0,1,"");
addTest(1,",","",0,1,"");
currentAlt->immediate = CastTypeTawkNow;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("StatementBody17");
currentRule = getRule("QualifyStart");
currentAlt = new Alternative();
addTest(1,"this","name",1,1,"");
addTest(3,"textFollow","",-1,1,"");
setNoSkip();
setBanged();
currentAlt->immediate = QualifyStartTawkNow;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(6,"FieldBody","field",1,1,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("NotQuote");
currentRule = getRule("ResetType");
currentRule = getRule("Start");
currentAlt = new Alternative();
addTest(6,"Inheritance","",1,999999,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("Statement");
currentAlt = new Alternative();
addTest(6,"CheckMacro","statement",1,1,"");
currentAlt->defer = StatementTawkAct;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(6,"StatementBody","statement",1,1,"");
currentAlt->immediate = Statement2TawkNow;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("FieldList");
currentAlt = new Alternative();
addTest(1,"Field","",1,1,"");
addTest(3,"textFollow","",-1,1,"");
setNoSkip();
setBanged();
addTest(3,"A-Za-z0-9_()*","name",0,999999,"");
addTest(1,";","",1,1,"");
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(1,"Map;","",1,1,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("StatementBody");
currentAlt = new Alternative();
addTest(6,"CommentBody","statement",1,1,"");
currentAlt->immediate = StatementBodyTawkNow;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(6,"Block","statement",1,1,"");
currentAlt->immediate = StatementBody2TawkNow;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(1,"if","",1,1,"");
addTest(3,"textFollow","",-1,1,"");
setNoSkip();
setBanged();
addTest(6,"IfBody","statement",1,1,"");
currentAlt->immediate = StatementBody3TawkNow;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(1,"return","statement",1,1,"");
addTest(3,"textFollow","",-1,1,"");
setNoSkip();
setBanged();
addTest(6,"Expression","instance",0,1,"");
addTest(1,";","",1,1,"");
currentAlt->immediate = StatementBody4TawkNow;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(1,"for","",1,1,"");
addTest(3,"textFollow","",-1,1,"");
setNoSkip();
setBanged();
addTest(6,"Iterating","",0,1,"");
addTest(6,"ForOption","instance",1,1,"");
addTest(6,"Statement","statement",1,1,"");
currentAlt->immediate = StatementBody5TawkNow;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(6,"Print","statement",1,1,"");
addTest(1,";","",1,1,"");
currentAlt->immediate = StatementBody6TawkNow;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(1,"while","",1,1,"");
addTest(3,"textFollow","",-1,1,"");
setNoSkip();
setBanged();
addTest(6,"Iterating","",0,1,"");
addTest(6,"Expression","instance",1,1,"");
addTest(6,"Comment","",0,1,"");
addTest(6,"Statement","statement",1,1,"");
currentAlt->immediate = StatementBody7TawkNow;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(6,"Case","statement",1,1,"");
currentAlt->immediate = StatementBody8TawkNow;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(1,"break","statement",1,1,"");
addTest(3,"textFollow","",-1,1,"");
setNoSkip();
setBanged();
addTest(1,";","",1,1,"");
currentAlt->immediate = StatementBody9TawkNow;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(1,"continue","statement",1,1,"");
addTest(3,"textFollow","",-1,1,"");
setNoSkip();
setBanged();
addTest(1,";","",1,1,"");
currentAlt->immediate = StatementBody10TawkNow;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(1,"goto","statement",1,1,"");
addTest(3,"textFollow","",-1,1,"");
setNoSkip();
setBanged();
addTest(6,"Indirection","direct",0,1,"");
addTest(6,"Target","field",1,1,"");
addTest(1,";","",1,1,"");
currentAlt->immediate = StatementBody11TawkNow;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(6,"Switch","statement",1,1,"");
addTest(6,"Comment","",0,1,"");
addTest(6,"Block","block",1,1,"");
currentAlt->immediate = StatementBody12TawkNow;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(6,"Commands","",1,1,"");
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(1,";","statement",1,1,"");
currentAlt->immediate = StatementBody13TawkNow;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(1,"delete","statement",1,1,"");
addTest(1,"[]","array",0,1,"");
addTest(6,"Qualified","instance",1,1,"");
addTest(1,";","",1,1,"");
currentAlt->immediate = StatementBody14TawkNow;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(1,"do","",1,1,"");
addTest(3,"textFollow","",-1,1,"");
setNoSkip();
setBanged();
addTest(6,"Iterating","",0,1,"");
addTest(6,"Statement","statement",1,1,"");
addTest(6,"ResetType","",1,1,"");
addTest(1,"while","",1,1,"");
addTest(3,"textFollow","",-1,1,"");
setNoSkip();
setBanged();
addTest(6,"Expression","instance",1,1,"");
addTest(1,";","",1,1,"");
currentAlt->immediate = StatementBody15TawkNow;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(6,"Throw","statement",1,1,"");
addTest(1,";","",1,1,"");
currentAlt->immediate = StatementBody16TawkNow;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(6,"Try","statement",1,1,"");
currentAlt->immediate = StatementBody17TawkNow;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(6,"Declaration","statement",1,1,"");
addTest(1,";","",1,1,"");
currentAlt->immediate = StatementBody18TawkNow;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(6,"Expression","statement",1,1,"");
addTest(1,";","",1,1,"");
currentAlt->immediate = StatementBody19TawkNow;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(6,"Lambda","statement",1,1,"");
currentAlt->immediate = StatementBody20TawkNow;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("CommentBody");
currentAlt = new Alternative();
addTest(6,"CodePass","comment",1,1,"");
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(1,"//","",1,1,"");
addTest(6,"EndComment","end",1,1,"");
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(1,"/*","",1,1,"");
addTest(1,"*/","",1,1,"");
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(1,"#ifdef","",1,1,"");
addTest(1,"#endif","",1,1,"");
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(1,"#define","",1,1,"");
addTest(6,"EndComment","end",1,1,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("StatementBody5");
currentRule = getRule("MethodNameSet");
currentAlt = new Alternative();
addTest(3,"nameStartSet","",1,1,"");
addTest(3,"methodNameSet","",0,999999,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("NameSet");
currentAlt = new Alternative();
addTest(3,"nameStartSet","",1,1,"");
addTest(3,"nameSet","",0,999999,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("StatementBody6");
currentRule = getRule("StatementBody7");
currentRule = getRule("StatementBody9");
currentRule = getRule("StatementBody10");
currentRule = getRule("StatementBody11");
currentRule = getRule("Replacement");
currentAlt = new Alternative();
addTest(6,"Quote","",1,1,"");
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(3,"^,;\n","",1,999999,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("NewArray");
currentAlt = new Alternative();
addTest(1,"[","",1,1,"");
addTest(6,"Expression","instance",0,1,"");
addTest(1,"]","",1,1,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("StatementBody12");
currentRule = getRule("ItemHead");
currentAlt = new Alternative();
addTest(6,"Indirection","direct",0,1,"");
addTest(6,"Name","name",1,1,"");
addTest(6,"ItemArray","array",0,999999,"");
addTest(6,"Bits","bits",0,1,"");
currentAlt->immediate = ItemHeadTawkNow;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("StatementBody13");
currentRule = getRule("QualifyTail");
currentAlt = new Alternative();
addTest(1,".","",1,1,"");
addTest(6,"FieldBody","field",1,1,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("StructureType2");
currentRule = getRule("StatementBody15");
currentRule = getRule("StatementBody18");
currentRule = getRule("StatementBody19");
currentRule = getRule("StatementBody20");
currentRule = getRule("EscapeCharacters");
currentAlt = new Alternative();
addTest(3,"nrtbf\"'\\","",1,1,"");
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(1,"u","",1,999999,"");
addTest(3,"0-9a-fA-F","",4,0,"");
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(3,"0-3","",1,1,"");
addTest(3,"0-9","",0,1,"");
addTest(3,"0-9","",0,1,"");
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(3,"4-7","",1,1,"");
addTest(3,"0-9","",0,1,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("Switch");
currentAlt = new Alternative();
addTest(1,"switch","statement",1,1,"");
addTest(3,"textFollow","",-1,1,"");
setNoSkip();
setBanged();
addTest(6,"FieldBody3","name",0,1,"");
currentAlt->immediate = SwitchTawkNow;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("Target");
currentAlt = new Alternative();
addTest(6,"Qualified","field",1,1,"");
currentAlt->immediate = TargetTawkNow;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(6,"Name","field",1,1,"");
currentAlt->immediate = Target2TawkNow;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("PrintTarget");
currentAlt = new Alternative();
addTest(1,"(","",1,1,"");
setNoSkip();
addTest(6,"FieldExpression","instance",0,1,"");
addTest(1,")","",1,1,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("CastTail");
currentAlt = new Alternative();
addTest(1,"(","",1,1,"");
addTest(6,"CastType","rest",1,999999,"");
addTest(1,")","",1,1,"");
currentAlt->immediate = CastTailTawkNow;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("DotH");
currentRule = getRule("Bump");
currentAlt = new Alternative();
addTest(6,"BumpBlock0","bump",1,1,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("CodePass");
currentAlt = new Alternative();
addTest(1,"-%","",1,1,"");
addTest(3,"^%-","comment",1,999999,"");
addTest(1,"%-","",1,1,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("CastTypeBlock0");
currentAlt = new Alternative();
addTest(1,"[]","",1,999999,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("OverLoadItemBlock0");
currentAlt = new Alternative();
addTest(3,"operatorSet","",1,999999,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("IncludeBlock0");
currentAlt = new Alternative();
addTest(1,"include","",1,1,"");
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(1,"import","",1,1,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("Comment");
currentAlt = new Alternative();
addTest(6,"CommentBody","comment",1,999999,"");
currentAlt->immediate = CommentTawkNow;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("EndComment");
currentAlt = new Alternative();
addTest(3,"^\n","",1,999999,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("Count");
currentAlt = new Alternative();
addTest(3,"0-9","",1,999999,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("ConditionList");
currentRule = getRule("Declaration");
currentAlt = new Alternative();
addTest(1,"outlet","outlet",0,1,"");
{
Element 	*elem = new Element();
	elem->minimum = 0;
	elem->maximum = 999999;
	elem->kind = 8;
	elem->label = "modify";
	currentAlt->elements->add((void*)elem);
}
addTest(6,"DeclareType","type",1,1,"");
addTest(6,"DeclareItem","declare",1,999999,"");
currentAlt->defer = DeclarationTawkAct;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(6,"Structure","declare",1,1,"");
currentAlt->defer = Declaration2TawkAct;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(6,"DeclareConditions","",1,1,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("CaseLabel3");
currentRule = getRule("AliasTarget");
currentAlt = new Alternative();
addTest(6,"Type","type",1,1,"");
addTest(6,"Indirection","indirect",0,1,"");
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(6,"Name","target",1,1,"");
addTest(6,"AliasBody","body",0,1,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("Escape");
currentAlt = new Alternative();
addTest(1,"\\","",1,1,"");
addTest(6,"EscapeCharacters","",1,1,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("ExpressList");
currentAlt = new Alternative();
addTest(6,"NoShortcuts","",1,1,"");
addTest(6,"ExpressItem","list",1,999999,"");
currentAlt->immediate = ExpressListTawkNow;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("MacroPart");
currentAlt = new Alternative();
addTest(3,"^,(","",1,999999,"");
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(6,"Braced","",1,1,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("ExpressPart");
currentAlt = new Alternative();
addTest(6,"SaveVirtuals","",1,1,"");
addTest(6,"UnaryOperator","unaryOp",0,1,"");
addTest(6,"UnaryExpression","instance",1,1,"");
addTest(6,"RangeTail","",-1,1,"");
setBanged();
addTest(6,"ExpressType","",0,1,"");
addTest(6,"OperationTail","express",0,999999,"");
currentAlt->immediate = ExpressPartTawkNow;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("ClassAttributes4");
currentRule = getRule("DeclareConditions");
currentAlt = new Alternative();
addTest(1,"Conditions","",1,1,"");
addTest(6,"ConditionLabel","",1,999999,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("Field");
currentAlt = new Alternative();
addTest(6,"Name","name",1,1,"");
currentAlt->immediate = FieldTawkNow;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("ParameterList");
currentAlt = new Alternative();
addTest(6,"ExpressItem","expression",1,999999,"");
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(6,"TypeList","expression",1,999999,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("Initializer");
currentAlt = new Alternative();
addTest(6,"ExpressList","instance",1,1,"");
addTest(1,",","",0,1,"");
currentAlt->immediate = InitializerTawkNow;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(6,"ArrayInitializer","instance",1,1,"");
addTest(1,",","",0,1,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("ArrayInitializer");
currentAlt = new Alternative();
addTest(1,"{","",1,1,"");
addTest(6,"Initializer","instance",0,999999,"");
addTest(1,"}","",1,1,"");
currentAlt->immediate = ArrayInitializerTawkNow;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("ItemInitializer");
currentAlt = new Alternative();
addTest(1,"=","",1,1,"");
addTest(6,"SetObject","",0,1,"");
addTest(6,"ItemInitializerBlock0","instance",1,1,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("MethodInitializer");
currentAlt = new Alternative();
addTest(1,"=","",1,1,"");
addTest(6,"FieldExpression","instance",1,1,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("InstanceBody");
currentAlt = new Alternative();
addTest(6,"InstanceTail","body",1,1,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("Quote");
currentAlt = new Alternative();
addTest(1,"@","string",0,1,"");
addTest(1,"\"","instance",1,1,"");
addTest(3,"^\"","body",1,999999,"");
setNoSkip();
addTest(1,"\"","",1,1,"");
setNoSkip();
currentAlt->immediate = QuoteTawkNow;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("Label");
currentAlt = new Alternative();
addTest(6,"Name","name",1,1,"");
addTest(1,":","",1,1,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("MethodParameters");
currentAlt = new Alternative();
addTest(1,"(","",1,1,"");
addTest(6,"Parameter","parameter",0,999999,"");
addTest(1,"...","ellipsis",0,1,"");
addTest(1,")","",1,1,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("AssumedString");
currentAlt = new Alternative();
addTest(6,"NameSet","instance",1,1,"");
currentAlt->immediate = AssumedStringTawkNow;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("MacroParameters");
currentAlt = new Alternative();
addTest(1,"(","",1,1,"");
addTest(6,"MacroElement","parameters",1,999999,"");
addTest(1,")","",1,1,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("AliasParameters");
currentAlt = new Alternative();
addTest(6,"AliasParameter","body",0,999999,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("InitializerItem");
currentAlt = new Alternative();
addTest(6,"NameSet","field",1,1,"");
addTest(6,"NameSet","function",1,1,"");
currentAlt->immediate = InitializerItemTawkNow;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("CheckMacroParameters");
currentAlt = new Alternative();
addTest(1,"(","braced",1,1,"");
addTest(1,")","",1,1,"");
currentAlt->immediate = CheckMacroParametersTawkNow;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("Modify");
currentRule = getRule("NumberBlock2");
currentAlt = new Alternative();
addTest(6,"NumberBlock2Block3","",1,1,"");
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(6,"NumberBlock2Block4","",1,1,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("Name");
currentAlt = new Alternative();
addTest(6,"NameSet","name",1,1,"");
currentAlt->immediate = NameTawkNow;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("NotQuote2");
currentRule = getRule("Number");
currentAlt = new Alternative();
addTest(6,"NumberBlock0","instance",1,1,"");
currentAlt->immediate = NumberTawkNow;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(6,"NumberBlock2","instance",1,1,"");
addTest(3,"lL","isLong",0,1,"");
currentAlt->immediate = Number2TawkNow;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("Operator");
currentAlt = new Alternative();
{
Element 	*elem = new Element();
	elem->minimum = 1;
	elem->maximum = 1;
	elem->kind = 8;
	elem->label = "operand";
	currentAlt->elements->add((void*)elem);
}
currentAlt->immediate = OperatorTawkNow;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
{
Element 	*elem = new Element();
	elem->minimum = 1;
	elem->maximum = 1;
	elem->kind = 8;
	elem->label = "comparator";
	currentAlt->elements->add((void*)elem);
}
addTest(3,"alphaSet","",-1,1,"");
setNoSkip();
setBanged();
currentAlt->immediate = Operator2TawkNow;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("UnaryOperator");
currentAlt = new Alternative();
addTest(6,"Bump","operate",1,1,"");
currentAlt->defer = UnaryOperatorTawkAct;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(3,"-+!~","operate",1,999999,"");
currentAlt->immediate = UnaryOperator2TawkNow;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("Path");
currentAlt = new Alternative();
addTest(1,"/","",0,1,"");
addTest(6,"PathBlock0","",1,999999,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("Imports");
currentAlt = new Alternative();
addTest(6,"Include","",1,1,"");
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(6,"Commands","",1,1,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("Question");
currentAlt = new Alternative();
addTest(1,"?","question",1,1,"");
addTest(6,"Expression","trueExp",1,1,"");
addTest(1,":","",1,1,"");
addTest(6,"Expression","falseExp",1,1,"");
currentAlt->immediate = QuestionTawkNow;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("RangeField");
currentAlt = new Alternative();
addTest(6,"Name","instance",1,1,"");
currentAlt->immediate = RangeFieldTawkNow;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(6,"RangeExpression","instance",1,1,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("RuleList");
currentAlt = new Alternative();
addTest(1,"Rule","",1,1,"");
addTest(3,"textFollow","",-1,1,"");
setNoSkip();
setBanged();
addTest(1,";","",1,1,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("ClassName");
currentAlt = new Alternative();
addTest(6,"ClassAttributes","",-1,1,"");
setBanged();
addTest(6,"Path","path",0,1,"");
addTest(6,"NameSet","name",1,1,"");
setNoSkip();
addTest(6,"Template","temp",0,1,"");
addTest(1,".h","dotH",0,1,"");
setNoSkip();
currentAlt->immediate = ClassNameTawkNow;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("Strings");
currentAlt = new Alternative();
addTest(6,"StringExpression","item",1,999999,"");
currentAlt->immediate = StringsTawkNow;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("SyntaxExtensions");
currentAlt = new Alternative();
addTest(1,"overload","",1,1,"");
addTest(3,"textFollow","",-1,1,"");
setNoSkip();
setBanged();
addTest(6,"OverLoadItem","",1,999999,"");
addTest(1,";","",1,1,"");
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(1,"alias","",1,1,"");
addTest(3,"textFollow","",-1,1,"");
setNoSkip();
setBanged();
addTest(6,"AliasItem","",1,999999,"");
addTest(1,";","",1,1,"");
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(1,"extender","",1,1,"");
addTest(3,"textFollow","",-1,1,"");
setNoSkip();
setBanged();
addTest(6,"Extender","",1,999999,"");
addTest(1,";","",1,1,"");
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(1,"initializer","",1,1,"");
addTest(3,"textFollow","",-1,1,"");
setNoSkip();
setBanged();
addTest(6,"InitializerItem","",1,999999,"");
addTest(1,";","",1,1,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("Template");
currentAlt = new Alternative();
addTest(1,"<>","",1,1,"");
setNoSkip();
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(1,"<","",1,1,"");
setNoSkip();
addTest(1,">","",1,1,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("Macro");
currentRule = getRule("Try");
currentAlt = new Alternative();
addTest(1,"try","",1,1,"");
addTest(6,"Statement","statement",1,1,"");
addTest(6,"ResetType","",1,1,"");
addTest(6,"Catch","catch",0,999999,"");
addTest(6,"Final","end",0,1,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("StructureBody2");
currentRule = getRule("Type");
currentAlt = new Alternative();
addTest(1,"const","hasConst",0,1,"");
addTest(1,"unsigned","noSign",0,1,"");
addTest(6,"TypeName","type",1,1,"");
addTest(6,"Template","temp",0,1,"");
currentAlt->immediate = TypeTawkNow;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("TypeList");
currentAlt = new Alternative();
addTest(6,"Type","instance",1,1,"");
addTest(1,",","",0,1,"");
currentAlt->immediate = TypeListTawkNow;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("ClassAttributes");
currentAlt = new Alternative();
{
Element 	*elem = new Element();
	elem->minimum = 1;
	elem->maximum = 1;
	elem->kind = 8;
	elem->label = "trait";
	currentAlt->elements->add((void*)elem);
}
addTest(3,"textFollow","",-1,1,"");
setNoSkip();
setBanged();
currentAlt->defer = ClassAttributesTawkAct;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(1,"extends","",1,1,"");
addTest(6,"Type","type",1,1,"");
currentAlt->defer = ClassAttributes2TawkAct;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(1,"implements","",1,1,"");
addTest(6,"Type","proto",1,999999,"");
currentAlt->defer = ClassAttributes3TawkAct;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(1,"namespace","",1,1,"");
addTest(6,"NameSet","nSpace",1,1,"");
currentAlt->defer = ClassAttributes4TawkAct;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("AliasItem");
currentAlt = new Alternative();
addTest(6,"Field","name",1,1,"");
addTest(6,"TargetMethod","target",1,1,"");
currentAlt->immediate = AliasItemTawkNow;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(1,"new","",1,1,"");
addTest(6,"Name","alias",1,1,"");
currentAlt->immediate = AliasItem2TawkNow;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(6,"Comment","",1,1,"");
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(6,"NameSet","alias",1,1,"");
addTest(6,"AliasTarget","value",1,1,"");
currentAlt->immediate = AliasItem3TawkNow;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("AliasItem3");
currentRule = getRule("Final");
currentAlt = new Alternative();
addTest(1,"finally","",1,1,"");
addTest(6,"Statement","statement",1,1,"");
addTest(6,"ResetType","",0,1,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("CheckMacro");
currentAlt = new Alternative();
addTest(6,"MacroName","statement",1,1,"");
addTest(6,"CheckMacroParameters","braced",0,1,"");
addTest(1,";","",0,1,"");
currentAlt->immediate = CheckMacroTawkNow;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("ConditionLabel");
currentAlt = new Alternative();
addTest(6,"Name","label",1,1,"");
addTest(3," \t","",0,1,"");
addTest(3,"^\n","text",1,999999,"");
setNoSkip();
addTest(1,"\n","",1,1,"");
setNoSkip();
currentAlt->immediate = ConditionLabelTawkNow;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("Extender");
currentAlt = new Alternative();
addTest(6,"Name","name",1,1,"");
currentAlt->immediate = ExtenderTawkNow;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("QualifyType");
currentAlt = new Alternative();
addTest(6,"Type","type",1,1,"");
addTest(1,".","",1,1,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("OverLoadItem");
currentAlt = new Alternative();
addTest(6,"Operator","operate",1,1,"");
addTest(6,"Name","name",1,1,"");
currentAlt->immediate = OverLoadItemTawkNow;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(1,"[]","",1,1,"");
addTest(1,"=","assign",0,1,"");
addTest(6,"Name","name",1,1,"");
currentAlt->immediate = OverLoadItem2TawkNow;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(6,"Bump","operate",1,1,"");
addTest(6,"Name","name",1,1,"");
currentAlt->immediate = OverLoadItem3TawkNow;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(6,"OverLoadItemBlock0","newOp",1,1,"");
addTest(6,"Name","name",1,1,"");
currentAlt->immediate = OverLoadItem4TawkNow;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(1,"()","",1,1,"");
addTest(6,"Name","name",1,1,"");
currentAlt->immediate = OverLoadItem5TawkNow;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("ButtonArray");
currentAlt = new Alternative();
addTest(1,"[","",1,1,"");
addTest(6,"Name","button",1,999999,"");
addTest(1,"]","",1,1,"");
currentAlt->defer = ButtonArrayTawkAct;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("OverLoadItem2");
currentRule = getRule("OverLoadItem3");
currentRule = getRule("MacroBody");
currentAlt = new Alternative();
addTest(6,"MacroBodyPart","parts",1,999999,"");
currentAlt->immediate = MacroBodyTawkNow;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("MacroDelimit");
currentAlt = new Alternative();
addTest(3,"^a-zA-z0-9;","delimiter",1,1,"");
currentAlt->immediate = MacroDelimitTawkNow;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("MacroDefine");
currentAlt = new Alternative();
addTest(6,"Name","name",1,1,"");
addTest(6,"MacroParameters","parameters",0,1,"");
addTest(6,"MacroDelimit","",1,1,"");
addTest(3,"^","body",1,999999,"");
addTest(1,"","",1,1,"");
currentAlt->immediate = MacroDefineTawkNow;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("AliasParameterBlock0");
currentAlt = new Alternative();
addTest(1,"=","",1,1,"");
addTest(6,"Replacement","replacedBy",1,1,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("CharacterBlock0");
currentAlt = new Alternative();
addTest(3,"'","",1,1,"");
addTest(6,"CharacterBlock0Block1","",1,1,"");
addTest(3,"'","",1,1,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("NumberBlock0");
currentAlt = new Alternative();
addTest(3,"0-9","",1,999999,"");
addTest(1,".","",1,1,"");
addTest(3,"0-9","",1,999999,"");
addTest(6,"NumberBlock0Block1","",0,1,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("ForOptionBlock0");
currentAlt = new Alternative();
addTest(1,"on","",1,1,"");
addTest(6,"Name","name",1,1,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("ItemInitializerBlock0");
currentAlt = new Alternative();
addTest(6,"ArrayInitializer","",1,1,"");
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(6,"InitExpression","",1,1,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("AliasBody");
currentAlt = new Alternative();
addTest(1,"(","body",1,1,"");
addTest(1,")","",1,1,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("LambdaName");
currentAlt = new Alternative();
addTest(6,"NameSet","name",1,1,"");
currentAlt->immediate = LambdaNameTawkNow;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("MacroElement");
currentAlt = new Alternative();
addTest(3,"^,)","element",1,999999,"");
addTest(1,",","",0,1,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("MacroBit");
currentAlt = new Alternative();
addTest(3,"a-zA-z0-9","bitpart",1,999999,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("DeclareItem3");
currentRule = getRule("Assuming");
currentRule = getRule("MacroBodyPart");
currentAlt = new Alternative();
addTest(3,"^","other",1,999999,"");
addTest(1,"","",1,1,"");
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(4,"","rest",1,999999,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("Braced");
currentAlt = new Alternative();
addTest(1,"(","",1,1,"");
addTest(1,")","",1,1,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("Case");
currentAlt = new Alternative();
addTest(1,"default:","instance",1,1,"");
currentAlt->immediate = CaseTawkNow;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(1,"case","",1,1,"");
addTest(3,"textFollow","",-1,1,"");
setNoSkip();
setBanged();
addTest(6,"Assuming","",1,1,"");
addTest(6,"CaseLabel","instance",1,1,"");
addTest(1,":","",1,1,"");
currentAlt->immediate = Case2TawkNow;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(6,"Label","instance",1,1,"");
currentAlt->immediate = Case3TawkNow;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("MacroArgument");
currentAlt = new Alternative();
addTest(6,"MacroArgumentBlock0","part",1,1,"");
addTest(1,",","",0,1,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("MacroArgumentBlock0");
currentAlt = new Alternative();
addTest(6,"MacroPart","",1,999999,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("ExpressTailBlock0");
currentAlt = new Alternative();
addTest(1,"&&","",1,1,"");
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(1,"||","",1,1,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("BumpBlock0");
currentAlt = new Alternative();
addTest(1,"++","",1,1,"");
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(1,"--","",1,1,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("FileName");
currentAlt = new Alternative();
addTest(6,"Path","path",0,1,"");
addTest(6,"NameSet","name",1,1,"");
addTest(1,".twk","",1,1,"");
currentAlt->immediate = FileNameTawkNow;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("PathBlock0");
currentAlt = new Alternative();
addTest(6,"Alpha","",1,1,"");
addTest(1,"/","",1,1,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("Case2");
currentRule = getRule("Case3");
currentRule = getRule("CaseLabel");
currentAlt = new Alternative();
addTest(6,"Constant","instance",1,1,"");
addTest(1,":","",1,1,"");
setIgnored();
currentAlt->immediate = CaseLabelTawkNow;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(6,"RangeField","instance",1,1,"");
currentAlt->immediate = CaseLabel2TawkNow;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(6,"Qualified","instance",1,1,"");
addTest(1,":","",1,1,"");
setIgnored();
currentAlt->immediate = CaseLabel3TawkNow;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(6,"Expression","instance",1,1,"");
currentAlt->immediate = CaseLabel4TawkNow;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(6,"Name","instance",1,1,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("CaseLabel4");
currentRule = getRule("ClassAttributes2");
currentRule = getRule("ClassAttributes3");
currentRule = getRule("Constant2");
currentRule = getRule("Constant3");
currentRule = getRule("Stop");
currentRule = getRule("DebugDirective");
currentAlt = new Alternative();
addTest(1,"#","",-1,1,"");
setBanged();
addTest(6,"Comment","",0,1,"");
addTest(6,"Name","method",1,1,"");
addTest(6,"CodeMatch","body",0,1,"");
{
Element 	*elem = new Element();
	elem->minimum = 0;
	elem->maximum = 1;
	elem->kind = 8;
	elem->label = "locate";
	currentAlt->elements->add((void*)elem);
}
addTest(1,"active","active",0,1,"");
addTest(3,"^#;","code",1,999999,"");
addTest(1,"#;","",1,1,"");
currentAlt->defer = DebugDirectiveTawkAct;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("Directive");
currentAlt = new Alternative();
addTest(6,"Type","type",1,1,"");
addTest(6,"DebugDirective","directives",0,999999,"");
currentAlt->immediate = DirectiveTawkNow;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("Number2");
currentRule = getRule("SyntaxExtensions2");
currentRule = getRule("Alpha");
currentAlt = new Alternative();
addTest(3,"a-zA-Z0-9_.","",1,999999,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("CharacterBlock0Block1");
currentAlt = new Alternative();
addTest(6,"Escape","",1,1,"");
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(3,"^'","",1,1,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("NumberBlock0Block1");
currentAlt = new Alternative();
addTest(3,"eE","",1,1,"");
addTest(3,"+-","",0,1,"");
addTest(3,"0-9","",1,999999,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("Commands");
currentAlt = new Alternative();
addTest(1,"#","",1,1,"");
addTest(6,"PoundCommand","",1,1,"");
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(6,"SyntaxExtensions","",1,1,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("CodeMatch");
currentAlt = new Alternative();
addTest(6,"Quote","body",1,1,"");
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
{
Element 	*elem = new Element();
	elem->minimum = -1;
	elem->maximum = 1;
	elem->kind = 8;
	elem->banged = 1;
	currentAlt->elements->add((void*)elem);
}
addTest(3,"^","body",1,999999,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("Format");
currentAlt = new Alternative();
addTest(1,"#","",1,1,"");
addTest(3,"- 0+","",0,1,"");
addTest(3,"0-9","width",0,999999,"");
addTest(3,"*%.0-9a-zA-Z","",0,999999,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("DebugText");
currentAlt = new Alternative();
addTest(1,"=","",1,1,"");
addTest(6,"Quote","upcoming",1,1,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("DebugRule");
currentAlt = new Alternative();
addTest(6,"NameSet","name",1,999999,"");
addTest(6,"DebugText","upcoming",0,1,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("NumberBlock2Block3");
currentAlt = new Alternative();
addTest(1,"0","",1,1,"");
addTest(3,"xX","",1,1,"");
addTest(3,"0-9a-fA-F","",1,999999,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("NumberBlock2Block4");
currentAlt = new Alternative();
addTest(3,"0-9","",1,999999,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("DeclareItem");
currentAlt = new Alternative();
addTest(6,"MethodHead","item",1,1,"");
addTest(6,"MethodInitializer","instance",0,1,"");
addTest(1,",","",0,1,"");
addTest(6,"Comment","",0,1,"");
currentAlt->defer = DeclareItemTawkAct;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(6,"Name","item",1,1,"");
addTest(1,"(","",1,1,"");
addTest(6,"Expression","argument",1,1,"");
addTest(1,")","",1,1,"");
currentAlt->defer = DeclareItem2TawkAct;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(6,"ItemHead","item",1,1,"");
addTest(6,"ItemInitializer","assign",0,1,"");
addTest(1,":","initialize",0,1,"");
addTest(1,",","",0,1,"");
addTest(6,"Comment","",0,1,"");
currentAlt->defer = DeclareItem3TawkAct;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("StructureItem");
currentAlt = new Alternative();
addTest(6,"Declaration","item",1,1,"");
addTest(1,";","",0,1,"");
currentAlt->defer = StructureItemTawkAct;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(6,"Name","name",1,1,"");
addTest(6,"Bits","bits",0,1,"");
addTest(6,"ButtonArray","buttons",0,1,"");
addTest(1,",","",0,1,"");
currentAlt->defer = StructureItem2TawkAct;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("DeclareItem2");
currentRule = getRule("StructureItem2");
currentRule = getRule("DeclareType");
currentAlt = new Alternative();
addTest(6,"Type","type",1,1,"");
currentAlt->immediate = DeclareTypeTawkNow;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("StructureType");
currentAlt = new Alternative();
addTest(6,"Type","type",1,1,"");
currentAlt->defer = StructureTypeTawkAct;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(6,"Name","type",1,1,"");
currentAlt->defer = StructureType2TawkAct;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("Declaration2");
currentRule = getRule("SetObject");
currentRule = getRule("Structure");
currentAlt = new Alternative();
{
Element 	*elem = new Element();
	elem->minimum = 1;
	elem->maximum = 1;
	elem->kind = 8;
	elem->label = "kind";
	currentAlt->elements->add((void*)elem);
}
addTest(3,"textFollow","",-1,1,"");
setNoSkip();
setBanged();
addTest(6,"StructureBody","body",1,1,"");
currentAlt->defer = StructureTawkAct;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("StructureBody");
currentAlt = new Alternative();
addTest(6,"StructureType","label",1,1,"");
addTest(1,"{","",1,1,"");
addTest(6,"StructureItem","entry",1,999999,"");
addTest(1,"}","",1,1,"");
addTest(6,"DeclareItem","field",0,999999,"");
currentAlt->defer = StructureBodyTawkAct;
currentRule->alternatives->add(currentAlt);
currentAlt = new Alternative();
addTest(6,"StructureItem","entry",1,999999,"");
currentAlt->defer = StructureBody2TawkAct;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("TypeName");
currentAlt = new Alternative();
addTest(6,"NameSet","type",1,1,"");
currentAlt->immediate = TypeNameTawkNow;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("Bits");
currentAlt = new Alternative();
addTest(1,":","",1,1,"");
addTest(6,"Number","length",1,1,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("ConditionWord");
currentAlt = new Alternative();
{
Element 	*elem = new Element();
	elem->minimum = 1;
	elem->maximum = 1;
	elem->kind = 8;
	elem->label = "list";
	currentAlt->elements->add((void*)elem);
}
addTest(3,"textFollow","",-1,1,"");
setNoSkip();
setBanged();
currentAlt->defer = ConditionWordTawkAct;
currentRule->alternatives->add(currentAlt);
currentRule = getRule("ExpressType");
currentRule = getRule("OperationTail2");
currentRule = getRule("Operator2");
currentRule = getRule("Divert");
currentAlt = new Alternative();
addTest(6,"Inheritance","",1,999999,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("UnaryOperator2");
currentRule = getRule("ExpressTail");
currentAlt = new Alternative();
addTest(6,"ExpressTailBlock0","operate",1,1,"");
addTest(6,"ExpressPart","instance",1,1,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("Catch");
currentAlt = new Alternative();
addTest(1,"catch","",1,1,"");
addTest(6,"Parameter","except",0,1,"");
addTest(6,"Statement","statement",1,1,"");
addTest(6,"ResetType","",0,1,"");
currentRule->alternatives->add(currentAlt);
currentRule = getRule("PrintTo");
currentAlt = new Alternative();
addTest(1,"to","",1,1,"");
addTest(6,"FieldExpression","instance",1,1,"");
currentRule->alternatives->add(currentAlt);
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
// Ignoring declaration of unused variable type in method: AliasItem3TawkNow(PLGitem*)
// Ignoring declaration of unused variable brace in method: BlockStartTawkNow(PLGitem*)
// Ignoring declaration of unused variable body in method: BodyTawkAct(PLGitem*)
// Ignoring declaration of unused variable instance in method: ConstantTawkAct(PLGitem*)
// Ignoring declaration of unused variable staticCopy in method: DeclarationTawkAct(PLGitem*)
// Ignoring declaration of unused variable initialize in method: DeclareItem3TawkAct(PLGitem*)
// Ignoring declaration of unused variable statement in method: ElseTawkAct(PLGitem*)
// Ignoring declaration of unused variable path in method: FileNameTawkNow(PLGitem*)
// Ignoring declaration of unused variable method in method: InheritanceTawkAct(PLGitem*)
// Ignoring declaration of unused variable direct in method: ItemHeadTawkNow(PLGitem*)
// Ignoring declaration of unused variable name in method: ItemHeadTawkNow(PLGitem*)
// Ignoring declaration of unused variable bits in method: ItemHeadTawkNow(PLGitem*)
// Ignoring declaration of unused variable statement in method: Line2TawkNow(PLGitem*)
// Ignoring declaration of unused variable modify in method: MethodTypeTawkAct(PLGitem*)
// Ignoring declaration of unused variable methodHead in method: MethodTypeTawkAct(PLGitem*)
// Ignoring declaration of unused variable selectedField in method: PoundCommandTawkNow(PLGitem*)
// Ignoring declaration of unused variable instance in method: PrintItemTawkAct(PLGitem*)
// Ignoring declaration of unused variable statement in method: StatementTawkAct(PLGitem*)
// Ignoring declaration of unused variable instance in method: StringExpressionTawkAct(PLGitem*)
// Ignoring declaration of unused variable type in method: StructureTypeTawkAct(PLGitem*)
// Ignoring declaration of unused variable operate in method: UnaryOperatorTawkAct(PLGitem*)
/*	Warning: the following methods were referenced but not declared
	string()
	toString()
	unString()
	setString(char*)
	copyTo(Buffer*)
*/
