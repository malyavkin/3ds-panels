//
// Created by lichevsky on 24.09.16.
//
#include "FilePane.h"
using namespace std;
template <class T>
string to_string (const T& t) {
    std::stringstream ss;
    ss << t;
    return ss.str();
}
string getSizeString(u32 size){
    string prefixes[5] = {"","K","M","G","T"};
    u8 idx = 0;
    while(size >= 1024){
        idx++;
        size /= 1024;
    }
    if (idx>4){
        return "!";
    } else {
        return to_string(size)+prefixes[idx];
    }
}
string leftpad(string s, u32 width){
    string s1 = s.substr(0, width);
    while (s1.length() < width){
        s1+=" ";
    }
    return s1;
}
string rightpad(string s, u32 width){
    string s1 = s.substr(0, width);
    while (s1.length() < width){
        s1=" "+s1;
    }
    return s1;
}

FilePane::FilePane(PrintConsole printConsole, FileManager* fm) {
    this->printConsole = printConsole;
    this->active = false;
    this->fm = fm;
}

void FilePane::setActive(bool active){
    this->active = active;
    draw();
}

FileInfo* FilePane::getItem(u32 index) {
    return &this->items[index];

}

FileInfo* FilePane::getSelectedItem() {
    return getItem(this->ctx.selectedItem);
}

string FilePane::getCWD() {
    return this->ctx.cwd;
}

void FilePane::moveUp(){
    if(this->ctx.selectedItem > 0){
        this->ctx.selectedItem--;
        // если курсор совпадает с самым верхним отображающимся элементом
        if (this->ctx.selectedItem == this->ctx.startingIndex){
            // если после перемещения курсор не стал совпадать с самым первым элементом
            if (this->ctx.selectedItem > 0){
                //двигаем окно вверх
                this->ctx.startingIndex = this->ctx.selectedItem-1;
            }
        }
        draw();
    }
}

void FilePane::moveDown(){
    if(this->ctx.selectedItem < this->getMaxIndex()){
        this->ctx.selectedItem++;
        // если курсор совпадает с самым нижним отображающимся элементом
        if (this->ctx.selectedItem == this->getBottomIndex()){
            // если после перемещения курсор не стал совпадать с самым последним элементом
            if (this->ctx.selectedItem < this->getMaxIndex()){
                //двигаем окно вниз
                this->ctx.startingIndex++;
            }
        }
        draw();
    }
}

void FilePane::moveTop(){
    this->ctx.selectedItem = 0;
    this->ctx.startingIndex = 0;
    draw();
}
void FilePane::moveEnd(){
    this->ctx.selectedItem = this->getMaxIndex();
    if (this->getNumberOfItems() < this->getDisplayHeight()){
        this->ctx.startingIndex = 0;
    } else {
        this->ctx.startingIndex = this->ctx.selectedItem - this->getDisplayHeight() + 1;
    }

    draw();
}
void FilePane::updir(){
    if(history.size()){
        DisplayContext restored = history.back();
        history.pop_back();
        setContext(restored);

    }
}
void FilePane::enter(){
    FileInfo* f = getSelectedItem();
    if(f->special && f->name==UPDIR){
        updir();
    } else {
        if(S_ISDIR(f->stats.st_mode)){
            history.push_back(this->ctx);
            setContext(f->path, 0, 0);

        }

    }
}
//
void FilePane::setContext(DisplayContext newContext){
    this->ctx.cwd = newContext.cwd;
    this->items = fm->list_files(newContext.cwd);
    this->ctx.selectedItem = newContext.selectedItem;
    this->ctx.startingIndex = newContext.startingIndex;
    fm->onCWDUpdate(this);
    draw();
}
void FilePane::setContext(string cwd, u32 selectedItem, u32 startingIndex){
    DisplayContext context;
    context.cwd = cwd;
    context.selectedItem = selectedItem;
    context.startingIndex = startingIndex;
    setContext(context);
}
void FilePane::setContext(string cwd){
    setContext(cwd, 0, 0);
}
//
string FilePane::getSupplementaryInfo(FileInfo info){
    if(info.special){
        if(info.name == UPDIR){
            return "UPDIR";
        }
    }
    if(S_ISDIR(info.stats.st_mode)){
        return "     ";
    }
    return getSizeString((u32)info.stats.st_size);
}
//
string FilePane::getTypeIcon(FileInfo info) {

    if(info.special){
        if(info.name == UPDIR){
            return UPWARDS_ARROW;
        }
    }
    if(S_ISDIR(info.stats.st_mode)){
        return "/";
    }
    return " ";
}
//

void FilePane::drawHeader(BorderSet borderSet, u32 width) {
    std::string b = {borderSet.HORIZONTAL};
    cout << borderSet.TOP_LEFT_CORNER << repeat(b , width-2) << borderSet.TOP_RIGHT_CORNER;
}

void FilePane::drawFooter(BorderSet borderSet, u32 width) {
    std::string b = {borderSet.HORIZONTAL};
    cout << borderSet.BOTTOM_LEFT_CORNER << repeat(b , width-2) << borderSet.BOTTOM_RIGHT_CORNER;
}

string FilePane::getBottomInfo(){
    u32 all = getNumberOfItems();
    string s_all = to_string(all);
    u32 len = s_all.length();
    string s_current = to_string(ctx.selectedItem+1);
    s_current = rightpad(s_current, len);
    return "[" + s_current + "/" + s_all + "]";
}

void FilePane::drawStat(FileInfo* fileInfo){
    consoleSelect(fm->getBottomConsole());
    consoleClear();
    printf("\nStat for file %s\n", fileInfo->name.c_str());
    printf("Path: %s\n", fileInfo->path.c_str());
    printf("Mode: %d\n", fileInfo->stats.st_mode);
    printf("Size: %lu\n", (u32)fileInfo->stats.st_size);
    consoleSelect(&this->printConsole);
}

void FilePane::draw(){
    consoleSelect(&this->printConsole);
    cout << position(0,0) << BG_DEFAULT;
    BorderSet borderSet = border_single;
    u32 width = getDisplayWidth();
    drawHeader(borderSet, width);
    cout << position(0,1) << BG_DEFAULT_INVERTED << "Name";
    cout << position(0,FILENAME_WIDTH+3) << "Size";
    // scrollbar prep
    double scrollbar_start = 0;
    double scrollbar_end = 0;
    if(this->getNumberOfItems() > this->getDisplayHeight()){
        scrollbar_start = (double)(this->ctx.startingIndex * this->getDisplayHeight())/(double)this->getNumberOfItems();
        scrollbar_end = (double)(this->getBottomIndex() * this->getDisplayHeight())/(double)this->getNumberOfItems();
    }

    // number of lines we need to skip from the top of under_panels ( top border in this case)
    u32 offset = 1;
    for (u32 i = 0; i < this->getDisplayHeight() ; ++i) {
        colors_t text_color;
        colors_t background_color;
        colors_t delimiter_color;
        u32 drawingItemIndex = i + this->ctx.startingIndex;
        bool is_selected = (drawingItemIndex == this->ctx.selectedItem && this->active);
        // checking if there is a file at all
        cout << position(i + offset,0) << BG_DEFAULT << borderSet.VERTICAL;
        if(drawingItemIndex <= this->getMaxIndex()){
            FileInfo* f = this->getItem(drawingItemIndex);
            if(is_selected){
                drawStat(f);
            }
            bool is_marked = f->marked;
            // determining text color
            if(is_marked){
                text_color = COLOR_YELLOW;
            } else {
                if(is_selected){
                    text_color = COLOR_BLACK;
                } else{
                    text_color = COLOR_WHITE;
                }
            }
            //determining background color
            if(is_selected){
                background_color = COLOR_CYAN;
                delimiter_color = COLOR_BLACK;
            } else {
                background_color = COLOR_BLUE;
                delimiter_color = COLOR_WHITE;
            }
            string text_style = setColor(text_color, background_color);
            string delim_style = setColor(delimiter_color, background_color);
            string type = getTypeIcon(*f);
            string supplementary = getSupplementaryInfo(*f).substr(0,5);
            ////////////////////////////////////////////////////////////////////////////////////////////////////////////
            // actual writing
            // filename
            cout << text_style << type << leftpad(f->name, FILENAME_WIDTH);
            // delimiter
            cout << delim_style << borderSet.VERTICAL << text_style;
            // additional info
            cout << rightpad(supplementary, 5);
        } else {
            cout << " " << leftpad("", FILENAME_WIDTH) << borderSet.VERTICAL<< leftpad("", 5);
        }
        cout << BG_DEFAULT;
        if (i >= scrollbar_start && i < scrollbar_end) {
            cout << PROGRESSBAR_FILLED;
        } else {
            cout << PROGRESSBAR_EMPTY;
        }
    }
    cout << position(getDisplayHeight() + offset,0) << BG_DEFAULT;
    drawFooter(borderSet, width );
    // drawing current selected item an total # of items
    string bottomInfo = getBottomInfo();
    vector<FileInfo*> fis =getMarkedItems(false);
    u32 nMarked = fis.size();
    if (nMarked){
        cout << position(getDisplayHeight() + offset, 2) << BG_DEFAULT_INVERTED << "[*" << nMarked << "]";
    }
    cout << position(getDisplayHeight() + offset, width - bottomInfo.length() - 3) << BG_DEFAULT_INVERTED << bottomInfo;
}

u32 FilePane::getDisplayHeight(){
    int ht = this->printConsole.windowHeight - OCCUPIED;
    if (ht < OCCUPIED){
        return 0;
    }
    return (u32)ht;
}
//
u32 FilePane::getDisplayWidth(){
    int wt = this->printConsole.windowWidth;
    if (wt < 0){
        return 0;
    }
    return (u32)wt;
}
//
u32 FilePane::getBottomIndex(){
    return this->ctx.startingIndex + this->getDisplayHeight() - 1;
}
//
u32 FilePane::getMaxIndex(){
    return getNumberOfItems()-1;
}
//
u32 FilePane::getNumberOfItems(){
    return items.size();
}

void FilePane::redraw() {
    u32 selected = this->ctx.selectedItem;
    u32 starting = this->ctx.startingIndex;
    u32 oldLength = this->getNumberOfItems();
    this->items = fm->list_files(this->ctx.cwd);
    if(this->getNumberOfItems() < oldLength){
        if(selected > getMaxIndex()){
            selected = getMaxIndex();
        }
        if(starting>0){
            starting--;
        }
    }
    this->ctx.startingIndex = starting;
    this->ctx.selectedItem = selected;
    draw();
}

vector<FileInfo*> FilePane::getMarkedItems(bool fallbackToSelectedItem){
    vector<FileInfo*> vec;
    for (u32 i = 0; i < items.size(); ++i) {
        if(items[i].marked){
            vec.push_back(&items[i]);
        }
    }
    // if there are no marked items we can return currently selected item
    if(!vec.size() && fallbackToSelectedItem){
        vec.push_back(this->getSelectedItem());
    }
    return vec;
}

void FilePane::clock(u32 kDown, u32 kHeld) {
    if ((kHeld | kDown) & KEY_DOWN){
        this->moveDown();
    }
    if ((kHeld | kDown) & KEY_UP){
        this->moveUp();
    }
    if ((kHeld | kDown) & KEY_DLEFT){
        this->moveTop();
    }
    if ((kHeld | kDown) & KEY_DRIGHT){
        this->moveEnd();
    }
    if (kDown & KEY_A){
        this->enter();
    }
    if (kDown & KEY_B){
        this->updir();
    }
    if (kDown & KEY_X){
        this->getSelectedItem()->marked = !this->getSelectedItem()->marked;
        draw();
    }

}