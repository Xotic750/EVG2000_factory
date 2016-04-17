/***********************************************************************
 *
 *  Copyright (c) 2007  Broadcom Corporation
 *  All Rights Reserved
 *
<:license-private
 *
 ************************************************************************/


#include "cms.h"
#include "cms_util.h"


void usage(UINT32 exitCode)
{
   printf("usage: ledctl [ppp] [on|off|red]\n");
   printf("    ppp is the only led that is controllable by this app right now.\n");
   printf("    ppp must be specified.\n");
   printf("    one of on, off, or red must be specified.\n");

   exit(exitCode);
}


void processPppLed(const char *state)
{
   if (!cmsUtl_strcmp(state, "on"))
   {
      cmsLed_setPppConnected();
   }
   else if (!cmsUtl_strcmp(state, "off"))
   {
      cmsLed_setPppDisconnected();
   }
   else if (!cmsUtl_strcmp(state, "red"))
   {
      cmsLed_setPppFailed();
   }
   else
   {
      usage(1);
   }

}


int main(int argc, char *argv[])
{

   if (argc != 3)
   {
      usage(1);
   }

   if (!cmsUtl_strcmp(argv[1], "ppp"))
   {
      processPppLed(argv[2]);
   }
   else
   {
      usage(1);
   }

   return 0;
}
