/***************************************************************************
 ***
 ***    Copyright 2005  Hon Hai Precision Ind. Co. Ltd.
 ***    All Rights Reserved. 
 ***    No portions of this material shall be reproduced in any form without the
 ***    written permission of Hon Hai Precision Ind. Co. Ltd.
 ***
 ***    All information contained in this document is Hon Hai Precision Ind.  
 ***    Co. Ltd. company private, proprietary, and trade secret property and 
 ***    are protected by international intellectual property laws and treaties.
 ***
 ****************************************************************************
 ***
 ***  Filename: lanhostmsg.c
 ***
 ***  Description:
 ***     Provide function with italia telcom spec, list lan host identification, host name check
 ***
 ***  History:
 ***
 ***  Modify Reason            Author          Date             Search Flag(Option)
 ***--------------------------------------------------------------------------------------
 ***  Created                Lewis            09/25/2008
 ***     
 *******************************************************************************/

 #include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "dhcpd.h"
#include "files.h"
#include "options.h"
#include "leases.h"
#include "debug.h"

#define HostQId 0x484944
#define MAX_MSG_LEN 256
typedef struct S_mymesg {
    long mtype;
    char mtext[MAX_MSG_LEN];
} T_mymesg;

T_mymesg mesg; 
int queId;
void lan_host_add(char * psendmsg);
void lan_host_delete(char * psendmsg);
extern char *acosNvramConfig_get(const char *name);

IT_IPTVService_type_t get_iptv_service_type()
{
    char * type_string;
    type_string=server_config.IPTV_service;
    DEBUG(LOG_INFO, "get_iptv_service_type:  %s\n", server_config.IPTV_service);

    if(0==strcmp(type_string,"Natted"))
      return IPTV_Natted;
    else if(0==strcmp(type_string,"Dyn_Portmapping"))
      return IPTV_Dyn_Portmapping;
    else if(0==strcmp(type_string,"Static_Portmapping"))
      return IPTV_Static_Portmapping;
    else
      return IPTV_OFF;
}

void create_que_id(void)
{
    queId = msgget( HostQId, 0 );
    if( queId < 0 )
    {
        DEBUG(LOG_INFO, " Creat HostId queue error\n" );
        return ;
    }
    DEBUG(LOG_INFO, " Creat HostId queue OK, queId %d\n",queId );
    return ;
}

int host_delete(int host_ip)
{
    char snd_msg[MAX_MSG_LEN];
    DEBUG(LOG_INFO, "host_delete: 0x%x", host_ip);

    sprintf(snd_msg,"%x", host_ip);
    DEBUG(LOG_INFO, "host_delete: %s", snd_msg);

    lan_host_delete(snd_msg);

}

/*
12.1 MANAGED HOSTS
Managed hosts are identified by CPE based on DHCP protocol. These hosts can be compliant with
or WT-111v2 [50] or TR-111 [51]. Managed Hosts WT-111v2 declare themselves using DHCP option 60
(string ¡§dslforum.org¡¨ - all lower case - in the Vendor Class Identifier [50]), while managed Hosts TR-
111 declare themselves using DHCP option 125 as specified in [51].
12.2 DATA HOSTS
Data hosts are identified by the CPE identifies as those hosts that are neither managed hosts nor
SIP hosts.
*/
/*
AG acknowledges the type of LAN terminals via specific protocols, standards and
owners and breaks these down into the following categories:
a) VoIP terminals
Set out in the APT CP protocol (see document [3]). These are associated with several
telephone numbers (CLI), through which VoIP calls can be made via the Telecom Italian
network. For further information please refer to ¡± 3.1.
b) remote terminals
These are set out in the DHCP protocol, using options 43 and 60 (as specified in [11] or
alternatively using option 125 (as specified in [10]). These terminals are managed by a
remote ACS. For further information on the remote management of the AG and on the
TR-609 information model, please refer to document [4].
c) data terminals
These are not included in the terminals defined in the previous points. These can be set
out via the DHCP protocol, but must not use any of the below options: 43, 60 (as
specified in [11]) and 125 (as specified in [10]).
d) Set-Top-Box (S TB) terminals
These are set out in the DHCP protocol using option 60, which is valorised with a string
ending in ¢wstb¡ü. The string may be variable in length but will always have the ¢wstb¡ü 3-
character suffix. Once it has acknowledges this terminal, the AG behaves as described in
¡± 3.2.
*/

/*
When the AG acknowledges the presence of an STB in the LAN network via option
DHCP 60, it behaves differently depending on the value of the proprietary parameter
TELECOMITALIA_IT_IPTVService.
X_TELECOMITALIA_IT_IPTVService="OFF": in this case, the IPTV service is not
active on the appliance and therefore
the presence of the STB in the LAN network is ignored. The AG will therefore assign
and IP to the STB via DHCP server and will regard this as a data terminal;
X_TELECOMITALIA_IT_IPTVService="Static_portmapping": the IPTV service is
active and is statically
configured on the appliance. In this case, the presence of the STB in the LAN network
is ignored by the AG, as the configuration of the AG is such that the DHCP request of
the STB is directly transmitted via the network (the Ethernet port to which the STB is
connected is in bridge configuration with the WAN network);
*/
int get_host_type(int host_type, char * vendor_id)
{
     char * stb_string;
     //printf("\nget_host_type: 0x%x %s", host_type,  vendor_id);

     stb_string=strstr(vendor_id, STB_STRING);
     //printf("get_host_type: %s\n", stb_string);
     if ((NULL!=stb_string)&&(3 ==strlen(stb_string)))//will always have the stb 3-character suffix means STP HOST
     {
         switch (get_iptv_service_type())
         {
             case IPTV_Natted:
                 return STB_HOST;
             case IPTV_Static_Portmapping:
                 return DATA_HOST;
             case IPTV_Dyn_Portmapping:
                 return STB_HOST;
             default:
                 return DATA_HOST;
         }
     }
     else if(host_type==1)
         return MANAGED_HOST;
     else
         return DATA_HOST;
}


int new_host_add(int host_ip, int host_type, char * vendor_id,char * host_name)
{
    char snd_msg[MAX_MSG_LEN];
    int   lan_host_type;
    //printf("new_host_add: 0x%x %s %s", host_ip,  vendor_id,host_name);
    lan_host_type= get_host_type(host_type,vendor_id);

    sprintf(snd_msg,"%x %d %s", host_ip, lan_host_type, host_name);
    DEBUG(LOG_INFO, "new_host_add: %s", snd_msg);

    lan_host_add(snd_msg);
}

/*
To add lan host information:
*/
void lan_host_add(char * psendmsg)
{
    int iRet;

    mesg.mtype = 1;
    strcpy( mesg.mtext, psendmsg);
    iRet=msgsnd( queId, &mesg, strlen(mesg.mtext)+1, IPC_NOWAIT);
    DEBUG(LOG_INFO, "lan_host_add :queId %d send length %d, content %s", queId,iRet,mesg.mtext);
}
/*
To delete lan host information:
*/
void lan_host_delete(char * psendmsg)
{
    int iRet;

    mesg.mtype = 2;
    strcpy( mesg.mtext, psendmsg);
    iRet=msgsnd( queId, &mesg, strlen(mesg.mtext)+1, IPC_NOWAIT);
    DEBUG(LOG_INFO, "lan_host_delete :queId %d send length %d, content %s", queId,iRet,mesg.mtext);
}
/*
The AG manages option 12 DHCP for assigning names to LAN clients. The GUI
must display the name associated to LAN Hosts in accordance with the following rule:
a. for a hostname with the root BaseEthDECT, the name "Aladino VoIP" must be
displayed (see [2] [7])
b. for a hostname with the root FAXPOS_Eth, the name "FAX Adapter" must be displayed
b. for a hostname with the root .lgvp¡¥, the name "New Video Telephone" must be
displayed
d. for hosts presenting themselves via option 12, the option value must be displayed
e. for hosts presenting themselves without valorising option 12, the AG must assign and
display the name ¢wHost-xxx¡ü, where xxx is a sequential, 3-digit numeric code
commencing at 001 for the first host without name associated to the AG
f. for hosts that have been assigned a static IP, the AG must display the name
The AG must furthermore ensure that hostnames are unique, both in respect of LAN hosts
with known hostname root (points a, b, c and d) and in the case of LAN hosts without name
(point e), in accordance with the following rules:
¡E in the case of known hostnames (points a, b, c and d), the AG must add a final digit to
the displayed name (e.g. Aladino VoIP1, Aladino VoIP 2, etc.) for the second and third
hosts with the hostname BaseEthDECT)
¡E in the case of LAN hosts without name (point e), the AG must increment the numeric
code by 3 digits as and when hosts without name connect, so the names displayed
are always unique. Such names will be of this type: Host-001, Host-002, etc.
*/
void check_host_name(char * oldhostname, char *newhostname, char * chaAddr)
{
    struct dhcpOfferedAddr *old_lease,*old_lease_cha;
    char temphostname[64];
    int i;
    DEBUG(LOG_INFO, "\ncheck_host_name: oldhostname %s  ",oldhostname);

    if(0==oldhostname)/*b. If one or more LAN hosts don¡¦t present the hostname*/
    {
         for(i=0;i<server_config.max_leases;i++)/*the AG must force the client hostname to the string ¡§Host-xxx¡¨*/
         {
               sprintf(temphostname,"Host-%03d",(i+1));
               old_lease=find_lease_by_hostname(temphostname);
               if(NULL==old_lease)
               {
                      DEBUG(LOG_INFO, "\nIf one or more LAN hosts don¡¦t present the hostname");
                      strcpy(newhostname,temphostname);
                      break;

               }
         }
    }
    else
    {
        old_lease=find_lease_by_hostname(oldhostname);
        old_lease_cha=find_lease_by_chaddr(chaAddr);

        if(NULL==old_lease)
        {

            /*d. If the hostname is ¡§BaseEthDECT¡¨, the name ¡§Aladino VoIP¡¨ must be displayed on the GUI 
            e. If the hostname is ¡§FAXPOS_Eth¡¨, the name ¡§FAX Adapter¡¨ must be displayed on the GUI*/
            if(0==strcmp("BaseEthDECT",oldhostname))
                strcpy(newhostname,"Aladino VoIP");
            else if(0==strcmp("FAXPOS_Eth",oldhostname))
                strcpy(newhostname,"FAX Adapter");
            else
                strcpy(newhostname,oldhostname);
            DEBUG(LOG_INFO, "\n If the hostname is FAXPOS_Eth or BaseEthDECT");
        }
        else if((old_lease)==(old_lease_cha))/*Already in lease table*/
        {
            DEBUG(LOG_INFO, "\n Already in lease table");
            strcpy(newhostname,oldhostname);
        }
        else/*If different hosts use the same hostname in DHCP option 12*/
        {
            for(i=0;i<server_config.max_leases;i++)/*adding the string ¡§-xxx, where xxx is a numerical and increasing suffix starting from 001*/
            {
                sprintf(temphostname,"%s-%03d",oldhostname,(i+1));
                old_lease=find_lease_by_hostname(temphostname);
                if(NULL==old_lease)
                {
                    DEBUG(LOG_INFO, "\n different hosts use the same hostname");
                    strcpy(newhostname,temphostname);
                    break;
                }
             }
           }
       }
       DEBUG(LOG_INFO, "newhostname %s\n",newhostname);
}
