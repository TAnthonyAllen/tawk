class BaseHash;
class DoubleLinkList;
class Buffer;
class Symbol;
class Instance;
class Stak;
class Types;
class PLGitem;
class Tawk;
/******************************************************************************
	The SymbolType defines a tawk class
******************************************************************************/

class SymbolType
{
public:
char *name;
char *comment;
char *constructor;
char *dotHname;
char *nameSpace;
struct 
	{
	unsigned int addClassNameToMethods:1;
	unsigned int autoGetSet:1;
	unsigned int classOK:1;
	unsigned int composed:1;
	unsigned int debug:1;
	unsigned int hasDescendentTypes:1;
	unsigned int hasExtern:1;
	unsigned int hasInitializer:1;
	unsigned int hasLambda:1;
	unsigned int hasMethods:1;
	unsigned int hasOC:1;
	unsigned int hasTypedef:1;
	unsigned int isAliasType:1;
	unsigned int isAtomic:1;
	unsigned int isC:1;
	unsigned int isChar:1;
	unsigned int isDeclared:1;
	unsigned int isDirect:1;
	unsigned int isExternal:1;
	unsigned int isFilled:1;
	unsigned int isFlagged:1;
	unsigned int isGlobal:1;
	unsigned int isLocal:1;
	unsigned int isNumber:1;
	unsigned int isOC:1;
	unsigned int isReferenced:1;
	unsigned int isTemplate:1;
	unsigned int isVirtuous:1;
	unsigned int mustDeclare:1;
	unsigned int nameLess:1;
	unsigned int noClassForward:1;
	unsigned int noDotH:1;
	unsigned int noSign:1;
	unsigned int proper:1;
	unsigned int structure:4;
	unsigned int typesFilled:1;
	};
#define isBoolean(button) (button == 1)
#define isEnumerator(button) (button == 2)
#define isProtocol(button) (button == 3)
#define isType(button) (button == 4)
#define isStruct(button) (button == 5)
#define isUnion(button) (button == 6)
BaseHash *components;
BaseHash *componentFields;
BaseHash *methods;
DoubleLinkList *componentTypes;
DoubleLinkList *descendentTypes;
DoubleLinkList *overloads;
DoubleLinkList *protocols;
DoubleLinkList *sortedComponents;
DoubleLinkList *sortedMethods;
Buffer *codeBuffer;
Symbol *initializer;
SymbolType *parent;
int lastBitOffset;
int lastOffset;
int levelMax;
int symbolMapped;
int typeIndex;
int typeSize;
Instance *format;
Stak *aliasStack;
static Types *types;
static SymbolType *buttonType;
static SymbolType *charType;
static SymbolType *doubleType;
static SymbolType *floatType;
static SymbolType *globalType;
static SymbolType *idType;
static SymbolType *internalType;
static SymbolType *intType;
static SymbolType *longType;
static SymbolType *nullType;
static SymbolType *ocRoutines;
static SymbolType *ocStringType;
static SymbolType *pointerType;
static SymbolType *selectorType;
static SymbolType *shortType;
static SymbolType *stringRoutines;
static SymbolType *stringType;
static SymbolType *voidType;
static BaseHash *ocSymbols;
static DoubleLinkList *globalList;
SymbolType(char *n);
SymbolType(SymbolType *type);
void add(Symbol *item);
Symbol *add(char *item, SymbolType *type);
Symbol *add(char *item, char *t);
void addAncestorTypes();
void addComponentField(char *text, Symbol *item);
void addComponentType(Symbol *item);
void addMethod(Symbol *method);
void addProtocol(SymbolType *p);
void checkGetterSetter(Symbol *method);
void debugComponent(char *text);
void dump();
void dumpFields();
void dumpSymbol(Symbol *symbol);
Instance *fillComponentFields(char *text);
static SymbolType *find(char *n);
Symbol *findAliasTarget(char *text);
Instance *findComponent(char *text, Instance *ancestor);
Symbol *findField(char *text);
Instance *findFieldInstance(char *text);
Symbol *findMethod(Instance *source);
Instance *findMethodInstance(Instance *source);
Symbol *get(PLGitem *item);
unsigned int getAutoGetSet();
Symbol *getConverter(SymbolType *target, SymbolType *source);
Symbol *getLocal(char *name);
Symbol *getMethod(char *name);
static SymbolType *getType(char *n);
Symbol *handleAliasParameters(Symbol *symbol, char *aliasName, PLGitem *body, Tawk *tok);
int hasParent(SymbolType *type);
void makeAlias(char *aliasName, char *target, PLGitem *body, Tawk *tok);
int matches(SymbolType *type);
void mention();
void overload(char *op, char *method);
char *overloaded(char *op);
Symbol *ownMethod(char *name);
void setOverloadTable();
void setParent(SymbolType *type);
void setRefer();
static void setTypeTable();
};
