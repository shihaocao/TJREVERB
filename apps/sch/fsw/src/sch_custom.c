/*
** $Id: sch_custom.c 1.2 2011/06/30 20:39:09EDT aschoeni Exp  $
**
** Purpose: Scheduler (SCH) application custom component
**
** Author:
**
** Notes:
**
** $Log: sch_custom.c  $
**
*/

/*************************************************************************
**
** Include section
**
**************************************************************************/

#include "cfe.h"
#include "sch_platform_cfg.h"

#include "sch_app.h"

#include "cfe_time_msg.h"
#include "cfe_psp.h"

/* Standard includes. */
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <inttypes.h>
#include <string.h>

/* FreeRTOS includes */
#include "FreeRTOS.h"
#include "task.h"

/* Processor constants. */
#include <dev/arm/at91sam7.h>
#include <dev/cpu.h>

/*************************************************************************
**
** Macro definitions
**
**************************************************************************/

#define SCH_TIMER_NAME   "SCH_MINOR_TIMER"

/*************************************************************************
** Local function prototypes
**************************************************************************/


/************************************************************************/
/** \brief Computes a minor slot number from a MET subseconds zero point
**  
**  \par Description
**       This function determines the current slot (minor frame) number if
**       one were to assume that slot zero started when the MET microseconds
**       are equal to zero. 
**
**  \par Assumptions, External Events, and Notes:
**       None
**       
**  \returns
**  \retstmt Returns slot index from zero to (#SCH_TOTAL_SLOTS-1) \endcode
**  \endreturns
**
*************************************************************************/
uint32 SCH_GetMETSlotNumber(void);

/************************************************************************/
/** \brief Performs Major Frame Synchronization
**  
**  \par Description
**       This function is called by cFE TIME services when a Major Frame
**       synchronization signal is received.  It then synchronizes the
**       minor frame (slot) processing of the Schedule Definition Table. 
**
**  \par Assumptions, External Events, and Notes:
**       None
**       
*************************************************************************/
void  SCH_MajorFrameCallback(void);

/************************************************************************/
/** \brief Performs Minor Frame time step
**  
**  \par Description
**       This function is called by an OSAL timer when the minor frame
**       timing reference sends a signal.  The Scheduler Application uses
**       this to drive the Application's processing of each minor frame. 
**
**  \par Assumptions, External Events, and Notes:
**       None
**       
*************************************************************************/
void  SCH_MinorFrameCallback(uint32 TimerId);

/*************************************************************************
**
** Function definitions
**
**************************************************************************/

/*************************************************************************
**
** At some point the CFE_PSP should support customizable timers, which at
** minimum can fall through to the OSAL timers. Until this is implemented
** in CFE_PSP, this redefinition supplies the same functionality to allow
** Scheduler to work out of box.
**
** This implementation is for the Gomspace Nanomind Simple Timer 1 (ST1)
** on the ATMEL AT91SAM7A1.
**  It is likely to work with other AT91SAM7 CPUs
** 
** The PSP code should be relocated to the Nanomind/FreeRTOS PSP. It will fail 
** to compile on any other system
**
**
**************************************************************************/

#ifndef CFE_PSP_MAX_TIMERS

void  CFE_PSP_TicksToUsecs(uint16 ticks, uint32 *usecs);
void  CFE_PSP_UsecsToTicks(uint32 usecs, uint16 *ticks);
void  CFE_PSP_TimerHandler(void);
/****************************************************************************************
                                     DEFINES
****************************************************************************************/
#define F_OSC           8000000
#define UNINITIALIZED   0
#define TIMER_IDLE      1
#define TIMER_START     2
#define TIMER_PERIODIC  3
#define TIMER_ID        0
#define TIMER_SCALER    8
#define TIMER_ACCURACY  128
#define TIMER_MAX_TICKS 0xFFFF
/****************************************************************************************
                                    LOCAL TYPEDEFS 
****************************************************************************************/

typedef struct 
{
   uint32              free;
   char                name[OS_MAX_API_NAME];
   uint32              start_time;
   uint32              periodic_time;
   uint32              accuracy;
   OS_TimerCallback_t  callback_ptr;
   uint32              state;
   uint32              timer_count;

} CFE_PSP_timer_record_t;

/****************************************************************************************
                                   GLOBAL DATA
****************************************************************************************/
/* 
** Setup for one timer now, can be 
** expanded later for multiple timer devices
** The Nanomind has 2 Simple Timers, with 2 channels each
*/
CFE_PSP_timer_record_t CFE_PSP_timer_table; 
uint32                 CFE_PSP_ST1Count = 0;

/****************************************************************************************
                                Functions
****************************************************************************************/

void  CFE_PSP_TimerAPIInit ( void )
{
   int32             return_code = CFE_SUCCESS;
   
   /*
   ** Mark the timer as available
   */
   CFE_PSP_timer_table.free      = TRUE;
   strcpy(CFE_PSP_timer_table.name,"");
   CFE_PSP_timer_table.start_time = 0;
   CFE_PSP_timer_table.periodic_time = 0;
   CFE_PSP_timer_table.state = TIMER_IDLE;
   CFE_PSP_timer_table.accuracy = TIMER_ACCURACY; /* Microseconds per tick */
   CFE_PSP_timer_table.timer_count = 0;   
   
}

/*
** CFE_PSP_TimerShell -
** This is the ST1 Channel 1 ISR shell for the SCH timer.
** The shell is needed by FreeRTOS.
*/
void CFE_PSP_TimerShell(void) __attribute__((naked));
void CFE_PSP_TimerShell(void)
{
   /*
   ** Interrupt entry
   */
   portSAVE_CONTEXT();

   CFE_PSP_ST1Count++;

   CFE_PSP_TimerHandler();

   /* End the interrupt in the AIC. */
   ST1_CSR = ST1_CSR_CHEND0;
   AT91C_BASE_AIC->AIC_EOICR = 1;

   portRESTORE_CONTEXT();
}

/*
** Timer Interrupt Handler.
**  This is only called by the above shell.
*/
void CFE_PSP_TimerHandler(void)
{    
   uint16 timeout;

   /*
   ** If this interrupt occurs and the table entry is free, something is wrong
   */  
   if ( CFE_PSP_timer_table.free == FALSE )
   {      
      /*
      ** If this interrupt is for the timer start period, transition to the 
      ** periodic state
      */
      if ( CFE_PSP_timer_table.state == TIMER_START )
      {
         /*
         ** Only re-arm the timer if the periodic time is greater than zero.
         */ 
         if ( CFE_PSP_timer_table.periodic_time > 0 )
         {     
            /*
            ** How many ticks?
            */
            CFE_PSP_UsecsToTicks(CFE_PSP_timer_table.periodic_time, &timeout);
  
            /*
            ** Disable the timer
            */
            ST1_DCR = ST1_DCR_ST;

            /*
            ** Program the new interval
            */
            ST1_CT0 = timeout; 

            /*
            ** Enable the timer
            */
            ST1_ECR = ST1_ECR_ST;

            /*
            ** Set the state
            */
            CFE_PSP_timer_table.state = TIMER_PERIODIC;

         }
         else
         {
            /*
            ** The periodic time is 0, so stop the timer, we are done.
            ** Disable the timer
            */
            ST1_DCR = ST1_DCR_ST;

            /*
            ** Set the state
            */
            CFE_PSP_timer_table.state = TIMER_IDLE;

         }

      } 
      /*
      ** Call the user callback
      */
      CFE_PSP_timer_table.timer_count++;
      (CFE_PSP_timer_table.callback_ptr)(0);
   }

   /*
   ** Return to interrupt shell
   */

}
 
/******************************************************************************
 **  Function:  CFE_PSP_UsecToTicks
 **
 **  Purpose:  Convert Microseconds to a number of ticks.
 **
 */
void  CFE_PSP_UsecsToTicks(uint32 usecs, uint16 *ticks)
{
   uint32  usecs_per_tick = TIMER_ACCURACY;
   
   if ( usecs < usecs_per_tick )
   {
      *ticks = 1;
   }
   else
   {
      *ticks = usecs / usecs_per_tick;
      /* Need to round up?? */ 
   }
	
}

/******************************************************************************
 **  Function:  CFE_PSP_TicksToUsec
 **
 **  Purpose:  Convert a number of Ticks to microseconds
 **
 */
void  CFE_PSP_TicksToUsecs(uint16 ticks, uint32 *usecs)
{
   uint32 usecs_per_tick = TIMER_ACCURACY;
   
   *usecs = ticks * usecs_per_tick;
}

/****************************************************************************************
                                   Timer API
****************************************************************************************/

/******************************************************************************
**  Function:  CFE_PSP_TimerCreate
**
**  Purpose:  Create a new PSP Timer
**
**  Arguments:
**
**  Return:
*/
int32 CFE_PSP_TimerCreate(uint32 *timer_id,       const char         *timer_name, 
                          uint32 *clock_accuracy, OS_TimerCallback_t  callback_ptr)
{

   if ( timer_id == NULL || timer_name == NULL || clock_accuracy == NULL )
   {
        return OS_INVALID_POINTER;
   }

   /* 
   ** we don't want to allow names too long
   ** if truncated, two names might be the same 
   */
   if (strlen(timer_name) > OS_MAX_API_NAME)
   {
      return OS_ERR_NAME_TOO_LONG;
   }

   /* 
   ** Check Parameters 
   */
   if (CFE_PSP_timer_table.free == TRUE)
   {
      /*
      ** Verify callback parameter
      */
      if (callback_ptr == NULL ) 
      {
         return OS_TIMER_ERR_INVALID_ARGS;
      }    

      /* 
      ** Allocate the timer so that
      ** no other task can try to use it 
      */
      CFE_PSP_timer_table.free = FALSE;
      strncpy(CFE_PSP_timer_table.name, timer_name, OS_MAX_API_NAME);
      CFE_PSP_timer_table.start_time = 0;
      CFE_PSP_timer_table.periodic_time = 0;
      CFE_PSP_timer_table.callback_ptr = callback_ptr;
      CFE_PSP_timer_table.state = TIMER_IDLE;
      CFE_PSP_timer_table.accuracy = TIMER_ACCURACY;

      /*
      ** Return the clock accuracy to the user
      */
      *clock_accuracy = CFE_PSP_timer_table.accuracy;

      /*
      ** Return timer ID. There really is only one timer ID
      ** for this implementation. 
      */
      *timer_id = TIMER_ID;

   }
   else
   { 
        return OS_ERR_NO_FREE_IDS;
   }

   return OS_SUCCESS;
}

/******************************************************************************
**  Function:  CFE_PSP_TimerSet
**
**  Purpose:  
**
**  Arguments:
**    (none)
**
**  Return:
**    (none)
*/
int32 CFE_PSP_TimerSet(uint32 timer_id, uint32 start_time, uint32 interval_time)
{
   uint16 timeout;
   int    prescalar = TIMER_SCALER; /* 8 gives 128 Microseconds per tick */
   int    syscal = 0;

   /* 
   ** Check to see if the timer_id given is valid 
   */
   if (timer_id != TIMER_ID || CFE_PSP_timer_table.free == TRUE)
   {
      return OS_ERR_INVALID_ID;
   }

   /*
   ** Round up the accuracy of the start time and interval times.
   ** Still want to preserve zero, since that has a special meaning. 
   */
   if (( start_time > 0 ) && (start_time < CFE_PSP_timer_table.accuracy))
   {
      start_time = CFE_PSP_timer_table.accuracy;
   }
 
   if ((interval_time > 0) && (interval_time < CFE_PSP_timer_table.accuracy))
   {
      interval_time = CFE_PSP_timer_table.accuracy;
   }

   /*
   ** Save the start and interval times 
   */
   CFE_PSP_timer_table.start_time = start_time;
   CFE_PSP_timer_table.periodic_time = interval_time;

   /*
   ** The defined behavior is to not arm the timer if the start time is zero.
   ** If the interval time is zero, then the timer will not be re-armed.
   */
   if ( start_time > 0 )
   {
      CFE_PSP_timer_table.state = TIMER_START;

      /*
      ** Convert from Microseconds to the timeout
      */
      CFE_PSP_UsecsToTicks(start_time, &timeout);

      /*
      ** Program the timer
      */
      /* Reset simple timer */
      ST1_CR = ST1_CR_SWRST;

      /* Enable the simple timer clock */
      ST1_ECR = ST1_ECR_ST;

      ST1_PR0 = (syscal << ST1_PR0_SYSCAL_BIT) | ST1_PR0_AUTOREL | ST1_PR0_SELECTCLK | 
                (prescalar << ST1_PR0_PRESCALAR_BIT);
      ST1_CT0 = timeout; 

      /* Reset and enable the channel 0 */
      ST1_CSR = ST1_CSR_CHEND0 | ST1_CSR_CHDIS0 | ST1_CSR_CHLD0;
      ST1_CR = ST1_CR_CHEN0;
      ST1_IER = ST1_IER_CHEND0;

      /* Setup the AIC for PIT interrupts. */
      AT91F_AIC_ConfigureIt(AT91C_BASE_AIC, AT91C_ID_ST1, AT91C_AIC_PRIOR_HIGHEST,
                          AT91C_AIC_SRCTYPE_INT_HIGH_LEVEL, 
                          ( void (*)(void) ) CFE_PSP_TimerShell );

      AT91F_AIC_EnableIt(AT91C_BASE_AIC, AT91C_ID_ST1);

   }
   return OS_SUCCESS;
}

#endif

/*******************************************************************
**
** SCH_CustomEarlyInit
**
** NOTE: For complete prolog information, see 'sch_custom.h'
**
** This function MUST update SCH_AppData.ClockAccuracy to the
** resolution of the minor frame timer.
********************************************************************/

int32 SCH_CustomEarlyInit(void)
{
    int32  Status = CFE_SUCCESS;
    
    CFE_PSP_TimerAPIInit();

    Status = CFE_PSP_TimerCreate(&SCH_AppData.TimerId,
                                 SCH_TIMER_NAME,
                                 &SCH_AppData.ClockAccuracy,
                                 SCH_MinorFrameCallback);
    
    return Status;

} /* End of CustomEarlyInit() */


/*******************************************************************
**
** SCH_CustomLateInit
**
** NOTE: For complete prolog information, see 'sch_custom.h'
**
** This function MUST perform any startup synchronization required,
** and MUST finish setting up the major and minor frame timers. 
********************************************************************/

int32 SCH_CustomLateInit(void)
{
    int32  Status    = CFE_SUCCESS;
    
    CFE_ES_WaitForStartupSync(SCH_STARTUP_SYNC_TIMEOUT);

    /*
    ** Connect to cFE TIME's time reference marker (typically 1 Hz)
    ** to use it as the Major Frame synchronization source
    */
    Status = CFE_TIME_RegisterSynchCallback((CFE_TIME_SynchCallbackPtr_t)&SCH_MajorFrameCallback);
    if (Status == CFE_SUCCESS)
    {
        /*
        ** Start the Minor Frame Timer with an extended delay to allow a Major Frame Sync
        ** to start processing.  If the Major Frame Sync fails to arrive, then we will
        ** start when this timer expires and synch ourselves to the MET clock.
        */
        Status = CFE_PSP_TimerSet(SCH_AppData.TimerId, SCH_STARTUP_PERIOD, 0);
    }

    return Status;

} /* End of SH_CustomLateInit() */


/*******************************************************************
**
** SCH_CustomGetCurrentSlotNumber
**
** NOTE: For complete prolog information, see 'sch_custom.h'
********************************************************************/

uint32 SCH_CustomGetCurrentSlotNumber(void)
{
    uint32  CurrentSlot;
    
    if (SCH_AppData.SyncToMET != SCH_NOT_SYNCHRONIZED)
    {
        CurrentSlot = SCH_GetMETSlotNumber();
        
        /* 
        ** If we are only concerned with synchronizing the minor frames to an MET,
        ** then we need to adjust the current slot by whatever MET time is prevalent
        ** when the Major Frame Signal is received.
        ** If we are synchronizing the Major Frame, then, by definition, LastSyncMETSlot
        ** would be a zero and the current slot would be appropriate.
        */
        if (CurrentSlot < SCH_AppData.LastSyncMETSlot)
        {
            CurrentSlot = CurrentSlot + SCH_TOTAL_SLOTS - SCH_AppData.LastSyncMETSlot;
        }
        else
        {
            CurrentSlot = CurrentSlot - SCH_AppData.LastSyncMETSlot;
        }
    }
    else
    {
        CurrentSlot = SCH_AppData.MinorFramesSinceTone;
    }
    
    return CurrentSlot;
} /* End of SH_CustomGetCurrentSlotNumber() */

/*******************************************************************
**
** SCH_CustomCleanup
**
** NOTE: For complete prolog information, see 'sch_custom.h'
********************************************************************/

void SCH_CustomCleanup(void)
{
    /* unregister the TIME callback for the major frame */
    CFE_TIME_UnregisterSynchCallback((CFE_TIME_SynchCallbackPtr_t)&SCH_MajorFrameCallback);

} /* End of SH_CustomCleanup() */


/*******************************************************************
**
** SCH_GetMETSlotNumber
**
** NOTE: For complete prolog information, see above
********************************************************************/

uint32 SCH_GetMETSlotNumber(void)
{
    uint32 SubSeconds = 0;
    uint32 MicroSeconds;
    uint32 Remainder;
    uint32 METSlot;
    
    /*
    ** Use MET rather than current time to avoid time changes
    */
    SubSeconds = CFE_TIME_GetMETsubsecs();

    /*
    ** Convert sub-seconds to micro-seconds
    */
    MicroSeconds = CFE_TIME_Sub2MicroSecs(SubSeconds);

    /*
    ** Calculate schedule table slot number
    */
    METSlot = (MicroSeconds / SCH_NORMAL_SLOT_PERIOD);

    /*
    ** Check to see if close enough to round up to next slot
    */
    Remainder = MicroSeconds - (METSlot * SCH_NORMAL_SLOT_PERIOD);
    
    /*
    ** Add one more microsecond and see if it is sufficient to add another slot
    */
    Remainder += 1;
    METSlot += (Remainder / SCH_NORMAL_SLOT_PERIOD);
    
    /*
    ** Check to see if the Current Slot number needs to roll over
    */
    if (METSlot == SCH_TOTAL_SLOTS)
    {
        METSlot = 0;
    }
    
    return METSlot;
    
}

/*******************************************************************
**
** SCH_MajorFrameCallback
**
** NOTE: For complete prolog information, see above
********************************************************************/

void SCH_MajorFrameCallback(void)
{
    /*
    ** Synchronize slot zero to the external tone signal
    */
    uint16 StateFlags;

    /*
    ** If cFE TIME is in FLYWHEEL mode, then ignore all synchronization signals
    */
    StateFlags = CFE_TIME_GetClockInfo();
    
    if ((StateFlags & CFE_TIME_FLAG_FLYING) == 0)
    {
        /*
        ** Determine whether the major frame is noisy or not
        **
        ** Conditions below are as follows:
        **    If we are NOT synchronized to the MET (i.e. - the Minor Frame timer
        **    has an acceptable resolution), then the Major Frame signal should
        **    only occur in the last slot of the schedule table.
        **
        **    If we ARE synchronized to the MET (i.e. - the Minor Frame timer is
        **    not as good as we would like), then the Major Frame signal should
        **    occur within a window of slots at the end of the table.
        */
        if (((SCH_AppData.SyncToMET == SCH_NOT_SYNCHRONIZED) && 
             (SCH_AppData.MinorFramesSinceTone != SCH_TIME_SYNC_SLOT)) ||
            ((SCH_AppData.SyncToMET == SCH_MINOR_SYNCHRONIZED) && 
             (SCH_AppData.NextSlotNumber != 0) && 
             (SCH_AppData.NextSlotNumber < 
              (SCH_TOTAL_SLOTS - SCH_AppData.WorstCaseSlotsPerMinorFrame - 1))))
        {
            /*
            ** Count the number of consecutive noisy major frames and the Total number
            ** of noisy major frames.  Also, indicate in telemetry that this particular
            ** Major Frame signal is considered noisy.
            */
            SCH_AppData.UnexpectedMajorFrame = TRUE;
            SCH_AppData.UnexpectedMajorFrameCount++;

            /*
            ** If the Major Frame is not being ignored yet, then increment the consecutive noisy
            ** Major Frame counter.
            */
            if (!SCH_AppData.IgnoreMajorFrame)
            {
                SCH_AppData.ConsecutiveNoisyFrameCounter++;
                
                /*
                ** If the major frame is too "noisy", then send event message and ignore future signals
                */
                if (SCH_AppData.ConsecutiveNoisyFrameCounter >= SCH_MAX_NOISY_MAJORF)
                {
                    SCH_AppData.IgnoreMajorFrame = TRUE;
                }
            }
        }
        else /* Major Frame occurred when expected */
        {
            SCH_AppData.UnexpectedMajorFrame = FALSE;
            SCH_AppData.ConsecutiveNoisyFrameCounter = 0;
        }
        
        /*
        ** Ignore this callback if SCH has detected a noisy Major Frame Synch signal
        */
        if (SCH_AppData.IgnoreMajorFrame == FALSE)
        {
            /*
            ** Stop Minor Frame Timer (which should be waiting for an unusually long
            ** time to allow the Major Frame source to resynchronize timing) and start
            ** it again with nominal Minor Frame timing
            */
            CFE_PSP_TimerSet(SCH_AppData.TimerId, SCH_NORMAL_SLOT_PERIOD, SCH_NORMAL_SLOT_PERIOD);
    
            /*
            ** Increment Major Frame process counter
            */
            SCH_AppData.ValidMajorFrameCount++;
    
            /*
            ** Set current slot = zero to synchronize activities
            */
            SCH_AppData.MinorFramesSinceTone = 0;
            
            /*
            ** Major Frame Source is now from CFE TIME
            */
            SCH_AppData.MajorFrameSource = SCH_MAJOR_FS_CFE_TIME;
            
            /* Clear any Major Frame In Sync with MET flags */
            /* But keep the Minor Frame In Sync with MET flag if it is set */
            SCH_AppData.SyncToMET &= SCH_MINOR_SYNCHRONIZED;
            
            /*
            ** Give "wakeup SCH" semaphore
            */
            OS_BinSemGiveFromISR(SCH_AppData.TimeSemaphore);
        }
    }

    /*
    ** We should assume that the next Major Frame will be in the same MET slot as this 
    */
    SCH_AppData.LastSyncMETSlot = SCH_GetMETSlotNumber();

    return;

} /* End of SCH_MajorFrameCallback() */


/*******************************************************************
**
** SCH_MinorFrameCallback
**
** NOTE: For complete prolog information, see above
********************************************************************/

void SCH_MinorFrameCallback(uint32 TimerId)
{
    uint32  CurrentSlot;

    /*
    ** If this is the very first timer interrupt, then the initial 
    ** Major Frame Synchronization timed out.  This can occur when
    ** either the signal is not arriving or the clock has gone into
    ** FLYWHEEL mode.  We should synchronize to the MET time instead.
    */
    if (SCH_AppData.MajorFrameSource == SCH_MAJOR_FS_NONE)
    {
        SCH_AppData.MajorFrameSource = SCH_MAJOR_FS_MINOR_FRAME_TIMER;
        
        /* Synchronize timing to MET */
        SCH_AppData.SyncToMET |= SCH_PENDING_MAJOR_SYNCH;
        SCH_AppData.SyncAttemptsLeft = SCH_MAX_SYNC_ATTEMPTS;
        SCH_AppData.LastSyncMETSlot = 0;
    }
    
    /* If attempting to synchronize the Major Frame with MET, then wait for zero subsecs before starting */
    if (((SCH_AppData.SyncToMET & SCH_PENDING_MAJOR_SYNCH) != 0) &&
        (SCH_AppData.MajorFrameSource == SCH_MAJOR_FS_MINOR_FRAME_TIMER))
    {
        /* Whether we have found the Major Frame Start or not, wait another slot */
        CFE_PSP_TimerSet(SCH_AppData.TimerId, SCH_NORMAL_SLOT_PERIOD, SCH_NORMAL_SLOT_PERIOD);

        /* Determine if this was the last attempt */
        SCH_AppData.SyncAttemptsLeft--;

        CurrentSlot = SCH_GetMETSlotNumber();
        if ((CurrentSlot != 0) && (SCH_AppData.SyncAttemptsLeft > 0))
        {
            return;
        }
        else  /* Synchronization achieved (or at least, aborted) */
        {
            /* Clear the pending synchronization flag and set the "Major In Sync" flag */
            SCH_AppData.SyncToMET &= ~SCH_PENDING_MAJOR_SYNCH;
            SCH_AppData.SyncToMET |= SCH_MAJOR_SYNCHRONIZED;
            
            /* CurrentSlot should be equal to zero.  If not, this is the best estimate we can use */
            SCH_AppData.MinorFramesSinceTone = CurrentSlot;
            SCH_AppData.LastSyncMETSlot = 0;
        }
    }
    else
    {
        /*
        ** If we are already synchronized with MET or don't care to be, increment current slot
        */
        SCH_AppData.MinorFramesSinceTone++;
    }

    if (SCH_AppData.MinorFramesSinceTone >= SCH_TOTAL_SLOTS)
    {
        /*
        ** If we just rolled over from the last slot to slot zero,
        ** It means that the Major Frame Callback did not cancel the
        ** "long slot" timer that was started in the last slot
        **
        ** It also means that we may now need a "short slot"
        ** timer to make up for the previous long one
        */
        CFE_PSP_TimerSet(SCH_AppData.TimerId, SCH_SHORT_SLOT_PERIOD, SCH_NORMAL_SLOT_PERIOD);
        
        SCH_AppData.MinorFramesSinceTone = 0;
        
        SCH_AppData.MissedMajorFrameCount++;
    }

    /*
    ** Determine the timer delay value for the next slot
    */
    if (SCH_AppData.MinorFramesSinceTone == SCH_TIME_SYNC_SLOT)
    {
        /*
        ** Start "long slot" timer (should be stopped by Major Frame Callback)
        */
        CFE_PSP_TimerSet(SCH_AppData.TimerId, SCH_SYNC_SLOT_PERIOD, 0);
    }
    
    /*
    ** Note that if this is neither the first "short" minor frame nor the
    ** last "long" minor frame, the timer is not modified.  This should
    ** provide more stable timing than introducing the dither associated
    ** with software response times to timer interrupts.
    */

    /*
    ** Give "wakeup SCH" semaphore
    */
    OS_BinSemGiveFromISR(SCH_AppData.TimeSemaphore);

    return;

} /* End of SCH_MinorFrameCallback() */

/************************/
/*  End of File Comment */
/************************/


