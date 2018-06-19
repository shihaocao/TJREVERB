#include <libserialport.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

#ifndef BAD_ARRAYLIST_H
#define BAD_ARRAYLIST_H

//Array List structure --> used for byte_buff handling.
typedef struct{
    unsigned char* array; //stores char values
    size_t space; //amount of total space 
    size_t size; //amount of space taken up so far

} ArrayList;

void initArrayList(ArrayList** arr, size_t initialSize);

//adds data to arraylist at end of array
void add(ArrayList* arr, char data);

//removes value stored at index
//resizes arraylist
int drop(ArrayList* arr, int index);

//returns 1 if arr1 == arr2 (by values stored in array); 0 otherwise
//returns 0 if either array is NULL (even if both arrays are NULL)
int equals(ArrayList* arr1, ArrayList* arr2);

//returns new arraylist that is contains the same values as arr from start to end-1
//returns null if end or start are out of bounds of arr
ArrayList* extract(ArrayList* arr, int start, int end);

//prints array list arr
void print_array_list(ArrayList* arr);

//deletes array for memory management
void delete(ArrayList* arr);

#endif
