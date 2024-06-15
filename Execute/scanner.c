/*scanner.c*/

//
// << THIS FILE CONTAINS C CODE THAT SCANS THROUGH PYTHON CODE FILES OR INPUTS AND IDENTIFIES PUNCTUATIONS, KEYWORDS, IDENTIFIERS, AND STRING, INTEGER, AND REAL LITERALS
//    AND RETURNS A CORRESPONDING TOKEN TYPE >>
// << NAME: JAY YEGON >>
// << SCHOOL: NORTHWESTERN UNIVERSITY >>
// << MAJOR: COMPUTER SCIENCE (McCORMICK) >>

#include <stdio.h>   //input output stream
#include <stdbool.h> // true, false
#include <ctype.h>   // isspace, isdigit, isalpha
#include <string.h>  // strcmp - string compare
#include <assert.h>  // assert in functions

#include "scanner.h"

//
// COLLECT_IDENTIFIER
//
// Function to collect an identifier from the input stream
// Parameters:
//   - FILE *input: Input stream
//   - int c: Current character, should be the start of an identifier
//   - int *colNumber: Pointer to column number, updated during processing
//   - char *value: Buffer to store the collected identifier
static void collect_identifier(FILE *input, int c, int *colNumber, char *value)
{
  assert(isalpha(c) || c == '_'); // Assert that the character is a letter or underscore, indicating the start of an identifier

  int i = 0;

  // Loop to collect the identifier characters (letters, digits, or underscores)
  while (isalnum(c) || c == '_') // letter, digit, or underscore
  {
    value[i] = (char)c; // Store the current character in the buffer
    i++;

    (*colNumber)++; // Advance the column number past the processed character

    c = fgetc(input); // Get the next character from the input stream
  }

  // At this point, we found the end of the identifier, so put the last character back for processing next:
  ungetc(c, input);

  // Turn the collected characters into a string:
  value[i] = '\0'; // Build a C-style string by null-terminating the buffer

  return;
}

// COLLECT_STRING_LITERAL

// Function to collect a string literal from the input stream
// Parameters:
//   - FILE *input: Input stream
//   - int c: Current character, should be a single or double quote
//   - int *colNumber: Pointer to column number, updated during processing
//   - char *value: Buffer to store the collected string literal
//   - int *lineNumber: Pointer to line number, updated during processing
static void collect_string_literal(FILE *input, int c, int *colNumber, char *value, int *lineNumber)
{

  assert(c == '\'' || '"'); // Assert that the character is a single or double quote, indicating the start of a string literal

  int i = 0;

  char initial_quote_character = c; // Store the initial quote character

  c = fgetc(input);                   // Get the next character from the input stream
  int initial_colNumber = *colNumber; // Store the initial column number

  (*colNumber)++; // Increment the column number

  // Loop to collect characters until the closing quote or newline or end of file is encountered
  while (c != initial_quote_character && c != '\n' && c != EOF) // while c is not equal to the first character and is not a newline character or an EOF,
  {
    value[i] = (char)c; // Store the current character in the buffer
    i++;

    (*colNumber)++;

    c = fgetc(input); // Get the next character from the input stream
  }

  // Check if the loop ended due to a newline or end of file
  if (c == '\n' || c == EOF)
  {
    printf("**WARNING: string literal @ (%d, %d) not terminated properly\n", *lineNumber, initial_colNumber);

    ungetc(c, input); // Put the last character back into the input stream
  }

  // The moment c == initial_quote_character, the loop doesn't run, and colNumber is incremented

  value[i] = '\0'; // Null-terminate the collected characters to form a C-style string
  (*colNumber)++;

  return;
}

// INT_OR_REAL_LITERAL
// Function to identify and process integer or real literals
// Parameters:
//   - FILE *input: Input stream
//   - int c: Current character, should be a digit or a dot
//   - int *colNumber: Pointer to column number, updated during processing
//   - char *value: Buffer to store the collected literal
// Returns:
//   - int: Token type for the identified literal (nuPy_INT_LITERAL, nuPy_REAL_LITERAL, nuPy_UNKNOWN)
static int int_or_real_literal(FILE *input, int c, int *colNumber, char *value)
{
  assert(isdigit(c) || c == '.'); // assert that its a digit or a dot

  int i = 0;
  bool isReal = (c == '.');        // Check if the first character is a dot
  bool decimalEncountered = false; // Track if a decimal point has been encountered
  (*colNumber)++;

  // If the first character is a dot, set isReal to true
  if (isReal)
  {
    value[i] = (char)c;
    i++;

    c = fgetc(input); // get the next character after the decimal point
    (*colNumber)++;

    // If the character after the dot is not a digit, return as unknown
    if (!isdigit(c))
    {
      ungetc(c, input);
      (*colNumber)--; // Correcting the column number
      value[i] = '\0';
      return nuPy_UNKNOWN;
    }

    // Continue reading digits after the dot
    while (isdigit(c))
    {
      value[i] = (char)c;
      i++;
      c = fgetc(input);
      (*colNumber)++;
    }

    // If the next character is not a digit, return as real
    if (!isdigit(c))
    {
      ungetc(c, input);
      (*colNumber)--; // Correcting the column number
      value[i] = '\0';
      return nuPy_REAL_LITERAL;
    }
  }

  // Main loop for collecting digits in the integer or real literal
  while (isdigit(c) || (!isReal && c == '.'))
  {
    value[i] = (char)c;
    i++;
    c = fgetc(input);
    (*colNumber)++;

    // If a dot is encountered, indicating a real literal
    if (c == '.')
    {
      if (decimalEncountered) // Check if a decimal point has already been encountered
      {
        // Handle consecutive decimal points
        ungetc(c, input);
        (*colNumber)--; // Correcting the column number
        break;
      }

      decimalEncountered = true;
      isReal = true;
      value[i] = (char)c;
      i++;

      c = fgetc(input); // Get the next character after the decimal point
      (*colNumber)++;

      // If the next character is not a digit, it's a real literal
      if (!isdigit(c))
      {
        ungetc(c, input);
        (*colNumber)--; // Correcting the column number
        value[i] = '\0';
        return nuPy_REAL_LITERAL;
      }
    }

    // If the character is not a digit, break the loop
    if (!isdigit(c))
    {
      ungetc(c, input);
      (*colNumber)--;
      break;
    }
  }

  // If the last character in value is a dot, consider it an unknown literal
  if (value[i - 1] == '.')
  {
    (*colNumber)--;
    value[i - 1] = '\0'; // Remove the trailing dot
    return nuPy_UNKNOWN;
  }

  value[i] = '\0'; // Null-terminate the value

  // Return the appropriate token type
  return isReal ? nuPy_REAL_LITERAL : nuPy_INT_LITERAL;
}

// ID_OR_KEYWORD
//
// returns token id for identifier or keyword
//
// static makes this function local/private to the file
static int id_or_keyword(char *value)
{
  assert(strlen(value) > 0); // asserting that the string being checked should not be empty otherwise no need to run the function in the first place

  char *keywords[] = {"and", "break", "continue", "def", "elif", "else", "False", "for", "if", "in", "is", "None", "not", "or", "pass", "return", "True", "while"};

  // sizeof is the number of bytes for the whole array
  // in this case, n is gonna be 18 X 8 = 144 bytes
  // dividing the size of the entire array by the size of one element, ie, 144/8 = 18, gives you the length of the array
  int N = sizeof(keywords) / sizeof(keywords[0]); // array size
  // printf("N = %d\n", N);

  int index = -1; // not found at the start

  for (int i = 0; i < N; i++)
  {

    // value is a pointer and keyword is an array of pointers
    // they never gonna point to the same piece of memory
    //(value == keyword[i]) - this is not comparing strings like python and other languages do. this is comparing the raw data
    // in C, you use string compare function, which returns zero if they are the same; it takes the two pointers and compare them character to character.
    if (strcmp(value, keywords[i]) == 0)
    { // they the same if the difference between them is zero
      // printf("found a match!\n");
      index = i;
      break;
    }
  }
  // found it?
  if (index > -1)
  { // yes!
    return nuPy_KEYW_AND + index;
  }
  else
  {
    return nuPy_IDENTIFIER;
  }
}

//
// scanner_init
//
// Initializes line number, column number, and value before
// the start of processing the input stream.
//
void scanner_init(int *lineNumber, int *colNumber, char *value)
{
  assert(lineNumber != NULL);
  assert(colNumber != NULL);
  assert(value != NULL);

  *lineNumber = 1;
  *colNumber = 1;
  value[0] = '\0'; // empty string
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
struct Token scanner_nextToken(FILE *input, int *lineNumber, int *colNumber, char *value)
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

    if (c == EOF || c == '$') // no more input, return EOS:
    {
      T.id = nuPy_EOS; // end of stream
      T.line = *lineNumber;
      T.col = *colNumber;

      value[0] = '$';
      value[1] = '\0';

      return T;
    }

    else if (c == '\n') // end of line, keep going:
    {
      (*lineNumber)++; // next line, restart column:
      *colNumber = 1;
      continue;
    }
    else if (isspace(c)) // other form of whitespace, skip:
    {
      (*colNumber)++; // advance col # past char
      continue;
    }
    else if (c == '(')
    {
      T.id = nuPy_LEFT_PAREN;
      T.line = *lineNumber;
      T.col = *colNumber;

      (*colNumber)++; // advance col # past char

      value[0] = (char)c;
      value[1] = '\0';

      return T;
    }
    else if (c == ')')
    {
      T.id = nuPy_RIGHT_PAREN;
      T.line = *lineNumber;
      T.col = *colNumber;

      (*colNumber)++; // advance col # past char

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

      T.id = id_or_keyword(value);

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

      (*colNumber)++; // advance col # past char

      value[0] = '*';
      value[1] = '\0';

      //
      // now let's read the next char and see what we have:
      //
      c = fgetc(input);

      if (c == '*') // it's **
      {
        T.id = nuPy_POWER;

        (*colNumber)++; // advance col # past char

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
    // PUNCTUATIONS
    else if (c == '+')
    {
      T.id = nuPy_PLUS;
      T.line = *lineNumber;
      T.col = *colNumber;

      (*colNumber)++; // advance past character

      value[0] = '+';
      value[1] = '\0';

      return T;
    }

    else if (c == '-')
    {
      T.id = nuPy_MINUS;
      T.line = *lineNumber;
      T.col = *colNumber;

      (*colNumber)++; // advance past character

      value[0] = '-';
      value[1] = '\0';

      return T;
    }

    else if (c == '%')
    {
      T.id = nuPy_PERCENT;
      T.line = *lineNumber;
      T.col = *colNumber;

      (*colNumber)++; // advance past character

      value[0] = '%';
      value[1] = '\0';

      return T;
    }

    else if (c == '/')
    {
      T.id = nuPy_SLASH;
      T.line = *lineNumber;
      T.col = *colNumber;

      (*colNumber)++; // advance past character

      value[0] = '/';
      value[1] = '\0';

      return T;
    }

    else if (c == '=')
    {
      //
      // could be = or ==, let's assume = for now:
      //
      T.id = nuPy_EQUAL;
      T.line = *lineNumber;
      T.col = *colNumber;

      (*colNumber)++; // advance col # past char

      value[0] = '=';
      value[1] = '\0';

      //
      // now let's read the next char and see what we have:
      //
      c = fgetc(input);

      if (c == '=') // it's ==
      {
        T.id = nuPy_EQUALEQUAL; // ==

        (*colNumber)++; // advance col # past char

        value[1] = '=';
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

    else if (c == '!')
    {
      //
      // could be a !=, but let's assume its a ! for now
      //
      T.id = nuPy_UNKNOWN;
      T.line = *lineNumber;
      T.col = *colNumber;

      (*colNumber)++; // advance col # past char

      value[0] = '!';
      value[1] = '\0';

      //
      // now let's read the next char and see what we have:
      //
      c = fgetc(input);

      if (c == '=') // it's !=
      {
        T.id = nuPy_NOTEQUAL; // !=

        (*colNumber)++; // advance col # past char

        value[1] = '=';
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

    else if (c == '<')
    {
      //
      // could be a <=, but let's assume its a < for now
      //
      T.id = nuPy_LT;
      T.line = *lineNumber;
      T.col = *colNumber;

      (*colNumber)++; // advance col # past char

      value[0] = '<';
      value[1] = '\0';

      //
      // now let's read the next char and see what we have:
      //
      c = fgetc(input);

      if (c == '=') // it's <=
      {
        T.id = nuPy_LTE; // <=

        (*colNumber)++; // advance col # past char

        value[1] = '=';
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

    else if (c == '>')
    {
      //
      // could be a >=, but let's assume its a > for now
      //
      T.id = nuPy_GT;
      T.line = *lineNumber;
      T.col = *colNumber;

      (*colNumber)++; // advance col # past char

      value[0] = '>';
      value[1] = '\0';

      //
      // now let's read the next char and see what we have:
      //
      c = fgetc(input);

      if (c == '=') // it's >=
      {
        T.id = nuPy_GTE; // >=

        (*colNumber)++; // advance col # past char

        value[1] = '=';
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
    else if (c == '[')
    {
      T.id = nuPy_LEFT_BRACKET;
      T.line = *lineNumber;
      T.col = *colNumber;

      (*colNumber)++;

      value[0] = '[';
      value[1] = '\0';

      return T;
    }

    else if (c == ']')
    {
      T.id = nuPy_RIGHT_BRACKET;
      T.line = *lineNumber;
      T.col = *colNumber;

      (*colNumber)++;

      value[0] = ']';
      value[1] = '\0';

      return T;
    }

    else if (c == '{')
    {
      T.id = nuPy_LEFT_BRACE;
      T.line = *lineNumber;
      T.col = *colNumber;

      (*colNumber)++;

      value[0] = '{';
      value[1] = '\0';

      return T;
    }

    else if (c == '}')
    {
      T.id = nuPy_RIGHT_BRACE;
      T.line = *lineNumber;
      T.col = *colNumber;

      (*colNumber)++;

      value[0] = '}';
      value[1] = '\0';

      return T;
    }

    else if (c == '&')
    {
      T.id = nuPy_AMPERSAND;
      T.line = *lineNumber;
      T.col = *colNumber;

      (*colNumber)++;

      value[0] = '&';
      value[1] = '\0';

      return T;
    }

    else if (c == ':')
    {
      T.id = nuPy_COLON;
      T.line = *lineNumber;
      T.col = *colNumber;

      (*colNumber)++;

      value[0] = ':';
      value[1] = '\0';

      return T;
    }

    // STRING LITERAL
    else if (c == '\'' || c == '"')
    {
      T.id = nuPy_STR_LITERAL;
      T.line = *lineNumber;
      T.col = *colNumber;

      collect_string_literal(input, c, colNumber, value, lineNumber);

      return T;
    }

    // INTEGER OR REAL LITERAL
    else if (isdigit(c) || c == '.') // If it is a . or a digit, make T.id and T.line and T.col appropriately
    {

      T.line = *lineNumber;
      T.col = *colNumber;
      T.id = int_or_real_literal(input, c, colNumber, value);

      return T;
    }

    // DISCARDING COMMENTS
    else if (c == '#')
    {
      // start of python comment
      while ((c = fgetc(input)) != '\n' && c != EOF)
      {
        // not yet at the end, continue
      }

      if (c == '\n')
      {
        (*lineNumber)++;  // move to next line
        (*colNumber) = 1; // restart column
        continue;         // skip the rest of the loop and move to the next iteration
      }
      else
      {
        ungetc(c, input); // put the last character back
        (*colNumber)++;   // comment continues on the same line, so just advance the column
        continue;         // skip the rest of the loop and move to the next iteration
      }
    }

    else
    {
      //
      // if we get here, then char denotes an UNKNOWN token:
      //
      T.id = nuPy_UNKNOWN;
      T.line = *lineNumber;
      T.col = *colNumber;

      (*colNumber)++; // advance past char

      value[0] = (char)c;
      value[1] = '\0';

      return T;
    }

  } // while

  //
  // execution should never get here, return occurs
  // from within loop
  //
}