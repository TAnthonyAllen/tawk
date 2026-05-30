#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "StringRoutines.h"
#include "PLGparse.h"
#include "Symbol.h"
#include "SymbolType.h"
#include "Operate.h"
#include "DoubleLinkList.h"
#include "DoubleLink.h"
#include "BaseHash.h"
#include "Buffer.h"
#include "PLGitem.h"
#include "BlockTok.h"
#include "Expression.h"
#include "Statement.h"
#include "Tawk.h"
#include "Tok.h"
#include "Instance.h"
int Instance::instanceCount;
Symbol *Instance::objectiveCsend;

/*******************************************************************************
        Constructors
*******************************************************************************/
Instance::Instance()
{
	block = 0;
	express = 0;
	statement = 0;
	symbol = 0;
	type = 0;
	cast = 0;
	format = 0;
	parent = 0;
	level = 0;
	arrayRef = 0;
	assigning = 0;
	atString = 0;
	checked = 0;
	emitted = 0;
	hasBranch = 0;
	indirection = 0;
	isCase = 0;
	isCast = 0;
	isComment = 0;
	isCondition = 0;
	isConstant = 0;
	isCopy = 0;
	isDeclaration = 0;
	isError = 0;
	isExternalType = 0;
	isLabel = 0;
	isLambda = 0;
	isLocal = 0;
	isMethod = 0;
	isNew = 0;
	isPrintMethod = 0;
	isRange = 0;
	isSelector = 0;
	noBody = 0;
	noGetter = 0;
	reference = 0;
	requalified = 0;
	resolved = 0;
	terminated = 0;
	virtuous = 0;
	prefix = 0;
	postfix = 0;
	parameters = 0;
	isUsed = 0;
	instanceIndex = Instance::instanceCount++;
}

Instance::Instance(SymbolType *t)
{
	block = 0;
	express = 0;
	statement = 0;
	symbol = 0;
	cast = 0;
	format = 0;
	parent = 0;
	level = 0;
	arrayRef = 0;
	assigning = 0;
	atString = 0;
	checked = 0;
	emitted = 0;
	hasBranch = 0;
	indirection = 0;
	isCase = 0;
	isCast = 0;
	isComment = 0;
	isCondition = 0;
	isConstant = 0;
	isCopy = 0;
	isDeclaration = 0;
	isError = 0;
	isExternalType = 0;
	isLabel = 0;
	isLambda = 0;
	isLocal = 0;
	isMethod = 0;
	isNew = 0;
	isPrintMethod = 0;
	isRange = 0;
	isSelector = 0;
	noBody = 0;
	noGetter = 0;
	reference = 0;
	requalified = 0;
	resolved = 0;
	terminated = 0;
	virtuous = 0;
	prefix = 0;
	postfix = 0;
	parameters = 0;
	isUsed = 0;
	type = t;
	instanceIndex = Instance::instanceCount++;
}

Instance::Instance(Symbol *s)
{
	block = 0;
	express = 0;
	statement = 0;
	type = 0;
	cast = 0;
	format = 0;
	parent = 0;
	arrayRef = 0;
	assigning = 0;
	atString = 0;
	checked = 0;
	emitted = 0;
	hasBranch = 0;
	indirection = 0;
	isCase = 0;
	isCast = 0;
	isComment = 0;
	isCondition = 0;
	isConstant = 0;
	isCopy = 0;
	isDeclaration = 0;
	isError = 0;
	isExternalType = 0;
	isLabel = 0;
	isLocal = 0;
	isNew = 0;
	isPrintMethod = 0;
	isRange = 0;
	isSelector = 0;
	noBody = 0;
	noGetter = 0;
	requalified = 0;
	resolved = 0;
	terminated = 0;
	virtuous = 0;
	prefix = 0;
	postfix = 0;
	parameters = 0;
	isUsed = 0;
	symbol = s;
	isMethod = s->isMethod ? (unsigned int)1 : (unsigned int)0;
	reference = isMethod ? s->reference : (unsigned int)0;
	isLambda = s->isLambda;
	level = 1;
	instanceIndex = Instance::instanceCount++;
}

Instance::Instance(char *s)
{
	block = 0;
	express = 0;
	statement = 0;
	symbol = 0;
	cast = 0;
	format = 0;
	parent = 0;
	level = 0;
	arrayRef = 0;
	assigning = 0;
	atString = 0;
	checked = 0;
	emitted = 0;
	hasBranch = 0;
	indirection = 0;
	isCase = 0;
	isCast = 0;
	isComment = 0;
	isCondition = 0;
	isCopy = 0;
	isDeclaration = 0;
	isError = 0;
	isExternalType = 0;
	isLabel = 0;
	isLambda = 0;
	isLocal = 0;
	isMethod = 0;
	isNew = 0;
	isPrintMethod = 0;
	isRange = 0;
	isSelector = 0;
	noBody = 0;
	noGetter = 0;
	reference = 0;
	requalified = 0;
	resolved = 0;
	terminated = 0;
	virtuous = 0;
	postfix = 0;
	parameters = 0;
	isUsed = 0;
	prefix = s;
	isConstant = 1;
	type = SymbolType::stringType;
	instanceIndex = Instance::instanceCount++;
}

Instance::Instance(Statement *s)
{
	block = 0;
	express = 0;
	symbol = 0;
	type = 0;
	cast = 0;
	format = 0;
	parent = 0;
	level = 0;
	arrayRef = 0;
	assigning = 0;
	atString = 0;
	checked = 0;
	emitted = 0;
	hasBranch = 0;
	indirection = 0;
	isCase = 0;
	isCast = 0;
	isComment = 0;
	isCondition = 0;
	isConstant = 0;
	isCopy = 0;
	isDeclaration = 0;
	isError = 0;
	isExternalType = 0;
	isLabel = 0;
	isLambda = 0;
	isLocal = 0;
	isMethod = 0;
	isNew = 0;
	isPrintMethod = 0;
	isRange = 0;
	isSelector = 0;
	noBody = 0;
	noGetter = 0;
	reference = 0;
	requalified = 0;
	resolved = 0;
	terminated = 0;
	virtuous = 0;
	prefix = 0;
	postfix = 0;
	parameters = 0;
	isUsed = 0;
	statement = s;
	instanceIndex = Instance::instanceCount++;
}

Instance::Instance(Instance *i)
{
DoubleLink 	*link = 0;
Instance 	*parameter = 0;
	*this = *i;
	isDeclaration = 0;
	parameters = 0;
	if ( i->parameters )
		for ( link = i->parameters->first; link; link = link->next )
			{
			parameter = (Instance*)link->value;
			addParameter(parameter);
			}
	instanceIndex = Instance::instanceCount++;
	isCopy = 1;
}

Instance::Instance(Expression *e)
{
	block = 0;
	statement = 0;
	symbol = 0;
	type = 0;
	cast = 0;
	format = 0;
	parent = 0;
	level = 0;
	arrayRef = 0;
	assigning = 0;
	atString = 0;
	checked = 0;
	emitted = 0;
	hasBranch = 0;
	indirection = 0;
	isCase = 0;
	isCast = 0;
	isComment = 0;
	isCondition = 0;
	isConstant = 0;
	isCopy = 0;
	isDeclaration = 0;
	isError = 0;
	isExternalType = 0;
	isLabel = 0;
	isLambda = 0;
	isLocal = 0;
	isMethod = 0;
	isNew = 0;
	isPrintMethod = 0;
	isRange = 0;
	isSelector = 0;
	noBody = 0;
	noGetter = 0;
	reference = 0;
	requalified = 0;
	resolved = 0;
	terminated = 0;
	virtuous = 0;
	prefix = 0;
	postfix = 0;
	parameters = 0;
	isUsed = 0;
	express = e;
	instanceIndex = Instance::instanceCount++;
	if ( e->subject )
		isMethod = e->subject->isMethod;
}

Instance::Instance(BlockTok *b)
{
	express = 0;
	statement = 0;
	symbol = 0;
	type = 0;
	cast = 0;
	format = 0;
	parent = 0;
	level = 0;
	arrayRef = 0;
	assigning = 0;
	atString = 0;
	checked = 0;
	emitted = 0;
	hasBranch = 0;
	indirection = 0;
	isCase = 0;
	isCast = 0;
	isComment = 0;
	isCondition = 0;
	isConstant = 0;
	isCopy = 0;
	isDeclaration = 0;
	isError = 0;
	isExternalType = 0;
	isLabel = 0;
	isLambda = 0;
	isLocal = 0;
	isMethod = 0;
	isNew = 0;
	isPrintMethod = 0;
	isRange = 0;
	isSelector = 0;
	noBody = 0;
	noGetter = 0;
	reference = 0;
	requalified = 0;
	resolved = 0;
	terminated = 0;
	virtuous = 0;
	prefix = 0;
	postfix = 0;
	parameters = 0;
	isUsed = 0;
	block = b;
	instanceIndex = Instance::instanceCount++;
}

/*******************************************************************************
        Add a parameter to this instance (when instance is a method or array)
*******************************************************************************/
void Instance::addParameter(Instance *parameter)
{
	if ( !parameters )
		parameters = new DoubleLinkList();
	parameters->add((void*)parameter);
}

/*******************************************************************************
        Add a postfix to instance (append after existing postfix if there is one)
*******************************************************************************/
void Instance::addPostFix(char *s)
{
	if ( postfix )
		postfix = ::concat(2,postfix,s);
	else	postfix = s;
}

/*******************************************************************************
        Add a prefix to instance (append after existing prefix if there is one)
*******************************************************************************/
void Instance::addPrefix(char *s)
{
	if ( prefix )
		prefix = ::concat(2,prefix,s);
	else	prefix = s;
}

/*******************************************************************************
        If alias and alias not the same type as source, generate a cast
		and wrap in an expression
*******************************************************************************/
Instance *Instance::castAlias()
{
Instance 	*instance = 0;
Expression 	*exp = 0;
	if ( !symbol || !symbol->isAlias || symbol->type == symbol->source->type )
		return this;
	instance = new Instance(symbol->type);
	instance->indirection = getDirect();
	exp = new Expression();
	exp->subject = new Instance(this);
	exp->hasParens = 1;
	exp->subject->cast = instance;
	if ( isMethod )
		{
		exp->subject->cast->setReference(reference);
		exp->subject->cast->isMethod = isMethod;
		}
	exp->subject->cast->isCast = 1;
	exp->subject->symbol = symbol->source;
	instance = new Instance(exp);
	return instance;
}

/*******************************************************************************
        Runs all fields thru a sanity check
*******************************************************************************/
void Instance::check()
{
	if ( express )
		{
		if ( !emitted )
			express->check();
		if ( statement || symbol || type )
			::fprintf(stderr,"Instance express check: multiple fields set\n");
		}
	else
	if ( statement )
		{
		if ( !emitted )
			statement->check();
		if ( symbol || type )
			::fprintf(stderr,"Instance statement check: multiple fields set\n");
		}
	else
	if ( symbol && type )
		::fprintf(stderr,"Instance check: multiple fields set\n");
	else
	if ( block && !symbol && !type && !emitted )
		block->check();
	if ( !emitted )
		emitted = 1;
	//else cerr "Instance check: already done":;
}

/*******************************************************************************
        Process overload of []= (assumes this instance is an expression)
*******************************************************************************/
void Instance::checkBracketEqual()
{
SymbolType 	*assignType = 0;
Instance 	*instance = 0;
char 		*text = 0;
	if ( express->subject->symbol )
		{
		assignType = express->subject->getType();
		if ( text = assignType->overloaded("[]=") )
			{
			instance = Tok::tawking->getInstance(text);
			instance->parameters = express->subject->parameters;
			instance->addParameter(express->object);
			if ( instance->symbol = assignType->findMethod(instance) )
				{
				express->subject->parameters = 0;
				express->subject->arrayRef = 0;
				instance->setParent(express->subject);
				instance->prefix = 0;
				instance->resolved = 1;
				instance->isMethod = 1;
				*this = *instance;
				}
			}
		}
}

/*******************************************************************************
        Checks that the type of the instance passed in matches this instance
        and if not, casts this instance to match. Note this ignores alias type
*******************************************************************************/
Instance *Instance::checkCast(Instance *target)
{
int 		indirect = howDirect();
int 		targetDirect = target->howDirect();
SymbolType 	*thisType = 0;
SymbolType 	*targetType = 0;
Instance 	*instance = this;
	if ( target )
		{
		targetType = target->getType();
		if ( targetType == SymbolType::intType && isVoidPointer() )
			targetType = SymbolType::longType;
		thisType = sourceType();
		if ( !cast && thisType != targetType || indirect != targetDirect || (target->reference && isMethod != target->isMethod) )
			{
			instance = new Instance(this);
			instance->cast = new Instance(targetType);
			instance->cast->indirection = targetDirect;
			if ( target->isMethod )
				{
				instance->cast->setReference(target->reference);
				instance->cast->isMethod = target->isMethod;
				instance->cast->symbol = target->symbol;
				}
			instance->cast->isCast = 1;
			}
		else
		if ( cast && thisType != targetType || indirect != targetDirect )
			{
			instance = new Instance(this);
			instance->cast->type = targetType;
			instance->cast->indirection = targetDirect;
			if ( target->isMethod )
				{
				instance->cast->setReference(target->reference);
				instance->cast->isMethod = target->isMethod;
				instance->cast->symbol = target->symbol;
				}
			instance->cast->isCast = 1;
			}
		}
	return instance;
}

/*******************************************************************************
        Checks that the type of the symbol passed in matches this instance
        and if not, casts this instance to match.
*******************************************************************************/
Instance *Instance::checkCast(Symbol *target)
{
int 		indirect = howDirect();
SymbolType 	*thisType = 0;
SymbolType 	*targetType = 0;
Instance 	*instance = this;
	if ( !cast && target )
		{
		thisType = sourceType();
		targetType = target->type;
		if ( targetType == SymbolType::intType && isVoidPointer() )
			targetType = SymbolType::longType;
		if ( thisType != targetType || indirect != target->indirect )
			{
			instance = new Instance(this);
			instance->cast = new Instance(targetType);
			instance->cast->indirection = target->indirect;
			if ( target->isMethod )
				{
				instance->cast->setReference(target->reference);
				instance->cast->isMethod = target->isMethod;
				instance->cast->symbol = target;
				}
			instance->cast->isCast = 1;
			}
		}
	return instance;
}

/*******************************************************************************
        If there is an overloaded method that applies, convert this instance
		to contain the overloaded method. If there is no overloaded method,
		returns null. Note [] has precedent over prefix or postfix (++ or --)
*******************************************************************************/
Instance *Instance::checkOverload()
{
SymbolType 		*overloadType = 0;
char 			*splitOver = 0;
char 			*text = 0;
Symbol 			*method = 0;
Instance 		*copy = 0;
Instance 		*instance = 0;
DoubleLinkList 	*objectParameters = 0;
int 			chained = 0;
	if ( checked )
		return this;
	checked = 1;
startOver:
	if ( express )
		{
		if ( express->object && !express->object->checked && (express->object->express || express->object->isVirtuous()) )
			express->object = express->object->checkOverload();
		if ( express->subject && express->subject->isVirtuous() )
			{
			if ( express->verb && express->subject->arrayRef && express->verb->compare("=") == 0 )
				{
				checkBracketEqual();
				return this;
				}
			if ( !express->subject->checked )
				express->subject = express->subject->checkOverload();
			}
		}
	if ( express && express->subject )
		overloadType = express->subject->getType();
	else	overloadType = getType();
	if ( !overloadType || !overloadType->overloads )
		return this;
	/***************************************************************************
	Resolve overloaded verb
	***************************************************************************/
	if ( express && express->verb && express->subject && express->object )
		{
		char 	*operation = express->verb->op;
		if ( text = overloadType->overloaded(operation) )
			if ( express->object->express && express->object->express->verb )
				{
				operation = ::concat(2,express->verb->op,express->object->express->verb->op);
				splitOver = overloadType->overloaded(operation);
				if ( splitOver )
					text = splitOver;
				else	chained = 1;
				}
		if ( text )
			{
			instance = Tok::tawking->getInstance(text);
			/*******************************************************************
			In case of casts, expression wont have both subject and object
			*******************************************************************/
			if ( express->object->express && express->object->express->full )
				{
				instance->addParameter(express->subject);
				if ( chained )
					instance->addParameter(express->object->express->subject);
				else	instance->addParameter(express->object->express->object);
				}
			else	instance->addParameter(express->object);
			if ( copy = overloadType->findMethodInstance(instance) )
				{
				copy = new Instance(copy);
				copy->addParameter(express->object);
				instance = copy;
				}
			if ( !instance->symbol )
				{
				instance->parameters->first->value = (void*)express->object->getSubject();
				instance->symbol = overloadType->findMethod(instance);
				if ( !instance->symbol )
					instance->error("Could not find overload method");
				}
			if ( !instance->isError )
				{
				instance->prefix = 0;
				instance->type = 0;
				instance->isConstant = 0;
				instance->isMethod = 1;
				if ( chained )
					{
					express->object->express->subject = instance;
					express = express->object->express;
					chained = 0;
					goto startOver;
					}
				if ( express->object->isConstant && express->object->parameters )
					objectParameters = express->object->parameters;
				instance->setParent(express->subject);
				overloadType = instance->getType();
				/***************************************************************
				Prepare to handle remaining overload of [] if any
				***************************************************************/
				if ( objectParameters )
					{
					Instance 	*block = new Instance(overloadType);
					block->setParent(instance);
					block->arrayRef = 1;
					block->parameters = objectParameters;
					copy = block;
					}
				else	copy = instance;
				resolved = 1;
				goto finish;
				}
			}
		}
	/***************************************************************************
	Resolve overloaded array reference
	***************************************************************************/
	if ( !express && arrayRef && !howDirect() )
		if ( text = overloadType->overloaded("[]") )
			{
			copy = Tok::tawking->getInstance(text);
			copy->parameters = parameters;
			if ( instance = overloadType->findMethodInstance(copy) )
				{
				*copy = *instance;
				copy->parameters = parameters;
				parameters = 0;
				arrayRef = 0;
				copy->setParent(this);
				resolved = 1;
				goto finish;
				}
			delete copy;
			}
	if ( symbol )
		{
		/***********************************************************************
		Resolve increment or decrement (assumes overload type stays the same).
		No distinction made between prefix or postfix.
		***********************************************************************/
		if ( prefix || postfix )
			{
			if ( text = prefix ? overloadType->overloaded(prefix) : overloadType->overloaded(postfix) )
				{
				text = ::concat(2,text,"()");
				if ( method = overloadType->getMethod(text) )
					{
					instance = new Instance(method);
					instance->setParent(copy);
					copy = instance;
					resolved = 1;
					}
				prefix = 0;
				postfix = 0;
				}
			goto finish;
			}
		/***********************************************************************
		Handle overloaded () on a symbol that is not a method. Note: the
		fact that this instance isMethod means that there are () and may
		or may not be parameters
		***********************************************************************/
		if ( isMethod && !symbol->isMethod && (text = overloadType->overloaded("()")) )
			{
			copy = Tok::tawking->getInstance(text);
			copy->parameters = parameters;
			if ( instance = overloadType->findMethodInstance(copy) )
				{
				*copy = *instance;
				copy->parameters = parameters;
				copy->setReference((unsigned int)0);
				parameters = 0;
				isMethod = 0;
				copy->setParent(this);
				resolved = 1;
				}
			}
		}
finish:
	checked = 1;
	if ( copy )
		return copy;
	return this;
	//if resolved cout "checkOverload resolved: " text:;
}

/*******************************************************************************
        Check symbol and resolve alias if there is one
*******************************************************************************/
void Instance::checkSymbol()
{
Symbol 		*thisSymbol = 0;
Instance 	*newParameter = 0;
	if ( !symbol )
		return;
	/***************************************************************************
	Check for C method and extensions; fix if necessary. The alias is not
	resolved if the alias type does not match the source type.
	***************************************************************************/
	if ( isMethod && !reference && (symbol->isAlias || (symbol->parentClass && symbol->parentClass->isC)) )
		{
		//cout "checkSymbol: " symbol.name:;
		if ( symbol->isExtension )
			fixExtension();
		else {
			while ( symbol->source )
				if ( symbol->type != symbol->source->type )
					break;
				else	symbol = symbol->source;
			if ( symbol->parentClass && symbol->parentClass->isC && !symbol->parentClass->isGlobal && symbol->parameters && symbol->parentClass->components && (!parameters || symbol->parameters->length > parameters->length) )
				{
				if ( parent )
					{
					newParameter = parent;
					setParent((Instance*)0);
					}
				if ( !newParameter )
					{
					thisSymbol = (Symbol*)symbol->parentClass->components->get("tHIS");
					newParameter = new Instance(thisSymbol);
					}
				if ( !parameters )
					addParameter(newParameter);
				else	parameters->insert((void*)newParameter);
				}
			}
		}
}

/******************************************************************************
	Returns a copy of this with a parent set
******************************************************************************/
Instance *Instance::copyAndSetParent(Instance *instance)
{
Instance 	*copy = new Instance(this);
	copy->setParent(instance);
	return copy;
}

/*******************************************************************************
        Debugging routine
*******************************************************************************/
void Instance::dump()
{
	if ( symbol )
		::printf("Symbol ");
	else
	if ( express )
		::printf("Expression ");
	::printf("%s\n",toString());
}

/*******************************************************************************
        Write error message into this instance
*******************************************************************************/
void Instance::error(char *text)
{
char 	*message = ::concat(2," ERROR ",text);
	if ( postfix )
		postfix = ::concat(2,postfix,message);
	else	postfix = message;
	isError = 1;
	if ( symbol )
		message = ::concat(3,symbol->name,":",message);
	symbol = new Symbol(message,SymbolType::nullType);
	postfix = 0;
}

/*******************************************************************************
        Looks thru instance parent hierarchy to find the named setter or getter
        The name passed should be a methodName
*******************************************************************************/
Instance *Instance::findGetterOrSetter(char *name)
{
Instance 	*instance = parent;
Symbol 		*method = 0;
	while ( instance )
		{
		SymbolType 	*methodType = instance->getType();
		if ( method = methodType->findField(name) )
			if ( method->isMethod )
				break;
		instance = instance->parent;
		}
	return instance;
}

/*******************************************************************************
        If the instance symbol is an extension, set the symbol to its source
        and fix the parameters to include the extended class pointer
*******************************************************************************/
void Instance::fixExtension()
{
Instance 	*argument = 0;
	if ( !symbol || !symbol->isExtension )
		return;
	if ( parent )
		{
		argument = parent;
		setParent((Instance*)0);
		while ( symbol->source )
			if ( symbol->type != symbol->source->type )
				break;
			else	symbol = symbol->source;
		if ( !parameters )
			addParameter(argument);
		else
		if ( parameters->length < symbol->parameters->length )
			parameters->insert((void*)argument);
		}
}

/*******************************************************************************
        Get the block associated with this instance (if it has one)
*******************************************************************************/
BlockTok *Instance::getBlock()
{
Instance 	*instance = 0;
	if ( block )
		return block;
	if ( statement )
		return statement->getBlock();
	if ( express )
		{
		instance = getSubject();
		return instance->getBlock();
		}
	return 0;
}

char *Instance::getCast()
{
char 	*castText = 0;
Buffer 	*buffer = new Buffer();
	getCast(buffer);
	castText = buffer->toString();
	delete buffer;
	return castText;
}

/*******************************************************************************
        Return cast string for this instance w/out enclosing ()
*******************************************************************************/
void Instance::getCast(Buffer *buffer)
{
SymbolType 	*castType = getType();
int 		i = 0;
	if ( !castType )
		{
		error("Instance getCast: bogus type");
		::printf("%s",postfix);
		buffer->appendString(postfix,0,0);
		}
	else
	if ( symbol && isMethod && (isLambda || reference) )
		buffer->appendString(symbol->getSignature(1),0,0);
	else {
		buffer->appendString(castType->name,0,0);
		for ( i = getDirect(); i > 0; i-- )
			buffer->appendString("*",0,0);
		for ( i = reference; i; i-- )
			buffer->appendString("&",0,0);
		}
}

/*******************************************************************************
        Get the compare method associated with this instance (if it has one)
*******************************************************************************/
Symbol *Instance::getCompareMethod(Instance *object)
{
Symbol 		*method = 0;
SymbolType 	*objectType = object->getType();
char 		*buffer = (char*)::alloca(50);
char 		*atBuffer = 0;
int 		i = 0;
	if ( !objectType )
		return 0;
	atBuffer = buffer;
	::sprintf(atBuffer,"compare(%s",objectType->name);
	atBuffer += (int)::strlen(atBuffer);
	i = howDirect();
	while ( i-- > 0 )
		{
		*atBuffer++ = '*';
		}
	::sprintf(atBuffer,")");
	objectType = getType();
	if ( !objectType )
		return 0;
	method = objectType->ownMethod(buffer);
	return method;
}

/*******************************************************************************
        Get the indirection of this instance, that is in effect, the number
        of asterisks prepended to it (as opposed to its native indirection,
        which is returned by howDirect().
*******************************************************************************/
int Instance::getDirect()
{
int 	direct = 0;
	if ( indirection )
		direct = indirection;
	else
	if ( express && express->subject )
		direct = express->subject->getDirect();
	return direct;
}

/*******************************************************************************
        Get the format associated with this instance (if there is one)
*******************************************************************************/
Instance *Instance::getFormat()
{
Instance 	*fmt = 0;
Symbol 		*sym = getSymbol();
SymbolType 	*typ = 0;
int 		direct = 0;
	if ( format )
		fmt = format;
	else
	if ( sym && sym->format )
		fmt = sym->format;
	else {
		typ = getType();
		direct = howDirect();
		if ( (typ == SymbolType::charType || typ == SymbolType::stringType) && direct == 1 )
			fmt = Tok::tawking->getInstance("%s");
		else
		if ( direct )
			fmt = SymbolType::pointerType->format;
		else	fmt = typ->format;
		}
	return fmt;
}

/*******************************************************************************
        Get the name of this instance (if it has one)
*******************************************************************************/
char *Instance::getName()
{
Symbol 	*expressSymbol = 0;
	if ( symbol )
		return symbol->name;
	if ( express )
		{
		expressSymbol = getSymbol();
		return expressSymbol->name;
		}
	if ( prefix )
		return prefix;
	if ( isComment )
		return "comment";
	return "no name";
}

/*******************************************************************************
        Get the name of this instance (if it has one) w/qualifications.
        This is only for debugging.
*******************************************************************************/
char *Instance::getQualifiedName()
{
Instance 	*instance = this;
char 		*qName = 0;
Symbol 		*qSymbol = getSymbol();
	while ( qSymbol )
		{
		if ( !qName )
			if ( qSymbol->isMethod )
				qName = ::concat(3,qSymbol->name," ",qSymbol->methodName);
			else	qName = qSymbol->name;
		else
		if ( qSymbol->isMethod )
			qName = ::concat(3,qSymbol->methodName,".",qName);
		else	qName = ::concat(3,qSymbol->name,".",qName);
		if ( instance = instance->parent )
			qSymbol = instance->getSymbol();
		else	break;
		}
	return qName;
}

/*******************************************************************************
        Get the subject of this instance
*******************************************************************************/
Instance *Instance::getSubject()
{
	if ( statement && statement->first )
		return statement->first->getSubject();
	if ( express )
		if ( express->subject )
			return express->subject->getSubject();
		else
		if ( express->object )
			return express->object->getSubject();
	return this;
}

/*******************************************************************************
        Get the Symbol associated with this instance
*******************************************************************************/
Symbol *Instance::getSymbol()
{
Instance 	*instance = 0;
	if ( symbol )
		return symbol;
	instance = getSubject();
	if ( instance && instance != this )
		return instance->getSymbol();
	return 0;
}

/*******************************************************************************
        Get the SymbolType associated with this instance
*******************************************************************************/
SymbolType *Instance::getType()
{
	if ( cast )
		return cast->getType();
	else
	if ( type )
		return type;
	else
	if ( symbol )
		return symbol->type;
	else
	if ( express )
		return express->getType();
	else
	if ( statement && statement->first )
		return statement->first->getType();
	else
	if ( isSelector )
		return SymbolType::selectorType;
	if ( isError )
		return SymbolType::nullType;
	return 0;
}

/*******************************************************************************
        Get the name of the SymbolType associated with this instance
*******************************************************************************/
char *Instance::getTypeName()
{
SymbolType 	*type = getType();
char 		*error = 0;
	if ( type )
		return type->name;
	else
	if ( prefix )
		return prefix;
	if ( symbol )
		error = ::concat(2,"ERROR: no type for ",symbol->name);
	else	error = "ERROR: no type or symbol";
	return error;
}

/*******************************************************************************
        Get the text associated with this instance
*******************************************************************************/
char *Instance::getValue()
{
	if ( isConstant )
		{
		if ( prefix )
			return prefix;
		else
		if ( symbol )
			return symbol->name;
		}
	else
	if ( express )
		if ( express->subject )
			return express->subject->getValue();
		else
		if ( express->object )
			return express->object->getValue();
	return 0;
}

/*******************************************************************************
        Returns true if its symbol is a method and has no body (ie. passed as
        a reference in a parameter)
*******************************************************************************/
int Instance::hasNoBody()
{
	if ( symbol )
		return noBody;
	else
	if ( express && express->subject && !express->object )
		return express->subject->hasNoBody();
	return 0;
}

/*******************************************************************************
        Get the level of indirection of this instance
*******************************************************************************/
int Instance::howDirect()
{
int 	direct = 0;
	if ( cast )
		{
		direct = cast->indirection;
		if ( cast->prefix )
			direct -= (int)::strlen(cast->prefix);
		}
	else
	if ( isExternalType || (isConstant && type == SymbolType::stringType) )
		return 1;
	else
	if ( express )
		{
		if ( express->verb && *express->verb->op == '?' )
			direct = express->object->howDirect();
		else
		if ( express->subject )
			direct = express->subject->howDirect();
		}
	else
	if ( symbol )
		if ( symbol->isArray )
			direct = symbol->isArray;
		else	direct = symbol->indirect;
	if ( indirection )
		direct -= indirection;
	if ( reference )
		direct += reference;
	if ( arrayRef )
		direct -= arrayRef;
	if ( direct < 0 )
		error("Too much indirection");
	return direct;
}

/*******************************************************************************
    Remove parent and insert parent as the first parameter.
    If this is an alias, the parent may be from a descendent class so have to
    walk the parent chain to find the right parent to use as parameter
*******************************************************************************/
void Instance::insertParentAsParameter()
{
Instance 	*argument = 0;
	if ( !symbol )
		return;
	//cout "insertParentAsParameter: " symbol.name:;
	argument = parent;
	while ( argument && argument->getType() != symbol->parentClass )
		argument = argument->parent;
	if ( argument && symbol->parentClass && symbol->parentClass->isC )
		{
		if ( parameters )
			parameters->insert(argument);
		else	addParameter(argument);
		setParent((Instance*)0);
		}
}

/*******************************************************************************
        Returns true if instance is OC
*******************************************************************************/
int Instance::instanceOC()
{
	if ( symbol && symbol->isOCfield )
		return 1;
	return 0;
}

/*******************************************************************************
        Returns true if this is a block statement
*******************************************************************************/
int Instance::isBlockStatement()
{
	return statement && statement->first && statement->first->block;
}

/*******************************************************************************
        Returns true if this instance refers to something virtuous
*******************************************************************************/
int Instance::isVirtuous()
{
SymbolType 	*myType = 0;
int 		direct = 0;
	if ( !virtuous )
		{
		direct = howDirect();
		if ( !(direct > 1 || (arrayRef && direct == 1) || (symbol && symbol->isConstructor)) )
			{
			if ( type == SymbolType::stringType && isConstant && parent )
				myType = parent->getType();
			else	myType = getType();
			if ( myType && myType->isVirtuous )
				virtuous = 1;
			}
		}
	return virtuous;
}

/*******************************************************************************
        Returns true if this instance is a void * pointer.
*******************************************************************************/
int Instance::isVoidPointer()
{
SymbolType 	*instanceType = getType();
Symbol 		*instanceSymbol = getSymbol();
	if ( instanceType )
		if ( (instanceType == SymbolType::voidType && instanceSymbol && instanceSymbol->indirect && howDirect() == 1) || ::compare(instanceType->name,"void*") == 0 )
			return 1;
	return 0;
}

/*******************************************************************************
        Set prefix to the mangled name for a method instance and return
        the mangled name
*******************************************************************************/
char *Instance::mangle()
{
int 		i = 0;
int 		length = 0;
int 		parameterDirect = 0;
DoubleLink 	*link = 0;
Instance 	*parameter = 0;
char 		*methodName = 0;
Symbol 		*pSymbol = 0;
	if ( symbol && symbol->isMethod )
		return symbol->gitMethodName();
	if ( !parameters )
		{
		methodName = ::concat(2,prefix,"()");
		goto endMangle;
		}
	else {
		length = (int)::strlen(prefix) + 2;
		for ( link = parameters->first; link; link = link->next )
			{
			parameter = (Instance*)link->value;
			if ( parameter->cast )
				{
				parameter = parameter->cast;
				parameterDirect = parameter->getDirect();
				}
			else	parameterDirect = parameter->howDirect();
			pSymbol = parameter->getSymbol();
			if ( parameter->isMethod && parameter->hasNoBody() )
				pSymbol->reference++;
			if ( pSymbol && (pSymbol->isLambda || (pSymbol->isMethod && pSymbol->reference)) )
				{
				methodName = pSymbol->getSignature();
				length += (int)::strlen(methodName);
				if ( pSymbol->isMethod )
					length++;
				// assumes method is a reference
				}
			else	length += (int)::strlen(parameter->getTypeName()) + parameterDirect;
			length++;
			// allow for comma
			}
		}
	methodName = (char*)::malloc(++length);
	::strcpy(methodName,prefix);
	::strcat(methodName,"(");
	for ( link = parameters->first; link; link = link->next )
		{
		parameter = (Instance*)link->value;
		if ( parameter->cast )
			{
			parameter = parameter->cast;
			parameterDirect = parameter->getDirect();
			}
		else	parameterDirect = parameter->howDirect();
		pSymbol = parameter->getSymbol();
		if ( pSymbol && (pSymbol->isLambda || (pSymbol->isMethod && pSymbol->reference)) )
			{
			::strcat(methodName,pSymbol->getSignature());
			if ( parameter->hasNoBody() )
				pSymbol->reference--;
			}
		else {
			::strcat(methodName,parameter->getTypeName());
			for ( i = parameterDirect; i > 0; i-- )
				::strcat(methodName,"*");
			}
		if ( link->next )
			::strcat(methodName,",");
		}
	::strcat(methodName,")");
endMangle:
	return methodName;
}

/*******************************************************************************
        Get a pointer name by appending instanceIndex to name.
*******************************************************************************/
char *Instance::pointerName()
{
char 	*text = 0;
	if ( symbol )
		{
		text = (char*)::malloc(::strlen(symbol->name) + 10);
		::sprintf(text,"%s%hdp",symbol->name,symbol->symbolIndex);
		}
	return text;
}

/*******************************************************************************
        Returns true if this instance is qualified. Returns false if this
        does not contain a symbol.
*******************************************************************************/
int Instance::qualified()
{
	return symbol && parent;
}

/******************************************************************************
	In the case of an alias method that has default parameter(s), insert the
    default parameters amongst the other parameters supplied in the method call.
    If the alias for a method with 3 parameters has one default, the alias is
    essentially a two parameter method calling the underlying method with 3
    parameters, including the default. Note that the non-default parameters
    have to be supplied in the order given by the underlying method.
******************************************************************************/
void Instance::setDefaults(Tawk *tok)
{
Symbol 			*arg = 0;
Instance 		*instance = 0;
int 			count = 0;
int 			defaults = 0;
DoubleLinkList 	*list = parameters;
	parameters = 0;
	if ( symbol->parameters )
		{
		/**********************************************************************
		Only need to add in default parameters if the number of parameters
		supplied is less than expected
		defaults    The number of parameters needed
		count       The number of default parameters added
		**********************************************************************/
		defaults = list ? symbol->parameters->length - list->length : symbol->parameters->length;
		if ( defaults > 0 )
			{
			symbol->parameters->entry = 0;
			while ( arg = (Symbol*)symbol->parameters->next() )
				{
				if ( list && count == defaults )
					break;
				if ( arg->isThis )
					{
					count++;
					continue;
					}
				if ( arg->isDefault )
					{
					count++;
					if ( arg->comment )
						{
						PLGitem 	*inst = 0;
						PLGitem 	*exp = tok->divertInput(arg->comment,"Expression");
						if ( exp )
							if ( inst = (PLGitem*)exp->children->get("instance") )
								instance = (Instance*)inst->itemValue;
							else	::fprintf(stderr,"Instance setDefaults: could not get expression instance for %s\n",arg->comment);
						else	::fprintf(stderr,"Instance setDefaults: alias argument parse failed for %s\n",arg->comment);
						}
					}
				else
				if ( list )
					instance = (Instance*)list->next();
				if ( instance )
					addParameter(instance);
				}
			}
		}
	if ( list )
		{
		while ( instance = (Instance*)list->next() )
			addParameter(instance);
		list->clear();
		}
	//cout "setDefaults added " count " default parameters " mangle():;
}

/*****************************************************************************
	Set indirection and reference levels in a copy of this instance and
    returns the copy
*****************************************************************************/
Instance *Instance::setIndirectItem(PLGitem *direct)
{
int 		i = 0;
char 		*atChar = direct->itemStart;
Instance 	*instance = new Instance(this);
	instance->indirection = 0;
	instance->setReference((unsigned int)0);
	for ( i = direct->itemLength; i; i--, atChar++ )
		if ( *atChar == '*' )
			instance->indirection++;
		else
		if ( *atChar == '&' )
			instance->reference++;
		else
		if ( *atChar == '^' )
			instance->isLambda = 1;
	return instance;
}

/******************************************************************************
	Sets the isUsed flag
******************************************************************************/
void Instance::setIsUsed()
{
Instance 	*piece = 0;
Instance 	*instance = 0;
	if ( isComment )
		return;
	for ( instance = this; instance; instance = instance->parent )
		if ( isDeclaration )
			{
			if ( express )
				{
				express->object->setIsUsed();
				if ( express->subject && express->subject->express )
					express->subject->setIsUsed();
				}
			continue;
			}
		else {
			Symbol 	*symbol = instance->symbol;
			if ( symbol )
				{
				if ( !symbol->isUsed )
					{
					//cout "setIsUsed for:",symbol.name:;
					symbol->isUsed = 1;
					}
				if ( parameters )
					{
					parameters->resetIterator();
					while ( piece = (Instance*)parameters->next() )
						piece->setIsUsed();
					}
				}
			else
			if ( express )
				{
				if ( express->subject )
					express->subject->setIsUsed();
				if ( express->object )
					express->object->setIsUsed();
				}
			else
			if ( statement )
				statement->setIsUsed();
			else
			if ( block && block->statements )
				{
				block->statements->resetIterator();
				while ( piece = (Instance*)block->statements->next() )
					piece->setIsUsed();
				}
			}
}

/******************************************************************************
	Sets the level
******************************************************************************/
void Instance::setLevel()
{
Instance 	*instance = 0;
	if ( !symbol )
		return;
	level = 0;
	for ( instance = this; instance; instance = instance->parent )
		level++;
}

/******************************************************************************
	Sets the parent checking to make sure there is no looping.
******************************************************************************/
void Instance::setParent(Instance *instance)
{
Instance 	*last = 0;
Instance 	*before = this;
	if ( !instance )
		parent = instance;
	else {
		for ( ; before; before = before->parent )
			if ( before == instance )
				{
				::fprintf(stderr,"Attempt to assign recursive parent\n");
				dump();
				return;
				}
			else
			if ( before->parent )
				{
				last = new Instance(before->parent);
				before->parent = last;
				}
			else	break;
		if ( !before->symbol || !before->symbol->isStatic )
			if ( before == this )
				parent = instance;
			else	before->parent = instance;
		}
	setLevel();
}

/******************************************************************************
	Sets the isReferenced flag for its symbol and parents, if any
******************************************************************************/
void Instance::setRefer()
{
Symbol 	*item = getSymbol();
	if ( !item )
		return;
	item->setRefer();
	if ( parent )
		parent->setRefer();
}

/******************************************************************************
	Sets the instance reference. This setter is here because there are a lot
    of places where reference is set and this gives us a place to debug them.
    Note reference is when you use & (as opposed to *);
******************************************************************************/
void Instance::setReference(int flag)
{
	reference = flag;
}

/*******************************************************************************
        Like getType but ignores alias type
*******************************************************************************/
SymbolType *Instance::sourceType()
{
	if ( isMethod && reference && instanceOC() )
		return SymbolType::selectorType;
	else
	if ( cast )
		return cast->getType();
	else
	if ( type )
		return type;
	else
	if ( isError )
		return SymbolType::nullType;
	else
	if ( express && !express->verb && express->subject )
		return express->subject->sourceType();
	else
	if ( !(express && express->verb && express->verb->compare("?") == 0) )
		{
		Symbol 	*symbol = getSymbol();
		if ( symbol )
			if ( symbol->isAlias )
				return symbol->source->type;
			else	return symbol->type;
		}
	return getType();
}

/*******************************************************************************
        Create a string representation of this instance best we can
*******************************************************************************/
char *Instance::toString()
{
Instance 	*instance = 0;
char 		*text = 0;
	if ( symbol )
		text = ::concat(2,"Symbol: ",symbol->name);
	else
	if ( express )
		text = ::concat(2,"Expression: ",express->toString());
	else
	if ( statement )
		text = ::concat(2,"Statement Type: ",statement->toString());
	else
	if ( prefix )
		if ( type )
			text = ::concat(3,type->name," ",prefix);
		else	text = prefix;
	if ( parameters )
		{
		int 	nextFlag = 0;
		text = ::concat(2,text,"(");
		parameters->entry = 0;
		while ( instance = (Instance*)parameters->next() )
			{
			if ( nextFlag++ )
				text = ::concat(2,text,",");
			text = ::concat(2,text,instance->toString());
			}
		text = ::concat(2,text,")");
		}
	if ( postfix )
		text = ::concat(2,text,postfix);
	return text;
}
