#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>
#include <stddef.h>

typedef signed char int8;
typedef signed short int int16;
typedef signed long int int32;
typedef signed long long int int64;

typedef unsigned char uint8;
typedef unsigned short int uint16;
typedef unsigned long int uint32;
typedef unsigned long long int uint64;

typedef unsigned int uint;
typedef uint8 byte;
typedef wchar_t rune;

typedef float float32;
typedef double float64;
typedef size_t size;

#define nil ((void*)0)

#define len(array) (sizeof(array) / sizeof(*array))
#define unused(item) ((void)(item))

#include <stdlib.h>
#include <stdio.h>

#define TO_STRING_(x) #x
#define TO_STRING(x) TO_STRING_(x)
#define panic(message) do { fputs(__FILE__ ":" TO_STRING(__LINE__) " PANICKED ~> " message, stderr); exit(1); } while (0)

#endif //TYPES_H
