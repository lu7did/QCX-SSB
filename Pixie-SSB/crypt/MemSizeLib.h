/*--------------------------------------------------------------------------------------------------
//* MemSizeLib Version 1.0
//*--------------------------------------------------------------------------------------------------
//* Solo para uso de radioaficionados, prohibido su utilizacion comercial
//* Copyright 2018 Dr. Pedro E. Colla (LU7DID)
//*--------------------------------------------------------------------------------------------------
#include <avr/pgmspace.h>
extern unsigned int __heap_start;
extern void *__brkval;

/*
 * The free list structure as maintained by the
 * avr-libc memory allocation routines.
 */
struct __freelist
{
  size_t sz;
  struct __freelist *nx;
};

/* The head of the free list structure */
extern struct __freelist *__flp;
//*==============================================================================================================================
// MemoryFree library based on code posted here:
// http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1213583720/15
// Extended by Matthew Murdoch to include walking of the free list.

#ifndef  MEMORY_FREE_H
#define MEMORY_FREE_H

#ifdef __cplusplus
extern "C" {
#endif

int freeMemory();

#ifdef  __cplusplus
}
#endif
#endif
extern unsigned int __bss_end;
extern unsigned int __heap_start;
extern void *__brkval;

//*==============================================================================================================
//* Calculates the size of the free list                                                                        */
//*==============================================================================================================
//*--- freeListSize
int freeListSize()
{
  struct __freelist* current;
  int total = 0;
  for (current = __flp; current; current = current->nx)
  {
    total += 2; /* Add two bytes for the memory block's header  */
    total += (int) current->sz;
  }

  return total;
}
//*--- Total Free memory
int freeMemory()
{
  int free_memory;
  if ((int)__brkval == 0)
  {
    free_memory = ((int)&free_memory) - ((int)&__heap_start);
  }
  else
  {
    free_memory = ((int)&free_memory) - ((int)__brkval);
    free_memory += freeListSize();
  }
  return free_memory;
}
