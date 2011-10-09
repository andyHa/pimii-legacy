#include "filesextension.h"

FilesExtension* FilesExtension::INSTANCE = new FilesExtension();

QString FilesExtension::name() {
    return QString("FilesExtension");
}

void FilesExtension::registerBuiltInFunctions(Engine* engine) {
    /*
    engine->makeBuiltInFunction("cwd", bif_cwd);
    engine->makeBuiltInFunction("absolutePath", bif_absolutePath);
    engine->makeBuiltInFunction("getFile", bif_getFile);
    engine->makeBuiltInFunction("getHome", bif_getHome);
    engine->makeBuiltInFunction("getPath", bif_getPath);
    engine->makeBuiltInFunction("listFiles", bif_listFiles);
    engine->makeBuiltInFunction("isFile", bif_isFile);
    engine->makeBuiltInFunction("isDirectory", bif_isDirectory);
    engine->makeBuiltInFunction("fileExists", bif_fileExists);
    engine->makeBuiltInFunction("getParentFile", bif_getParentFile);
    engine->makeBuiltInFunction("getChildFile", bif_getChildFile);
    engine->makeBuiltInFunction("mkDir", bif_mkDir);
    engine->makeBuiltInFunction("deleteFile", bif_deleteFile);
    engine->makeBuiltInFunction("moveFile", bif_moveFile);
    */
    // listFiles isFile isDirectory exists file(forString), directory(forString)
    // cdUp read readall readLine eof write write line openFile closeFile
    // fileClosed mkDir
}
/*

Atom FilesExtension::bif_getFile(Engine* engine, Storage* storage, Atom args) {

}

Atom FilesExtension::bif_getHome(Engine* engine, Storage* storage, Atom args) {

}

Atom FilesExtension::bif_getPath(Engine* engine, Storage* storage, Atom args) {

}

Atom FilesExtension::bif_listFiles(Engine* engine,
                                   Storage* storage,
                                   Atom args) {

}

Atom FilesExtension::bif_isFile(Engine* engine,
                                Storage* storage,
                                Atom args) {

}

Atom FilesExtension::bif_isDirectory(Engine* engine,
                                     Storage* storage,
                                     Atom args) {

}

Atom FilesExtension::bif_fileExists(Engine* engine,
                                    Storage* storage,
                                    Atom args) {

}

Atom FilesExtension::bif_getParentFile(Engine* engine, Storage* storage, Atom args);

Atom FilesExtension::bif_getChildFile(Engine* engine, Storage* storage, Atom args);

Atom FilesExtension::bif_mkDir(Engine* engine, Storage* storage, Atom args);

Atom FilesExtension::bif_deleteFile(Engine* engine, Storage* storage, Atom args);

Atom FilesExtension::bif_moveFile(Engine* engine, Storage* storage, Atom args);

Atom FilesExtension::bif_cwd(Engine* engine, Storage* storage, Atom args) {
    return storage->makeReference(new DirReference(new QDir()));
}

Atom FilesExtension::bif_absolutePath(Engine *engine,
                                      Storage *storage,
                                      Atom args)
{
    engine->expect(isCons(args),
                   "bif_absolutePath called without parameters!",
                   __FILE__,
                   __LINE__);
    Atom first = storage->getCons(args)->car;
    engine->expect(isReference(first),
                   "bif_absolutePath requires a reference as first parameter!",
                   __FILE__,
                   __LINE__);
    Reference* ref = storage->getReference(first);
    FileReference* file = dynamic_cast<FileReference*>(ref);
    if (file != NULL) {
        return storage->makeString("TODO");
    }
    DirReference* dir = dynamic_cast<DirReference*>(ref);
    if (dir != NULL) {
        return storage->makeString(dir->dir->absolutePath());
    }
    engine->expect(
        false,
        "bif_absolutePath requires a file or directory as first parameter!",
         __FILE__,
         __LINE__);
    return NIL;
}
*/
