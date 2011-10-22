#include "webextension.h"

WebExtension* WebExtension::INSTANCE = new WebExtension();

QString WebExtension::name() {
    return QString("WebExtension");
}

void WebExtension::registerBuiltInFunctions(Engine* engine) {
    engine->makeBuiltInFunction("web::open", bif_openWeb);
    engine->makeBuiltInFunction("web::clear", bif_clearWeb);
    engine->makeBuiltInFunction("web::append", bif_appendWeb);
    engine->makeBuiltInFunction("web::query", bif_queryWeb);
}

void WebExtension::bif_openWeb(const CallContext& ctx) {
    ctx.setReferenceResult(WebWindowReference::make());
}

void WebExtension::bif_clearWeb(const CallContext& ctx) {
    Atom input;
    WebWindowReference* ww = ctx.fetchRef<WebWindowReference>(BIF_INFO, &input);
    ww->window->getWebView()->setHtml(
                QString("<html><head></head><body></body></html>"));
    ctx.setResult(input);
}

void WebExtension::bif_appendWeb(const CallContext& ctx) {
    Atom input;
    WebWindowReference* ww = ctx.fetchRef<WebWindowReference>(BIF_INFO, &input);
    QWebElement doc = ww->window->getWebView()->
            page()->mainFrame()->documentElement();
    doc.findFirst(QString("body")).appendInside(
                ctx.engine->toSimpleString(ctx.fetchArgument(BIF_INFO)));
    ctx.setResult(input);
}

void WebExtension::bif_queryWeb(const CallContext& ctx) {
    Reference* ref = ctx.fetchReference(BIF_INFO).data();
    QWebElement base;
    WebWindowReference* ww = dynamic_cast<WebWindowReference*>(ref);
    if (ww != NULL) {
        base = ww->window->getWebView()->
                page()->mainFrame()->documentElement();
    } else {
        WebElementReference* dom = dynamic_cast<WebElementReference*>(ref);
        if (dom != NULL) {
            base = dom->element;
        } else {
            ctx.engine->expect(
                        false,
                        "bif_queryWeb requires either a WebWindow or a DOMElement as first parameter",
                        __FILE__,
                        __LINE__);
        }
    }
    QWebElementCollection results = base.findAll(ctx.fetchString(BIF_INFO));
    if (results.count() == 0) {
        ctx.setResult(NIL);
    } else if (results.count() == 1) {
        ctx.setReferenceResult(WebElementReference::make(results.first()));
    } else {
        ListBuilder builder(ctx.storage);
        foreach (QWebElement e, results){
            builder.append(
                        ctx.storage->makeReference(
                            WebElementReference::make(e)));
        }
        ctx.setResult(builder.getResult());
    }
}
