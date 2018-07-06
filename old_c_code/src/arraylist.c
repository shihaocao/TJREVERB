#include <libserialport.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

//Array List structure --> used for byte_buff handling.
typedef struct{
    unsigned char* array; //stores char values
    size_t space; //amount of total space 
    size_t size; //amount of space taken up so far

} ArrayList;

void initArrayList(ArrayList** arr, size_t initialSize){
    *arr = malloc(sizeof(ArrayList));
    (*arr)->array  = (unsigned char*)malloc(initialSize * sizeof(int));
    (*arr)->space = initialSize;
    (*arr)->size = 0;
}

//adds data to arraylist at end of array
void add(ArrayList* arr, char data){
    if(arr->size == arr->space){
        arr->space *=2;
        arr-> array = (unsigned char*)realloc(arr->array, arr->size * sizeof(int)); //doubles array space if no more space
    
    }
    arr->array[arr->size] = data;
    arr->size += 1;

}

//removes value stored at index
//resizes arraylist
int drop(ArrayList* arr, int index){
    int ret = arr->array[index];
    for(int i=index ; i<arr->size-1 ; i++){
        arr->array[i] = arr->array[i + 1];
    
    }
    arr->size = arr->size-1;
    return ret;

}

//returns 1 if arr1 == arr2 (by values stored in array); 0 otherwise
//returns 0 if either array is NULL (even if both arrays are NULL)
int equals(ArrayList* arr1, ArrayList* arr2){
    if(arr1 == NULL){
        return 0;
    
    }
    if(arr2 == NULL){
        return 0;
    
    }
    if(arr1->size == arr2->size){
        for(int i=0 ; i < arr1->size ; i++){
            if(arr1->array[i] != arr2->array[i]){
                return 0;
            
            }
        
        }
        return 1;   
    
    }
    else{
        return 0;
    
    }

}

//returns new arraylist that is contains the same values as arr from start to end-1
//returns null if end or start are out of bounds of arr
ArrayList* extract(ArrayList* arr, int start, int end){
    ArrayList* temp;
    initArrayList(temp, 15);
    if(end > arr->size || start < 0){
        return NULL;
    
    }
    for(int i=start ; i < end ; i++){
        add(temp, arr->array[1]);
    
    }
    return temp;

}


//prints array list arr
void print_array_list(ArrayList* arr){
    for(int i=0 ; i<arr->size ; i++){
        char ret = arr->array[i];
        fprintf(stderr, "%c", ret);  
    
    }

}


//deletes array for memory management
void delete(ArrayList* arr){
    free(arr->array);
    arr->array = NULL;
    arr->size = 0;
    arr->space = 0;

}

