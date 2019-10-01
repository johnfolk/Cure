//
// = LIBRARY
//
// = FILENAME
//    testKeyReader.cc
//
// = FUNCTION
//
// = AUTHOR(S)
//    Patric Jensfelt
//
// = DESCRIPTION
//    
// = COPYRIGHT
//    Copyright (c) 2004 Patric Jensfelt
//
/*----------------------------------------------------------------------*/

#include "KeyReader.hh"

#ifndef DEPEND
#include <string>
#include <iostream>
#include <unistd.h>
#endif

void printMenu()
{ 
  std::cout << "\033[H\033[J\n";  // Set cursor in top left corner
  std::cout << "Menu\n==============\n";
  std::cout << "1 Quit now\n"
            << "2 Quick immediately\n"
            << std::endl;
}

int main()
{
  Cure::KeyReader kr;

  printMenu();

  long i = 0;
  while (1) {    
    char c;
    if ((c = kr.keyPressed()) != 0) {
      if (c == '1' || c == '2') {
        std::cout << "\nUser selected item " << c << std::endl;
        break;
      } else {
        std::cout << "\nOption \"" << c << "\" not handled\n"; 
        usleep(500000);
        printMenu();
      }
    } else {
      // Here we could do something useful or something not so useful
      // like incrementing a counter and taking a nap
      i++;
      usleep(10000);
    }
  }

  std::cout << "I managed to count to " << i 
            << " before you made up your mind\n";
}
