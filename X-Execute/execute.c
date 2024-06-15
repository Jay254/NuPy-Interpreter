/*execute.c*/

//
// Executes nuPython program, given as a Program Graph. At this
// point we are supporting multiple data types (int, string, real,
// boolean), and assignment statements with binary expressions.
//

// to eliminate warnings about stdlib in Visual Studio
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h> // true, false
#include <string.h>
#include <assert.h>
#include <math.h>
#include <limits.h>

#include "programgraph.h"
#include "ram.h"
#include "execute.h"
#include "util.h"

//
// Private functions:
//

//
// starts_with_zero
//
// Returns true if the given string starts with a
// '0', false if not. Starting with a 0 is all you
// need for atoi() and atof() to work successfully.
//
static bool starts_with_zero(char *s)
{
  if (strlen(s) > 0 && s[0] == '0')
    return true;

  return false;
}

//
// dupAndConcat
//
// Given 2 strings, makes a copy by concatenating
// them together, and returns the copy.
//
// NOTE: this function allocates memory for the copy,
// the caller takes ownership of the copy and must
// eventually free that memory.
//
static char *dupAndConcat(char *s1, char *s2)
{
  assert(s1 != NULL);
  assert(s2 != NULL);

  //
  // be sure to include extra location for null terminator:
  //
  char *copy = (char *)malloc(sizeof(char) * (strlen(s1) + strlen(s2) + 1));
  if (copy == NULL)
  {
    printf("**EXECUTION ERROR: out of memory (dupAndConcat)");
    return NULL;
  }

  strcpy(copy, s1);
  strcat(copy, s2);

  return copy;
}

//
// get_element_value
//
// Given a basic element of an expression --- an identifier
// "x" or some kind of literal like 123 --- the value of
// this identifier or literal is returned as a pointer to
// a RAM_VALUE that contains the value and it's type; NULL
// is returned if this failed.
//
// Why would it fail? If the identifier does not exist in
// memory. This is a semantic error, and an error message is
// output before returning.
//
// NOTE: this function allocates memory for the value that
// is returned. The caller takes ownership of the copy and
// must eventually free this memory via ram_free_value().
//
static struct RAM_VALUE *get_element_value(
    struct STMT *stmt,
    struct RAM *memory,
    struct ELEMENT *element)
{
  struct RAM_VALUE *value = NULL;

  if (element->element_type == ELEMENT_IDENTIFIER)
  {
    //
    // identifier => variable
    //
    assert(element->element_type == ELEMENT_IDENTIFIER);

    char *var_name = element->element_value;

    value = ram_read_cell_by_id(memory, var_name);

    if (value == NULL)
    {
      printf("**SEMANTIC ERROR: name '%s' is not defined (line %d)\n", var_name, stmt->line);
      return NULL;
    }
  }
  else
  {
    //
    // one of the literal types, we need to allocate memory:
    //
    char *literal = element->element_value;

    value = malloc(sizeof(struct RAM_VALUE));
    if (value == NULL)
    {
      printf("**EXECUTION ERROR: out of memory\n");
      return NULL;
    }

    switch (element->element_type)
    {
    case ELEMENT_INT_LITERAL:
      value->value_type = RAM_TYPE_INT;
      value->types.i = atoi(literal);
      break;

    case ELEMENT_REAL_LITERAL:
      value->value_type = RAM_TYPE_REAL;
      value->types.d = atof(literal);
      break;

    case ELEMENT_STR_LITERAL:
      value->value_type = RAM_TYPE_STR;
      value->types.s = dupString(literal);
      break;

    case ELEMENT_TRUE:
      value->value_type = RAM_TYPE_BOOLEAN;
      value->types.i = 1;
      break;

    case ELEMENT_FALSE:
      value->value_type = RAM_TYPE_BOOLEAN;
      value->types.i = 0;
      break;

    default:
      printf("**EXECUTION ERROR: unexpected element type in get_element_value");
      return NULL;
    }
  } // else

  return value;
}

//
// get_unary_value
//
// Given a unary expr, returns the value that it represents.
// This could be the result of a literal 123 or the value
// from memory for an identifier such as "x". Unary values
// may have unary operators, such as + or -, applied.
// This is returned as a pointer to a RAM_VALUE that contains
// the value and it's type; NULL is returned if this failed.
//
// Why would it fail? If the identifier does not exist in
// memory. This is a semantic error, and an error message is
// output before returning.
//
// NOTE: this function allocates memory for the value that
// is returned. The caller takes ownership of the copy and
// must eventually free this memory via ram_free_value().
//
static struct RAM_VALUE *get_unary_value(
    struct STMT *stmt,
    struct RAM *memory,
    struct UNARY_EXPR *unary)
{
  struct RAM_VALUE *value = NULL;

  switch (unary->expr_type)
  {
  case UNARY_ELEMENT:
  {
    struct ELEMENT *element = unary->element;
    value = get_element_value(stmt, memory, element);
    break;
  }

  case UNARY_ADDRESS_OF:
  {
    // Declarations should be at the beginning of the block
    char *identifier = unary->element->element_value;
    int address = ram_get_addr(memory, identifier);

    // Move the declaration outside the if block
    if (address != -1)
    {
      value = malloc(sizeof(struct RAM_VALUE));
      if (value == NULL)
      {
        printf("**EXECUTION ERROR: Out of memory\n");
        return NULL;
      }

      value->value_type = RAM_TYPE_PTR;
      value->types.i = address;
    }
    else
    {
      printf("**SEMANTIC ERROR: name '%s' is not defined (line %d)\n", identifier, stmt->line);
      return NULL;
    }

    break;
  }
  case UNARY_PTR_DEREF:
  {
    // Get the identifier from element
    char *identifier = unary->element->element_value;
    // Read value from RAM using the identifier
    value = ram_read_cell_by_id(memory, identifier);

    if (value == NULL)
    {
      // Print error message for an undefined identifier
      printf("**SEMANTIC ERROR: name '%s' is not defined (line %d)\n", identifier, stmt->line);
      // Return a special value to indicate an error
      return NULL;
    }
    // else if (value->value_type != RAM_TYPE_PTR)
    // {
    //   // Print error message for an undefined identifier
    //   printf("**SEMANTIC ERROR: invalid operand types (line %d)\n", stmt->line);
    //   // Return a special value to indicate an error
    //   return NULL;
    // }
    else if (value->value_type != RAM_TYPE_PTR || value->types.i < 0 || value->types.i >= memory->num_values)
    {
      printf("**SEMANTIC ERROR: '%s' contains invalid address (line %d)\n", identifier, stmt->line);
      return NULL;
    }
    else
    {
      return value;
    }
  }

  default:
    printf("Error: Unsupported unary expression type\n");
    break;
  }

  return value;
}

//
// perform_int_operation
// perform_real_operation
// perform_str_comparison
//
// Given "lhs operator rhs", performs the operation
// and stores the result in the destination value.
//
static void perform_int_operation(
    struct RAM_VALUE *dest,
    int lhs,
    int operator,
    int rhs)
{
  //
  // make sure we are not overwriting string values
  // which has memory consequences:
  //
  assert(dest->value_type != RAM_TYPE_STR);

  assert(operator== OPERATOR_PLUS ||
         operator== OPERATOR_MINUS ||
         operator== OPERATOR_ASTERISK ||
         operator== OPERATOR_POWER ||
         operator== OPERATOR_MOD ||
         operator== OPERATOR_DIV ||
         operator== OPERATOR_EQUAL ||
         operator== OPERATOR_NOT_EQUAL ||
         operator== OPERATOR_LT ||
         operator== OPERATOR_LTE ||
         operator== OPERATOR_GT ||
         operator== OPERATOR_GTE);

  //
  // make sure the target cell is the correct type:
  //
  dest->value_type = RAM_TYPE_INT;

  switch (operator)
  {
  case OPERATOR_PLUS:
    dest->types.i = lhs + rhs;
    break;

  case OPERATOR_MINUS:
    dest->types.i = lhs - rhs;
    break;

  case OPERATOR_ASTERISK:
    dest->types.i = lhs * rhs;
    break;

  case OPERATOR_POWER:
    dest->types.i = (int)pow(lhs, rhs);
    break;

  case OPERATOR_MOD:
    dest->types.i = lhs % rhs;
    break;

  case OPERATOR_DIV:
    dest->types.i = lhs / rhs;
    break;

  case OPERATOR_EQUAL:
    dest->value_type = RAM_TYPE_BOOLEAN;

    if (lhs == rhs)
      dest->types.i = 1; // true
    else
      dest->types.i = 0; // false
    break;

  case OPERATOR_NOT_EQUAL:
    dest->value_type = RAM_TYPE_BOOLEAN;

    if (lhs != rhs)
      dest->types.i = 1; // true
    else
      dest->types.i = 0; // false
    break;

  case OPERATOR_LT:
    dest->value_type = RAM_TYPE_BOOLEAN;

    if (lhs < rhs)
      dest->types.i = 1; // true
    else
      dest->types.i = 0; // false
    break;

  case OPERATOR_LTE:
    dest->value_type = RAM_TYPE_BOOLEAN;

    if (lhs <= rhs)
      dest->types.i = 1; // true
    else
      dest->types.i = 0; // false
    break;

  case OPERATOR_GT:
    dest->value_type = RAM_TYPE_BOOLEAN;

    if (lhs > rhs)
      dest->types.i = 1; // true
    else
      dest->types.i = 0; // false
    break;

  case OPERATOR_GTE:
    dest->value_type = RAM_TYPE_BOOLEAN;

    if (lhs >= rhs)
      dest->types.i = 1; // true
    else
      dest->types.i = 0; // false
    break;
  }
}

static void perform_real_operation(
    struct RAM_VALUE *dest,
    double lhs,
    int operator,
    double rhs)
{
  //
  // make sure we are not overwriting string values,
  // which has memory consequences:
  //
  assert(dest->value_type != RAM_TYPE_STR);

  assert(operator== OPERATOR_PLUS ||
         operator== OPERATOR_MINUS ||
         operator== OPERATOR_ASTERISK ||
         operator== OPERATOR_POWER ||
         operator== OPERATOR_MOD ||
         operator== OPERATOR_DIV ||
         operator== OPERATOR_EQUAL ||
         operator== OPERATOR_NOT_EQUAL ||
         operator== OPERATOR_LT ||
         operator== OPERATOR_LTE ||
         operator== OPERATOR_GT ||
         operator== OPERATOR_GTE);

  //
  // make sure the target cell is the correct type:
  //
  dest->value_type = RAM_TYPE_REAL;

  switch (operator)
  {
  case OPERATOR_PLUS:
    dest->types.d = lhs + rhs;
    break;

  case OPERATOR_MINUS:
    dest->types.d = lhs - rhs;
    break;

  case OPERATOR_ASTERISK:
    dest->types.d = lhs * rhs;
    break;

  case OPERATOR_POWER:
    dest->types.d = pow(lhs, rhs);
    break;

  case OPERATOR_MOD:
    dest->types.d = fmod(lhs, rhs);
    break;

  case OPERATOR_DIV:
    dest->types.d = lhs / rhs;
    break;

  case OPERATOR_EQUAL:
    dest->value_type = RAM_TYPE_BOOLEAN;

    if (lhs == rhs)
      dest->types.i = 1; // true
    else
      dest->types.i = 0; // false
    break;

  case OPERATOR_NOT_EQUAL:
    dest->value_type = RAM_TYPE_BOOLEAN;

    if (lhs != rhs)
      dest->types.i = 1; // true
    else
      dest->types.i = 0; // false
    break;

  case OPERATOR_LT:
    dest->value_type = RAM_TYPE_BOOLEAN;

    if (lhs < rhs)
      dest->types.i = 1; // true
    else
      dest->types.i = 0; // false
    break;

  case OPERATOR_LTE:
    dest->value_type = RAM_TYPE_BOOLEAN;

    if (lhs <= rhs)
      dest->types.i = 1; // true
    else
      dest->types.i = 0; // false
    break;

  case OPERATOR_GT:
    dest->value_type = RAM_TYPE_BOOLEAN;

    if (lhs > rhs)
      dest->types.i = 1; // true
    else
      dest->types.i = 0; // false
    break;

  case OPERATOR_GTE:
    dest->value_type = RAM_TYPE_BOOLEAN;

    if (lhs >= rhs)
      dest->types.i = 1; // true
    else
      dest->types.i = 0; // false
    break;
  }
}

static void perform_str_operation(
    struct RAM_VALUE *dest,
    char *lhs,
    int operator,
    char * rhs)
{
  //
  // let's confirm that the lhs and rhs are strings, and
  // the dest is also a string that will be overwritten:
  //
  assert(dest->value_type == RAM_TYPE_STR);
  assert(dest->types.s == lhs || dest->types.s == rhs);

  assert(operator== OPERATOR_PLUS ||
         operator== OPERATOR_EQUAL ||
         operator== OPERATOR_NOT_EQUAL ||
         operator== OPERATOR_LT ||
         operator== OPERATOR_LTE ||
         operator== OPERATOR_GT ||
         operator== OPERATOR_GTE);

  switch (operator)
  {
  case OPERATOR_PLUS:
    //
    // string concatenation:
    //
    dest->value_type = RAM_TYPE_STR;
    dest->types.s = dupAndConcat(lhs, rhs);
    break;

  case OPERATOR_EQUAL:
    dest->value_type = RAM_TYPE_BOOLEAN;

    if (strcmp(lhs, rhs) == 0)
      dest->types.i = 1; // true
    else
      dest->types.i = 0; // false
    break;

  case OPERATOR_NOT_EQUAL:
    dest->value_type = RAM_TYPE_BOOLEAN;

    if (strcmp(lhs, rhs) != 0)
      dest->types.i = 1; // true
    else
      dest->types.i = 0; // false
    break;

  case OPERATOR_LT:
    dest->value_type = RAM_TYPE_BOOLEAN;

    if (strcmp(lhs, rhs) < 0)
      dest->types.i = 1; // true
    else
      dest->types.i = 0; // false
    break;

  case OPERATOR_LTE:
    dest->value_type = RAM_TYPE_BOOLEAN;

    if (strcmp(lhs, rhs) <= 0)
      dest->types.i = 1; // true
    else
      dest->types.i = 0; // false
    break;

  case OPERATOR_GT:
    dest->value_type = RAM_TYPE_BOOLEAN;

    if (strcmp(lhs, rhs) > 0)
      dest->types.i = 1; // true
    else
      dest->types.i = 0; // false
    break;

  case OPERATOR_GTE:
    dest->value_type = RAM_TYPE_BOOLEAN;

    if (strcmp(lhs, rhs) >= 0)
      dest->types.i = 1; // true
    else
      dest->types.i = 0; // false
    break;
  }
}

static void perform_ptr_operation(
    struct RAM_VALUE *dest,
    int lhs,
    int operator,
    int rhs)
{
  // assert(operator== OPERATOR_PLUS || operator== OPERATOR_MINUS);

  dest->value_type = RAM_TYPE_PTR;

  switch (operator)
  {
  case OPERATOR_PLUS:
    dest->types.i = lhs + rhs;
    break;

  case OPERATOR_MINUS:
    dest->types.i = lhs - rhs;
    break;

  case OPERATOR_ASTERISK:

    break;

  case OPERATOR_DIV:

    break;
  case OPERATOR_GT:
    if (lhs > rhs)
    {
      dest->types.i = 1;
    }
    dest->types.i = 0;
    break;

  case OPERATOR_GTE:
    if (lhs >= rhs)
    {
      dest->types.i = 1;
    }
    dest->types.i = 0;
    break;

  case OPERATOR_LT:
    if (lhs < rhs)
    {
      dest->types.i = 1;
    }
    dest->types.i = 0;
    break;

  case OPERATOR_LTE:
    if (lhs <= rhs)
    {
      dest->types.i = 1;
    }
    dest->types.i = 0;
    break;
  }
}

static void perform_ptr_real_operation(
    struct RAM_VALUE *dest,
    double lhs,
    int operator,
    double rhs)
{
  assert(operator== OPERATOR_PLUS || operator== OPERATOR_MINUS);

  dest->value_type = RAM_TYPE_PTR;

  switch (operator)
  {
  case OPERATOR_PLUS:
    dest->types.i = lhs + rhs;
    break;

  case OPERATOR_MINUS:
    dest->types.d = lhs - rhs;
    break;
  }
}

//
// execute_binary_expr
//
// Given two values and an operator, performs the operation
// and updates the value in the lhs. Returns true if successful,
// false if not.
//
static bool execute_binary_expr(struct STMT *stmt, struct RAM_VALUE *lhs, int operator, struct RAM_VALUE * rhs, struct RAM *memory)
{
  assert(lhs != NULL);
  assert(rhs != NULL);
  assert(operator!= OPERATOR_NO_OP);
  if (lhs->value_type == RAM_TYPE_INT && rhs->value_type == RAM_TYPE_INT)
  {
    perform_int_operation(lhs, lhs->types.i, operator, rhs->types.i);
  }
  else if (lhs->value_type == RAM_TYPE_REAL && rhs->value_type == RAM_TYPE_REAL)
  {
    perform_real_operation(lhs, lhs->types.d, operator, rhs->types.d);
  }
  else if (lhs->value_type == RAM_TYPE_INT && rhs->value_type == RAM_TYPE_REAL)
  {
    perform_real_operation(lhs, lhs->types.i, operator, rhs->types.d);
  }
  else if (lhs->value_type == RAM_TYPE_REAL && rhs->value_type == RAM_TYPE_INT)
  {
    perform_real_operation(lhs, lhs->types.d, operator, rhs->types.i);
  }
  else if (lhs->value_type == RAM_TYPE_PTR && rhs->value_type == RAM_TYPE_PTR)
  {
    struct RAM_VALUE *left = ram_read_cell_by_addr(memory, lhs->types.i);
    struct RAM_VALUE *right = ram_read_cell_by_addr(memory, rhs->types.i);
    if (left->value_type == RAM_TYPE_INT && right->value_type == RAM_TYPE_INT)
    {
      perform_int_operation(lhs, left->types.i, operator, right->types.i);
    }
    else if (left->value_type == RAM_TYPE_REAL && right->value_type == RAM_TYPE_REAL)
    {
      perform_real_operation(lhs, lhs->types.d, operator, rhs->types.d);
    }
    else if (left->value_type == RAM_TYPE_INT && right->value_type == RAM_TYPE_REAL)
    {
      perform_real_operation(lhs, lhs->types.i, operator, rhs->types.d);
    }
    else if (left->value_type == RAM_TYPE_REAL && right->value_type == RAM_TYPE_INT)
    {
      perform_real_operation(lhs, lhs->types.d, operator, rhs->types.i);
    }
    else if (left->value_type == RAM_TYPE_STR && right->value_type == RAM_TYPE_PTR)
    {
      switch (operator)
      {
      case OPERATOR_PLUS:
        lhs->types.s = dupAndConcat(left->types.s, right->types.s);
        break;
      }
    }
    else if (left->value_type == RAM_TYPE_STR && right->value_type == RAM_TYPE_STR)
    {
      switch (operator)
      {
      case OPERATOR_PLUS:
        lhs->types.s = dupAndConcat(left->types.s, right->types.s);
        break;
      }
    }
    else if (left->value_type == RAM_TYPE_PTR && right->value_type == RAM_TYPE_STR)
    {
      switch (operator)
      {
      case OPERATOR_PLUS:
        lhs->types.s = dupAndConcat(left->types.s, right->types.s);
        break;
      }
    }
  }
  else if (lhs->value_type == RAM_TYPE_PTR && rhs->value_type == RAM_TYPE_INT)
  {
    struct RAM_VALUE *ptrValue = ram_read_cell_by_addr(memory, lhs->types.i);
    if (ptrValue->value_type == RAM_TYPE_INT)
    {
      perform_int_operation(lhs, ptrValue->types.i, operator, rhs->types.i);
    }
    else if (ptrValue->value_type == RAM_TYPE_REAL)
    {
      perform_real_operation(lhs, ptrValue->types.d, operator,(double) ptrValue->types.d);
    }
    else
    {
      printf("**SEMANTIC ERROR: invalid operand types (line %d)\n", stmt->line);
    }
  }
  else if (lhs->value_type == RAM_TYPE_INT && rhs->value_type == RAM_TYPE_PTR)
  {
    struct RAM_VALUE *ptrValue = ram_read_cell_by_addr(memory, rhs->types.i);
    if (ptrValue->value_type == RAM_TYPE_INT)
    {
      perform_int_operation(lhs, ptrValue->types.i, operator, rhs->types.i);
    }
    else if (ptrValue->value_type == RAM_TYPE_REAL)
    {
      perform_real_operation(lhs, (double)lhs->types.i, operator, ptrValue->types.d);
    }
    else
    {
      printf("**SEMANTIC ERROR: invalid operand types (line %d)\n", stmt->line);
    }
  }
  else if (lhs->value_type == RAM_TYPE_PTR && rhs->value_type == RAM_TYPE_REAL)
  {
    struct RAM_VALUE *ptrValue = ram_read_cell_by_addr(memory, lhs->types.i);
    perform_real_operation(lhs, ptrValue->types.d, operator, rhs->types.d);
  }
  else if (lhs->value_type == RAM_TYPE_REAL && rhs->value_type == RAM_TYPE_PTR)
  {
    struct RAM_VALUE *ptrValue = ram_read_cell_by_addr(memory, rhs->types.i);
    // printf("%lf\n", ptrValue->value_type.d);
    // printf("%lf\n", lhs->value_type.d);
    perform_real_operation(lhs, lhs->types.d, operator, ptrValue->types.d);
  }
  else if (lhs->value_type == RAM_TYPE_REAL && rhs->value_type == RAM_TYPE_PTR)
  {
    switch (operator)
    {
    case OPERATOR_PLUS:
      lhs->types.d = lhs->types.d + rhs->types.i;
      break;

    case OPERATOR_MINUS:
      lhs->types.d = lhs->types.d - rhs->types.i;
      break;
    }
  }
  else if (lhs->value_type == RAM_TYPE_PTR && rhs->value_type == RAM_TYPE_STR)
  {
    struct RAM_VALUE *ptrValue = ram_read_cell_by_addr(memory, lhs->types.i);
    switch (operator)
    {
    case OPERATOR_PLUS:

      lhs->value_type = RAM_TYPE_STR;
      lhs->types.s = dupAndConcat(ptrValue->types.s, rhs->types.s);
      break;
    }
  }
  else if (lhs->value_type == RAM_TYPE_STR && rhs->value_type == RAM_TYPE_STR && (operator== OPERATOR_PLUS || operator== OPERATOR_EQUAL || operator== OPERATOR_NOT_EQUAL || operator== OPERATOR_LT || operator== OPERATOR_LTE || operator== OPERATOR_GT || operator== OPERATOR_GTE))
  {
    perform_str_operation(lhs, lhs->types.s, operator, rhs->types.s);
  }
  else
  {
    printf("**SEMANTIC ERROR: invalid operand types (line %d)\n", stmt->line);
    return false;
  }
  return true;
}

//
// execute_ptr_arithmetic
//
// Given a statement, memory, a pointer value, an operator, and another value,
// performs the pointer arithmetic and updates the pointer value.
// Returns true if successful, false otherwise.
//
static bool execute_ptr_arithmetic(struct STMT *stmt, struct RAM *memory, struct RAM_VALUE *ptr, int operator, struct RAM_VALUE * rhs)
{
  assert(ptr != NULL);
  assert(rhs != NULL);

  // printf("%d\n",ptr->types.i);
  // printf("%d\n", rhs->types.i);
  // Ensure the pointer value is within valid bounds
  if (ptr->types.i < 0 || ptr->types.i >= memory->num_values)
  {
    printf("**SEMANTIC ERROR: Invalid pointer address (line %d)\n", stmt->line);
    return false;
  }

  // Get the target address based on the pointer and perform the arithmetic
  struct RAM_VALUE *target = ram_read_cell_by_addr(memory, ptr->types.i);
  switch (operator)
  {
  case OPERATOR_PLUS:
    target->types.i += rhs->types.i;
    break;

  case OPERATOR_MINUS:
    target->types.i -= rhs->types.i;
    break;

  default:
    printf("**SEMANTIC ERROR: Unsupported pointer arithmetic operator (line %d)\n", stmt->line);
    return false;
  }

  // Update the pointer value to point to the new address
  // ptr->types.i = target_addr;
  // printf("here\n");
  // printf("%d\n",ptr->types.i);
  // printf("%d\n", target->types.i);
  // printf("here\n");
  // ptr->types.i = target->types.i;
  ram_write_cell_by_addr(memory, *target, ptr->types.i);

  return true;
}

//
// execute_assignment
//
// Executes an assignment statement, returning true if
// successful and false if not (an error message will be
// output before false is returned, so the caller doesn't
// need to output anything).
//
// Examples: x = 123
//           y = x ** 2
//
static bool execute_assignment(struct STMT *stmt, struct RAM *memory)
{
  struct STMT_ASSIGNMENT *assign = stmt->types.assignment;

  char *var_name = assign->var_name;

  struct RAM_VALUE *value = NULL;
  // printf(var_name);

  //
  // no pointers yet:
  //
  // assert(assign->isPtrDeref == false);
  if (assign->isPtrDeref)
  {
    // Check if the variable exists and is a pointer
    struct RAM_VALUE *pointerValue = ram_read_cell_by_id(memory, var_name);
    if (pointerValue == NULL)
    {
      printf("**SEMANTIC ERROR: name '%s' is not defined (line %d)\n", var_name, stmt->line);
      return false;
    }
    else if (pointerValue->value_type != RAM_TYPE_PTR)
    {
      printf("**SEMANTIC ERROR: invalid operand types (line %d)\n", stmt->line);
      return false;
    }

    // Check if the pointer value is a valid memory address
    if (pointerValue->types.i < 0 || pointerValue->types.i >= memory->num_values)
    {
      printf("**SEMANTIC ERROR: '%s' contains invalid address (line %d)\n", var_name, stmt->line);
      return false;
    }

    if (assign->rhs->value_type == VALUE_EXPR)
    {
      struct VALUE_EXPR *expr = assign->rhs->types.expr;
      assert(expr->lhs != NULL);

      value = get_unary_value(stmt, memory, expr->lhs);

      if (value == NULL)
        return false;

      if (expr->isBinaryExpr)
      {
        assert(expr->rhs != NULL);
        assert(expr->operator!= OPERATOR_NO_OP);

        struct RAM_VALUE *rhs_value = get_unary_value(stmt, memory, expr->rhs);

        if (rhs_value == NULL)
          return false;

        bool success = execute_ptr_arithmetic(stmt, memory, value, expr->operator, rhs_value);

        if (!success)
          return false;

        return true;
      }
    }
  }

  //
  // right now we only support expressions, no function calls:
  //

  else
  {
    assert(assign->rhs->value_type == VALUE_EXPR);

    struct VALUE_EXPR *expr = assign->rhs->types.expr;

    //
    // We have an expression, either unary or binary.
    //
    // Examples:
    //
    // 123
    // 123 + 456
    // x + 1
    // 2 * y
    // a ** b
    //

    //
    // we always have a LHS:
    //
    assert(expr->lhs != NULL);

    value = get_unary_value(stmt, memory, expr->lhs);

    if (value == NULL) // semantic error? If so, return now:
      return false;

    //
    // do we have a binary expression?
    //
    if (expr->isBinaryExpr)
    {
      assert(expr->rhs != NULL);               // we must have a RHS
      assert(expr->operator!= OPERATOR_NO_OP); // we must have an operator

      struct RAM_VALUE *rhs_value = get_unary_value(stmt, memory, expr->rhs);

      if (rhs_value == NULL) // semantic error? If so, return now:
        return false;

      //
      // perform the operation, updating value:
      //
      bool success = execute_binary_expr(stmt, value, expr->operator, rhs_value, memory);

      if (!success)
        return false;

      //
      // success! Fall through and write the updated value:
      //
    }
  }

  //
  // write the value to memory:
  //
  // printf("%s\n", var_name);
  // printf("%d\n", value->types.i);

  bool success = ram_write_cell_by_id(memory, *value, var_name);

  return success;
}

//
// execute_function_call
//
// Executes a function call statement, returning true if
// successful and false if not (an error message will be
// output before false is returned, so the caller doesn't
// need to output anything).
//
// Examples: print()
//           print(x)
//           print(123)
//
static bool execute_function_call(struct STMT *stmt, struct RAM *memory)
{
  struct STMT_FUNCTION_CALL *call = stmt->types.function_call;

  char *function_name = call->function_name;

  //
  // there's only one function we support as a statement: print
  //
  assert(strcmp(function_name, "print") == 0);

  //
  // print the parameter:
  //
  if (call->parameter == NULL)
  {
    printf("\n");
  }
  else
  {
    //
    // we have a parameter, which type of parameter?
    // Note that a parameter is a simple element, i.e.
    // identifier or literal (or True, False, None):
    //
    struct RAM_VALUE *value = get_element_value(stmt, memory, call->parameter);

    if (value == NULL) // semantic error?
      return false;

    //
    // now just print the value:
    //
    switch (value->value_type)
    {
    case RAM_TYPE_INT:
      printf("%d\n", value->types.i);
      break;

    case RAM_TYPE_REAL:
      printf("%lf\n", value->types.d);
      break;

    case RAM_TYPE_STR:
      printf("%s\n", value->types.s);
      break;

    case RAM_TYPE_BOOLEAN:
      if (value->types.i == 0)
        printf("False\n");
      else
        printf("True\n");
      break;

    case RAM_TYPE_PTR:
      printf("%d\n", value->types.i);
      break;

    default:
      printf("**EXECUTION ERROR: unexpected element type in execute_function_call");
      return false;
    }
  }

  return true;
}

//
// Public functions:
//

//
// execute
//
// Given a nuPython program graph and a memory,
// executes the statements in the program graph.
// If a semantic error occurs (e.g. type error),
// an error message is output, execution stops,
// and the function returns.
//
void execute(struct STMT *program, struct RAM *memory)
{
  //
  // execute the program, stmt by stmt, until we
  // fall off the end of the list (i.e. NULL):
  //
  struct STMT *stmt = program;

  //
  // Traverse through the body of stmts:
  //
  while (stmt != NULL)
  {

    if (stmt->stmt_type == STMT_ASSIGNMENT)
    {

      bool success = execute_assignment(stmt, memory);

      if (!success)
        return;

      stmt = stmt->types.assignment->next_stmt; // advance
    }
    else if (stmt->stmt_type == STMT_FUNCTION_CALL)
    {

      bool success = execute_function_call(stmt, memory);

      if (!success)
        return;

      stmt = stmt->types.function_call->next_stmt;
    }
    else if (stmt->stmt_type == STMT_WHILE_LOOP)
    {

      //
      // while loops are not supported:
      //
      printf("**EXECUTION ERROR\n");
      printf("**EXECUTION ERROR: while loops are not supported.\n");
      printf("**EXECUTION ERROR\n");

      break;
    }
    else if (stmt->stmt_type == STMT_IF_THEN_ELSE)
    {

      //
      // if statements are not supported:
      //
      printf("**EXECUTION ERROR\n");
      printf("**EXECUTION ERROR: if statements are not supported.\n");
      printf("**EXECUTION ERROR\n");

      break;
    }
    else
    {
      assert(stmt->stmt_type == STMT_PASS);

      //
      // nothing to do!
      //

      stmt = stmt->types.pass->next_stmt;
    }
  } // while

  //
  // if we get here, we successfully executed the
  // body of stmts:
  //
  return;
}
