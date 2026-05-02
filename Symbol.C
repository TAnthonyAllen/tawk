#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "StringRoutines.h"
#include "BaseHash.h"
#include "Types.h"
#include "SymbolType.h"
#include "DoubleLinkList.h"
#include "DoubleLink.h"
#include "PLGitem.h"
#include "InstanceTable.h"
#include "BlockTok.h"
#include "Instance.h"
#include "Symbol.h"
int Symbol::symbolCount;

// If isMethod, parameters is list of symbols defining method parameters
Symbol::Symbol(char *n)
{
	methodName = 0;
	parentClass = 0;
	structType = 0;
	type = 0;
	getter = 0;
	setter = 0;
	source = 0;
	block = 0;
	directives = 0;
	parameters = 0;
	hasEllipsis = 0;
	isAlias = 0;
	isArray = 0;
	isAssigned = 0;
	isButton = 0;
	isConst = 0;
	isConstructor = 0;
	isDefault = 0;
	isExtern = 0;
	isExtension = 0;
	isFlag = 0;
	isGetter = 0;
	isHidden = 0;
	isInitialized = 0;
	isInitializer = 0;
	isInline = 0;
	isItem = 0;
	isLambda = 0;
	isMethod = 0;
	isOCfield = 0;
	isOutlet = 0;
	isProper = 0;
	isSetter = 0;
	isStatic = 0;
	isThis = 0;
	isUsed = 0;
	isVirtual = 0;
	referred = 0;
	reference = 0;
	utilized = 0;
	array = 0;
	comment = 0;
	commentItem = 0;
	symbolBitLength = 0;
	symbolBitOffset = 0;
	symbolOffset = 0;
	format = 0;
	name = n;
	indirect = 1;
	Symbol::symbolCount++;
	symbolIndex = SymbolType::types->add(name);
}

/*******************************************************************************
        Copy constructor (shallow copy).
*******************************************************************************/
Symbol::Symbol(Symbol *src)
{
	*this = *src;
	Symbol::symbolCount++;
}

Symbol::Symbol(char *n, char *t)
{
	methodName = 0;
	parentClass = 0;
	structType = 0;
	getter = 0;
	setter = 0;
	source = 0;
	block = 0;
	directives = 0;
	parameters = 0;
	hasEllipsis = 0;
	indirect = 0;
	isAlias = 0;
	isArray = 0;
	isAssigned = 0;
	isButton = 0;
	isConst = 0;
	isConstructor = 0;
	isDefault = 0;
	isExtern = 0;
	isExtension = 0;
	isFlag = 0;
	isGetter = 0;
	isHidden = 0;
	isInitialized = 0;
	isInitializer = 0;
	isInline = 0;
	isItem = 0;
	isLambda = 0;
	isMethod = 0;
	isOCfield = 0;
	isOutlet = 0;
	isProper = 0;
	isSetter = 0;
	isStatic = 0;
	isThis = 0;
	isUsed = 0;
	isVirtual = 0;
	referred = 0;
	reference = 0;
	utilized = 0;
	array = 0;
	comment = 0;
	commentItem = 0;
	symbolBitLength = 0;
	symbolBitOffset = 0;
	symbolOffset = 0;
	format = 0;
	name = n;
	type = SymbolType::find(t);
	if ( !type )
		::printf("Symbol constructor could not find type %s while creating %s\n",t,n);
	indirect = type && type->isDirect ? 0 : 1;
	Symbol::symbolCount++;
	symbolIndex = SymbolType::types->add(name);
}

Symbol::Symbol(char *n, SymbolType *t)
{
	methodName = 0;
	parentClass = 0;
	structType = 0;
	getter = 0;
	setter = 0;
	source = 0;
	block = 0;
	directives = 0;
	parameters = 0;
	hasEllipsis = 0;
	indirect = 0;
	isAlias = 0;
	isArray = 0;
	isAssigned = 0;
	isButton = 0;
	isConst = 0;
	isConstructor = 0;
	isDefault = 0;
	isExtern = 0;
	isExtension = 0;
	isFlag = 0;
	isGetter = 0;
	isHidden = 0;
	isInitialized = 0;
	isInitializer = 0;
	isInline = 0;
	isItem = 0;
	isLambda = 0;
	isMethod = 0;
	isOCfield = 0;
	isOutlet = 0;
	isProper = 0;
	isSetter = 0;
	isStatic = 0;
	isThis = 0;
	isUsed = 0;
	isVirtual = 0;
	referred = 0;
	reference = 0;
	utilized = 0;
	array = 0;
	comment = 0;
	commentItem = 0;
	symbolBitLength = 0;
	symbolBitOffset = 0;
	symbolOffset = 0;
	format = 0;
	name = n;
	type = t;
	indirect = type && type->isDirect ? 0 : 1;
	Symbol::symbolCount++;
	symbolIndex = SymbolType::types->add(name);
}

/*******************************************************************************
        Add a parameter.
*******************************************************************************/
void Symbol::addParameter(Symbol *arg)
{
	if ( !isMethod )
		::fprintf(stderr,"Symbol addParameter: %s is not a method\n",name);
	else {
		if ( !parameters )
			parameters = new DoubleLinkList();
		parameters->add((void*)arg);
		if ( parentClass && isType(arg->type->structure) )
			parentClass->hasTypedef = 1;
		}
}

/*******************************************************************************
        Make sure parameter names and types are consistent upon redeclaration
*******************************************************************************/
void Symbol::checkParameters(PLGitem *parameter)
{
DoubleLink 	*link = 0;
PLGitem 	*atItem = 0;
PLGitem 	*item = 0;
Symbol 		*argument = 0;
Symbol 		*symbolParameter = 0;
	if ( parameters )
		{
		link = parameters->first;
		if ( parentClass->isC && link )
			link = link->next;
		}
	for ( atItem = parameter; atItem; atItem = atItem->next )
		{
		item = atItem->get("type");
		for ( item = (PLGitem*)item->value; item; item = item->next )
			{
			argument = (Symbol*)item->value;
			if ( link )
				{
				symbolParameter = (Symbol*)link->value;
				if ( !(symbolParameter->hasEllipsis && symbolParameter->type == SymbolType::nullType) )
					link->value = (void*)argument;
				link = link->next;
				}
			}
		}
}

/*******************************************************************************
        Simple version used for display in debugging.
*******************************************************************************/
char *Symbol::displayName()
{
char 	*text = 0;
	if ( isAlias && source )
		text = source->displayName();
	else	text = isMethod ? gitMethodName() : name;
	return text;
}

/*******************************************************************************
        Debug print alias method name showing defaults.
*******************************************************************************/
void Symbol::dumpDefaultName()
{
Symbol 	*arg = 0;
	::printf("%s %s( ",gitMethodName(),name);
	while ( arg = (Symbol*)parameters->next() )
		if ( arg->isDefault )
			::printf("%s=%s,",arg->name,arg->comment);
		else
		if ( arg->type->comment )
			::printf("%s %s,",arg->type->comment,arg->name);
		else	::printf("%s %s,",arg->type->name,arg->name);
	::printf(")\n");
}

/*******************************************************************************
        Treat this method as an extension of the class of its first parameter
        (which must have an indirection of 1).
*******************************************************************************/
void Symbol::extendType()
{
BaseHash 	*extenderMethods = 0;
Symbol 		*alias = 0;
Symbol 		*argument = 0;
Symbol 		*extender = 0;
	if ( !isMethod || !parameters )
		return;
	extender = (Symbol*)parameters->first->value;
	// For now, cannot extend simple types like int or char or String
	if ( extender->type->isAtomic || (extender->type->structure && !isType(extender->type->structure)) )
		{
		//cerr "Symbol extendType: cannot extend simple types like int or char or String.":;
		return;
		}
	if ( extender->indirect > 1 )
		{
		::fprintf(stderr,"Symbol extendType: first parameter of an extender must be a pointer.\n");
		return;
		}
	if ( !extender->type->methods )
		extender->type->methods = new BaseHash();
	extenderMethods = extender->type->methods;
	alias = makeAlias(name);
	alias->isExtension = 1;
	if ( alias->parameters && alias->parameters->length <= 1 )
		alias->parameters = 0;
	else {
		DoubleLinkList 	*list = new DoubleLinkList();
		DoubleLink 		*item = alias->parameters->first->next;
		for ( ; item; item = item->next )
			list->add(item->value);
		alias->parameters = list;
		}
	alias->methodName = 0;
	alias->gitMethodName();
	extenderMethods->hashList->entry = 0;
	while ( argument = (Symbol*)extenderMethods->next(name) )
		if ( ::compare(argument->methodName,alias->methodName) == 0 )
			{
			extenderMethods->hashList->entry = 0;
			return;
			}
	extender->type->addMethod(alias);
	extender->type->checkGetterSetter(alias);
}

/*******************************************************************************
        Returns the name of a method taking into account external C methods.
*******************************************************************************/
char *Symbol::externalMethodName()
{
char 	*text = 0;
	if ( isMethod && parentClass && parentClass->isC )
		text = ::concat(2,name,parentClass->name);
	else	text = name;
	return text;
}

/*******************************************************************************
        get methodName using Objective-C naming convention
*******************************************************************************/
char *Symbol::getOCmethodName()
{
int 		length = 0;
DoubleLink 	*link = 0;
Symbol 		*symbol = 0;
char 		*methodName = 0;
	if ( isAlias && source )
		return source->getOCmethodName();
	if ( !parameters )
		return name;
	else {
		length = (int)::strlen(name);
		for ( link = parameters->first; link; link = link->next )
			{
			symbol = (Symbol*)link->value;
			if ( symbol->type == SymbolType::nullType )
				break;
			if ( link->prior )
				length += (int)::strlen(symbol->name);
			length++;
			}
		methodName = (char*)::malloc(++length);
		::strcpy(methodName,name);
		for ( link = parameters->first; link; link = link->next )
			{
			symbol = (Symbol*)link->value;
			if ( symbol->type == SymbolType::nullType )
				break;
			if ( link->prior )
				::strcat(methodName,symbol->name);
			::strcat(methodName,":");
			}
		}
	return methodName;
}

/*******************************************************************************
		Like mangle but it includes the type and does not include the
		method name (used when a method is a parameter). If cppFlag set,
		writes it as a C++ signature.
*******************************************************************************/
char *Symbol::getSignature()
{
	return getSignature(0);
}

char *Symbol::getSignature(int cppFlag)
{
DoubleLink 	*link = 0;
Symbol 		*symbol = 0;
int 		i = 0;
char 		*signature = 0;
char 		*result = 0;
	if ( isAlias && source )
		return source->getSignature(cppFlag);
	if ( !isMethod && (isLambda || reference) )
		return "";
	signature = (char*)::alloca(1000);
	*signature = '\0';
	::strcat(signature,type->name);
	if ( !type->isDirect )
		::strcat(signature,"*");
	if ( isLambda || (reference && (isMethod || cppFlag)) )
		if ( cppFlag )
			if ( isLambda )
				::strcat(signature,"(^)");
			else	::strcat(signature,"(*)");
		else
		if ( isLambda )
			::strcat(signature,"^");
		else	::strcat(signature,"&");
	::strcat(signature,"(");
	if ( parameters )
		for ( link = parameters->first; link; link = link->next )
			{
			symbol = (Symbol*)link->value;
			if ( symbol->reference || symbol->isLambda )
				::strcat(signature,symbol->getSignature(cppFlag));
			else {
				::strcat(signature,symbol->type->name);
				for ( i = 0; i < symbol->indirect; i++ )
					::strcat(signature,"*");
				}
			if ( link->next )
				::strcat(signature,",");
			}
	::strcat(signature,")");
	result = (char*)::calloc(1 + (int)::strlen(signature),sizeof(char));
	::strcat(result,signature);
	return result;
}

/*******************************************************************************
        methodName getter.
*******************************************************************************/
char *Symbol::gitMethodName()
{
	if ( !isMethod )
		{
		char 	*error = ::concat(2,name," is not a method");
		return error;
		}
	if ( !methodName )
		mangle();
	return methodName;
}

/*******************************************************************************
        insert a parameter if this is a method. This is an unnecessary wrapper
        to make it easier to track parameter maintenance while debugging
*******************************************************************************/
void Symbol::insertParameter(Symbol *arg)
{
	if ( !isMethod )
		::fprintf(stderr,"insertParameter: %s is not a method\n",name);
	else {
		if ( !parameters )
			addParameter(arg);
		else	parameters->insert((void*)arg);
		}
}

/*******************************************************************************
        Make and return an alias of this symbol.
*******************************************************************************/
Symbol *Symbol::makeAlias(char *aliasName)
{
Symbol 	*alias = 0;
	alias = new Symbol(this);
	if ( !isAlias )
		{
		alias->isAlias = 1;
		alias->source = this;
		}
	if ( ::compare(alias->name,aliasName) != 0 )
		{
		alias->name = aliasName;
		if ( isMethod )
			alias->methodName = 0;
		}
	alias->symbolIndex = SymbolType::types->add(aliasName);
	return alias;
}

/******************************************************************************
    Makes a setter for Objective-C properties
******************************************************************************/
Symbol *Symbol::makeSetter()
{
char 	*first = 0;
char 	*target = 0;
Symbol 	*parameter = new Symbol(this);
Symbol 	*method = 0;
	parameter->parentClass = 0;
	::asprintf(&first,"%s",name);
	if ( *name < 'A' || *name > 'Z' )
		{
		*first -= 32;
		}
	::asprintf(&target,"set%s",first);
	method = new Symbol(target,SymbolType::voidType);
	method->isMethod = 1;
	method->addParameter(parameter);
	first = method->gitMethodName();
	if ( !parentClass->methods || !parentClass->methods->get(first) )
		{
		if ( isAlias )
			{
			method->isAlias = 1;
			method->source = source->setter;
			}
		parentClass->addMethod(method);
		return method;
		}
	else {
		delete method;
		delete parameter;
		}
	return 0;
}

/*******************************************************************************
        mangle creates the methodName by which methods are found. It appends
        the parameter types to the name. Note that it ignores the return type.
*******************************************************************************/
void Symbol::mangle()
{
int 		length = 0;
int 		i = 0;
int 		flag = 0;
DoubleLink 	*link = 0;
Symbol 		*symbol = 0;
	if ( !parameters )
		methodName = ::concat(2,name,"()");
	else {
		length = (int)::strlen(name) + 2;
		link = parameters->first;
		if ( parentClass && parentClass->isC && isAlias )
			link = link->next;
		for ( ; link; link = link->next )
			{
			symbol = (Symbol*)link->value;
			if ( symbol->isDefault )
				continue;
			if ( symbol->isLambda || (symbol->reference && symbol->isMethod) )
				{
				length++;
				length += (int)::strlen(symbol->getSignature());
				}
			else {
				length += (int)::strlen(symbol->type->name) + symbol->indirect;
				//if symbol.reference length++;
				}
			if ( link->next )
				length++;
			}
		methodName = (char*)::malloc(++length);
		::strcpy(methodName,name);
		::strcat(methodName,"(");
		link = parameters->first;
		if ( parentClass && parentClass->isC && isAlias )
			{
			link = link->next;
			flag = 1;
			}
		for ( ; link; link = link->next )
			{
			symbol = (Symbol*)link->value;
			if ( symbol->isDefault )
				{
				flag = 1;
				continue;
				}
			if ( !flag && link->prior )
				::strcat(methodName,",");
			flag = 0;
			if ( symbol->isLambda || (symbol->reference && symbol->isMethod) )
				::strcat(methodName,symbol->getSignature());
			else {
				::strcat(methodName,symbol->type->name);
				for ( i = 0; i < symbol->indirect; i++ )
					::strcat(methodName,"*");
				//if symbol.reference strcat(methodName,"&");
				}
			}
		::strcat(methodName,")");
		}
}

/*******************************************************************************
        Return true if this method matches signature of method instance passed
        in. Does a fuzzy match to allow for null parameters or ellipsis.
        Assumes the names match already checks.
*******************************************************************************/
int Symbol::matchMethod(Instance *target)
{
DoubleLink 	*link = 0;
DoubleLink 	*targetLink = 0;
Symbol 		*arg = 0;
Instance 	*targetArg = 0;
SymbolType 	*targetType = 0;
int 		flag = 0;
	if ( !target || !isMethod || (parameters && !target->parameters) || (target->parameters && !parameters) )
		return 0;
	if ( !parameters && !target->parameters )
		return 1;
	link = parameters->first;
	targetLink = target->parameters->first;
	for ( ; link; link = link->next )
		{
		if ( !targetLink )
			break;
		arg = (Symbol*)link->value;
		if ( arg->type == SymbolType::nullType )
			return 1;
		// match on ellipsis
		targetArg = (Instance*)targetLink->value;
		targetType = targetArg->getType();
		if ( targetType != arg->type )
			flag = 2;
		if ( !arg->type->matches(targetType) )
			{
			if ( arg->type == SymbolType::voidType && arg->indirect == 1 || targetArg->isVoidPointer() )
				goto keepChecking;
			else
			if ( targetType != SymbolType::nullType )
				if ( arg->type == SymbolType::idType && (targetType->isOC || targetType == SymbolType::stringType) )
					goto keepChecking;
				else
				if ( targetType == SymbolType::idType && arg->type->isOC )
					goto keepChecking;
			break;
			}
keepChecking:
		targetLink = targetLink->next;
		}
	if ( !link && !targetLink )
		if ( !flag )
			return 1;
		else	return flag;
	return 0;
}

/*******************************************************************************
    Search parentClass and return its first component with a matching type
*******************************************************************************/
Symbol *Symbol::matchType(SymbolType *match)
{
Symbol 	*matched = 0;
	if ( parentClass && parentClass->components )
		{
		parentClass->components->hashList->entry = 0;
		while ( matched = (Symbol*)parentClass->components->hashList->next() )
			if ( matched->type == match )
				return matched;
		}
	return 0;
}

/*******************************************************************************
        Push the method parameters onto the symbol table passed in
*******************************************************************************/
void Symbol::pushParameters(InstanceTable *table)
{
DoubleLink 	*link = 0;
Symbol 		*symbol = 0;
Instance 	*instance = 0;
	if ( !parameters )
		return;
	for ( link = parameters->first; link; link = link->next )
		{
		symbol = (Symbol*)link->value;
		instance = new Instance(symbol);
		if ( symbol->isLambda )
			symbol->isAssigned = 1;
		table->add(instance);
		}
}

/*****************************************************************************
	Set indirection and reference levels
*****************************************************************************/
void Symbol::setIndirection(PLGitem *direct)
{
int 	i = 0;
int 	saveIndirect = indirect;
char 	*atChar = direct->itemStart;
	indirect = 0;
	reference = 0;
	for ( i = direct->itemLength; i; i--, atChar++ )
		if ( *atChar == '*' )
			indirect++;
		else
		if ( *atChar == '&' )
			reference++;
		else
		if ( *atChar == '^' )
			isLambda = 1;
	// The following maintains the normal indirection that the above code would wipe out
	if ( reference && saveIndirect && !indirect )
		indirect = 1;
}

/******************************************************************************
	Sets the isReferenced flag for its type and its qualifiers, if any
******************************************************************************/
void Symbol::setRefer()
{
	if ( referred )
		return;
	referred = 1;
	if ( isAlias && source && source->referred )
		return;
	if ( source && !source->referred )
		source->setRefer();
	if ( !type->isReferenced )
		type->setRefer();
	if ( parentClass && !parentClass->isReferenced )
		parentClass->setRefer();
	if ( isMethod && parameters )
		{
		Symbol 	*argument = 0;
		while ( argument = (Symbol*)parameters->next() )
			if ( argument->type->isReferenced )
				argument->referred = 1;
			else	argument->setRefer();
		}
}

/******************************************************************************
	Checks if the symbol type can be cast to the type passed in. void types
	are matches either way (since tok casts voids automagically).
******************************************************************************/
int Symbol::typeMatch(SymbolType *tYPE)
{
SymbolType 	*myType = 0;
	if ( tYPE == SymbolType::voidType )
		return 1;
	for ( myType = type; myType; myType = myType->parent )
		if ( myType->structure )
			break;
		else
		if ( myType == tYPE || myType == SymbolType::voidType )
			return 1;
	for ( myType = tYPE; myType; myType = myType->parent )
		if ( myType->structure )
			break;
		else
		if ( myType == type || myType == SymbolType::voidType )
			return 1;
	return 0;
}
