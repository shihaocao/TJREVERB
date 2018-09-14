/*************************************************************************
** File:
**   $Id: mm_mem8.c 1.8 2011/12/05 15:17:20GMT-05:00 jmdagost Exp  $
**
** Purpose: 
**   Provides CFS Memory Manager functions that are used
**   for the conditionally compiled MM_MEM8 optional memory type.
**
**   $Log: mm_mem8.c  $
**   Revision 1.8 2011/12/05 15:17:20GMT-05:00 jmdagost 
**   Added check for zero bytes read from file load (with event message on error)
**   Revision 1.7 2011/06/23 12:02:37EDT jmdagost 
**   Corrected type for local BytesRead, initialized local variables.
**   Revision 1.6 2010/11/29 13:35:21EST jmdagost 
**   Replaced ifdef tests with if-true tests.
**   Revision 1.5 2009/06/12 14:37:28EDT rmcgraw 
**   DCR82191:1 Changed OS_Mem function calls to CFE_PSP_Mem
**   Revision 1.4 2008/09/05 14:24:02EDT dahardison 
**   Updated references to local HK variables
**   Revision 1.3 2008/05/19 15:23:28EDT dahardison 
**   Version after completion of unit testing
** 
*************************************************************************/

/*************************************************************************
** Includes
*************************************************************************/
#include "mm_mem8.h"
#include "mm_app.h"
#include "mm_events.h"
#include "mm_utils.h"
#include <string.h>

/*
** The code in this file is optional.
** See mm_platform_cfg.h to set this compiler switch.
*/
#if (MM_OPT_CODE_MEM8_MEMTYPE == TRUE)

/*************************************************************************
** External Data
*************************************************************************/
extern MM_AppData_t MM_AppData; 

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Load memory from a file using only 8 bit wide writes            */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */   
boolean MM_LoadMem8FromFile(uint32                   FileHandle, 
                            char                    *FileName,
                            MM_LoadDumpFileHeader_t *FileHeader, 
                            uint32                   DestAddress)
{
       uint32      i           = 0;
       int32       BytesRead   = -1;
       uint8      *AddressPtr  = NULL;
       boolean     AtLeastOneRead = FALSE;
       boolean     Valid       = TRUE;
static uint8       DataArray[MM_MAX_LOAD_DATA_SEG];

   /* Update telemetry */
   MM_ResetHk();
   MM_AppData.LastAction = MM_LOAD_FROM_FILE;
   MM_AppData.MemType    = MM_MEM8;
   MM_AppData.Address    = DestAddress;
   strncpy(MM_AppData.FileName, FileName, OS_MAX_PATH_LEN);

   /* Initialize address pointer */
   AddressPtr =  (uint8 *)DestAddress;
      
   /*
   ** Load data from file one segment at a time
   */
   while((BytesRead = OS_read(FileHandle, DataArray, sizeof(DataArray))) > 0)
   {
      AtLeastOneRead = TRUE;
      for (i = 0; i < BytesRead; i++)
      {
         CFE_PSP_MemWrite8((uint32)AddressPtr, DataArray[i]);
         MM_AppData.BytesProcessed++;
         AddressPtr++;
      }
               
      /* Delay to prevent CPU hogging if we read a full segment buffer */
      if(BytesRead == sizeof(DataArray))
      {
         MM_SegmentBreak();
      }
      
   } /* end while */
   
   /* 
   ** Check if we broke out of the loop due to zero bytes read or an error return
   ** from OS_read and handle accordingly
   */
   if ((BytesRead < 0) || (AtLeastOneRead != TRUE))   
   {
      Valid = FALSE;
      MM_AppData.ErrCounter++;
      if (BytesRead < 0)
      {
         CFE_EVS_SendEvent(MM_OS_READ_ERR_EID, CFE_EVS_ERROR,
                           "OS_read error received: RC = 0x%08X File = '%s'", 
                                                      BytesRead, FileName);
      }
      else 
      {
         CFE_EVS_SendEvent(MM_OS_ZERO_READ_ERR_EID, CFE_EVS_ERROR,
                           "Zero bytes read by OS_read of file '\%s'", FileName);
      }
   }
      
   return(Valid);   
    
} /* end MM_LoadMem8FromFile */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Dump the requested number of bytes from memory to a file using  */
/* only 8 bit wide reads                                           */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */   
boolean MM_DumpMem8ToFile(uint32                    FileHandle, 
                          char                     *FileName, 
                          MM_LoadDumpFileHeader_t  *FileHeader)
{
       boolean       Valid          = TRUE;
       int32         OS_Status      = OS_ERROR;
       uint32        i              = 0;
       uint32        BytesRemaining = 0;
       uint8        *DataPtr        = NULL;
static uint8         DataArray[MM_MAX_DUMP_DATA_SEG];
   
   /* Update telemetry */
   MM_ResetHk();
   MM_AppData.LastAction = MM_DUMP_TO_FILE;
   strncpy(MM_AppData.FileName, FileName, OS_MAX_PATH_LEN);
   MM_AppData.MemType    = MM_MEM8;
   MM_AppData.Address    = FileHeader->SymAddress.Offset;
    
   /* Initialize byte counter */
   BytesRemaining = FileHeader->NumOfBytes;
    
   /* Initialize data pointer */
   DataPtr = (uint8 *)FileHeader->SymAddress.Offset;
    
   /* Check if data size is smaller than one dump segment */
   if(BytesRemaining <= MM_MAX_DUMP_DATA_SEG)
   {
      /* Initialize buffer and copy memory  */
      CFE_PSP_MemSet(DataArray, 0, MM_MAX_DUMP_DATA_SEG);
      
      for (i = 0; i < BytesRemaining; i++)
      {
          CFE_PSP_MemRead8((uint32)DataPtr, &DataArray[i]);
          DataPtr++;
      }
      
      /* Write buffer contents to file */
      if((OS_Status = OS_write(FileHandle, DataArray, BytesRemaining)) != BytesRemaining)
      {
         Valid = FALSE;
         MM_AppData.ErrCounter++;
         CFE_EVS_SendEvent(MM_OS_WRITE_EXP_ERR_EID, CFE_EVS_ERROR,
                           "OS_write error received: RC = 0x%08X Expected = %d File = '%s'", 
                            OS_Status, BytesRemaining, FileName);
         /* 
         ** We may have gotten a negative error code or we may have written
         ** less than we expected, fix up the housekeeping counter if
         ** the second case is true
         */
         if(OS_Status > 0)
         {
            MM_AppData.BytesProcessed += OS_Status;           
         }
         
      } /* end OS_Status != BytesRemaining if */
      else
      {
         MM_AppData.BytesProcessed += BytesRemaining;         
      }
      
   } /* end BytesRemaining <= MM_MAX_DUMP_DATA_SEG if */
   else
   {
      /* Read data blocks and write to file one segment at a time */ 
      while((BytesRemaining / MM_MAX_DUMP_DATA_SEG) >= 1)
      {
         /* Initialize buffer and copy memory  */
         CFE_PSP_MemSet(DataArray, 0, MM_MAX_DUMP_DATA_SEG);

         for (i = 0; i < MM_MAX_DUMP_DATA_SEG; i++)
         {
             CFE_PSP_MemRead8((uint32)DataPtr, &DataArray[i]);
             DataPtr++;
         }
            
         /* Write buffer contents to file */
         if((OS_Status = OS_write(FileHandle, DataArray, MM_MAX_DUMP_DATA_SEG)) != MM_MAX_DUMP_DATA_SEG)
         {
            Valid = FALSE;
            MM_AppData.ErrCounter++;
            CFE_EVS_SendEvent(MM_OS_WRITE_EXP_ERR_EID, CFE_EVS_ERROR,
                              "OS_write error received: RC = 0x%08X Expected = %d File = '%s'", 
                               OS_Status, MM_MAX_DUMP_DATA_SEG, FileName);
            /* 
            ** We may have gotten a negative error code or we may have written
            ** less than we expected, fix up the housekeeping counter if
            ** the second case is true
            */
            if(OS_Status > 0)
            {
               MM_AppData.BytesProcessed += OS_Status;           
            }
            
            /* get out of this while loop on error */
            break;
            
         } /* end OS_Status != MM_MAX_DUMP_DATA_SEG if */
         else
         {
            /*
            ** We managed to write a full dump segment, update byte counters
            ** and go back to the top of the while loop
            */   
            MM_AppData.BytesProcessed += MM_MAX_DUMP_DATA_SEG;   
            BytesRemaining = BytesRemaining - MM_MAX_DUMP_DATA_SEG;
               
            /* 
            ** Delay to prevent CPU hogging between dump segments 
            */
            MM_SegmentBreak();
         }
            
      } /* end while */
      
      /*
      ** Take care of anything over a segment multiple if no prior error
      */
      if((Valid == TRUE) && (BytesRemaining > 0))
      {
         /* Initialize buffer and copy memory  */
         CFE_PSP_MemSet(DataArray, 0, MM_MAX_DUMP_DATA_SEG);

         for (i = 0; i < BytesRemaining; i++)
         {
             CFE_PSP_MemRead8((uint32)DataPtr, &DataArray[i]);
             DataPtr++;
         }
         
         /* Write buffer contents to file */
         if((OS_Status = OS_write(FileHandle, DataArray, BytesRemaining)) != BytesRemaining)
         {
            Valid = FALSE;
            MM_AppData.ErrCounter++;
            CFE_EVS_SendEvent(MM_OS_WRITE_EXP_ERR_EID, CFE_EVS_ERROR,
                              "OS_write error received: RC = 0x%08X Expected = %d File = '%s'", 
                               OS_Status, BytesRemaining, FileName);
            /* 
            ** We may have gotten a negative error code or we may have written
            ** less than we expected, fix up the housekeeping counter if
            ** the second case is true
            */
            if(OS_Status > 0)
            {
               MM_AppData.BytesProcessed += OS_Status;           
            }
         }
         else
         {
            MM_AppData.BytesProcessed += BytesRemaining;            
         }
         
      } /* end Valid == TRUE && BytesRemaining > 0 if */
      
   } /* end BytesRemaining <= MM_MAX_DUMP_DATA_SEG else */
    
   return(Valid);

} /* end MM_DumpMem8ToFile */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Fill memory with the command specified fill pattern using only  */
/* 8 bit wide writes                                               */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */   
void MM_FillMem8(uint32           DestAddress, 
                 MM_FillMemCmd_t *CmdPtr)
{
   uint8    FillPattern8   = 0;
   uint16   i              = 0;
   uint32   WritePtr       = 0;
   uint32   BytesRemaining = 0;

   /* 
   ** For the Mem8 type we only use the least significant byte of the 
   ** 32 bit fill pattern sent in the command message
   */ 
   FillPattern8 = (uint8)CmdPtr->FillPattern;
   
   /* Set housekeeping variables */ 
   MM_ResetHk();
   MM_AppData.LastAction   = MM_FILL;
   MM_AppData.MemType      = MM_MEM8;
   MM_AppData.Address      = DestAddress;
   MM_AppData.FillPattern  = FillPattern8;
           
   BytesRemaining = CmdPtr->NumOfBytes;
   WritePtr = DestAddress;
           
   /*
   ** Fill blocks of memory equal to our segmented fill size
   ** if needed
   */
   while((BytesRemaining / MM_MAX_FILL_DATA_SEG) >= 1)
   {
      for (i = 0; i < MM_MAX_FILL_DATA_SEG; i++)
      {
         CFE_PSP_MemWrite8(WritePtr, FillPattern8);
         WritePtr++;
      }
              
      /* Increment byte counter in HK data */   
      MM_AppData.BytesProcessed += MM_MAX_FILL_DATA_SEG;   
      BytesRemaining = BytesRemaining - MM_MAX_FILL_DATA_SEG;
                                
      /* Segment to prevent CPU hogging */
      MM_SegmentBreak();
   }
        
   /*
   ** Take care of any left over bytes less than our segment size
   */
   if((BytesRemaining > 0) && (BytesRemaining <= MM_MAX_FILL_DATA_SEG))
   {
      for (i = 0; i < BytesRemaining; i++)
      {
         CFE_PSP_MemWrite8(WritePtr, FillPattern8);
         WritePtr++;
      }

   MM_AppData.BytesProcessed += BytesRemaining;
   
   } 

   return;
   
}/* End MM_FillMem8 */

#endif /* MM_OPT_CODE_MEM8_MEMTYPE */

/************************/
/*  End of File Comment */
/************************/
