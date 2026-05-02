#include <string.h>
#include <stdio.h>
#include "StringRoutines.h"
#include "SearchTree.h"
#include "Buffer.h"
#include "PLGparse.h"
#include "BaseHash.h"
#include "Types.h"
#include "Symbol.h"
#include "Directive.h"
#include "Stak.h"
#include "SymbolType.h"
#include "Operate.h"
#include "DoubleLinkList.h"
#include "DoubleLink.h"
#include "PLGitem.h"
#include "InstanceTable.h"
#include "BlockTok.h"
#include "Expression.h"
#include "Instance.h"
#include "Statement.h"
#include "Tawk.h"
#include "Tok.h"
#include "FormatC.h"

FormatC::FormatC()
{
	errorBuffer = 0;
	stringEncoder = 0;
	currentNameSpace = 0;
	filename = 0;
	currentType = 0;
	ocStringConverter = 0;
	enclosingMethod = 0;
	suppressedDeclarations = 0;
	pendingDirective = 0;
	endDirective = 0;
	startDirective = 0;
	argumentIsOC = 0;
	jitting = 0;
	makeOCfile = 0;
	processingGlobalMethods = 0;
	writingParameters = 0;
	forwardBuffer = ::bufferFactory2("forward");
	headerBuffer = ::bufferFactory2("header");
	includeText = ::bufferFactory2("include");
	junkBuffer = ::bufferFactory2("junk");
	staticBlock = new BlockTok();
	structBuffer = ::bufferFactory2("struct");
	mStak = new Stak();
	buffer = junkBuffer;
}

/*******************************************************************************
        Use mStak to stash referenced namespaces (used when creating forward
        class references)
*******************************************************************************/
void FormatC::addNameSpace(char *name)
{
char 	*space = 0;
	mStak->entry = 0;
	while ( space = (char*)mStak->next() )
		if ( ::compare(space,name) == 0 )
			return;
	//cout "Adding namespace: " name:;
	mStak->push(name);
}

/*******************************************************************************
        Set isInitialized
*******************************************************************************/
void FormatC::checkInitialize(Instance *instance)
{
Symbol 	*symbol = 0;
	if ( !instance )
		return;
	if ( instance->statement )
		checkStatement(instance->statement);
	else
	if ( instance->express && instance->express->verb && instance->express->verb->compare("=") == 0 )
		{
		symbol = instance->express->subject->getSymbol();
		if ( symbol && !symbol->isStatic && !symbol->isInitialized && !symbol->isMethod )
			symbol->isInitialized = 1;
		}
}

/*******************************************************************************
        Check that sortedComponents and sortedMethods have been set
*******************************************************************************/
void FormatC::checkSort(SymbolType *type)
{
DoubleLinkList 	*list = 0;
DoubleLink 		*newLink = 0;
DoubleLink 		*link = 0;
	/**************************************************************************
	The following is done in order to have components and methods
	declared in the order they were entered rather than in hash order
	**************************************************************************/
	if ( type->components && !type->sortedComponents )
		{
		list = new DoubleLinkList();
		list->hasKeys = 1;
		list->isSorted = 1;
		type->components->hashList->resetIterator();
		while ( link = type->components->hashList->nextLink() )
			list->addInLinkOrder(link);
		type->sortedComponents = list;
		}
	// Need to strip out the duplicate methodname entries
	if ( type->methods && !type->sortedMethods )
		{
		list = new DoubleLinkList();
		list->hasKeys = 1;
		list->isSorted = 1;
		type->methods->hashList->resetIterator();
		while ( link = type->methods->hashList->nextLink() )
			{
			Symbol 	*method = (Symbol*)link->value;
			if ( link->key != method->name )
				continue;
			else	newLink = list->add(link->key,link->value);
			}
		type->sortedMethods = list;
		}
}

/*******************************************************************************
        Set isInitialized
*******************************************************************************/
void FormatC::checkStatement(Statement *statement)
{
	if ( statement->statementType != NOTSPECIFIED )
		return;
	checkInitialize(statement->first);
	if ( statement->second )
		{
		checkInitialize(statement->second);
		if ( statement->third )
			{
			checkInitialize(statement->third);
			if ( statement->fourth )
				checkInitialize(statement->fourth);
			}
		}
}

/*******************************************************************************
        Make sure referenced typedef include files get referenced
*******************************************************************************/
void FormatC::checkTypedefs()
{
SymbolType 	*type = 0;
Symbol 		*field = 0;
	while ( type = (SymbolType*)SymbolType::types->hashList->next() )
		{
		if ( !type->isReferenced || !type->hasTypedef )
			continue;
		checkSort(type);
		if ( type->sortedComponents )
			{
			type->sortedComponents->resetIterator();
			while ( field = (Symbol*)type->sortedComponents->next() )
				if ( isType(field->type->structure) )
					field->setRefer();
			}
		if ( type->sortedMethods )
			{
			type->sortedMethods->resetIterator();
			while ( field = (Symbol*)type->sortedMethods->next() )
				{
				if ( field->isHidden )
					continue;
				if ( isType(field->type->structure) )
					field->setRefer();
				}
			}
		}
}

/*****************************************************************************
	Close the formatter and print the formatted files
*****************************************************************************/
void FormatC::close()
{
	forwardBuffer->appendString(headerBuffer->string());
	headerBuffer->reset();
	forwardBuffer->closeFile();
	if ( buffer != forwardBuffer && buffer->file )
		buffer->closeFile();
	if ( suppressedDeclarations )
		::printf("Declarations ignored because not used: %d\n",suppressedDeclarations);
}

/*****************************************************************************
	Inserts a cast when void* is involved in simple assignments and
	munges comparison expressions
*****************************************************************************/
void FormatC::convert(Expression *express)
{
Symbol 		*method = 0;
Symbol 		*toString = 0;
SymbolType 	*subjectType = 0;
SymbolType 	*objectType = 0;
Operate 	*isComparison = 0;
Instance 	*right = 0;
Instance 	*left = 0;
Instance 	*string = 0;
int 		objectDirect = 0;
int 		subjectDirect = 0;
	if ( !express->subject || !express->object || express->subject->isError || express->object->isError || !express->verb )
		return;
	subjectType = express->subject->sourceType();
	objectType = express->object->sourceType();
	objectDirect = express->object->howDirect();
	subjectDirect = express->subject->howDirect();
	if ( express->verb->compare("=") == 0 )
		{
		if ( subjectType->isOC && (objectType == SymbolType::stringType && (objectDirect == 1 || express->object->isConstant)) )
			express->object = makeOCstring(express->object);
		if ( (subjectType == SymbolType::stringType && (subjectDirect == 1 || express->subject->isConstant)) && !(objectType == SymbolType::stringType && (objectDirect == 1 || express->object->isConstant)) && !objectType->isNumber )
			{
			if ( subjectDirect != 1 )
				goto checkStringCast;
			toString = objectType->getMethod("toString");
			if ( !toString )
				goto checkStringCast;
			string = new Instance(toString);
			if ( toString->isExtension )
				{
				string->symbol = toString->source;
				string->addParameter(express->object);
				}
			else	string->setParent(express->object);
			express->object = string;
			objectType = toString->type;
			}
checkStringCast:
		if ( !express->subject->isLambda && (express->subject->isVoidPointer() || express->object->isVoidPointer()) && !(express->subject->symbol && express->subject->symbol->setter && !express->subject->symbol->isSetter) && (subjectType != objectType || subjectDirect != objectDirect || express->subject->isMethod != express->object->isMethod) )
			{
			express->object = express->object->checkCast(express->subject);
			return;
			}
		}
	else
	if ( express->verb->comparison && subjectType->isChar && objectType->isChar )
		{
		// Convenience fix for string/char mismatch
		if ( subjectType == SymbolType::stringType && objectType == SymbolType::charType && subjectDirect == 1 && !objectDirect )
			{
			express->subject = new Instance(express->subject);
			express->subject->indirection = 1;
			}
		else
		if ( subjectType == SymbolType::charType && objectType == SymbolType::stringType && !subjectDirect && objectDirect == 1 )
			{
			express->object = new Instance(express->object);
			express->object->indirection = 1;
			}
		}
	/*************************************************************************
	Checks comparison expressions to handle string compares and
	insertion of compare methods
	*************************************************************************/
	if ( express->verb->call )
		isComparison = (Operate*)Expression::CompareOperators->get(express->verb->op);
	if ( isComparison )
		{
		express->verb = isComparison;
		right = Tok::tawking->getInstance("0");
		right->isConstant = 1;
		right->type = SymbolType::intType;
		if ( !subjectType->isAtomic )
			method = express->subject->getCompareMethod(express->object);
		if ( !method )
			{
			if ( !objectType->isAtomic )
				method = express->object->getCompareMethod(express->subject);
			if ( method )
				{
				left = new Instance(method);
				left->addParameter(express->subject);
				left->setParent(express->object);
				}
			}
		else {
			left = new Instance(method);
			left->addParameter(express->object);
			left->setParent(express->subject);
			}
		if ( !method && ((subjectType == SymbolType::stringType && (subjectDirect == 1 || express->subject->isConstant)) || (objectType == SymbolType::stringType && (objectDirect == 1 || express->object->isConstant))) )
			if ( subjectType != SymbolType::stringType )
				{
				if ( subjectDirect != 1 )
					goto bail;
				toString = subjectType->ownMethod("toString()");
				if ( !toString )
					goto bail;
				string = new Instance(toString);
				string->setParent(express->subject);
				express->subject = string;
				subjectType = SymbolType::stringType;
				}
			else
			if ( objectType != SymbolType::stringType )
				{
				if ( objectDirect != 1 )
					goto bail;
				toString = objectType->ownMethod("toString()");
				if ( !toString )
					goto bail;
				string = new Instance(toString);
				string->setParent(express->object);
				express->object = string;
				objectType = SymbolType::stringType;
				}
			else {
				method = new Symbol("compare",SymbolType::intType);
				SymbolType::stringRoutines->setRefer();
				method->isMethod = 1;
				method->parentClass = SymbolType::globalType;
				left = new Instance(method);
				left->addParameter(express->subject);
				left->addParameter(express->object);
				}
bail:
		if ( method )
			{
			express->subject = left;
			express->object = right;
			}
		}
}

/*******************************************************************************
        Called when declaring static expressions
*******************************************************************************/
void FormatC::declare(BlockTok *block)
{
DoubleLink 	*line = 0;
Instance 	*instance = 0;
	if ( !block->statements )
		return;
	for ( line = block->statements->first; line; line = line->next )
		{
		instance = (Instance*)line->value;
		instance->statement->first->isDeclaration = 1;
		declare(instance->statement->first,0);
		buffer->appendString(";\n");
		}
}

/*******************************************************************************
	Write out a declaration for this symbol. If the flag is set, the declaration
	is for a .h file, otherwise it is a declaration appropriate for a method
	parameter, Does not write out the trailing ;
*******************************************************************************/
int FormatC::declare(Symbol *symbol, int flag, int width)
{
int 	j = 0;
int 	isString = 0;
char 	*atArray = 0;
Buffer 	*saveBuffer = 0;
	symbol->setRefer();
	if ( flag && symbol->isStatic )
		buffer->appendString("static ");
	if ( symbol->type->structure )
		{
		/**********************************************************************
		Declare enums, unions, booleans and structs
		**********************************************************************/
		if ( symbol->type->structure && !symbol->type->isDeclared && !isType(symbol->type->structure) && !isProtocol(symbol->type->structure) )
			{
			saveBuffer = buffer;
			if ( symbol->isHidden && !(isStruct(symbol->parentClass->structure) || isUnion(symbol->parentClass->structure)) )
				buffer = headerBuffer;
			else	buffer = structBuffer;
			declareStructure(symbol->type);
			buffer = saveBuffer;
			}
		if ( !symbol->isHidden && !symbol->isItem )
			goto normal;
		return 0;
		}
	else
	if ( symbol->isMethod )
		{
		writeSignature(symbol,0);
		return 1;
		}
	else {
		/**********************************************************************
		Declare regular variables, arrays
		**********************************************************************/
normal:
		if ( !symbol->type )
			{
			buffer->appendString("missing type for ");
			buffer->appendString(symbol->name);
			return 1;
			}
		if ( !(symbol->isItem && isEnumerator(symbol->parentClass->structure)) )
			{
			if ( symbol->isConst )
				buffer->appendString("const ");
			if ( symbol->isOutlet && currentType->isOC )
				buffer->appendString("IBOutlet ");
			if ( symbol->type->nameSpace )
				{
				buffer->appendString(symbol->type->nameSpace);
				buffer->appendString("::");
				}
			if ( symbol->type == SymbolType::stringType )
				{
				isString = 1;
				buffer->appendString("char ");
				}
			else
			if ( symbol->type != SymbolType::nullType )
				if ( symbol->isItem && symbol->structType && isBoolean(symbol->structType->structure) && currentType->isOC )
					buffer->appendString("BOOL ");
				else {
					buffer->appendString(symbol->type->name);
					buffer->appendString(" ");
					}
			for ( j = width - (1 + (int)::strlen(symbol->type->name)) / 4; j > 0; j-- )
				buffer->appendString("\t");
			if ( symbol->indirect )
				for ( j = symbol->indirect - symbol->isArray; j > 0; j-- )
					buffer->appendString("*");
			else
			if ( !symbol->type->isDirect && !symbol->isConstructor )
				buffer->appendString(::toStringFromChar('*'));
			for ( j = symbol->reference; j; j-- )
				buffer->appendString(::toStringFromChar('&'));
			if ( !flag && symbol->isStatic && !currentType->isOC )
				{
				buffer->appendString(symbol->parentClass->name);
				buffer->appendString("::");
				}
			}
		buffer->appendString(symbol->name);
		if ( !symbol->isArray && flag && symbol->array && !currentType->isOC )
			buffer->appendString(symbol->array);
		else
		if ( symbol->isArray && symbol->array )
			if ( flag )
				buffer->appendString(symbol->array);
			else
			for ( atArray = symbol->array; *atArray; atArray++ )
				if ( *atArray == '[' )
					buffer->appendString("[]");
		return 1;
		}
}

/*******************************************************************************
        Write out a declaration for this instance
*******************************************************************************/
void FormatC::declare(Instance *instance, int forDotH)
{
int 	width = 0;
	if ( !instance->isDeclaration )
		{
		writeInstance(instance);
		return;
		}
	if ( instance->block )
		width = instance->block->getWidth();
	if ( instance->symbol && !instance->symbol->isAlias )
		{
		/***********************************************************************
		Write out warning if declaration is not used
		***********************************************************************/
		if ( instance->isLocal && !instance->symbol->isUsed )
			{
			Buffer 	*saveBuffer = buffer;
			if ( !errorBuffer )
				errorBuffer = ::bufferFactory2("errors");
			buffer = errorBuffer;
			buffer->appendString("// Ignoring declaration of unused variable");
			buffer->appendString(" ");
			if ( enclosingMethod )
				{
				buffer->appendString(instance->getSymbol()->name);
				buffer->appendString(" ");
				buffer->appendString("in method:");
				buffer->appendString(" ");
				buffer->appendString(enclosingMethod->methodName);
				buffer->appendString("\n");
				}
			else {
				buffer->appendString(instance->getSymbol()->name);
				buffer->appendString("\n");
				}
			suppressedDeclarations++;
			buffer = saveBuffer;
			instance->isComment = 1;
			}
		else {
			declare(instance->symbol,forDotH,width);
			if ( instance->symbol->isConstructor && instance->parameters )
				{
				writeParameters(instance);
				// clean out converting constructor stuff
				enclosingMethod->isConstructor = 0;
				instance->isMethod = 0;
				instance->parameters = 0;
				}
			}
		}
	else
	if ( instance->express )
		declareTail(instance,forDotH);
	instance->isDeclaration = 0;
}

/*******************************************************************************
        Write out a declaration for this expression
*******************************************************************************/
void FormatC::declare(Expression *expression)
{
int 	j = 0;
	expression->checkExpression(this);
	if ( expression->subject )
		declare(expression->subject,0);
	if ( expression->object )
		{
		if ( expression->verb )
			if ( expression->verb->compare(",") == 0 )
				{
				if ( expression->subject && expression->subject->symbol )
					buffer->appendString(" = 0,\n");
				else	buffer->appendString(",\n");
				if ( expression->object->block )
					for ( j = expression->object->block->width; j > 0; j-- )
						buffer->appendString("\t");
				expression->object->isDeclaration = 1;
				writeIndirect(expression->object);
				expression->object->isDeclaration = 0;
				}
			else {
				buffer->appendString(" ");
				buffer->appendString(expression->verb->op);
				buffer->appendString(" ");
				}
		declareTail(expression->object,1);
		}
}

/******************************************************************************
	Print out class body
******************************************************************************/
void FormatC::declareBody(SymbolType *type)
{
Symbol 		*field = 0;
Buffer 		*saveBuffer = buffer;
DoubleLink 	*entry = 0;
char 		*codeName = ::concat(2,type->name,"code");
	//cout "Declare class body for " type.name:;
	if ( !type->codeBuffer )
		type->codeBuffer = ::bufferFactory2(codeName);
	buffer = type->codeBuffer;
	checkSort(type);
	if ( type->sortedComponents )
		while ( field = (Symbol*)type->sortedComponents->next(entry) )
			{
			field->setRefer();
			if ( field->isStatic && !field->isMethod && !field->isInitialized )
				{
				declare(field,0,0);
				buffer->appendString(";\n");
				}
			}
	if ( type->sortedMethods )
		{
		entry = 0;
		while ( field = (Symbol*)type->sortedMethods->next(entry) )
			{
			if ( field->isHidden )
				continue;
			field->setRefer();
			if ( field->isAlias )
				continue;
			int j = 0;
			if ( ::compare(field->name,"lazyGetField") == 0 )
				j = 0;
			writeSignature(field,1);
			}
		}
	buffer = saveBuffer;
}

/******************************************************************************
	Write out the files needed to implement this type as a class.
	Note that the header and code PrintBuffers are
	not closed here so that globals can be added to the files
******************************************************************************/
void FormatC::declareClass(SymbolType *type)
{
Buffer 		*saveBuffer = 0;
SymbolType 	*structType = 0;
char 		*codeName = ::concat(2,type->name,"code");
	if ( type->isExternal || type->isAtomic || isType(type->structure) )
		return;
	checkSort(type);
	/**************************************************************************
	If protocol, write the protocol declaration
	**************************************************************************/
	if ( isProtocol(type->structure) && !type->isExternal )
		{
		saveBuffer = buffer;
		buffer = headerBuffer;
		buffer->appendString("@protocol ");
		buffer->appendString(type->name);
		buffer->appendString("\n");
		if ( type->sortedMethods )
			declareMethods(type,0);
		buffer->appendString("@end\n");
		buffer = saveBuffer;
		return;
		}
	if ( !filename )
		{
		::fprintf(stderr,"No file name set to write out type: %s\n",type->name);
		return;
		}
	if ( type->structure )
		{
		saveBuffer = buffer;
		buffer = structBuffer;
		declareStructure(type);
		buffer = saveBuffer;
		return;
		}
	type->setRefer();
	currentType = type;
	if ( !(ocStringConverter && stringEncoder) )
		{
		SymbolType 	*tYPE = (SymbolType*)SymbolType::types->get("NSString");
		if ( tYPE )
			{
			Instance 	*instance = (Instance*)Tok::tawking->currentSymbols->globalFields->get("encoder");
			if ( instance )
				{
				ocStringConverter = tYPE->getMethod("ocString");
				stringEncoder = instance;
				}
			}
		}
	/**************************************************************************
	Write headerBuffer
	**************************************************************************/
	buffer = headerBuffer;
	forwardClass(type);
	if ( type->isOC )
		{
		if ( includeText->length() > 0 )
			{
			buffer->appendString(includeText->string());
			includeText->reset();
			}
		if ( type->comment )
			buffer->appendString(type->comment);
		buffer->appendString("@interface ");
		buffer->appendString(type->name);
		if ( type->parent )
			{
			buffer->appendString(" : ");
			buffer->appendString(type->parent->name);
			}
		else	buffer->appendString(" : NSObject");
		if ( type->protocols )
			{
			int 	flag = 0;
			type->protocols->entry = 0;
			buffer->appendString(" <");
			while ( structType = (SymbolType*)type->protocols->next() )
				{
				if ( flag )
					buffer->appendString(",");
				else	flag = 1;
				buffer->appendString(structType->name);
				}
			buffer->appendString(">");
			}
		buffer->appendString("\n{\n@public\n");
		}
	else {
		if ( type->comment )
			{
			buffer->appendString(type->comment);
			buffer->appendString("\n");
			}
		buffer->appendString("class ");
		buffer->appendString(type->name);
		if ( type->parent )
			{
			buffer->appendString(" : public ");
			buffer->appendString(type->parent->name);
			}
		buffer->appendString("\n{\npublic:\n");
		}
	while ( structType = (SymbolType*)SymbolType::types->hashList->next() )
		if ( structType->mustDeclare )
			declareStructure(structType);
	declareHeaders(type);
	if ( !type->isC )
		if ( type->isOC )
			buffer->appendString("@end\n");
		else	buffer->appendString("};\n");
	saveBuffer = buffer;
	buffer = forwardBuffer;
	while ( currentNameSpace = (char*)mStak->pop() )
		{
		buffer->appendString("namespace ");
		buffer->appendString(currentNameSpace);
		buffer->appendString(" {");
		buffer->appendString("\n");
		while ( structType = (SymbolType*)SymbolType::types->hashList->next() )
			if ( structType->isReferenced && structType->nameSpace && ::compare(structType->nameSpace,currentNameSpace) == 0 )
				{
				int 	save = structType->noClassForward;
				structType->noClassForward = 0;
				flagType(structType);
				structType->noClassForward = save;
				}
		buffer->appendString("}");
		buffer->appendString("\n");
		}
	buffer = saveBuffer;
	if ( structBuffer->length() )
		{
		forwardBuffer->appendString(structBuffer->string());
		forwardBuffer->appendString("\n");
		structBuffer->reset();
		}
	forwardBuffer->setFile(type->dotHname);
	/**************************************************************************
	Write codeBuffer
	**************************************************************************/
	if ( !type->codeBuffer )
		type->codeBuffer = ::bufferFactory2(codeName);
	buffer = type->codeBuffer;
	if ( type->isOC )
		{
		buffer->appendString("\n@implementation ");
		buffer->appendString(type->name);
		buffer->appendString("\n");
		}
	if ( staticBlock->length() )
		{
		declare(staticBlock);
		staticBlock->statements->clear();
		}
	declareBody(type);
	if ( type->isOC )
		buffer->appendString("@end\n");
}

/******************************************************************************
	Print out the component headers in a format appropriate to a header file
******************************************************************************/
void FormatC::declareHeaders(SymbolType *type)
{
Symbol 	*field = 0;
	checkSort(type);
	if ( type->sortedComponents )
		{
		type->sortedComponents->resetIterator();
		while ( field = (Symbol*)type->sortedComponents->next() )
			{
			if ( field->isAlias || field->isVirtual || (field->isHidden && (field->type == SymbolType::buttonType || field->type->isExternal || !(field->isItem || field->type->structure))) )
				continue;
			if ( field->isItem && (!type->structure || field->isHidden) )
				continue;
			if ( field->isItem || type->structure )
				::indent(BlockTok::indentCount,"\t",buffer);
			if ( declare(field,1,0) )
				{
				if ( !(field->isItem && isEnumerator(field->parentClass->structure)) )
					buffer->appendString(";");
				else
				if ( type->sortedComponents->entry->next )
					buffer->appendString(",");
				buffer->appendString("\n");
				}
			}
		}
	if ( type->isOC )
		buffer->appendString("}\n");
	else
	if ( type->isC )
		buffer->appendString("};\n");
	if ( type->sortedMethods )
		if ( type->isC )
			{
			buffer->appendString("extern \"C\"\n{\n");
			declareMethods(type,0);
			buffer->appendString("}\n");
			}
		else
		if ( type->hasExtern )
			{
			declareMethods(type,1);
			buffer->appendString("extern \"C\"\n{\n");
			declareMethods(type,2);
			buffer->appendString("}\n");
			}
		else	declareMethods(type,0);
}

/******************************************************************************
	Print out method declarations in a format appropriate to a header file
******************************************************************************/
void FormatC::declareMethods(SymbolType *type, int flag)
{
Symbol 	*field = 0;
	type->sortedMethods->resetIterator();
	while ( field = (Symbol*)type->sortedMethods->next() )
		{
		// exclude methodName entries
		if ( field->reference || field->isHidden )
			continue;
		if ( flag == 1 && field->isExtern )
			continue;
		else
		if ( flag == 2 && !field->isExtern )
			continue;
		if ( field->isAlias || field->isHidden )
			continue;
		if ( field->isVirtual )
			buffer->appendString("virtual ");
		else
		if ( field->isInline )
			buffer->appendString("inline ");
		if ( type->structure )
			::indent(BlockTok::indentCount,"\t",buffer);
		writeSignature(field,0);
		buffer->appendString(";\n");
		}
}

/******************************************************************************
	Print out a structure declaration
******************************************************************************/
void FormatC::declareStructure(SymbolType *type)
{
Symbol 	*field = 0;
	if ( type->isDeclared || type->isExternal )
		return;
	type->isDeclared = 1;
	checkSort(type);
	forwardClass(type);
	if ( isBoolean(type->structure) && currentType->isOC )
		{
		declareHeaders(type);
		return;
		}
	if ( type->sortedComponents )
		{
		type->sortedComponents->resetIterator();
		while ( field = (Symbol*)type->sortedComponents->next() )
			if ( field->isAlias )
				continue;
			else	flagType(field->type);
		}
	if ( BlockTok::indentCount > 1 )
		::indent(BlockTok::indentCount,"\t",buffer);
	if ( isBoolean(type->structure) || isStruct(type->structure) )
		buffer->appendString("struct ");
	else
	if ( isEnumerator(type->structure) )
		buffer->appendString("enum ");
	else	buffer->appendString("union ");
	if ( type->nameLess )
		buffer->appendString("\n");
	else {
		buffer->appendString(type->name);
		buffer->appendString("\n");
		}
	BlockTok::indentCount++;
	::indent(BlockTok::indentCount,"\t",buffer);
	buffer->appendString("{\n");
	declareHeaders(type);
	::indent(BlockTok::indentCount,"\t",buffer);
	buffer->appendString("};\n");
	if ( isBoolean(type->structure) )
		while ( field = (Symbol*)type->components->hashList->next() )
			if ( field->isButton )
				{
				buffer->appendString("#define");
				buffer->appendString(" ");
				buffer->appendString(field->name);
				buffer->appendString("(button) (button == ");
				buffer->appendString(field->array);
				buffer->appendString(")");
				buffer->appendString("\n");
				}
	BlockTok::indentCount--;
}

/*******************************************************************************
        Write out the tail end of an instance declaration expression
*******************************************************************************/
void FormatC::declareTail(Instance *instance, int flag)
{
	if ( instance->express )
		{
		instance->express->checkExpression(this);
		if ( instance->cast )
			writeInstance(instance->cast);
		if ( instance->resolved && !instance->express )
			writeInstance(instance);
		else {
			if ( instance->express->subject )
				if ( instance->express->subject->express )
					declareTail(instance->express->subject,flag);
				else {
					instance->express->subject->isDeclaration = 1;
					declare(instance->express->subject,flag);
					}
			if ( instance->express->object )
				{
				if ( instance->express->verb && instance->express->verb->compare(",") == 0 )
					{
					instance->express->object->isDeclaration = 1;
					if ( !instance->express->subject->isComment )
						if ( instance->express->subject->symbol && ((!instance->express->subject->isMethod && !instance->express->subject->symbol->type->structure) || instance->express->subject->howDirect()) )
							{
							buffer->appendString(" = 0;");
							buffer->appendString("\n");
							}
						else {
							buffer->appendString(";");
							buffer->appendString("\n");
							}
					writeInstance(instance->express->object);
					}
				else
				if ( !instance->express->subject->isComment )
					{
					if ( instance->express->verb )
						{
						buffer->appendString(" ");
						buffer->appendString(instance->express->verb->op);
						buffer->appendString(" ");
						}
					writeInstance(instance->express->object);
					}
				else	instance->isComment = 1;
				}
			}
		}
	else	writeInstance(instance);
}

/*******************************************************************************
        print out the class declarations in the .h file
*******************************************************************************/
void FormatC::flagType(SymbolType *type)
{
	if ( type->nameSpace && !currentNameSpace )
		addNameSpace(type->nameSpace);
	if ( type->noClassForward || type->isAtomic || type->structure || ::compare(type->name,currentType->name) == 0 )
		return;
	if ( type->isOC )
		{
		buffer->appendString("@class ");
		buffer->appendString(type->name);
		buffer->appendString(";");
		buffer->appendString("\n");
		if ( ::compare(type->dotHname,"Cocoa/Cocoa.h") == 0 )
			{
			type->noClassForward = 1;
			return;
			}
		}
	else {
		if ( type->nameSpace && ::compare(currentNameSpace,type->nameSpace) != 0 )
			return;
		if ( type->isTemplate )
			buffer->appendString("template<> ");
		buffer->appendString("class ");
		buffer->appendString(type->name);
		buffer->appendString(";");
		buffer->appendString("\n");
		}
	type->noClassForward = 1;
}

/*******************************************************************************
        Write out class forward declarations
*******************************************************************************/
void FormatC::forwardClass(SymbolType *type)
{
Symbol 	*argument = 0;
Symbol 	*field = 0;
Buffer 	*saveBuffer = buffer;
	checkSort(type);
	buffer = forwardBuffer;
	if ( type->sortedComponents )
		{
		type->sortedComponents->resetIterator();
		while ( field = (Symbol*)type->sortedComponents->next() )
			if ( field->isAlias )
				continue;
			else	flagType(field->type);
		}
	if ( type->sortedMethods )
		{
		type->sortedMethods->resetIterator();
		while ( field = (Symbol*)type->sortedMethods->next() )
			{
			if ( field->isAlias || field->isHidden )
				continue;
			flagType(field->type);
			if ( field->parameters )
				while ( argument = (Symbol*)field->parameters->next() )
					flagType(argument->type);
			}
		}
	buffer = saveBuffer;
}

/*******************************************************************************
        Write out indent
*******************************************************************************/
void FormatC::indent()
{
int 	i = 0;
	for ( i = BlockTok::indentCount; i > 0; i-- )
		buffer->appendString("\t");
}

/*******************************************************************************
        Handle indentation for a multi-line comment
*******************************************************************************/
void FormatC::indentComment(Instance *instance)
{
int 	length = (int)::strlen(instance->prefix);
char 	*atText = 0;
int 	flag = 0;
	for ( atText = instance->prefix; *atText; atText++ )
		if ( *atText == '\n' )
			length += BlockTok::indentCount;
	buffer->extend(++length);
	for ( atText = instance->prefix; *atText; atText++ )
		{
		if ( flag )
			switch (*atText)
				{
				case ' ':
				case '\t':
					continue;
				default:
					flag = 0;
				}
		*buffer->current++ = *atText;
		if ( *atText == '\n' )
			{
			flag++;
			::indent(BlockTok::indentCount,"\t",buffer);
			}
		}
	buffer->appendString("\n");
}

/*******************************************************************************
        Generate the initialization statement
*******************************************************************************/
Instance *FormatC::initialStatement(Symbol *symbol)
{
Statement 	*statement = 0;
Instance 	*instance = 0;
Instance 	*nullInstance = Tok::tawking->getInstance("0");
Expression 	*expression = 0;
	nullInstance->type = SymbolType::intType;
	nullInstance->isConstant = 1;
	instance = new Instance(symbol);
	expression = new Expression(instance,new Instance(nullInstance),"=");
	statement = new Statement();
	statement->add(expression);
	instance = new Instance(statement);
	return instance;
}

/*******************************************************************************
        Add initializations to constructor
*******************************************************************************/
void FormatC::initialize(Symbol *method)
{
	if ( !method || !method->block || !method->isMethod || method->isInitialized )
		return;
	method->isInitialized = 1;
SymbolType *type = method->parentClass;
Statement *statement = 0;
Symbol *symbol = 0;
Instance *instance = 0;
	method->block->statements->entry = 0;
	while ( instance = (Instance*)method->block->statements->next() )
		{
		statement = instance->statement;
		checkStatement(statement);
		}
	checkSort(type);
	if ( type->sortedComponents )
		{
		type->sortedComponents->resetIterator();
		while ( symbol = (Symbol*)type->sortedComponents->prior() )
			{
			if ( symbol->isStatic || (symbol->isHidden && !symbol->isItem) || symbol->isAlias || symbol->isMethod )
				continue;
			if ( symbol->isItem && !symbol->symbolBitLength && (isEnumerator(symbol->parentClass->structure) || symbol->array) )
				continue;
			if ( symbol->indirect == 0 && symbol->type->structure )
				continue;
			if ( !symbol->isInitialized )
				method->block->insert(initialStatement(symbol));
			else	symbol->isInitialized = 0;
			}
		}
	if ( type->sortedMethods )
		{
		type->sortedMethods->resetIterator();
		while ( symbol = (Symbol*)type->sortedMethods->next() )
			{
			if ( symbol->isStatic || symbol->isHidden || symbol->isAlias || ::compare(type->sortedMethods->entry->key,symbol->methodName) == 0 )
				continue;
			if ( symbol->reference )
				if ( !symbol->isInitialized )
					method->block->insert(initialStatement(symbol));
				else	symbol->isInitialized = 0;
			}
		}
}

/*******************************************************************************
        Output the instance as a string
*******************************************************************************/
Instance *FormatC::makeOCstring(Instance *instance)
{
Instance 	*string = 0;
	if ( instance->isConstant || (instance->express && !instance->express->verb && instance->express->subject->isConstant) )
		instance->atString = 1;
	else
	if ( ocStringConverter && stringEncoder )
		{
		string = new Instance(ocStringConverter);
		string->addParameter(instance);
		string->addParameter(stringEncoder);
		return string;
		}
	return instance;
}

/******************************************************************************
	Loops thru types and prints include statements if the type has been referenced.
******************************************************************************/
void FormatC::printCode()
{
SymbolType 	*ancestor = 0;
SymbolType 	*type = 0;
char 		*includeString = 0;
char 		*cocoaString = "Cocoa/Cocoa.h";
Buffer 		*saveBuffer = buffer;
SearchTree 	*tree = new SearchTree();
	if ( !currentType->isOC && includeText->length() > 0 )
		{
		buffer->appendString(includeText->string());
		includeText->reset();
		}
	buffer = includeText;
	if ( currentType->isOC )
		includeString = "#import ";
	else	includeString = "#include ";
	/**************************************************************************
	Force reference to string.h and stdio.h.
	**************************************************************************/
	type = SymbolType::getType("notTYPEstring");
	if ( !type->dotHname )
		type->dotHname = "string.h";
	type->isGlobal = 1;
	type->setRefer();
	type->isLocal = 1;
	type = SymbolType::getType("notTYPEstdio");
	if ( !type->dotHname )
		type->dotHname = "stdio.h";
	type->isGlobal = 1;
	type->setRefer();
	type->isLocal = 1;
	SymbolType::types->hashList->entry = 0;
	checkTypedefs();
	while ( type = (SymbolType*)SymbolType::types->hashList->next() )
		{
		if ( type->isAtomic || type->noDotH )
			{
			if ( type->isReferenced )
				type->isReferenced = 0;
			continue;
			}
		if ( type->isReferenced )
			{
			if ( !makeOCfile && (type->isOC || type->hasOC) )
				{
				buffer->appendString(includeString);
				buffer->appendString("<");
				buffer->appendString(cocoaString);
				buffer->appendString(">");
				buffer->appendString("\n");
				tree->add(cocoaString);
				makeOCfile = 1;
				}
			if ( type->dotHname && !tree->find(type->dotHname) && type->isLocal )
				{
				buffer->appendString(includeString);
				buffer->appendString("<");
				buffer->appendString(type->dotHname);
				buffer->appendString(">\n");
				tree->add(type->dotHname);
				type->isReferenced = 0;
				}
			}
		}
	if ( makeOCfile )
		{
		SymbolType::ocRoutines->setRefer();
		SymbolType::ocStringType->setRefer();
		}
	while ( type = (SymbolType*)SymbolType::types->hashList->next() )
		{
		if ( isProtocol(type->structure) )
			continue;
		if ( type->isReferenced && type != currentType )
			{
			type->isReferenced = 0;
			if ( !type->dotHname )
				type->dotHname = ::concat(2,type->name,".h");
			if ( !tree->find(type->dotHname) )
				{
				ancestor = type->parent;
				while ( ancestor )
					{
					if ( ancestor->isReferenced )
						{
						ancestor->isReferenced = 0;
						if ( !ancestor->noDotH && ancestor != SymbolType::globalType )
							{
							buffer->appendString(includeString);
							buffer->appendString("\"");
							buffer->appendString(ancestor->dotHname);
							buffer->appendString("\"\n");
							tree->add(ancestor->dotHname);
							}
						}
					ancestor = ancestor->parent;
					}
				buffer->appendString(includeString);
				buffer->appendString("\"");
				buffer->appendString(type->dotHname);
				buffer->appendString("\"\n");
				tree->add(type->dotHname);
				}
			}
		}
	if ( !tree->find(currentType->dotHname) )
		{
		buffer->appendString(includeString);
		buffer->appendString("\"");
		buffer->appendString(currentType->dotHname);
		buffer->appendString("\"\n");
		}
	if ( junkBuffer->length() )
		buffer->appendString(junkBuffer->string());
	junkBuffer->reset();
	buffer = saveBuffer;
}

//&& !(parent.symbol && parent.symbol.isHidden)
void FormatC::printQualified(Instance *instance)
{
char 	*qualifiedName = 0;
int 	ocFlag = 0;
int 	ocParent = 0;
int 	ocMethodRef = 0;
	if ( instance->express )
		{
		if ( instance->parent )
			{
			if ( instance->parent->indirection )
				{
				buffer->appendString("(");
				writeIndirect(instance->parent);
				}
			printQualified(instance->parent);
			if ( instance->parent->indirection )
				buffer->appendString(")");
			buffer->appendString("->");
			}
		writeExpression(instance->express);
		return;
		}
	else
	if ( !instance->symbol )
		{
		if ( instance->prefix && instance->parent && instance->parent->getType()->isOC )
			{
			buffer->appendString("[");
			if ( instance->parent )
				{
				printQualified(instance->parent);
				buffer->appendString(" ");
				}
			buffer->appendString(instance->prefix);
			buffer->appendString("]");
			}
		else
		if ( instance->type )
			if ( instance->type->isOC )
				buffer->appendString(instance->type->name);
			else {
				buffer->appendString(instance->type->name);
				buffer->appendString("::");
				}
		else {
			instance->error("Expected a symbol");
			writeInstance(instance);
			}
		return;
		}
	qualifiedName = instance->symbol->name;
	if ( !instance->indirection && !instance->noGetter && instance->symbol->getter && !instance->symbol->isGetter && !enclosingMethod->isGetter && !enclosingMethod->isConstructor )
		{
		Symbol 		*method = instance->symbol->getter;
		Instance 	*getterParent = instance->findGetterOrSetter(method->methodName);
		Instance 	*argument = 0;
		if ( instance->symbol->isHidden && !instance->symbol->isItem )
			argument = Tok::tawking->getInstance(instance->symbol->name);
		// Will not come here for OC properties where getter not set (no need)
		instance = new Instance(instance);
		instance->symbol = instance->symbol->getter;
		instance->isMethod = 1;
		if ( argument )
			instance->addParameter(argument);
		if ( getterParent )
			{
			instance->setParent((Instance*)0);
			instance->setParent(getterParent);
			}
		else
		if ( instance->symbol->parentClass == enclosingMethod->parentClass )
			instance->setParent((Instance*)0);
		}
	instance->checkSymbol();
	if ( instance->parent )
		ocParent = instance->parent->instanceOC();
	if ( !instance->symbol->isStatic && instance->symbol->isOCfield && instance->isNew && instance->symbol->type->isOC )
		{
		buffer->appendString("[[");
		buffer->appendString(instance->symbol->type->name);
		buffer->appendString(" alloc] ");
		ocFlag = 1;
		}
	if ( !argumentIsOC && !instance->symbol->isStatic && instance->symbol->isMethod && instance->symbol->parentClass && instance->symbol->parentClass->isGlobal && !instance->parent )
		buffer->appendString("::");
	if ( instance->parent && !(instance->symbol->isItem && isEnumerator(instance->symbol->parentClass->structure)) && !(instance->parent->type && instance->symbol->isStatic) )
		{
		if ( instance->symbol->isProper || instance->symbol->isMethod && instance->symbol->isOCfield )
			{
			ocFlag = 1;
			buffer->appendString("[");
			}
		if ( instance->parent->indirection )
			{
			buffer->appendString("(");
			writeIndirect(instance->parent);
			}
		if ( instance->parent->cast )
			writeInstance(instance->parent->cast);
		if ( instance->parent->symbol && instance->parent->symbol->getter && !instance->parent->symbol->isGetter && !instance->parent->noGetter )
			printQualified(instance->parent);
		else	printQualified(instance->parent->castAlias());
		if ( instance->parent->indirection )
			buffer->appendString(")");
		if ( instance->symbol->isOCfield && (instance->symbol->isMethod || instance->symbol->isProper) )
			buffer->appendString(" ");
		else
		if ( instance->parent->howDirect() )
			buffer->appendString("->");
		else
		if ( !instance->parent->type )
			buffer->appendString(".");
		}
	if ( !instance->qualified() )
		if ( instance->symbol->isOCfield && !isType(instance->symbol->parentClass->structure) && !instance->isNew && !instance->symbol->isStatic && !instance->symbol->isVirtual && !instance->symbol->parentClass->isGlobal && ((instance->isMethod && !instance->reference) || instance->symbol->parentClass->proper) )
			{
			ocFlag = 1;
			buffer->appendString("[self ");
			}
		else
		if ( currentType->isC && (instance->symbol->parentClass == currentType || (instance->symbol->parentClass && instance->symbol->parentClass->hasParent(currentType))) && (!instance->isMethod || (instance->isMethod && instance->reference)) && !instance->symbol->isThis && !instance->symbol->isStatic )
			buffer->appendString("tHIS->");
	ocMethodRef = instance->isMethod && instance->reference && instance->instanceOC();
	if ( printQualified(instance,ocParent,ocMethodRef) )
		ocFlag = 1;
	if ( instance->isMethod || instance->parameters )
		{
		if ( ocFlag && instance->isMethod && instance->parameters )
			buffer->appendString(":");
		writeParameters(instance);
		}
	if ( ocFlag )
		buffer->appendString("]");
}

/*******************************************************************************
        Print the qualified name of a symbol
*******************************************************************************/
int FormatC::printQualified(Instance *instance, int ocParent, int ocMethodRef)
{
int 	ocFlag = 1;
Symbol 	*symbol = instance->symbol;
	while ( symbol->source && !symbol->isButton )
		symbol = symbol->source;
	if ( symbol->isStatic && symbol->parentClass )
		if ( symbol->parentClass->isOC )
			{
			buffer->appendString("[");
			buffer->appendString(symbol->parentClass->name);
			buffer->appendString(" ");
			buffer->appendString(symbol->name);
			}
		else {
			buffer->appendString(symbol->parentClass->name);
			buffer->appendString("::");
			buffer->appendString(symbol->name);
			ocFlag = 0;
			}
	else {
		ocFlag = 0;
		if ( ocMethodRef )
			buffer->appendString(symbol->getOCmethodName());
		else	buffer->appendString(symbol->name);
		if ( symbol->isMethod && !symbol->reference && symbol->parentClass && symbol->parentClass->addClassNameToMethods )
			buffer->appendString(symbol->parentClass->name);
		}
	if ( ocFlag )
		if ( !symbol->parameters )
			buffer->appendString("]");
		else	return 1;
	return 0;
}

/*******************************************************************************
        Output the instance as a string
*******************************************************************************/
char *FormatC::toString(Instance *instance)
{
Buffer 	*saveBuffer = buffer;
char 	*text = 0;
	buffer = junkBuffer;
	writeInstance(instance);
	text = buffer->toString();
	junkBuffer->reset();
	buffer = saveBuffer;
	return text;
}

/*******************************************************************************
        Write a block
*******************************************************************************/
void FormatC::writeBlock(BlockTok *block)
{
DoubleLink 	*line = 0;
Instance 	*instance = 0;
int 		isLabel = 0;
Statement 	*stating = 0;
Directive 	*directive = 0;
	if ( block->isArrayInitializer )
		buffer->appendString("{");
	else
	if ( block->isBlock )
		{
		buffer->appendString("{\n");
		if ( BlockTok::indentCount == 0 )
			BlockTok::indentCount++;
		block->indenting = 1;
		}
	else	block->indenting = 0;
	if ( block->statements )
		{
		for ( line = block->statements->first; line; line = line->next )
			{
			instance = (Instance*)line->value;
			stating = instance->statement;
			isLabel = instance->isLabel || (instance->statement && instance->statement->first && instance->statement->first->isLabel);
			if ( block->isSwitch && !isLabel )
				BlockTok::indentCount++;
			if ( !line->next && stating->statementType == RETURN )
				if ( block->isMethodBlock && enclosingMethod->directives && endDirective )
					{
					directive = endDirective;
					directive->emitDirective();
					endDirective = 0;
					}
			writeInstance(instance);
			if ( block->isSwitch && !isLabel )
				BlockTok::indentCount--;
			if ( block->isArrayInitializer && line->next )
				buffer->appendString(",");
			}
		}
	if ( block->isArrayInitializer )
		buffer->appendString("}");
	else
	if ( block->isBlock )
		{
		if ( block->isMethodBlock && enclosingMethod->directives && endDirective )
			{
			directive = endDirective;
			directive->emitDirective();
			endDirective = 0;
			}
		if ( BlockTok::indentCount == 1 )
			BlockTok::indentCount--;
		::indent(BlockTok::indentCount,"\t",buffer);
		if ( block->isLambda )
			buffer->appendString("}");
		else	buffer->appendString("}\n");
		}
}

/*******************************************************************************
        Write out the expression
*******************************************************************************/
void FormatC::writeExpression(Expression *expression)
{
DoubleLink 	*link = 0;
	expression->checkExpression(this);
	if ( expression->subject && expression->subject->getSubject()->isDeclaration && !writingParameters )
		{
		declare(expression);
		return;
		}
	if ( expression->hasParens )
		buffer->appendString("(");
	if ( expression->subject )
		{
		if ( expression->verb && expression->verb->assign )
			expression->subject->noGetter = 1;
		writeInstance(expression->subject);
		}
	if ( expression->verb )
		{
		if ( expression->subject && expression->verb->assign )
			expression->subject->noGetter = 0;
		if ( !expression->subject )
			buffer->appendString(expression->verb->op);
		else
		if ( expression->verb->compare(",") == 0 )
			{
			buffer->appendString(expression->verb->op);
			buffer->appendString(" ");
			}
		else
		if ( expression->verb->unary )
			buffer->appendString(expression->verb->op);
		else {
			buffer->appendString(" ");
			buffer->appendString(expression->verb->op);
			buffer->appendString(" ");
			}
		/***********************************************************************
		Lambda assignment parameters are written here
		***********************************************************************/
		if ( expression->subject && expression->subject->symbol && expression->subject->isLambda )
			{
			buffer->appendString("^");
			if ( expression->subject->symbol->parameters )
				{
				buffer->appendString("(");
				for ( link = expression->subject->symbol->parameters->first; link; link = link->next )
					{
					Symbol 	*symbol = (Symbol*)link->value;
					declare(symbol,0,0);
					if ( symbol->isMethod )
						buffer->current -= 2;
					if ( link->next )
						buffer->appendString(", ");
					}
				buffer->appendString(")\n");
				}
			else	buffer->appendString("\n");
			BlockTok::indentCount++;
			::indent(BlockTok::indentCount,"\t",buffer);
			}
		}
	if ( expression->object )
		writeInstance(expression->object);
	if ( expression->hasParens )
		buffer->appendString(")");
	if ( expression->verb && expression->verb->compare("=") == 0 && expression->subject && expression->subject->symbol && expression->subject->isLambda )
		BlockTok::indentCount--;
}

/*******************************************************************************
        Write the appropriate indirection markers.
*******************************************************************************/
void FormatC::writeIndirect(Instance *instance)
{
int 	i = instance->isDeclaration ? instance->howDirect() : (int)instance->indirection;
	for ( ; i > 0; i-- )
		buffer->appendString("*");
	if ( instance->reference && !instance->isMethod )
		for ( i = instance->reference; i; i-- )
			buffer->appendString("&");
}

/*******************************************************************************
        Write out the instance
*******************************************************************************/
void FormatC::writeInstance(Instance *instance)
{
	if ( instance->isDeclaration && !writingParameters )
		{
		if ( BlockTok::indentCount > 1 )
			::indent(BlockTok::indentCount,"\t",buffer);
		declare(instance,0);
		if ( !instance->isComment && instance->symbol && ((!instance->isMethod && !instance->symbol->type->structure) || instance->howDirect()) )
			buffer->appendString(" = 0");
		return;
		}
	if ( instance->atString )
		buffer->appendString("@");
	if ( instance->cast )
		{
		writeInstance(instance->cast);
		//cast = null;
		}
	if ( instance->isCast || (instance->type && !instance->symbol && !instance->isSelector) )
		{
		if ( instance->isCast )
			{
			if ( instance->prefix )
				buffer->appendString(instance->prefix);
			buffer->appendString("(");
			}
		if ( instance->isCast && instance->isMethod && instance->symbol )
			buffer->appendString(instance->symbol->getSignature(1));
		else {
			if ( instance->type && !instance->isConstant )
				{
				buffer->appendString(instance->type->name);
				writeIndirect(instance);
				}
			if ( instance->isConstant && (instance->type == SymbolType::stringType || instance->atString) )
				if ( instance->parent && instance->parent->getType()->isOC )
					printQualified(instance);
				else {
					buffer->appendString("\"");
					buffer->appendString(instance->prefix);
					buffer->appendString("\"");
					}
			else
			if ( !instance->isCast && instance->prefix )
				buffer->appendString(instance->prefix);
			if ( instance->isMethod && (instance->isLambda || instance->reference) )
				{
				if ( instance->isLambda )
					buffer->appendString("(^)(");
				else	buffer->appendString("(*)(");
				if ( instance->parameters )
					{
					Instance 	*argument = 0;
					instance->parameters->entry = 0;
					while ( argument = (Instance*)instance->parameters->next() )
						{
						writeInstance(argument);
						buffer->appendString(",");
						}
					buffer->current--;
					}
				buffer->appendString(")");
				}
			if ( instance->postfix )
				buffer->appendString(instance->postfix);
			}
		if ( instance->isCast )
			buffer->appendString(")");
		}
	else
	if ( instance->isComment && !instance->isLabel && instance->prefix )
		buffer->appendString(instance->prefix);
	else
	if ( instance->symbol )
		{
		instance->setRefer();
		if ( !instance->isNew )
			writeIndirect(instance);
		if ( instance->prefix )
			buffer->appendString(instance->prefix);
		printQualified(instance);
		if ( instance->postfix )
			buffer->appendString(instance->postfix);
		}
	else {
		if ( instance->express )
			{
			if ( instance->resolved && !instance->express )
				writeInstance(instance);
			else {
				if ( instance->prefix )
					buffer->appendString(instance->prefix);
				if ( instance->indirection )
					writeIndirect(instance);
				writeExpression(instance->express);
				}
			}
		else
		if ( instance->block )
			writeBlock(instance->block);
		else
		if ( instance->statement )
			writeStatement(instance->statement);
		else
		if ( instance->prefix )
			if ( instance->isSelector )
				{
				SymbolType 	*type = (SymbolType*)SymbolType::types->get(instance->prefix);
				if ( type && isProtocol(type->structure) )
					buffer->appendString("@protocol(");
				else	buffer->appendString("@selector(");
				if ( type )
					type->setRefer();
				buffer->appendString(instance->prefix);
				buffer->appendString(")");
				}
			else {
				if ( instance->isConstant && instance->type == SymbolType::stringType )
					{
					buffer->appendString("\"");
					buffer->appendString(instance->prefix);
					buffer->appendString("\"");
					}
				else	buffer->appendString(instance->prefix);
				writeParameters(instance);
				}
		if ( instance->postfix )
			buffer->appendString(instance->postfix);
		}
}

/*******************************************************************************
        Write the front part of a lambda statement
*******************************************************************************/
void FormatC::writeLambda(Statement *statement)
{
DoubleLink 	*link = 0;
Symbol 		*parameter = 0;
	statement->first->symbol->setRefer();
	writeParameterType(statement->first->symbol);
	buffer->appendString("(^");
	buffer->appendString(statement->first->symbol->name);
	buffer->appendString(")(");
	if ( statement->first->symbol->parameters )
		for ( link = statement->first->symbol->parameters->first; link; link = link->next )
			{
			parameter = (Symbol*)link->value;
			writeParameterType(parameter);
			if ( link->next )
				buffer->appendString(", ");
			}
	buffer->appendString(") = ^");
	BlockTok::indentCount++;
	if ( statement->first->symbol->parameters )
		{
		buffer->appendString("(");
		for ( link = statement->first->symbol->parameters->first; link; link = link->next )
			{
			parameter = (Symbol*)link->value;
			writeParameterType(parameter);
			buffer->appendString(parameter->name);
			if ( link->next )
				buffer->appendString(", ");
			}
		buffer->appendString(")");
		}
	buffer->appendString("\n");
	::indent(BlockTok::indentCount,"\t",buffer);
	writeInstance(statement->second);
	buffer->appendString(";\n");
	BlockTok::indentCount--;
}

/*******************************************************************************
        prints out the Objective-C signature of a method
*******************************************************************************/
void FormatC::writeOCsignature(Symbol *method)
{
int 		j = 0;
DoubleLink 	*link = 0;
char 		*atArray = 0;
	method->setRefer();
	if ( !method->isConstructor )
		{
		if ( method->isStatic )
			buffer->appendString("+ (");
		else	buffer->appendString("- (");
		if ( method->type == SymbolType::stringType )
			buffer->appendString("char");
		else	buffer->appendString(method->type->name);
		j = method->indirect;
		if ( j )
			for ( ; j > 0; j-- )
				buffer->appendString("*");
		else
		if ( method->type->isDirect || (method->type == SymbolType::stringType && method->isArray) )
			;
		else	buffer->appendString(::toStringFromChar('*'));
		buffer->appendString(")");
		buffer->appendString(method->name);
		}
	else	buffer->appendString("- (id)init");
	if ( method->parameters )
		{
		buffer->appendString(":");
		for ( link = method->parameters->first; link; link = link->next )
			{
			Symbol 	*symbol = (Symbol*)link->value;
			j = symbol->indirect;
			if ( symbol->isMethod )
				{
				writeOCsignature(symbol);
				buffer->current--;
				}
			else {
				if ( link->prior )
					{
					buffer->appendString(" ");
					buffer->appendString(symbol->name);
					buffer->appendString(":");
					}
				buffer->appendString("(");
				if ( symbol->type == SymbolType::stringType )
					buffer->appendString("char");
				else	buffer->appendString(symbol->type->name);
				if ( !j && !symbol->type->isDirect )
					buffer->appendString("*");
				for ( ; j > 0; j-- )
					buffer->appendString("*");
				if ( symbol->isArray )
					for ( atArray = symbol->array; *atArray; atArray++ )
						if ( *atArray == '[' )
							buffer->appendString("[]");
				buffer->appendString(")");
				buffer->appendString(symbol->name);
				}
			}
		}
}

/*******************************************************************************
     Write the type part of a parameter
*******************************************************************************/
void FormatC::writeParameterType(Symbol *symbol)
{
int 	j = 0;
	if ( symbol->type == SymbolType::stringType )
		buffer->appendString("char ");
	else
	if ( symbol->type != SymbolType::nullType )
		if ( symbol->isItem && symbol->structType && isBoolean(symbol->structType->structure) && currentType->isOC )
			buffer->appendString("BOOL ");
		else {
			buffer->appendString(symbol->type->name);
			buffer->appendString(" ");
			}
	if ( symbol->indirect )
		for ( j = symbol->indirect - symbol->isArray; j > 0; j-- )
			buffer->appendString("*");
	else
	if ( !symbol->type->isDirect && !symbol->isConstructor )
		buffer->appendString(::toStringFromChar('*'));
	for ( j = symbol->reference; j; j-- )
		buffer->appendString(::toStringFromChar('&'));
}

/*******************************************************************************
        write parameters
*******************************************************************************/
void FormatC::writeParameters(Instance *instance)
{
DoubleLink 	*symbolLink = 0;
DoubleLink 	*link = 0;
Symbol 		*symbol = instance->getSymbol();
int 		isOC = 0;
	writingParameters = 1;
	if ( symbol )
		if ( symbol->isMethod )
			{
			if ( instance->isNew && symbol->type->isOC )
				isOC = 1;
			else
			if ( symbol->parentClass && !symbol->parentClass->isGlobal )
				isOC = symbol->parentClass->isOC;
			}
		else	isOC = symbol->type->isOC;
	if ( instance->isMethod && instance->reference && !instance->parameters )
		goto bailOnParameters;
	if ( instance->isLambda && instance->assigning )
		goto bailOnParameters;
	if ( !instance->parameters )
		{
		if ( !isOC && (instance->isNew || instance->isMethod) )
			buffer->appendString("()");
		goto bailOnParameters;
		}
	argumentIsOC = isOC;
	if ( instance->isMethod && !isOC )
		buffer->appendString("(");
	if ( symbol && symbol->parameters )
		symbolLink = symbol->parameters->first;
	for ( link = instance->parameters->first; link; link = link->next )
		{
		Symbol 		*parameterSymbol = 0;
		SymbolType 	*argumentType = 0;
		Instance 	*argument = (Instance*)link->value;
		argumentType = argument->sourceType();
		if ( symbolLink )
			parameterSymbol = (Symbol*)symbolLink->value;
		if ( isOC && instance->isMethod && parameterSymbol )
			if ( !(parameterSymbol->hasEllipsis && parameterSymbol->type == SymbolType::nullType) )
				if ( link->prior )
					{
					buffer->appendString(parameterSymbol->name);
					buffer->appendString(":");
					}
		if ( instance->arrayRef )
			buffer->appendString("[");
		if ( parameterSymbol && parameterSymbol->type->isOC && argumentType == SymbolType::stringType )
			argument = makeOCstring(argument);
		/***********************************************************************
		Set cast for void* arguments
		***********************************************************************/
		if ( argument->isVoidPointer() && parameterSymbol && argumentType != parameterSymbol->type )
			{
			Instance 	*psym = new Instance(parameterSymbol);
			argument = argument->checkCast(psym);
			delete psym;
			}
		writeInstance(argument);
		if ( instance->arrayRef )
			buffer->appendString("]");
		if ( !instance->arrayRef && link->next )
			if ( !isOC )
				buffer->appendString(",");
			else
			if ( instance->isMethod && (!parameterSymbol || parameterSymbol->hasEllipsis) )
				buffer->appendString(",");
			else	buffer->appendString(" ");
		if ( symbolLink )
			symbolLink = symbolLink->next;
		}
	if ( instance->isMethod && !isOC )
		buffer->appendString(")");
	argumentIsOC = 0;
bailOnParameters:
	writingParameters = 0;
}

/*******************************************************************************
        writes the C++ method, if flag not set only the signature is written
*******************************************************************************/
void FormatC::writeSignature(Symbol *method, int flag)
{
DoubleLink 	*link = 0;
int 		isString = 0;
int 		j = 0;
	if ( !method->isMethod )
		{
		buffer->appendString(method->name);
		buffer->appendString(" is not a method\n");
		return;
		}
	method->setRefer();
	if ( flag )
		{
		enclosingMethod = method;
		mStak->push(method);
		}
	if ( method->parentClass && method->parentClass->isOC )
		{
		if ( flag )
			buffer->appendString("\n");
		writeOCsignature(method);
		if ( flag )
			{
			if ( method->isConstructor )
				initialize(method);
			goto writeBody;
			}
		return;
		}
	isString = method->type == SymbolType::stringType;
	if ( flag && !method->block )
		return;
	if ( flag )
		{
		buffer->appendString("\n");
		if ( method->comment )
			buffer->appendString(method->comment);
		}
	if ( !method->isConstructor )
		{
		if ( method->isStatic && !flag )
			buffer->appendString("static ");
		if ( method->isExtern && method->parentClass->isGlobal )
			buffer->appendString("extern \"C\" ");
		if ( method->type->nameSpace )
			{
			buffer->appendString(method->type->nameSpace);
			buffer->appendString("::");
			}
		if ( isString )
			buffer->appendString("char ");
		else {
			buffer->appendString(method->type->name);
			buffer->appendString(" ");
			}
		j = method->indirect;
		if ( j )
			for ( ; j > 0; j-- )
				buffer->appendString("*");
		else
		if ( method->type->isDirect || (isString && method->isArray) )
			;
		else	buffer->appendString(::toStringFromChar('*'));
		if ( method->reference )
			buffer->appendString("(*");
		else
		if ( method->isLambda )
			buffer->appendString("(^");
		}
	else
	if ( flag )
		initialize(method);
	if ( flag && !processingGlobalMethods && !method->parentClass->isC )
		{
		buffer->appendString(method->parentClass->name);
		buffer->appendString("::");
		}
	buffer->appendString(method->name);
	if ( method->parentClass && method->parentClass->addClassNameToMethods && !method->reference )
		buffer->appendString(method->parentClass->name);
	if ( method->reference || method->isLambda )
		buffer->appendString(")");
	buffer->appendString("(");
	if ( method->parameters )
		for ( link = method->parameters->first; link; link = link->next )
			{
			Symbol 	*symbol = (Symbol*)link->value;
			declare(symbol,flag,0);
			if ( symbol->isMethod && !(symbol->reference || symbol->isLambda) )
				buffer->current -= 2;
			if ( link->next )
				buffer->appendString(", ");
			}
	buffer->appendString(")");
writeBody:
	if ( flag && method->block )
		{
		buffer->appendString("\n");
		if ( method->directives )
			{
			method->directives->resetIterator();
			startDirective = 0;
			endDirective = 0;
			Directive *directive = (Directive*)method->directives->next();
			while ( directive )
				{
				directive->isDirected = 0;
				if ( directive->atStart )
					startDirective = directive;
				if ( directive->atEnd )
					endDirective = directive;
				directive = (Directive*)method->directives->next();
				}
			}
		buffer->setMark();
		writeBlock(method->block);
		}
	if ( flag )
		enclosingMethod = (Symbol*)mStak->pop();
}

/*******************************************************************************
        Write a statement
*******************************************************************************/
void FormatC::writeStatement(Statement *statement)
{
Directive 	*directive = 0;
int 		pendingFlag = 0;
	if ( pendingDirective && (!statement->first || !statement->first->block) )
		{
		directive = pendingDirective;
		pendingDirective = 0;
		pendingFlag = 1;
		}
	if ( !Tok::tawking->noLoop && enclosingMethod && enclosingMethod->directives )
		if ( statement->pointInCode )
			{
			if ( !statement->first || (statement->first && !statement->first->isDeclaration) )
				if ( startDirective )
					{
					directive = startDirective;
					startDirective = 0;
					enclosingMethod->directives->next();
					directive->emitDirective();
					directive = 0;
					}
			if ( !directive && !pendingFlag )
				{
				if ( enclosingMethod->directives->entry )
					directive = (Directive*)enclosingMethod->directives->entry->value;
				else	directive = (Directive*)enclosingMethod->directives->next();
				if ( directive )
					if ( directive->atEnd || directive->atStart )
						directive = 0;
					else
					if ( directive->isDirected || ::strncmp(directive->codeMatch,statement->pointInCode->itemStart,::strlen(directive->codeMatch)) != 0 )
						directive = 0;
				}
			if ( directive )
				if ( directive->comesBefore || pendingFlag )
					{
					Tok::tawking->noLoop = 1;
					directive->emitDirective();
					Tok::tawking->noLoop = 0;
					directive = 0;
					}
			}
	if ( statement->indented )
		::indent(BlockTok::indentCount,"\t",buffer);
restart:
	switch (statement->statementType)
		{
		case DELETE:
			buffer->appendString("delete ");
			writeInstance(statement->first);
			buffer->appendString(";\n");
			break;
		case DO:
			buffer->appendString("do\t");
			statement->first->statement->indented = 0;
			BlockTok::indentCount++;
			statement->checkBlock(statement->first);
			writeInstance(statement->first);
			BlockTok::indentCount--;
			::indent(BlockTok::indentCount,"\t",buffer);
			buffer->appendString("while ");
			if ( statement->second->express && statement->second->express->hasParens && !statement->second->indirection )
				;
			else	buffer->appendString("( ");
			writeInstance(statement->second);
			if ( statement->second->express && statement->second->express->hasParens && !statement->second->indirection )
				;
			else	buffer->appendString(" )");
			buffer->appendString(";\n");
			break;
		case FOR:
			buffer->appendString("for ( ");
			if ( statement->first && statement->first->express && statement->first->express->verb )
				writeInstance(statement->first);
			buffer->appendString("; ");
			if ( statement->second )
				writeInstance(statement->second);
			buffer->appendString("; ");
			if ( statement->third )
				writeInstance(statement->third);
			buffer->appendString(" )\n");
			BlockTok::indentCount++;
			statement->checkBlock(statement->fourth);
			writeInstance(statement->fourth);
			BlockTok::indentCount--;
			break;
		case IF:
			buffer->appendString("if ");
			if ( statement->first->express && statement->first->express->hasParens && !statement->first->indirection )
				;
			else	buffer->appendString("( ");
			writeInstance(statement->first);
			if ( statement->first->express && statement->first->express->hasParens && !statement->first->indirection )
				;
			else	buffer->appendString(" )");
			buffer->appendString("\n");
			BlockTok::indentCount++;
			statement->checkBlock(statement->second);
			if ( directive && directive->within && statement->second->statement && statement->second->statement->first && statement->second->statement->first->block )
				{
				pendingDirective = directive;
				directive = 0;
				}
			if ( !statement->second->statement )
				::indent(BlockTok::indentCount,"\t",buffer);
			writeInstance(statement->second);
			BlockTok::indentCount--;
			if ( statement->third )
				{
				::indent(BlockTok::indentCount,"\t",buffer);
				buffer->appendString("else");
				if ( !statement->third->isBlockStatement() )
					{
					if ( statement->third->statement->statementType == NOTSPECIFIED || statement->third->statement->statementType == RETURN || statement->third->statement->statementType == GOTO )
						{
						statement->third->statement->indented = 0;
						buffer->appendString("\t");
						}
					else	buffer->appendString("\n");
					statement->checkBlock(statement->third);
					writeInstance(statement->third);
					}
				else {
					buffer->appendString(" ");
					BlockTok::indentCount++;
					statement->checkBlock(statement->third);
					writeInstance(statement->third);
					BlockTok::indentCount--;
					}
				}
			break;
		case LABEL:
			writeInstance(statement->first);
			buffer->appendString(":\n");
			break;
		case LAMBDA:
			writeLambda(statement);
			break;
		case RETURN:
			buffer->appendString("return");
			if ( statement->first )
				{
				buffer->appendString(" ");
				writeInstance(statement->first);
				}
			buffer->appendString(";\n");
			break;
		case SWITCH:
			if ( statement->noFallThru )
				statement->addBreaks();
			if ( statement->switching )
				{
				statement->convertSwitch();
				goto restart;
				}
			buffer->appendString("switch ");
			if ( statement->first->express && statement->first->express->hasParens && !statement->first->indirection )
				;
			else	buffer->appendString("( ");
			writeInstance(statement->first);
			if ( statement->first->express && statement->first->express->hasParens && !statement->first->indirection )
				;
			else	buffer->appendString(" )");
			buffer->appendString("\n");
			BlockTok::indentCount++;
			::indent(BlockTok::indentCount,"\t",buffer);
			writeInstance(statement->second);
			BlockTok::indentCount--;
			break;
		case WHILE:
			buffer->appendString("while ");
			if ( statement->first->express && statement->first->express->hasParens && !statement->first->indirection )
				;
			else	buffer->appendString("( ");
			writeInstance(statement->first);
			if ( statement->first->express && statement->first->express->hasParens && !statement->first->indirection )
				;
			else	buffer->appendString(" )");
			buffer->appendString("\n");
			BlockTok::indentCount++;
			// The following only relevent when for gets changed to while
			if ( !statement->second )
				statement->second = statement->fourth;
			statement->checkBlock(statement->second);
			writeInstance(statement->second);
			BlockTok::indentCount--;
			break;
		case GOTO:
			buffer->appendString("goto ");
			writeInstance(statement->first);
			buffer->appendString(";\n");
			break;
		default:
			if ( statement->first )
				{
				if ( statement->first->isComment )
					indentComment(statement->first);
				else	writeInstance(statement->first);
				if ( !statement->first->isComment && !statement->first->isLabel && !(statement->block && statement->block->isArrayInitializer) && !(statement->first->block && !(statement->first->symbol || statement->first->express || statement->first->statement)) )
					buffer->appendString(";\n");
				}
			else	buffer->appendString(";\n");
		}
	if ( directive )
		{
		Tok::tawking->noLoop = 1;
		directive->emitDirective();
		Tok::tawking->noLoop = 0;
		}
}
