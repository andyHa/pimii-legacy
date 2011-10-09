#ifndef ENGINEEXTENSION_H
#define ENGINEEXTENSION_H

#include "vm/reference.h"
#include "vm/engine.h"

#include <QtPlugin>
#include <vector>

class EngineExtension
{    
public:

    virtual ~EngineExtension() {}

    virtual QString name() = 0;

    virtual void registerBuiltInFunctions(Engine* engine) = 0;
};

Q_DECLARE_INTERFACE(EngineExtension,
                    "pimii.EngineExtension/1.0")

#endif // ENGINEEXTENSION_H
