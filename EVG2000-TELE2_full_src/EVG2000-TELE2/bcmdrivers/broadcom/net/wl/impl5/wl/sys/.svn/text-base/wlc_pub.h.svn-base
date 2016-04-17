/*
 * Common (OS-independent) definitions for
 * Broadcom 802.11abg Networking Device Driver
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: wlc_pub.h,v 1.293.2.41 2009/05/22 23:06:27 Exp $
 */

#ifndef _wlc_pub_h_
#define _wlc_pub_h_

/* enable structure packing */
#if defined(__GNUC__)
#define PACKED  __attribute__((packed))
#else
#define PACKED
#endif

/* Each DPT connection may dynamically allocate upto 2 timers */
#define	MAX_TIMERS	(27 + (2 * WLC_MAXDPT))		/* max # wl timers */

#define	WLC_NUMRATES	16	/* max # of rates in a rateset */
#define	MAXMULTILIST	32	/* max # multicast addresses */
#define	D11_PHY_HDR_LEN	6	/* Phy header length - 6 bytes */

#ifndef LINUX_WLUSER_POSTMOGRIFY_REMOVAL
/* phy types */
#define	PHY_TYPE_A	0	/* Phy type A */
#define	PHY_TYPE_G	2	/* Phy type G */
#define	PHY_TYPE_N	4	/* Phy type N */
#define	PHY_TYPE_LP	5	/* Phy type Low Power A/B/G */
#define	PHY_TYPE_SSN	6	/* Phy type Single Stream N */
#define	PHY_TYPE_QN	7	/* Phy type Quad N */
#define	PHY_TYPE_NULL	0xf	/* Invalid Phy value */

/* bw */
#define WLC_10_MHZ	10	/* 10Mhz nphy channel bandwidth */
#define WLC_20_MHZ	20	/* 20Mhz nphy channel bandwidth */
#define WLC_40_MHZ	40	/* 40Mhz nphy channel bandwidth */

#define CHSPEC_WLC_BW(chanspec)	(CHSPEC_IS40(chanspec) ? WLC_40_MHZ : \
				 CHSPEC_IS20(chanspec) ? WLC_20_MHZ : \
							 WLC_10_MHZ)
#endif /* LINUX_WLUSER_POSTMOGRIFY_REMOVAL */

#define	WLC_RSSI_MINVAL		-200	/* Low value, e.g. for forcing roam */
#define	WLC_RSSI_NO_SIGNAL	-91	/* NDIS RSSI link quality cutoffs */
#define	WLC_RSSI_VERY_LOW	-80	/* Very low quality cutoffs */
#define	WLC_RSSI_LOW		-70	/* Low quality cutoffs */
#define	WLC_RSSI_GOOD		-68	/* Good quality cutoffs */
#define	WLC_RSSI_VERY_GOOD	-58	/* Very good quality cutoffs */
#define	WLC_RSSI_EXCELLENT	-57	/* Excellent quality cutoffs */
#define	WLC_RSSI_INVALID	 0	/* invalid RSSI value */

#ifndef LINUX_POSTMOGRIFY_REMOVAL
#define WLC_PHYTYPE(_x) (_x) /* macro to perform WLC PHY -> D11 PHY TYPE, currently 1:1 */

#define MA_WINDOW_SZ		8	/* moving average window size */

/* a large TX Power as an init value to factor out of MIN() calculations,
 * keep low enough to fit in an int8, units are .25 dBm
 */
#define WLC_TXPWR_MAX		(127)	/* ~32 dBm = 1,500 mW */

/* Rx Antenna diversity control values */
#define	ANT_RX_DIV_FORCE_0		0	/* Use antenna 0 */
#define	ANT_RX_DIV_FORCE_1		1	/* Use antenna 1 */
#define	ANT_RX_DIV_START_1		2	/* Choose starting with 1 */
#define	ANT_RX_DIV_START_0		3	/* Choose starting with 0 */
#define	ANT_RX_DIV_ENABLE		3	/* APHY bbConfig Enable RX Diversity */
#define ANT_RX_DIV_DEF		ANT_RX_DIV_START_0	/* default antdiv setting */

/* Tx Antenna control values */
#define ANT_TX_FORCE_0			0	/* Tx on antenna 0, "Main" */
#define ANT_TX_FORCE_1			1	/* Tx on antenna 1, "Aux" */
#define ANT_TX_LAST_RX		3	/* Tx on phy's last good Rx antenna */
#define ANT_TX_DEF		3	/* driver's default tx antenna setting */

/* Tx Chain values */
#define TXCHAIN_DEF		0x1	/* def bitmap of txchain */
#define TXCHAIN_NPHY_DEF	0x3	/* default bitmap of tx chains for nphy */
#define RXCHAIN_DEF		0x1	/* def bitmap of rxchain */
#define RXCHAIN_NPHY_DEF	0x3	/* default bitmap of rx chains for nphy */
#define ANTSWITCH_NONE		0	/* no antenna switch */
#define ANTSWITCH_TYPE_1	1	/* antenna switch on 4321CB2, 2of3 */
#define ANTSWITCH_TYPE_2	2	/* antenna switch on 4321MPCI, 2of3 */
#define ANTSWITCH_TYPE_3	3	/* antenna switch on 4322, 2of3 */

#define ANTSEL_NA		0	/* No boardlevel selection available */
#define ANTSEL_2x4		1	/* 2x4 boardlevel selection available */
#define ANTSEL_2x3		2	/* 2x3 CB2 boardlevel selection available */

#define RXBUFSZ		PKTBUFSZ
#define AIDMAPSZ	(ROUNDUP(MAXSCB, NBBY)/NBBY)	/* aid bitmap size in bytes */
#endif /* LINUX_POSTMOGRIFY_REMOVAL */
typedef struct wlc_rateset {
	uint	count;			/* # rates in this set */
	uint8	rates[WLC_NUMRATES];	/* rates in 500kbps units w/hi bit set if basic */
	uint8	htphy_membership;	/* HT PHY Membership */
	uint8	mcs[MCSSET_LEN];	/* supported mcs index bit map */
} wlc_rateset_t;

struct rsn_parms {
	uint8 flags;		/* misc booleans (e.g., supported) */
	uint8 multicast;	/* multicast cipher */
	uint8 ucount;		/* count of unicast ciphers */
	uint8 unicast[4];	/* unicast ciphers */
	uint8 acount;		/* count of auth modes */
	uint8 auth[4];		/* Authentication modes */
};

#ifndef LINUX_POSTMOGRIFY_REMOVAL
/*
 * buffer length needed for wlc_format_ssid
 * 32 SSID chars, max of 4 chars for each SSID char "\xFF", plus NULL.
 */
#define SSID_FMT_BUF_LEN	((4 * DOT11_MAX_SSID_LEN) + 1)


#define RSN_FLAGS_SUPPORTED		0x1 /* Flag for rsn_params */
#ifdef BCMWPA2
#define RSN_FLAGS_PREAUTH		0x2 /* Flag for WPA2 rsn_params */
#endif /* BCMWPA2 */

/* All the HT-specific default advertised capabilities (including AMPDU)
 * should be grouped here at one place
 */
#define AMPDU_DEF_MPDU_DENSITY	6	/* default mpdu density (110 ==> 4us) */

/* defaults for the HT (MIMO) bss */
#define HT_CAP		((HT_CAP_MIMO_PS_OFF << HT_CAP_MIMO_PS_SHIFT) | HT_CAP_40MHZ | \
		HT_CAP_GF | HT_CAP_MAX_AMSDU | HT_CAP_DSSS_CCK)
#endif /* LINUX_POSTMOGRIFY_REMOVAL */

/* WLC packet type is a void * */
typedef void *wlc_pkt_t;

/* Event data type */
typedef struct wlc_event {
	wl_event_msg_t event;		/* encapsulated event */
	struct ether_addr *addr;	/* used to keep a trace of the potential present of
					 * an address in wlc_event_msg_t
					 */
	int bsscfgidx;			/* BSS config when needed */
	struct wl_if	*wlif;		/* pointer to wlif */
	void *data;                     /* used to hang additional data on an event */
	struct wlc_event *next;         /* enables ordered list of pending events */
} wlc_event_t;

/* wlc internal bss_info, wl external one is in wlioctl.h */
typedef struct wlc_bss_info
{
	struct ether_addr BSSID;	/* network BSSID */
	uint16		flags;		/* flags for internal attibutes */
	uint8		SSID_len;	/* the length of SSID */
	uint8		SSID[32];	/* SSID string */
	int16		RSSI;		/* receive signal strength (in dBm) */
	uint16		beacon_period;	/* units are Kusec */
	uint16		atim_window;	/* units are Kusec */
	chanspec_t	chanspec;	/* Channel num, bw, ctrl_sb and band */
	int8		infra;		/* 0=IBSS, 1=infrastructure, 2=unknown */
	wlc_rateset_t	rateset;	/* supported rates */
	uint8		dtim_period;	/* DTIM period */
	int8		phy_noise;	/* noise right after tx (in dBm) */
	uint16		capability;	/* Capability information */
#ifdef WLSCANCACHE
	uint32		timestamp;	/* in ms since boot, OSL_SYSUPTIME() */
#endif
	struct dot11_bcn_prb *bcn_prb;	/* beacon/probe response frame (ioctl na) */
	uint16		bcn_prb_len;	/* beacon/probe response frame length (ioctl na) */
	uint8		wme_qosinfo;	/* QoS Info from WME IE; valid if WLC_BSS_WME flag set */
	struct rsn_parms wpa;
#ifdef BCMWPA2
	struct rsn_parms wpa2;
#endif /* BCMWPA2 */
	uint16		qbss_load_aac;	/* qbss load available admission capacity */
	/* qbss_load_chan_free <- (0xff - channel_utilization of qbss_load_ie_t) */
	uint8		qbss_load_chan_free;	/* indicates how free the channel is */
	uint8		mcipher;	/* multicast cipher */
	uint8		wpacfg;		/* wpa config index */
} wlc_bss_info_t;

#ifndef LINUX_WLUSER_POSTMOGRIFY_REMOVAL
/* forward declarations */
struct wlc_if;

/* wlc_ioctl error codes */
#define WLC_ENOIOCTL	1 /* No such Ioctl */
#define WLC_EINVAL	2 /* Invalid value */
#define WLC_ETOOSMALL	3 /* Value too small */
#define WLC_ETOOBIG	4 /* Value too big */
#define WLC_ERANGE	5 /* Out of range */
#define WLC_EDOWN	6 /* Down */
#define WLC_EUP		7 /* Up */
#define WLC_ENOMEM	8 /* No Memory */
#define WLC_EBUSY	9 /* Busy */

/* IOVar flags for common error checks */
#define IOVF_WHL	(1<<4)	/* value must be whole (0-max) */
#define IOVF_NTRL	(1<<5)	/* value must be natural (1-max) */

#define IOVF_SET_UP	(1<<6)	/* set requires driver be up */
#define IOVF_SET_DOWN	(1<<7)	/* set requires driver be down */
#define IOVF_SET_CLK	(1<<8)	/* set requires core clock */
#define IOVF_SET_BAND	(1<<9)	/* set requires fixed band */

#define IOVF_GET_UP	(1<<10)	/* get requires driver be up */
#define IOVF_GET_DOWN	(1<<11)	/* get requires driver be down */
#define IOVF_GET_CLK	(1<<12)	/* get requires core clock */
#define IOVF_GET_BAND	(1<<13)	/* get requires fixed band */
#define IOVF_OPEN_ALLOW	(1<<14)	/* set allowed iovar for opensrc */

#define IOVF_BMAC_IOVAR	(1<<15) /* flag for BMAC iovars */

/* watchdog down and dump callback function proto's */
typedef int (*watchdog_fn_t)(void *handle);
typedef int (*down_fn_t)(void *handle);
typedef int (*dump_fn_t)(void *handle, struct bcmstrbuf *b);
/* IOVar handler
 *
 * handle - a pointer value registered with the function
 * vi - iovar_info that was looked up
 * actionid - action ID, calculated by IOV_GVAL() and IOV_SVAL() based on varid.
 * name - the actual iovar name
 * params/plen - parameters and length for a get, input only.
 * arg/len - buffer and length for value to be set or retrieved, input or output.
 * vsize - value size, valid for integer type only.
 * wlcif - interface context (wlc_if pointer)
 *
 * All pointers may point into the same buffer.
 */
typedef int (*iovar_fn_t)(void *handle, const bcm_iovar_t *vi, uint32 actionid,
	const char *name, void *params, uint plen, void *arg, int alen,
	int vsize, struct wlc_if *wlcif);
#endif /* LINUX_WLUSER_POSTMOGRIFY_REMOVAL */

/*
 * Public portion of "common" os-independent state structure.
 * The wlc handle points at this.
 */
typedef struct wlc_pub {
#ifdef WLC_HIGH
	uint		unit;			/* device instance number */
	uint		corerev;		/* core revision */
	osl_t		*osh;			/* pointer to os handle */
	si_t		*sih;			/* SB handle (cookie for siutils calls) */
	char		*vars;			/* "environment" name=value */
	bool		up;			/* interface up and running */
	bool		hw_off;			/* HW is off */
#endif
	bool		hw_up;			/* one time hw up/down(from boot or hibernation) */
	bool		_piomode;		/* true if pio mode */ /* BMAC_NOTE: NEED In both */
#ifdef WLC_HIGH
	uint		_nbands;		/* # bands supported */
	uint		now;			/* # elapsed seconds */

	bool		promisc;		/* promiscuous destination address */
#if defined(MACOSX)
	bool		sendup_mgmt;	/* sendup mgmt per packet filter setting */
#endif
	bool		delayed_down;		/* down delayed */
#ifdef TOE
	bool		_toe;			/* TOE mode enabled */
#endif
#ifdef ARPOE
	bool		_arpoe;			/* Arp agent offload enabled */
#endif
	bool		_ap;			/* AP mode enabled */
	bool		_apsta;			/* simultaneous AP/STA mode enabled */
	bool		_assoc_recreate;	/* association recreation on up transitions */
	int		_wme;			/* WME QoS mode */
	uint8		_mbss;			/* MBSS mode on */
	bool		allmulti;		/* enable all multicasts */
	bool		BSS;			/* infrastructure or ad hoc */
	bool		associated;		/* true:part of [I]BSS, false: not */
						/* (union of sta_associated, aps_associated) */
	bool		phytest_on;		/* whether a PHY test is running */
	bool		bf_preempt;		/* True to enable 'darwin' mode */
	uint		txqstopped;		/* tx flow control on */

#ifdef WOWL
	bool		_wowl;			/* wowl enabled or not (in sw) */
	bool		_wowl_active;		/* Is Wake mode actually active
						 * (used during transition)
						 */
#endif
#ifdef WLAMPDU
	bool		_ampdu;			/* ampdu enabled or not */
#endif
#ifdef WLAMSDU
	bool		_amsdu_tx;		/* true if currently amsdu agg is enabled */
#endif
#ifdef WLBA
	bool		_ba;			/* block ack enabled or not */
#endif
	bool		_cac;			/* 802.11e CAC enabled */
#ifdef CRAM
	bool		_cram;			/* enable: cram small tcp packets */
#endif
	int		_n_enab;		/* N support enable(1)/disable(0) */
	int		_n_reqd;		/* N support required for clients */
	int8		_coex;			/* 20/40 MHz BSS Management AUTO, ENAB, DISABLE */

	bool		_priofc;		/* Priority based flowcontrol */

	struct ether_addr	cur_etheraddr;	/* our local ethernet address */
	struct ether_addr	multicast[MAXMULTILIST]; /* multicast addresses */
	uint		nmulticast;		/* # enabled multicast addresses */
#ifdef BCMWPA2
	pmkid_cand_t	pmkid_cand[MAXPMKID];	/* PMKID candidate list */
	uint		npmkid_cand;		/* num PMKID candidates */
	pmkid_t		pmkid[MAXPMKID];	/* PMKID cache */
	uint		npmkid;			/* num cached PMKIDs */
#endif /* BCMWPA2 */

	wlc_bss_info_t	current_bss;		/* STA BSS if active, else first AP BSS */

	uint32		wlfeatureflag;		/* Flags to control sw features from registry */
	int		psq_pkts_total;		/* total num of ps pkts */

#ifdef WLLED
	uint		_activity;		/* Packet activity counter for LED module */
#endif

	uint16		txmaxpkts;		/* max number of large pkts allowed to be pending */

	/* Regulatory power limits */
	int8		txpwr_local_max;	/* regulatory local txpwr max */
	uint8		txpwr_local_constraint;	/* local power contraint in dB */

	/* s/w decryption counters */
	uint32		swdecrypt;		/* s/w decrypt attempts */

	int 		bcmerror;		/* last bcm error */

	mbool		radio_disabled;		/* bit vector for radio disabled reasons */
#if defined(WME_PER_AC_TX_PARAMS)
	bool		_per_ac_maxrate;	/* Max TX rate per AC */
#endif

#ifdef DSLCPE
#ifdef DSLCPE_DELAY
	uint		delaymode;		/* watchdog/delay mode */
#endif /* DSLCPE_DELAY */
#ifdef DSLCPE_BLOG
	bool		fcache;			/* 1-- enable; 0--disable */
#endif /* DSLCPE_BLOG */
	uint		assoc_count;		/* total number of associated sta */
	bool 		selectivedisgard;	/* selective disgard on prec queues */
#endif	/* DSLCPE */

#ifdef STA
	uint16		roam_time_thresh;	/* Max. # secs. of not hearing beacons
						 * before roaming.
						 */
	bool		align_wd_tbtt;		/* Align watchdog with tbtt indication
						 * handling. This flag is cleared by default
						 * and is set by per port code explicitly and
						 * you need to make sure the OSL_SYSUPTIME()
						 * is implemented properly in osl of that port
						 * when it enables this Power Save feature.
						 */
#endif /* STA */
#ifdef DSLCPE_SDIO
		uint		sdiod_drive_strength;	/* SDIO drive strength */
#endif /* BCMSDIO */
	uint16	bt_period;		/* bt coex period. read from shm. */

	uint16		boardrev;		/* version # of particular board */
	uint8		sromrev;		/* version # of the srom */
	uint32		boardflags;		/* Board specific flags from srom */
	uint32		boardflags2;		/* More board flags if sromrev >= 4 */
#endif /* WLC_HIGH */
	uint8		antsel_type;		/* Type of boardlevel mimo antenna switch-logic
						 * 0 = N/A, 1 = 2x4 board, 2 = 2x3 CB2 board
						 */
	bool		antsel_avail;           /* Antenna selection availability (SROM based) */

	/* BMAC_NOTE: wlc_phy code requires this */
	uint32 		radar;			/* radar info: just on or off for now */

#ifdef WLCNT
	wl_cnt_t	_cnt;			/* monolithic counters struct */
	wl_wme_cnt_t	_wme_cnt;		/* Counters for WMM */
#endif /* WLCNT */

} wlc_pub_t;

/* wl_monitor rx status per packet */
typedef struct	wl_rxsts {
	uint	pkterror;		/* error flags per pkt */
	uint	phytype;		/* 802.11 A/B/G ... */
	uint	channel;		/* channel */
	uint	datarate;		/* rate in 500kbps */
	uint	antenna;		/* antenna pkts recieved on */
	uint	pktlength;		/* pkt length minus bcm phy hdr */
	uint32	mactime;		/* time stamp from mac, count per 1us */
	uint	sq;			/* signal quality */
	int32	signal;			/* in dbm */
	int32	noise;			/* in dbm */
	uint	preamble;		/* Unknown, short, long */
	uint	encoding;		/* Unknown, CCK, PBCC, OFDM */
	uint	nfrmtype;		/* special 802.11n frames(AMPDU, AMSDU) */
} wl_rxsts_t;

#ifndef LINUX_POSTMOGRIFY_REMOVAL
/* status per error RX pkt */
#define WL_RXS_CRC_ERROR		0x00000001 /* CRC Error in packet */
#define WL_RXS_RUNT_ERROR		0x00000002 /* Runt packet */
#define WL_RXS_ALIGN_ERROR		0x00000004 /* Misaligned packet */
#define WL_RXS_OVERSIZE_ERROR		0x00000008 /* packet bigger than RX_LENGTH (usually 1518) */
#define WL_RXS_WEP_ICV_ERROR		0x00000010 /* Integrity Check Value error */
#define WL_RXS_WEP_ENCRYPTED		0x00000020 /* Encrypted with WEP */
#define WL_RXS_PLCP_SHORT		0x00000040 /* Short PLCP error */
#define WL_RXS_DECRYPT_ERR		0x00000080 /* Decryption error */
#define WL_RXS_OTHER_ERR		0x80000000 /* Other errors */

/* phy type */
#define WL_RXS_PHY_A			0x00000000 /* A phy type */
#define WL_RXS_PHY_B			0x00000001 /* B phy type */
#define WL_RXS_PHY_G			0x00000002 /* G phy type */
#define WL_RXS_PHY_N			0x00000004 /* N phy type */

/* encoding */
#define WL_RXS_ENCODING_CCK		0x00000000 /* CCK encoding */
#define WL_RXS_ENCODING_OFDM		0x00000001 /* OFDM encoding */

/* preamble */
#define WL_RXS_UNUSED_STUB		0x0		/* stub to match with wlc_ethereal.h */
#define WL_RXS_PREAMBLE_SHORT		0x00000001	/* Short preamble */
#define WL_RXS_PREAMBLE_LONG		0x00000002	/* Long preamble */
#define WL_RXS_PREAMBLE_MIMO_MM		0x00000003	/* MIMO mixed mode preamble */
#define WL_RXS_PREAMBLE_MIMO_GF		0x00000004	/* MIMO green field preamble */

#define WL_RXS_NFRM_AMPDU_FIRST		0x00000001 /* first MPDU in A-MPDU */
#define WL_RXS_NFRM_AMPDU_SUB		0x00000002 /* subsequent MPDU(s) in A-MPDU */
#define WL_RXS_NFRM_AMSDU_FIRST		0x00000004 /* first MSDU in A-MSDU */
#define WL_RXS_NFRM_AMSDU_SUB		0x00000008 /* subsequent MSDU(s) in A-MSDU */
#endif /* LINUX_POSTMOGRIFY_REMOVAL */

/* forward declare and use the struct notation so we don't have to
 * have it defined if not necessary.
 */
struct wlc_info;
struct wlc_if;

#ifndef LINUX_POSTMOGRIFY_REMOVAL
/* Structure for Pkttag area in a packet.
 * CAUTION: Pls carefully consider your design before adding any new fields to the pkttag
 * The size is limited to 32 bytes which on 64-bit machine allows only 4 fields
 * If adding a member, be sure to check if WLPKTTAG_INFO_MOVE should transfer it.
 */
typedef struct {
	uint32		flags;		/* Describe various packet properties */
	uint16		seq;		/* preassigned seqnum for AMPDU */
#ifdef BCMDBG
	uint16		_scb_dbgid;	/* Uniqe debug id for the scb pointed by scb pointer */
#endif
	uint8		callbackidx;	/* Index into pkt_callback tables for callback function */
	int8		_bsscfgidx;	/* Index of bsscfg for this frame */
	int8		tx_fifo;	/* fifo on which this frame should go */
	int8		rssi;		/* RSSI for the recvd. packet */
	uint32		exptime;	/* Time of expiry for the packet */
	struct scb*	_scb;		/* Pointer to SCB for associated ea */
	uint32		rspec;		/* Phy rate for received packet */
	uint32		packetid;	/* Packet ID */
} wlc_pkttag_t;

#define WLPKTTAG(p) ((wlc_pkttag_t*)PKTTAG(p))

/* Flags used in wlc_pkttag_t.
 * If adding a flag, be sure to check if WLPKTTAG_INFO_MOVE should transfer it.
 */
#define WLF_PSMARK		0x00000001	/* PKT marking for PSQ ageing */
#define WLF_PSDONTQ		0x00000002	/* PS-Poll response don't queue flag */
#define WLF_MPDU		0x00000004	/* Set if pkt is a PDU as opposed to MSDU */
#define WLF_NON8023		0x00000008	/* original pkt is not 8023 */
#define WLF_8021X		0x00000010	/* original pkt is 802.1x */
#define WLF_APSD		0x00000020	/* APSD delivery frame */
#define WLF_AMSDU		0x00000040	/* pkt is aggregated msdu */
#define WLF_HWAMSDU		0x00000080	/* HW/ucode has deaggregated this A-MSDU */
#define WLF_BA			0x00000100	/* Used by BA module if pkt is not to be acked */
#define WLF_EXPTIME		0x00000200	/* pkttag has a valid expiration time for the pkt */
#define WLF_AMPDU_MPDU		0x00000400	/* mpdu in a ampdu */
#define WLF_MIMO		0x00000800	/* mpdu has a mimo rate */
#define WLF_RIFS		0x00001000	/* frameburst with RIFS seperated */
#define WLF_VRATE_PROBE		0x00002000	/* vertical rate probe mpdu */
#define WLF_BSS_DOWN		0x00004000	/* The BSS associated with the pkt has gone down */
#define WLF_UMAC_PKT		0x00008000	/* UMAC Packet */
#define WLF_EXEMPT_MASK		0x00030000	/* mask for encryption exemption (Vista) */
#define WLF_WME_NOACK		0x00040000	/* pkt use WME No ACK policy */
#define WLF_DPT_TYPE		0x00800000	/* pkt is of DPT type */
#define WLF_DPT_DIRECT		0x01000000	/* pkt will use direct DPT path */
#define WLF_DPT_APPATH		0x02000000	/* pkt will use AP path */
#define WLF_RATE_AUTO		0x04000000	/* pkt uses rates from the rate selection module */
#define WLF_DATA		    0x08000000	/* pkt is pure data */
#define WLF_TX_PKT_IDX_SHIFT	21

#if defined(WLLMAC)

typedef struct {
	uint phy_type;
	uint8 channel;
	long rssi;
	int rate;
	uint8 numfrags;
	bool prepend_dot11_hdr;
	struct dot11_header	dot11_hdr;
}
#if !defined(BCMUSBDEV)
rx_ctxt_t;
#else
PACKED rx_ctxt_t;
#endif 

#endif 

/* Flags used in txqstopped */
#define TXQ_STOP_FOR_PRIOFC_MASK	0x000000FF	/* per prio flow control bits */
#define	TXQ_STOP_FOR_FLOW_CNTRL	0x00000100	/* stop txq enqueue for flow control */
#define	TXQ_STOP_FOR_PKT_DRAIN	0x00000200	/* stop txq enqueue for packet drain */

#ifdef WLBA
#define WLPKTFLAG_BA(pkttag)	((pkttag)->flags & WLF_BA)
#else
#define WLPKTFLAG_BA(pkttag)	FALSE
#endif

#ifdef WLAMPDU
#define WLPKTFLAG_AMPDU(pkttag)	((pkttag)->flags & WLF_AMPDU_MPDU)
#else
#define WLPKTFLAG_AMPDU(pkttag)	FALSE
#endif

#ifdef WLAMSDU
#define WLPKTFLAG_AMSDU(pkttag)	((pkttag)->flags & WLF_AMSDU)
#else
#define WLPKTFLAG_AMSDU(pkttag)	FALSE
#endif

#define WLPKTFLAG_PMF(pkttag)	FALSE

#define WLPKTFLAG_DPT(pkttag)	FALSE

#define WLPKTFLAG_RIFS(pkttag)	((pkttag)->flags & WLF_RIFS)

#define WLPKTFLAG_BSS_DOWN_GET(pkttag) ((pkttag)->flags & WLF_BSS_DOWN)
#define WLPKTFLAG_BSS_DOWN_SET(pkttag, val) (pkttag)->flags |= ((val) ? WLF_BSS_DOWN : 0)

#define WLPKTFLAG_EXEMPT_GET(pkttag) (((pkttag)->flags & WLF_EXEMPT_MASK) >> 16)
#define WLPKTFLAG_EXEMPT_SET(pkttag, val) ((pkttag)->flags = \
			((pkttag)->flags & ~WLF_EXEMPT_MASK) | (val << 16));
#define WLPKTFLAG_NOACK(pkttag)	((pkttag)->flags & WLF_WME_NOACK)

/* API for accessing BSSCFG index in WLPKTTAG */
#define BSSCFGIDX_ISVALID(bsscfgidx) (((bsscfgidx >= 0)&&(bsscfgidx < WLC_MAXBSSCFG)) ? 1 : 0)

static INLINE int8
wlc_pkttag_bsscfg_get(void *p)
{
	int8 idx = WLPKTTAG(p)->_bsscfgidx;
#ifdef BCMDBG
	ASSERT(BSSCFGIDX_ISVALID(idx));
#endif
	return idx;
}

#define WLPKTTAGBSSCFGGET(p) (wlc_pkttag_bsscfg_get(p))
#define WLPKTTAGBSSCFGSET(p, bsscfgidx) (WLPKTTAG(p)->_bsscfgidx = bsscfgidx)

/* Raw get of bss idx from pkt tag without error checking */
#define WLPKTTAG_BSSIDX_GET(pkttag) ((pkttag)->_bsscfgidx)
#endif /* LINUX_POSTMOGRIFY_REMOVAL */

/***********************************************
 * Feature-related macros to optimize out code *
 * *********************************************
 */

/* AP Support (versus STA) */
#if defined(AP) && !defined(STA)
#define	AP_ENAB(pub)	(1)
#elif !defined(AP) && defined(STA)
#define	AP_ENAB(pub)	(0)
#else /* both, need runtime check */
#define	AP_ENAB(pub)	((pub)->_ap)
#endif /* defined(AP) && !defined(STA) */

/* Macro to check if APSTA mode enabled */
#if defined(AP) && defined(STA)
#define APSTA_ENAB(pub)	((pub)->_apsta)
#else
#define APSTA_ENAB(pub)	(0)
#endif /* defined(AP) && defined(STA) */

/* Some useful combinations */
#define STA_ONLY(pub)	(!AP_ENAB(pub))
#define AP_ONLY(pub)	(AP_ENAB(pub) && !APSTA_ENAB(pub))

#ifndef LINUX_POSTMOGRIFY_REMOVAL
/* Primary MBSS enable check macro */

#if defined(MBSS)
#define MBSS_OFF		0
#define MBSS4_ENABLED	1
#define MBSS16_ENABLED	2
#define MBSS_ENAB(pub) ((pub)->_mbss)
#define MBSS_ENAB4(pub) ((pub)->_mbss == MBSS4_ENABLED)
#define MBSS_ENAB16(pub) ((pub)->_mbss == MBSS16_ENABLED)
#else /* !MBSS */
#define MBSS_ENAB(pub) (0)
#define MBSS_ENAB4(pub) (0)
#define MBSS_ENAB16(pub) (0)
#endif /* MBSS */

#if defined(WME_PER_AC_TX_PARAMS)
#define WME_PER_AC_TX_PARAMS_ENAB(pub) (1)
#define WME_PER_AC_MAXRATE_ENAB(pub) ((pub)->_per_ac_maxrate)
#else /* WME_PER_AC_TX_PARAMS */
#define WME_PER_AC_TX_PARAMS_ENAB(pub) (0)
#define WME_PER_AC_MAXRATE_ENAB(pub) (0)
#endif /* WME_PER_AC_TX_PARAMS */

/* WL11N Support */
#ifdef WL11N
#if ((defined(NCONF) && (NCONF != 0)) || (defined(SSLPNCONF) && (SSLPNCONF != 0)))
#define N_ENAB(pub) ((NCONF || SSLPNCONF) && (pub)->_n_enab)
#else
#define N_ENAB(pub) 0
#endif
#if ((defined(NCONF) && (NCONF != 0)) || (defined(SSLPNCONF) && (SSLPNCONF != 0)))
#define N_REQD(pub) ((NCONF || SSLPNCONF) && (pub)->_n_reqd)
#else
#define N_REQD(pub) 0
#endif
#else /* WL11N */
#define N_ENAB(pub) 0
#define N_REQD(pub) 0
#endif /* WL11N */

/* Block Ack Support */
#ifdef WLBA
#define WLBA_ENAB(pub) ((pub)->_ba)
#else
#define WLBA_ENAB(pub) 0
#endif /* WLBA */

/* CRAM support */
#ifdef CRAM
#define CRAM_ENAB(pub) ((pub)->_cram)
#else
#define CRAM_ENAB(pub) 0
#endif /* CRAM */

/* WLAMSDU Support */
#ifdef WLAMSDU
#define AMSDU_ENAB(pub) ((pub)->_amsdu_tx)
#else
#define AMSDU_ENAB(pub) 0
#endif /* WLAMSDU */

/* WLAMPDU Support */
#ifdef WLAMPDU
#define AMPDU_ENAB(pub) ((pub)->_ampdu)
#else
#define AMPDU_ENAB(pub) 0
#endif /* WLAMPDU */

/* WOWL support */
#ifdef WOWL
#define WOWL_ENAB(pub) ((pub)->_wowl)
#define WOWL_ACTIVE(pub) ((pub)->_wowl_active)
#else
#define WOWL_ACTIVE(wlc) (0)
#define WOWL_ENAB(pub) (0)
#endif /* WOWL */

/* WLDPT Support */
#define DPT_ENAB(pub) 0

/* PIO Mode Support */
#ifdef WLPIO
#define PIO_ENAB(pub) ((pub)->_piomode)
#else
#define PIO_ENAB(pub) 0
#endif /* WLPIO */

/* Call Admission Control support */
#ifdef WLCAC
#define CAC_ENAB(pub) ((pub)->_cac)
#else
#define CAC_ENAB(pub) 0
#endif

/* OBSS Coexistence support */
#ifdef WLCOEX
#define COEX_ENAB(pub) ((pub)->_coex != OFF)
#define COEX_ACTIVE(wlc) ((wlc)->obss.coex_enab)
#else
#define COEX_ACTIVE(wlc) 0
#define COEX_ENAB(pub) 0
#endif /* WLCOEX */

#ifdef BCMSPACE
#define	RXIQEST_ENAB(pub)	(1)
#else
#define	RXIQEST_ENAB(pub)	(0)
#endif

#define EDCF_ENAB(pub) (WME_ENAB(pub))

#define QOS_ENAB(pub) (WME_ENAB(pub) || N_ENAB(pub) || WLBA_ENAB(pub))

/* Priority based flowcontrol */
#define PRIOFC_ENAB(pub) ((pub)->_priofc)

#if defined(WL_MONITOR)
#define MONITOR_ENAB(wlc)	(wlc)->monitor
#else
#define MONITOR_ENAB(wlc)	(bcmspace && (wlc)->monitor)
#endif 
#define PROMISC_ENAB(wlc)	(bcmspace && (wlc)->promisc)

#if defined(MACOSX)
#define WLC_SENDUP_MGMT_ENAB(pub)	((pub)->sendup_mgmt)
#else
#define WLC_SENDUP_MGMT_ENAB(pub)	0
#endif

#ifdef TOE
#define TOE_ENAB(pub) ((pub)->_toe)
#else
#define TOE_ENAB(pub) 0
#endif

#ifdef ARPOE
#define ARPOE_ENAB(pub) ((pub)->_arpoe)
#else
#define ARPOE_ENAB(pub) 0
#endif

#ifdef PACKET_FILTER
#define PKT_FILTER_ENAB(pub) 	1
#else
#define PKT_FILTER_ENAB(pub)	0
#endif

#ifdef WL_ASSOC_RECREATE
#define ASSOC_RECREATE_ENAB(pub) ((pub)->_assoc_recreate)
#else
#define ASSOC_RECREATE_ENAB(pub) 0
#endif

/* PM2 Receive Throttle Duty Cycle */
#if defined(WL_PM2_RCV_DUR_LIMIT)
#define WLC_PM2_RCV_DUR_MIN		(10)	/* 10% of beacon interval */
#define WLC_PM2_RCV_DUR_MAX		(80)	/* 80% of beacon interval */
#define PM2_RCV_DUR_ENAB(wlc) ((wlc)->pm2_rcv_percent > 0)
#else
#define PM2_RCV_DUR_ENAB(wlc) 0
#endif /* WL_PM2_RCV_DUR_LIMIT */

/* Default PM2 Return to Sleep value, in ms */
#define PM2_SLEEP_RET_MS_DEFAULT 200

#ifdef WLLMAC
#define WL_LMAC_PKTAG_INFO_MOVE(pkt_from, pkt_to)	\
	do { \
		WLPKTTAG(pkt_to)->tx_fifo = WLPKTTAG(pkt_from)->tx_fifo; \
		WLPKTTAG(pkt_to)->packetid = WLPKTTAG(pkt_from)->packetid; \
	} while (0)
#else /* WLLMAC */
#define WL_LMAC_PKTAG_INFO_MOVE(pkt_from, pkt_to) 	do {} while (0)
#endif /* WLLMAC */

#define WLPKTTAG_FLAG_MOVE(pkt_from, pkt_to) \
	do { \
		/* transfer over AMSDU, AMPDU and EOSP flags */ \
		WLPKTTAG(pkt_to)->flags |= WLPKTTAG(pkt_from)->flags & \
			(WLF_AMSDU | WLF_APSD | WLF_AMPDU_MPDU | WLF_BSS_DOWN | \
			WLF_DPT_TYPE | WLF_DPT_DIRECT | WLF_DPT_APPATH | WLF_RATE_AUTO); \
	} while (0)

/* Move callback functions from a packet to another
 * CAUTION: This is destructive operation for pkt_from
 */

#define WLPKTTAG_INFO_MOVE(pkt_from, pkt_to) \
	do { \
		/* Make sure not moving to same packet! */ \
		ASSERT(pkt_from != pkt_to); \
		WLPKTTAG(pkt_to)->callbackidx = WLPKTTAG(pkt_from)->callbackidx; \
		WLPKTTAG(pkt_from)->callbackidx = 0; \
		WLPKTTAG(pkt_to)->_bsscfgidx = WLPKTTAG(pkt_from)->_bsscfgidx; \
		WLPKTTAG(pkt_to)->_scb = WLPKTTAG(pkt_from)->_scb; \
		WLPKTTAG(pkt_to)->seq = WLPKTTAG(pkt_from)->seq; \
		WLPKTTAG_FLAG_MOVE(pkt_from, pkt_to); \
		/* LMAC specific pkttag info */	\
		WL_LMAC_PKTAG_INFO_MOVE(pkt_from, pkt_to);	\
	} while (0)

#define WLPKTTAGSCB(p) (WLPKTTAG(p)->_scb)

#define	WLC_PREC_COUNT		16 /* Max precedence level implemented */

/* pri is PKTPRIO encoded in the packet. This maps the Packet priority to
 * enqueue precedence as defined in wlc_prec_map
 */
extern const uint8 wlc_prio2prec_map[];
#define WLC_PRIO_TO_PREC(pri)	wlc_prio2prec_map[(pri) & 7]

/* This maps priority to one precedence higher - Used by PS-Poll response packets to
 * simulate enqueue-at-head operation, but still maintain the order on the queue
 */
#define WLC_PRIO_TO_HI_PREC(pri)	MIN(WLC_PRIO_TO_PREC(pri) + 1, WLC_PREC_COUNT - 1)

extern const uint8 wme_fifo2ac[];
#define WME_PRIO2AC(prio)	wme_fifo2ac[prio2fifo[(prio)]]

#endif /* LINUX_POSTMOGRIFY_REMOVAL */

#ifndef LINUX_WLUSER_POSTMOGRIFY_REMOVAL
/* Mask to describe all precedence levels */
#define WLC_PREC_BMP_ALL		MAXBITVAL(WLC_PREC_COUNT)

/* Define a bitmap of precedences comprised by each AC */
#define WLC_PREC_BMP_AC_BE	(NBITVAL(WLC_PRIO_TO_PREC(PRIO_8021D_BE)) |	\
				NBITVAL(WLC_PRIO_TO_HI_PREC(PRIO_8021D_BE)) |	\
				NBITVAL(WLC_PRIO_TO_PREC(PRIO_8021D_EE)) |	\
				NBITVAL(WLC_PRIO_TO_HI_PREC(PRIO_8021D_EE)))
#define WLC_PREC_BMP_AC_BK	(NBITVAL(WLC_PRIO_TO_PREC(PRIO_8021D_BK)) |	\
				NBITVAL(WLC_PRIO_TO_HI_PREC(PRIO_8021D_BK)) |	\
				NBITVAL(WLC_PRIO_TO_PREC(PRIO_8021D_NONE)) |	\
				NBITVAL(WLC_PRIO_TO_HI_PREC(PRIO_8021D_NONE)))
#define WLC_PREC_BMP_AC_VI	(NBITVAL(WLC_PRIO_TO_PREC(PRIO_8021D_CL)) |	\
				NBITVAL(WLC_PRIO_TO_HI_PREC(PRIO_8021D_CL)) |	\
				NBITVAL(WLC_PRIO_TO_PREC(PRIO_8021D_VI)) |	\
				NBITVAL(WLC_PRIO_TO_HI_PREC(PRIO_8021D_VI)))
#define WLC_PREC_BMP_AC_VO	(NBITVAL(WLC_PRIO_TO_PREC(PRIO_8021D_VO)) |	\
				NBITVAL(WLC_PRIO_TO_HI_PREC(PRIO_8021D_VO)) |	\
				NBITVAL(WLC_PRIO_TO_PREC(PRIO_8021D_NC)) |	\
				NBITVAL(WLC_PRIO_TO_HI_PREC(PRIO_8021D_NC)))

#endif /* LINUX_WLUSER_POSTMOGRIFY_REMOVAL */

/* WME Support */
#ifdef WME
#define WME_ENAB(pub) ((pub)->_wme != OFF)
#define WME_AUTO(wlc) ((wlc)->pub._wme == AUTO)
#else
#define WME_ENAB(pub) 0
#define WME_AUTO(wlc) (0)
#endif /* WME */

#define WLC_USE_COREFLAGS	0xffffffff	/* invalid core flags, use the saved coreflags */

#ifdef WLCNT
#define WLC_UPDATE_STATS(wlc)	1	/* Stats support */
#define WLCNTINCR(a)		((a)++)	/* Increment by 1 */
#define WLCNTDECR(a)		((a)--)	/* Decrement by 1 */
#define WLCNTADD(a,delta)	((a) += (delta)) /* Increment by specified value */
#define WLCNTSET(a,value)	((a) = (value)) /* Set to specific value */
#define WLCNTVAL(a)		(a)	/* Return value */
#else /* WLCNT */
#define WLC_UPDATE_STATS(wlc)	0	/* No stats support */
#define WLCNTINCR(a)			/* No stats support */
#define WLCNTDECR(a)			/* No stats support */
#define WLCNTADD(a,delta)		/* No stats support */
#define WLCNTSET(a,value)		/* No stats support */
#define WLCNTVAL(a)		0	/* No stats support */
#endif /* WLCNT */

#ifndef LINUX_WLUSER_POSTMOGRIFY_REMOVAL
/* common functions for every port */
extern void * wlc_attach(void *wl, uint16 vendor, uint16 device, uint unit, bool piomode,
	osl_t *osh, void *regsva, uint bustype, void *btparam, uint *perr);
extern uint wlc_detach(struct wlc_info *wlc);
extern int  wlc_up(struct wlc_info *wlc);
extern uint wlc_down(struct wlc_info *wlc);

extern int wlc_set(struct wlc_info *wlc, int cmd, int arg);
extern int wlc_get(struct wlc_info *wlc, int cmd, int *arg);
extern int wlc_iovar_getint(struct wlc_info *wlc, const char *name, int *arg);
extern int wlc_iovar_setint(struct wlc_info *wlc, const char *name, int arg);
extern bool wlc_chipmatch(uint16 vendor, uint16 device);
extern void wlc_init(struct wlc_info *wlc);
extern void wlc_reset(struct wlc_info *wlc);

extern void wlc_intrson(struct wlc_info *wlc);
extern uint32 wlc_intrsoff(struct wlc_info *wlc);
extern void wlc_intrsrestore(struct wlc_info *wlc, uint32 macintmask);
extern bool wlc_intrsupd(struct wlc_info *wlc);
extern bool wlc_isr(struct wlc_info *wlc, bool *wantdpc);
extern bool wlc_dpc(struct wlc_info *wlc, bool bounded);
extern bool wlc_sendpkt(struct wlc_info *wlc, void *sdu, struct wlc_if *wlcif);
extern bool wlc_send80211_raw(struct wlc_info *wlc, void *p, uint ac);
extern int wlc_iovar_op(struct wlc_info *wlc, const char *name, void *params, int p_len, void *arg,
	int len, bool set, struct wlc_if *wlcif);
extern int wlc_ioctl(struct wlc_info *wlc, int cmd, void *arg, int len, struct wlc_if *wlcif);
/* helper functions */
extern void wlc_statsupd(struct wlc_info *wlc);
#endif /* LINUX_WLUSER_POSTMOGRIFY_REMOVAL */
#ifndef LINUX_POSTMOGRIFY_REMOVAL
/* common functions for every port */
extern void wlc_corereset(struct wlc_info *wlc, uint32 flags);
extern int wlc_bmac_up_prep(struct wlc_info *wlc);
extern int wlc_bmac_up_finish(struct wlc_info *wlc);
extern int wlc_bmac_down_prep(struct wlc_info *wlc);
extern int wlc_bmac_down_finish(struct wlc_info *wlc);

extern int wlc_nin_ioctl(struct wlc_info *wlc, int cmd, void *arg, int len, struct wlc_if *wlcif);
extern bool wlc_nin_process_sendup(struct wlc_info *wlc, void * p);
void wlc_nin_create_iapp_ind(struct wlc_info *wlc, void *p, int len);


extern uint32 wlc_reg_read(struct wlc_info *wlc, void *r, uint size);
extern void wlc_reg_write(struct wlc_info *wlc, void *r, uint32 v, uint size);
extern void wlc_mhf(struct wlc_info *wlc, uint8 idx, uint16 mask, uint16 val, int bands);
extern uint wlc_ctrupd(struct wlc_info *wlc, uint ucode_offset, uint offset);
extern uint32 wlc_delta_txfunfl(struct wlc_info *wlc, int fifo);
extern void wlc_rate_lookup_init(struct wlc_info *wlc, wlc_rateset_t *rateset);
extern void wlc_default_rateset(struct wlc_info *wlc, wlc_rateset_t *rs);
#ifdef STA
extern void wlc_join_attempt(struct wlc_info *wlc);
extern void wlc_join_bss_start(struct wlc_info *wlc);
extern void wlc_join_BSS(struct wlc_info *wlc, wlc_bss_info_t* bi);
#endif /* STA */

/* wlc_phy.c helper functions */
extern bool wlc_scan_inprog(struct wlc_info *wlc);
extern void *wlc_cur_phy(struct wlc_info *wlc);
extern void wlc_set_ps_ctrl(struct wlc_info *wlc);
extern void wlc_mctrl(struct wlc_info *wlc, uint32 mask, uint32 val);
extern void wlc_scb_ratesel_init_all(struct wlc_info *wlc);

/* ioctl */
extern int wlc_iovar_getint8(struct wlc_info *wlc, const char *name, int8 *arg);
extern int wlc_iovar_getbool(struct wlc_info *wlc, const char *name, bool *arg);
extern int wlc_iovar_check(wlc_pub_t *pub, const bcm_iovar_t *vi, void *arg, int len, bool set);
#endif /* LINUX_POSTMOGRIFY_REMOVAL */

#ifndef LINUX_WLUSER_POSTMOGRIFY_REMOVAL
extern int wlc_module_register(wlc_pub_t *pub, const bcm_iovar_t *iovars,
                               const char *name, void *hdl, iovar_fn_t iovar_fn,
                               watchdog_fn_t watchdog_fn, down_fn_t down_fn);
extern int wlc_module_unregister(wlc_pub_t *pub, const char *name, void *hdl);
#endif /* LINUX_WLUSER_POSTMOGRIFY_REMOVAL */

#ifndef LINUX_POSTMOGRIFY_REMOVAL
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
extern int wlc_iovar_dump(struct wlc_info *wlc, char *params, int p_len,
                          char *out_buf, int out_len);
#endif /* BCMDBG */
extern int wlc_dump_register(wlc_pub_t *pub, const char *name, dump_fn_t dump_fn,
                             void *dump_fn_arg);

extern uint wlc_txpktcnt(struct wlc_info *wlc);
extern void wlc_event_if(struct wlc_info *wlc, wlc_event_t *e, const struct ether_addr *addr);
extern void wlc_suspend_mac_and_wait(struct wlc_info *wlc);
extern void wlc_enable_mac(struct wlc_info *wlc);
extern uint16 wlc_rate_shm_offset(struct wlc_info *wlc, uint8 rate);
extern uint32 wlc_get_rspec_history(struct wlc_info *wlc);
extern uint32 wlc_get_current_highest_rate(struct wlc_info *wlc);

static INLINE int wlc_iovar_getuint(struct wlc_info *wlc, const char *name, uint *arg)
{
	return wlc_iovar_getint(wlc, name, (int*)arg);
}

static INLINE int wlc_iovar_getuint8(struct wlc_info *wlc, const char *name, uint8 *arg)
{
	return wlc_iovar_getint8(wlc, name, (int8*)arg);
}

static INLINE int wlc_iovar_setuint(struct wlc_info *wlc, const char *name, uint arg)
{
	return wlc_iovar_setint(wlc, name, (int)arg);
}

/* ioctl helper */
extern uint wlc_freq2channel(uint freq);
extern uint wlc_channel2freq(uint channel);

#ifdef WLC_HIGH
/* BMAC_NOTE: Hack away iovar helper functions until we know what to do with iovar support */
#if defined(BCMDBG) || defined(WLTEST) || defined(BCMDBG_ERR) || defined(BCMDBG_DUMP)
extern int wlc_iocregchk(struct wlc_info *wlc, uint band);
#endif 
#if defined(BCMDBG) || defined(WLTEST) || defined(STA)
extern int wlc_iocbandchk(struct wlc_info *wlc, int *arg, int len, uint *bands, bool clkchk);
#endif
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
extern int wlc_iocpichk(struct wlc_info *wlc, uint phytype);
#endif 

#else /* !WLC_HIGH */
#define wlc_iocregchk(w, b)	(0)
#define wlc_iocbandchk(w, a, l, b, c)	(0)
#define wlc_iocpichk(w, p)	(0)
#endif /* WLC_HIGH */

/* helper functions */
extern void wlc_getrand(struct wlc_info *wlc, uint8 *buf, int len);

struct scb;
extern void wlc_ps_on(struct wlc_info *wlc, struct scb *scb);
extern void wlc_ps_off(struct wlc_info *wlc, struct scb *scb, bool discard);

#if defined(WLTINYDUMP) || defined(BCMDBG) || defined(WLMSG_ASSOC) || \
	defined(WLMSG_PRPKT) || defined(WLMSG_OID) || defined(BCMDBG_DUMP) || \
	defined(WLMSG_INFORM) || defined(WLMSG_WSEC) || defined(WLEXTLOG)
extern int wlc_format_ssid(char* buf, const uchar ssid[], uint ssid_len);
#endif

#ifdef STA
#ifdef BCMSUP_PSK
extern bool wlc_sup_mic_error(struct wlc_info *wlc, bool group);
#endif /* BCMSUP_PSK */
#ifdef	BCMWPA2
extern void wlc_pmkid_build_cand_list(struct wlc_info *wlc, bool check_SSID);
extern void wlc_pmkid_event(struct wlc_info *wlc);
#endif /* BCMWPA2 */
#endif /* STA */

#define	MAXBANDS		2	/* Maximum #of bands */
/* bandstate array indices */
#define BAND_2G_INDEX		0	/* wlc->bandstate[x] index */
#define BAND_5G_INDEX		1	/* wlc->bandstate[x] index */

#define BAND_2G_NAME		"2.4G"
#define BAND_5G_NAME		"5G"


/* LMAC API routines, called from either bus side */

extern void * wlc_attach_lmac(void *wl, uint16 vendor, uint16 device, uint unit, bool piomode,
	osl_t *osh, void *regsva, uint bustype, void *btparam, uint *perr);
extern bool wlc_lmac_hostpkt(struct wlc_info *wlc, void *pdu);
extern int wlc_ioctl_lmac(struct wlc_info *wlc, int cmd, void *arg, int len, struct wlc_if *wlcif);

#if defined(WLC_HIGH_ONLY)
void wlc_device_removed(void *arg);
#endif
#endif /* LINUX_POSTMOGRIFY_REMOVAL */

#ifndef LINUX_WLUSER_POSTMOGRIFY_REMOVAL
/* BMAC RPC: 7 uint32 params: pkttotlen, fifo, commit, fid, txpktpend, pktflag, pktflags2, rpc_id */
#define WLC_RPCTX_PARAMS	32

#endif /* LINUX_WLUSER_POSTMOGRIFY_REMOVAL */
#undef PACKED
#endif /* _wlc_pub_h_ */
