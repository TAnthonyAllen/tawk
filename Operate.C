#include <string.h>
#include <stdio.h>
#include "KeyTable.h"
#include "Instance.h"
#include "Operate.h"
KeyTable *Operate::verbs;

/*******************************************************************************
        The following constructor is used when creating new overload operators.
*******************************************************************************/
Operate::Operate(char *v)
{
	method = 0;
	assign = 0;
	call = 0;
	comparison = 0;
	conjunction = 0;
	overload = 0;
	pointing = 0;
	question = 0;
	isRange = 0;
	unary = 0;
	op = v;
	rank = 99;
	Operate::verbs->add(op,(void*)this);
}

/*******************************************************************************
        The following constructor is used to create operators in setOperators()
        in the Tok parser.
*******************************************************************************/
Operate::Operate(char *v, int p)
{
	method = 0;
	assign = 0;
	call = 0;
	comparison = 0;
	conjunction = 0;
	overload = 0;
	pointing = 0;
	question = 0;
	isRange = 0;
	unary = 0;
	op = v;
	rank = p;
	Operate::verbs->add(op,(void*)this);
}

int Operate::compare(char *v)
{
	return ::strcmp(op,v);
}
