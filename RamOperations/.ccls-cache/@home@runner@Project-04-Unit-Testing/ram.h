/*ram.h*/

//
// Project: random access memory (RAM) for nuPython
//
// Prof. Joe Hummel
// Northwestern University
// CS 211
//

#pragma once

#include <stdbool.h>  // true, false


//
// Functions:
//

//
// ram_init
//
// Returns a pointer to a dynamically-allocated memory
// for storing nuPython variables and their values. All
// memory cells are initialized to the value None.
//
struct RAM* ram_init(void);

//
// ram_free
//
// Frees the dynamically-allocated memory associated with
// the given memory. After the call returns, you cannot
// use the memory that was passed to ram_free().
//
void ram_free(struct RAM* memory);

//
// ram_get_addr
// 
// Searches memory for the given identifier (e.g. x), and if
// found, returns the memory "address" --- an integer in the
// range 0..N-1 where N is the number of values currently 
// stored in memory. Returns -1 if no such identifier exists 
// in memory. 
//
int ram_get_addr(struct RAM* memory, char* identifier);

// 
// ram_get_cell_by_id
// ram_get_cell_by_addr
//
// Searches memory for the given identifier (e.g. x), and if
// found, returns a pointer to that memory cell; returns NULL
// if no such identifier exists in memory. Likewise, given a
// memory address (e.g. 2), returns a pointer to that memory
// cell; returns NULL if the memory address is invalid.
//
struct RAM_CELL* ram_get_cell_by_id(struct RAM* memory, char* identifier);
struct RAM_CELL* ram_get_cell_by_addr(struct RAM* memory, int address);

//
// ram_write_int_by_id
// ram_write_int_by_addr
// ram_write_real_by_id
// ram_write_real_by_addr
// ram_write_str_by_id
// ram_write_str_by_addr
// ram_write_ptr_by_id
// ram_write_ptr_by_addr
//
// Writes a given value to the given memory cell, either
// by identifier or by address. Overwrites the previous
// value if necessary. If the identifier does not exist,
// it is added to memory; if the address does not exist,
// this is considered a serious error and a "panic" occurs
// (error message and program exit).
//
void ram_write_int_by_id(struct RAM* memory, char* identifier, int value);
void ram_write_int_by_addr(struct RAM* memory, int address, int value);
void ram_write_real_by_id(struct RAM* memory, char* identifier, double value);
void ram_write_real_by_addr(struct RAM* memory, int address, double value);
void ram_write_str_by_id(struct RAM* memory, char* identifier, char* value);
void ram_write_str_by_addr(struct RAM* memory, int address, char* value);
void ram_write_ptr_by_id(struct RAM* memory, char* identifier, int value);
void ram_write_ptr_by_addr(struct RAM* memory, int address, int value);

//
// ram_print
//
// Prints the contents of RAM, generally for
// debugging purposes.
//
void ram_print(struct RAM* memory);

//
// Definition of random access memory (RAM)
//
enum RAM_CELL_TYPES
{
  RAM_TYPE_INT = 0,
  RAM_TYPE_REAL, 
  RAM_TYPE_STR,
  RAM_TYPE_PTR,
  RAM_TYPE_BOOLEAN,
  RAM_TYPE_NONE
};

struct RAM_CELL
{
  union
  {
    int i;
    double d;
    char* s;
  } types;

  char* identifier;   // variable name
  int ram_cell_type;  // enum RAM_CELL_TYPES
};

struct RAM
{
  struct RAM_CELL* cells;  // array of memory cells
  int num_values;   // N = # of values currently stored in memory
  int memory_size;  // total # of cells available in memory
};
