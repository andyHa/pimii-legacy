#ifndef WEBEXTENSION_H
#define WEBEXTENSION_H

#include "engineextension.h"
#include "vm/reference.h"
#include "callcontext.h"
#include "gui/webwindow.h"

#include <QtWebKit>


class WebElementReference : public Reference {
private:
    const QWebElement element;
    WebElementReference(const QWebElement& e) : element(e) {}
public:
    static QSharedPointer<Reference> make(const QWebElement& info) {
        return QSharedPointer<Reference>(new WebElementReference(info));
    }

    virtual QString toString() {
        return element.toOuterXml();
    }

    friend class WebExtension;
};

class WebWindowReference : public Reference {
private:
    WebWindow* window;
    WebWindowReference() : window(new WebWindow()){
       window->show();
       window->getWebView()->setHtml(
                   QString("<html><head></head><body></body></html>"));
    }
public:
    static QSharedPointer<Reference> make() {
        return QSharedPointer<Reference>(new WebWindowReference());
    }

    virtual QString toString() {
        return QString("[WebWindow]");
    }

    virtual ~WebWindowReference() {
    }

    friend class WebExtension;
};

class WebExtension : public EngineExtension
{
private:
    /**
      Opens a new WebWindow

        println := () -> WebWindow

     */
    static void bif_openWeb(const CallContext& ctx);

    /**
      Clears a given WebWindow

        clearWeb := (wnd : WebWindow) -> WebWindow

      */
    static void bif_clearWeb(const CallContext& ctx);

    /**
      Appends XML data to the body of the given web-window. If the first
      argument is omitted, a string value is popped from the stack, this
      permits patterns like: <div>Hello</div>; appendWeb(window);

        appendWeb := (xml : String?, wnd : WebWindow) -> WebWindow

      */
    static void bif_appendWeb(const CallContext& ctx);

    /**
      Querys for a single or a collection of web elements. A base either a
      WebWindow or a WebElement can be given.

        queryWeb := (path : String, parent : (WebWindow | WebElement)
                        -> (WebElement | List)

      */
    static void bif_queryWeb(const CallContext& ctx);

public:
    /**
      Contains the static instance of the extension. This is directly loaded
      by the Engine.
      */
    static WebExtension* INSTANCE;

    /**
      see: EngineExtension.name()
      */
    virtual QString name();

    /**
      see: EngineExtension.registerBuiltInFunctions()
      */
    virtual void registerBuiltInFunctions(Engine* engine);

};

#endif // WEBEXTENSION_H
