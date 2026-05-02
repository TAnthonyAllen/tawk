class SymbolType;
class BlockTok;
class DoubleLinkList;
class PLGitem;
class Instance;
class InstanceTable;
/*******************************************************************************
        The Symbol class defines a declared symbol (when a symbol is referenced,
		that is captured in the Instance class).
*******************************************************************************/

class Symbol
{
public:
char *name;
char *methodName;
SymbolType *parentClass;
SymbolType *structType;
SymbolType *type;
Symbol *getter;
Symbol *setter;
Symbol *source;
BlockTok *block;
DoubleLinkList *directives;
DoubleLinkList *parameters;
struct 
	{
	unsigned int hasEllipsis:1;
	unsigned int indirect:4;
	unsigned int isAlias:1;
	unsigned int isArray:4;
	unsigned int isAssigned:1;
	unsigned int isButton:1;
	unsigned int isConst:1;
	unsigned int isConstructor:1;
	unsigned int isDefault:1;
	unsigned int isExtern:1;
	unsigned int isExtension:1;
	unsigned int isFlag:1;
	unsigned int isGetter:1;
	unsigned int isHidden:1;
	unsigned int isInitialized:1;
	unsigned int isInitializer:1;
	unsigned int isInline:1;
	unsigned int isItem:1;
	unsigned int isLambda:1;
	unsigned int isMethod:1;
	unsigned int isOCfield:1;
	unsigned int isOutlet:1;
	unsigned int isProper:1;
	unsigned int isSetter:1;
	unsigned int isStatic:1;
	unsigned int isThis:1;
	unsigned int isUsed:1;
	unsigned int isVirtual:1;
	unsigned int referred:1;
	unsigned int reference:4;
	unsigned int utilized:1;
	};
char *array;
union 
	{
	char *comment;
	PLGitem *commentItem;
	};
short symbolBitLength;
short symbolBitOffset;
short symbolIndex;
short symbolOffset;
Instance *format;
static int symbolCount;
Symbol(char *n);
Symbol(Symbol *src);
Symbol(char *n, char *t);
Symbol(char *n, SymbolType *t);
void addParameter(Symbol *arg);
void checkParameters(PLGitem *parameter);
char *displayName();
void dumpDefaultName();
void extendType();
char *externalMethodName();
char *getOCmethodName();
char *getSignature();
char *getSignature(int cppFlag);
char *gitMethodName();
void insertParameter(Symbol *arg);
Symbol *makeAlias(char *aliasName);
Symbol *makeSetter();
void mangle();
int matchMethod(Instance *target);
Symbol *matchType(SymbolType *match);
void pushParameters(InstanceTable *table);
void setIndirection(PLGitem *direct);
void setRefer();
int typeMatch(SymbolType *tYPE);
};
