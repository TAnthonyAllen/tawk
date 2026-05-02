class BaseHash;
class PLGset;
class SymbolType;
class PLGitem;
/******************************************************************************
	The following are added by PLG when parsing Tawk.g
******************************************************************************/

class Types : public BaseHash
{
public:
BaseHash *nameTable;
int listLength;
int typeCount;
int typeDebug;
PLGset *typesSet;
Types();
int add(char *name);
SymbolType *getFromItem(PLGitem *name);
int getNameIndex(char *name);
SymbolType *getType(char *name);
char *nameAt(int index);
void resetIsFilled();
void resetIsFlagged();
void setNameList();
};
