#ifndef CALLCONTEXT_H
#define CALLCONTEXT_H

#include "vm/engine.h"

/**
  Used for calling the fetch argument functions of CallContext. This needs
  to be done in a macro since it calls other macros.
  */
#define BIF_INFO __FUNCTION__, __FILE__, __LINE__

/**
  An instance of BIF_Context is passed into a built in function. It provides
  access to the engine and the storage. Furthermore it provides convenience
  methods to check and fetch parameters and result values.

  Since this class has some inline functions, it is defined completely in the
  header file.
 */
class CallContext {
private:
    const Atom args;
    mutable Atom currentParam;
    mutable int currentIndex;
    mutable Atom result;
public:
    Engine* const engine;
    Storage* const storage;

    CallContext(Engine* engine, Storage* storage, Atom args)
        : args(args),
          currentParam(args),
          currentIndex(0),
          result(NIL),
          engine(engine),
          storage(storage) {}

    /**
      Checks if more arguments are available.
      */
    inline bool hasMoreArguments() const {
        return isCons(currentParam);
    }

    /**
      Fetches another argument. Raises an exception if no argument is
      available. bifName is used for the error-message.
      */
    Atom fetchArgument(const char* bifName,
                       const char* file,
                       int line) const {
        currentIndex++;
        if (!hasMoreArguments()) {
            if (currentIndex == 1) {
                engine->panic(QString("The built in function: %1 requires at least one argument! (%3:%4)").
                      arg(QString(bifName),
                          intToString(currentIndex),
                          QString(file),
                          intToString(line)));
            } else {
                engine->panic(QString("The built in function: %1 requires at least %2 argument(s)! (%3:%4)").
                      arg(QString(bifName),
                          intToString(currentIndex),
                          QString(file),
                          intToString(line)));
            }
        }
        Cons cons = storage->getCons(currentParam);
        currentParam = cons->cdr;
        return cons->car;
    }

    /**
      Fetches a string argument.
      */
    QString fetchString(const char* bifName,
                        const char* file,
                        int line) const {
        Atom result = fetchArgument(bifName, file, line);
        if (!isString(result)) {
            engine->panic(QString("The %2. argument of  %1 must be a string! (%3:%4)").
                  arg(QString(bifName),
                      intToString(currentIndex),
                      QString(file),
                      intToString(line)));
        }
        return storage->getString(result);
    }

    /**
      Fetches a integer argument.
      */
    int fetchNumber(const char* bifName,
                    const char* file,
                    int line) const {
        Atom result = fetchArgument(bifName, file, line);
        if (!isNumber(result)) {
            engine->panic(QString("The %2. argument of  %1 must be a number! (%3:%4)").
                  arg(QString(bifName),
                      intToString(currentIndex),
                      QString(file),
                      intToString(line)));
        }
        return storage->getNumber(result);
    }

    /**
      Fetches a double argument. If an integer was given, this is
      automatically converted.
      */
    double fetchDouble(const char* bifName,
                       const char* file,
                       int line) const {
        Atom result = fetchArgument(bifName, file, line);
        if (isNumber(result)) {
            return (double)storage->getNumber(result);
        } else if (isDecimalNumber(result)) {
            return storage->getDecimal(result);
        } else {
            engine->panic(QString("The %2. argument of  %1 must be a number! (%3:%4)").
                  arg(QString(bifName),
                      intToString(currentIndex),
                      QString(file),
                      intToString(line)));
        }
    }

    /**
      Fetches a reference argument.
      */
    Reference* fetchReference(const char* bifName,
                              const char* file,
                              int line) const {
        Atom result = fetchArgument(bifName, file, line);
        if (!isReference(result)) {
            engine->panic(QString("The %2. argument of  %1 must be a reference! (%3:%4)").
                  arg(QString(bifName),
                      intToString(currentIndex),
                      QString(file),
                      intToString(line)));
        }
        return storage->getReference(result);
    }

    /**
      Fetches a list argument.
      */
    Cons fetchCons(const char* bifName,
                   const char* file,
                   int line) const {
        Atom result = fetchArgument(bifName, file, line);
        if (!isCons(result)) {
            engine->panic(QString("The %2. argument of  %1 must be a reference! (%3:%4)").
                  arg(QString(bifName),
                      intToString(currentIndex),
                      QString(file),
                      intToString(line)));
        }
        return storage->getCons(result);
    }
    /**
      Fetches a list argument (without fetching the first cons).
      */
    Atom fetchList(const char* bifName,
                   const char* file,
                   int line) const {
        Atom result = fetchArgument(bifName, file, line);
        if (!isCons(result)) {
            engine->panic(QString("The %2. argument of  %1 must be a reference! (%3:%4)").
                  arg(QString(bifName),
                      intToString(currentIndex),
                      QString(file),
                      intToString(line)));
        }
        return result;
    }

    /**
      Called by the BIF to set the result value. If this is not called,
      NIL is implicitel returned.
      */
    inline void setResult(Atom result) const {
        this->result = result;
    }

    /**
      Convenience to return a string.
      */
    inline void setStringResult(const QString& result) const {
        this->result = storage->makeString(result);
    }

    /**
      Convenience to return a number.
      */
    inline void setNumberResult(int number) const {
        this->result = storage->makeNumber(number);
    }

    /**
      Convenience to return a double.
      */
    inline void setDoubleResult(double number) const {
        this->result = storage->makeDecimal(number);
    }

    /**
      Convenience to return a reference.
      */
    inline void setReferenceResult(Reference* result) const {
        this->result = storage->makeReference(result);
    }

    /**
      Used by the engine to obtain the result.
      */
    inline Atom getResult() const {
        return result;
    }
};

#endif // CALLCONTEXT_H
