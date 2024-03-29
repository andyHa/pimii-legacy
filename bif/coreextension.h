#ifndef COREEXTENSION_H
#define COREEXTENSION_H

#include "engineextension.h"
#include "callcontext.h"
#include "tools/logger.h"


class CoreExtension : public EngineExtension
{
private:


    /**
      Contains the appliction logger
      */
    static Logger log;

    /**
      Sets an engine parameter.

        setValue := (name : Symbol, value : *) -> NIL

     */
    static void bif_setValue(const CallContext& ctx);

    /**
      Reads an engine parameter.

        getValue := (name : Symbol) -> *

     */
    static void bif_getValue(const CallContext& ctx);

    /**
      Returns all available engine parameters

        getValueKeys := () -> List

     */
    static void bif_getValueKeys(const CallContext& ctx);

    /**
      Returns the current time in milliseconds.

        time := () -> Number

     */
    static void bif_time(const CallContext& ctx);

    /**
      Returns the number of bits in a word for this machine.

        wordsize := () -> Number

     */
    static void bif_wordsize(const CallContext& ctx);

    /**
      Returns the version of pimii

        version := () -> String

     */
    static void bif_version(const CallContext& ctx);

    /**
      Prints the given argument to the console and appends a
      linebreak.

        log := (str : *) -> NIL

     */
    static void bif_log(const CallContext& ctx);

    /**
      Determines the type of the given argument and pushes an appropriate
      symbol on the stack.

        typeOf := (val : *) -> Symbol

     */
    static void bif_typeOf(const CallContext& ctx);

    /**
      Converts the given string into a symbol.

        symbol := (str : String) -> Symbol

     */
    static void bif_symbol(const CallContext& ctx);

    /**
      Converts the given argument into a string which will be
      pushed on the stack.

        asString := (val : *) -> String

     */
    static void bif_asString(const CallContext& ctx);

    /**
      Parses the given string into a number and pushes the
      result on the stack.

        parse := (str : String) -> (Integer|Double)

     */
    static void bif_parse(const CallContext& ctx);

    /**
      Creates a new array with the desired size.

        makeArray := (size : Integer) -> Array

     */
    static void bif_makeArray(const CallContext& ctx);

    /**
      Returns the value of the requested cell in the given array.

        readArray := (array : Array, pos : Integer) -> *

     */
    static void bif_readArray(const CallContext& ctx);

    /**
      Writes the requested cell in the given array.

        writeArray := (array : Array, pos : Integer, value : *) -> *

     */
    static void bif_writeArray(const CallContext& ctx);

    /**
      Compiles and executes the given file. If this file was already included
      nothing will happen.

        include := (file : String) -> NIL

      */
    static void bif_include(const CallContext& ctx);

    /**
      Compiles the given string and pushes the resulting op-codes on the stack
      (as list).

        compile := (code : String) -> List

      */
    static void bif_compile(const CallContext& ctx);

    /**
      Compiles and evaluates the given string

        eval := (code : String) -> *

      */
    static void bif_eval(const CallContext& ctx);

    /**
      Calls the given list as function with no arguments.

        call := (code : List) -> *

      */
    static void bif_call(const CallContext& ctx);

    /**
      Returns the length of a given string.

        strlen := (str : String) -> Integer

      */
    static void bif_strlen(const CallContext& ctx);

    /**
      Returns a substring of the given string.

        substr := (str : String, pos : Integer, length : Integer) -> String

      */
    static void bif_substr(const CallContext& ctx);

    /**
      Reads the given setting

        readSetting := (name : String) -> *

     */
    static void bif_readSetting(const CallContext& ctx);

    /**
      Writes the given setting

        writeSetting := (name : String, value : *) -> *

     */
    static void bif_writeSetting(const CallContext& ctx);

    /**
      Returns the numeric acsii code
      for the first character in the given String

        ascii := (char : String) -> Integer

     */
    static void bif_ascii(const CallContext& ctx);

    /**
      Returns the character for the given ascii code

        char := (code : Integer) -> String

     */
    static void bif_char(const CallContext& ctx);

    /**
      Fetches a QVariant from the given context.
      */
    static QVariant fetchQVariant(const CallContext& ctx,
                                  const char* bifName,
                                  const char* file,
                                  int line);

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
