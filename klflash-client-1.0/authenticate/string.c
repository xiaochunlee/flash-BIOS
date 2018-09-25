#include <stdlib.h>

size_t EfiAsciiStrLen (char   *String)
/*++

Routine Description:
  Return the number of Ascii characters in String. This is not the same as
  the length of the string in bytes.

Arguments:
  String - String to process

Returns:
  Number of Ascii characters in String

--*/
{
  size_t Length;
  
  for (Length=0; *String; String++, Length++);
  return Length;
}
