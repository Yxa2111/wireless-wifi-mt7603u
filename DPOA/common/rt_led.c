/****************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 * (c) Copyright 2002-2010, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ****************************************************************************

    Module Name:
	rt_led.c
 
    Abstract:
 
    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */

#include	"rt_config.h"

#ifdef LED_CONTROL_SUPPORT


#ifdef CONFIG_ANDES_SUPPORT
INT LED_Array[16][12]={
	{-1, -1,   -1,   -1,   -1, -1, -1, -1, -1, -1, -1, -1},
	{ 0,   2,     1,    0,   -1,  -1,  0, -1,   5, -1, -1, 17},
	{-1, -1,   -1,   -1,   -1, -1, -1, -1, -1, -1, -1, -1},
	{-1, -1,   -1,   -1,   -1, -1, -1, -1, -1, -1, -1, -1},
	{-1, -1,   -1,   -1,   -1, -1, -1, -1, -1, -1, -1, -1},
	{-1, -1,   -1,   -1,   -1, -1, -1, -1, -1, -1, -1, -1},
	{-1, -1,   -1,   -1,   -1, -1, -1, -1, -1, -1, -1, -1},
	{-1, -1,   -1,   -1,   -1, -1, -1, -1, -1, -1, -1, -1},
	{-1, -1,   -1,   -1,   -1, -1, -1, -1, -1, -1, -1, -1},
	{-1, -1,   -1,   -1,   -1, -1, -1, -1, -1, -1, -1, -1},
	{  3,  2,   -1,   -1,   -1, -1, 16,   1,  5,  -1, -1, 17},
	{-1, -1,   -1,   -1,   -1, -1, -1, -1, -1, -1, -1, -1},
	{-1, -1,   -1,   -1,   -1, -1, -1, -1, -1, -1, -1, -1},
	{-1, -1,   -1,   -1,   -1, -1, -1, -1, -1, -1, -1, -1},
	{ 1,   2,     1,   -1,   -1, -1,  3, -1,   6, -1, -1,   0},
	{ 1,   2,     1,   -1,   -1, -1, -1,  1,   4, -1, -1, 18}
};
#endif /* CONFIG_ANDES_SUPPORT */
	



/*
	========================================================================
	
	Routine Description:
		Set LED Status

	Arguments:
		pAd						Pointer to our adapter
		Status					LED Status

	Return Value:
		None

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL
	
	Note:
	
	========================================================================
*/
VOID RTMPSetLEDStatus(RTMP_ADAPTER *pAd, UCHAR Status)
{
	/*ULONG			data; */
	UCHAR			LinkStatus = 0;
	UCHAR			LedMode;
	UCHAR			MCUCmd = 0;
	BOOLEAN 		bIgnored = FALSE;
#ifdef MT7603
	INT 				LED_CMD = -1;
#endif /*MT7603*/

#ifdef CONFIG_ATE
	/*
		In ATE mode of RT2860 AP/STA, we have erased 8051 firmware.
		So LED mode is not supported when ATE is running.
	*/
	if (!IS_RT3572(pAd))
	{
		if (ATE_ON(pAd))
			return;
	}
#endif /* CONFIG_ATE */

#ifdef RTMP_MAC_USB
#ifdef STATS_COUNT_SUPPORT
	if(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF))
		return;
#endif /* STATS_COUNT_SUPPORT */
#endif /* RTMP_MAC_USB */


	LedMode = 1;//LED_MODE(pAd);
#ifdef MT7603
	if (IS_MT7603(pAd))
	{
		if (LedMode < 0 || Status <0 || LedMode > 15 || Status > 11)
			return;
		else
			LED_CMD = LED_Array[LedMode][Status];
	}
#endif /*MT7603*/
	switch (Status)
	{
		case LED_LINK_DOWN:
			LinkStatus = LINK_STATUS_LINK_DOWN;
			pAd->LedCntl.LedIndicatorStrength = 0;
			MCUCmd = MCU_SET_LED_MODE;
			break;
		case LED_LINK_UP:
			if (pAd->CommonCfg.Channel > 14)
				LinkStatus = LINK_STATUS_ABAND_LINK_UP;
			else
				LinkStatus = LINK_STATUS_GBAND_LINK_UP;

			MCUCmd = MCU_SET_LED_MODE;
			break;
		case LED_RADIO_ON:
			LinkStatus = LINK_STATUS_RADIO_ON;
			MCUCmd = MCU_SET_LED_MODE;
			break;
		case LED_HALT: 
			LedMode = 0; /* Driver sets MAC register and MAC controls LED */
		case LED_RADIO_OFF:
			LinkStatus = LINK_STATUS_RADIO_OFF;
			MCUCmd = MCU_SET_LED_MODE;
			break;
		case LED_WPS:
			LinkStatus = LINK_STATUS_WPS;
			MCUCmd = MCU_SET_LED_MODE;
			break;
		case LED_ON_SITE_SURVEY:
			LinkStatus = LINK_STATUS_ON_SITE_SURVEY;
			MCUCmd = MCU_SET_LED_MODE;
			break;
		case LED_POWER_UP:
			LinkStatus = LINK_STATUS_POWER_UP;
			MCUCmd = MCU_SET_LED_MODE;
			break;
#ifdef CONFIG_ATE
#endif /* CONFIG_ATE */
		default:
			DBGPRINT(RT_DEBUG_WARN, ("RTMPSetLED::Unknown Status 0x%x\n", Status));
			break;
	}

#ifdef MT7603
	if (IS_MT7603(pAd))
	{
		AndesLedOP(pAd, 0, LED_CMD);
		DBGPRINT(RT_DEBUG_TRACE, ("%s: LED_CMD:0x%x, LED Mode:0x%x, LinkStatus:0x%x\n", __FUNCTION__, LED_CMD, LedMode, LinkStatus)); 
	} else
#endif /*MT7603*/
	if (MCUCmd)
	{
		AsicSendCommandToMcu(pAd, MCUCmd, 0xff, LedMode, LinkStatus, FALSE);
		DBGPRINT(RT_DEBUG_TRACE, ("%s: MCUCmd:0x%x, LED Mode:0x%x, LinkStatus:0x%x\n", __FUNCTION__, MCUCmd, LedMode, LinkStatus)); 
	}
	
    /* */
	/* Keep LED status for LED SiteSurvey mode. */
	/* After SiteSurvey, we will set the LED mode to previous status. */
	/* */
	if ((Status != LED_ON_SITE_SURVEY) && (Status != LED_POWER_UP) && (bIgnored == FALSE))
		pAd->LedCntl.LedStatus = Status;
    
}


/*
	========================================================================
	
	Routine Description:
		Set LED Signal Stregth 

	Arguments:
		pAd						Pointer to our adapter
		Dbm						Signal Stregth

	Return Value:
		None

	IRQL = PASSIVE_LEVEL
	
	Note:
		Can be run on any IRQL level. 

		According to Microsoft Zero Config Wireless Signal Stregth definition as belows.
		<= -90  No Signal
		<= -81  Very Low
		<= -71  Low
		<= -67  Good
		<= -57  Very Good
		 > -57  Excellent		
	========================================================================
*/
VOID RTMPSetSignalLED(RTMP_ADAPTER *pAd, NDIS_802_11_RSSI Dbm)
{
	UCHAR		nLed = 0;


#ifdef RTMP_MAC_USB
#ifdef STATS_COUNT_SUPPORT
	if(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF))
		return;
#endif /* STATS_COUNT_SUPPORT */
#endif /* RTMP_MAC_USB */


	if (pAd->LedCntl.MCULedCntl.field.LedMode == LED_MODE_SIGNAL_STREGTH)
	{
		if (Dbm <= -90)
			nLed = 0;
		else if (Dbm <= -81)
			nLed = 1;
		else if (Dbm <= -71)
			nLed = 3;
		else if (Dbm <= -67)
			nLed = 7;
		else if (Dbm <= -57)
			nLed = 15;
		else 
			nLed = 31;

		/* */
		/* Update Signal Stregth to firmware if changed. */
		/* */
		if (pAd->LedCntl.LedIndicatorStrength != nLed)
		{
			AsicSendCommandToMcu(pAd, MCU_SET_LED_GPIO_SIGNAL_CFG, 0xff, nLed, pAd->LedCntl.MCULedCntl.field.Polarity, FALSE);
			pAd->LedCntl.LedIndicatorStrength = nLed;
		}
	}
}




void rtmp_read_led_setting_from_eeprom(RTMP_ADAPTER *pAd)
{
	USHORT Value;
	PLED_CONTROL pLedCntl = &pAd->LedCntl;
	{
		RT28xx_EEPROM_READ16(pAd, EEPROM_FREQ_OFFSET, Value);
		pLedCntl->MCULedCntl.word = (Value >> 8);
		RT28xx_EEPROM_READ16(pAd, EEPROM_LEDAG_CONF_OFFSET, Value);
		pLedCntl->LedAGCfg= Value;
		RT28xx_EEPROM_READ16(pAd, EEPROM_LEDACT_CONF_OFFSET, Value);
		pLedCntl->LedACTCfg = Value;
		RT28xx_EEPROM_READ16(pAd, EEPROM_LED_POLARITY_OFFSET, Value);
		pLedCntl->LedPolarity = Value;
	}
}


void RTMPStartLEDMode(IN RTMP_ADAPTER *pAd)
{
}


void RTMPInitLEDMode(IN RTMP_ADAPTER *pAd)
{
	PLED_CONTROL pLedCntl = &pAd->LedCntl;

	if (pLedCntl->MCULedCntl.word == 0xFF)
	{
		pLedCntl->MCULedCntl.word = 0x01;
		pLedCntl->LedAGCfg = 0x5555;
		pLedCntl->LedACTCfg= 0x2221;

#ifdef RTMP_MAC_USB
		pLedCntl->LedPolarity = 0x5627; 
#endif /* RTMP_MAC_USB */
	}
	
	AsicSendCommandToMcu(pAd, MCU_SET_LED_AG_CFG, 0xff, (UCHAR)pLedCntl->LedAGCfg, (UCHAR)(pLedCntl->LedAGCfg >> 8), FALSE);
	AsicSendCommandToMcu(pAd, MCU_SET_LED_ACT_CFG, 0xff, (UCHAR)pLedCntl->LedACTCfg, (UCHAR)(pLedCntl->LedACTCfg >> 8), FALSE);
	AsicSendCommandToMcu(pAd, MCU_SET_LED_POLARITY, 0xff, (UCHAR)pLedCntl->LedPolarity, (UCHAR)(pLedCntl->LedPolarity >> 8), FALSE);
	AsicSendCommandToMcu(pAd, MCU_SET_LED_GPIO_SIGNAL_CFG, 0xff, 0, pLedCntl->MCULedCntl.field.Polarity, FALSE);

	pAd->LedCntl.LedIndicatorStrength = 0xFF;
	RTMPSetSignalLED(pAd, -100);	/* Force signal strength Led to be turned off, before link up */

	RTMPStartLEDMode(pAd);
}


inline void RTMPExitLEDMode(IN RTMP_ADAPTER *pAd)
{

	RTMPSetLED(pAd, LED_RADIO_OFF);

	return;
}

#endif /* LED_CONTROL_SUPPORT */


INT RTMPSetLED(RTMP_ADAPTER *pAd, UCHAR Status)
{
	// TODO: shiang-7603

#ifdef LED_CONTROL_METHOD_1
	if (pAd->LedCntl.LedConfigMethod == 1)
		return TRUE;
#endif /* LED_CONTROL_Method1 */

#ifdef RTMP_MAC_USB
	if (IS_USB_INF(pAd))
	{
		UCHAR LEDStatus;
		LEDStatus = Status;
		if (Status == LED_RADIO_OFF)
			RTMPSetLEDStatus(pAd, Status);
		else
			RTEnqueueInternalCmd(pAd, CMDTHREAD_SET_LED_STATUS, &LEDStatus, sizeof(LEDStatus));
	}	
#endif /* RTMP_MAC_USB */


	return TRUE;
}


