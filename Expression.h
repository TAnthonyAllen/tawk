class Operate;
class Instance;
class BaseHash;
class FormatC;
class SymbolType;
/*******************************************************************************
	The Expression class holds an operation triplet, that is, an operator
	and two Instances (each of which might be an expression in turn)
*******************************************************************************/

class Expression
{
public:
Operate *verb;
Instance *subject;
Instance *object;
struct 
	{
	unsigned int buttoned:1;
	unsigned int checked:1;
	unsigned int full:1;
	unsigned int hasParens:1;
	unsigned int reordered:1;
	};
static BaseHash *CompareOperators;
Expression();
Expression(Instance *s, Instance *o, Operate *v);
Expression(Instance *s, Instance *o, char *v);
void check();
void checkCast(Instance *target);
void checkExpression(FormatC *formatter);
Instance *convertBoolean(Instance *item, int setterFlag);
void dump();
SymbolType *getType();
void handleBooleans();
void handleButtons();
void reorder();
char *toString();
};
