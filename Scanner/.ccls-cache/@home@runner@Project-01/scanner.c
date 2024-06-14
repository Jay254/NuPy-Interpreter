/*scanner.c*/

//
// << WHAT IS THE PURPOSE OF THIS FILE??? >>
//
// << WHAT IS YOUR NAME >>
// << WHAT SCHOOL IS THIS >>
// << WHAT COURSE IS THIS >>
// << WHAT QUARTER IS THIS >>
// 
// Starter code: Prof. Joe Hummel
//

#include <stdio.h>
#include <stdbool.h>  // true, false
#include <ctype.h>    // isspace, isdigit, isalpha
#include <string.h>   // strcmp
#include <assert.h>   // assert

#include "scanner.h"


//
// collect_identifier
//
// Given the start of an identifier, collects the rest into value
// while advancing the column number.
//
static void collect_identifier(FILE* input, int c, int* colNumber, char* value)
{
  assert(isalpha(c) || c == '_');  // should be start of an identifier

  int i = 0;

  while (isalnum(c) || c == '_')  // letter, digit, or underscore
  {
    value[i] = (char)c;  // store char
    i++;

    (*colNumber)++;  // advance col # past char

    c = fgetc(input);  // get next char
  }

  // at this point we found the end of the identifer, so put
  // that last char back for processing next:
  ungetc(c, input);

  // turn the value into a string:
  value[i] = '\0';  // build C-style string:

  return;
}


//
// scanner_init
//
// Initializes line number, column number, and value before
// the start of processing the input stream.
//
void scanner_init(int* lineNumber, int* colNumber, char* value)
{
  assert(lineNumber != NULL);
  assert(colNumber != NULL);
  assert(value != NULL);

  *lineNumber = 1;
  *colNumber = 1;
  value[0] = '\0';  // empty string
}


//
// scanner_nextToken
//
// Returns the next token in the given input stream, advancing the line
// number and column number as appropriate. The token's string-based 
// value is returned via the "value" parameter. For example, if the 
// token returned is an integer literal, then the value returned is
// the actual literal in string form, e.g. "456". For an identifer,
// the value is the identifer itself, e.g. "print" or "y". For a 
// string literal such as 'hi class', the value is the contents of the 
// string literal without the quotes.
//
struct Token scanner_nextToken(FILE* input, int* lineNumber, int* colNumber, char* value)
{
  assert(input != NULL);
  assert(lineNumber != NULL);
  assert(colNumber != NULL);
  assert(value != NULL);

  struct Token T;

  //
  // repeatedly input characters one by one until a token is found:
  //
  while (true)
  {
    //
    // Get the next input character:
    //
    int c = fgetc(input);

    //
    // Let's see what we have...
    //

    if (c == EOF || c == '$')  // no more input, return EOS:
    {
      T.id = nuPy_EOS;
      T.line = *lineNumber;
      T.col = *colNumber;

      value[0] = '$';
      value[1] = '\0';

      return T;
    }
    else if (c == '\n')  // end of line, keep going:
    {
      (*lineNumber)++;  // next line, restart column:
      *colNumber = 1;
      continue;
    }
    else if (isspace(c))  // other form of whitespace, skip:
    {
      (*colNumber)++;  // advance col # past char
      continue;
    }
    else if (c == '(')
    {
      T.id = nuPy_LEFT_PAREN;
      T.line = *lineNumber;
      T.col = *colNumber;

      (*colNumber)++;  // advance col # past char

      value[0] = (char)c;
      value[1] = '\0';

      return T;
    }
    else if (c == ')')
    {
      T.id = nuPy_RIGHT_PAREN;
      T.line = *lineNumber;
      T.col = *colNumber;

      (*colNumber)++;  // advance col # past char

      value[0] = (char)c;
      value[1] = '\0';

      return T;
    }
    else if (c == '_' || isalpha(c))
    {
      //
      // start of identifier or keyword, let's assume identifier for now:
      //
      T.id = nuPy_IDENTIFIER;
      T.line = *lineNumber;
      T.col = *colNumber;

      collect_identifier(input, c, colNumber, value);

      //
      // TODO: is the identifier a keyword? If so, return that
      // token id instead.
      //

      return T;
    }
    else if (c == '*')
    {
      //
      // could be * or **, let's assume * for now:
      //
      T.id = nuPy_ASTERISK;
      T.line = *lineNumber;
      T.col = *colNumber;

      (*colNumber)++;  // advance col # past char

      value[0] = '*';
      value[1] = '\0';

      //
      // now let's read the next char and see what we have:
      //
      c = fgetc(input);

      if (c == '*')  // it's **
      {
        T.id = nuPy_POWER;

        (*colNumber)++;  // advance col # past char

        value[1] = '*';
        value[2] = '\0';

        return T;
      }

      //
      // if we get here, then next char did not 
      // form a token, so we need to put the char
      // back to be processed on the next call:
      //
      ungetc(c, input);

      return T;
    }
    //
    //
    // TODO: all the remaining tokens (punctuation, literals), and
    // also need to handle line comments.
    //
    //
    else
    {
      //
      // if we get here, then char denotes an UNKNOWN token:
      //
      T.id = nuPy_UNKNOWN;
      T.line = *lineNumber;
      T.col = *colNumber;

      (*colNumber)++;  // advance past char

      value[0] = (char)c;
      value[1] = '\0';

      return T;
    }

  }//while

  //
  // execution should never get here, return occurs
  // from within loop
  //
}
