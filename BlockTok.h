class Symbol;
class DoubleLinkList;
class Instance;
class Statement;
/*******************************************************************************
        A Block contains a list of statements
*******************************************************************************/

class BlockTok
{
public:
Symbol *blockMethod;
DoubleLinkList *statements;
int modified;
int width;
struct 
	{
	unsigned int hasBreak:1;
	unsigned int indenting:1;
	unsigned int isArrayInitializer:1;
	unsigned int isBlock:1;
	unsigned int isLambda:1;
	unsigned int isMethodBlock:1;
	unsigned int isSwitch:1;
	};
static int indentCount;
BlockTok();
BlockTok(int i);
void add(Instance *i);
void add(char *s);
void add(Statement *s);
void check();
int getWidth();
void insert(Instance *i);
int length();
};
