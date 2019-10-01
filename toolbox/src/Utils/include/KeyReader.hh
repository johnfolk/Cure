//
// = FILENAME
//    KeyReader.h
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

#ifndef KeyReader_hh
#define KeyReader_hh

#ifndef DEPEND
#include <termios.h>
#endif

namespace Cure {

/**
 * An instances of this class is used to detect when a key on the
 * keyboard is being pressed without having to wait for ENTER being
 * pressed. Can typically be used when writing menu stuff.
 *
 * @author Patric Jensfelt
 */
class KeyReader
{
public:
  /** Constructor */
  KeyReader();

  ~KeyReader();

  /** Returns the character that was pressed, if no character was
      pressed 0 is returned (NOTE not '0') */
  char keyPressed(void);

 protected:

  /** The termios that allows us to read key strokes without waiting
      for ENTER to be pressed */
  struct termios m_NewTermios;

  /** The original termios we should restore to */
  struct termios m_OldTermios;
};

} // namespace Cure 

#endif // ISR_KeyReader_h //
