#include <string.h>
#include <stdio.h>
#include "StringRoutines.h"
#include "PLGparse.h"
#include "BaseHash.h"
#include "Types.h"
#include "Symbol.h"
#include "Stak.h"
#include "DoubleLinkList.h"
#include "DoubleLink.h"
#include "PLGitem.h"
#include "InstanceTable.h"
#include "Buffer.h"
#include "Instance.h"
#include "Tawk.h"
#include "Tok.h"
#include "SymbolType.h"
Types *SymbolType::types;
SymbolType *SymbolType::buttonType;
SymbolType *SymbolType::charType;
SymbolType *SymbolType::doubleType;
SymbolType *SymbolType::floatType;
SymbolType *SymbolType::globalType;
SymbolType *SymbolType::idType;
SymbolType *SymbolType::internalType;
SymbolType *SymbolType::intType;
SymbolType *SymbolType::longType;
SymbolType *SymbolType::nullType;
SymbolType *SymbolType::ocRoutines;
SymbolType *SymbolType::ocStringType;
SymbolType *SymbolType::pointerType;
SymbolType *SymbolType::selectorType;
SymbolType *SymbolType::shortType;
SymbolType *SymbolType::stringRoutines;
SymbolType *SymbolType::stringType;
SymbolType *SymbolType::voidType;
BaseHash *SymbolType::ocSymbols;
DoubleLinkList *SymbolType::globalList;

/******************************************************************************
	Constructor
******************************************************************************/
SymbolType::SymbolType(char *n)
{
	comment = 0;
	constructor = 0;
	dotHname = 0;
	nameSpace = 0;
	addClassNameToMethods = 0;
	autoGetSet = 0;
	classOK = 0;
	composed = 0;
	debug = 0;
	hasDescendentTypes = 0;
	hasExtern = 0;
	hasInitializer = 0;
	hasLambda = 0;
	hasMethods = 0;
	hasOC = 0;
	hasTypedef = 0;
	isAliasType = 0;
	isAtomic = 0;
	isC = 0;
	isChar = 0;
	isDeclared = 0;
	isDirect = 0;
	isExternal = 0;
	isFilled = 0;
	isFlagged = 0;
	isGlobal = 0;
	isLocal = 0;
	isNumber = 0;
	isOC = 0;
	isReferenced = 0;
	isTemplate = 0;
	isVirtuous = 0;
	mustDeclare = 0;
	nameLess = 0;
	noClassForward = 0;
	noDotH = 0;
	noSign = 0;
	proper = 0;
	structure = 0;
	typesFilled = 0;
	components = 0;
	methods = 0;
	componentTypes = 0;
	descendentTypes = 0;
	overloads = 0;
	protocols = 0;
	sortedComponents = 0;
	sortedMethods = 0;
	codeBuffer = 0;
	initializer = 0;
	parent = 0;
	lastBitOffset = 0;
	lastOffset = 0;
	symbolMapped = 0;
	typeIndex = 0;
	typeSize = 0;
	format = 0;
	aliasStack = 0;
	name = n;
	levelMax = 4;
	componentFields = new BaseHash();
}

/******************************************************************************
	Copy constructor (used for typedefs)
******************************************************************************/
SymbolType::SymbolType(SymbolType *type)
{
	name = 0;
	comment = 0;
	constructor = 0;
	dotHname = 0;
	nameSpace = 0;
	addClassNameToMethods = 0;
	autoGetSet = 0;
	classOK = 0;
	composed = 0;
	debug = 0;
	hasDescendentTypes = 0;
	hasExtern = 0;
	hasInitializer = 0;
	hasLambda = 0;
	hasMethods = 0;
	hasOC = 0;
	hasTypedef = 0;
	isAliasType = 0;
	isAtomic = 0;
	isC = 0;
	isChar = 0;
	isDeclared = 0;
	isDirect = 0;
	isExternal = 0;
	isFilled = 0;
	isFlagged = 0;
	isGlobal = 0;
	isLocal = 0;
	isNumber = 0;
	isOC = 0;
	isReferenced = 0;
	isTemplate = 0;
	isVirtuous = 0;
	mustDeclare = 0;
	nameLess = 0;
	noClassForward = 0;
	noDotH = 0;
	noSign = 0;
	proper = 0;
	structure = 0;
	typesFilled = 0;
	components = 0;
	componentFields = 0;
	methods = 0;
	componentTypes = 0;
	descendentTypes = 0;
	overloads = 0;
	protocols = 0;
	sortedComponents = 0;
	sortedMethods = 0;
	codeBuffer = 0;
	initializer = 0;
	parent = 0;
	lastBitOffset = 0;
	lastOffset = 0;
	levelMax = 0;
	symbolMapped = 0;
	typeIndex = 0;
	typeSize = 0;
	format = 0;
	aliasStack = 0;
	*this = *type;
}

/******************************************************************************
	Add a symbol to this type
******************************************************************************/
void SymbolType::add(Symbol *item)
{
Symbol 	*previous = getLocal(item->name);
	if ( isType(item->type->structure) )
		hasTypedef = 1;
	if ( !hasOC && item->type->isOC )
		hasOC = 1;
	if ( previous )
		if ( previous->parentClass != this )
			previous = 0;
		else {
			if ( previous->isMethod && item->isMethod && ::compare(previous->methodName,item->methodName) == 0 )
				return;
			if ( !previous->isMethod && !item->isMethod )
				return;
			}
	item->parentClass = this;
	if ( isGlobal )
		Tok::tawking->currentSymbols->addGlobalField(item->name,item);
	if ( !isType(structure) )
		{
		item->isProper = proper;
		item->isOCfield = isOC;
		}
	if ( item->isMethod )
		{
		addMethod(item);
		if ( !item->reference )
			return;
		}
	if ( !components )
		components = new BaseHash();
	//cout "Adding " item.name,item.displayName() " to " name "\n";
	components->add(item->name,item);
	addComponentType(item);
	if ( item->isProper )
		item->makeSetter();
	if ( !item->isAlias )
		{
		item->symbolOffset = lastOffset;
		if ( !item->isItem )
			if ( !item->indirect )
				lastOffset += item->type->typeSize;
			else	lastOffset += SymbolType::pointerType->typeSize;
		else
		if ( !item->source )
			{
			int 	wordLength = SymbolType::longType->typeSize << 3;
			if ( item->symbolBitLength )
				lastBitOffset += item->symbolBitLength;
			item->symbolBitOffset = lastBitOffset;
			if ( lastBitOffset >= wordLength )
				{
				if ( lastBitOffset > wordLength )
					::fprintf(stderr,"WARNING: %s field overlaps word boundary\n",item->name);
				lastOffset += SymbolType::longType->typeSize;
				lastBitOffset = 0;
				}
			//cout "SymbolType.add: class",name,item.name,symbolOffset,symbolBitOffset:;
			}
		if ( isOC )
			{
			if ( !SymbolType::ocSymbols )
				SymbolType::ocSymbols = new BaseHash();
			SymbolType::ocSymbols->add(item->name,(void*)item);
			}
		}
}

/******************************************************************************
	Create a new symbol and add it to this type
******************************************************************************/
Symbol *SymbolType::add(char *item, SymbolType *type)
{
Symbol 	*symbol = 0;
	if ( components && (symbol = (Symbol*)components->get(item)) )
		return symbol;
	if ( methods && (symbol = (Symbol*)methods->get(item)) )
		return symbol;
	symbol = new Symbol(item,type);
	add(symbol);
	return symbol;
}

/******************************************************************************
	Create a new symbol and add it to this type
******************************************************************************/
Symbol *SymbolType::add(char *item, char *t)
{
SymbolType 	*type = SymbolType::types->getType(t);
	return add(item,type);
}

/******************************************************************************
	Adds descendent types to componentTypes
******************************************************************************/
void SymbolType::addAncestorTypes()
{
Instance 	*descendent = 0;
Instance 	*matched = 0;
Instance 	*instance = 0;
SymbolType 	*ancestorType = parent;
SymbolType 	*type = 0;
	// It may end up not true but setting hasDescendentTypes will prevent re-entry here
	hasDescendentTypes = 1;
	while ( ancestorType )
		{
		Symbol 	*symbol = 0;
		if ( ancestorType->components )
			{
			ancestorType->components->hashList->resetIterator();
			while ( symbol = (Symbol*)ancestorType->components->hashList->next() )
				addComponentType(symbol);
			}
		if ( ancestorType->methods )
			{
			ancestorType->methods->hashList->resetIterator();
			while ( symbol = (Symbol*)ancestorType->methods->hashList->next() )
				{
				addComponentField(symbol->name,symbol);
				addComponentField(symbol->methodName,symbol);
				}
			}
		ancestorType = ancestorType->parent;
		}
	if ( !componentTypes )
		return;
	componentTypes->resetIterator();
	while ( instance = (Instance*)componentTypes->next() )
		{
		type = instance->getType();
		if ( type == this || type->isAtomic || !type->componentTypes || type->nameLess )
			continue;
		if ( type->structure && type->structure < 5 )
			continue;
		if ( !type->hasDescendentTypes )
			type->addAncestorTypes();
		type->componentTypes->resetIterator();
		while ( descendent = (Instance*)type->componentTypes->next() )
			{
			ancestorType = descendent->getType();
			if ( ancestorType == this )
				continue;
			matched = (Instance*)componentTypes->get(ancestorType->name);
			if ( !matched || matched->level > instance->level + descendent->level )
				{
				instance = new Instance(instance);
				descendent = new Instance(descendent);
				descendent->setParent(instance);
				if ( !matched )
					componentTypes->add(ancestorType->name,(void*)descendent);
				else	componentTypes->put(ancestorType->name,(void*)descendent);
				}
			}
		}
}

/******************************************************************************
	Add the symbol passed in to the componentFields list.
******************************************************************************/
void SymbolType::addComponentField(char *text, Symbol *item)
{
	if ( componentFields->get(text) )
		return;
	else {
		Instance 	*instance = new Instance(item);
		componentFields->add(text,instance);
		Tok::tawking->componentCount++;
		}
}

/******************************************************************************
	Add the symbol passed in to the componentTypes list if appropriate. It
    gets added as an instance because we need to consider the hierarchy when
    adding descendent types
******************************************************************************/
void SymbolType::addComponentType(Symbol *item)
{
SymbolType 	*type = item->type;
Instance 	*instance = new Instance(item);
	if ( !componentTypes )
		componentTypes = new DoubleLinkList();
	componentFields->put(item->name,instance);
	if ( item->isAlias || isAtomic || item->isStatic || isUnion(structure) || item->indirect > 1 || type->isAtomic || item->typeMatch(this) || item->isMethod || type->nameLess )
		return;
	componentTypes->put(type->name,instance);
}

/******************************************************************************
	Add a method to this type method list.
******************************************************************************/
void SymbolType::addMethod(Symbol *method)
{
Symbol 	*item = 0;
Symbol 	*tHIS = 0;
	if ( !method->reference )
		hasMethods = 1;
	if ( !methods )
		methods = new BaseHash();
	checkGetterSetter(method);
	method->parentClass = this;
	if ( !isType(structure) )
		method->isOCfield = isOC;
	method->isMethod = 1;
	//cout "Adding method " method.name,method.displayName(),methodName " to " name "\n";
	if ( isOC )
		{
		if ( !SymbolType::ocSymbols )
			SymbolType::ocSymbols = new BaseHash();
		SymbolType::ocSymbols->add(method->name,(void*)method);
		}
	if ( isType(method->type->structure) )
		hasTypedef = 1;
	if ( !hasOC && method->type->isOC )
		hasOC = 1;
	/**************************************************************************
	Check parameters for OC types
	**************************************************************************/
	if ( !hasOC && method->parameters )
		while ( item = (Symbol*)method->parameters->next() )
			if ( item->type->isOC )
				hasOC = 1;
	methods->add(method->name,method);
	addComponentField(method->name,method);
	if ( isGlobal )
		Tok::tawking->currentSymbols->addGlobalField(method->name,method);
	if ( isC && !method->reference && !method->isAlias )
		{
		/**********************************************************************
		Insert tHIS parameter.
		**********************************************************************/
		if ( tHIS = getLocal("tHIS") )
			if ( method->parameters )
				method->insertParameter(tHIS);
			else	method->addParameter(tHIS);
		else	::fprintf(stderr,"addMethod: could not find tHIS\n");
		//cout `"Adding C method: " method.name " isC":;
		}
	if ( !method->isAlias && (method->reference || method->isLambda) )
		{
		method->symbolOffset = lastOffset;
		if ( !method->isItem )
			lastOffset += SymbolType::pointerType->typeSize;
		}
	if ( !method->methodName )
		method->gitMethodName();
	methods->put(method->methodName,method);
	addComponentField(method->methodName,method);
	if ( isGlobal )
		Tok::tawking->currentSymbols->addGlobalField(method->methodName,method);
	//cout "Adding method: " method.name,methodName:;
	if ( isOC && method->parameters && !method->isAlias )
		{
		char 	*ocName = method->getOCmethodName();
		//cout `"Adding OC method: " ocName " isOC":;
		methods->put(ocName,method);
		if ( isGlobal )
			Tok::tawking->currentSymbols->addGlobalField(ocName,method);
		}
}

/******************************************************************************
	Add a protocol to this type (if type is OC).
******************************************************************************/
void SymbolType::addProtocol(SymbolType *p)
{
	if ( !isOC )
		::fprintf(stderr,"Can only specify protocol for objective-C class\n");
	else {
		if ( !protocols )
			{
			protocols = new DoubleLinkList();
			protocols->hasKeys = 1;
			}
		if ( !isProtocol(p->structure) )
			::fprintf(stderr,"AddProtocol: %s is not a protocol\n",p->name);
		else
		if ( !protocols->get(p->name) )
			{
			protocols->add(p->name,(void*)p);
			if ( !isExternal )
				p->setRefer();
			}
		}
}

/******************************************************************************
	Check if method is a setter or getter
******************************************************************************/
void SymbolType::checkGetterSetter(Symbol *method)
{
char 	first = 0;
char 	*text = 0;
char 	*target = 0;
Symbol 	*symbol = 0;
	if ( !method || method->isSetter || method->isGetter || (!isOC && !getAutoGetSet()) )
		return;
	if ( (int)::strlen(method->name) <= 3 )
		return;
	if ( !::strncmp(method->name,"get",3) || !::strncmp(method->name,"set",3) )
		{
		text = method->name + 3;
		if ( components )
			{
			symbol = (Symbol*)components->get(text);
			if ( !symbol )
				{
				first = *text;
				if ( first < 'A' || first > 'Z' )
					return;
				else	*text += 32;
				::asprintf(&target,"%s",text);
				*text = first;
				symbol = findAliasTarget(target);
				}
			}
		if ( symbol )
			if ( *method->name == 'g' && (method->parameters == 0 || isC && method->parameters->length == 1) && !(method->type == SymbolType::voidType && method->indirect == 0) )
				{
				if ( symbol->type == method->type )
					{
					symbol->getter = method;
					method->isGetter = 1;
					method->getter = symbol;
					}
				}
			else
			if ( *method->name == 's' && method->parameters && (method->parameters->length == 1 || (isC && method->parameters->length == 2)) && method->type == SymbolType::voidType )
				{
				symbol->setter = method;
				method->isSetter = 1;
				method->setter = symbol;
				}
		}
}

/******************************************************************************
	Debugging routine to list out the components and methods of this type
******************************************************************************/
void SymbolType::dump()
{
Symbol 		*symbol = 0;
DoubleLink 	*link = 0;
	::printf("%s\n",name);
	::printf("%s Component map\n",name);
	if ( !components )
		::printf("\thas no components\n");
	else {
		/**********************************************************************
		Move the bottom comment marker to comment out/uncomment next 4 lines
		**********************************************************************/
		::printf("\tComponents\n");
		components->hashList->resetIterator();
		while ( symbol = (Symbol*)components->hashList->next() )
			dumpSymbol(symbol);
		}
	if ( methods )
		{
		::printf("\tMethods\n");
		methods->hashList->resetIterator();
		while ( link = methods->hashList->nextLink() )
			{
			symbol = (Symbol*)link->value;
			// No need to display duplicate entry
			if ( ::compare(link->key,symbol->methodName) == 0 )
				continue;
			if ( symbol->isOCfield && !symbol->isAlias && ::compare(link->key,symbol->getOCmethodName()) != 0 )
				continue;
			if ( symbol->isAlias )
				::printf("\t\t%s %s %s",link->key,symbol->methodName,symbol->source->methodName);
			else	::printf("\t\t%s %s",link->key,symbol->methodName);
			if ( symbol->isExtension )
				::printf(" extension ");
			if ( symbol->isAlias )
				::printf(" alias");
			::printf(" %hd\n",symbol->symbolIndex);
			}
		}
	if ( componentTypes )
		{
		Instance 	*instance = 0;
		::printf("\tComponentTypes\n");
		componentTypes->resetIterator();
		while ( instance = (Instance*)componentTypes->next() )
			{
			symbol = instance->symbol;
			::printf("\t\t%-15s %s %hd\n",symbol->type->name,symbol->displayName(),symbol->symbolIndex);
			}
		}
	if ( overloads && overloads->length )
		{
		char 	*name = 0;
		overloads->resetIterator();
		::printf("\tOverload Table\n");
		while ( name = (char*)overloads->next() )
			::printf("\t\t%s %s\n",overloads->entry->key,name);
		}
}

/******************************************************************************
	Debugging routine to list out components
******************************************************************************/
void SymbolType::dumpFields()
{
Instance 	*instance = 0;
DoubleLink 	*link = 0;
	if ( componentFields )
		{
		::printf("Type: %s Component Fields\n",name);
		componentFields->hashList->resetIterator();
		while ( link = componentFields->hashList->nextLink() )
			if ( instance = (Instance*)link->value )
				::printf("\t%s\n",instance->getQualifiedName());
		}
	else	::printf("\tNo component fields\n");
}

/******************************************************************************
	Debugging symbol print
******************************************************************************/
void SymbolType::dumpSymbol(Symbol *symbol)
{
	::printf("\t\t%s",symbol->name);
	if ( symbol->isMethod )
		::printf(" %s",symbol->methodName);
	if ( symbol->isExtension )
		::printf(" extension ");
	if ( symbol->isHidden )
		::printf(" hidden");
	if ( symbol->isAlias )
		::printf(" alias");
	if ( symbol->getter )
		::printf(" has getter");
	if ( symbol->setter )
		::printf(" has setter");
	::printf("\n");
}

/******************************************************************************
	Creates and fills componentFields with instances matching the name passed in
******************************************************************************/
Instance *SymbolType::fillComponentFields(char *text)
{
SymbolType 	*type = 0;
Instance 	*ancestor = 0;
Instance 	*component = 0;
Instance 	*instance = 0;
	isFilled = 1;
	if ( !hasDescendentTypes )
		addAncestorTypes();
	if ( instance = (Instance*)componentFields->get(text) )
		if ( instance != Tok::tawking->currentSymbols->nullInstance )
			return instance;
		else	return 0;
	if ( componentTypes )
		{
		componentTypes->resetIterator();
		/**********************************************************************
		Add descendent fields to componentFields. Only want to keep the
		lowest level field that matches text.
		**********************************************************************/
		while ( ancestor = (Instance*)componentTypes->prior() )
			{
			type = ancestor->getType();
			component = (Instance*)type->componentFields->get(text);
			if ( !component && !type->isFilled )
				component = type->fillComponentFields(text);
			if ( component && component != Tok::tawking->currentSymbols->nullInstance )
				{
				ancestor = new Instance(ancestor);
				if ( instance )
					{
					if ( component->level + ancestor->level < instance->level )
						{
						instance->symbol = component->symbol;
						instance->setParent((Instance*)0);
						if ( component->parent )
							instance->setParent(component->parent);
						instance->setParent(ancestor);
						}
					}
				else {
					instance = new Instance(component);
					instance->setParent(ancestor);
					instance->isDeclaration = 0;
					componentFields->add(text,instance);
					Tok::tawking->componentCount++;
					}
				if ( instance->level == 2 )
					break;
				}
			}
		}
	if ( !instance )
		componentFields->add(text,Tok::tawking->currentSymbols->nullInstance);
	return instance;
}

/******************************************************************************
	Looks up the parameter name in types and returns what it finds.
******************************************************************************/
SymbolType *SymbolType::find(char *n)
{
	if ( !SymbolType::types )
		SymbolType::setTypeTable();
	return (SymbolType*)SymbolType::types->get(n);
}

/******************************************************************************
	Search components for the first field matching the name passed in.
    This only searches thru components it does not descend each component.
******************************************************************************/
Symbol *SymbolType::findAliasTarget(char *text)
{
Symbol 	*target = 0;
Symbol 	*symbol = 0;
	if ( !isAtomic )
		{
		SymbolType::types->resetIsFlagged();
		isFlagged = 1;
		if ( components )
			{
			if ( target = (Symbol*)components->get(text) )
				goto endFind;
			components->hashList->resetIterator();
			while ( symbol = (Symbol*)components->hashList->next() )
				if ( symbol->type->isFlagged )
					continue;
				else {
					SymbolType 	*type = symbol->type;
					type->isFlagged = 1;
					if ( type->components )
						if ( target = (Symbol*)type->components->get(text) )
							goto endFind;
					}
			}
		}
endFind:
	SymbolType::types->resetIsFlagged();
	return target;
}

/******************************************************************************
	Search components and methods for a field matching the name passed in
******************************************************************************/
Symbol *SymbolType::findField(char *text)
{
Symbol 	*symbol = 0;
	if ( !isAtomic )
		{
		if ( components )
			symbol = (Symbol*)components->get(text);
		if ( !symbol && methods )
			symbol = (Symbol*)methods->get(text);
		}
	return symbol;
}

/******************************************************************************
	Search component fields for a field matching the name passed in and return
    its containing instance
******************************************************************************/
Instance *SymbolType::findFieldInstance(char *text)
{
Instance 	*instance = 0;
	SymbolType::types->resetIsFilled();
	if ( !isAtomic )
		instance = fillComponentFields(text);
	return instance;
}

/******************************************************************************
	Search methods for a method matching the instance passed in and
    return the symbol found. This only looks at own methods
******************************************************************************/
Symbol *SymbolType::findMethod(Instance *source)
{
Symbol 	*method = 0;
char 	*text = 0;
	if ( !isAtomic && source )
		{
		if ( text = source->mangle() )
			if ( method = findField(text) )
				if ( method->isMethod )
					return method;
		if ( source->prefix )
			if ( method = findField(source->prefix) )
				if ( method->isMethod )
					{
					methods->hashList->resetIterator();
					while ( method = (Symbol*)methods->next(source->prefix) )
						if ( method->isMethod && method->matchMethod(source) )
							return method;
					}
		}
	return 0;
}

/******************************************************************************
	Search components for a method matching the instance passed in and
    return the instance found
******************************************************************************/
Instance *SymbolType::findMethodInstance(Instance *source)
{
char 		*text = 0;
Instance 	*field = 0;
	if ( !isAtomic && source )
		{
		if ( text = source->mangle() )
			{
			if ( field = findFieldInstance(text) )
				if ( field->isMethod )
					return field;
			}
		else
		if ( source->prefix )
			if ( field = findFieldInstance(source->prefix) )
				if ( field->isMethod )
					return field;
		}
	return field;
}

/******************************************************************************
	Search components for a field matching the item passed in
******************************************************************************/
Symbol *SymbolType::get(PLGitem *item)
{
Symbol 	*symbol = getLocal(item->string());
	item->unString();
	return symbol;
}

/******************************************************************************
	Returns true if autoGetSet true here or in ancestors
******************************************************************************/
unsigned int SymbolType::getAutoGetSet()
{
SymbolType 	*type = this;
	if ( autoGetSet || proper || isOC )
		return 1;
	while ( type = type->parent )
		if ( type->autoGetSet || type->proper )
			return 1;
	return 0;
}

/******************************************************************************
	Return a method that converts object of type source to an object of
	type target (if there is such a method)
******************************************************************************/
Symbol *SymbolType::getConverter(SymbolType *target, SymbolType *source)
{
Symbol 	*argument = 0;
Symbol 	*symbol = 0;
	//cout "Checking " name " to convert " target.name " from " source.name "\n";
	if ( methods )
		{
		methods->hashList->resetIterator();
		while ( symbol = (Symbol*)methods->hashList->next() )
			{
			if ( symbol->type != target || symbol->indirect > 1 )
				continue;
			if ( symbol->parameters && symbol->parameters->length == 1 )
				{
				argument = (Symbol*)symbol->parameters->first->value;
				if ( argument && argument->type == source && ((argument->indirect == 1 && !source->isAtomic) || (argument->indirect == 0 && source->isAtomic)) )
					break;
				}
			else
			if ( source == this && !symbol->parameters )
				break;
			}
		}
	return symbol;
}

/******************************************************************************
	Like findField but only considers symbols that belong directly to this type
	or parent type and embedded structures.
******************************************************************************/
Symbol *SymbolType::getLocal(char *name)
{
Symbol 	*part = 0;
Symbol 	*symbol = 0;
	if ( components )
		{
		symbol = (Symbol*)components->get(name);
		if ( !symbol && methods )
			symbol = (Symbol*)methods->get(name);
		if ( !symbol )
			{
			components->hashList->resetIterator();
			while ( part = (Symbol*)components->hashList->prior() )
				{
				if ( part->isHidden && part->type != this && !part->type->isAtomic && !part->isItem && !part->isMethod )
					symbol = part->type->getLocal(name);
				if ( symbol )
					break;
				}
			}
		}
	if ( !symbol && parent && !structure )
		return parent->getLocal(name);
	return symbol;
}

/******************************************************************************
	Like getLocal but only looks at methods. Does not look in embedded structures.
******************************************************************************/
Symbol *SymbolType::getMethod(char *name)
{
Symbol 	*current = 0;
	if ( methods )
		current = (Symbol*)methods->get(name);
	if ( !current && parent )
		current = parent->getMethod(name);
	return current;
}

/******************************************************************************
	Factory that returns a type. If the type exists in types, returns it,
    otherwise creates a new type, adds it to types then returns it.
******************************************************************************/
SymbolType *SymbolType::getType(char *n)
{
	if ( !SymbolType::types )
		SymbolType::setTypeTable();
	return SymbolType::types->getType(n);
}

/*******************************************************************************
        Process alias default parameters
*******************************************************************************/
Symbol *SymbolType::handleAliasParameters(Symbol *symbol, char *aliasName, PLGitem *body, Tawk *tok)
{
Symbol 		*arg = 0;
Symbol 		*symbolArg = 0;
Symbol 		*aliasSymbol = 0;
PLGitem 	*atBody = 0;
PLGitem 	*parameter = 0;
PLGitem 	*replacedBy = 0;
	if ( !aliasStack )
		aliasStack = new Stak();
	/***************************************************************************
	Make sure the cuffs and collars match
	***************************************************************************/
	if ( !symbol->parameters || (symbol->isExtension && symbol->parameters->length == 1) )
		return aliasSymbol;
	symbol->parameters->entry = 0;
	if ( symbol->isExtension )
		symbol->parameters->next();
	atBody = body;
	parameter = atBody->get("parameter");
	replacedBy = atBody->get("replacedBy");
	while ( symbolArg = (Symbol*)symbol->parameters->next() )
		{
		if ( symbolArg->isThis )
			continue;
		if ( !replacedBy )
			{
			if ( atBody = atBody->next )
				{
				parameter = atBody->get("parameter");
				replacedBy = atBody->get("replacedBy");
				}
			}
		else
		if ( parameter->compare(symbolArg->name) == 0 )
			if ( atBody = atBody->next )
				{
				parameter = atBody->get("parameter");
				replacedBy = atBody->get("replacedBy");
				}
		if ( !atBody )
			break;
		}
	/***************************************************************************
	If there is a body part left at this point, no match
	***************************************************************************/
	if ( atBody )
		return aliasSymbol;
	if ( aliasSymbol = symbol->makeAlias(aliasName) )
		{
		aliasSymbol->isDefault = 1;
		aliasSymbol->parameters = new DoubleLinkList();
		symbol->parameters->entry = 0;
		/***********************************************************************
		If not specified otherwise, default parameters replace
		parameters in the order they are encountered. Alternatively,
		a default parameter can specify which parameter it is replacing.
		Parameters that do not specify a replacement are substituted
		left to right.
		***********************************************************************/
		for ( atBody = body; atBody; atBody = atBody->next )
			{
			parameter = atBody->get("parameter");
			replacedBy = atBody->get("replacedBy");
			if ( !parameter )
				continue;
			while ( symbolArg = (Symbol*)symbol->parameters->next() )
				{
				if ( symbolArg->isThis )
					{
					aliasSymbol->addParameter(symbolArg);
					continue;
					}
				arg = 0;
				if ( !replacedBy )
					{
					arg = new Symbol(symbolArg);
					arg->comment = parameter->toString();
					}
				else
				if ( parameter->compare(symbolArg->name) == 0 )
					{
					arg = new Symbol(symbolArg);
					arg->comment = replacedBy->toString();
					}
				if ( arg )
					{
					arg->isDefault = 1;
					aliasSymbol->addParameter(arg);
					break;
					}
				else	aliasSymbol->addParameter(symbolArg);
				}
			}
		if ( symbol->parameters->entry )
			while ( arg = (Symbol*)symbol->parameters->next() )
				aliasSymbol->addParameter(arg);
		aliasSymbol->methodName = 0;
		aliasStack->push((void*)aliasSymbol);
		//cout ``"handleAliasParameters:",name, aliasSymbol.name,aliasSymbol.symbolIndex, aliasSymbol.gitMethodName(),aliasSymbol.displayName():;
		}
	return aliasSymbol;
}

/******************************************************************************
	Returns true if type is a parent of this
******************************************************************************/
int SymbolType::hasParent(SymbolType *type)
{
SymbolType 	*ancestor = parent;
	while ( ancestor )
		if ( ancestor == type )
			return 1;
		else	ancestor = ancestor->parent;
	return 0;
}

/*******************************************************************************
        Make an alias
*******************************************************************************/
void SymbolType::makeAlias(char *aliasName, char *target, PLGitem *body, Tawk *tok)
{
Symbol 	*symbol = 0;
Symbol 	*aliasMethod = 0;
Symbol 	*aliasSymbol = 0;
	if ( !body )
		{
		if ( components )
			symbol = (Symbol*)components->get(target);
		if ( symbol )
			{
			if ( aliasSymbol = symbol->makeAlias(aliasName) )
				add(aliasSymbol);
			}
		else
		if ( methods )
			while ( symbol = (Symbol*)methods->next(target) )
				if ( aliasSymbol = symbol->makeAlias(aliasName) )
					addMethod(aliasSymbol);
		goto finishMakeAlias;
		}
	if ( !methods )
		return;
	methods->hashList->resetIterator();
	while ( symbol = (Symbol*)methods->next(target) )
		if ( aliasMethod = handleAliasParameters(symbol,aliasName,body,tok) )
			aliasSymbol = aliasMethod;
finishMakeAlias:
	/***************************************************************************
	If alias not found, check globals and static fields
	***************************************************************************/
	if ( !aliasSymbol )
		{
		Instance 	*instance = (Instance*)tok->currentSymbols->globalFields->get(target);
		if ( instance )
			{
			symbol = instance->symbol;
			if ( aliasSymbol = symbol->makeAlias(aliasName) )
				{
				add(aliasSymbol);
				tok->currentSymbols->addGlobalField(aliasName,aliasSymbol);
				}
			}
		}
	if ( !aliasSymbol )
		::fprintf(stderr,"makeAlias failed for %s on %s\n",aliasName,target);
}

/******************************************************************************
	Returns true if the SymbolType parameter matches this type. Numeric types
	match and String and char types match, signed or unsigned.
******************************************************************************/
int SymbolType::matches(SymbolType *type)
{
SymbolType 	*current = 0;
SymbolType 	*target = 0;
	if ( type == this )
		return 1;
	if ( isChar && type->isChar )
		return 1;
	else
	if ( isNumber && type->isNumber )
		return 1;
	for ( current = this; current; current = current->parent )
		for ( target = type; target; target = target->parent )
			if ( current == target )
				return 1;
	return 0;
}

/******************************************************************************
	Adds methods that overload the associated op
******************************************************************************/
void SymbolType::overload(char *op, char *method)
{
	if ( !overloads )
		setOverloadTable();
	overloads->put(op,method);
}

/******************************************************************************
	If the op is overloaded, returns the name of the overloading method
******************************************************************************/
char *SymbolType::overloaded(char *op)
{
	if ( overloads )
		return (char*)overloads->get(op);
	return 0;
}

/******************************************************************************
	Only returns a method if the method belongs to this type
******************************************************************************/
Symbol *SymbolType::ownMethod(char *name)
{
Symbol 	*current = findField(name);
	if ( current && current->isMethod && current->parentClass == this )
		return current;
	return 0;
}

/******************************************************************************
	Creates the overload table, inheriting from parent, if parent exists
	and is overloaded
******************************************************************************/
void SymbolType::setOverloadTable()
{
	overloads = new DoubleLinkList();
	overloads->hasKeys = 1;
	if ( parent && parent->overloads )
		{
		DoubleLinkList 	*list = parent->overloads;
		list->resetIterator();
		while ( list->next() )
			overloads->put(list->entry->key,list->entry->value);
		}
}

/******************************************************************************
	Sets parent and adds this class to the parent descendents
******************************************************************************/
void SymbolType::setParent(SymbolType *type)
{
	if ( !type->descendentTypes )
		type->descendentTypes = new DoubleLinkList();
	type->descendentTypes->add((void*)this);
	isOC = type->isOC;
	parent = type;
}

/******************************************************************************
	Sets the isReferenced flag for this type and its ancestors and protocols
    (delegates) if any
******************************************************************************/
void SymbolType::setRefer()
{
SymbolType 	*type = 0;
	if ( !Tok::tawking->referring || isReferenced )
		return;
	isReferenced = 1;
	/**************************************************************************
	if this is a typedef, access type table to obtain the associated type
	**************************************************************************/
	if ( isType(structure) )
		if ( type = SymbolType::getType(name) )
			if ( type != this )
				type->setRefer();
	if ( parent )
		parent->setRefer();
	if ( protocols )
		while ( type = (SymbolType*)protocols->next() )
			type->setRefer();
}

/******************************************************************************
	Initializes the type types
******************************************************************************/
void SymbolType::setTypeTable()
{
	SymbolType::globalList = new DoubleLinkList();
	SymbolType::voidType = SymbolType::types->getType("void");
	SymbolType::voidType->isAtomic = 1;
	SymbolType::voidType->isDirect = 1;
	SymbolType::voidType->noDotH = 1;
	SymbolType::buttonType = SymbolType::types->getType("bUTTOn");
	SymbolType::buttonType->isAtomic = 1;
	SymbolType::buttonType->isNumber = 1;
	SymbolType::buttonType->isDirect = 1;
	SymbolType::buttonType->name = "unsigned int";
	SymbolType::buttonType->noDotH = 1;
	SymbolType::charType = SymbolType::types->getType("unsigned char");
	SymbolType::charType->isAtomic = 1;
	SymbolType::charType->isNumber = 1;
	SymbolType::charType->isChar = 1;
	SymbolType::charType->isDirect = 1;
	SymbolType::charType->format = Tok::tawking->getInstance("%u");
	SymbolType::charType->noDotH = 1;
	SymbolType::charType->noSign = 1;
	SymbolType::charType->typeSize = sizeof(char);
	SymbolType::charType = SymbolType::types->getType("char");
	SymbolType::charType->isAtomic = 1;
	SymbolType::charType->isNumber = 1;
	SymbolType::charType->isChar = 1;
	SymbolType::charType->isDirect = 1;
	SymbolType::charType->format = Tok::tawking->getInstance("%c");
	SymbolType::charType->noDotH = 1;
	SymbolType::charType->typeSize = sizeof(char);
	SymbolType::shortType = SymbolType::types->getType("unsigned short");
	SymbolType::shortType->isAtomic = 1;
	SymbolType::shortType->isNumber = 1;
	SymbolType::shortType->isDirect = 1;
	SymbolType::shortType->format = Tok::tawking->getInstance("%hu");
	SymbolType::shortType->noDotH = 1;
	SymbolType::shortType->noSign = 1;
	SymbolType::shortType->typeSize = sizeof(short);
	SymbolType::shortType = SymbolType::types->getType("short");
	SymbolType::shortType->isAtomic = 1;
	SymbolType::shortType->isNumber = 1;
	SymbolType::shortType->isDirect = 1;
	SymbolType::shortType->format = Tok::tawking->getInstance("%hd");
	SymbolType::shortType->noDotH = 1;
	SymbolType::shortType->typeSize = sizeof(short);
	SymbolType::intType = SymbolType::types->getType("unsigned int");
	SymbolType::intType->isAtomic = 1;
	SymbolType::intType->isNumber = 1;
	SymbolType::intType->isDirect = 1;
	SymbolType::intType->format = Tok::tawking->getInstance("%u");
	SymbolType::intType->noDotH = 1;
	SymbolType::intType->noSign = 1;
	SymbolType::intType->typeSize = sizeof(int);
	SymbolType::intType = SymbolType::types->getType("int");
	SymbolType::intType->isAtomic = 1;
	SymbolType::intType->isNumber = 1;
	SymbolType::intType->isDirect = 1;
	SymbolType::intType->format = Tok::tawking->getInstance("%d");
	SymbolType::intType->noDotH = 1;
	SymbolType::intType->typeSize = sizeof(int);
	SymbolType::longType = SymbolType::types->getType("unsigned long");
	SymbolType::longType->isAtomic = 1;
	SymbolType::longType->isNumber = 1;
	SymbolType::longType->isDirect = 1;
	SymbolType::longType->format = Tok::tawking->getInstance("%lu");
	SymbolType::longType->noSign = 1;
	SymbolType::longType->typeSize = sizeof(long);
	SymbolType::pointerType = SymbolType::longType;
	SymbolType::longType = SymbolType::types->getType("long");
	SymbolType::longType->isAtomic = 1;
	SymbolType::longType->isNumber = 1;
	SymbolType::longType->isDirect = 1;
	SymbolType::longType->format = Tok::tawking->getInstance("%ld");
	SymbolType::longType->noDotH = 1;
	SymbolType::longType->typeSize = sizeof(long);
	SymbolType::doubleType = SymbolType::types->getType("float");
	SymbolType::doubleType->isAtomic = 1;
	SymbolType::doubleType->isNumber = 1;
	SymbolType::doubleType->isDirect = 1;
	SymbolType::doubleType->format = Tok::tawking->getInstance("%.0f");
	SymbolType::doubleType->noDotH = 1;
	SymbolType::doubleType->typeSize = sizeof(float);
	SymbolType::doubleType = SymbolType::types->getType("double");
	SymbolType::doubleType->isAtomic = 1;
	SymbolType::doubleType->isNumber = 1;
	SymbolType::doubleType->isDirect = 1;
	SymbolType::doubleType->format = Tok::tawking->getInstance("%.0f");
	SymbolType::doubleType->noDotH = 1;
	SymbolType::doubleType->typeSize = sizeof(double);
	SymbolType::idType = SymbolType::types->getType("id");
	SymbolType::idType->isOC = 1;
	SymbolType::idType->isAtomic = 1;
	SymbolType::idType->isDirect = 1;
	SymbolType::idType->noDotH = 1;
	SymbolType::internalType = SymbolType::types->getType("internal");
	SymbolType::internalType->isExternal = 1;
	SymbolType::internalType->isGlobal = 1;
	SymbolType::internalType->noDotH = 1;
	SymbolType::globalList->add((void*)SymbolType::internalType);
	SymbolType::globalType = SymbolType::types->getType("globals");
	SymbolType::globalType->isExternal = 1;
	SymbolType::globalType->isGlobal = 1;
	SymbolType::globalType->noDotH = 1;
	SymbolType::globalList->add((void*)SymbolType::globalType);
	SymbolType::nullType = SymbolType::types->getType("null");
	SymbolType::nullType->isAtomic = 1;
	SymbolType::nullType->noDotH = 1;
	SymbolType::nullType->format = Tok::tawking->getInstance("%s");
	SymbolType::ocRoutines = SymbolType::types->getType("OCroutines");
	SymbolType::ocStringType = SymbolType::types->getType("NSString");
	SymbolType::ocStringType->isOC = 1;
	SymbolType::ocStringType->isChar = 1;
	SymbolType::ocStringType->dotHname = "Cocoa/Cocoa.h";
	SymbolType::stringType = SymbolType::types->getType("String");
	SymbolType::stringType->name = "char";
	SymbolType::stringType->format = Tok::tawking->getInstance("%s");
	SymbolType::stringType->noDotH = 1;
	SymbolType::stringType->isChar = 1;
	SymbolType::stringType->isAtomic = 1;
	SymbolType::stringType->comment = "String";
	SymbolType::stringRoutines = SymbolType::types->getType("StringRoutines");
	SymbolType::selectorType = SymbolType::types->getType("SEL");
	SymbolType::selectorType->isOC = 1;
	SymbolType::selectorType->isAtomic = 1;
	SymbolType::selectorType->isDirect = 1;
	SymbolType::selectorType->dotHname = "Cocoa/Cocoa.h";
}
