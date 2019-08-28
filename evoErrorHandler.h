/************************************************************
 * Project:	EVOPRO-FWDB
 * CPU:		STM32F407 VGT & STM32F303 RCT
 * Compiler:    - CooCox- COIDE Version: 1.7.6
 *              - IAR Embedded Workbench for ARM Version: 7.60
 * Date:	2.09.2014
************************************************************/
/********************************************************//**
 * @file   	evoErrorHandler.h
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

#ifndef __EVO_ERRORHANDLER_H
#define __EVO_ERRORHANDLER_H

#include <stdint.h>

typedef enum {
	DISABLE = 0,
	ENABLE = !DISABLE
}FunctionalState;

/** @addtogroup EVOPRO-FWDB
 * @{
  */

/** @addtogroup FWDB_ErrorHandler
  * @{
  */

/***********************************************
 *Includes
 ***********************************************/
#ifdef STM32L1XX_MDP
	#include "stm32l1xx.h"
	#include "stm32l1xx_conf.h"
#endif	/*STM32L1XX_MDP*/


#ifdef STM32F30X
	#include "stm32f30x.h"
	#include "stm32f30x_conf.h"
#endif	/*STM32F30x*/

#ifdef STM32F4XX
	#include "stm32f4xx.h"
	#include "stm32f4xx_conf.h"
#endif	/*STM32F4*/
/***********************************************
 *Defines
 ***********************************************/
/** @defgroup evoErrorHandler_HeaderDefinitions
  * @{
  */
/** @defgroup evoEH_ErrorCode
  * @{
  */

/** @defgroup evoEH_ErrorType
  * @{
  */

/** @brief
 * ErrorType: Type of the occurred Error
 */
typedef enum
{
  type_NoError 	        = (uint8_t)0,	/*!< 0*/
  type_Unknown	        = (uint8_t)1,	/*!< 1*/

  type_Init	        	= (uint8_t)2,	/*!< 2*/
  type_Timeout          = (uint8_t)3,	/*!< 3*/
  type_OutOfBounds      = (uint8_t)4,	/*!< 4*/
}ErrorType_t;
/**
  * @}
  */

/** @defgroup evoEH_ErrorSource
  * @{
  */
/** @brief
 * ErrorSource: Source of the occurred Error
 *  @note
 *  All possible Error sources must be added here in the following configuration:
 *  -> ModuleSource[1^10] + PrivateSource	(->So there are 9 internal Error sources possible)
 */
typedef enum
{
  src_Unknown 	        = (uint16_t)0,		/*!< 0*/

  /*--Project-Specific-ErrorSources-addresses-1--99--------------------------*/
  /**************************************************************
  * --Handle project Error sources in a separate file:
  * ErrorHandler_"projectname".h
  **************************************************************/

  /*--EVOInterface-specific--------------------------------------------------*/
  src_evoTerminal			= (uint16_t)100,	/*!< 100*/
  src_evoCAN				= (uint16_t)110,	/*!< 110*/
  src_evoLED				= (uint16_t)120,	/*!< 120*/
  src_evoSoftwareSPI		= (uint16_t)130,	/*!< 130*/
  src_evoSoftwareUSART		= (uint16_t)140,	/*!< 140*/
  src_NEVOI2C				= (uint16_t)150,    /*!< 150*/
  src_evoInSystemBootloader	= (uint16_t)160,    /*!< 160*/
  src_evoFLEXCAN			= (uint16_t)170,    /*!< 170*/
  src_evoXSolo				= (uint16_t)180,    /*!< 180*/
  src_evoMIN				= (uint16_t)190,	/*!< 190*/

  /*--EVOLibary-specific--------------------------------------------------*/
  src_ADS124x		= (uint16_t)200,	/*!< 200*/
  src_ADS125x		= (uint16_t)210,	/*!< 210*/
  src_M24C0x		= (uint16_t)220,	/*!< 220*/
  src_23x1024		= (uint16_t)230,	/*!< 230*/
  src_ISL22323		= (uint16_t)240,	/*!< 240*/
  src_25AA512		= (uint16_t)250,	/*!< 250*/
  src_MCP3202x		= (uint16_t)260,	/*!< 260*/
  src_ADC121S101x	= (uint16_t)270,	/*!< 270*/
  src_MCP492x		= (uint16_t)280,	/*!< 280*/
  src_DACx311		= (uint16_t)290,	/*!< 290*/
  src_MCP23x08		= (uint16_t)300,	/*!< 300*/
  src_PCF8574x		= (uint16_t)310,	/*!< 310*/
  src_DACx760		= (uint16_t)320,	/*!< 320*/
  src_DS442x		= (uint16_t)330,	/*!< 330*/
  src_ADS866x		= (uint16_t)340,	/*!< 340*/
  src_N01S830		= (uint16_t)350,	/*!< 350*/
  src_TLC594x 		= (uint16_t)360,	/*!< 360*/
  src_SN74LV595A	= (uint16_t)370,	/*!< 370*/
  src_ADG70x		= (uint16_t)380,	/*!< 380*/
  src_MAX482x		= (uint16_t)390,	/*!< 390*/
  src_DOGM128_6		= (uint16_t)400,	/*!< 400*/
  src_NPIC6C595		= (uint16_t)410,	/*!< 410*/

}ErrorSource_t;
/**
  * @}
  */

/** @defgroup evoEH_ErrorPriority
  * @{
  */
/** @brief
 * ErrorPriority: Priority of the occurred Error
 */
typedef enum
{
  prio_None 	    = (uint8_t)0,	/*!< 0*/
  prio_Low	        = (uint8_t)1,	/*!< 1*/
  prio_High	        = (uint8_t)2	/*!< 2*/
}ErrorPriority_t;

/**
  * @}
  */
/**
  * @}
  */

/** @defgroup evoEH_ErrorTransmission_Types
  * @{
  */
/** @brief
 * ResultState: is the combined ErrorCode
 */
typedef struct
{
  ErrorType_t	ResultType;
  ErrorSource_t	ResultSource;
  ErrorPriority_t ResultPriority;
}ResultState_t;

/** @brief
 * uint8_ResultContainer: is a 8Bit-DataValue inclusive the combined ErrorCode
 */
typedef struct
{
  uint8_t Value;
  ResultState_t State;
}uint8_ResultContainer_t;

/** @brief
 * uint16_ResultContainer: is a 16Bit-DataValue inclusive the combined ErrorCode
 */
typedef struct
{
  uint16_t Value;
  ResultState_t State;
}uint16_ResultContainer_t;

/** @brief
 * uint32_ResultContainer: is a 32Bit-DataValue inclusive the combined ErrorCode
 */
typedef struct
{
  uint32_t Value;
  ResultState_t State;
}uint32_ResultContainer_t;

/** @brief
 * int32_ResultContainer: is a int32-DataValue inclusive the combined ErrorCode
 */
typedef struct
{
  int32_t Value;
  ResultState_t State;
}int32_ResultContainer_t;

/** @brief
 * flaot_ResultContainer: is a floating-DataValue inclusive the combined ErrorCode
 */
typedef struct
{
  float Value_f;
  ResultState_t State;
}float_ResultContainer_t;

/**
  * @}
  */

/**
  * @}
  */
/***********************************************
 *Functions
 ***********************************************/
/** @addtogroup evoErrorHandler_HeaderFunctionPrototypes
  * @{
  */

void Init_ErrorHandler(void);
void Set_ErrorHandler_DebugOutput(FunctionalState newState);
ResultState_t CreateResultState(ErrorType_t type, ErrorSource_t src,ErrorPriority_t prio);

/**
  * @}
  */

/**
  * @}
  */
/**
  * @}
  */
#endif	/*__ERROR_HANDLER_H*/
