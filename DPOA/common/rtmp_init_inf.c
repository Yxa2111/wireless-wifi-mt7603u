/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	rtmp_init_inf.c

	Abstract:
	Miniport generic portion header file

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/
#include	"rt_config.h"



#ifdef CONFIG_STA_SUPPORT
#ifdef PROFILE_STORE
NDIS_STATUS WriteDatThread(RTMP_ADAPTER *pAd);
#endif /* PROFILE_STORE */
#endif /* CONFIG_STA_SUPPORT */

#ifdef LINUX
#ifdef OS_ABL_FUNC_SUPPORT
/* Utilities provided from NET module */
RTMP_NET_ABL_OPS RtmpDrvNetOps, *pRtmpDrvNetOps = &RtmpDrvNetOps;
RTMP_PCI_CONFIG RtmpPciConfig, *pRtmpPciConfig = &RtmpPciConfig;
RTMP_USB_CONFIG RtmpUsbConfig, *pRtmpUsbConfig = &RtmpUsbConfig;

VOID RtmpDrvOpsInit(
	OUT VOID *pDrvOpsOrg,
	INOUT VOID *pDrvNetOpsOrg,
	IN RTMP_PCI_CONFIG *pPciConfig,
	IN RTMP_USB_CONFIG *pUsbConfig)
{
	RTMP_DRV_ABL_OPS *pDrvOps = (RTMP_DRV_ABL_OPS *)pDrvOpsOrg;
#ifdef RTMP_USB_SUPPORT
	RTMP_NET_ABL_OPS *pDrvNetOps = (RTMP_NET_ABL_OPS *)pDrvNetOpsOrg;
#endif /* RTMP_USB_SUPPORT */


	/* init PCI/USB configuration in different OS */
	if (pPciConfig != NULL)
		RtmpPciConfig = *pPciConfig;

	if (pUsbConfig != NULL)
		RtmpUsbConfig = *pUsbConfig;

	/* init operators provided from us (DRIVER module) */
	pDrvOps->RTMPAllocAdapterBlock = RTMPAllocAdapterBlock;
	pDrvOps->RTMPFreeAdapter = RTMPFreeAdapter;

	pDrvOps->RtmpRaDevCtrlExit = RtmpRaDevCtrlExit;
	pDrvOps->RtmpRaDevCtrlInit = RtmpRaDevCtrlInit;
	pDrvOps->RTMPSendPackets = RTMPSendPackets;

	pDrvOps->RTMP_COM_IoctlHandle = RTMP_COM_IoctlHandle;
#ifdef CONFIG_AP_SUPPORT
	pDrvOps->RTMP_AP_IoctlHandle = RTMP_AP_IoctlHandle;
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	pDrvOps->RTMP_STA_IoctlHandle = RTMP_STA_IoctlHandle;
#endif /* CONFIG_STA_SUPPORT */

	pDrvOps->RTMPDrvOpen = RTMPDrvOpen;
	pDrvOps->RTMPDrvClose = RTMPDrvClose;
	pDrvOps->RTMPInfClose = RTMPInfClose;
	pDrvOps->rt28xx_init = rt28xx_init;

	/* init operators provided from us and netif module */
#ifdef RTMP_USB_SUPPORT
	*pRtmpDrvNetOps = *pDrvNetOps;
	pRtmpDrvNetOps->RtmpDrvUsbBulkOutDataPacketComplete = RTUSBBulkOutDataPacketComplete;
	pRtmpDrvNetOps->RtmpDrvUsbBulkOutMLMEPacketComplete = RTUSBBulkOutMLMEPacketComplete;
	pRtmpDrvNetOps->RtmpDrvUsbBulkOutNullFrameComplete = RTUSBBulkOutNullFrameComplete;
/*	pRtmpDrvNetOps->RtmpDrvUsbBulkOutRTSFrameComplete = RTUSBBulkOutRTSFrameComplete;*/
	pRtmpDrvNetOps->RtmpDrvUsbBulkOutPsPollComplete = RTUSBBulkOutPsPollComplete;
	pRtmpDrvNetOps->RtmpDrvUsbBulkRxComplete = RTUSBBulkRxComplete;
#ifdef MT_MAC
    pRtmpDrvNetOps->RtmpDrvUsbBulkOutBCNPacketComplete = RTUSBBulkOutBCNPacketComplete;
#endif
	*pDrvNetOps = *pRtmpDrvNetOps;
#endif /* RTMP_USB_SUPPORT */
}

RTMP_BUILD_DRV_OPS_FUNCTION_BODY

#endif /* OS_ABL_FUNC_SUPPORT */
#endif /* LINUX */


INT rtmp_sys_exit(RTMP_ADAPTER *pAd)
{

	MeasureReqTabExit(pAd);
	TpcReqTabExit(pAd);

#ifdef DOT11_N_SUPPORT
	if(pAd->mpdu_blk_pool.mem) {
		os_free_mem(pAd, pAd->mpdu_blk_pool.mem); /* free BA pool*/
		pAd->mpdu_blk_pool.mem = NULL;
	}
#endif /* DOT11_N_SUPPORT */

	return TRUE;
}


INT rtmp_sys_init(RTMP_ADAPTER *pAd)
{
	NDIS_STATUS status;

#ifdef DOT11_N_SUPPORT
	/* Allocate BA Reordering memory*/
	if (ba_reordering_resource_init(pAd, MAX_REORDERING_MPDU_NUM) != TRUE)
		goto err;
#endif /* DOT11_N_SUPPORT */

#ifdef BLOCK_NET_IF
	initblockQueueTab(pAd);
#endif /* BLOCK_NET_IF */

	status = MeasureReqTabInit(pAd);
	if (status != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT_ERR(("MeasureReqTabInit failed, Status[=0x%08x]\n", status));
		goto err;
	}
	status = TpcReqTabInit(pAd);
	if (status != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT_ERR(("TpcReqTabInit failed, Status[=0x%08x]\n", status));
		goto err;
	}


	return TRUE;

err:
	return FALSE;

}



INT rtmp_cfg_exit(RTMP_ADAPTER *pAd)
{
	UserCfgExit(pAd);

	return TRUE;
}


INT rtmp_cfg_init(RTMP_ADAPTER *pAd, RTMP_STRING *pHostName)
{
	NDIS_STATUS status;

	UserCfgInit(pAd);


	CfgInitHook(pAd);

	/*
		WiFi system operation mode setting base on following partitions:
		1. Parameters from config file
		2. Hardware cap from EEPROM
		3. Chip capabilities in code
	*/
	if (pAd->RfIcType == 0) {
		/* RfIcType not assigned, should not happened! */
		pAd->RfIcType = RFIC_UNKNOWN;
		DBGPRINT(RT_DEBUG_OFF, ("%s(): Invalid RfIcType, reset it first\n",
					__FUNCTION__));
	}

    
    RTMPPreReadParametersHook(pAd);
	status = RTMPReadParametersHook(pAd);
	if (status != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT_ERR(("RTMPReadParametersHook failed, Status[=0x%08x]\n",status));
		return FALSE;
	}

#ifdef DOT11_N_SUPPORT
   	/*Init Ba Capability parameters.*/
	pAd->CommonCfg.DesiredHtPhy.MpduDensity = (UCHAR)pAd->CommonCfg.BACapability.field.MpduDensity;
	pAd->CommonCfg.DesiredHtPhy.AmsduEnable = (USHORT)pAd->CommonCfg.BACapability.field.AmsduEnable;
	pAd->CommonCfg.DesiredHtPhy.AmsduSize = (USHORT)pAd->CommonCfg.BACapability.field.AmsduSize;
	pAd->CommonCfg.DesiredHtPhy.MimoPs = (USHORT)pAd->CommonCfg.BACapability.field.MMPSmode;
	/* Updata to HT IE*/
	pAd->CommonCfg.HtCapability.HtCapInfo.MimoPs = (USHORT)pAd->CommonCfg.BACapability.field.MMPSmode;
	pAd->CommonCfg.HtCapability.HtCapInfo.AMsduSize = (USHORT)pAd->CommonCfg.BACapability.field.AmsduSize;
	pAd->CommonCfg.HtCapability.HtCapParm.MpduDensity = (UCHAR)pAd->CommonCfg.BACapability.field.MpduDensity;
#endif /* DOT11_N_SUPPORT */

	return TRUE;
}


INT rtmp_mgmt_init(RTMP_ADAPTER *pAd)
{

	return TRUE;
}


int rt28xx_init(VOID *pAdSrc, RTMP_STRING *pDefaultMac, RTMP_STRING *pHostName)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pAdSrc;
	NDIS_STATUS Status;
#ifdef RTMP_MAC_USB
	UINT index = 0;
#endif /* RTMP_MAC_USB */

	if (!pAd)
		return FALSE;

#if defined(RLT_MAC) || defined(RTMP_MAC)
	if (rtmp_asic_top_init(pAd) != TRUE)
		goto err0;
#endif /* defined(RLT_MAC) || defined(RTMP_MAC) */

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT) {
		if (mt_asic_top_init(pAd) != TRUE)
			goto err0;
	}
#endif /* MT_MAC */

	DBGPRINT(RT_DEBUG_TRACE, ("MAC[Ver:Rev=0x%08x : 0x%08x]\n",
				pAd->MACVersion, pAd->ChipID));

	if (hif_sys_init(pAd, TRUE) != TRUE)
		goto err1;

	Status = RtmpNetTaskInit(pAd);
	if (Status != NDIS_STATUS_SUCCESS)
		goto err2;


	/* reset Adapter flags */
	RTMP_CLEAR_FLAGS(pAd);
	
#ifdef RTMP_MAC_USB
#ifdef CONFIG_ATE
/*if not init ATECtrl->Mode = ATE_STOP ,Rxr receive will pending, and Fw_init will fail*/
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	NdisZeroMemory(ATECtrl, sizeof(*ATECtrl));
	ATECtrl->Mode = ATE_STOP;
#endif /* CONFIG_ATE */
#endif /* RTMP_MAC_USB */

	
	Status = RtmpMgmtTaskInit(pAd);
	if (Status != NDIS_STATUS_SUCCESS)
		goto err3;


	/* initialize MLME*/
	Status = MlmeInit(pAd);
	if (Status != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT_ERR(("MlmeInit failed, Status[=0x%08x]\n", Status));
		goto err4;
	}

	/* Initialize pAd->StaCfg, pAd->ApCfg, pAd->CommonCfg to manufacture default*/
	if (rtmp_cfg_init(pAd, pHostName) != TRUE)
		goto err5;


	if (MCUSysInit(pAd) != NDIS_STATUS_SUCCESS)
		goto err6;

	/* hook e2p operation */
	RtmpChipOpsEepromHook(pAd, pAd->infType);

#if defined(MT7603_FPGA) || defined(MT7628_FPGA)
	if (pAd->chipCap.hif_type == HIF_MT) {
		UINT32 ver, date_code, rev;
		UINT32 mac_val;

		RTMP_IO_READ32(pAd, 0x2700, &ver);
		RTMP_IO_READ32(pAd, 0x2704, &rev);
		RTMP_IO_READ32(pAd, 0x2708, &date_code);
		RTMP_IO_READ32(pAd, 0x201f8, &mac_val);
		DBGPRINT(RT_DEBUG_OFF, ("##########################################\n"));
		DBGPRINT(RT_DEBUG_OFF, ("%s(%d): MT7603 FPGA Version:\n", __FUNCTION__, __LINE__));

		DBGPRINT(RT_DEBUG_OFF, ("\tFGPA1: Code[0x700]:0x%x, [0x704]:0x%x, [0x708]:0x%x\n",
					ver, rev, date_code));
		DBGPRINT(RT_DEBUG_OFF, ("\tFPGA2: Version[0x201f8]:0x%x\n", mac_val));
		DBGPRINT(RT_DEBUG_OFF, ("##########################################\n"));
	}
#endif /* MT7603_FPGA */

	if (MAX_LEN_OF_MAC_TABLE > MAX_AVAILABLE_CLIENT_WCID(pAd))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("MAX_LEN_OF_MAC_TABLE can not be larger than MAX_AVAILABLE_CLIENT_WCID!!!!\n"));
		goto err6;
	}

	if (rtmp_sys_init(pAd) != TRUE)
		goto err7;

	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE);
	

	DBGPRINT(RT_DEBUG_OFF, ("1. Phy Mode = %d\n", pAd->CommonCfg.PhyMode));

	/* We should read EEPROM for all cases */
	// TODO: shiang-7603, revise this!
	NICReadEEPROMParameters(pAd, (RTMP_STRING *)pDefaultMac);

	DBGPRINT(RT_DEBUG_OFF, ("2. Phy Mode = %d\n", pAd->CommonCfg.PhyMode));

	RTMP_NET_DEV_NICKNAME_INIT(pAd);

	/* After operation mode is finialized, init the AP or STA mode */
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		APInitialize(pAd);

		/* Init BssTab & ChannelInfo tabbles for auto channel select.*/
		AutoChBssTableInit(pAd);
		ChannelInfoInit(pAd);
	}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		STAInitialize(pAd);

#ifdef CREDENTIAL_STORE
		RecoverConnectInfo(pAd);
#endif /* CREDENTIAL_STORE */
	}
#endif /* CONFIG_STA_SUPPORT */

	/* after reading Registry, we now know if in AP mode or STA mode */
	DBGPRINT(RT_DEBUG_OFF, ("3. Phy Mode = %d\n", pAd->CommonCfg.PhyMode));

	/*
		All settle down, now it's time to init asic related parameters
	*/
	/* Init the hardware, we need to init asic before read registry, otherwise mac register will be reset */
	Status = NICInitializeAdapter(pAd, TRUE);
	if (Status != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT_ERR(("NICInitializeAdapter failed, Status[=0x%08x]\n", Status));
		if (Status != NDIS_STATUS_SUCCESS)
		goto err8;
	}

#ifdef CONFIG_FPGA_MODE
#ifdef CAPTURE_MODE
	cap_mode_init(pAd);
#endif /* CAPTURE_MODE */
#endif /* CONFIG_FPGA_MODE */

	NICInitAsicFromEEPROM(pAd);


	
#ifdef LED_CONTROL_SUPPORT
	/* Send LED Setting to MCU */
	RTMPInitLEDMode(pAd);
#endif /* LED_CONTROL_SUPPORT */

	/*
		Do necessary calibration after ASIC initialized
		this's chip variant and may different for different chips
	*/


	tx_pwr_comp_init(pAd);


	/* Set PHY to appropriate mode and will update the ChannelListNum in this function */
	RTMPSetPhyMode(pAd, pAd->CommonCfg.PhyMode);
	if (pAd->ChannelListNum == 0)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Wrong configuration. No valid channel found. Check \"ContryCode\" and \"ChannelGeography\" setting.\n"));
		goto err8;
	}

#ifdef DOT11_N_SUPPORT
	DBGPRINT(RT_DEBUG_OFF, ("MCS Set = %02x %02x %02x %02x %02x\n",
				pAd->CommonCfg.HtCapability.MCSSet[0],
				pAd->CommonCfg.HtCapability.MCSSet[1],
				pAd->CommonCfg.HtCapability.MCSSet[2],
				pAd->CommonCfg.HtCapability.MCSSet[3],
				pAd->CommonCfg.HtCapability.MCSSet[4]));
#endif /* DOT11_N_SUPPORT */

#ifdef WIN_NDIS
	/* Patch cardbus controller if EEPROM said so. */
	if (pAd->bTest1 == FALSE)
		RTMPPatchCardBus(pAd);
#endif /* WIN_NDIS */

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
#ifdef AP_QLOAD_SUPPORT
		QBSS_LoadInit(pAd);
#endif /* AP_QLOAD_SUPPORT */
	}
#endif /* CONFIG_AP_SUPPORT */

#ifdef IKANOS_VX_1X0
	VR_IKANOS_FP_Init(pAd->ApCfg.BssidNum, pAd->PermanentAddress);
#endif /* IKANOS_VX_1X0 */

#ifdef RTMP_MAC_USB

	if (pAd->chipCap.MCUType == M8051) {
		AsicSendCommandToMcu(pAd, 0x31, 0xff, 0x00, 0x02, FALSE);
		RtmpusecDelay(10000);
	}
#endif /* RTMP_MAC_USB */

#ifdef CONFIG_ATE
	if (ATEInit(pAd) != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s(): ATE initialization failed !\n", __FUNCTION__));
		goto err9;
	}

#endif /* CONFIG_ATE */


#ifdef RTMP_INTERNAL_TX_ALC
#endif /* RTMP_INTERNAL_TX_ALC */

	/*
		Some modules init must be called before APStartUp().
		Or APStartUp() will make up beacon content and call
		other modules API to get some information to fill.
	*/

	/* Microsoft HCT require driver send a disconnect event after driver initialization.*/
	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED);
	OPSTATUS_CLEAR_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED);

	DBGPRINT(RT_DEBUG_TRACE, ("NDIS_STATUS_MEDIA_DISCONNECT Event B!\n"));

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		if (pAd->ApCfg.bAutoChannelAtBootup || (pAd->CommonCfg.Channel == 0))
		{
			/* Enable Interrupt first due to we need to scan channel to receive beacons.*/
#ifdef RTMP_MAC_USB
			RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS);
			RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_REMOVE_IN_PROGRESS);

			/* Support multiple BulkIn IRP,*/
			/* the value on pAd->CommonCfg.NumOfBulkInIRP may be large than 1.*/

			for(index=0; index<pAd->CommonCfg.NumOfBulkInIRP; index++)
			{
				RTUSBBulkReceive(pAd);
				DBGPRINT(RT_DEBUG_TRACE, ("RTUSBBulkReceive!\n" ));
			}
#endif /* RTMP_MAC_USB */

			/* Now Enable RxTx*/
			RTMPEnableRxTx(pAd);
				RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_START_UP);

			/* Let BBP register at 20MHz to do scan */
			bbp_set_bw(pAd, BW_20);

			/* Now we can receive the beacon and do the listen beacon*/
			/* use default BW to select channel*/
			pAd->CommonCfg.Channel = AP_AUTO_CH_SEL(pAd, pAd->ApCfg.AutoChannelAlg);
			pAd->ApCfg.bAutoChannelAtBootup = FALSE;
		}

#ifdef DOT11_N_SUPPORT
		/* If WMODE_CAP_N(phymode) and BW=40 check extension channel, after select channel  */
		N_ChannelCheck(pAd);

#ifdef DOT11N_DRAFT3
    		/*
     			We only do this Overlapping BSS Scan when system up, for the
			other situation of channel changing, we depends on station's
			report to adjust ourself.
		*/
		if (pAd->CommonCfg.bForty_Mhz_Intolerant == TRUE)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("Disable 20/40 BSSCoex Channel Scan(BssCoex=%d, 40MHzIntolerant=%d)\n",
										pAd->CommonCfg.bBssCoexEnable,
										pAd->CommonCfg.bForty_Mhz_Intolerant));
		}
		else if(pAd->CommonCfg.bBssCoexEnable == TRUE)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("Enable 20/40 BSSCoex Channel Scan(BssCoex=%d)\n",
						pAd->CommonCfg.bBssCoexEnable));
			APOverlappingBSSScan(pAd);
		}

		RTMP_11N_D3_TimerInit(pAd);
/*			RTMPInitTimer(pAd, &pAd->CommonCfg.Bss2040CoexistTimer, GET_TIMER_FUNCTION(Bss2040CoexistTimeOut), pAd, FALSE);*/
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */

		APStartUp(pAd);
	
		MlmeRadioOn(pAd);

	}
#endif /* CONFIG_AP_SUPPORT */

#ifdef DYNAMIC_VGA_SUPPORT
	if (pAd->CommonCfg.lna_vga_ctl.bDyncVgaEnable)
	{
		dynamic_vga_enable(pAd);
	}
#endif /* DYNAMIC_VGA_SUPPORT */


// TODO: shiang-7603, work-around for it now!! Need a better place for it!
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		if (IS_MT7603(pAd) || IS_MT7628(pAd) || IS_MT7636(pAd)) {
			/* Now Enable RxTx*/
			RTMPEnableRxTx(pAd);
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_START_UP);
		}
	}

#ifdef RTMP_MAC_USB
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS);
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_REMOVE_IN_PROGRESS);

	/* Support multiple BulkIn IRP,*/
	/* the value on pAd->CommonCfg.NumOfBulkInIRP may be large than 1.*/
	for (index=0; index<pAd->CommonCfg.NumOfBulkInIRP; index++)
	{
		RTUSBBulkReceive(pAd);
		DBGPRINT(RT_DEBUG_TRACE, ("RTUSBBulkReceive!\n" ));
	}
#endif /* RTMP_MAC_USB */

	/* Set up the Mac address*/
#ifdef CONFIG_AP_SUPPORT
	RtmpOSNetDevAddrSet(pAd->OpMode, pAd->net_dev, &pAd->CurrentAddress[0], NULL);
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	NdisMoveMemory(&pAd->StaCfg.wdev.if_addr[0], &pAd->CurrentAddress[0], MAC_ADDR_LEN);
	RtmpOSNetDevAddrSet(pAd->OpMode, pAd->net_dev, &pAd->CurrentAddress[0], (PUCHAR)(pAd->StaCfg.dev_name));
	NdisMoveMemory(&pAd->StaCfg.wdev.if_addr[0], &pAd->CurrentAddress[0], MAC_ADDR_LEN);
#endif /* CONFIG_STA_SUPPORT */

#ifdef UAPSD_SUPPORT
        UAPSD_Init(pAd);
#endif /* UAPSD_SUPPORT */

	/* assign function pointers*/

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		ap_func_init(pAd);
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		sta_func_init(pAd);
#endif /* CONFIG_STA_SUPPORT */

#ifdef STREAM_MODE_SUPPORT
	RtmpStreamModeInit(pAd);
#endif /* STREAM_MODE_SUPPORT */


#ifdef DOT11_N_SUPPORT
#endif /* DOT11_N_SUPPORT */



	DBGPRINT_S(("<==== rt28xx_init, Status=%x\n", Status));

	return TRUE;

#ifdef CONFIG_ATE
err9:
	ATEExit(pAd);
#endif

err8:
#ifdef IGMP_SNOOP_SUPPORT
	MultiCastFilterTableReset(&pAd->pMulticastFilterTable);
#endif /* IGMP_SNOOP_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		/* Free BssTab & ChannelInfo tabbles.*/
		AutoChBssTableDestroy(pAd);
		ChannelInfoDestroy(pAd);
	}
#endif /* CONFIG_AP_SUPPORT */

err7:
	rtmp_sys_exit(pAd);
	
err6:
	MCUSysExit(pAd);

err5:
	rtmp_cfg_exit(pAd);

err4:
	MlmeHalt(pAd);
	RTMP_AllTimerListRelease(pAd);

err3:
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE);

	RtmpMgmtTaskExit(pAd);
#ifdef RTMP_TIMER_TASK_SUPPORT
	NdisFreeSpinLock(&pAd->TimerQLock);
#endif

err2:
	RtmpNetTaskExit(pAd);

err1:
	hif_sys_exit(pAd);

err0:

	DBGPRINT(RT_DEBUG_ERROR, ("!!! rt28xx init fail !!!\n"));
	return FALSE;
}


VOID RTMPDrvOpen(VOID *pAdSrc)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pAdSrc;

	RTMP_CLEAR_PSFLAG(pAd, fRTMP_PS_MCU_SLEEP);
#ifdef CONFIG_STA_SUPPORT
#ifdef CFG_TDLS_SUPPORT
#ifdef MT_MAC //ADD TDLS TxsType
	if (pAd->chipCap.hif_type == HIF_MT) {
		AddTxSTypePerPkt(pAd, PID_TDLS, TXS_FORMAT0, TdlsTxSHandler);
		TxSTypeCtlPerPkt(pAd, PID_TDLS, TXS_FORMAT0, FALSE, TRUE, FALSE, 0); 
	}
#endif //MT_MAC
#endif //CFG_TDLS_SUPPORT
#endif /* CONFIG_STA_SUPPORT */
#ifdef UAPSD_SUPPORT
#ifdef MT_MAC //ADD UAPSD TxsType
	if (pAd->chipCap.hif_type == HIF_MT) {
		AddTxSTypePerPkt(pAd, PID_UAPSD, TXS_FORMAT0, UAPSDTxSHandler);
		TxSTypeCtlPerPkt(pAd, PID_UAPSD, TXS_FORMAT0, FALSE, TRUE, FALSE, 0); 
	}
#endif //MT_MAC
#endif

#ifdef RTMP_MAC
	// TODO: shiang-usw, check this for RMTP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP) {

		/* Now Enable RxTx*/
		RTMPEnableRxTx(pAd);
	}
#endif /* RTMP_MAC */


	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_START_UP);




#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
#ifdef PCIE_PS_SUPPORT
		RTMPInitPCIeLinkCtrlValue(pAd);
#endif /* PCIE_PS_SUPPORT */

	}
#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
#ifdef BG_FT_SUPPORT
	BG_FTPH_Init();
#endif /* BG_FT_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */


#ifdef CONFIG_STA_SUPPORT
	/*
		To reduce connection time,
		do auto reconnect here instead of waiting STAMlmePeriodicExec to do auto reconnect.
	*/
	if (pAd->OpMode == OPMODE_STA)
		MlmeAutoReconnectLastSSID(pAd);
#endif /* CONFIG_STA_SUPPORT */


#ifdef CONFIG_STA_SUPPORT
#ifdef DOT11W_PMF_SUPPORT
	if (pAd->OpMode == OPMODE_STA)
	{
		pAd->StaCfg.PmfCfg.MFPC = FALSE;
		pAd->StaCfg.PmfCfg.MFPR = FALSE;
		pAd->StaCfg.PmfCfg.PMFSHA256 = FALSE;
		if ((pAd->StaCfg.wdev.AuthMode == Ndis802_11AuthModeWPA2 || pAd->StaCfg.wdev.AuthMode == Ndis802_11AuthModeWPA2PSK)
			&& (pAd->StaCfg.wdev.WepStatus == Ndis802_11AESEnable))
		{
			pAd->StaCfg.PmfCfg.PMFSHA256 = pAd->StaCfg.PmfCfg.Desired_PMFSHA256;
			if (pAd->StaCfg.PmfCfg.Desired_MFPC)
			{
				pAd->StaCfg.PmfCfg.MFPC = TRUE;
				pAd->StaCfg.PmfCfg.MFPR = pAd->StaCfg.PmfCfg.Desired_MFPR;

				if (pAd->StaCfg.PmfCfg.MFPR)
					pAd->StaCfg.PmfCfg.PMFSHA256 = TRUE;
			}
		} else if (pAd->StaCfg.PmfCfg.Desired_MFPC) {
			DBGPRINT(RT_DEBUG_ERROR, ("[PMF]%s:: Security is not WPA2/WPA2PSK AES\n", __FUNCTION__));
		}

		DBGPRINT(RT_DEBUG_ERROR, ("[PMF]%s:: MFPC=%d, MFPR=%d, SHA256=%d\n",
					__FUNCTION__, pAd->StaCfg.PmfCfg.MFPC, pAd->StaCfg.PmfCfg.MFPR,
					pAd->StaCfg.PmfCfg.PMFSHA256));
	}
#endif /* DOT11W_PMF_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */

#if defined(RT_CFG80211_P2P_SUPPORT) && defined(SUPPORT_ACS_ALL_CHANNEL_RANK)
    if (pAd->ApCfg.bAutoChannelAtBootup && pAd->ApCfg.bAutoChannelScaned == 0)
    {
#ifdef RTMP_MAC_USB
        RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS);
        RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_REMOVE_IN_PROGRESS);

        {
            UCHAR index;
            for(index=0; index<pAd->CommonCfg.NumOfBulkInIRP; index++)
            {
                RTUSBBulkReceive(pAd);
                DBGPRINT(RT_DEBUG_TRACE, ("RTUSBBulkReceive!\n" ));
            }
        }
#endif /* RTMP_MAC_USB */
        /* Now Enable RxTx*/
        RTMPEnableRxTx(pAd);
        //RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_START_UP);
        
        RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_START_UP);
        /* Let BBP register at 20MHz to do scan */
        bbp_set_bw(pAd, BW_20);
        DBGPRINT(RT_DEBUG_ERROR, ("SYNC - BBP R4 to 20MHz.l\n"));
        AP_AUTO_CH_SEL(pAd, pAd->ApCfg.AutoChannelAlg);
        pAd->ApCfg.bAutoChannelScaned = 1;
    }
#endif /* SUPPORT_ACS_ALL_CHANNEL_RANK */


#ifdef MT_MAC
	pAd->PSEWatchDogEn = 1;

#ifdef MT_WOW_SUPPORT
	pAd->WOW_Cfg.bWoWRunning = FALSE;
#endif

#endif

}


VOID RTMPDrvClose(VOID *pAdSrc, VOID *net_dev)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pAdSrc;
	UINT32 i = 0;

#ifdef MT_MAC
	pAd->PSEWatchDogEn = 0;
#endif

#ifdef CONFIG_STA_SUPPORT
#ifdef CREDENTIAL_STORE
		if (pAd->IndicateMediaState == NdisMediaStateConnected)
			StoreConnectInfo(pAd);
		else
		{
			RTMP_SEM_LOCK(&pAd->StaCtIf.Lock);
			pAd->StaCtIf.Changeable = FALSE;
			RTMP_SEM_UNLOCK(&pAd->StaCtIf.Lock);
		}
#endif /* CREDENTIAL_STORE */
#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
#ifdef BG_FT_SUPPORT
	BG_FTPH_Remove();
#endif /* BG_FT_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */


#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
#ifdef PCIE_PS_SUPPORT
		RTMPPCIeLinkCtrlValueRestore(pAd, RESTORE_CLOSE);
#endif /* PCIE_PS_SUPPORT */

		/* If dirver doesn't wake up firmware here,*/
		/* NICLoadFirmware will hang forever when interface is up again.*/
		if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE))
        {
		    AsicForceWakeup(pAd, TRUE);
        }

#ifdef RTMP_MAC_USB

#if (defined(MT_WOW_SUPPORT) && defined(WOW_IFDOWN_SUPPORT))
		if (!((pAd->WOW_Cfg.bEnable == TRUE) && INFRA_ON(pAd)))
#endif
#ifndef PAT
		{
#endif		
		RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_REMOVE_IN_PROGRESS);
		
				// workaround to prevent EP84 from disappearing unexpectedly
		if (1){
			int k = 0;
			while(RTUSBQueueLen(pAd, &pAd->RxBulkInQ))
			{
				msleep(1);
				DBGPRINT(RT_DEBUG_OFF, ("\twait RxBulkInQ empty\n"));
				k++;
				if (k == 100)
					break;
			}
			//CmdRadioOnOffCtrl(pAd, WIFI_RADIO_OFF);
			/*	Disable RX */
			AsicSetMacTxRx(pAd, ASIC_MAC_RX, FALSE);
		}		
				}

#endif /* RTMP_MAC_USB */

	}
#endif /* CONFIG_STA_SUPPORT */

#if ((defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) || defined(MT_WOW_SUPPORT)) && defined(WOW_IFDOWN_SUPPORT)
	if (!((pAd->WOW_Cfg.bEnable == TRUE) && INFRA_ON(pAd)))
#endif /* ((defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) || defined(MT_WOW_SUPPORT)) && defined(WOW_IFDOWN_SUPPORT) */
	{
#ifdef MT_MAC
		if (pAd->chipCap.hif_type != HIF_MT)
#endif /* MT_MAC */
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);
	}

#ifdef EXT_BUILD_CHANNEL_LIST
	if (pAd->CommonCfg.pChDesp != NULL)
		os_free_mem(NULL, pAd->CommonCfg.pChDesp);
	pAd->CommonCfg.pChDesp = NULL;
	pAd->CommonCfg.DfsType = MAX_RD_REGION;
	pAd->CommonCfg.bCountryFlag = 0;
#endif /* EXT_BUILD_CHANNEL_LIST */
	pAd->CommonCfg.bCountryFlag = FALSE;




	for (i = 0 ; i < NUM_OF_TX_RING; i++)
	{
		while (pAd->DeQueueRunning[i] == TRUE)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("Waiting for TxQueue[%d] done..........\n", i));
			RtmpusecDelay(1000);
		}
	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		BOOLEAN Cancelled = FALSE;
#ifdef RTMP_MAC_USB
#ifndef BCN_OFFLOAD_SUPPORT
		RTMPCancelTimer(&pAd->CommonCfg.BeaconUpdateTimer, &Cancelled);
#endif
#endif /* RTMP_MAC_USB */

#ifdef DOT11N_DRAFT3
		if (pAd->CommonCfg.Bss2040CoexistFlag & BSS_2040_COEXIST_TIMER_FIRED)
		{
			RTMPCancelTimer(&pAd->CommonCfg.Bss2040CoexistTimer, &Cancelled);
			pAd->CommonCfg.Bss2040CoexistFlag  = 0;
		}
#endif /* DOT11N_DRAFT3 */

		/* PeriodicTimer already been canceled by MlmeHalt() API.*/
		/*RTMPCancelTimer(&pAd->PeriodicTimer,	&Cancelled);*/
	}
#endif /* CONFIG_AP_SUPPORT */
	
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
#if (defined(MT_WOW_SUPPORT) && defined(WOW_IFDOWN_SUPPORT))
		if (!((pAd->WOW_Cfg.bEnable == TRUE) && INFRA_ON(pAd)))
#endif 
			MacTableReset(pAd, 1);
#if ((defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) || defined(MT_WOW_SUPPORT)) && defined(WOW_IFDOWN_SUPPORT)
		if ((pAd->WOW_Cfg.bEnable == TRUE) && INFRA_ON(pAd)){
			
			pAd->WOW_Cfg.bWoWRunning = TRUE;
			
			// Set channel to AP's channel
			if (pAd->chipOps.ChipSwitchChannel){
				pAd->chipOps.ChipSwitchChannel(pAd, pAd->CommonCfg.Channel, FALSE);
#ifdef MT_MAC
				// TODO: shiang-7603
				if (pAd->chipCap.hif_type == HIF_MT){
					UINT32 val;
			
					RTMP_IO_READ32(pAd, RMAC_CHFREQ, &val);
					val = 1;
					RTMP_IO_WRITE32(pAd, RMAC_CHFREQ, val);
			
				}
#endif /* MT_MAC */

					
				RTMPSetAGCInitValue(pAd, pAd->CommonCfg.BBPCurrentBW);
			}
				
			ASIC_WOW_ENABLE(pAd);
			// TODO: Pat: set when FW is ready
			//CmdExtPwrMgtBitWifi(pAd, BSSID_WCID, PWR_SAVE); //It is used for STA in Infra mode
			CmdExtPmStateCtrl(pAd, BSSID_WCID, PM4, ENTER_PM_STATE);			
		}
		else
#endif /* ((defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) || defined(MT_WOW_SUPPORT)) && defined(WOW_IFDOWN_SUPPORT) */
			MlmeRadioOff(pAd);
	}
#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{

#ifdef CLIENT_WDS
		CliWds_ProxyTabDestory(pAd);
#endif /* CLIENT_WDS */
		/* Shutdown Access Point function, release all related resources */
		APShutdown(pAd);

/*#ifdef AUTO_CH_SELECT_ENHANCE*/
		/* Free BssTab & ChannelInfo tabbles.*/
/*		AutoChBssTableDestroy(pAd); */
/*		ChannelInfoDestroy(pAd); */
/*#endif  AUTO_CH_SELECT_ENHANCE */
	}
#endif /* CONFIG_AP_SUPPORT */

	/* Close net tasklets*/
	RtmpNetTaskExit(pAd);

	/* Stop Mlme state machine*/
	MlmeHalt(pAd);
	
	MeasureReqTabExit(pAd);
	TpcReqTabExit(pAd);

#ifdef LED_CONTROL_SUPPORT
	RTMPExitLEDMode(pAd);
#endif // LED_CONTROL_SUPPORT

	/* Close kernel threads*/
	RtmpMgmtTaskExit(pAd);

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		/* must after RtmpMgmtTaskExit(); Or pAd->pChannelInfo will be NULL */
		/* Free BssTab & ChannelInfo tabbles.*/
		AutoChBssTableDestroy(pAd);
		ChannelInfoDestroy(pAd);
	}
#endif /* CONFIG_AP_SUPPORT */


	/* Free IRQ*/
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
	{
		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE);
	}

#ifdef SINGLE_SKU_V2
	{
		CH_POWER *ch, *ch_temp;
		DlListForEachSafe(ch, ch_temp, &pAd->SingleSkuPwrList, CH_POWER, List)
		{
			DlListDel(&ch->List);
			os_free_mem(NULL, ch);
		}
	}
#endif /* SINGLE_SKU_V2 */

	MCU_CTRL_EXIT(pAd);

	/* Free Ring or USB buffers*/
#ifdef RESOURCE_PRE_ALLOC
	RTMPResetTxRxRingMemory(pAd);
#else
	/* Free Ring or USB buffers*/
	RTMPFreeTxRxRingMemory(pAd);
#endif /* RESOURCE_PRE_ALLOC */

	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);

#ifdef WLAN_SKB_RECYCLE
	skb_queue_purge(&pAd->rx0_recycle);
#endif /* WLAN_SKB_RECYCLE */

#ifdef DOT11_N_SUPPORT
	/* Free BA reorder resource*/
	ba_reordering_resource_release(pAd);
#endif /* DOT11_N_SUPPORT */

	UserCfgExit(pAd); /* must after ba_reordering_resource_release */

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
		ExitTxSTypeTable(pAd);
#endif

#ifdef CONFIG_STA_SUPPORT
#endif /* CONFIG_STA_SUPPORT */

	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_START_UP);

/*+++Modify by woody to solve the bulk fail+++*/
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
	}
#endif /* CONFIG_STA_SUPPORT */

	/* clear MAC table */
	/* TODO: do not clear spin lock, such as fLastChangeAccordingMfbLock */
	NdisZeroMemory(&pAd->MacTab, sizeof(MAC_TABLE));

	/* release all timers */
	RtmpusecDelay(2000);
	RTMP_AllTimerListRelease(pAd);

#ifdef RTMP_TIMER_TASK_SUPPORT
	NdisFreeSpinLock(&pAd->TimerQLock);
#endif /* RTMP_TIMER_TASK_SUPPORT */

#ifdef CONFIG_FPGA_MODE
#ifdef CAPTURE_MODE
	cap_mode_deinit(pAd);
#endif /* CAPTURE_MODE */
#endif /* CONFIG_FPGA_MODE */

}


VOID RTMPInfClose(VOID *pAdSrc)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pAdSrc;

#ifdef CONFIG_AP_SUPPORT
	pAd->ApCfg.MBSSID[MAIN_MBSSID].bcn_buf.bBcnSntReq = FALSE;

	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		/* kick out all STAs behind the bss.*/
		MbssKickOutStas(pAd, MAIN_MBSSID, REASON_DISASSOC_INACTIVE);
	}

	//CFG_TODO
#ifndef RT_CFG80211_SUPPORT
	APMakeAllBssBeacon(pAd);
	APUpdateAllBeaconFrame(pAd);
#endif /* RT_CFG80211_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */



#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
#ifdef PROFILE_STORE
		WriteDatThread(pAd);
		RtmpusecDelay(1000);
#endif /* PROFILE_STORE */

		if (INFRA_ON(pAd) &&
#if ((defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) || defined(MT_WOW_SUPPORT)) && defined(WOW_IFDOWN_SUPPORT)
	/* In WOW state, can't issue disassociation reqeust */
			pAd->WOW_Cfg.bEnable == FALSE &&
#endif /* ((defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) || defined(MT_WOW_SUPPORT)) && defined(WOW_IFDOWN_SUPPORT) */
			(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)))
		{
			MLME_DISASSOC_REQ_STRUCT	DisReq;
			MLME_QUEUE_ELEM *MsgElem;

			os_alloc_mem(NULL, (UCHAR **)&MsgElem, sizeof(MLME_QUEUE_ELEM));
			if (MsgElem)
			{
			COPY_MAC_ADDR(DisReq.Addr, pAd->CommonCfg.Bssid);
			DisReq.Reason =  REASON_DEAUTH_STA_LEAVING;

			MsgElem->Machine = ASSOC_STATE_MACHINE;
			MsgElem->MsgType = MT2_MLME_DISASSOC_REQ;
			MsgElem->MsgLen = sizeof(MLME_DISASSOC_REQ_STRUCT);
			NdisMoveMemory(MsgElem->Msg, &DisReq, sizeof(MLME_DISASSOC_REQ_STRUCT));

			/* Prevent to connect AP again in STAMlmePeriodicExec*/
			pAd->MlmeAux.AutoReconnectSsidLen= 32;
			NdisZeroMemory(pAd->MlmeAux.AutoReconnectSsid, pAd->MlmeAux.AutoReconnectSsidLen);

			pAd->Mlme.CntlMachine.CurrState = CNTL_WAIT_OID_DISASSOC;
			MlmeDisassocReqAction(pAd, MsgElem);
			os_free_mem(NULL, MsgElem);
			}

			RtmpusecDelay(1000);
		}

#ifdef WPA_SUPPLICANT_SUPPORT
#ifndef NATIVE_WPA_SUPPLICANT_SUPPORT
		/* send wireless event to wpa_supplicant for infroming interface down.*/
		RtmpOSWrielessEventSend(pAd->net_dev, RT_WLAN_EVENT_CUSTOM, RT_INTERFACE_DOWN, NULL, NULL, 0);
#endif /* NATIVE_WPA_SUPPLICANT_SUPPORT */

		if (pAd->StaCfg.wpa_supplicant_info.pWpsProbeReqIe)
		{
			os_free_mem(NULL, pAd->StaCfg.wpa_supplicant_info.pWpsProbeReqIe);
			pAd->StaCfg.wpa_supplicant_info.pWpsProbeReqIe = NULL;
			pAd->StaCfg.wpa_supplicant_info.WpsProbeReqIeLen = 0;
		}

		if (pAd->StaCfg.wpa_supplicant_info.pWpaAssocIe)
		{
			os_free_mem(NULL, pAd->StaCfg.wpa_supplicant_info.pWpaAssocIe);
			pAd->StaCfg.wpa_supplicant_info.pWpaAssocIe = NULL;
			pAd->StaCfg.wpa_supplicant_info.WpaAssocIeLen = 0;
		}
#endif /* WPA_SUPPLICANT_SUPPORT */
	}
#endif /* CONFIG_STA_SUPPORT */
}




PNET_DEV RtmpPhyNetDevMainCreate(VOID *pAdSrc)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pAdSrc;
	PNET_DEV pDevNew;
	UINT32 MC_RowID = 0, IoctlIF = 0;
	char *dev_name;

#ifdef MULTIPLE_CARD_SUPPORT
	MC_RowID = pAd->MC_RowID;
#endif /* MULTIPLE_CARD_SUPPORT */
#ifdef HOSTAPD_SUPPORT
	IoctlIF = pAd->IoctlIF;
#endif /* HOSTAPD_SUPPORT */

	dev_name = get_dev_name_prefix(pAd, INT_MAIN);
	pDevNew = RtmpOSNetDevCreate((INT32)MC_RowID, (UINT32 *)&IoctlIF,
					INT_MAIN, 0, sizeof(struct mt_dev_priv), dev_name);

#ifdef HOSTAPD_SUPPORT
	pAd->IoctlIF = IoctlIF;
#endif /* HOSTAPD_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
    if (pAd->OpMode == OPMODE_AP)
	{
        BSS_STRUCT *pMbss;
		pMbss = &pAd->ApCfg.MBSSID[MAIN_MBSSID];
        ASSERT(pMbss);
		if (pMbss) {
			wdev_bcn_buf_init(pAd, &pMbss->bcn_buf);
		} else {
			DBGPRINT(RT_DEBUG_ERROR, ("%s():func_dev is NULL!\n", __FUNCTION__));
			return NULL;
		}
	}
#endif

	return pDevNew;
}


#ifdef CONFIG_STA_SUPPORT
#ifdef PROFILE_STORE
static void WriteConfToDatFile(RTMP_ADAPTER *pAd)
{
	char	*cfgData = 0, *offset = 0;
	RTMP_STRING *fileName = NULL, *pTempStr = NULL;
	RTMP_OS_FD file_r, file_w;
	RTMP_OS_FS_INFO osFSInfo;
	LONG rv, fileLen = 0;


	DBGPRINT(RT_DEBUG_TRACE, ("-----> WriteConfToDatFile\n"));

		fileName = STA_PROFILE_PATH;

	RtmpOSFSInfoChange(&osFSInfo, TRUE);

	file_r = RtmpOSFileOpen(fileName, O_RDONLY, 0);
	if (IS_FILE_OPEN_ERR(file_r))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("-->1) %s: Error opening file %s\n", __FUNCTION__, fileName));
		return;
	}
	else
	{
		char tempStr[64] = {0};
		while((rv = RtmpOSFileRead(file_r, tempStr, 64)) > 0)
		{
			fileLen += rv;
		}
		os_alloc_mem(NULL, (UCHAR **)&cfgData, fileLen);
		if (cfgData == NULL)
		{
			RtmpOSFileClose(file_r);
			DBGPRINT(RT_DEBUG_TRACE, ("CfgData mem alloc fail. (fileLen = %ld)\n", fileLen));
			goto out;
		}
		NdisZeroMemory(cfgData, fileLen);
		RtmpOSFileSeek(file_r, 0);
		rv = RtmpOSFileRead(file_r, (RTMP_STRING *)cfgData, fileLen);
		RtmpOSFileClose(file_r);
		if (rv != fileLen)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("CfgData mem alloc fail, fileLen = %ld\n", fileLen));
			goto ReadErr;
		}
	}

	file_w = RtmpOSFileOpen(fileName, O_WRONLY|O_TRUNC, 0);
	if (IS_FILE_OPEN_ERR(file_w))
	{
		goto WriteFileOpenErr;
	}
	else
	{
		offset = (PCHAR) rtstrstr((RTMP_STRING *) cfgData, "Default\n");
		offset += strlen("Default\n");
		RtmpOSFileWrite(file_w, (RTMP_STRING *)cfgData, (int)(offset-cfgData));
		os_alloc_mem(NULL, (UCHAR **)&pTempStr, 512);
		if (!pTempStr)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("pTempStr mem alloc fail. (512)\n"));
			RtmpOSFileClose(file_w);
			goto WriteErr;
		}

		for (;;)
		{
			int i = 0;
			RTMP_STRING *ptr;

			NdisZeroMemory(pTempStr, 512);
			ptr = (RTMP_STRING *) offset;
			while(*ptr && *ptr != '\n')
			{
				pTempStr[i++] = *ptr++;
			}
			pTempStr[i] = 0x00;
			if ((size_t)(offset - cfgData) < fileLen)
			{
				offset += strlen(pTempStr) + 1;
				if (strncmp(pTempStr, "SSID=", strlen("SSID=")) == 0)
				{
					NdisZeroMemory(pTempStr, 512);
					NdisMoveMemory(pTempStr, "SSID=", strlen("SSID="));
					NdisMoveMemory(pTempStr + 5, pAd->CommonCfg.Ssid, pAd->CommonCfg.SsidLen);
				}
				else if (strncmp(pTempStr, "AuthMode=", strlen("AuthMode=")) == 0)
				{
					NdisZeroMemory(pTempStr, 512);
					if (pAd->StaCfg.AuthMode == Ndis802_11AuthModeOpen)
						sprintf(pTempStr, "AuthMode=OPEN");
					else if (pAd->StaCfg.AuthMode == Ndis802_11AuthModeShared)
						sprintf(pTempStr, "AuthMode=SHARED");
					else if (pAd->StaCfg.AuthMode == Ndis802_11AuthModeAutoSwitch)
						sprintf(pTempStr, "AuthMode=WEPAUTO");
					else if (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPAPSK)
						sprintf(pTempStr, "AuthMode=WPAPSK");
					else if (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2PSK)
						sprintf(pTempStr, "AuthMode=WPA2PSK");
					else if (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA)
						sprintf(pTempStr, "AuthMode=WPA");
					else if (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2)
						sprintf(pTempStr, "AuthMode=WPA2");
					else if (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPANone)
						sprintf(pTempStr, "AuthMode=WPANONE");
				}
				else if (strncmp(pTempStr, "EncrypType=", strlen("EncrypType=")) == 0)
				{
					NdisZeroMemory(pTempStr, 512);
					if (pAd->StaCfg.WepStatus == Ndis802_11WEPDisabled)
						sprintf(pTempStr, "EncrypType=NONE");
					else if (pAd->StaCfg.WepStatus == Ndis802_11WEPEnabled)
						sprintf(pTempStr, "EncrypType=WEP");
					else if (pAd->StaCfg.WepStatus == Ndis802_11TKIPEnable)
						sprintf(pTempStr, "EncrypType=TKIP");
					else if (pAd->StaCfg.WepStatus == Ndis802_11AESEnable)
						sprintf(pTempStr, "EncrypType=AES");
				}
				RtmpOSFileWrite(file_w, pTempStr, strlen(pTempStr));
				RtmpOSFileWrite(file_w, "\n", 1);
			}
			else
			{
				break;
			}
		}
		RtmpOSFileClose(file_w);
	}

WriteErr:
	if (pTempStr)
		os_free_mem(NULL, pTempStr);
ReadErr:
WriteFileOpenErr:
	if (cfgData)
		os_free_mem(NULL, cfgData);
out:
	RtmpOSFSInfoChange(&osFSInfo, FALSE);


	DBGPRINT(RT_DEBUG_TRACE, ("<----- WriteConfToDatFile\n"));
	return;
}


INT write_dat_file_thread (
    IN ULONG Context)
{
	RTMP_OS_TASK *pTask;
	RTMP_ADAPTER *pAd;
	//int 	Status = 0;

	pTask = (RTMP_OS_TASK *)Context;

	if (pTask == NULL)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s: pTask is NULL\n", __FUNCTION__));
		return 0;
	}

	pAd = (PRTMP_ADAPTER)RTMP_OS_TASK_DATA_GET(pTask);

	if (pAd == NULL)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd is NULL\n", __FUNCTION__));
		return 0;
	}

	RtmpOSTaskCustomize(pTask);

	/* Update ssid, auth mode and encr type to DAT file */
	WriteConfToDatFile(pAd);

		RtmpOSTaskNotifyToExit(pTask);

	return 0;
}

NDIS_STATUS WriteDatThread(
	IN  RTMP_ADAPTER *pAd)
{
	NDIS_STATUS status = NDIS_STATUS_FAILURE;
	RTMP_OS_TASK *pTask;

	if (pAd->bWriteDat == FALSE)
		return 0;

	DBGPRINT(RT_DEBUG_TRACE, ("-->WriteDatThreadInit()\n"));

	pTask = &pAd->WriteDatTask;

	RTMP_OS_TASK_INIT(pTask, "RtmpWriteDatTask", pAd);
	status = RtmpOSTaskAttach(pTask, write_dat_file_thread, (ULONG)&pAd->WriteDatTask);
	DBGPRINT(RT_DEBUG_TRACE, ("<--WriteDatThreadInit(), status=%d!\n", status));

	return status;
}
#endif /* PROFILE_STORE */
#endif /* CONFIG_STA_SUPPORT */

