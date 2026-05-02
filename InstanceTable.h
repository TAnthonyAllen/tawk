class Stak;
class DoubleLinkList;
class Instance;
class SymbolType;
class BaseHash;
class Symbol;
/******************************************************************************
	The following are added by PLG when parsing Tawk.g
******************************************************************************/

class InstanceTable
{
public:
long count;
int debug;
int scope;
Stak *scopeStack;
DoubleLinkList *instances;
Instance *foundAncestor;
Instance *nullInstance;
SymbolType *presentClass;
BaseHash *globalFields;
InstanceTable();
void add(Instance *instance);
Instance *add(char *name, SymbolType *type);
void addGlobalField(char *text, Symbol *symbol);
void dump();
void dump(char *text);
void dumpGlobals();
void fillMaps();
Instance *find(char *name);
Symbol *findGlobalMethod(char *name);
Instance *findInstance(char *name);
Instance *findMethod(Instance *source);
Symbol *findSymbol(char *name);
Instance *getConverter(SymbolType *target, SymbolType *source);
void pop(char *text);
void printQualifiedName(Instance *instance);
void push(char *text);
};
