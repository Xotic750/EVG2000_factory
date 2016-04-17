/***********************************************************************
 *
 *  Copyright (c) 2006-2007  Broadcom Corporation
 *  All Rights Reserved
 *
# 
# 
# This program may be linked with other software licensed under the GPL. 
# When this happens, this program may be reproduced and distributed under 
# the terms of the GPL. 
# 
# 
# 1. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" 
#    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR 
#    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH 
#    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND 
#    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, 
#    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR 
#    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE 
#    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR 
#    PERFORMANCE OF THE SOFTWARE. 
# 
# 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR 
#    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, 
#    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY 
#    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN 
#    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; 
#    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE 
#    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS 
#    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY 
#    LIMITED REMEDY. 
#
 *
 ************************************************************************/

#ifndef __CMS_STRCONV_H__
#define __CMS_STRCONV_H__

/*!\file cms_strconv.h
 * \brief Header file for the application to convert value from string to other formats.
 * This is in the cms_util library.
 */

/** Macro to determine if a string parameter is empty or not */
#define IS_EMPTY_STRING(s)    ((s == NULL) || (*s == '\0'))

/** convert vpiVciStr (eg. "0/35") to vpi=2 and vci=35
 * @param vpiVciStr (IN) vpi/vci string
 * @param vpi (OUT) vpi in SINT32 format 
 * @param vci (OUT) vpi in SINT32 format
 * @return CmsRet enum.
 */
CmsRet cmsUtl_atmVpiVciStrToNum(const char *vpiVciStr, SINT32 *vpi, SINT32 *vci);

/**
 * @param vpiVciStr (IN) vpi/vci string
 * @param vpi (OUT) vpi in SINT32 format 
 * @param vci (OUT) vpi in SINT32 format
 * @return CmsRet enum.
 */
CmsRet cmsUtl_atmVpiVciNumToStr(const SINT32 vpi, const SINT32 vci, char *vpiVciStr);

/** convert macStr to macNum (array of 6 bytes)
 *  Ex: 0a:0b:0c:0d:0e:0f -> 0a0b0c0d0e0f
 *
 * @param macStr (IN) macStr to be converted.
 * @param macNum (OUT) macNum must point to a buffer of at least 6 bytes which the
 *                     caller provides.
 * @return CmsRet enum.
 */
CmsRet cmsUtl_macStrToNum(const char *macStr, UINT8 *macNum);

/** convert macNum (array of 6 bytes) to macStr.
 *  Ex: 0a0b0c0d0e0f -> 0a:0b:0c:0d:0e:0f
 * 
 * @param macNum (IN) macNum array to be converted.
 * @param macStr (OUT) macStr must point to a buffer of at least MAC_STR_LEN+1 (17+1)
 *                     to store the results.
 * @return CmsRet enum.
 */
CmsRet cmsUtl_macNumToStr(const UINT8 *macNum, char *macStr);

/** returns the CIDR IP mask length of an IP mask string.
 *  Ex: 255.255.255.0 -> 24
 * 
 * @param ipMask (IN) IP mask string in dot notation.
 *
 * @return CIDR IP mask length.
 */
UINT32 cmsUtl_ipMaskToCidrLen(const char *ipMask);

/** convert string to signed 32 bit integer, similar to unix strtol
 *
 * @param str (IN)        string to convert
 * @param endptr (IN/OUT) optional, if provided, will point to the first
 *                        character which caused conversion error.
 * @param base (IN)       radix to use for conversion.  If 0, then the
 *                        function will decide the radix based on input string.
 * @param val (OUT)       if conversion was successful, the resulting value.
 * @return CmsRet enum.
 */
CmsRet cmsUtl_strtol(const char *str, char **endptr, SINT32 base, SINT32 *val);


/** convert string to unsigned 32 bit integer, similar to unix strtoul
 *
 * @param str (IN)        string to convert
 * @param endptr (IN/OUT) optional, if provided, will point to the first
 *                        character which caused conversion error.
 * @param base (IN)       radix to use for conversion.  If 0, then the
 *                        function will decide the radix based on input string.
 * @param val (OUT)       if conversion was successful, the resulting value.
 * @return CmsRet enum.
 */
CmsRet cmsUtl_strtoul(const char *str, char **endptr, SINT32 base, UINT32 *val);


/** convert string to lowercase string
 *
 * @param string (IN/OUT) the upper case of the characters in the string will be 
 *                        converted to lower case characters.
 */
void cmsUtl_strToLower(char *string);


/*!\enum UrlProto
 * \brief URL protocols returned by cmsUtl_parseUrl().
 */
typedef enum 
{
   URL_PROTO_HTTP=0, /**< http */
   URL_PROTO_HTTPS=1, /**< https */
   URL_PROTO_FTP=2,   /**< ftp */
   URL_PROTO_TFTP=3   /**< tftp */
} UrlProto;


/** Parse an URL and return is components.
 *
 * @param url    (IN) url to parse.
 * @param proto (OUT) If not NULL, and if URL is well formed, it will be
 *                    set to the URL protocol (URL_PROTO_HTTPD, URL_PROTO_FTP, etc.)
 * @param addr  (OUT) If not NULL, and if URL is well formed, it will be 
 *                    set to the address portion of the URL.  Caller is responsible
 *                    for freeing addr string.
 * @param port  (OUT) If not NULL, and if URL is well formed, it will be 
 *                    set to the port portion of the URL.  If no port number
 *                    is specified, then port will be set to 0.
 * @param path  (OUT) If not NULL, and if URL is well formed, it will be 
 *                    set to the path portion of the URL.  Caller is responsible for
 *                    freeing path string.
 *
 * @return CmsRet enum, specifically, CMSRET_SUCCESS if URL is well formed,
 *                      CMSRET_INVALID_ARGUMENTS if URL is not well formed.
 */
CmsRet cmsUtl_parseUrl(const char *url, UrlProto *proto, char **addr, UINT16 *port, char **path);
 


/** Return the path to the CommEngine build directory in the given buffer.
 *
 * This function should only be used for unit testing on the Linux desktop,
 * but we define the symbol for modem builds as well to reduce the number
 * of ifdef's in the code.
 *
 * @param pathBuf   (OUT) Contains the pathBuf.
 * @param pathBufLen (IN) Length of the buffer.  If the buffer is not large
 *                        enough to hold the path, an error will be returned.
 * @return CmsRet enum.
 */
CmsRet cmsUtl_getBaseDir(char *pathBuf, UINT32 pathBufLen);


/** Return the Primary and Secondary DNS sever string.
 *
 *
 * @param str (IN)        comma seperated DNS Servers string to be converted
 * @param str (OUT)       if conversion was successful, Primary DNS string.
 * @param str (OUT)       if conversion was successful, Secondary DNS string.

 * @return CmsRet enum.
 */
CmsRet cmsUtl_parseDNS(const char *inDsnServers, char *outDnsPrimary, char *outDnsSecondary);


/** Return a syslog mode integer corresponding to the mode string.
 *
 * Mode refers to where the log goes.
 * The mode numbers are hard coded in logconfig.html, so there is no point in
 * defining #def's for them.
 * 1 = local circular buffer in shared mem
 * 2 = remote
 * 3 = local cirulcar buffer in shared mem and remote
 * 4 = local file (not clear if software actually supports this)
 * 5 = local file and remote (not clear if software actually supports this)
 *
 * @param modeStr (IN) mode string.
 *
 * @return mode number.
 */
SINT32 cmsUtl_syslogModeToNum(const char *modeStr);


/** Given a syslog mode number, return the associated string.
 *
 * See cmsUtl_syslogModeToNum for listing of valid mode numbers.
 *
 * @param mode (IN) mode number.
 *
 * @return mode string, this string is a pointer to a static string, so it
 * must not be modified or freed.
 */
char * cmsUtl_numToSyslogModeString(SINT32 mode);


/** Return true if the given number is a valid and supported syslog mode.
 *
 * See cmsUtl_syslogModeToNum for listing of valid mode numbers.
 * Only modes 1, 2, and 3 are supported.
 *
 * @param mode (IN) mode number.
 *
 * @return true if the given mode number is a valid and supported syslog mode.
 */
UBOOL8 cmsUtl_isValidSyslogMode(const char *modeStr);


/** Given a syslog level string, return equivalent log level.
 *
 * The log level numbers are defined in /usr/include/sys/syslog.h.
 *
 * @param levelStr (IN) The level in string format.
 *
 * @return the equivalent syslog level number.
 */
SINT32 cmsUtl_syslogLevelToNum(const char *levelStr);


/** Given a syslog level number, return the equivalent log level string.
 *
 * The log level numbers are defined in /usr/include/sys/syslog.h.
 *
 * @param level (IN) The log level number.
 *
 * @return the log level string, this string is a pointer to a static string, so it
 * must not be modified or freed.
 */
char * cmsUtl_numToSyslogLevelString(SINT32 level);


/** Return true if the given number is a valid syslog level.
 *  This is designed for the cli menu input validation.  But I put this
 *  function here to keep all the syslog level conversions together.
 *
 * @param level (IN) The log level number in string format.
 * 
 * @return TRUE if the given number is a valid syslog level.
 */
UBOOL8 cmsUtl_isValidSyslogLevel(const char *levelStr);


/** Return true if the string is a valid syslog level string.
 *
 * @param level (IN) The log level string.
 * 
 * @return TRUE if the given string is a valid syslog level string.
 */
UBOOL8 cmsUtl_isValidSyslogLevelString(const char *level);

#ifdef DMP_X_BROADCOM_COM_IPV6_1 /* aka SUPPORT_IPV6 */
CmsRet cmsUtl_stdIp6Address(const char *address, char *stdAddr);
UBOOL8 cmsUtl_isGlobalUnicastAddress(const char *address);

/** Return the Primary and Secondary DNS sever string.
 *
 *
 * @param str (IN)        comma seperated DNS Servers string to be converted
 * @param str (OUT)       if conversion was successful, Primary DNS string.
 * @param str (OUT)       if conversion was successful, Secondary DNS string.

 * @return CmsRet enum.
 */
CmsRet cmsUtl_getAddrPrefix(const char *address, UINT32 plen, char *prefix);

/** Return the Primary and Secondary DNS sever string.
 *
 *
 * @param str (IN)        comma seperated DNS Servers string to be converted
 * @param str (OUT)       if conversion was successful, Primary DNS string.
 * @param str (OUT)       if conversion was successful, Secondary DNS string.

 * @return CmsRet enum.
 */
CmsRet cmsUtl_replaceEui64(const char *address1, char *address2);

/** Return the Primary and Secondary DNS sever string.
 *
 *
 * @param str (IN)        comma seperated DNS Servers string to be converted
 * @param str (OUT)       if conversion was successful, Primary DNS string.
 * @param str (OUT)       if conversion was successful, Secondary DNS string.

 * @return CmsRet enum.
 */
CmsRet cmsUtl_parsePrefixAddress(const char *prefixAddr, char *address, UINT32 *plen);

#endif


/*!
 * \brief defines for ppp auto method in number
 */
#define PPP_AUTH_METHOD_AUTO        0
#define PPP_AUTH_METHOD_PAP         1
#define PPP_AUTH_METHOD_CHAP        2
#define PPP_AUTH_METHOD_MSCHAP      3


/** Given a ppp  auth method string, return the equivalent auth method number.
 *
 *
 * @param authStr (IN) The auth method string.
 *
 * @return the  auth method number
 */
SINT32 cmsUtl_pppAuthToNum(const char *authStr);

/** Given a ppp auth method number, return the equivalent auth method string.
 *
 *
 * @param authNum (IN) The auth method number.
 *
 * @return the  auth method string, this string is a pointer to a static string, so it
 * must not be modified or freed.
 */
char * cmsUtl_numToPppAuthString(SINT32 authNum);


/** Given a log level string, return the equivalent CmsLogLevel enum.
 *
 * @param logLevel (IN) The log level in string form.
 * @return CmsLogLevel enum.
 */
CmsLogLevel cmsUtl_logLevelStringToEnum(const char *logLevel);


/** Given a log destination string, return the equivalent CmsLogDestination enum.
 *
 * @param logDest (IN) The log destination in string form.
 * @return CmsLogDestination enum.
 */
CmsLogDestination cmsUtl_logDestinationStringToEnum(const char *logDest);


/** Given an IPv4 address string, return true if it is a valid IPv4 address
 *  in dotted notation.
 *
 * @param ipAddr (IN) IPv4 address string.
 *
 * @return TRUE if the string is a valid IPv4 address in dotted notation.
 */
UBOOL8 cmsUtl_isValidIpAddress(const char* ipAddr);


/** Given a MAC address string, return true if it is a valid mac address
 *  string.  Note the string must be in the following format:
 *  xx:xx:xx:xx:xx:xx where x is a hex digit.
 *
 * @param macAddr (IN) mac address string.
 *
 * @return TRUE if the string is a valid mac address.
 */
UBOOL8 cmsUtl_isValidMacAddress(const char* macAddress);


/** Given a port number string, return true if it is a valid port number.
 *
 * @param portNumberStr (IN) port number string.
 *
 * @return TRUE if the string is a valid port number.
 */
UBOOL8 cmsUtl_isValidPortNumber(const char* portNumberStr);

/** Compare two strings.  Similar to strcmp except that NULL string will be
 *  treated as zero length string.
 *
 * @param s1 (IN) the first string.
 * @param s2 (IN) the second string.
 *
 * @return same return value as strcmp
 */
SINT32 cmsUtl_strcmp(const char *s1, const char *s2);


/** Compare two strings disregarding case.  Similar to strcasecmp except that
 *  NULL string will be treated as zero length string.
 *
 * @param s1 (IN) the first string.
 * @param s2 (IN) the second string.
 *
 * @return same return value as strcasecmp
 */
SINT32 cmsUtl_strcasecmp(const char *s1, const char *s2);


/** Compare two strings.  Similar to strncmp except that NULL string will be
 *  treated as zero length string.
 *
 * @param s1 (IN) the first string.
 * @param s2 (IN) the second string.
 * @param n  (IN) number of characters to compare.
 *
 * @return same return value as strncmp
 */
SINT32 cmsUtl_strncmp(const char *s1, const char *s2, SINT32 n); 


/** Compare two strings disregarding case.  Similar to strncasecmp except that
 *  NULL string will be treated as zero length string.
 *
 * @param s1 (IN) the first string.
 * @param s2 (IN) the second string.
 * @param n  (IN) number of characters to compare.
 *
 * @return same return value as strcmp
 */
SINT32 cmsUtl_strncasecmp(const char *s1, const char *s2, SINT32 n); 


/** locate a substring.  Similar to strstr except that NUll string will be
 *  treated as zero length string.
 *
 * @param s1 (IN) the first string.
 * @param s2 (IN) the second string.
 *
 * @return same return value as strstr
 */
char *cmsUtl_strstr(const char *s1, const char *s2);


/** locate a sub-option string in a string of sub-options separated by commas.
 * 
 * This function is useful if you have a parameter string that looks like:
 * AAAA, AAAABBB, CCC, DD
 * and you want to know if sub-option AAAA is present.  Note that strstr cannot
 * be used here because if your parameter string was
 * AAAABBB, CCC, DD
 * and you used strstr to look for option AAAA, then strstr would return a pointer
 * to AAAABBB, which is not the sub-option string you were really looking for.
 * This is required by the DSL code in dealing with the AdslModulationCfg parameter.
 *
 * @param s1 (IN) the full sub-option string.  Sub-options can be separated by any
 *                number of spaces and commas.
 * @param s2 (IN) the sub-option string.
 *
 * @return TRUE if the subOption string was found in the fullOptionString.
 */
UBOOL8 cmsUtl_isSubOptionPresent(const char *fullOptionString, const char *subOption);


/** Retrieve WAN protocol name of the given protocol ID
 *
 * @param prtcl     (IN) network protocol
 * @param portId  (OUT) name - protocol name for html
 *
 * @return None.
 */
void cmsUtl_getWanProtocolName(UINT8 protocol, char *name) ;


/** Convert a linear array of DHCP Vendor Ids into a single comma separated string.
 *
 * @param vendorIds (IN) This is a sequence of DHCP Vendor Ids laid out as a single linear buffer.
 *                       There are MAX_PORTMAPPING_DHCP_VENDOR_IDS (5) chunks in the buffer.
 *                       Each chunk is DHCP_VENDOR_ID_LEN + 1 bytes long.  Within each chunk
 *                       there may be a single DHCP vendor id.
 *
 * @return comma separated string of vendor ids.  The caller is responsible for freeing
 *         this buffer.
 */
char *cmsUtl_getAggregateStringFromDhcpVendorIds(const char *vendorIds);

/** Convert a comma separated string of vendor ids into a linear array of DHCP vendor ids.
 *
 * @param aggregateString (IN) This is a sequence of up to MAX_PORTMAPPING_DHCP_VENDOR_IDS(5) comma 
 *                             separated dhcp vendor id strings.
 *
 * @return a linear buffer of vendor ids.  The caller is responsible for freeing this buffer.
 */
char *cmsUtl_getDhcpVendorIdsFromAggregateString(const char *aggregateString);


/** Get number of DHCP vendor ids in the aggregate string
 * 
 * This is not implemented yet because it is not needed.  But it seems like a
 * useful function to have.
 * 
 * @param aggregateString (IN) This is a sequence of up to MAX_PORTMAPPING_DHCP_VENDOR_IDS(5) comma 
 *                             separated dhcp vendor id strings.
 * 
 * @return number of DHCP vendor ids.
 */
UINT32 cmsUtl_getNumberOfDhcpVendorIds(const char *aggregateString);


#define DSL_LINK_DESTINATION_PREFIX_PVC   "PVC:"
#define DSL_LINK_DESTINATION_PREFIX_SVC   "SVC:"


/** Convert connection mode string to a ConnectionModeType number
 *
 * @param connModeStr (IN) MDMVS_ string defined in mdm 
 *
 * @return a ConnectionModeType number
 */
ConnectionModeType cmsUtl_connectionModeStrToNum(const char *connModeStr);


#endif /* __CMS_STRCONV_H__ */


