#ifndef FFF_PROCESSOR_H
#define FFF_PROCESSOR_H

#include <algorithm>
#include <sstream>
#include "utils/gettime.h"
#include "utils/logoutput.h"
#include "utils/polygondebug.h"
#include "exportBase.h"
#include "commandSocket.h"
#include "cutStorage.h"

namespace cura {

//FusedFilamentFabrication processor.
class cutProcessor : public SettingsBase
{
private:
    ExportBase* exportObject;
    CommandSocket* commandSocket;

public:
    CutStorage storage;

    cutProcessor()
    {
        exportObject = NULL;
        commandSocket = NULL;
    }

    void setCommandSocket(CommandSocket* socket)
    {
        commandSocket = socket;
    }
    
    void processData()
    {
        int object_index = 0;
        for(StorageObject& object : storage.objects)
        {
            int tool_size_radius = 4000;
            Point min = object.polygons.min();
            Point max = object.polygons.max();
            Point offset = -(max - min) / 2 - min;
            offset += Point(object.getSettingInt("position.X"), object.getSettingInt("position.Y"));
            log("Offset: %d %d\n", offset.X, offset.Y);
            for(unsigned int n=0; n<object.polygons.size(); n++)
            {
                PolygonRef polygon = object.polygons[n];
                for(unsigned int m=0; m<polygon.size(); m++)
                {
                    polygon[m] = polygon[m] + offset;
                }
            }
            object.polygons = object.polygons.processEvenOdd();
            Polygons cut_paths = object.polygons.offset(tool_size_radius);
            
            Polygons outer_cut_lines = cut_paths.offset(-tool_size_radius, ClipperLib::jtRound);
            outer_cut_lines.add(cut_paths.offset(tool_size_radius, ClipperLib::jtRound));
            commandSocket->sendPolygons("cut_outside", object_index, outer_cut_lines);
            object_index++;
        }
    }
    
    void finalize()
    {
    }
};

}//namespace cura

#endif//FFF_PROCESSOR_H
