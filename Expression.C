#include <string.h>
#include <stdio.h>
#include "StringRoutines.h"
#include "PLGparse.h"
#include "BaseHash.h"
#include "Types.h"
#include "Symbol.h"
#include "KeyTable.h"
#include "FormatC.h"
#include "SymbolType.h"
#include "Operate.h"
#include "Instance.h"
#include "Tawk.h"
#include "Tok.h"
#include "Expression.h"
BaseHash *Expression::CompareOperators;

Expression::Expression()
{
	verb = 0;
	subject = 0;
	object = 0;
	buttoned = 0;
	checked = 0;
	full = 0;
	hasParens = 0;
	reordered = 0;
	// this does not do anything except initialize contents to null
}

Expression::Expression(Instance *s, Instance *o, Operate *v)
{
	buttoned = 0;
	checked = 0;
	full = 0;
	hasParens = 0;
	reordered = 0;
	subject = s;
	object = o;
	verb = v;
	if ( subject && object && verb )
		full = 1;
}

Expression::Expression(Instance *s, Instance *o, char *v)
{
	verb = 0;
	buttoned = 0;
	checked = 0;
	full = 0;
	hasParens = 0;
	reordered = 0;
	subject = s;
	object = o;
	if ( v )
		verb = (Operate*)Operate::verbs->find(v);
	else	verb = 0;
	if ( subject && object && verb )
		full = 1;
}

/*******************************************************************************
        Runs all fields thru a sanity check
*******************************************************************************/
void Expression::check()
{
	if ( subject )
		subject->check();
	if ( object )
		object->check();
}

/*******************************************************************************
        Checks casting of ?: expressions
*******************************************************************************/
void Expression::checkCast(Instance *target)
{
	if ( verb && verb->compare("?") == 0 )
		{
		object->express->checkCast(target);
		return;
		}
	if ( !verb || verb->compare(":") != 0 )
		return;
	subject = subject->checkCast(target);
	object = object->checkCast(target);
}

/*****************************************************************************
	Checks the expression for errors
*****************************************************************************/
void Expression::checkExpression(FormatC *formatter)
{
SymbolType 	*subjectType = 0;
SymbolType 	*objectType = 0;
	if ( checked )
		return;
	if ( subject )
		subjectType = subject->getType();
	if ( object )
		objectType = object->getType();
	if ( subjectType == SymbolType::buttonType || objectType == SymbolType::buttonType )
		if ( formatter->jitting )
			handleBooleans();
		else	handleButtons();
	if ( subject )
		{
		if ( !subject->symbol && !subject->isConstant && !subject->express )
			subject->error("expected a symbol or expression");
		else
		if ( !subjectType )
			subject->error("expression subject is missing type");
		if ( subject->isError )
			goto endCheckExpress;
		if ( formatter->jitting && subject->symbol && subject->symbol->structType && isBoolean(subject->symbol->structType->structure) )
			handleBooleans();
		}
	if ( object && verb )
		{
		if ( subject && subject->isConstant && verb->assign )
			subject->error("Invalid assignment subject");
		if ( object->isError )
			goto endCheckExpress;
		objectType = object->getType();
		if ( object->express )
			object->express->checkExpression(formatter);
		formatter->convert(this);
		if ( subject )
			subjectType = subject->getType();
		objectType = object->getType();
		// types might have been change by convert
		if ( verb->compare("&&") == 0 || verb->compare("||") == 0 )
			goto endCheckExpress;
		/***********************************************************************
		Check subject for setter. There is no conversion to setter if the
		enclosing method is a setter, getter or constructor.
		***********************************************************************/
		if ( !buttoned && verb->assign )
			{
			SymbolType 	*enclosingType = 0;
			if ( formatter->enclosingMethod )
				enclosingType = formatter->enclosingMethod->parentClass;
			if ( subject )
				{
				Symbol 	*method = subject->getSymbol();
				if ( method && *verb->op == '=' )
					{
					if ( method->isGetter )
						{
						/***********************************************************
						Replaces the getter method w/getter target symbol
						Since cannot have a getter as the subject of an assign
						***********************************************************/
						subject->symbol = method = method->getter;
						subject->isMethod = 0;
						}
					if ( method = method->setter )
						{
						Instance 	*methodParent = subject->findGetterOrSetter(method->name);
						if ( subject->symbol && method && method != formatter->enclosingMethod && (!formatter->enclosingMethod || (!formatter->enclosingMethod->isSetter && !formatter->enclosingMethod->isConstructor && !formatter->enclosingMethod->isGetter)) && (method->parentClass == enclosingType || methodParent) )
							{
							Instance 	*instance = 0;
							instance = new Instance(method);
							/*******************************************************
							The following caveat is added so that a setter can
							apply on a component field
							*******************************************************/
							if ( methodParent )
								instance->setParent(methodParent);
							// Test this Not sure if it works: if method.parentClass.isC && subject.parent insertParentAsParameter();
							instance->addParameter(object->checkCast(subject));
							object = 0;
							verb = 0;
							full = 0;
							subject = instance;
							}
						else
						if ( method = object->getSymbol() )
							if ( method = method->getter )
								{
								methodParent = object->findGetterOrSetter(method->name);
								if ( method->parentClass == enclosingType || methodParent )
									{
									object->symbol = method;
									object->express = 0;
									object->isMethod = 1;
									object->setParent((Instance*)0);
									if ( methodParent )
										object->setParent(methodParent);
									}
								}
						}
					}
				}
noSetter:
			if ( object && object->express )
				object->express->checkCast(subject);
			}
		/***********************************************************************
		Check object for up cast and boolean reference
		***********************************************************************/
		if ( object )
			{
			if ( !object->cast && subjectType && objectType && subjectType->hasParent(objectType) )
				object = object->checkCast(subject);
			if ( formatter->jitting && object->symbol && isBoolean(object->symbol->type->structure) )
				handleBooleans();
			}
		}
endCheckExpress:
	checked = 1;
}

/*****************************************************************************
	Converts this expression into a call to .
*****************************************************************************/
Instance *Expression::convertBoolean(Instance *item, int setterFlag)
{
Instance 	*instance = 0;
Instance 	*newSubject = 0;
Symbol 		*handler = 0;
Symbol 		*itemObject = 0;
SymbolType 	*uChar = SymbolType::types->getType("unsigned char");
int 		buttonFlag = 0;
	if ( item->symbol->type == SymbolType::buttonType )
		buttonFlag = 1;
	if ( item->symbol && item->symbol->isItem )
		{
		// The following sets instance as a pointer to the item parent object cast as unsigned char*
		if ( !(instance = item->parent) )
			{
			itemObject = new Symbol("this",item->symbol->parentClass);
			instance = new Instance(itemObject);
			}
		instance->cast = new Instance(uChar);
		instance->cast->indirection = 1;
		instance->cast->isCast = 1;
		if ( setterFlag )
			if ( buttonFlag )
				handler = SymbolType::globalType->ownMethod("setButton");
			else	handler = SymbolType::globalType->ownMethod("setBoolean");
		else
		if ( buttonFlag )
			handler = SymbolType::globalType->ownMethod("buttonIsSet");
		else	handler = SymbolType::globalType->ownMethod("booleanIsSet");
		newSubject = new Instance(handler);
		newSubject->isMethod = 1;
		newSubject->addParameter(instance);
		if ( buttonFlag )
			instance = Tok::tawking->getInstance(::toStringFromInt((int)item->symbol->source->symbolOffset));
		else	instance = Tok::tawking->getInstance(::toStringFromInt((int)item->symbol->symbolOffset));
		instance->isConstant = 1;
		instance->type = SymbolType::intType;
		newSubject->addParameter(instance);
		if ( buttonFlag )
			instance = Tok::tawking->getInstance(::toStringFromInt((int)item->symbol->source->symbolBitOffset));
		else	instance = Tok::tawking->getInstance(::toStringFromInt((int)item->symbol->symbolBitOffset));
		instance->isConstant = 1;
		instance->type = SymbolType::intType;
		newSubject->addParameter(instance);
		if ( buttonFlag )
			{
			instance = Tok::tawking->getInstance(::toStringFromInt((int)item->symbol->source->symbolBitLength));
			instance->isConstant = 1;
			instance->type = SymbolType::intType;
			newSubject->addParameter(instance);
			instance = Tok::tawking->getInstance(item->symbol->array);
			instance->isConstant = 1;
			instance->type = SymbolType::intType;
			newSubject->addParameter(instance);
			}
		else
		if ( setterFlag )
			newSubject->addParameter(object);
		return newSubject;
		}
	return item;
}

/*******************************************************************************
        Debugging routine
*******************************************************************************/
void Expression::dump()
{
	::printf("%s\n",toString());
}

/*******************************************************************************
        Get the SymbolType associated with this expression
*******************************************************************************/
SymbolType *Expression::getType()
{
SymbolType 	*subjectType = 0;
SymbolType 	*objectType = 0;
	if ( object )
		objectType = object->getType();
	if ( verb )
		{
		if ( verb->comparison )
			if ( object->express && object->express->verb && object->express->verb->compare("?") == 0 )
				return object->getType();
			else	return SymbolType::intType;
		if ( verb->compare("?") == 0 )
			return objectType;
		}
	if ( subject )
		{
		subjectType = subject->getType();
		if ( verb && subjectType == objectType && subject->howDirect() && (verb->compare("-") == 0 || verb->compare("+") == 0) )
			return SymbolType::intType;
		else	return subjectType;
		}
	return objectType;
}

/*****************************************************************************
	Handle boolean and button references in the expression. Only called when
    jitting to handle addressing and bit manipulation.
*****************************************************************************/
void Expression::handleBooleans()
{
int 		setterFlag = 0;
Instance 	*newItem = 0;
	if ( object && object->symbol && ((object->symbol->structType && isBoolean(object->symbol->structType->structure)) || object->symbol->type == SymbolType::buttonType) )
		object = convertBoolean(object,0);
	if ( subject && subject->symbol && ((subject->symbol->structType && isBoolean(subject->symbol->structType->structure)) || subject->symbol->type == SymbolType::buttonType) )
		{
		if ( verb && verb->assign )
			setterFlag = 1;
		newItem = convertBoolean(subject,setterFlag);
		if ( subject != newItem )
			subject = newItem;
		}
	if ( setterFlag )
		{
		object = 0;
		verb = 0;
		}
}

/*****************************************************************************
	Handle button references in the expression. Note, buttons have source
    set but they are not aliases.
*****************************************************************************/
void Expression::handleButtons()
{
Symbol 		*button = 0;
Instance 	*buttonInstance = 0;
	if ( verb && verb->compare("=") == 0 )
		{
		button = subject->symbol;
		if ( button && button->type == SymbolType::buttonType )
			{
			subject = new Instance(subject);
			subject->symbol = button->source;
			buttoned = 1;
			if ( (object->isConstant && ::compare(object->prefix,"1") == 0) || (object->symbol && ::compare(object->symbol->name,"1") == 0) )
				{
				object = new Instance(object);
				object->type = SymbolType::intType;
				object->symbol = 0;
				object->prefix = button->array;
				object->isConstant = 1;
				}
			else {
				SymbolType 	*objectType = object->getType();
				if ( !objectType->isNumber )
					{
					object = new Instance(object);
					object->type = SymbolType::intType;
					object->symbol = 0;
					object->prefix = "0";
					object->isConstant = 1;
					}
				}
			}
		if ( object->symbol )
			{
			button = object->getSymbol();
			if ( button && button->isButton )
				{
				buttonInstance = new Instance(button);
				buttonInstance->isMethod = 1;
				object = new Instance(object);
				object->symbol = button->source;
				buttonInstance->addParameter(object);
				object = buttonInstance;
				}
			}
		}
	else {
		if ( subject && subject->symbol )
			{
			button = subject->getSymbol();
			if ( button && button->isButton )
				{
				buttonInstance = new Instance(button);
				buttonInstance->isMethod = 1;
				subject = new Instance(subject);
				subject->symbol = button->source;
				buttonInstance->addParameter(subject);
				subject = buttonInstance;
				buttoned = 1;
				}
			}
		if ( object && object->symbol )
			{
			button = object->getSymbol();
			if ( button && button->isButton )
				{
				buttonInstance = new Instance(button);
				buttonInstance->isMethod = 1;
				object = new Instance(object);
				object->symbol = button->source;
				buttonInstance->addParameter(object);
				object = buttonInstance;
				}
			}
		}
}

/*******************************************************************************
        Rearranges the expression to deal w/operator precedence
*******************************************************************************/
void Expression::reorder()
{
	if ( full && !reordered && object->express && object->express->full && !object->express->hasParens && verb->rank < object->express->verb->rank )
		{
		Instance 	*oldObject = object;
		Operate 	*oldVerb = verb;
		verb = object->express->verb;
		object = oldObject->express->object;
		oldObject->express->object = oldObject->express->subject;
		oldObject->express->subject = subject;
		oldObject->express->verb = oldVerb;
		subject = oldObject;
		reordered = 1;
		}
}

/*******************************************************************************
        Create a string representation of this expression
*******************************************************************************/
char *Expression::toString()
{
char 	*text = 0;
	if ( full )
		text = ::concat(5,subject->toString()," ",verb->op," ",object->toString());
	else
	if ( subject )
		text = subject->toString();
	else
	if ( object )
		text = object->toString();
	if ( hasParens )
		text = ::concat(3,"(",text,")");
	return text;
}
