/*tests.c*/

//
// << DESCRIPTION >>
//
// << YOUR NAME >>
//
// Initial template: Prof. Joe Hummel
// Northwestern University
// CS 211
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ram.h"
#include "gtest/gtest.h"

//
// private helper functions:
//

//
// Test case: writing one integer value
//
TEST(memory_module, write_one_int)
{
  //
  // create a new memory:
  //
  struct RAM *memory = ram_init();

  ASSERT_TRUE(memory != NULL);
  ASSERT_TRUE(memory->cells != NULL);
  ASSERT_TRUE(memory->num_values == 0);
  ASSERT_TRUE(memory->capacity == 4);

  //
  // we want to store the integer 123:
  //
  struct RAM_VALUE i;

  i.value_type = RAM_TYPE_INT;
  i.types.i = 123;

  bool success = ram_write_cell_by_id(memory, i, "x");
  ASSERT_TRUE(success);

  //
  // check the memory, does it contain x = 123?
  //
  ASSERT_TRUE(memory->num_values == 1);
  ASSERT_TRUE(memory->cells[0].value.value_type == RAM_TYPE_INT);
  ASSERT_TRUE(memory->cells[0].value.types.i == 123);
  ASSERT_TRUE(strcmp(memory->cells[0].identifier, "x") == 0);

  //
  // done test, free memory
  //
  ram_destroy(memory);
}

//
// TODO: add many more unit tests
//
// test writing and reading one string literal
TEST(memory_module, one_str_write_read)
{
  // Initialize RAM module
  struct RAM *memory = ram_init();
  ASSERT_TRUE(memory != NULL);

  // Write a string to memory
  ram_write_cell_by_id(memory, (struct RAM_VALUE){RAM_TYPE_STR, {.s = strdup("hello")}}, "x");

  // Assertions for the written value
  ASSERT_TRUE(memory->num_values == 1);
  ASSERT_TRUE(memory->capacity == 4);
  ASSERT_TRUE(memory->cells[0].value.value_type == RAM_TYPE_STR);
  ASSERT_TRUE(strcmp(memory->cells[0].value.types.s, "hello") == 0);
  ASSERT_TRUE(strcmp(memory->cells[0].identifier, "x") == 0);

  // Read the value from memory using ram_read_cell_by_id
  struct RAM_VALUE *readValue = ram_read_cell_by_id(memory, "x");

  // Assertions for the read value
  ASSERT_TRUE(readValue != NULL);
  ASSERT_TRUE(readValue->value_type == RAM_TYPE_STR);
  ASSERT_TRUE(strcmp(readValue->types.s, "hello") == 0);

  // // Get the cell by ID
  // struct RAM_CELL *cell = ram_read_cell_by_id(memory, "x");

  // // Assertions for the retrieved cell
  // ASSERT_TRUE(cell != NULL);
  // ASSERT_TRUE(cell->value.value_type == RAM_TYPE_STR);
  // ASSERT_TRUE(strcmp(cell->value.types.s, "hello") == 0);
  // ASSERT_TRUE(strcmp(cell->identifier, "x") == 0);

  // Read the cell by address
  struct RAM_VALUE *cellByAddr = ram_read_cell_by_addr(memory, 0);

  // Assertions for the retrieved cell by address
  ASSERT_TRUE(cellByAddr != NULL);
  ASSERT_TRUE(readValue == cellByAddr);

  // Free the memory
  free(memory);
  // Free the value returned by ram_read_cell_by_id
  ram_free_value(readValue);
}