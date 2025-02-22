/****************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ****************************************************************************

    Module Name:
    connect.c

    Abstract:
    Routines to deal Link UP/DOWN and build/update BEACON frame contents

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    John Chang  08-04-2003    created for 11g soft-AP
 */

#include "rt_config.h"

UCHAR PowerConstraintIE[3] = {IE_POWER_CONSTRAINT, 1, 3};


/*
	==========================================================================
	Description:
		Used to check the necessary to send Beancon.
	return value
		0: mean no necessary.
		0: mean need to send Beacon for the service.
	==========================================================================
*/
BOOLEAN BeaconTransmitRequired(RTMP_ADAPTER *pAd, INT apidx, BSS_STRUCT *pMbss)
{
	BOOLEAN result = FALSE;
	BCN_BUF_STRUC *bcn_info;

	do
	{

#ifdef CARRIER_DETECTION_SUPPORT
		if (isCarrierDetectExist(pAd) == TRUE)
			break;
#endif /* CARRIER_DETECTION_SUPPORT */


		bcn_info = &pMbss->bcn_buf;
		if (bcn_info->BcnBufIdx >= HW_BEACON_MAX_NUM)
			break;

		if (apidx == MAIN_MBSSID)
		{
			if (bcn_info->bBcnSntReq == TRUE)
			{
				result = TRUE;
				break;
			}

		}
		else
		{
			if (bcn_info->bBcnSntReq == TRUE)
				result = TRUE;
		}
	}
	while (FALSE);

	return result;
}


#ifdef MT_MAC
VOID write_tmac_info_beacon(RTMP_ADAPTER *pAd, INT apidx, UCHAR *tmac_buf, HTTRANSMIT_SETTING *BeaconTransmit, ULONG frmLen)
{
	MAC_TX_INFO mac_info;

	NdisZeroMemory((UCHAR *)&mac_info, sizeof(mac_info));

	mac_info.FRAG = FALSE;
	mac_info.CFACK = FALSE;
	mac_info.InsTimestamp = TRUE;
	mac_info.AMPDU = FALSE;
	mac_info.BM = 1;
	mac_info.Ack = FALSE;
	mac_info.NSeq = TRUE;
	mac_info.BASize = 0;
	mac_info.WCID = 0;
	mac_info.Length = frmLen;
	mac_info.TID = 0;
	mac_info.TxRate = 0;
	mac_info.Txopmode = IFS_HTTXOP;
#ifdef MT_MAC
	mac_info.q_idx = Q_IDX_BCN;
	mac_info.TxSPriv = apidx;
#endif /* MT_MAC */
	mac_info.hdr_len = 24;
	mac_info.bss_idx = apidx;
	mac_info.SpeEn = 1;
	mac_info.Preamble = LONG_PREAMBLE;
	NdisZeroMemory(tmac_buf, sizeof(TMAC_TXD_L));
	write_tmac_info(pAd, tmac_buf, &mac_info, BeaconTransmit);
}


VOID asic_write_bcn_buf(RTMP_ADAPTER *pAd, UCHAR *tmac_info, INT info_len, UCHAR *bcn_buf, INT buf_len, UINT32 hw_addr)
{
	UCHAR *ptr;

#ifdef RT_BIG_ENDIAN
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP || pAd->chipCap.hif_type == HIF_RLT)
	{
	    RTMPWIEndianChange(pAd, tmac_info, TYPE_TXWI);
	}
#endif
#endif
	/* update BEACON frame content. start right after the TXWI field. */
	ptr = bcn_buf;
#ifdef RT_BIG_ENDIAN
	RTMPFrameEndianChange(pAd, ptr, DIR_WRITE, FALSE);
#endif

	// TODO: shiang-MT7603, Send to ASIC!

}
#endif /* MT_MAC */


#if defined(RTMP_MAC) || defined(RLT_MAC)
VOID write_tmac_info_beacon(RTMP_ADAPTER *pAd, INT apidx, UCHAR *tmac_buf, HTTRANSMIT_SETTING *BeaconTransmit, ULONG frmLen)
{
	MAC_TX_INFO mac_info;

	NdisZeroMemory((UCHAR *)&mac_info, sizeof(mac_info));

	mac_info.FRAG = FALSE;
	mac_info.CFACK = FALSE;
	mac_info.InsTimestamp = TRUE;
	mac_info.AMPDU = FALSE;
	mac_info.BM = 1;
	mac_info.Ack = FALSE;
	mac_info.NSeq = TRUE;
	mac_info.BASize = 0;
	mac_info.WCID = RESERVED_WCID;
	mac_info.Length = frmLen;
	mac_info.PID = PID_MGMT;
	mac_info.TID = 0;
	mac_info.TxRate = 0;
	mac_info.Txopmode = IFS_HTTXOP;
	mac_info.Preamble = LONG_PREAMBLE;
	write_tmac_info(pAd, tmac_buf, &mac_info, BeaconTransmit);

#ifdef SPECIFIC_TX_POWER_SUPPORT
	/* Specific Power for Long-Range Beacon */
	if ((pAd->ApCfg.MBSSID[apidx].TxPwrAdj != -1) &&
	    (BeaconTransmit->field.MODE == MODE_CCK))
	{
		UCHAR TxPwrAdj = 0;
		TXWI_STRUC *pTxWI = (TXWI_STRUC *)tmac_buf;

		TxPwrAdj = pAd->ApCfg.MBSSID[apidx].TxPwrAdj;
#ifdef RTMP_MAC
		if (pAd->chipCap.hif_type == HIF_RTMP)
      		  	pTxWI->TXWI_O.TxPwrAdj = TxPwrAdj;
#endif /* RTMP_MAC */
#ifdef RLT_MAC
		if (pAd->chipCap.hif_type == HIF_RLT)
			pTxWI->TXWI_N.TxPwrAdj = TxPwrAdj;
#endif /* RLT_MAC */
	}
#endif /* SPECIFIC_TX_POWER_SUPPORT */
}


VOID asic_write_bcn_buf(RTMP_ADAPTER *pAd, UCHAR *tmac_info, INT info_len, UCHAR *bcn_buf, INT buf_len, UINT32 hw_addr)
{
	INT i;
	UCHAR *ptr = tmac_info;
	UINT32 longValue, reg_base = hw_addr;

#ifdef RT_BIG_ENDIAN
	    RTMPWIEndianChange(pAd, tmac_info, TYPE_TXWI);
#endif

	for (i = 0; i < info_len; i += 4)
	{
		longValue = *ptr + (*(ptr+1)<<8) + (*(ptr+2)<<16) + (*(ptr+3)<<24);
		RTMP_CHIP_UPDATE_BEACON(pAd, reg_base + i, longValue, 4);
		ptr += 4;
	}

	/* update BEACON frame content. start right after the TXWI field. */
	ptr = bcn_buf;
#ifdef RT_BIG_ENDIAN
	RTMPFrameEndianChange(pAd, ptr, DIR_WRITE, FALSE);
#endif

	reg_base = hw_addr + info_len;
	for (i= 0; i< buf_len; i+=4)
	{
		longValue =  *ptr + (*(ptr+1)<<8) + (*(ptr+2)<<16) + (*(ptr+3)<<24);
		RTMP_CHIP_UPDATE_BEACON(pAd, reg_base + i, longValue, 4);
		ptr += 4;
	}

}
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */


/*
	==========================================================================
	Description:
		Pre-build a BEACON frame in the shared memory
	==========================================================================
*/
VOID APMakeBssBeacon(RTMP_ADAPTER *pAd, INT apidx)
{
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[apidx];
	UCHAR DsLen = 1, SsidLen;
	HEADER_802_11 BcnHdr;
	LARGE_INTEGER FakeTimestamp;
	ULONG FrameLen = 0;
	UCHAR *pBeaconFrame, *tmac_info;
	UINT i;
	HTTRANSMIT_SETTING BeaconTransmit = {.word = 0};   /* MGMT frame PHY rate setting when operatin at HT rate. */
	UCHAR PhyMode, SupRateLen;
	UINT8 TXWISize = pAd->chipCap.TXWISize;
	UINT8 tx_hw_hdr_len = pAd->chipCap.tx_hw_hdr_len;

	if(!BeaconTransmitRequired(pAd, apidx, pMbss))
		return;

	if (pMbss->bcn_buf.BeaconPkt == NULL) {
		DBGPRINT(RT_DEBUG_ERROR, ("%s():Invalid BeaconPkt for MBSS[%d]\n",
					__FUNCTION__, apidx));
		return;
	}

#ifdef MT_MAC
    if (pAd->chipCap.hif_type == HIF_MT) {
        return;
//Carter, 20140306 for MT7603, merge MakeAllBeacon into UpdateAllBeacon
    }
#endif
	tmac_info = (UCHAR *)GET_OS_PKT_DATAPTR(pMbss->bcn_buf.BeaconPkt);
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		pBeaconFrame = (UCHAR *)(tmac_info + tx_hw_hdr_len);
	}
	else
	{
		pBeaconFrame = (UCHAR *)(tmac_info + TXWISize);
	}

	PhyMode = pMbss->wdev.PhyMode;
	SsidLen = (pMbss->bHideSsid) ? 0 : pMbss->SsidLen;
	MgtMacHeaderInit(pAd, &BcnHdr, SUBTYPE_BEACON,
						0, BROADCAST_ADDR,
						pMbss->wdev.if_addr,
						pMbss->wdev.bssid);

	/* for update framelen to TxWI later. */
	SupRateLen = pAd->CommonCfg.SupRateLen;
	if (PhyMode == WMODE_B)
		SupRateLen = 4;

	MakeOutgoingFrame(pBeaconFrame,                  &FrameLen,
					sizeof(HEADER_802_11),           &BcnHdr,
					TIMESTAMP_LEN,                   &FakeTimestamp,
					2,                               &pAd->CommonCfg.BeaconPeriod,
					2,                               &pMbss->CapabilityInfo,
					1,                               &SsidIe,
					1,                               &SsidLen,
					SsidLen,                      pMbss->Ssid,
					1,                               &SupRateIe,
					1,                               &SupRateLen,
					SupRateLen,                pAd->CommonCfg.SupRate,
					1,                               &DsIe,
					1,                               &DsLen,
					1,                               &pAd->CommonCfg.Channel,
					END_OF_ARGS);

	if ((pAd->CommonCfg.ExtRateLen) && (PhyMode != WMODE_B))
	{
		ULONG TmpLen;
		MakeOutgoingFrame(pBeaconFrame+FrameLen,         &TmpLen,
						1,                               &ExtRateIe,
						1,                               &pAd->CommonCfg.ExtRateLen,
						pAd->CommonCfg.ExtRateLen,           pAd->CommonCfg.ExtRate,
						END_OF_ARGS);
		FrameLen += TmpLen;
	}


    /* add country IE, power constraint IE */
	if (pAd->CommonCfg.bCountryFlag)
	{
		ULONG TmpLen, TmpLen2=0;
		UCHAR *TmpFrame = NULL;
		UCHAR CountryIe = IE_COUNTRY;

		os_alloc_mem(NULL, (UCHAR **)&TmpFrame, 256);
		if (TmpFrame != NULL)
		{
			NdisZeroMemory(TmpFrame, 256);

			/* prepare channel information */
#ifdef EXT_BUILD_CHANNEL_LIST
			BuildBeaconChList(pAd, TmpFrame, &TmpLen2);
#else
			{
				UCHAR MaxTxPower = GetCuntryMaxTxPwr(pAd, pAd->CommonCfg.Channel);
				MakeOutgoingFrame(TmpFrame+TmpLen2,     &TmpLen,
									1,                 	&pAd->ChannelList[0].Channel,
									1,                 	&pAd->ChannelListNum,
									1,                 	&MaxTxPower,
									END_OF_ARGS);
				TmpLen2 += TmpLen;
			}
#endif /* EXT_BUILD_CHANNEL_LIST */


			/* need to do the padding bit check, and concatenate it */
			if ((TmpLen2%2) == 0)
			{
				UCHAR	TmpLen3 = TmpLen2+4;
				MakeOutgoingFrame(pBeaconFrame+FrameLen,&TmpLen,
				                  1,                 	&CountryIe,
				                  1,                 	&TmpLen3,
				                  3,                 	pAd->CommonCfg.CountryCode,
				                  TmpLen2+1,				TmpFrame,
				                  END_OF_ARGS);
			}
			else
			{
				UCHAR	TmpLen3 = TmpLen2+3;
				MakeOutgoingFrame(pBeaconFrame+FrameLen,&TmpLen,
				                  1,                 	&CountryIe,
				                  1,                 	&TmpLen3,
				                  3,                 	pAd->CommonCfg.CountryCode,
				                  TmpLen2,				TmpFrame,
				                  END_OF_ARGS);
			}
			FrameLen += TmpLen;

			os_free_mem(NULL, TmpFrame);
		}
		else
			DBGPRINT(RT_DEBUG_ERROR, ("%s: Allocate memory fail!!!\n", __FUNCTION__));
	}


#ifdef DOT11_N_SUPPORT
	/* AP Channel Report */
	{
		UCHAR APChannelReportIe = IE_AP_CHANNEL_REPORT;
		ULONG	TmpLen;

		/*
			802.11n D2.0 Annex J, USA regulatory
				class 32, channel set 1~7
				class 33, channel set 5-11
		*/
		UCHAR rclass32[]={32, 1, 2, 3, 4, 5, 6, 7};
        UCHAR rclass33[]={33, 5, 6, 7, 8, 9, 10, 11};
		UCHAR rclasslen = 8; /*sizeof(rclass32); */
		if (PhyMode == (WMODE_B | WMODE_G | WMODE_GN))
		{
			MakeOutgoingFrame(pBeaconFrame+FrameLen,&TmpLen,
							  1,                    &APChannelReportIe,
							  1,                    &rclasslen,
							  rclasslen,            rclass32,
   							  1,                    &APChannelReportIe,
							  1,                    &rclasslen,
							  rclasslen,            rclass33,
							  END_OF_ARGS);
			FrameLen += TmpLen;
		}
	}

#endif /* DOT11_N_SUPPORT */


	BeaconTransmit.word = 0;

	write_tmac_info_beacon(pAd, apidx, tmac_info, &BeaconTransmit, FrameLen);

	/* step 6. move BEACON TXD and frame content to on-chip memory */
	asic_write_bcn_buf(pAd,
						tmac_info, TXWISize,
						pBeaconFrame, FrameLen,
						pAd->BeaconOffset[pMbss->bcn_buf.BcnBufIdx]);

	pMbss->TimIELocationInBeacon = (UCHAR)FrameLen;
	pMbss->bcn_buf.cap_ie_pos = sizeof(HEADER_802_11) + TIMESTAMP_LEN + 2;

    //pMbss->bcn_buf.bcn_state = BCN_TX_IDLE;
//+++Add by shiang for debug
//---Add by shiang for debug
}


/*
	==========================================================================
	Description:
		Update the BEACON frame in the shared memory. Because TIM IE is variable
		length. other IEs after TIM has to shift and total frame length may change
		for each BEACON period.
	Output:
		pAd->ApCfg.MBSSID[apidx].CapabilityInfo
		pAd->ApCfg.ErpIeContent
	==========================================================================
*/
VOID APUpdateBeaconFrame(RTMP_ADAPTER *pAd, INT apidx)
{
	UCHAR *pBeaconFrame, *tmac_info;
	UCHAR *ptr;
	ULONG FrameLen = 0;
	ULONG UpdatePos;
	UCHAR RSNIe=IE_WPA, RSNIe2=IE_WPA2;
	UCHAR ID_1B, TimFirst, TimLast, *pTim;
	BSS_STRUCT *pMbss;
	COMMON_CONFIG *pComCfg;
	BOOLEAN bHasWpsIE = FALSE;
	UINT  i;
	HTTRANSMIT_SETTING	BeaconTransmit = {.word = 0};   /* MGMT frame PHY rate setting when operatin at Ht rate. */
	struct wifi_dev *wdev;
	UCHAR tx_hw_hdr_len = pAd->chipCap.tx_hw_hdr_len;
	UINT8 TXWISize = pAd->chipCap.TXWISize;

	UCHAR DsLen = 1, SsidLen;
	HEADER_802_11 BcnHdr;
	LARGE_INTEGER FakeTimestamp;
	UCHAR PhyMode = 0, SupRateLen;

	pComCfg = &pAd->CommonCfg;
	pMbss = &pAd->ApCfg.MBSSID[apidx];
	wdev = &pMbss->wdev;

	if (!pMbss || !pMbss->bcn_buf.BeaconPkt)
		return;

	tmac_info = (UCHAR *)GET_OS_PKT_DATAPTR(pMbss->bcn_buf.BeaconPkt);
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		pBeaconFrame = (UCHAR *)(tmac_info + tx_hw_hdr_len);
	}
	else
	{
		pBeaconFrame = (UCHAR *)(tmac_info + TXWISize);
	}

	if(!BeaconTransmitRequired(pAd, apidx, pMbss)) {
#ifdef BCN_OFFLOAD_SUPPORT
        RT28xx_UpdateBeaconToMcu(pAd, apidx, FALSE, 0, 0, 0);
        pMbss->updateEventIsTriggered = FALSE;
#endif /* BCN_OFFLOAD_SUPPORT */
		return;
    }


#ifdef CONFIG_FPGA_MODE
	if (pAd->fpga_ctl.fpga_on & 0x1) {
		if (pAd->fpga_ctl.tx_kick_cnt == 0)
			return;
	}

#ifdef MT_MAC
	if (pAd->fpga_ctl.no_bcn) {
		DBGPRINT(RT_DEBUG_OFF, ("%s():Bcn Tx is blocked!\n", __FUNCTION__));
		return;
	}
#endif /* MT_MAC */
#endif /* CONFIG_FPGA_MODE */

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT) {
		BOOLEAN is_pretbtt_int = FALSE;


	if (pMbss->bcn_buf.bcn_state != BCN_TX_IDLE) {
		if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) {
			DBGPRINT(RT_DEBUG_WARN, ("%s()=>BSS%d:BcnPkt not idle(%d)!\n",
							__FUNCTION__, apidx, pMbss->bcn_buf.bcn_state));
		}
			return;
	}

        PhyMode = pMbss->wdev.PhyMode;
        SsidLen = (pMbss->bHideSsid) ? 0 : pMbss->SsidLen;
        MgtMacHeaderInit(pAd, &BcnHdr, SUBTYPE_BEACON,
                            0, BROADCAST_ADDR,
                            pMbss->wdev.if_addr,
                            pMbss->wdev.bssid);

        /* for update framelen to TxWI later. */
        SupRateLen = pAd->CommonCfg.SupRateLen;
        if (PhyMode == WMODE_B)
            SupRateLen = 4;

        MakeOutgoingFrame(pBeaconFrame,                  &FrameLen,
                        sizeof(HEADER_802_11),           &BcnHdr,
                        TIMESTAMP_LEN,                   &FakeTimestamp,
                        2,                               &pAd->CommonCfg.BeaconPeriod,
                        2,                               &pMbss->CapabilityInfo,
                        1,                               &SsidIe,
                        1,                               &SsidLen,
                        SsidLen,                      pMbss->Ssid,
                        1,                               &SupRateIe,
                        1,                               &SupRateLen,
                        SupRateLen,                pAd->CommonCfg.SupRate,
                        1,                               &DsIe,
                        1,                               &DsLen,
                        1,                               &pAd->CommonCfg.Channel,
                        END_OF_ARGS);

        if ((pAd->CommonCfg.ExtRateLen) && (PhyMode != WMODE_B))
        {
            ULONG TmpLen;
            MakeOutgoingFrame(pBeaconFrame+FrameLen,         &TmpLen,
                            1,                               &ExtRateIe,
                            1,                               &pAd->CommonCfg.ExtRateLen,
                            pAd->CommonCfg.ExtRateLen,           pAd->CommonCfg.ExtRate,
                            END_OF_ARGS);
            FrameLen += TmpLen;
        }


        /* add country IE, power constraint IE */
        if (pAd->CommonCfg.bCountryFlag)
        {
            ULONG TmpLen, TmpLen2=0;
            UCHAR *TmpFrame = NULL;
            UCHAR CountryIe = IE_COUNTRY;

            os_alloc_mem(NULL, (UCHAR **)&TmpFrame, 256);
            if (TmpFrame != NULL)
            {
                NdisZeroMemory(TmpFrame, 256);

                /* prepare channel information */
#ifdef EXT_BUILD_CHANNEL_LIST
                BuildBeaconChList(pAd, TmpFrame, &TmpLen2);
#else
                {
                    UCHAR MaxTxPower = GetCuntryMaxTxPwr(pAd, pAd->CommonCfg.Channel);
                    MakeOutgoingFrame(TmpFrame+TmpLen2,     &TmpLen,
                                        1,                 	&pAd->ChannelList[0].Channel,
                                        1,                 	&pAd->ChannelListNum,
                                        1,                 	&MaxTxPower,
                                        END_OF_ARGS);
                    TmpLen2 += TmpLen;
                }
#endif /* EXT_BUILD_CHANNEL_LIST */


                /* need to do the padding bit check, and concatenate it */
                if ((TmpLen2%2) == 0)
                {
                    UCHAR	TmpLen3 = TmpLen2+4;
                    MakeOutgoingFrame(pBeaconFrame+FrameLen,&TmpLen,
                                    1,                 	&CountryIe,
                                    1,                 	&TmpLen3,
                                    3,                 	pAd->CommonCfg.CountryCode,
                                    TmpLen2+1,				TmpFrame,
                                    END_OF_ARGS);
                }
                else
                {
                    UCHAR	TmpLen3 = TmpLen2+3;
                    MakeOutgoingFrame(pBeaconFrame+FrameLen,&TmpLen,
                                    1,                 	&CountryIe,
                                    1,                 	&TmpLen3,
                                    3,                 	pAd->CommonCfg.CountryCode,
                                    TmpLen2,				TmpFrame,
                                    END_OF_ARGS);
                }
                FrameLen += TmpLen;

                os_free_mem(NULL, TmpFrame);
            }
            else
                DBGPRINT(RT_DEBUG_ERROR, ("%s: Allocate memory fail!!!\n", __FUNCTION__));
        }


#ifdef DOT11_N_SUPPORT
        /* AP Channel Report */
        {
            UCHAR APChannelReportIe = IE_AP_CHANNEL_REPORT;
            ULONG	TmpLen;

            /*
                802.11n D2.0 Annex J, USA regulatory
                    class 32, channel set 1~7
                    class 33, channel set 5-11
            */
            UCHAR rclass32[]={32, 1, 2, 3, 4, 5, 6, 7};
            UCHAR rclass33[]={33, 5, 6, 7, 8, 9, 10, 11};
            UCHAR rclasslen = 8; /*sizeof(rclass32); */
            if (PhyMode == (WMODE_B | WMODE_G | WMODE_GN))
            {
                MakeOutgoingFrame(pBeaconFrame+FrameLen,&TmpLen,
                                1,                    &APChannelReportIe,
                                1,                    &rclasslen,
                                rclasslen,            rclass32,
                                1,                    &APChannelReportIe,
                                1,                    &rclasslen,
                                rclasslen,            rclass33,
                                END_OF_ARGS);
                FrameLen += TmpLen;
            }
        }
#endif /* DOT11_N_SUPPORT */


        BeaconTransmit.word = 0;

        //write_tmac_info_beacon(pAd, apidx, tmac_info, &BeaconTransmit, FrameLen);

        pMbss->TimIELocationInBeacon = (UCHAR)FrameLen;
        pMbss->bcn_buf.cap_ie_pos = sizeof(HEADER_802_11) + TIMESTAMP_LEN + 2;

        FrameLen = UpdatePos = pMbss->TimIELocationInBeacon;
        PhyMode = wdev->PhyMode;



	}
#endif /* MT_MAC */

	/*
		step 1 - update BEACON's Capability
	*/
	ptr = pBeaconFrame + pMbss->bcn_buf.cap_ie_pos;
	*ptr = (UCHAR)(pMbss->CapabilityInfo & 0x00ff);
	*(ptr+1) = (UCHAR)((pMbss->CapabilityInfo & 0xff00) >> 8);


    UINT32  mac_val = 0, bmc_cnt = 0;
    AsicSetBmcQCR(pAd, BMC_CNT_UPDATE, CR_READ, apidx, &mac_val);

    if ((apidx >= 0) && (apidx <= 4))
    {
        if (apidx == 0)
            bmc_cnt = mac_val & 0xf;
        else
            bmc_cnt = (mac_val >> (12+ (4*apidx))) & 0xf;
    }
    else if ((apidx >= 5) && (apidx <= 12))
    {
        bmc_cnt = (mac_val >> (4*(apidx-5))) & 0xf;
    }
    else if ((apidx >=13) && (apidx <= 15))
    {
        bmc_cnt = (mac_val >> (4*(apidx-13))) & 0xf;
    }

    if (bmc_cnt > 0)
    {
        WLAN_MR_TIM_BCMC_SET(apidx);
    }
    //else
    //{
        //WLAN_MR_TIM_BCMC_CLEAR(apidx);
    //}

	/*
		step 2 - update TIM IE
		TODO: enlarge TIM bitmap to support up to 64 STAs
		TODO: re-measure if RT2600 TBTT interrupt happens faster than BEACON sent out time
	*/
	ptr = pBeaconFrame + pMbss->TimIELocationInBeacon;
	*ptr = IE_TIM;
	*(ptr + 2) = pAd->ApCfg.DtimCount;
	*(ptr + 3) = pAd->ApCfg.DtimPeriod;

	/* find the smallest AID (PS mode) */
	TimFirst = 0; /* record first TIM byte != 0x00 */
	TimLast = 0;  /* record last  TIM byte != 0x00 */
	pTim = pMbss->TimBitmaps;

	for(ID_1B=0; ID_1B<WLAN_MAX_NUM_OF_TIM; ID_1B++)
	{
		/* get the TIM indicating PS packets for 8 stations */
		UCHAR tim_1B = pTim[ID_1B];

		if (ID_1B == 0)
			tim_1B &= 0xfe; /* skip bit0 bc/mc */

		if (tim_1B == 0)
			continue; /* find next 1B */

		if (TimFirst == 0)
			TimFirst = ID_1B;

		TimLast = ID_1B;
	}

	/* fill TIM content to beacon buffer */
	if (TimFirst & 0x01)
		TimFirst --; /* find the even offset byte */

	*(ptr + 1) = 3+(TimLast-TimFirst+1); /* TIM IE length */
	*(ptr + 4) = TimFirst;

	for(i=TimFirst; i<=TimLast; i++)
		*(ptr + 5 + i - TimFirst) = pTim[i];

	/* bit0 means backlogged mcast/bcast */
    if (pAd->ApCfg.DtimCount == 0)
		*(ptr + 4) |= (pMbss->TimBitmaps[WLAN_CT_TIM_BCMC_OFFSET] & 0x01);

	/* adjust BEACON length according to the new TIM */
	FrameLen += (2 + *(ptr+1));

	/* move RSN IE from below to here for Ralink Win7 v3.0.0.61 version parse beacon issue. */
	/* sync the order with BRCM's AP. */
	if ((wdev->AuthMode == Ndis802_11AuthModeWPA) ||
		(wdev->AuthMode == Ndis802_11AuthModeWPAPSK))
		RSNIe = IE_WPA;
	else if ((wdev->AuthMode == Ndis802_11AuthModeWPA2) ||
		(wdev->AuthMode == Ndis802_11AuthModeWPA2PSK))
		RSNIe = IE_WPA2;

	/* Append RSN_IE when  WPA OR WPAPSK, */
	if ((wdev->AuthMode == Ndis802_11AuthModeWPA1WPA2) ||
		(wdev->AuthMode == Ndis802_11AuthModeWPA1PSKWPA2PSK))
	{
		ULONG TmpLen;
		MakeOutgoingFrame(pBeaconFrame+FrameLen,        &TmpLen,
						  1,                            &RSNIe,
						  1,                            &pMbss->RSNIE_Len[0],
						  pMbss->RSNIE_Len[0],      pMbss->RSN_IE[0],
						  1,                            &RSNIe2,
						  1,                            &pMbss->RSNIE_Len[1],
						  pMbss->RSNIE_Len[1],      pMbss->RSN_IE[1],
						  END_OF_ARGS);
		FrameLen += TmpLen;
	}
	else if (wdev->AuthMode >= Ndis802_11AuthModeWPA)
	{
		ULONG TmpLen;
		{
			MakeOutgoingFrame(pBeaconFrame+FrameLen,        &TmpLen,
						  1,                            &RSNIe,
						  1,                            &pMbss->RSNIE_Len[0],
						  pMbss->RSNIE_Len[0],      pMbss->RSN_IE[0],
						  END_OF_ARGS);
			FrameLen += TmpLen;
		}
	}

#ifdef HOSTAPD_SUPPORT
	if (pMbss->HostapdWPS && (pMbss->WscIEBeacon.ValueLen))
		bHasWpsIE = TRUE;
#endif


	if (bHasWpsIE)
	{
		ULONG WscTmpLen = 0;

		MakeOutgoingFrame(pBeaconFrame+FrameLen, &WscTmpLen,
						pMbss->WscIEBeacon.ValueLen, pMbss->WscIEBeacon.Value,
						END_OF_ARGS);
		FrameLen += WscTmpLen;
	}



	/* Update ERP */
    if ((pComCfg->ExtRateLen) && (PhyMode != WMODE_B))
    {
        /* fill ERP IE */
        ptr = (UCHAR *)pBeaconFrame + FrameLen; /* pTxD->DataByteCnt; */
        *ptr = IE_ERP;
        *(ptr + 1) = 1;
        *(ptr + 2) = pAd->ApCfg.ErpIeContent;
		FrameLen += 3;
	}

#ifdef A_BAND_SUPPORT
	/* fill up Channel Switch Announcement Element */
	if ((pComCfg->Channel > 14)
		&& (pComCfg->bIEEE80211H == 1)
		&& (pAd->Dot11_H.RDMode == RD_SWITCHING_MODE))
	{
		ptr = pBeaconFrame + FrameLen;
		*ptr = IE_CHANNEL_SWITCH_ANNOUNCEMENT;
		*(ptr + 1) = 3;
		*(ptr + 2) = 1;
		*(ptr + 3) = pComCfg->Channel;
		*(ptr + 4) = (pAd->Dot11_H.CSPeriod - pAd->Dot11_H.CSCount - 1);
		ptr += 5;
		FrameLen += 5;

#ifdef DOT11_N_SUPPORT
		/* Extended Channel Switch Announcement Element */
		if (pComCfg->bExtChannelSwitchAnnouncement)
		{
			HT_EXT_CHANNEL_SWITCH_ANNOUNCEMENT_IE	HtExtChannelSwitchIe;
			build_ext_channel_switch_ie(pAd, &HtExtChannelSwitchIe);
			NdisMoveMemory(ptr, &HtExtChannelSwitchIe, sizeof(HT_EXT_CHANNEL_SWITCH_ANNOUNCEMENT_IE));
			ptr += sizeof(HT_EXT_CHANNEL_SWITCH_ANNOUNCEMENT_IE);
			FrameLen += sizeof(HT_EXT_CHANNEL_SWITCH_ANNOUNCEMENT_IE);
		}

#ifdef DOT11_VHT_AC
		if (WMODE_CAP_AC(PhyMode)) {
			INT tp_len, wb_len = 0;
			UCHAR *ch_sw_wrapper;
			VHT_TXPWR_ENV_IE txpwr_env;


			*ptr = IE_CH_SWITCH_WRAPPER;
			ch_sw_wrapper = (UCHAR *)(ptr + 1); // reserve for length
			ptr += 2; // skip len

			if (pComCfg->RegTransmitSetting.field.BW == BW_40) {
				WIDE_BW_CH_SWITCH_ELEMENT wb_info;

				*ptr = IE_WIDE_BW_CH_SWITCH;
				*(ptr + 1) = sizeof(WIDE_BW_CH_SWITCH_ELEMENT);
				ptr += 2;
				NdisZeroMemory(&wb_info, sizeof(WIDE_BW_CH_SWITCH_ELEMENT));
				if (pComCfg->vht_bw == VHT_BW_2040)
					wb_info.new_ch_width = 0;
				else
					wb_info.new_ch_width = 1;

				if (pComCfg->vht_bw == VHT_BW_80) {
					wb_info.center_freq_1 = vht_cent_ch_freq(pAd, pComCfg->Channel);
					wb_info.center_freq_2 = 0;
				}
				NdisMoveMemory(ptr, &wb_info, sizeof(WIDE_BW_CH_SWITCH_ELEMENT));
				wb_len = sizeof(WIDE_BW_CH_SWITCH_ELEMENT);
				ptr += wb_len;
				wb_len += 2;
			}

			*ptr = IE_VHT_TXPWR_ENV;
			NdisZeroMemory(&txpwr_env, sizeof(VHT_TXPWR_ENV_IE));
			tp_len = build_vht_txpwr_envelope(pAd, (UCHAR *)&txpwr_env);
			*(ptr + 1) = tp_len;
			ptr += 2;
			NdisMoveMemory(ptr, &txpwr_env, tp_len);
			ptr += tp_len;
			tp_len += 2;
			*ch_sw_wrapper = wb_len + tp_len;

			FrameLen += (2 + wb_len + tp_len);
		}
#endif /* DOT11_VHT_AC */

#endif /* DOT11_N_SUPPORT */
	}
#endif /* A_BAND_SUPPORT */

#ifdef DOT11_N_SUPPORT
	/* step 5. Update HT. Since some fields might change in the same BSS. */
	if (WMODE_CAP_N(PhyMode) && (wdev->DesiredHtPhyInfo.bHtEnable))
	{
		ULONG TmpLen;
		UCHAR HtLen, HtLen1;
		/*UCHAR i; */

		HT_CAPABILITY_IE HtCapabilityTmp;
#ifdef RT_BIG_ENDIAN
		ADD_HT_INFO_IE	addHTInfoTmp;
/*		USHORT	b2lTmp, b2lTmp2; // no use */
#endif

		/* add HT Capability IE */
		HtLen = sizeof(pComCfg->HtCapability);
		HtLen1 = sizeof(pComCfg->AddHTInfo);
#ifndef RT_BIG_ENDIAN
		NdisMoveMemory(&HtCapabilityTmp, &pComCfg->HtCapability, HtLen);
		HtCapabilityTmp.HtCapInfo.ChannelWidth = pComCfg->AddHTInfo.AddHtInfo.RecomWidth;

		MakeOutgoingFrame(pBeaconFrame+FrameLen,         &TmpLen,
								  1,                                &HtCapIe,
								  1,                                &HtLen,
								 HtLen,          &HtCapabilityTmp,
								  1,                                &AddHtInfoIe,
								  1,                                &HtLen1,
								 HtLen1,          &pComCfg->AddHTInfo,
						  END_OF_ARGS);
#else
		NdisMoveMemory(&HtCapabilityTmp, &pComCfg->HtCapability, HtLen);
		HtCapabilityTmp.HtCapInfo.ChannelWidth = pComCfg->AddHTInfo.AddHtInfo.RecomWidth;
		*(USHORT *)(&HtCapabilityTmp.HtCapInfo) = SWAP16(*(USHORT *)(&HtCapabilityTmp.HtCapInfo));
#ifdef UNALIGNMENT_SUPPORT
		{
			EXT_HT_CAP_INFO extHtCapInfo;

			NdisMoveMemory((PUCHAR)(&extHtCapInfo), (PUCHAR)(&HtCapabilityTmp.ExtHtCapInfo), sizeof(EXT_HT_CAP_INFO));
			*(USHORT *)(&extHtCapInfo) = cpu2le16(*(USHORT *)(&extHtCapInfo));
			NdisMoveMemory((PUCHAR)(&HtCapabilityTmp.ExtHtCapInfo), (PUCHAR)(&extHtCapInfo), sizeof(EXT_HT_CAP_INFO));
		}
#else
		*(USHORT *)(&HtCapabilityTmp.ExtHtCapInfo) = SWAP16(*(USHORT *)(&HtCapabilityTmp.ExtHtCapInfo));
#endif /* UNALIGNMENT_SUPPORT */

		NdisMoveMemory(&addHTInfoTmp, &pComCfg->AddHTInfo, HtLen1);
		*(USHORT *)(&addHTInfoTmp.AddHtInfo2) = SWAP16(*(USHORT *)(&addHTInfoTmp.AddHtInfo2));
		*(USHORT *)(&addHTInfoTmp.AddHtInfo3) = SWAP16(*(USHORT *)(&addHTInfoTmp.AddHtInfo3));

		MakeOutgoingFrame(pBeaconFrame+FrameLen,         &TmpLen,
								  1,                                &HtCapIe,
								  1,                                &HtLen,
								 HtLen,                   &HtCapabilityTmp,
								  1,                                &AddHtInfoIe,
								  1,                                &HtLen1,
								 HtLen1,                   &addHTInfoTmp,
						  END_OF_ARGS);
#endif
		FrameLen += TmpLen;

#ifdef DOT11N_DRAFT3
	 	/*
			P802.11n_D3.03, 7.3.2.60 Overlapping BSS Scan Parameters IE
		*/
	 	if ((pComCfg->Channel <= 14) &&
			(pComCfg->HtCapability.HtCapInfo.ChannelWidth == 1))
	 	{
			OVERLAP_BSS_SCAN_IE  OverlapScanParam;
			ULONG	TmpLen;
			UCHAR	OverlapScanIE, ScanIELen;

			OverlapScanIE = IE_OVERLAPBSS_SCAN_PARM;
			ScanIELen = 14;
			OverlapScanParam.ScanPassiveDwell = cpu2le16(pComCfg->Dot11OBssScanPassiveDwell);
			OverlapScanParam.ScanActiveDwell = cpu2le16(pComCfg->Dot11OBssScanActiveDwell);
			OverlapScanParam.TriggerScanInt = cpu2le16(pComCfg->Dot11BssWidthTriggerScanInt);
			OverlapScanParam.PassiveTalPerChannel = cpu2le16(pComCfg->Dot11OBssScanPassiveTotalPerChannel);
			OverlapScanParam.ActiveTalPerChannel = cpu2le16(pComCfg->Dot11OBssScanActiveTotalPerChannel);
			OverlapScanParam.DelayFactor = cpu2le16(pComCfg->Dot11BssWidthChanTranDelayFactor);
			OverlapScanParam.ScanActThre = cpu2le16(pComCfg->Dot11OBssScanActivityThre);

			MakeOutgoingFrame(pBeaconFrame + FrameLen, &TmpLen,
								1,			&OverlapScanIE,
								1,			&ScanIELen,
								ScanIELen,	&OverlapScanParam,
								END_OF_ARGS);

			FrameLen += TmpLen;
	 	}
#endif /* DOT11N_DRAFT3 */


#ifdef DOT11_VHT_AC
		if (WMODE_CAP_AC(PhyMode) && (pComCfg->Channel > 14))
		{
			int _len = build_vht_ies(pAd, (UCHAR *)(pBeaconFrame+FrameLen), SUBTYPE_BEACON);
			FrameLen += _len;
		}
#endif /* DOT11_VHT_AC */
	}
#endif /* DOT11_N_SUPPORT */

	/* 7.3.2.27 Extended Capabilities IE */
	{
		ULONG TmpLen, infoPos;
		PUCHAR pInfo;
		UCHAR extInfoLen;
		BOOLEAN	bNeedAppendExtIE = FALSE;
		EXT_CAP_INFO_ELEMENT	extCapInfo;


		extInfoLen = sizeof(EXT_CAP_INFO_ELEMENT);
		NdisZeroMemory(&extCapInfo, extInfoLen);

#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
		/* P802.11n_D1.10, HT Information Exchange Support */
		if (WMODE_CAP_N(PhyMode) && (pComCfg->Channel <= 14) &&
			(pMbss->wdev.DesiredHtPhyInfo.bHtEnable) &&
			(pComCfg->bBssCoexEnable == TRUE)
		)
		{
			extCapInfo.BssCoexistMgmtSupport = 1;
		}
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */



#ifdef DOT11_VHT_AC
		if (WMODE_CAP_AC(PhyMode) &&
			(pAd->CommonCfg.Channel > 14))
			extCapInfo.operating_mode_notification = 1;
#endif /* DOT11_VHT_AC */

		pInfo = (PUCHAR)(&extCapInfo);
		for (infoPos = 0; infoPos < extInfoLen; infoPos++)
		{
			if (pInfo[infoPos] != 0)
			{
				bNeedAppendExtIE = TRUE;
				break;
			}
		}

		if (bNeedAppendExtIE == TRUE)
		{
			MakeOutgoingFrame(pBeaconFrame+FrameLen, &TmpLen,
							1, &ExtCapIe,
							1, &extInfoLen,
							extInfoLen, &extCapInfo,
							END_OF_ARGS);
			FrameLen += TmpLen;
		}
	}

#ifdef WFA_VHT_PF
	if (pAd->force_vht_op_mode == TRUE)
	{
		ULONG TmpLen;
		UCHAR operating_ie = IE_OPERATING_MODE_NOTIFY, operating_len = 1;
		OPERATING_MODE operating_mode;

		operating_mode.rx_nss_type = 0;
		operating_mode.rx_nss = (pAd->vht_pf_op_ss - 1);
		operating_mode.ch_width = pAd->vht_pf_op_bw;

		MakeOutgoingFrame(pBeaconFrame+FrameLen, &TmpLen,
						  1,	&operating_ie,
						  1,	&operating_len,
						  1,	&operating_mode,
						  END_OF_ARGS);
		FrameLen += TmpLen;
	}
#endif /* WFA_VHT_PF */

	/* add WMM IE here */
	if (pMbss->wdev.bWmmCapable)
	{
		ULONG TmpLen;
		UCHAR i;
		UCHAR WmeParmIe[26] = {IE_VENDOR_SPECIFIC, 24, 0x00, 0x50, 0xf2, 0x02, 0x01, 0x01, 0, 0};
		UINT8 AIFSN[4];

		WmeParmIe[8] = pAd->ApCfg.BssEdcaParm.EdcaUpdateCount & 0x0f;

#ifdef UAPSD_SUPPORT
        UAPSD_MR_IE_FILL(WmeParmIe[8], &pMbss->wdev.UapsdInfo);
#endif /* UAPSD_SUPPORT */

		NdisMoveMemory(AIFSN, pAd->ApCfg.BssEdcaParm.Aifsn, sizeof(AIFSN));


		for (i=QID_AC_BE; i<=QID_AC_VO; i++)
		{
			WmeParmIe[10+ (i*4)] = (i << 5)                                         +     /* b5-6 is ACI */
								   ((UCHAR)pAd->ApCfg.BssEdcaParm.bACM[i] << 4)     +     /* b4 is ACM */
								   (AIFSN[i] & 0x0f);              /* b0-3 is AIFSN */
			WmeParmIe[11+ (i*4)] = (pAd->ApCfg.BssEdcaParm.Cwmax[i] << 4)           +     /* b5-8 is CWMAX */
								   (pAd->ApCfg.BssEdcaParm.Cwmin[i] & 0x0f);              /* b0-3 is CWMIN */
			WmeParmIe[12+ (i*4)] = (UCHAR)(pAd->ApCfg.BssEdcaParm.Txop[i] & 0xff);        /* low byte of TXOP */
			WmeParmIe[13+ (i*4)] = (UCHAR)(pAd->ApCfg.BssEdcaParm.Txop[i] >> 8);          /* high byte of TXOP */
		}

		MakeOutgoingFrame(pBeaconFrame+FrameLen,         &TmpLen,
						  26,                            WmeParmIe,
						  END_OF_ARGS);
		FrameLen += TmpLen;
	}

#ifdef AP_QLOAD_SUPPORT
	if (pAd->phy_ctrl.FlgQloadEnable != 0)
	{
		FrameLen += QBSS_LoadElementAppend(pAd, pBeaconFrame+FrameLen);
	}
#endif /* AP_QLOAD_SUPPORT */

#ifdef A_BAND_SUPPORT
	/*
		Only 802.11a APs that comply with 802.11h are required to include a
		Power Constrint Element(IE=32) in beacons and probe response frames
	*/
	if (((pComCfg->Channel > 14) && pComCfg->bIEEE80211H == TRUE)
		)
	{
		ULONG TmpLen;
		UINT8 PwrConstraintIE = IE_POWER_CONSTRAINT;
		UINT8 PwrConstraintLen = 1;
		UINT8 PwrConstraint = pComCfg->PwrConstraint;

		/* prepare power constraint IE */
		MakeOutgoingFrame(pBeaconFrame+FrameLen,	&TmpLen,
						1,							&PwrConstraintIE,
						1,							&PwrConstraintLen,
						1,							&PwrConstraint,
						END_OF_ARGS);
		FrameLen += TmpLen;

#ifdef DOT11_VHT_AC
		if (WMODE_CAP_AC(PhyMode)) {
			ULONG TmpLen;
			UINT8 vht_txpwr_env_ie = IE_VHT_TXPWR_ENV;
			UINT8 ie_len;
			VHT_TXPWR_ENV_IE txpwr_env;

			ie_len = build_vht_txpwr_envelope(pAd, (UCHAR *)&txpwr_env);
			MakeOutgoingFrame(pBeaconFrame+FrameLen, &TmpLen,
						1,							&vht_txpwr_env_ie,
						1,							&ie_len,
						ie_len,						&txpwr_env,
						END_OF_ARGS);
			FrameLen += TmpLen;
		}
#endif /* DOT11_VHT_AC */

	}
#endif /* A_BAND_SUPPORT */


#ifdef DOT11_N_SUPPORT
	if (WMODE_CAP_N(PhyMode) &&
		(wdev->DesiredHtPhyInfo.bHtEnable))
	{
		ULONG TmpLen;
		UCHAR HtLen, HtLen1;
#ifdef RT_BIG_ENDIAN
		HT_CAPABILITY_IE HtCapabilityTmp;
		ADD_HT_INFO_IE	addHTInfoTmp;
#endif
		/* add HT Capability IE */
		HtLen = sizeof(pComCfg->HtCapability);
		HtLen1 = sizeof(pComCfg->AddHTInfo);

		if (pAd->bBroadComHT == TRUE)
		{
			UCHAR epigram_ie_len;
			UCHAR BROADCOM_HTC[4] = {0x0, 0x90, 0x4c, 0x33};
			UCHAR BROADCOM_AHTINFO[4] = {0x0, 0x90, 0x4c, 0x34};


			epigram_ie_len = HtLen + 4;
#ifndef RT_BIG_ENDIAN
			MakeOutgoingFrame(pBeaconFrame + FrameLen,      &TmpLen,
						  1,                                &WpaIe,
						  1,                                &epigram_ie_len,
						  4,                                &BROADCOM_HTC[0],
						  HtLen,          					&pComCfg->HtCapability,
						  END_OF_ARGS);
#else
			NdisMoveMemory(&HtCapabilityTmp, &pComCfg->HtCapability, HtLen);
			*(USHORT *)(&HtCapabilityTmp.HtCapInfo) = SWAP16(*(USHORT *)(&HtCapabilityTmp.HtCapInfo));
#ifdef UNALIGNMENT_SUPPORT
		{
			EXT_HT_CAP_INFO extHtCapInfo;

			NdisMoveMemory((PUCHAR)(&extHtCapInfo), (PUCHAR)(&HtCapabilityTmp.ExtHtCapInfo), sizeof(EXT_HT_CAP_INFO));
			*(USHORT *)(&extHtCapInfo) = cpu2le16(*(USHORT *)(&extHtCapInfo));
			NdisMoveMemory((PUCHAR)(&HtCapabilityTmp.ExtHtCapInfo), (PUCHAR)(&extHtCapInfo), sizeof(EXT_HT_CAP_INFO));
		}
#else
			*(USHORT *)(&HtCapabilityTmp.ExtHtCapInfo) = SWAP16(*(USHORT *)(&HtCapabilityTmp.ExtHtCapInfo));
#endif /* UNALIGNMENT_SUPPORT */

			MakeOutgoingFrame(pBeaconFrame + FrameLen,       &TmpLen,
						1,                               &WpaIe,
						1,                               &epigram_ie_len,
						4,                               &BROADCOM_HTC[0],
						HtLen,                           &HtCapabilityTmp,
						END_OF_ARGS);
#endif

			FrameLen += TmpLen;

			epigram_ie_len = HtLen1 + 4;
#ifndef RT_BIG_ENDIAN
			MakeOutgoingFrame(pBeaconFrame + FrameLen,        &TmpLen,
						  1,                                &WpaIe,
						  1,                                &epigram_ie_len,
						  4,                                &BROADCOM_AHTINFO[0],
						  HtLen1, 							&pComCfg->AddHTInfo,
						  END_OF_ARGS);
#else
			NdisMoveMemory(&addHTInfoTmp, &pComCfg->AddHTInfo, HtLen1);
			*(USHORT *)(&addHTInfoTmp.AddHtInfo2) = SWAP16(*(USHORT *)(&addHTInfoTmp.AddHtInfo2));
			*(USHORT *)(&addHTInfoTmp.AddHtInfo3) = SWAP16(*(USHORT *)(&addHTInfoTmp.AddHtInfo3));

			MakeOutgoingFrame(pBeaconFrame + FrameLen,         &TmpLen,
							1,                             &WpaIe,
							1,                             &epigram_ie_len,
							4,                             &BROADCOM_AHTINFO[0],
							HtLen1,                        &addHTInfoTmp,
							END_OF_ARGS);
#endif
			FrameLen += TmpLen;
		}
	}
#endif /* DOT11_N_SUPPORT */

   	/* add Ralink-specific IE here - Byte0.b0=1 for aggregation, Byte0.b1=1 for piggy-back */
{
	ULONG TmpLen;
	UCHAR RalinkSpecificIe[9] = {IE_VENDOR_SPECIFIC, 7, 0x00, 0x0c, 0x43, 0x00, 0x00, 0x00, 0x00};

	if (pComCfg->bAggregationCapable)
		RalinkSpecificIe[5] |= 0x1;
	if (pComCfg->bPiggyBackCapable)
		RalinkSpecificIe[5] |= 0x2;
#ifdef DOT11_N_SUPPORT
	if (pComCfg->bRdg)
		RalinkSpecificIe[5] |= 0x4;
#endif /* DOT11_N_SUPPORT */
	MakeOutgoingFrame(pBeaconFrame+FrameLen, &TmpLen,
						9,                   RalinkSpecificIe,
						END_OF_ARGS);
	FrameLen += TmpLen;

}


	/* step 6. Since FrameLen may change, update TXWI. */
#ifdef A_BAND_SUPPORT
	if (pAd->CommonCfg.Channel > 14) {
		BeaconTransmit.field.MODE = MODE_OFDM;
		BeaconTransmit.field.MCS = MCS_RATE_6;
	}
#endif /* A_BAND_SUPPORT */

	write_tmac_info_beacon(pAd, apidx, tmac_info, &BeaconTransmit, FrameLen);

        /* step 6. move BEACON TXD and frame content to on-chip memory */
        asic_write_bcn_buf(pAd,
                            tmac_info, TXWISize,
                            pBeaconFrame, FrameLen,
                            pAd->BeaconOffset[pMbss->bcn_buf.BcnBufIdx]);

#if defined(MT7603_FPGA) || defined(MT7628_FPGA)
	// TODO: shiang-7603, we use different way to update beacon packet!
	if (0)//IS_MT7603(pAd))
	{
		hex_dump("Beacon_TMAC_INFO", (UCHAR *)tmac_info, tx_hw_hdr_len);
		dump_tmac_info(pAd, tmac_info);
		hex_dump("BeaconFrame", pBeaconFrame, FrameLen);
	}
#endif /* MT7603_FPGA */

	/* step 7. move BEACON TXWI and frame content to on-chip memory */
#ifdef BCN_OFFLOAD_SUPPORT
    if (pMbss->updateEventIsTriggered == FALSE) {
        RT28xx_UpdateBeaconToMcu(pAd, apidx, 0, TRUE, FrameLen, UpdatePos);
        pMbss->updateEventIsTriggered = TRUE;
    }
#endif
	RT28xx_UpdateBeaconToAsic(pAd, apidx, FrameLen, UpdatePos);

#ifdef DBG
	{
	    UINT32   Lowpart, Highpart;

	    AsicGetTsfTime(pAd, &Highpart, &Lowpart);
	    pMbss->WriteBcnDoneTime[pMbss->timer_loop] = Lowpart;
	}
#endif /* DBG */
}


/*
    ==========================================================================
    Description:
        Pre-build All BEACON frame in the shared memory
    ==========================================================================
*/
static UCHAR GetBcnNum(RTMP_ADAPTER *pAd)
{
	int i;
	int NumBcn;
	BCN_BUF_STRUC *bcn_info;

	NumBcn = 0;
	for (i=0; i<pAd->ApCfg.BssidNum; i++)
	{
		bcn_info = &pAd->ApCfg.MBSSID[i].bcn_buf;
		if (bcn_info->bBcnSntReq)
		{
			bcn_info->BcnBufIdx = NumBcn;
			NumBcn ++;
		}
	}


	return NumBcn;
}


VOID APMakeAllBssBeacon(RTMP_ADAPTER *pAd)
{
	INT i;
	UCHAR NumOfBcns;

	/* choose the Beacon number */
	NumOfBcns = GetBcnNum(pAd);

#if defined(RTMP_MAC) || defined(RLT_MAC)
	if ((pAd->chipCap.hif_type == HIF_RTMP) || (pAd->chipCap.hif_type == HIF_RLT)) {
		INT j;
		/*
			before MakeBssBeacon, clear all beacon TxD's valid bit

			Note: can not use MAX_MBSSID_NUM here, or
				1. when MBSS_SUPPORT is enabled;
				2. MAX_MBSSID_NUM will be 8;
				3. if HW_BEACON_OFFSET is 0x0200,
			we will overwrite other shared memory of chip.

			use pAd->ApCfg.BssidNum to avoid the case is best
		*/
		UINT8 TXWISize = pAd->chipCap.TXWISize;

		for (i=0; i<HW_BEACON_MAX_COUNT(pAd); i++)
		{
			for (j=0; j < TXWISize; j+=4)
			{
				RTMP_CHIP_UPDATE_BEACON(pAd, pAd->BeaconOffset[i] + j, 0, 4);
			}
		}
	}
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */

#ifdef RTMP_MAC_USB
	RTUSBBssBeaconStop(pAd);
#endif /* RTMP_MAC_USB */

	for(i=0; i<pAd->ApCfg.BssidNum; i++)
		APMakeBssBeacon(pAd, i);

	AsicSetMbssMode(pAd, NumOfBcns);

#ifdef RTMP_MAC_USB
#ifndef RT_CFG80211_SUPPORT 
	RTUSBBssBeaconStart(pAd);
#endif
#endif /* RTMP_MAC_USB */

}


/*
    ==========================================================================
    Description:
        Pre-build All BEACON frame in the shared memory
    ==========================================================================
*/
VOID APUpdateAllBeaconFrame(RTMP_ADAPTER *pAd)
{
	INT		i;
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
	BOOLEAN FlgQloadIsAlarmIssued = FALSE;
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */

	if (pAd->ApCfg.DtimCount == 0)
		pAd->ApCfg.DtimCount = pAd->ApCfg.DtimPeriod - 1;
	else
		pAd->ApCfg.DtimCount -= 1;
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
	/* QLOAD ALARM */
#ifdef AP_QLOAD_SUPPORT
	FlgQloadIsAlarmIssued = QBSS_LoadIsAlarmIssued(pAd);
#endif /* AP_QLOAD_SUPPORT */

	if ((pAd->ApCfg.DtimCount == 0) &&
		(((pAd->CommonCfg.Bss2040CoexistFlag & BSS_2040_COEXIST_INFO_SYNC) &&
		  (pAd->CommonCfg.bForty_Mhz_Intolerant == FALSE)) ||
		(FlgQloadIsAlarmIssued == TRUE)))
	{
		UCHAR	prevBW, prevExtChOffset;
		DBGPRINT(RT_DEBUG_TRACE, ("DTIM Period reached, BSS20WidthReq=%d, Intolerant40=%d!\n",
				pAd->CommonCfg.LastBSSCoexist2040.field.BSS20WidthReq, pAd->CommonCfg.LastBSSCoexist2040.field.Intolerant40));
		pAd->CommonCfg.Bss2040CoexistFlag &= (~BSS_2040_COEXIST_INFO_SYNC);

		prevBW = pAd->CommonCfg.AddHTInfo.AddHtInfo.RecomWidth;
		prevExtChOffset = pAd->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset;

		if (pAd->CommonCfg.LastBSSCoexist2040.field.BSS20WidthReq ||
			pAd->CommonCfg.LastBSSCoexist2040.field.Intolerant40 ||
			(pAd->MacTab.fAnyStaFortyIntolerant == TRUE) ||
			(FlgQloadIsAlarmIssued == TRUE))
		{
			pAd->CommonCfg.AddHTInfo.AddHtInfo.RecomWidth = 0;
			pAd->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset = 0;
		}
		else
		{
			pAd->CommonCfg.AddHTInfo.AddHtInfo.RecomWidth = pAd->CommonCfg.RegTransmitSetting.field.BW;
			pAd->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset = pAd->CommonCfg.RegTransmitSetting.field.EXTCHA;
		}
		DBGPRINT(RT_DEBUG_TRACE,("\tNow RecomWidth=%d, ExtChanOffset=%d, prevBW=%d, prevExtOffset=%d\n",
				pAd->CommonCfg.AddHTInfo.AddHtInfo.RecomWidth,
				pAd->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset,
				prevBW, prevExtChOffset));
		pAd->CommonCfg.Bss2040CoexistFlag |= BSS_2040_COEXIST_INFO_NOTIFY;
		pAd->CommonCfg.Bss2040CoexistFlag |= BSS_2040_COEXIST_BW_SYNC;
	}
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */

	for(i=0; i<pAd->ApCfg.BssidNum; i++)
	{
		APUpdateBeaconFrame(pAd, i);
	}
}

