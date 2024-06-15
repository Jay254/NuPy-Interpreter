/*execute.c*/

// << THIS FILE DEFINES C FUNCTIONS THAT EXECUTES PYTHON CODE GIVEN A PROGRAM GRAPH. IT HANDLES EVALUATING EXPRESSION AND EXPRESSION TYPES, EXECUTING STATEMENTS, ASSIGNMENTS, AND FUNCTION CALLS(PRINT(), INT(), FLOAT(), INPUT()), PERFORMING ITERATIONS THROUGH WHILE LOOPS, ALLOCATING MEMORY FOR VARIOUS DATA TYPES, AND READING FROM AND WRITING TO MEMORY>>
//
// << JAY KIPTOO YEGON >>
// << NORTHWESTERN UNIVERSITY >>
// << COMPUTER SCIENCE MAJOR >>


#include <stdio.h>   //standard input/output
#include <stdlib.h>  //standard library functions
#include <stdbool.h> // true, false
#include <string.h>
#include <assert.h> //debugging assertions
#include <math.h>   //math functions

#include "programgraph.h" //program graph
#include "ram.h"          //Random Access Memory (RAM) - functions for reading and writing from memory
#include "execute.h"      //execution-related functionality
#include "util.h"         //utility functions

//
// Private functions:
//

//
// get_element_value
//
// Given a basic element of an expression --- an identifier
// "x" or some kind of literal like 123 --- the value of
// this identifier or literal is returned via the reference
// parameter. Returns true if successful, false if not.
//
// Why would it fail? If the identifier does not exist in
// memory. This is a semantic error, and an error message is
// output before returning.
//
static bool get_element_value(struct STMT *stmt, struct RAM *memory, struct ELEMENT *element, struct RAM_VALUE *value)
{ // check element type
    if (element->element_type == ELEMENT_INT_LITERAL)
    { // integer literal
        char *literal = element->element_value;
        value->value_type = RAM_TYPE_INT;
        value->types.i = atoi(literal);
    }
    else if (element->element_type == ELEMENT_STR_LITERAL)
    { // string literal
        value->value_type = RAM_TYPE_STR;
        value->types.s = element->element_value;
    }
    else if (element->element_type == ELEMENT_REAL_LITERAL)
    { // real/floating point literal
        char *literal = element->element_value;
        value->value_type = RAM_TYPE_REAL;
        value->types.d = atof(literal);
    }
    else if (element->element_type == ELEMENT_TRUE)
    { // element true
        char *literal = element->element_value;
        value->value_type = RAM_TYPE_BOOLEAN;
        value->types.i = 1;
    }
    else if (element->element_type == ELEMENT_FALSE)
    { // boolean false
        char *literal = element->element_value;
        value->value_type = RAM_TYPE_BOOLEAN;
        value->types.i = 0;
    }
    else
    {
        // identifier => variable
        char *var_name = element->element_value;
        // read value from RAM using the variable name
        struct RAM_VALUE *ram_value = ram_read_cell_by_id(memory, var_name);
        // if value is not defined, output error
        if (ram_value == NULL)
        {
            printf("**SEMANTIC ERROR: name '%s' is not defined (line %d)\n", var_name, stmt->line);
            return false;
        }
        // copy value from RAM to provided RAM_VALUE struct
        *value = *ram_value;
    }

    return true;
}

//
// get_unary_value
//
// Given a unary expr, returns the value that it represents.
// This could be the result of a literal 123 or the value
// from memory for an identifier such as "x". Unary values
// may have unary operators, such as + or -, applied.
// This value is "returned" via the reference parameter.
// Returns true if successful, false if not.
//
// Why would it fail? If the identifier does not exist in
// memory. This is a semantic error, and an error message is
// output before returning.
//
static bool get_unary_value(struct STMT *stmt, struct RAM *memory, struct UNARY_EXPR *unary, struct RAM_VALUE *value)
{
    //
    // we only have simple elements so far (no unary operators):
    // so assert the unary expression type is UNARY_ELEMENT
    assert(unary->expr_type == UNARY_ELEMENT);
    // get element from the unary expression
    struct ELEMENT *element = unary->element;
    // get value of element by calling helper function get_element_value
    bool success = get_element_value(stmt, memory, element, value);

    return success;
}

// handle_addition
//
// given the lhs_value and rhs_value, this helper function performs
// addition (if integer or real), concatenation if string and stores
// the result in struct result

// program throws semantic error if the operands can't be added / invalid operand types

static bool handle_addition(struct STMT *stmt, struct RAM_VALUE lhs_value, struct RAM_VALUE rhs_value, struct RAM_VALUE *result)
{
    // Handle addition for different types
    if (lhs_value.value_type == RAM_TYPE_INT && rhs_value.value_type == RAM_TYPE_INT)
    { // integer addition
        result->value_type = RAM_TYPE_INT;
        result->types.i = lhs_value.types.i + rhs_value.types.i;
    }
    else if (lhs_value.value_type == RAM_TYPE_STR && rhs_value.value_type == RAM_TYPE_STR)
    { // string concatenation
        result->value_type = RAM_TYPE_STR;
        char *concatenated_str = malloc(strlen(lhs_value.types.s) + strlen(rhs_value.types.s) + 1);
        strcpy(concatenated_str, lhs_value.types.s);
        strcat(concatenated_str, rhs_value.types.s);
        result->types.s = concatenated_str;
    }
    else if (lhs_value.value_type == RAM_TYPE_REAL && rhs_value.value_type == RAM_TYPE_REAL)
    { // real addition
        result->value_type = RAM_TYPE_REAL;
        result->types.d = lhs_value.types.d + rhs_value.types.d;
    }
    else
    { // invalid operand types, output semantic error and return false
        printf("**SEMANTIC ERROR: invalid operand types (line %d)\n", stmt->line);
        return false;
    }
    // return true from successful addition
    return true;
}

// handle_subtraction
//
// given the lhs_value and rhs_value, this helper function performs
// subtraction (only if integer or real) and stores the result in struct result

// program throws semantic error if invalid operand types

static bool handle_subtraction(struct STMT *stmt, struct RAM_VALUE lhs_value, struct RAM_VALUE rhs_value, struct RAM_VALUE *result)
{
    // Handle subtraction for different types
    if (lhs_value.value_type == RAM_TYPE_INT && rhs_value.value_type == RAM_TYPE_INT)
    { // integer subtraction
        result->value_type = RAM_TYPE_INT;
        result->types.i = lhs_value.types.i - rhs_value.types.i;
    }
    else if (lhs_value.value_type == RAM_TYPE_REAL && rhs_value.value_type == RAM_TYPE_REAL)
    { // real subtraction
        result->value_type = RAM_TYPE_REAL;
        result->types.d = lhs_value.types.d - rhs_value.types.d;
    }
    else
    { // invalid operand types, output semantic error and return false
        printf("**SEMANTIC ERROR: invalid operand types (line %d)\n", stmt->line);
        return false;
    }
    // return true for successful subtraction
    return true;
}

// handle_multiplication
//
// Given the lhs_value and rhs_value, this helper function performs
// multiplication (only if both operands are integers or both are real numbers)
// and stores the result in the struct result.

// If the operand types are not valid for multiplication, it outputs a semantic error.

static bool handle_multiplication(struct STMT *stmt, struct RAM_VALUE lhs_value, struct RAM_VALUE rhs_value, struct RAM_VALUE *result)
{ // check operand types and perform multiplication
    if (lhs_value.value_type == RAM_TYPE_INT && rhs_value.value_type == RAM_TYPE_INT)
    { // integer multiplication
        result->value_type = RAM_TYPE_INT;
        result->types.i = lhs_value.types.i * rhs_value.types.i;
    }
    else if (lhs_value.value_type == RAM_TYPE_REAL && rhs_value.value_type == RAM_TYPE_REAL)
    { // real multiplication
        result->value_type = RAM_TYPE_REAL;
        result->types.d = lhs_value.types.d * rhs_value.types.d;
    }
    else
    {
        // Invalid operation for the given types, output semantic error and return false
        printf("**SEMANTIC ERROR: invalid operand types (line %d)\n", stmt->line);
        return false;
    }
    // return true from successful multiplication
    return true;
}

// handle_power
//
// Given the lhs_value and rhs_value, this helper function performs
// exponentiation (power) and stores the result in the struct result.

// If the operand types are not valid for power operation, it outputs a semantic error.

static bool handle_power(struct STMT *stmt, struct RAM_VALUE lhs_value, struct RAM_VALUE rhs_value, struct RAM_VALUE *result)
{ // check operand types
    if (lhs_value.value_type == RAM_TYPE_INT && rhs_value.value_type == RAM_TYPE_INT)
    { // integer power
        result->value_type = RAM_TYPE_INT;
        result->types.i = pow(lhs_value.types.i, rhs_value.types.i);
    }
    else if (lhs_value.value_type == RAM_TYPE_REAL && rhs_value.value_type == RAM_TYPE_REAL)
    { // real power
        result->value_type = RAM_TYPE_REAL;
        result->types.d = pow(lhs_value.types.d, rhs_value.types.d);
    }
    else
    { // Invalid operation for the given types, output semantic error and return false
        printf("**SEMANTIC ERROR: invalid operand types (line %d)\n", stmt->line);
        return false;
    }
    // return true to indicate successful power operation
    return true;
}

// handle_division
//
// Given the lhs_value and rhs_value, this helper function performs
// division and stores the result in the struct result.

// The function first checks for division by zero errors, both for integer and real divisions.
// If division by zero is detected, it outputs an execution error.

// Then, the function checks the types of the operands and performs division accordingly.
// If the operand types are not valid for division, it outputs a semantic error.

static bool handle_division(struct STMT *stmt, struct RAM_VALUE lhs_value, struct RAM_VALUE rhs_value, struct RAM_VALUE *result)
{ // check for division by zero errors
    if (rhs_value.value_type == RAM_TYPE_INT && rhs_value.types.i == 0)
    { // division by zero error for integer division
        printf("**EXECUTION ERROR: division by zero (line %d)\n", stmt->line);
        return false;
    }
    else if (rhs_value.value_type == RAM_TYPE_REAL && rhs_value.types.d == 0.0)
    { // division by zero error for real division
        printf("**EXECUTION ERROR: division by zero (line %d)\n", stmt->line);
        return false;
    }
    // Check operand types
    if (lhs_value.value_type == RAM_TYPE_INT && rhs_value.value_type == RAM_TYPE_INT)
    { // integer division
        result->value_type = RAM_TYPE_INT;
        result->types.i = lhs_value.types.i / rhs_value.types.i;
    }
    else if (lhs_value.value_type == RAM_TYPE_REAL && rhs_value.value_type == RAM_TYPE_REAL)
    { // real division
        result->value_type = RAM_TYPE_REAL;
        result->types.d = lhs_value.types.d / rhs_value.types.d;
    }
    else
    {
        // invalid operation for the given types, output semantic error and return false
        printf("**SEMANTIC ERROR: invalid operand types (line %d)\n", stmt->line);
        return false;
    }
    // return true to indicate successful division
    return true;
}

// handle_modulus
//
// Given the lhs_value and rhs_value, this helper function performs
// modulus operation and stores the result in the struct result.

// The function first checks for division by zero errors, both for integer and real modulus operations.
// If division by zero is detected, it outputs an execution error.

// Then, the function checks the types of the operands and performs modulus accordingly.
// If the operand types are not valid for modulus operation, it outputs a semantic error.

static bool handle_modulus(struct STMT *stmt, struct RAM_VALUE lhs_value, struct RAM_VALUE rhs_value, struct RAM_VALUE *result)
{ // Check for division by zero errors
    if (rhs_value.value_type == RAM_TYPE_INT && rhs_value.types.i == 0)
    {
        // division by zero error for integer modulus
        printf("**EXECUTION ERROR: division by zero (line %d)\n", stmt->line);
        return false;
    }
    else if (rhs_value.value_type == RAM_TYPE_REAL && rhs_value.types.d == 0.0)
    {
        // division by zero error for real modulus
        printf("**EXECUTION ERROR: division by zero (line %d)\n", stmt->line);
        return false;
    }
    // Check operand types and perform modulus
    if (lhs_value.value_type == RAM_TYPE_INT && rhs_value.value_type == RAM_TYPE_INT)
    {
        // integer modulus
        result->value_type = RAM_TYPE_INT;
        result->types.i = lhs_value.types.i % rhs_value.types.i;
    }
    else if (lhs_value.value_type == RAM_TYPE_REAL && rhs_value.value_type == RAM_TYPE_REAL)
    {
        // real modulus
        result->value_type = RAM_TYPE_REAL;
        result->types.d = fmod(lhs_value.types.d, rhs_value.types.d);
    }
    else
    {
        // invalid operands, output semantic error and return false
        printf("**SEMANTIC ERROR: invalid operand types (line %d)\n", stmt->line);
        return false;
    }
    // return true for successful modulus operation
    return true;
}

// handle_equal
//
// Given the lhs_value and rhs_value, this helper function performs
// equality comparison and stores the result in the struct result.

// The function checks the types of the operands and performs equality comparison accordingly.
// If the operand types are not valid for equality comparison, it outputs a semantic error.

static bool handle_equal(struct STMT *stmt, struct RAM_VALUE lhs_value, struct RAM_VALUE rhs_value, struct RAM_VALUE *result)
{ // check operand types and perform equality comparison
    if (lhs_value.value_type == RAM_TYPE_INT && rhs_value.value_type == RAM_TYPE_INT)
    { // integer equality comparison
        result->value_type = RAM_TYPE_BOOLEAN;
        result->types.i = (lhs_value.types.i == rhs_value.types.i) ? 1 : 0;
    }
    else if (lhs_value.value_type == RAM_TYPE_REAL && rhs_value.value_type == RAM_TYPE_REAL)
    { // real equality comparison with a small tolerance (0.001)
        result->value_type = RAM_TYPE_BOOLEAN;
        result->types.i = (fabs(lhs_value.types.d - rhs_value.types.d) < 0.001) ? 1 : 0;
    }
    else if (lhs_value.value_type == RAM_TYPE_STR && lhs_value.value_type == RAM_TYPE_STR)
    { // string equality comparison
        result->value_type = RAM_TYPE_BOOLEAN;
        result->types.i = (strcmp(lhs_value.types.s, rhs_value.types.s) == 0) ? 1 : 0;
    }
    else
    { // invalid operands, output semantic error and return false
        printf("**SEMANTIC ERROR: invalid operand types (line %d)\n", stmt->line);
        return false;
    }
    // return true to indicate successful equality comparison
    return true;
}

// handle_not_equal
//
// Given the lhs_value and rhs_value, this helper function performs
// inequality comparison and stores the result in the struct result.

// The function checks the types of the operands and performs inequality comparison accordingly.
// If the operand types are not valid for inequality comparison, it outputs a semantic error.

static bool handle_not_equal(struct STMT *stmt, struct RAM_VALUE lhs_value, struct RAM_VALUE rhs_value, struct RAM_VALUE *result)
{ // check operand types and perform inequality comparison
    if (lhs_value.value_type == RAM_TYPE_INT && rhs_value.value_type == RAM_TYPE_INT)
    { // integer inequality comparison
        result->value_type = RAM_TYPE_BOOLEAN;
        result->types.i = (lhs_value.types.i != rhs_value.types.i) ? 1 : 0;
    }
    else if (lhs_value.value_type == RAM_TYPE_REAL && rhs_value.value_type == RAM_TYPE_REAL)
    { // real inequality comparison with a small tolerance (0.001)
        result->value_type = RAM_TYPE_BOOLEAN;
        result->types.i = (fabs(lhs_value.types.d - rhs_value.types.d) > 0.001) ? 1 : 0;
    }
    else if (lhs_value.value_type == RAM_TYPE_STR && lhs_value.value_type == RAM_TYPE_STR)
    { // string inequality comparison
        result->value_type = RAM_TYPE_BOOLEAN;
        result->types.i = (strcmp(lhs_value.types.s, rhs_value.types.s) != 0) ? 1 : 0;
    }
    else
    { // invalid operation for the given types, output semantic error and return false
        printf("**SEMANTIC ERROR: invalid operand types (line %d)\n", stmt->line);
        return false;
    }
    // return true to indicate successful inequality comparison
    return true;
}

// handle_less_than
//
// Given the lhs_value and rhs_value, this helper function performs
// less-than comparison and stores the result in the struct result.

// The function checks the types of the operands and performs less-than comparison.
// If the operand types are invalid for less-than comparison, it outputs a semantic error.

static bool handle_less_than(struct STMT *stmt, struct RAM_VALUE lhs_value, struct RAM_VALUE rhs_value, struct RAM_VALUE *result)
{ // Check operand types and perform less-than comparison
    if (lhs_value.value_type == RAM_TYPE_INT && rhs_value.value_type == RAM_TYPE_INT)
    { // integer less-than comparison
        result->value_type = RAM_TYPE_BOOLEAN;
        result->types.i = (lhs_value.types.i < rhs_value.types.i) ? 1 : 0;
    }
    else if (lhs_value.value_type == RAM_TYPE_REAL && rhs_value.value_type == RAM_TYPE_REAL)
    { // real less-than comparison
        result->value_type = RAM_TYPE_BOOLEAN;
        result->types.i = (lhs_value.types.d < rhs_value.types.d) ? 1 : 0;
    }
    else if (lhs_value.value_type == RAM_TYPE_STR && lhs_value.value_type == RAM_TYPE_STR)
    { // string less-than comparison
        result->value_type = RAM_TYPE_BOOLEAN;
        result->types.i = (strcmp(lhs_value.types.s, rhs_value.types.s) < 0) ? 1 : 0;
    }
    else
    { // invalid operation for the given types, output semantic error and return false
        printf("**SEMANTIC ERROR: invalid operand types (line %d)\n", stmt->line);
        return false;
    }
    // return true to indicate successful less-than comparison
    return true;
}

// handle_less_than_or_equal
//
// Given the lhs_value and rhs_value, this helper function performs
// less-than-or-equal-to comparison and stores the result in the struct result.

// The function checks the types of the operands and performs less-than-or-equal-to comparison accordingly.
// If the operand types are not valid for less-than-or-equal-to comparison, it outputs a semantic error.

static bool handle_less_than_or_equal(struct STMT *stmt, struct RAM_VALUE lhs_value, struct RAM_VALUE rhs_value, struct RAM_VALUE *result)
{ // check operand types and perform less-than-or-equal-to comparison
    if (lhs_value.value_type == RAM_TYPE_INT && rhs_value.value_type == RAM_TYPE_INT)
    { // integer less-than-or-equal-to comparison
        result->value_type = RAM_TYPE_BOOLEAN;
        result->types.i = (lhs_value.types.i <= rhs_value.types.i) ? 1 : 0;
    }
    else if (lhs_value.value_type == RAM_TYPE_REAL && rhs_value.value_type == RAM_TYPE_REAL)
    { // real less-than-or-equal-to comparison
        result->value_type = RAM_TYPE_BOOLEAN;
        result->types.i = (lhs_value.types.d <= rhs_value.types.d + 0.0001) ? 1 : 0;
    }
    else if (lhs_value.value_type == RAM_TYPE_STR && rhs_value.value_type == RAM_TYPE_STR)
    { // string less-than-or-equal-to comparison
        result->value_type = RAM_TYPE_BOOLEAN;
        result->types.i = (strcmp(lhs_value.types.s, rhs_value.types.s) <= 0) ? 1 : 0;
    }
    else
    { // invalid operation for the given types, output semantic error and return false
        printf("**SEMANTIC ERROR: invalid operand types (line %d)\n", stmt->line);
        return false;
    }
    // return true to indicate successful less-than-or-equal-to comparison
    return true;
}

// handle_greater_than
//
// Given the lhs_value and rhs_value, this helper function performs
// greater-than comparison and stores the result in the struct result.

// The function checks the types of the operands and performs greater-than comparison accordingly.
// If the operand types are not valid for greater-than comparison, it outputs a semantic error.

static bool handle_greater_than(struct STMT *stmt, struct RAM_VALUE lhs_value, struct RAM_VALUE rhs_value, struct RAM_VALUE *result)
{ // check operand types and perform greater-than comparison
    if (lhs_value.value_type == RAM_TYPE_INT && rhs_value.value_type == RAM_TYPE_INT)
    { // integer greater-than comparison
        result->value_type = RAM_TYPE_BOOLEAN;
        result->types.i = (lhs_value.types.i > rhs_value.types.i) ? 1 : 0;
    }
    else if (lhs_value.value_type == RAM_TYPE_REAL && rhs_value.value_type == RAM_TYPE_REAL)
    { // real greater-than comparison
        result->value_type = RAM_TYPE_BOOLEAN;
        result->types.i = (lhs_value.types.d > rhs_value.types.d + 0.0001) ? 1 : 0;
    }
    else if (lhs_value.value_type == RAM_TYPE_STR && rhs_value.value_type == RAM_TYPE_STR)
    { // string greater-than comparison
        result->value_type = RAM_TYPE_BOOLEAN;
        result->types.i = (strcmp(lhs_value.types.s, rhs_value.types.s) > 0) ? 1 : 0;
    }
    else
    { // invalid operation for the given types, output semantic error and return false
        printf("**SEMANTIC ERROR: invalid operand types (line %d)\n", stmt->line);
        return false;
    }
    // return true to indicate successful greater-than comparison
    return true;
}

// handle_greater_than_or_equal
//
// Given the lhs_value and rhs_value, this helper function performs
// greater-than-or-equal comparison and stores the result in the struct result.

// The function checks the types of the operands and performs greater-than-or-equal comparison accordingly.
// If the operand types are not valid for greater-than-or-equal comparison, it outputs a semantic error.

static bool handle_greater_than_or_equal(struct STMT *stmt, struct RAM_VALUE lhs_value, struct RAM_VALUE rhs_value, struct RAM_VALUE *result)
{ // check operand types and perform greater-than-or-equal comparison accordingly
    if (lhs_value.value_type == RAM_TYPE_INT && rhs_value.value_type == RAM_TYPE_INT)
    { // integer greater-than-or-equal comparison
        result->value_type = RAM_TYPE_BOOLEAN;
        result->types.i = (lhs_value.types.i >= rhs_value.types.i) ? 1 : 0;
    }
    else if (lhs_value.value_type == RAM_TYPE_REAL && rhs_value.value_type == RAM_TYPE_REAL)
    { // real greater-than-or-equal comparison
        result->value_type = RAM_TYPE_BOOLEAN;
        result->types.i = (lhs_value.types.d > rhs_value.types.d - 0.0001) ? 1 : 0;
    }
    else if (lhs_value.value_type == RAM_TYPE_STR && rhs_value.value_type == RAM_TYPE_STR)
    { // string greater-than-or-equal comparison
        result->value_type = RAM_TYPE_BOOLEAN;
        result->types.i = (strcmp(lhs_value.types.s, rhs_value.types.s) >= 0) ? 1 : 0;
    }
    else
    { // invalid operation for the given types, output semantic error and return false
        printf("**SEMANTIC ERROR: invalid operand types (line %d)\n", stmt->line);
        return false;
    }
    // return true to indicate successful greater-than-or-equal comparison
    return true;
}
//
// execute_binary_expr
//
// Given two values and an operator, performs the operation
// and updates the value in the lhs (which can be updated
// because a pointer to the value is passed in). Returns
// true if successful and false if not.
//

static bool execute_binary_expr(struct STMT *stmt, struct RAM *memory, struct VALUE_EXPR *binary, struct RAM_VALUE *result)
{
    // ensure the binary expression has a valid left-hand side and operator
    assert(binary->lhs != NULL);
    assert(binary->operator!= OPERATOR_NO_OP);
    // initialize variables to store the left-hand side (lhs) and right-hand side (rhs) values
    struct RAM_VALUE lhs_value, rhs_value;
    // Retrieve left-hand side value
    bool success = get_unary_value(stmt, memory, binary->lhs, &lhs_value);

    if (!success)
        return false;
    // if the binary expression is not a unary operation, retrieve the value of the right-hand side
    if (binary->isBinaryExpr)
    {
        assert(binary->rhs != NULL);
        success = get_unary_value(stmt, memory, binary->rhs, &rhs_value);

        if (!success)
            return false;
    }
    // perform type conversion if one operand is real and the other is integer
    if ((lhs_value.value_type == RAM_TYPE_REAL && rhs_value.value_type == RAM_TYPE_INT) || (lhs_value.value_type == RAM_TYPE_INT && rhs_value.value_type == RAM_TYPE_REAL))
    { // convert left-hand side to real if needed
        if (lhs_value.value_type != RAM_TYPE_REAL)
        {
            lhs_value.value_type = RAM_TYPE_REAL;
            lhs_value.types.d = (double)lhs_value.types.i;
        }
        // convert the right-hand side to real if needed
        if (rhs_value.value_type != RAM_TYPE_REAL)
        {
            rhs_value.value_type = RAM_TYPE_REAL;
            rhs_value.types.d = (double)rhs_value.types.i;
        }
    }
    //
    // perform specified binary operation based on the operator
    //
    switch (binary->operator)
    {
    case OPERATOR_PLUS:
        return handle_addition(stmt, lhs_value, rhs_value, result);
    case OPERATOR_MINUS:
        return handle_subtraction(stmt, lhs_value, rhs_value, result);
    case OPERATOR_ASTERISK:
        return handle_multiplication(stmt, lhs_value, rhs_value, result);
    case OPERATOR_POWER:
        return handle_power(stmt, lhs_value, rhs_value, result);
    case OPERATOR_DIV:
        return handle_division(stmt, lhs_value, rhs_value, result);
    case OPERATOR_MOD:
        return handle_modulus(stmt, lhs_value, rhs_value, result);
    case OPERATOR_EQUAL:
        return handle_equal(stmt, lhs_value, rhs_value, result);
    case OPERATOR_NOT_EQUAL:
        return handle_not_equal(stmt, lhs_value, rhs_value, result);
    case OPERATOR_LT:
        return handle_less_than(stmt, lhs_value, rhs_value, result);
    case OPERATOR_LTE:
        return handle_less_than_or_equal(stmt, lhs_value, rhs_value, result);
    case OPERATOR_GT:
        return handle_greater_than(stmt, lhs_value, rhs_value, result);
    case OPERATOR_GTE:
        return handle_greater_than_or_equal(stmt, lhs_value, rhs_value, result);
    default:
        //
        // did we miss something? return semantic error for invalid operator
        // return false
        printf("**SEMANTIC ERROR: invalid operand types (line %d)\n", stmt->line);
        return false;
    }
    // return true to indicate successful execution of the binary expression
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
{ // extract assignment details of assignment
    struct STMT_ASSIGNMENT *assign = stmt->types.assignment;
    char *var_name = assign->var_name;
    // validate assignment does not involve pointer dereferencing
    assert(assign->isPtrDeref == false);
    // ensure right-hand side (rhs) of the assignment is a valid expression
    assert(assign->rhs->value_type == VALUE_EXPR || assign->rhs->value_type == VALUE_FUNCTION_CALL);
    // initialize a variable to store the computed value of right-hand side
    struct RAM_VALUE value;
    // process the right-hand side based on its type (expression or function call)
    if (assign->rhs->value_type == VALUE_EXPR)
    {
        struct VALUE_EXPR *expr = assign->rhs->types.expr;
        assert(expr->lhs != NULL);                                       // ensure expression has valid left-hand side (lhs)
        bool success = get_unary_value(stmt, memory, expr->lhs, &value); // retrieve left-hand side value

        if (!success)
            return false;
        // if expression involves binary operation, handle it
        if (expr->isBinaryExpr)
        {
            assert(expr->rhs != NULL);
            assert(expr->operator!= OPERATOR_NO_OP);
            struct RAM_VALUE rhs_value;
            success = get_unary_value(stmt, memory, expr->rhs, &rhs_value); // retrieve right-hand side value

            if (!success)
            {
                return false;
            }
            // compute result of binary operation and assign it to 'value'
            struct RAM_VALUE result;
            success = execute_binary_expr(stmt, memory, expr, &result);

            if (!success)
                return false;

            value = result;
        }
    }
    else if (assign->rhs->value_type == VALUE_FUNCTION_CALL)
    {
        struct VALUE_FUNCTION_CALL *func_call = assign->rhs->types.function_call;
        // handle the input() function
        if (strcmp(func_call->function_name, "input") == 0)
        { // assert function call has a string literal parameter
            assert(func_call->parameter->element_type == ELEMENT_STR_LITERAL);
            printf("%s", func_call->parameter->element_value);

            char line[256];
            fgets(line, sizeof(line), stdin); // dynamically allocate memory to hold user's input
            line[strcspn(line, "\n")] = '\0'; // delete EOL chars from input

            value.value_type = RAM_TYPE_STR;
            value.types.s = dupString(line); // Duplicate the input string and assign it to 'value'
        }
        // Handle int() logic
        else if (strcmp(func_call->function_name, "int") == 0)
        {
            struct ELEMENT *param = func_call->parameter;
            // ensure function call has a valid parameter of type identifier
            if (param != NULL && param->element_type == ELEMENT_IDENTIFIER)
            {
                struct RAM_VALUE param_value;
                bool success = get_element_value(stmt, memory, param, &param_value); // retrieve the value associated with the identifier

                if (!success || param_value.value_type != RAM_TYPE_STR)
                {
                    printf("**SEMANTIC ERROR: Invalid parameter for int() (line %d)\n", stmt->line);
                    return false;
                }
                int int_value = atoi(param_value.types.s); // Convert the string value to an integer
                // check for invalid string representation of an integer
                if (int_value == 0 && param_value.types.s[0] != '0')
                {
                    printf("**SEMANTIC ERROR: invalid string for int() (line %d)\n", stmt->line);
                    return false;
                }
                // Successfully converted to int? assign it to 'value'
                value.value_type = RAM_TYPE_INT;
                value.types.i = int_value;
            }
            else
            {
                printf("**SEMANTIC ERROR: Invalid parameter for int() (line %d)\n", stmt->line);
                return false;
            }
        }
        // Handle float() logic
        else if (strcmp(func_call->function_name, "float") == 0)
        {
            struct ELEMENT *param = func_call->parameter;
            // ensure function call has a valid parameter of type identifier
            if (param != NULL && param->element_type == ELEMENT_IDENTIFIER)
            {
                struct RAM_VALUE param_value;
                bool success = get_element_value(stmt, memory, param, &param_value); // retrieve the value associated with the identifier

                if (!success || param_value.value_type != RAM_TYPE_STR)
                {
                    printf("**SEMANTIC ERROR: Invalid parameter for float() (line %d)\n", stmt->line);
                    return false;
                }
                double float_value = atof(param_value.types.s); // Convert the string value to a float
                // check for invalid string representation of a float
                if (float_value == 0.0 && param_value.types.s[0] != '0')
                {
                    printf("**SEMANTIC ERROR: invalid string for float() (line %d)\n", stmt->line);
                    return false;
                }
                // Successfully converted to float? assign it to 'value'
                value.value_type = RAM_TYPE_REAL;
                value.types.d = float_value;
            }
            else
            {
                printf("**SEMANTIC ERROR: Invalid parameter for float() (line %d)\n", stmt->line);
                return false;
            }
        }
        else
        {
            printf("**EXECUTION ERROR: Unknown function call: %s (line %d)\n", func_call->function_name, stmt->line);
            return false;
        }
    }
    struct RAM_VALUE ram_value;
    ram_value = value;
    bool success = ram_write_cell_by_id(memory, ram_value, var_name); // write the computed value to the specified variable in the RAM

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

    //
    // for now we are assuming it's a call to print:
    //
    char *function_name = call->function_name;

    assert(strcmp(function_name, "print") == 0);

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
        char *element_value = call->parameter->element_value;

        if (call->parameter->element_type == ELEMENT_STR_LITERAL)
        {
            printf("%s\n", element_value);
        }
        else if (call->parameter->element_type == ELEMENT_INT_LITERAL)
        {
            printf("%d\n", atoi(element_value));
        }
        else if (call->parameter->element_type == ELEMENT_REAL_LITERAL)
        {
            printf("%lf\n", atof(element_value));
        }
        else if (call->parameter->element_type == ELEMENT_TRUE)
        {
            printf("%s\n", element_value);
        }
        else if (call->parameter->element_type == ELEMENT_FALSE)
        {
            printf("%s\n", element_value);
        }
        else
        {
            //
            // right now we are assuming ints or variables containing
            // ints, so call our get_element function to obtain the
            // integer value:
            struct RAM_VALUE value;

            bool success = get_element_value(stmt, memory, call->parameter, &value);

            if (!success)
                return false;

            if (value.value_type == RAM_TYPE_BOOLEAN)
            {
                printf("%s\n", value.types.i ? "True" : "False");
            }
            else
            {
                switch (value.value_type)
                {
                case RAM_TYPE_INT:
                    printf("%d\n", value.types.i);
                    break;
                case RAM_TYPE_REAL:
                    printf("%lf\n", value.types.d);
                    break;
                case RAM_TYPE_STR:
                    printf("%s\n", value.types.s);
                    break;
                default:
                    printf("**ERROR: Unsupported data type in print statement\n");
                    return false;
                }
            }
        }
    } // else

    return true;
}

// execute_while_loop
//
// Given a while loop statement, this function evaluates the condition expression and iteratively executes
// the statements within the loop body as long as the condition remains true. It handles assignments, function
// calls, and nested while loops. The function returns true if the loop is executed successfully.

static bool execute_while_loop(struct STMT *stmt, struct RAM *memory)
{ // retrieve the condition expression and loop body from the while loop statement
    struct VALUE_EXPR *condition_expr = stmt->types.while_loop->condition;
    struct STMT *loop_body = stmt->types.while_loop->loop_body;
    // save the next_stmt pointer before entering the loop
    struct STMT *next_stmt = stmt->types.while_loop->next_stmt;
    // evaluate the condition only once before entering the loop
    struct RAM_VALUE condition_value;
    bool success = execute_binary_expr(stmt, memory, condition_expr, &condition_value);
    // check for errors in the condition evaluation
    if (!success || condition_value.value_type != RAM_TYPE_BOOLEAN)
    {
        return false;
    }
    // enter the while loop based on the evaluated condition
    while (condition_value.types.i != 0)
    { // execute the statements within the while loop body
        struct STMT *current_stmt = loop_body;
        // iterate over each statement in the while loop body
        while (current_stmt != NULL && current_stmt != next_stmt)
        {
            if (current_stmt->stmt_type == STMT_ASSIGNMENT)
            { // execute an assignment statement and move to the next statement
                success = execute_assignment(current_stmt, memory);
                if (!success)
                    return false;
                current_stmt = current_stmt->types.assignment->next_stmt;
            }
            else if (current_stmt->stmt_type == STMT_FUNCTION_CALL)
            { // execute a function call statement and move to the next statement
                success = execute_function_call(current_stmt, memory);
                if (!success)
                    return false;
                current_stmt = current_stmt->types.function_call->next_stmt;
            }
            else if (current_stmt->stmt_type == STMT_WHILE_LOOP)
            { // recursively execute a nested while loop and move to the next statement
                success = execute_while_loop(current_stmt, memory);
                if (!success)
                    return false;
                current_stmt = current_stmt->types.while_loop->next_stmt;
            }
            else
            { // Assertion: Unknown statement type (should be STMT_PASS)
                assert(current_stmt->stmt_type == STMT_PASS);
                current_stmt = current_stmt->types.pass->next_stmt;
            }
        }
        // evaluate the condition again at the end of each iteration
        success = execute_binary_expr(stmt, memory, condition_expr, &condition_value);

        // check for errors in the condition evaluation
        if (!success || condition_value.value_type != RAM_TYPE_BOOLEAN)
        {
            return false;
        }
    }
    // restore the next_stmt pointer after exiting the loop
    stmt->types.while_loop->next_stmt = next_stmt;
    // the loop executed successfully
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
    struct STMT *stmt = program;

    //
    // traverse through the program statements:
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
            bool success = execute_while_loop(stmt, memory);
            if (!success)
                return;
            stmt = stmt->types.while_loop->next_stmt;
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
    // done:
    //
    return;
}