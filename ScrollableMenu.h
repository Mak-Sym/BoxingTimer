#ifndef SCROLLABLEMENU_h
#define SCROLLABLEMENU_h

class ScrollableMenu {
  private:
    unsigned char rowsCountOnScreen;
    unsigned char menuSize;
    
    unsigned char currentIdx;
    unsigned char headIdx;
    unsigned char tailIdx;
    
    void (*drawMenuItemCallback)(int item, int row, char isActive);
    unsigned char inc(unsigned char value, unsigned char module);
    unsigned char dec(unsigned char value, unsigned char module);
    
  public:
    ScrollableMenu(unsigned char rowsCount, unsigned char menuSz, void (*drawMenuItemClbk)(int item, int row, char isActive));
    int getCurrent();
    void keyUp();
    void keyDown();
    void draw();
};

#endif
