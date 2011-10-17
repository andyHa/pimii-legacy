#ifndef CALLCONTEXT_H
#define CALLCONTEXT_H

#include "vm/engine.h"
#include <stdlib.h>

#include <typeinfo>
#include <utility>

#ifdef __GNUC__
    // Utilizes GCCs name demangling stuff
    #include <cxxabi.h>

    template<typename T> QString demangle() {
        int status = -1;
        char* realName = NULL;
        const std::type_info& ti = typeid(T);
        realName = abi::__cxa_demangle(ti.name(), 0, 0, &status);
        if (status == 0) {
            QString result(realName);
            free(realName);
            return result;
        } else if (realName != NULL) {
            free(realName);
        }
        QString result(ti.name());
        return result;
    }
#else
    // Rely on the compiler to create sane type names..
    template<typename T> QString demangle() {
        const std::type_info& ti = typeid(T);
        QString result(ti.name());
        return result;
    }
#endif

/**
  Helper-class to construct lists.
  */
class ListBuilder {
private:
    Storage* storage;
    Atom start;
    Cons current;
public:
    ListBuilder(Storage* storage) :
        storage(storage),
        start(NIL),
        current(NULL) {}

    /**
      Appends a new value to the list.
      */
    void append(Atom cell) {
        if (isNil(start)) {
            std::pair<Atom, Cons> result = storage->cons(cell, NIL);
            current = result.second;
            start = result.first;
        } else {
            std::pair<Atom, Cons> result = storage->cons(cell, NIL);
            current->cdr = result.first;
            current = result.second;
        }
    }

    /**
      Returns the constructed list.
      */
    Atom getResult() {
        return start;
    }
};


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
            engine->panic(QString("The %2. argument of %1 must be a string! (%3:%4)").
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
            engine->panic(QString("The %2. argument of %1 must be a number! (%3:%4)").
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
            engine->panic(QString("The %2. argument of %1 must be a number! (%3:%4)").
                  arg(QString(bifName),
                      intToString(currentIndex),
                      QString(file),
                      intToString(line)));
        }
    }

    /**
      Fetches a reference argument.
      */
    QSharedPointer<Reference> fetchReference(const char* bifName,
                                             const char* file,
                                             int line,
                                             Atom* atom = NULL) const {
        Atom result = fetchArgument(bifName, file, line);
        if (atom != NULL) {
            *atom = result;
        }
        if (!isReference(result)) {
            engine->panic(QString("The %2. argument of %1 must be a reference! (%3:%4)").
                  arg(QString(bifName),
                      intToString(currentIndex),
                      QString(file),
                      intToString(line)));
        }
        return storage->getReference(result);
    }

    /**
      The method is kind of tricky, since it serves two purposes. First, it is
      a convenience method to return a pointer for an expected reference.
      Second it can also return the original Atom which
      contains the pointer. This can be done by passing a pointer to a local
      vairable into "ref". This is used to return the same value as it was
      passed in. We need the QSharedPointer for this, since we must not
      construct anotherone for the given R* pointer.
      */
    template<typename R> R* fetchRef(const char* bifName,
                                     const char* file,
                                     int line,
                                     Atom* atom = NULL)
    const {
        QSharedPointer<Reference> ref = fetchReference(bifName,
                                                       file,
                                                       line,
                                                       atom);
        R* result = dynamic_cast<R*>(ref.data());
        if (result == NULL) {
            engine->panic(QString("The %2. argument of %1 must be a '%5'! (%3:%4)").
                  arg(QString(bifName),
                      intToString(currentIndex),
                      QString(file),
                      intToString(line), demangle<R>()));
        }
        return result;
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
    inline void setReferenceResult(QSharedPointer<Reference> result) const {
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
