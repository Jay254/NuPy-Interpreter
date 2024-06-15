/*execute.c*/

//
// << THIS FILE DEFINES FUNCTIONS FOR EVALUATING EXPRESSION AND EXPRESSION TYPES, EXECUTING STATEMENTS AND ASSIGNMENTS,HANDLING FUNCTION CALLS(PRINT), AND READING FROM AND WRITING TO MEMORY>>
//
// << JAY KIPTOO YEGON >>
// << NORTHWESTERN UNIVERSITY >>
// << COMPUTER SCIENCE MAJOR >>
//
//

#include <stdio.h> //standard input/output
#include <stdlib.h> //standard library functions
#include <stdbool.h> //boolean definitions - true or false
#include <string.h>  //string manipulation functions
#include <assert.h> //debugging assertions
#include <math.h> //math functions
#include <limits.h> //

//user-defined headers
#include "programgraph.h" //program graph
#include "ram.h" //Random Access Memory (RAM) - functions for reading and writing from memory
#include "execute.h" //execution functionality

//
// Public functions:
//
int evaluate_element(struct ELEMENT *element, struct RAM *memory, int line)
{
  // Switch based on the type of element
  switch (element->element_type)
  {
  case ELEMENT_IDENTIFIER:
  {
    // Get identifier from element
    char *identifier = element->element_value;

    // Read value from RAM using the identifier
    struct RAM_VALUE *value = ram_read_cell_by_id(memory, identifier);

    // if the value is not NULL, return integer stored in the RAM cell
    if (value != NULL)
    {
      return value->types.i;
    }
    else
    {
      // error message for an undefined identifier
      printf("**SEMANTIC ERROR: name '%s' is not defined (line %d)\n", identifier, line);

      return INT_MIN;
    }
  }
  case ELEMENT_INT_LITERAL:
    // Convert the integer literal to an integer and return
    return atoi(element->element_value);
  case ELEMENT_REAL_LITERAL:
    // Convert the real literal to an integer and return
    return atoi(element->element_value);
  case ELEMENT_STR_LITERAL:
    // warning for unsupported string literals in this context
    printf("Warning: String literals are not supported in this context\n");
    // Return 0 as placeholder
    return 0;
  case ELEMENT_TRUE:
    // Return 1 for the Boolean true
    return 1;
  case ELEMENT_FALSE:
    // and 0 for the Boolean false
    return 0;
  case ELEMENT_NONE:
    // Return 0 for the None value
    return 0;
  default:
    // error message for unsupported element type
    printf("Error: Unsupported element type\n");

    return INT_MIN;
  }
  // Return a default value (this line should not be reached)
  return 0;
}

int evaluate_unary_expression(struct UNARY_EXPR *expr, struct RAM *memory, int line)
{
  // Switch based on the type of unary expression
  switch (expr->expr_type)
  {
  case UNARY_PTR_DEREF:
  {
    // Check if pointer dereference expression is not NULL
    if (expr->element != NULL)
    {
      // Get the identifier from element
      char *identifier = expr->element->element_value;

      // Read value from RAM using the identifier
      struct RAM_VALUE *value = ram_read_cell_by_id(memory, identifier);

      // if the value is not NULL, return integer value stored in the RAM cell
      if (value != NULL)
      {
        return value->types.i;
      }
      else
      {
        // Print error message for an undefined identifier
        printf("**SEMANTIC ERROR: name '%s' is not defined (line %d)\n", identifier, line);
        // Return a special value to indicate an error
        return INT_MIN;
      }
    }
    else
    {
      // Print an error message for a NULL pointer in UNARY_PTR_DEREF
      printf("Error: NULL pointer in UNARY_PTR_DEREF\n");

      return INT_MIN;
    }
  }
  case UNARY_ADDRESS_OF:
  {
    // get identifier from the element
    char *identifier = expr->element->element_value;

    // get address of the variable from RAM
    int address = ram_get_addr(memory, identifier);

    // if the address is valid, return it
    if (address != -1)
    {
      return address;
    }
    else
    {
      // Print an error message for an undefined variable
      printf("Error: Undefined variable '%s'\n", identifier);

      return INT_MIN;
    }
  }
  case UNARY_PLUS:
    // evaluate the unary expression for result
    return evaluate_unary_expression(expr, memory, line);
  case UNARY_MINUS:
    // Return negation of result of evaluating the unary expression
    return -evaluate_unary_expression(expr, memory, line);
  case UNARY_ELEMENT:
    // Evaluate the element of the unary expression
    return evaluate_element(expr->element, memory, line);
  default:
    // Print an error message for an unsupported unary expression type
    printf("Error: Unsupported unary expression type\n");

    return INT_MIN;
  }
  // Return default value
  return 0;
}

int evaluate_binary_expression(struct VALUE_EXPR *expr, struct RAM *memory, int line)
{
  // Check if the expression is binary
  if (expr->isBinaryExpr)
  {
    // Evaluate the left-hand side of the binary expression
    int lhs_value = evaluate_unary_expression(expr->lhs, memory, line);

    // Check if the evaluation resulted in an error (INT_MIN)
    if (lhs_value == INT_MIN)
    {
      return INT_MIN;
    }

    // Evaluate the right-hand side of the binary expression
    int rhs_value = evaluate_unary_expression(expr->rhs, memory, line);

    // Check if evaluation resulted in an error (INT_MIN)
    if (rhs_value == INT_MIN)
    {
      return INT_MIN;
    }

    // Switch based on the operator of the binary expression
    switch (expr->operator)
    {
    case OPERATOR_PLUS:
      return lhs_value + rhs_value;
    case OPERATOR_MINUS:
      return lhs_value - rhs_value;
    case OPERATOR_ASTERICK:
      return lhs_value * rhs_value;
    case OPERATOR_DIV:
      // Check for division by zero
      if (rhs_value != 0)
        return lhs_value / rhs_value;
      else
      {
        printf("Error: You cannot divide by zero\n");
        // Handle the error condition
        return INT_MIN;
      }
    case OPERATOR_MOD:
      // Check for modulo by zero
      if (rhs_value != 0)
        return lhs_value % rhs_value;
      else
      {
        printf("Error: Modulo by zero\n");

        return INT_MIN;
      }
    case OPERATOR_POWER:
      // Calculate power (lhs_value ^ rhs_value)
      return (int)pow(lhs_value, rhs_value);
    default:
      // Other operators not supported
      printf("Error: Unsupported operator\n");
      // Handle error
      return INT_MIN;
    }
  }
  else
  {
    // If not a binary expression, evaluate as unary
    return evaluate_unary_expression(expr->lhs, memory, line);
  }
}

bool execute_function_call(struct STMT *stmt, struct RAM *memory)
{
  // check if function name is "print"
  if (strcmp(stmt->types.function_call->function_name, "print") == 0)
  {
    // If it has no parameter, print a new line
    if (stmt->types.function_call->parameter == NULL)
    {
      printf("\n");
    }
    // if the parameter is a string literal
    else if (stmt->types.function_call->parameter->element_type == ELEMENT_STR_LITERAL)
    {
      // print the string literal
      printf("%s\n", stmt->types.function_call->parameter->element_value);
    }
    // Check if the parameter is an integer literal
    else if (stmt->types.function_call->parameter->element_type == ELEMENT_INT_LITERAL)
    {
      // Convert to integer and print it
      printf("%d\n", atoi(stmt->types.function_call->parameter->element_value));
    }
    // if it's an identifier
    else if (stmt->types.function_call->parameter->element_type == ELEMENT_IDENTIFIER)
    {
      // evaluate the identifier to get its value
      int x = evaluate_element(stmt->types.function_call->parameter, memory, stmt->line);

      // Check if evaluation was successful (value is not INT_MIN)
      if (x != INT_MIN)
      {
        // Print value
        printf("%d\n", x);
      }
      else
      {
        // unsuccessful function call
        return false;
      }
    }
    else
    {
      // Print newline for other cases/statements
      printf("\n");
    }
    // successful program run
    return true;
  }
  else
  {
    // if function name is other than "print"
    return false;
  }
}

bool execute_assignment(struct STMT *stmt, struct RAM *memory)
{
  // extract variable name and expression from assignment statement
  char *name = stmt->types.assignment->var_name;
  struct VALUE_EXPR *expr = (struct VALUE_EXPR *)stmt->types.assignment->rhs->types.expr;

  // evaluate expression to get result value
  int result = evaluate_binary_expression(expr, memory, stmt->line);

  // check if evaluation was successful (result is not INT_MIN)
  if (result != INT_MIN)
  {
    // create a RAM_VALUE to store result with the appropriate type
    struct RAM_VALUE assignmentResult;
    assignmentResult.value_type = RAM_TYPE_INT;
    assignmentResult.types.i = result;

    // write result to memory with the specified variable name
    ram_write_cell_by_id(memory, assignmentResult, name);

    // Return true to indicate a program success
    return true;
  }
  else
  {
    // false to indicate unsuccessful assignment
    return false;
  }
}

//
// execute
//
// Given a nuPython program graph and a memory,
// executes the statements in the program graph.
// If a semantic error occurs (e.g. type error),
// and error message is output, execution stops,
// and the function returns.
//

void execute(struct STMT *program, struct RAM *memory)
{
  // Initialize the statement pointer with the provided program
  struct STMT *stmt = program;

  // Continue looping until the end of the program is reached
  while (stmt != NULL)
  {
    // Check the type of the current statement
    if (stmt->stmt_type == STMT_ASSIGNMENT)
    {
      // Execute the assignment statement and check for errors
      if (!execute_assignment(stmt, memory)) break; //exit loop if error is found

      // else ove to the next statement
      stmt = stmt->types.assignment->next_stmt;
    }

    else if (stmt->stmt_type == STMT_FUNCTION_CALL)
    {
      if (!execute_function_call(stmt, memory)) break;

      stmt = stmt->types.function_call->next_stmt;
    }

    else
    {
      // assert stmt is of type STMT_PASS
      assert(stmt->stmt_type == STMT_PASS);

      // Move to the next statement (pass) in the program
      stmt = stmt->types.pass->next_stmt;
    }
  }
}
