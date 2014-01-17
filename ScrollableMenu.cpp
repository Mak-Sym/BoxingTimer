#ifndef SCROLLABLEMENU_cpp
#define SCROLLABLEMENU_cpp

#include "ScrollableMenu.h"

ScrollableMenu::ScrollableMenu(unsigned char rowsCount, unsigned char menuSz, void (*drawMenuItemClbk)(int item, int row, char isActive)) {
  rowsCountOnScreen = rowsCount;
  menuSize = menuSz;
  drawMenuItemCallback = drawMenuItemClbk;
  
  currentIdx = 0;
  headIdx = 0;
  tailIdx = (rowsCountOnScreen > menuSize) ? menuSize - 1 : rowsCountOnScreen - 1;
}

/**
 * Increment by module (cyclic increment). F.e. if module == 3:
 * inc(0, 3) == 1
 * inc(1, 3) == 2
 * inc(2, 3) == 0
 */
unsigned char ScrollableMenu::inc(unsigned char value, unsigned char module) {
  return (value + 1) % module;
}

/**
 * Decrement by module (cyclic decrement). F.e. if module == 3:
 * dec(2, 3) == 1
 * dec(1, 3) == 0
 * dec(0, 3) == 2
 */
unsigned char ScrollableMenu::dec(unsigned char value, unsigned char module) {
  return (value - 1 >= 0) ? value - 1 : module - 1;
  
}

int ScrollableMenu::getCurrent() {
  return currentIdx;
}

void ScrollableMenu::keyUp() {
  if(menuSize > rowsCountOnScreen && headIdx > currentIdx - 1){
    headIdx = dec(headIdx, menuSize);
    tailIdx = dec(tailIdx, menuSize);
  }
  currentIdx = dec(currentIdx, menuSize);
}

void ScrollableMenu::keyDown() {
  if(menuSize > rowsCountOnScreen && tailIdx < currentIdx + 1){
    headIdx = inc(headIdx, menuSize);
    tailIdx = inc(tailIdx, menuSize);
  }
  currentIdx = inc(currentIdx, menuSize);
}

void ScrollableMenu::draw() {
  int i = headIdx, row = 0;
  int endIdx = inc(tailIdx, menuSize);
  while(i != endIdx){
    (*drawMenuItemCallback)(i, row++, i == currentIdx);
    i = inc(i, menuSize);
  }
}

#endif
