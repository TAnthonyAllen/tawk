#include <string.h>
#include <stdio.h>
#include "SymbolType.h"
#include "DoubleLinkList.h"
#include "DoubleLink.h"
#include "PLGitem.h"
#include "PLGset.h"
#include "BaseHash.h"
#include "Types.h"

/*******************************************************************************
	Constructor
*******************************************************************************/
Types::Types()
{
	listLength = 0;
	typeCount = 0;
	typeDebug = 0;
	nameTable = new BaseHash();
	typesSet = new PLGset();
	typesSet->name = "typesSet";
}

/******************************************************************************
	Add name to the nameTable if not already there and return associated index
******************************************************************************/
int Types::add(char *name)
{
DoubleLink 	*link = nameTable->find(name);
	if ( !link )
		link = nameTable->add(name);
	return link->linkOrder;
}

/*******************************************************************************
	Look up a type and return it if found.
*******************************************************************************/
SymbolType *Types::getFromItem(PLGitem *name)
{
SymbolType 	*type = (SymbolType*)get(name->string());
	name->unString();
	return type;
}

/*******************************************************************************
	Returns the index of the name passed in.
*******************************************************************************/
int Types::getNameIndex(char *name)
{
DoubleLink 	*link = nameTable->find(name);
	if ( link )
		return link->linkOrder;
	return 0;
}

/*******************************************************************************
	Factory to return a type if the table contains it, otherwise builds one
	adds it to the table and returns it.
*******************************************************************************/
SymbolType *Types::getType(char *name)
{
SymbolType 	*type = (SymbolType*)get(name);
	if ( !type )
		{
		type = new SymbolType(name);
		put(name,(void*)type);
		type->typeIndex = ++typeCount;
		typesSet->set((int)*name);
		}
	return type;
}

/******************************************************************************
	Convenience method to reset isFlagged flag used in InstanceTable
    findInstance() and in FormatC
******************************************************************************/
void Types::resetIsFilled()
{
SymbolType 	*type = 0;
	hashList->entry = 0;
	while ( type = (SymbolType*)SymbolType::types->hashList->next() )
		type->isFilled = 0;
}

/******************************************************************************
	Convenience method to reset isFlagged flag used in InstanceTable
    findInstance() and in FormatC
******************************************************************************/
void Types::resetIsFlagged()
{
SymbolType 	*type = 0;
	hashList->entry = 0;
	while ( type = (SymbolType*)SymbolType::types->hashList->next() )
		type->isFlagged = 0;
}
