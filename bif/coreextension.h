#ifndef COREEXTENSION_H
#define COREEXTENSION_H

#include "engineextension.h"
#include "callcontext.h"

class CoreExtension : public EngineExtension
{
private:
    /**
      Prints the given argument to the console and appends a
      linebreak.
     */
    static void bif_println(const CallContext& ctx);


    /**
      Determines the type of the given argument and pushes an appropriate
      symbol on the stack.
     */
    static void bif_typeOf(const CallContext& ctx);

    /**
      Converts the given string into a symbol.
     */
    static void bif_symbol(const CallContext& ctx);

    /**
      Converts the given argument into a string which will be
      pushed on the stack.
     */
    static void bif_asString(const CallContext& ctx);

    /**
      Parses the given string into a number and pushes the
      result on the stack.
     */
    static void bif_parse(const CallContext& ctx);

    /**
      Compiles and executes the given file. If this file was already included
      nothing will happen.
      */
    static void bif_include(const CallContext& ctx);

    /**
      Compiles the given string and pushes the resulting op-codes on the stack
      (as list).
      */
    static void bif_compile(const CallContext& ctx);

    /**
      Compiles and evaluates the given string
      */
    static void bif_eval(const CallContext& ctx);

    /**
      Calls the given list as function with no arguments.
      */
    static void bif_call(const CallContext& ctx);

    /**
      Returns the length of a given string.
      */
    static void bif_strlen(const CallContext& ctx);

    /**
      Returns a substring of the given string.
      */
    static void bif_substr(const CallContext& ctx);

public:

    /**
      Contains the static instance of the extension. This is directly loaded
      by the Engine.
      */
    static CoreExtension* INSTANCE;

    /**
      see: EngineExtension.name()
      */
    virtual QString name();

    /**
      see: EngineExtension.registerBuiltInFunctions()
      */
    virtual void registerBuiltInFunctions(Engine* engine);
};

#endif // COREEXTENSION_H
