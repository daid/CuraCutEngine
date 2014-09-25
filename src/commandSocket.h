#ifndef COMMAND_SOCKET_H
#define COMMAND_SOCKET_H

#include "utils/socket.h"
#include "utils/polygon.h"
#include "settings.h"

namespace cura {

class cutProcessor;
class CommandSocket
{
private:
    ClientSocket socket;
public:
    CommandSocket(int portNr);
    
    void handleIncommingData(cutProcessor* processor);
    
    void sendPolygons(const char* name, int object_index, Polygons& polygons);
    void sendProgress(float amount);
};

}//namespace cura

#endif//COMMAND_SOCKET_H
