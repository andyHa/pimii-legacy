#ifndef FILESEXTENSION_H
#define FILESEXTENSION_H

#include "engineextension.h"
#include "vm/reference.h"

#include <QFile>
#include <QFileInfo>

class FileReference : public Reference {
    const QFile* file;
public:
    FileReference(const QFile* f) : file(f) {}

    virtual ~FileReference() {
        delete file;
    }

    virtual QString toString() {
        return file->fileName();
    }
    friend class FilesExtension;
};

class FileInfoReference : public Reference {
    const QFileInfo* info;
public:
    FileInfoReference(const QFileInfo* info) : info(info) {}

    virtual ~FileInfoReference() {
        delete info;
    }

    virtual QString toString() {
        return info->fileName();
    }
    friend class FilesExtension;
};

class FilesExtension : public EngineExtension
{
    /**
      Returns the file-info for the given string. If no parameter is given
      the current working directory is used:

        getFile := (path : String) -> FileInfo

      */
    static void bif_getFile(const CallContext& ctx);

    /**
      Returns a file-info for the current users' home on the stack:

        getHome := () -> FileInfo

      */
    static void bif_getHome(const CallContext& ctx);

    /**
      Pushes the absolute path of the given file-info stack:

        getPath := (info : FileInfo) -> String

      */
    static void bif_getPath(const CallContext& ctx);

    /**
      Pushes a list of a child files (files or directories) for the given
      file on the stack:

        listFiles := (info : FileInfo) -> List[FileInfo]

      */
    static void bif_listFiles(const CallContext& ctx);

    /**
      Pushes TRUE on the stack, if the given file-info points to a file,
      FALSE otherwise:

        isFile := (info : FileInfo) -> (TRUE|FALSE)

      */
    static void bif_isFile(const CallContext& ctx);

    /**
      Pushes TRUE on the stack, if the given file-info points to a directory,
      FALSE otherwise:

        isDirectory := (info : FileInfo) -> (TRUE|FALSE)

      */
    static void bif_isDirectory(const CallContext& ctx);

    /**
      Pushes TRUE on the stack, if the given file-info points to an existing
      file or directory, FALSE otherwise:

        fileExists := (info : FileInfo) -> (TRUE|FALSE)

      */
    static void bif_fileExists(const CallContext& ctx);

    /**
      Pushes the parent file for the given file on the stack. Pushes NIL,
      if the file was a root:

        getParentFile := (info : FileInfo) -> (FileInfo|NIL)

      */
    static void bif_getParentFile(const CallContext& ctx);

    /**
      Pushes a child file for the given file with the given name on the stack:

        getChildFile := (info : FileInfo, name : String) -> FileInfo

      */
    static void bif_getChildFile(const CallContext& ctx);

    /**
      Ensures, that the given directory exists (creates it if recessary):

        mkDir := (info : FileInfo) -> FileInfo
      */
    static void bif_mkDir(const CallContext& ctx);

    /**
      Deletes the given file or directory:

        deleteFile := (info : FileInfo) -> FileInfo

      */
    static void bif_deleteFile(const CallContext& ctx);

    /**
      Moves the info to the given destination:

        moveFile := (info : FileInfo, target : FileInfo) -> FileInfo

      */
    static void bif_moveFile(const CallContext& ctx);


    // readall readLine eof write write line openFile closeFile
    // fileClosed
public:

    /**
      Contains the static instance of the extension. This is directly loaded
      by the Engine.
      */
    static FilesExtension* INSTANCE;

    /**
      see: EngineExtension.name()
      */
    virtual QString name();

    /**
      see: EngineExtension.registerBuiltInFunctions()
      */
    virtual void registerBuiltInFunctions(Engine* engine);
};

#endif // FILESEXTENSION_H
