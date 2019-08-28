/************************************************************
 * Project:	EVOPRO-FWDB
 * CPU:		STM32F407 VGT & STM32F303 RCT
 * Compiler:    - CooCox- COIDE Version: 1.7.6
 *              - IAR Embedded Workbench for ARM Version: 7.60
 * Date:	12.09.2014
************************************************************/
/********************************************************//**
 * @file   	evoErrorHandler.c
 * @version  	V1.0
 * @date 	12.09.2014
 * @author 	maximilian.karl
 * @copyright	evopro systems engineering AG
 * @brief 	Process and Exception Watchdog
 * @note	Prototypefunctions and defines for
 *	        Error handling and logging of the device
 * @note	Manufacturer: evopro; Type: ErrorDetection
 * @attention	Every Module that uses the Error handling must be added @ enum ErrorSource!!!
 ************************************************************/

/** @addtogroup EVOPRO-FWDB
 * @{
  */

/** @addtogroup FWDB_ErrorHandler
  * @{
  */

/***********************************************
 *Includes
 ***********************************************/
#include <stdio.h>
#include "evoErrorHandler.h"
/***********************************************
 *Globals
 ***********************************************/
/** @defgroup main_SourcePrivateVariables
  * @{
  */
/** @brief
 * ErrorOutput: Occurred Error will be send over USART or not
 */
FunctionalState EH_ErrorNotification;
/**
  * @}
  */
/***********************************************
 *Functions
 ***********************************************/
/** @defgroup evoErrorHandler_SourcePrivateFunctions
  * @{
  */
/***********************************//**
  * @brief  Local Initialization Function for Error handling
  * @note   not used
  * @param  None
  * @retval None
***************************************/
void Init_ErrorHandler(void)
{
  /*global Error Output Setting:
   * normally the Output of Errors is Enabled
   * and can be disabled with the
   * "Set_ErrorHandler_DebugOutput"-function
   */
  EH_ErrorNotification = ENABLE;
}

void Set_ErrorHandler_DebugOutput(FunctionalState newState)
{
  EH_ErrorNotification = newState;
}

/***********************************//**
  * @brief  Local State Creation Function
  * @note   this is a global function that can be used to create and clear errors
  * 	    and notify them via serial interface for Debug
  * @note   Usage: CreateResultState(type, (Module ErrorSorce + internal/private ErrorSrc), prio);
  * @param  type: exception type that triggered the generation
  * @param  src: Caller = Source
  * @param  prio: priority of the occurred error
  * @retval ResultState_t: Created State for Debug and Error Routines
***************************************/
ResultState_t CreateResultState(ErrorType_t type, ErrorSource_t src,ErrorPriority_t prio)
{
  char ErrorBuffer[50];

  ResultState_t errorState;
  errorState.ResultType = type;
  errorState.ResultSource = src;
  errorState.ResultPriority = prio;

  /*Debug Errors over Serial Interface*/

  if((type != type_NoError) && (EH_ErrorNotification))
  {
	printf(ErrorBuffer,"ERROR: Type: %i, Src: %i, Prio: %i\r\n",type,src,prio);
  }

  return errorState;
}

/**
  * @}
  */

/**
  * @}
  */
/**
  * @}
  */
