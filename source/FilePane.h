//
// Created by lichevsky on 20.09.16.
//

#ifndef MAIN_EXEC_FILEPANE_H
#define MAIN_EXEC_FILEPANE_H
class FileManager;

#include "3ds.h"
#include "types.h"
#include "FileManager.h"
#include "drawing.h"
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <cstdio>
#define  UPDIR ".."

const u8 FILENAME_WIDTH = 16;


class FilePane {
public:
    FilePane(PrintConsole printConsole, FileManager* fm);

    void setActive(bool active);

    FileInfo* getItem(u32 index);

    FileInfo* getSelectedItem();

    std::vector<FileInfo*> getMarkedItems(bool fallbackToSelectedItem);

    std::string getCWD();

    void moveUp();

    void moveDown();

    void moveTop();

    void moveEnd();

    void updir();

    void enter();

    void setContext(std::string cwd);

    void redraw();

    void clock(u32 kDown, u32 kHeld);

private:
    void setContext(DisplayContext newContext);

    void setContext(std::string cwd, u32 selectedItem, u32 startingIndex);

    std::string getSupplementaryInfo(FileInfo info);
    
    std::string getTypeIcon(FileInfo info);

    void drawHeader(BorderSet borderSet, u32 width);

    void drawFooter(BorderSet borderSet, u32 width);

    void drawStat(FileInfo* fileInfo);

    void draw();

    u32 getDisplayHeight();

    u32 getDisplayWidth();

    u32 getBottomIndex();

    u32 getMaxIndex();

    u32 getNumberOfItems();
    /**
     * Build a string that has total # of items in current directory and current selected item
     * @return said string
     */
    std::string getBottomInfo();

    PrintConsole printConsole;
    std::vector<FileInfo> items;
    std::vector<DisplayContext> history;
    DisplayContext ctx;
    FileManager* fm;
    bool active;
    // # of lines that occupied by things that are not list of files
    const int OCCUPIED = 2;
};


#endif //MAIN_EXEC_FILEPANE_H
