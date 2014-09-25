#ifndef EXPORT_BASE_H
#define EXPORT_BASE_H

class ExportBase
{
public:
    ExportBase() {}
    virtual ~ExportBase() {}
    
    virtual void moveTo(Point position) = 0;
};

#endif//EXPORT_BASE_H
