/***************************************************************************
 ***
 ***    Copyright 2009  Hon Hai Precision Ind. Co. Ltd.
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
 ***  Modified               Eric Huang       08/05/2009        
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

int get_host_type(int host_type, char * vendor_id)
{
    if ( strcmp(vendor_id, "SingTel_DECT") == 0 )
    {
        return IP_DECT_HOST;
    }
    else if ( strcmp(vendor_id, "MSFT_IPTV") == 0 )
    {
        return STB_HOST;
    }
    else if ( strcmp(vendor_id, "Xbox 360") == 0 )
    {
        return XBOX360_HOST;
    }
    else
        return DATA_HOST;
}


int new_host_add(int host_ip, int host_type, char * vendor_id,char * host_name)
{
    char snd_msg[MAX_MSG_LEN];
    int   lan_host_type;

    lan_host_type= get_host_type(host_type,vendor_id);

    sprintf(snd_msg,"%x %d %s", host_ip, lan_host_type, vendor_id);
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
