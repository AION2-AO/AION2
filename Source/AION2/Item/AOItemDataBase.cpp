#include "Item/AOItemDataBase.h"

bool UAOItemDataBase::FindItemTemplate(int32 ItemId, FItemData& OutItemData) const
{
	if (const FItemData* FoundItem = ItemMap.Find(ItemId))
	{
		OutItemData = *FoundItem;
		return true;
	}
	return false;
}




