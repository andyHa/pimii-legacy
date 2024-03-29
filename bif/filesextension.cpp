#include "filesextension.h"


FilesExtension* FilesExtension::INSTANCE = new FilesExtension();

QString FilesExtension::name() {
    return QString("FilesExtension");
}

void FilesExtension::registerBuiltInFunctions(Engine* engine) {

    engine->makeBuiltInFunction("fs::getFile", bif_getFile);
    engine->makeBuiltInFunction("fs::home", bif_getHome);
    engine->makeBuiltInFunction("fs::getPath", bif_getPath);
    engine->makeBuiltInFunction("fs::list", bif_listFiles);
    engine->makeBuiltInFunction("fs::isFile", bif_isFile);
    engine->makeBuiltInFunction("fs::isDirectory", bif_isDirectory);
    engine->makeBuiltInFunction("fs::exists", bif_fileExists);
    engine->makeBuiltInFunction("fs::getParent", bif_getParentFile);
    engine->makeBuiltInFunction("fs::getChild", bif_getChildFile);
    engine->makeBuiltInFunction("fs::mkDir", bif_mkDir);
    engine->makeBuiltInFunction("fs::delete", bif_deleteFile);
    engine->makeBuiltInFunction("fs::move", bif_moveFile);

    // listFiles isFile isDirectory exists file(forString), directory(forString)
    // cdUp read readall readLine eof write write line openFile closeFile
    // fileClosed mkDir
}

void FilesExtension::bif_getFile(const CallContext& ctx) {
    QString name = ctx.fetchString(BIF_INFO);

    ctx.setReferenceResult(FileInfoReference::make(QFileInfo(name)));
}

void FilesExtension::bif_getHome(const CallContext& ctx) {
    QFileInfo info(QDir::homePath());

    ctx.setReferenceResult(FileInfoReference::make(info));
}

void FilesExtension::bif_getPath(const CallContext& ctx) {
    FileInfoReference* ref = ctx.fetchRef<FileInfoReference>(BIF_INFO);

    ctx.setStringResult(ref->info.absoluteFilePath());
}

void FilesExtension::bif_listFiles(const CallContext& ctx) {
    FileInfoReference* ref = ctx.fetchRef<FileInfoReference>(BIF_INFO);

    QDir dir(ref->info.absoluteFilePath());
    QFileInfoList list = dir.entryInfoList();
    ListBuilder builder(ctx.storage);
    foreach (QFileInfo f, list){
        builder.append(
                    ctx.storage->makeReference(
                        FileInfoReference::make(QFileInfo(f))));
    }
    ctx.setResult(builder.getResult());
}

void FilesExtension::bif_isFile(const CallContext& ctx) {
    FileInfoReference* ref = ctx.fetchRef<FileInfoReference>(BIF_INFO);

    ctx.setResult(ref->info.isFile() ? SYMBOL_TRUE : SYMBOL_FALSE);
}

void FilesExtension::bif_isDirectory(const CallContext& ctx) {
    FileInfoReference* ref = ctx.fetchRef<FileInfoReference>(BIF_INFO);

    ctx.setResult(ref->info.isDir() ? SYMBOL_TRUE : SYMBOL_FALSE);
}

void FilesExtension::bif_fileExists(const CallContext& ctx) {
    FileInfoReference* ref = ctx.fetchRef<FileInfoReference>(BIF_INFO);

    ctx.setResult(ref->info.exists() ? SYMBOL_TRUE : SYMBOL_FALSE);
}

void FilesExtension::bif_getParentFile(const CallContext& ctx) {
    FileInfoReference* ref = ctx.fetchRef<FileInfoReference>(BIF_INFO);

    if (ref->info.isRoot()) {
        ctx.setResult(NIL);
        return;
    }
    ctx.setReferenceResult(
                FileInfoReference::make(QFileInfo(ref->info.absolutePath())));
}

void FilesExtension::bif_getChildFile(const CallContext& ctx) {
    FileInfoReference* ref = ctx.fetchRef<FileInfoReference>(BIF_INFO);

    QDir dir(ref->info.absoluteFilePath());
    QString file = ctx.fetchString(BIF_INFO);
    ctx.setReferenceResult(FileInfoReference::make(QFileInfo(dir, file)));
}

void FilesExtension::bif_mkDir(const CallContext& ctx) {
    FileInfoReference* ref = ctx.fetchRef<FileInfoReference>(BIF_INFO);

    QDir dir;
    bool success = dir.mkpath(ref->info.absoluteFilePath());
    ctx.setResult(success ? SYMBOL_TRUE : SYMBOL_FALSE);
}

void FilesExtension::bif_deleteFile(const CallContext& ctx) {
    FileInfoReference* ref = ctx.fetchRef<FileInfoReference>(BIF_INFO);

    bool success = QFile::remove(ref->info.absoluteFilePath());
    ctx.setResult(success ? SYMBOL_TRUE : SYMBOL_FALSE);
}

void FilesExtension::bif_moveFile(const CallContext& ctx) {
    Atom input;
    FileInfoReference* src = ctx.fetchRef<FileInfoReference>(BIF_INFO);
    FileInfoReference* target = ctx.fetchRef<FileInfoReference>(BIF_INFO,
                                                                &input);

    QDir dir;
    dir.rename(src->info.absoluteFilePath(),
               target->info.absoluteFilePath());

    ctx.setResult(input);
}
