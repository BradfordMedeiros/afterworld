#ifndef MOD_AFTERWORLD_COMPONENTS_FILEEXPLORER
#define MOD_AFTERWORLD_COMPONENTS_FILEEXPLORER

#include "./common.h"
#include "./listitem.h"
#include "./list.h"
#include "./textbox.h"

enum FileContentType { File, Directory, NoContent };
struct FileContent {
  FileContentType type;
  std::string content;
};

typedef std::function<void(FileContentType type, std::string file)> FileCallback;
struct FileExplorer {
  int contentOffset;
  std::vector<std::string> currentPath;
  std::vector<FileContent> currentContents;
  FileCallback explorerOnChange;
};

extern Component fileexplorerComponent;
extern const int fileExplorerSymbol;
extern const int fileChangeSymbol;

extern FileExplorer testExplorer;

#endif

