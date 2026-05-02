class PLGitem;
class BlockTok;
class Instance;
class Stak;
class Expression;
enum sType
	{
	CATCH,
	DELETE,
	DO,
	FINAL,
	FOR,
	IF,
	LABEL,
	LAMBDA,
	RETURN,
	SWITCH,
	WHILE,
	GOTO,
	THROW,
	TRY,
	NOTSPECIFIED
	};

/*******************************************************************************
        The Statement class
*******************************************************************************/

class Statement
{
public:
PLGitem *pointInCode;
BlockTok *block;
Instance *first;
Instance *second;
Instance *third;
Instance *fourth;
struct 
	{
	unsigned int branch:1;
	unsigned int debug:1;
	unsigned int indented:1;
	unsigned int noFallThru:1;
	unsigned int switching:1;
	};
sType statementType;
Stak *fallThruStack;
Stak *phiStack;
int lvl;
Statement();
Statement(sType type);
void add(BlockTok *b);
void add(char *s);
void add(Statement *s);
void add(Instance *i);
void add(Expression *e);
void addBreaks();
void check();
void checkBlock(Instance *i);
void convertSwitch();
BlockTok *getBlock();
Instance *makeIfExpression(Statement *line, Instance *switcher, Instance *lastCase);
void makeIfStatement(Instance *lastCase, BlockTok *body);
void setIsUsed();
char *toString(sType t);
char *toString();
};
