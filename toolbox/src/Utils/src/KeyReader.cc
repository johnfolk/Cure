//
// = FILENAME
//    
// = FUNCTION
//
// = AUTHOR(S)
//    Patric Jensfelt
//
// = COPYRIGHT
//    Copyright (c) 1999 Patric Jensfelt
//
/*----------------------------------------------------------------------*/

#include "KeyReader.hh"

#ifndef DEPEND
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#endif

namespace Cure {

KeyReader::KeyReader()
{
  //fprintf(stderr, "Storing old termios\n");
  if(tcgetattr(0,&m_NewTermios) == -1) { // appropriate device 
    perror("KeyReader failed to get termios attr");
    fprintf(stderr, "bad ioct1\n");
    exit(0);
  }
  
  m_OldTermios = m_NewTermios;          // save 
  m_NewTermios.c_cc[VMIN]=0;
  m_NewTermios.c_cc[VTIME]=0;
  m_NewTermios.c_lflag&=~ICANON;
}

KeyReader::~KeyReader()
{
  tcsetattr(0, TCSANOW, &m_OldTermios);
}

char
KeyReader::keyPressed(void)
{
  char ch=0;

  //fprintf(stderr, "Using new termios\n");
  tcsetattr(0, TCSANOW, &m_NewTermios); // TCSANOW = doit now 
  read(0,&ch,sizeof(char));
  //fprintf(stderr, "Going back to old termios\n");
  tcsetattr(0, TCSANOW, &m_OldTermios);
  return(ch);
}

}
