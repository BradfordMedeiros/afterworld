#ifndef MOD_AFTERWORLD_COMPONENTS_FILEEXPLORER
#define MOD_AFTERWORLD_COMPONENTS_FILEEXPLORER

#include "./common.h"
#include "./basic/listitem.h"
#include "./basic/list.h"
#include "./basic/textbox.h"

struct FileContent {
  bool isDirectory;
  std::string content;
};

typedef std::function<void(bool, std::string)> FileCallback;
typedef std::function<bool(bool, std::string&)> FileFilter;

struct FileExplorer {
  std::vector<std::string> currentPath;
  std::vector<FileContent> currentContents;
  FileCallback explorerOnChange;
};

extern Component fileexplorerComponent;
extern const int fileExplorerSymbol;
extern const int fileChangeSymbol;
extern const int fileFilterSymbol;

extern FileExplorer testExplorer;

#endif

