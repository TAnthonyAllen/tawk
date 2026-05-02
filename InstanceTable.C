#include <string.h>
#include <stdio.h>
#include "StringRoutines.h"
#include "BaseHash.h"
#include "Types.h"
#include "Symbol.h"
#include "Stak.h"
#include "SymbolType.h"
#include "DoubleLinkList.h"
#include "DoubleLink.h"
#include "Instance.h"
#include "InstanceTable.h"

InstanceTable::InstanceTable()
{
	count = 0;
	debug = 0;
	scope = 0;
	foundAncestor = 0;
	presentClass = 0;
	scopeStack = new Stak();
	globalFields = new BaseHash(1000);
	instances = new DoubleLinkList();
	instances->hasKeys = 1;
	nullInstance = new Instance();
}

/******************************************************************************
	In order to handle function pointers, method names are added into
	the variable name space (may want to maintain a separate namespace??)
******************************************************************************/
void InstanceTable::add(Instance *instance)
{
Symbol 	*symbol = 0;
	symbol = instance->getSymbol();
	if ( !symbol )
		{
		instance->error("InstanceTable.add expected a symbol");
		return;
		}
	if ( debug )
		::printf("InstanceTable: adding %s\n",symbol->name);
	SymbolType::types->add(symbol->name);
	if ( symbol->isMethod && symbol->parameters )
		instances->add(symbol->gitMethodName(),(void*)instance);
	instances->add(symbol->name,(void*)instance);
	count++;
}

/******************************************************************************
	Creates a new symbol and adds it to the table
******************************************************************************/
Instance *InstanceTable::add(char *name, SymbolType *type)
{
Symbol 		*symbol = 0;
Instance 	*instance = 0;
	symbol = new Symbol(name,type);
	if ( debug )
		::printf("InstanceTable: adding new %s\n",symbol->name);
	instance = new Instance(symbol);
	add(instance);
	return instance;
}

/******************************************************************************
	Add instance to globalFields list.
******************************************************************************/
void InstanceTable::addGlobalField(char *text, Symbol *symbol)
{
Instance 	*instance = new Instance(symbol);
	globalFields->add(text,instance);
	if ( symbol->isMethod && symbol->methodName )
		globalFields->add(symbol->methodName,instance);
}

/******************************************************************************
	Simple dump of only whats in the table (no components)
******************************************************************************/
void InstanceTable::dump()
{
Instance 	*instance = 0;
Symbol 		*symbol = 0;
	::printf("Simple dump\n");
	instances->resetIterator();
	while ( instance = (Instance*)instances->next() )
		{
		symbol = instance->symbol;
		if ( symbol->isMethod )
			{
			::printf("\t%s %s ",symbol->type->name,instances->entry->key);
			::printf("%s %d\n",symbol->gitMethodName(),instance->instanceIndex);
			}
		else {
			::printf("\t%s",symbol->type->name);
			printQualifiedName(instance);
			::printf(" %d\n",instance->instanceIndex);
			}
		}
	::printf("End\n\n");
}

/******************************************************************************
	Debugging routine
******************************************************************************/
void InstanceTable::dump(char *text)
{
Instance 	*instance = 0;
Symbol 		*symbol = 0;
	::printf("%s\n",text);
	if ( presentClass )
		presentClass->dump();
	instances->resetIterator();
	::printf("Instances on stack\n");
	while ( instance = (Instance*)instances->prior() )
		{
		symbol = instance->symbol;
		if ( symbol->isMethod )
			{
			::printf("\t%s %s ",symbol->type->name,instances->entry->key);
			::printf("%s %d\n",symbol->gitMethodName(),instance->instanceIndex);
			}
		else {
			::printf("\t%s ",symbol->type->name);
			printQualifiedName(instance);
			::printf(" %d\n",instance->instanceIndex);
			}
		}
}

/******************************************************************************
	Dump global list
******************************************************************************/
void InstanceTable::dumpGlobals()
{
Instance 	*instance = 0;
Symbol 		*symbol = 0;
SymbolType 	*global = 0;
	::printf("Global Types\n");
	SymbolType::globalList->entry = 0;
	while ( global = (SymbolType*)SymbolType::globalList->next() )
		::printf("\t%s\n",global->name);
	::printf("Global Field List\n");
	globalFields->hashList->resetIterator();
	while ( instance = (Instance*)globalFields->hashList->next() )
		{
		symbol = instance->symbol;
		if ( symbol->isMethod )
			{
			::printf("\t%s %s ",symbol->type->name,globalFields->hashList->entry->key);
			::printf("%s %d\n",symbol->gitMethodName(),instance->instanceIndex);
			}
		else {
			::printf("\t%s ",symbol->type->name);
			printQualifiedName(instance);
			::printf(" %d\n",instance->instanceIndex);
			}
		}
	::printf("End Global Field List\n");
}

/******************************************************************************
	Find the named instance.
******************************************************************************/
Instance *InstanceTable::find(char *name)
{
Instance 	*copy = 0;
Instance 	*instance = (Instance*)instances->get(name);
	//cout "InstanceTable Finding " name:;
	if ( !instance )
		instance = findInstance(name);
	if ( instance )
		{
		copy = new Instance(instance);
		instance = copy;
		instance->isDeclaration = 0;
		if ( foundAncestor && instance != foundAncestor )
			instance->setParent(foundAncestor);
		}
	foundAncestor = 0;
	return instance;
}

/******************************************************************************
	Find the first global method matching the name passed in.
    Note: because of the way external .h declarations are handled, there can be
    multiple global types.
******************************************************************************/
Symbol *InstanceTable::findGlobalMethod(char *name)
{
Instance 	*instance = (Instance*)globalFields->get(name);
	if ( instance && instance->symbol && instance->isMethod )
		return instance->symbol;
	return 0;
}

/******************************************************************************
	Search thru current scope for matching symbol
******************************************************************************/
Instance *InstanceTable::findInstance(char *name)
{
Instance 	*field = 0;
Instance 	*last = 0;
Instance 	*lastParent = 0;
Instance 	*instance = 0;
SymbolType 	*type = 0;
Symbol 		*symbol = 0;
int 		currentLevel = 0;
int 		lowest = 0;
	foundAncestor = 0;
	SymbolType::types->resetIsFlagged();
	/**********************************************************************
	Look in the current stack.
	**********************************************************************/
	if ( field = (Instance*)instances->get(name) )
		{
		foundAncestor = 0;
		return field;
		}
	instances->resetIterator();
	while ( instance = (Instance*)instances->prior() )
		{
		if ( instance->isMethod )
			if ( ::compare(name,instance->symbol->methodName) == 0 )
				return instance;
			else	continue;
		if ( instance->howDirect() > 1 )
			continue;
		type = instance->getType();
		if ( type == presentClass || type->isFlagged || type->isAtomic )
			continue;
		type->isFlagged = 1;
		foundAncestor = instance;
		field = type->findFieldInstance(name);
		if ( field && (symbol = field->getSymbol()) )
			{
			if ( symbol->isStatic )
				return field;
			currentLevel = field->level;
			if ( foundAncestor )
				currentLevel++;
			if ( !lowest || currentLevel < lowest )
				{
				lowest = field->level;
				last = field;
				lastParent = foundAncestor;
				if ( foundAncestor )
					lowest++;
				}
			}
		}
	/**********************************************************************
	Look in the current type and its components.
	**********************************************************************/
	type = presentClass;
	foundAncestor = 0;
	field = type->findFieldInstance(name);
	if ( field && (symbol = field->getSymbol()) )
		{
		if ( symbol->isStatic )
			return field;
		currentLevel = field->level;
		if ( foundAncestor )
			currentLevel++;
		if ( !lowest || currentLevel < lowest )
			{
			lowest = field->level;
			last = field;
			lastParent = foundAncestor;
			if ( foundAncestor )
				lowest++;
			}
		}
	/**********************************************************************
	Look in globals. Only descends if we have not found anything yet
	**********************************************************************/
	if ( globalFields )
		{
		if ( field = (Instance*)globalFields->get(name) )
			{
			foundAncestor = 0;
			return field;
			}
		if ( !last )
			{
			globalFields->hashList->resetIterator();
			while ( instance = (Instance*)globalFields->hashList->prior() )
				{
				if ( instance->isMethod || instance->howDirect() != 1 )
					continue;
				type = instance->getType();
				if ( type->isFlagged || type->isAtomic )
					continue;
				type->isFlagged = 1;
				foundAncestor = instance;
				field = type->findFieldInstance(name);
				if ( field && (symbol = field->getSymbol()) )
					{
					if ( symbol->isStatic )
						return field;
					currentLevel = field->level;
					if ( foundAncestor )
						currentLevel++;
					if ( !lowest || currentLevel < lowest )
						{
						lowest = field->level;
						last = field;
						lastParent = foundAncestor;
						if ( foundAncestor )
							lowest++;
						}
					}
				}
			}
		}
	foundAncestor = lastParent;
	return last;
}

/******************************************************************************
	Return a new method instance matching the source passed in
******************************************************************************/
Instance *InstanceTable::findMethod(Instance *source)
{
Instance 	*copy = 0;
Instance 	*instance = 0;
char 		*name = source->symbol && source->symbol->isMethod ? source->symbol->methodName : source->mangle();
	/**********************************************************************
	In case what we are looking up is a constructor?
	**********************************************************************/
	if ( source->prefix && source->isMethod )
		if ( source->type = (SymbolType*)SymbolType::types->get(source->prefix) )
			return instance = source->type->findMethodInstance(source);
	if ( instance = findInstance(name) )
		{
		copy = new Instance(instance);
		copy->setReference((unsigned int)0);
		if ( foundAncestor && instance != foundAncestor )
			copy->setParent(foundAncestor);
		instance = copy;
		}
	foundAncestor = 0;
	return instance;
}

/******************************************************************************
	Simple search thru current scope for a matching symbol. Does not descend.
******************************************************************************/
Symbol *InstanceTable::findSymbol(char *name)
{
Instance 	*field = 0;
Symbol 		*symbol = 0;
	/**********************************************************************
	Look in the current stack.
	**********************************************************************/
	if ( field = (Instance*)instances->get(name) )
		if ( field->symbol )
			return symbol;
	/**********************************************************************
	Look in the current type and its components.
	**********************************************************************/
	symbol = presentClass->findField(name);
	if ( symbol )
		return symbol;
	/**********************************************************************
	Look in globals.
	**********************************************************************/
	if ( globalFields )
		if ( field = (Instance*)globalFields->get(name) )
			if ( field->symbol )
				return symbol;
	return 0;
}

/******************************************************************************
	Return a method instance that converts object of type source to an object of
	type target (if there is such a method)
******************************************************************************/
Instance *InstanceTable::getConverter(SymbolType *target, SymbolType *source)
{
Instance 	*result = 0;
Instance 	*instance = 0;
Symbol 		*found = 0;
SymbolType 	*type = 0;
	found = source->getConverter(target,source);
	if ( found )
		goto foundIt;
	found = target->getConverter(target,source);
	if ( found )
		goto foundIt;
	found = presentClass->getConverter(target,source);
	if ( found )
		goto foundIt;
	SymbolType::globalList->entry = 0;
	while ( type = (SymbolType*)SymbolType::globalList->next() )
		{
		found = type->getConverter(target,source);
		if ( found )
			goto foundIt;
		}
	instances->resetIterator();
	while ( instance = (Instance*)instances->prior() )
		{
		if ( instance->isMethod )
			continue;
		type = instance->getType();
		if ( type && !type->isAtomic )
			if ( found = type->getConverter(target,source) )
				{
				result = new Instance(found);
				result->setParent(instance);
				}
		}
	return result;
foundIt:
	result = new Instance(found);
	return result;
}

/******************************************************************************
	Pop the scopeStack stack
******************************************************************************/
void InstanceTable::pop(char *text)
{
long 		dump = count;
long 		i = 0;
Instance 	*instance = 0;
	count = (long)scopeStack->pop();
	//cout "Popping " text,"from " dump " to " count,scope "\n";
	for ( i = dump; i > count; i-- )
		instance = (Instance*)instances->pop();
	scope--;
}

/*******************************************************************************
        Print the fully qualified name of the instance for debugging
*******************************************************************************/
void InstanceTable::printQualifiedName(Instance *instance)
{
	if ( instance->parent )
		{
		printQualifiedName(instance->parent);
		::printf(".");
		}
	::printf("%s",instance->symbol->displayName());
}

/******************************************************************************
	Push the scopeStack stack
******************************************************************************/
void InstanceTable::push(char *text)
{
	scope++;
	//cout "Pushing " text,count,scope "\n";
	scopeStack->push((void*)count);
}
