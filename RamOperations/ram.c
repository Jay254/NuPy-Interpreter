/*ram.c*/

//
// << THIS FILE HANDLES IMPLEMENTATION OF A DYNAMIC MEMORY MODULE FOR STORING VARIABLES IN RAM. IT PROVIDES FUNCTIONS FOR INITIALIZATION, READING, WRITING, AND MEMORY MANAGEMENT >>
//
// << JAY KIPTOO YEGON >>
// << NORTHWESTERN UNIVERSITY >>
// << CS 211 WINTER MAJOR>>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h> // true, false
#include <string.h>
#include <assert.h>
#include <stdint.h> //int<->pointr tyoe conversion
#include "ram.h"

//
// Public functions:
//

//
// ram_init
//
// Returns a pointer to a dynamically-allocated memory
// for storing nuPython variables and their values. All
// memory cells are initialized to the value None.
//
struct RAM *ram_init(void)
{
  // allocate memory for the struct RAM
  struct RAM *memory = (struct RAM *)malloc(sizeof(struct RAM));

  // check if memory allocation was successful
  if (memory == NULL)
  {
    // memory allocation failure? return NULL
    return NULL;
  }
  else
  {
    // initialize the number of values in memory to 0
    memory->num_values = 0;
    // set the initial capacity of the memory to 4
    memory->capacity = 4;
    // allocate memory for the array of RAM cells
    memory->cells = (struct RAM_CELL *)malloc(sizeof(struct RAM_CELL) * memory->capacity);
    // check if memory allocation for cells was successful
    // if (memory->cells == NULL)
    // { // memory allocation failure? free previously allocated memory for RAM
    //   free(memory);
    //   return NULL;
    // }
    // initialize each RAM cell in the array
    for (int i = 0; i < memory->capacity; i++)
    { // set the identifier of the cell to NULL
      memory->cells[i].identifier = NULL;
      // set the value type of the cell to RAM_TYPE_NONE
      memory->cells[i].value.value_type = RAM_TYPE_NONE;
    }
    // return the initialized memory structure
    return memory;
  }

}

//
// ram_destroy
//
// Frees the dynamically-allocated memory associated with
// the given memory. After the call returns, you cannot
// use the memory.
//
void ram_destroy(struct RAM *memory)
{
  // if memory pointer is NULL
  if (memory == NULL)
  { // exit program with an error code (-123 in this case)
    exit(-123);
  }
  else
  { // iterate through each memory cell
    for (int i = 0; i < memory->num_values; i++)
    {
      // free the memory associated with the identifier in the current cell
      free(memory->cells[i].identifier);
      // Check if the value in the cell is of type RAM_TYPE_STR
      if (memory->cells[i].value.value_type == RAM_TYPE_STR)
      {
        // If it is a string value, free the dynamically allocated string
        free(memory->cells[i].value.types.s);
      }
    }
    // Free the memory for the array of cells
    free(memory->cells);
    // Free the memory for the RAM structure
    free(memory);
  }

}

//
// ram_get_addr
//
// If the given identifier (e.g. "x") has been written to
// memory, returns the address of this value --- an integer
// in the range 0..N-1 where N is the number of values currently
// stored in memory. Returns -1 if no such identifier exists
// in memory.
//
// NOTE: a variable has to be written to memory before you can
// get its address. Once a variable is written to memory, its
// address never changes.
//
int ram_get_addr(struct RAM *memory, char *identifier)
{
  // if the memory pointer is NULL
  if (memory == NULL)
  { // return -1 to signal an error (no address found in NULL memory)
    return -1;
  }
  // Check if the identifier pointer is NULL
  // if (identifier == NULL)
  // { // Return -1 to signal an error (no address found for a NULL identifier)
  //   return -1;
  // }
  // Iterate through each cell in the memory structure
  for (int i = 0; i < memory->num_values; i++)
  { // Compare the current cell's identifier with the specified identifier
    if (strcmp(memory->cells[i].identifier, identifier) == 0)
    { // If a match is found, return the address (index) of the cell
      return i;
    }
  }
  // If the identifier is not found in any cell, return -1 to signal an error
  return -1;


}

//
// ram_read_cell_by_addr
//
// Given a memory address (an integer in the range 0..N-1),
// returns a COPY of the value contained in that memory cell.
// Returns NULL if the address is not valid.
//
// NOTE: this function allocates memory for the value that
// is returned. The caller takes ownership of the copy and
// must eventually free this memory via ram_free_value().
//
// NOTE: a variable has to be written to memory before its
// address becomes valid. Once a variable is written to memory,
// its address never changes.
//

struct RAM_VALUE *ram_read_cell_by_addr(struct RAM *memory, int address)
{
  // Check if the address is out of bounds
  if (address < 0 || address >= memory->num_values)
  { // return NULL to indicate that the address is invalid
    return NULL;
  }
  else
  { // allocate memory for a copy of the value at the specified address
    struct RAM_VALUE *copy = (struct RAM_VALUE *)malloc(sizeof(struct RAM_VALUE));
    // copy the value from the specified memory cell to the newly allocated memory
    *copy = memory->cells[address].value;
    // check if the value is a string
    if (memory->cells[address].value.value_type == RAM_TYPE_STR)
    { // duplicate the string and update the copy's string pointer
      copy->types.s = strdup(copy->types.s);
    }
    // return copy of the value
    return copy;
  }

}

//
// ram_read_cell_by_id
//
// If the given identifier (e.g. "x") has been written to
// memory, returns a COPY of the value contained in memory.
// Returns NULL if no such identifier exists in memory.
//
// NOTE: this function allocates memory for the value that
// is returned. The caller takes ownership of the copy and
// must eventually free this memory via ram_free_value().
//
struct RAM_VALUE *ram_read_cell_by_id(struct RAM *memory, char *identifier)
{
  // if the identifier array in the first cell is NULL (no values stored)
  if (memory->cells->identifier == NULL)
  { // return NULL to indicate that no values are stored in memory
    return NULL;
  }
  else
  { // get the memory address (index) associated with the given identifier
    int address = ram_get_addr(memory, identifier);
    // Check if the identifier was not found in memory
    if (address < 0)
    { // return NULL to indicate that the identifier was not found
      return NULL;
    }
    // retrieve a copy of the value from the specified memory address
    return ram_read_cell_by_addr(memory, address);
  }

}

//
// ram_free_value
//
// Frees the memory value returned by ram_read_cell_by_id and
// ram_read_cell_by_addr.
//
void ram_free_value(struct RAM_VALUE *value)
{ // check if input value pointer is NULL
  if (value == NULL)
  { // return from the function if the value pointer is NULL
    return;
  }
  // check if the value type is RAM_TYPE_STR (string)
  if (value->value_type == RAM_TYPE_STR)
  { // free the dynamically allocated memory for the string
    free(value->types.s);
  }
  // free the memory associated with the RAM_VALUE structure
  free(value);
  // return from the function
  return;

}

//
// ram_write_cell_by_addr
//
// Writes the given value to the memory cell at the given
// address. If a value already exists at this address, that
// value is overwritten by this new value. Returns true if
// the value was successfully written, false if not (which
// implies the memory address is invalid).
//
// NOTE: if the value being written is a string, it will
// be duplicated and stored.
//
// NOTE: a variable has to be written to memory before its
// address becomes valid. Once a variable is written to memory,
// its address never changes.
//
bool ram_write_cell_by_addr(struct RAM *memory, struct RAM_VALUE value, int address)
{ 
  // Check if the address is valid
  if (address < 0 || address >= memory->num_values)
  {
    return false; // Invalid address
  }

  // If the existing identifier is not NULL, free it before overwriting
  // if (memory->cells[address].identifier != NULL)
  // {
  //   free(memory->cells[address].identifier);
  //   memory->cells[address].identifier = NULL;
  // }

  // If the existing value is a string, free it before overwriting
  if (memory->cells[address].value.value_type == RAM_TYPE_STR)
  {
    free(memory->cells[address].value.types.s);
    memory->cells[address].value.types.s = NULL;
  }

  // Handle the special case of a string value
  if (value.value_type == RAM_TYPE_STR)
  {
    // Duplicate the string and store it
    char *duplicated_string = strdup(value.types.s);
    if (duplicated_string == NULL)
    {
      // Memory allocation failure
      return false;
    }

    memory->cells[address].value.value_type = RAM_TYPE_STR;
    memory->cells[address].value.types.s = duplicated_string;
  }
  else
  {
    // For non-string values, copy the value directly
    memory->cells[address].value = value;
  }

  // Update the number of values if writing to a new address
  // if (address >= memory->num_values)
  // {
  //   memory->num_values = address + 1;
  // }

  return true;
}

//
// ram_write_cell_by_id
//
// Writes the given value to a memory cell named by the given
// identifier. If a memory cell already exists with this name,
// the existing value is overwritten by the given value. Returns
// true since this operation always succeeds.
//
// NOTE: if the value being written is a string, it will
// be duplicated and stored.
//
// NOTE: a variable has to be written to memory before its
// address becomes valid. Once a variable is written to memory,
// its address never changes.
//
bool ram_write_cell_by_id(struct RAM *memory, struct RAM_VALUE value, char *identifier)
{ 
  // iterate through existing cells to find a cell with the specified identifier
  for (int i = 0; i < memory->num_values; i++)
  { // if a cell with the specified identifier is found, overwrite its value
    if (strcmp(memory->cells[i].identifier, identifier) == 0)
    {
      int address = ram_get_addr(memory, identifier);
      return ram_write_cell_by_addr(memory, value, address);
    }
  }
  // check if the memory is full and needs to be resized
  if (memory->capacity == memory->num_values)
  {
    memory->capacity = memory->capacity * 2;
    memory->cells = (struct RAM_CELL *)realloc(memory->cells, sizeof(struct RAM_CELL) * memory->capacity);
    // initialize the new cells (starting from the current number of values)
    for (int i = memory->num_values; i < memory->capacity; i++)
    {
      memory->cells[i].identifier = NULL;
      memory->cells[i].value.value_type = RAM_TYPE_NONE;
    }
  }
  // save the new identifier in a new cell
  int address = memory->num_values;
  memory->cells[address].identifier = strdup(identifier);
  memory->num_values++;
  // write the value to the newly allocated cell

  return ram_write_cell_by_addr(memory, value, address);

}

//
// ram_print
//
// Prints the contents of memory to the console.
//
void ram_print(struct RAM *memory)
{

  printf("**MEMORY PRINT**\n");
  printf("Capacity: %d\n", memory->capacity);
  printf("Num values: %d\n", memory->num_values);
  printf("Contents:\n");

  for (int i = 0; i < memory->capacity; i++)
  {
    printf(" %d: %s, ", i, memory->cells[i].identifier);

    switch (memory->cells[i].value.value_type)
    {
    case RAM_TYPE_INT:
      printf("int, %d", memory->cells[i].value.types.i);
      break;
    case RAM_TYPE_REAL:
      printf("real, %lf", memory->cells[i].value.types.d);
      break;
    case RAM_TYPE_STR:
      printf("str, '%s'", memory->cells[i].value.types.s);
      break;
    case RAM_TYPE_PTR:
      printf("ptr, %p", (void *)(intptr_t)memory->cells[i].value.types.i);
      break;
    case RAM_TYPE_BOOLEAN:
      printf("boolean, %s", (memory->cells[i].value.types.i == 0) ? "False" : "True");
      break;
    case RAM_TYPE_NONE:
      printf("none, None");
      break;
    }

    printf("\n");
  }

  printf("**END PRINT**\n");
}
