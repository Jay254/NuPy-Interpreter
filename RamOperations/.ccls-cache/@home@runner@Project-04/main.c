/*main.c*/

//
// Main program to run google tests. Do not modify this file.
//
// Prof. Joe Hummel
// Northwestern University
// CS 211, Fall 2023
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gtest/gtest.h"


//
// main program: do not change this, google test will find all your
// tests and run them. So leave the code below as is:
//
int main() 
{
  ::testing::InitGoogleTest();

  //
  // run all the tests, returns 0 if all pass and 1 if any fail:
  //
  int result = RUN_ALL_TESTS();

  return result;  // 0 => all passed
}
