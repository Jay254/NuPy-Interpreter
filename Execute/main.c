/*main.c*/
#include "parser.h"
#include "programgraph.h"
#include "ram.h"
#include "execute.h"
//
// << THIS FILE DEFINES THE ENTRY POINT TO THE NUPYTHON INTERPRETER PROGRAM. IT ORCHESTRATES INPUT HANDLING, SYNTAX CHECKING, AND PROGRAM EXECUTION, BY INTERGRATING WITH OTHER MODULES (PARSER, PROGRAMGRAPH,RAM, EXECUTE) TO ACHIEVE THE OVERALL DESIRED FUNCTIONALITY>>
//
// << JAY KIPTOO YEGON >>
// << NORTHWESTERN UNIVERSITY >>
// << COMPUTER SCIENCE MAJOR >>
//
//

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>  // true, false
#include <string.h>   // strcspn

#include "token.h"    // token defs
#include "scanner.h"  // scanner


//
// main
//
// usage: program.exe [filename.py]
// 
// If a filename is given, the file is opened and serves as
// input to the scanner. If a filename is not given, then 
// input is taken from the keyboard until $ is input.
//
int main(int argc, char* argv[])
{
  FILE* input = NULL;
  bool  keyboardInput = false;

  if (argc < 2) {
    //
    // no args, just the program name:
    //
    input = stdin;
    keyboardInput = true;
  }
  else {
    //
    // assume 2nd arg is a nuPython file:
    //
    char* filename = argv[1];

    input = fopen(filename, "r");

    if (input == NULL) // unable to open:
    {
      printf("**ERROR: unable to open input file '%s' for input.\n", filename);
      return 0;
    }

    keyboardInput = false;
  }

  //
  // input the tokens, either from keyboard or the given nuPython 
  // file; the "input" variable controls the source. the scanner will
  // stop and return EOS when the user enters $ or we reach EOF on
  // the nuPython file:
  //
  // int lineNumber = -1;
  // int colNumber = -1;
  // char value[256] = "";
  // struct Token T;

  //
  // setup to start scanning:
  //
  //scanner_init(&lineNumber, &colNumber, value);

  if (keyboardInput)  // prompt the user if appropriate:
  {
    printf("nuPython input (enter $ when you're done)>\n");
  }

  //
  //call parser to check program syntax:
  parser_init();

  struct TokenQueue* tokens = parser_parse(input);

  if(tokens  == NULL)
  {
    //
    // program has a syntax error, error msg already output:
    //
  }
  else
  {
    //
    // parsing successful, now build program graph:
    //
    struct STMT* program = programgraph_build(tokens);
    printf("**no syntax errors...\n");
    printf("**building program graph...\n");
    programgraph_print(program);
    printf("**executing...\n");
    struct RAM* memory = ram_init();
    execute(program, memory);
    printf("**done\n");
    ram_print(memory);
  }

  //
  // call scanner to process input token by token until we see ; or $
  //
  // T = scanner_nextToken(input, &lineNumber, &colNumber, value);

  // while (T.id != nuPy_EOS)
  // {
  //   printf("Token %d ('%s') @ (%d, %d)\n", T.id, value, T.line, T.col);

  //   T = scanner_nextToken(input, &lineNumber, &colNumber, value);
  // }

  // output that last token:
  // printf("Token %d ('%s') @ (%d, %d)\n", T.id, value, T.line, T.col);

  //
  // done:
  //
  if (!keyboardInput)
    fclose(input);



  return 0;
}
