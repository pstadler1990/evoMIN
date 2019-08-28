/************************************************************
 * Project:	TemplateFW
 * CPU:		STM32F407 VGT
 * Compiler:    - CooCox- COIDE Version: 1.7.6
 *              - IAR Embedded Workbench for ARM Version: 7.60
 * Date:	06.04.2016
************************************************************/
/********************************************************//**
 * @file   		ErrorHandler_EvoMIN.h
 * @version  	V1.0
 * @date 		06.04.2016
 * @author 		maximilian.karl
 * @copyright	evopro systems engineering AG
 * @brief 		Process and Exception Watchdog
 * @note		Prototypefunctions and defines for
 *	   			Error handling and logging of the device
 * @note		Manufacturer: evopro; Type: ErrorDetection
 * @attention	Every Module that uses the Error handling must be added @ enum ErrorSource!!!
 ************************************************************/

#ifndef __ERRORHANDLER_EVOMIN_H
#define __ERRORHANDLER_EVOMIN_H

/** @addtogroup TemplateFW
 * @{
  */

/** @addtogroup TemplateFW_ErrorHandler
  * @{
  */

/***********************************************
 *Includes
 ***********************************************/

/***********************************************
 *Defines
 ***********************************************/
/** @defgroup ErrorHandler_TemplateFW_HeaderDefinitions
  * @{
  */

/** @addgroup evoEH_ErrorSource
  * @{
  */
/** @brief
 * ErrorSource: Source of the occurred Error
 *  @note
 *  All possible project Error sources must be added here in the following configuration:
 *  -> reserved addresses for the Project Error-Field 1-99!!!!
 */
typedef enum
{
	/*--EVOMIN-Specific--------------------------------------------------*/
	src_evoMIN_SOF 				= (uint16_t)1,		/*!< 1*/
	src_evoMIN_ACK 				= (uint16_t)2,		/*!< 2*/
	src_evoMIN_LEN 				= (uint16_t)3,		/*!< 3*/
	src_evoMIN_PAYLD 			= (uint16_t)4,		/*!< 4*/
	src_evoMIN_CRC 				= (uint16_t)5,		/*!< 5*/
	src_evoMIN_SENDFRAME		= (uint16_t)6,		/*!< 6*/
	src_evoMIN_SENDFRAME_BUF_ALLOC =  (uint16_t)7,		/*!< 7*/
	src_evoMIN_SENDFRAME_RETRY		= (uint16_t)8,		/*!< 8*/
	src_evoMIN_QUEUEFRAME		= (uint16_t)9,			/*!< 9*/
}EvoMIN_ErrorSource_t;

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */
/**
  * @}
  */
#endif	/*__ERRORHANDLER_EVOMIN_H*/
