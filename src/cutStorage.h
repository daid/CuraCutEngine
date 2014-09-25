#ifndef CUT_STORAGE_H
#define CUT_STORAGE_H

#include "settings.h"
#include "utils/polygon.h"

namespace cura {

class StorageObject : public SettingsBase
{
public:
    StorageObject(SettingsBase* base) : SettingsBase(base) {}

    Polygons polygons;
};

class CutStorage
{
public:
    std::vector<StorageObject> objects;
};

}

#endif//CUT_STORAGE_H
