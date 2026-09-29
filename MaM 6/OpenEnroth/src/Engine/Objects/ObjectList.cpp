#include "Engine/Objects/ObjectList.h"

#include "Engine/Graphics/Sprites.h"
#include "Engine/Objects/Mm6Ids.h"

#include "Utility/GameVariant.h"

ObjectList *pObjectList;

unsigned int ObjectList::ObjectIDByItemID(SpriteId uItemID) {
    if (isMm6())
        uItemID = mm6ObjectSprite(uItemID);
    for (size_t i = 0; i < pObjects.size(); i++)
        if (uItemID == pObjects[i].uObjectID)
            return i;
    return 0;
}

void ObjectList::InitializeSprites() {
    for (const ObjectDesc &object : pObjects) {
        pSpriteFrameTable->InitializeSprite(object.uSpriteID);
    }
}
