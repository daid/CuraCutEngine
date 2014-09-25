#include "utils/logoutput.h"
#include "commandSocket.h"
#include "cutProcessor.h"

namespace cura {

#define CURA_IDENTIFIER "CuraEngine"
const static int CMD_REQUEST_IDENTIFIER = 0x00100000;
const static int CMD_IDENTIFIER_REPLY = 0x00100001;
const static int CMD_REQUEST_VERSION = 0x00100002;
const static int CMD_VERSION_REPLY = 0x00100003;

const static int CMD_SETTING = 0x00100004;
const static int CMD_MATRIX = 0x00300002;
const static int CMD_OBJECT_COUNT = 0x00300003;
const static int CMD_OBJECT_LIST = 0x00200000;
const static int CMD_MESH_LIST = 0x00200001;
const static int CMD_VERTEX_LIST = 0x00200002;
const static int CMD_NORMAL_LIST = 0x00200003;
const static int CMD_PROCESS_MESH = 0x00300000;

const static int CMD_PROGRESS_REPORT = 0x00300001;
const static int CMD_OBJECT_PRINT_TIME = 0x00300004;
const static int CMD_OBJECT_PRINT_MATERIAL = 0x00300005;
const static int CMD_LAYER_INFO = 0x00300007;
const static int CMD_POLYGON = 0x00300006;

CommandSocket::CommandSocket(int portNr)
{
    socket.connectTo("127.0.0.1", portNr);
}

void CommandSocket::handleIncommingData(cutProcessor* processor)
{
    FMatrix3x3 matrix;
    
    while(true)
    {
        int command = socket.recvInt32();
        int dataSize = socket.recvInt32();
        if (dataSize < 0)
            break;
        switch(command)
        {
        case CMD_REQUEST_IDENTIFIER:
            socket.sendInt32(CMD_IDENTIFIER_REPLY);
            socket.sendInt32(strlen(CURA_IDENTIFIER) + 1);
            socket.sendAll(CURA_IDENTIFIER, strlen(CURA_IDENTIFIER) + 1);
            break;
        case CMD_SETTING:
            {
                char buffer[dataSize+1];
                buffer[dataSize] = '\0';
                socket.recvAll(buffer, dataSize);
                char* value = (buffer + strlen(buffer)) + 1;
                if ((value - buffer) < dataSize)
                {
                    if (processor->storage.objects.size() > 0)
                    {
                        processor->storage.objects.back().setSetting(buffer, value);
                    }else{
                        processor->setSetting(buffer, value);
                    }
                }
            }
            break;
        case CMD_MATRIX:
            {
                for(int x=0; x<3; x++)
                    for(int y=0; y<3; y++)
                        matrix.m[x][y] = socket.recvFloat32();
            }
            break;
        case CMD_OBJECT_COUNT:
            socket.recvInt32(); //Number of objects
            break;
        case CMD_OBJECT_LIST:
            socket.recvInt32(); //Number of following CMD_MESH_LIST commands
            processor->storage.objects.emplace_back(processor);
            break;
        case CMD_MESH_LIST:
            socket.recvInt32(); //Number of following CMD_?_LIST commands that fill this mesh with data
            break;
        case CMD_VERTEX_LIST:
            if (processor->storage.objects.size() > 0)
            {
                Polygons& polygons = processor->storage.objects.back().polygons;
                PolygonRef polygon = polygons.newPoly();
                for(int n=0; n<dataSize / int(sizeof(float) * 2); n++)
                {
                    FPoint3 mm_p3;
                    mm_p3.x = socket.recvFloat32();
                    mm_p3.y = socket.recvFloat32();
                    mm_p3.z = 0.0;
                    Point3 micron_p3 = matrix.apply(mm_p3);
                    polygon.add(Point(micron_p3.x, micron_p3.y));
                }
            }else{
                for(int n=0; n<dataSize; n++)
                    socket.recvAll(&command, 1);
            }
            break;
        case CMD_PROCESS_MESH:
            processor->processData();
            socket.close();
            break;
        default:
            logError("Unknown command: %04x (%i)\n", command, dataSize);
            for(int n=0; n<dataSize; n++)
                socket.recvAll(&command, 1);
            break;
        }
    }
}

void CommandSocket::sendPolygons(const char* name, int object_index, Polygons& polygons)
{
    int size = (strlen(name) + 1) + 3 * 4 + polygons.size() * 4;
    for(unsigned int n=0; n<polygons.size(); n++)
        size += polygons[n].size() * sizeof(Point);
    if (polygons.size() < 1)
        return;

    socket.sendInt32(CMD_POLYGON);
    socket.sendInt32(size);
    socket.sendAll(name, strlen(name) + 1);
    socket.sendInt32(object_index);
    socket.sendInt32(0);
    socket.sendInt32(polygons.size());
    for(unsigned int n=0; n<polygons.size(); n++)
    {
        socket.sendInt32(polygons[n].size());
        socket.sendAll(polygons[n].data(), polygons[n].size() * sizeof(Point));
    }
}

void CommandSocket::sendProgress(float amount)
{
    socket.sendInt32(CMD_PROGRESS_REPORT);
    socket.sendInt32(4);
    socket.sendFloat32(amount);
}

}//namespace cura
