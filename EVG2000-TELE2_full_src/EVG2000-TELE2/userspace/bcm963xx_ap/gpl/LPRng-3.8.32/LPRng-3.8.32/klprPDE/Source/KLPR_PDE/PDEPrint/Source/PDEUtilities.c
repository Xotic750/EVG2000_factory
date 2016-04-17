/*
********************************************************************************
    
    $Log: PDEUtilities.c,v $


    (c) Copyright 2002 Apple Computer, Inc.  All rights reserved.
    
    IMPORTANT: This Apple software is supplied to you by Apple Computer,
    Inc. ("Apple") in consideration of your agreement to the following
    terms, and your use, installation, modification or redistribution of
    this Apple software constitutes acceptance of these terms.  If you do
    not agree with these terms, please do not use, install, modify or
    redistribute this Apple software.
    
    In consideration of your agreement to abide by the following terms, and
    subject to these terms, Apple grants you a personal, non-exclusive
    license, under Apple's copyrights in this original Apple software (the
    "Apple Software"), to use, reproduce, modify and redistribute the Apple
    Software, with or without modifications, in source and/or binary forms;
    provided that if you redistribute the Apple Software in its entirety and
    without modifications, you must retain this notice and the following
    text and disclaimers in all such redistributions of the Apple Software.
    Neither the name, trademarks, service marks or logos of Apple Computer,
    Inc. may be used to endorse or promote products derived from the Apple
    Software without specific prior written permission from Apple.  Except
    as expressly stated in this notice, no other rights or licenses, express
    or implied, are granted by Apple herein, including but not limited to
    any patent rights that may be infringed by your derivative works or by
    other works in which the Apple Software may be incorporated.
    
    The Apple Software is provided by Apple on an "AS IS" basis. APPLE MAKES
    NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE
    IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR
    A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
    OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
    
    IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
    OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
    MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
    AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
    STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.
    
********************************************************************************
*/

#include <Carbon/Carbon.h>
#include <Print/PMPrintingDialogExtensions.h>

#include "PDECore.h"
#include "PDECustom.h"
#include "PDEUtilities.h"
#include <Kerberos/Kerberos.h>
#include <sys/types.h>
#include <sys/stat.h>

#define kCompareEqual 0

// callback function to handle the 'help' event
static OSStatus MyHandleHelpEvent (EventHandlerCallRef, EventRef, void *userData);

#define  kCustomSettingsKey \
            CFSTR("edu.ncsu.print.customSetting." kMyBundleSignature)


char * gServiceName = "lpr/print.ncsu.edu@EOS.NCSU.EDU";

/*
--------------------------------------------------------------------------------
    MyDebugMessage
--------------------------------------------------------------------------------
*/

extern void MyDebugMessage (char *msg, SInt32 value)
{
    char *debug = getenv ("PDEDebug");
//   if (debug != NULL)
    {
        fprintf (stdout, "%s (%d)\n", msg, (int) value);
        fflush (stdout);
    }
}

/*
--------------------------------------------------------------------------------
   yGetBundle
--------------------------------------------------------------------------------
*/

static CFBundleRef _MyGetBundle (Boolean stillNeeded)
{
    static CFBundleRef sBundle = NULL;
    
    if (stillNeeded)
    {
        if (sBundle == NULL)
        {
            sBundle = CFBundleGetBundleWithIdentifier (kMyBundleIdentifier);
            CFRetain (sBundle);
        }
    }
    else
    {
        if (sBundle != NULL)
        {
            CFRelease (sBundle);
            sBundle = NULL;
        }
    }

    return sBundle;
}


/*
--------------------------------------------------------------------------------
    MyGetBundle
--------------------------------------------------------------------------------
*/

extern CFBundleRef MyGetBundle()
{
    return _MyGetBundle (TRUE);
}


/*
--------------------------------------------------------------------------------
    MyFreeBundle
--------------------------------------------------------------------------------
*/

extern void MyFreeBundle()
{
    _MyGetBundle (FALSE);
}


#pragma mark -

/*
--------------------------------------------------------------------------------
    MyGetTitle
--------------------------------------------------------------------------------
*/

extern CFStringRef MyGetTitle()
{
    return MyGetCustomTitle (TRUE);
}


/*
--------------------------------------------------------------------------------
    MyFreeTitle
--------------------------------------------------------------------------------
*/

extern void MyFreeTitle()
{
    MyGetCustomTitle (FALSE);
}


#pragma mark -

/*
--------------------------------------------------------------------------------
    MyGetTicket
--------------------------------------------------------------------------------
*/

extern OSStatus MyGetTicket (
    PMPrintSession  session,
    CFStringRef     ticketID,
    PMTicketRef*    ticketPtr
)

{
    OSStatus result = noErr;
    CFTypeRef type = NULL;
    PMTicketRef ticket = NULL;
    
    *ticketPtr = NULL;

    result = PMSessionGetDataFromSession (session, ticketID, &type);

    if (result == noErr)
    {    
        if (CFNumberGetValue (
            (CFNumberRef) type, kCFNumberSInt32Type, (void*) &ticket))
        {
            *ticketPtr = ticket;
        }
        else {
            result = kPMInvalidValue;
        }
    }

    return result;
}


/*
--------------------------------------------------------------------------------
    MyEmbedControl
--------------------------------------------------------------------------------
*/

extern OSStatus MyEmbedControl (
    WindowRef nibWindow,
    ControlRef userPane,
    const ControlID *controlID,
    ControlRef* outControl
)

{
    ControlRef control = NULL;
    OSStatus result = noErr;

    *outControl = NULL;

    result = GetControlByID (nibWindow, controlID, &control);
    if (result == noErr)
    {
        SInt16 dh, dv;
        Rect nibFrame, controlFrame, paneFrame;

        (void) GetWindowBounds (nibWindow, kWindowContentRgn, &nibFrame);
        (void) GetControlBounds (userPane, &paneFrame);
        (void) GetControlBounds (control, &controlFrame);
        
        // find vertical and horizontal deltas needed to position the control
        // such that the nib-based interface is centered inside the dialog pane

        dh = ((paneFrame.right - paneFrame.left) - 
                (nibFrame.right - nibFrame.left))/2;

        if (dh < 0) dh = 0;

        dv = ((paneFrame.bottom - paneFrame.top) - 
                (nibFrame.bottom - nibFrame.top))/2;

        if (dv < 0) dv = 0;
                
        OffsetRect (
            &controlFrame, 
            paneFrame.left + dh, 
            paneFrame.top + dv
        );
 
        (void) SetControlBounds (control, &controlFrame);

        // make visible
        result = SetControlVisibility (control, TRUE, FALSE);

        if (result == noErr) 
        {
            result = EmbedControl (control, userPane);
            if (result == noErr)
            {
                // return the control only if everything worked
                *outControl = control;
            }
        }
    }

    return result;
}


#pragma mark -

/*
--------------------------------------------------------------------------------
    MyReleaseContext
--------------------------------------------------------------------------------
*/

extern void MyReleaseContext (MyContext context)
{
    if (context != NULL)
    {
        if (context->customContext != NULL) {
            MyReleaseCustomContext (context->customContext);
        }

        free (context);
    }
}


#pragma mark -

/*
--------------------------------------------------------------------------------
    MyInstallHelpEventHandler
--------------------------------------------------------------------------------
*/

#define kMyNumberOfEventTypes   1

extern OSStatus MyInstallHelpEventHandler (
    WindowRef inWindow, 
    EventHandlerRef *outHandler,
    EventHandlerUPP *outHandlerUPP,
    MyCustomContext context
)

{
    static const EventTypeSpec sEventTypes [kMyNumberOfEventTypes] =
    {
        { kEventClassCommand, kEventCommandProcess }
    };

    OSStatus result = noErr;
    EventHandlerRef handler = NULL;
    EventHandlerUPP handlerUPP = NewEventHandlerUPP (MyHandleHelpEvent);

    result = InstallWindowEventHandler (
        inWindow,
        handlerUPP,
        kMyNumberOfEventTypes,
        sEventTypes,
        context, // NULL
        &handler
    );

    *outHandler = handler;
    *outHandlerUPP = handlerUPP;
    
    MyDebugMessage("InstallEventHandler", result);
    return result;
}


/*
--------------------------------------------------------------------------------
    MyRemoveHelpEventHandler
--------------------------------------------------------------------------------
*/

extern OSStatus MyRemoveHelpEventHandler (
    EventHandlerRef *helpHandlerP, 
    EventHandlerUPP *helpHandlerUPP
)

{
    OSStatus result = noErr;
    
    // we remove the help handler if there is still one present
    if (*helpHandlerP != NULL)
    {
        MyDebugMessage("Removing event handler", result);
        result = RemoveEventHandler (*helpHandlerP);
        *helpHandlerP = NULL;
    }

    if (*helpHandlerUPP != NULL)
    {
        DisposeEventHandlerUPP (*helpHandlerUPP);
        *helpHandlerUPP = NULL;
    }
    return result;
}


#pragma mark -

/*
--------------------------------------------------------------------------------
    MyHandleHelpEvent
--------------------------------------------------------------------------------
*/

static OSStatus MyHandleHelpEvent
(
    EventHandlerCallRef call,
    EventRef event, 
    void *userData
)

{
    HICommand   commandStruct;
    OSStatus    result = eventNotHandledErr;
    PMTicketRef ticket = NULL;
    CFStringRef path;
    
    GetEventParameter (
        event, kEventParamDirectObject,
        typeHICommand, NULL, sizeof(HICommand), 
        NULL, &commandStruct
    );
    
    MyCustomContext context = (MyCustomContext)userData;
    CFStringRef serviceName = context->settings.serviceName;
    
    if (commandStruct.commandID == 'ok  ')
    {
        MyDebugMessage("handled ok event", 0);
        
        result = MyGetTicket(context->session, kPDE_PMPrintSettingsRef, &ticket);
        if(result == noErr)
        {
            result = PMTicketGetCFString(ticket, kPMTopLevel, kPMTopLevel, kCustomSettingsKey,&path);
        }
    
        if((result = GetServiceTicket(serviceName)) == 0)
		{                
			if((result = WriteSGT(serviceName, CFStringGetCStringPtr(path, kCFStringEncodingMacRoman))) != 0)
				MyDebugMessage("failed to write service tickets", result);
		}
		else
			MyDebugMessage("Failed to get service tickets", result);
		
    }

    return eventNotHandledErr; // always return this error because we don't want to stop the event from being handled
}

// makes sure there is both an tgt and a lpr sgt in the default cache
extern OSStatus GetServiceTicket(CFStringRef serviceName)
{
    char *cacheName;
    KLPrincipal outPrincipal;    
    KLStatus myKLErrCode;        
    
    krb5_error_code myKRB5ErrCode;
    krb5_context context;
    krb5_ccache ccache;
    krb5_creds in_creds;
    krb5_creds *out_creds;
    krb5_principal me;
    
    // want to have both a tgt and a sgt in the default cache
    
    // ensures we have a tgt
    myKLErrCode = KLAcquireInitialTickets(NULL, NULL, &outPrincipal, &cacheName);
    if(myKLErrCode != klNoErr)
    {
        printf("error getting tgt:%d", myKLErrCode);
        return myKLErrCode;
    }    
    
    // add a sgt to the cache;        
    if(myKRB5ErrCode = krb5_init_context(&context))
    {
        printf("error krb5_init_context:%d\n", myKRB5ErrCode);    
        return myKLErrCode;        
    }
    
    myKRB5ErrCode = krb5_cc_default(context, &ccache);
    if(myKRB5ErrCode != noErr)
    {
        printf("cc_default error:%d\n", myKRB5ErrCode);
        return myKLErrCode;        
    }

    myKRB5ErrCode = krb5_cc_get_principal(context, ccache, &me);
    if(myKRB5ErrCode != noErr)
    {
        printf("error getting principal name:%d\n", myKRB5ErrCode);
        return myKLErrCode;        
    }
    
    // fill out properites we want for the new credential
    memset(&in_creds, 0, sizeof(in_creds));
        
    in_creds.client = me;
    
    myKRB5ErrCode = krb5_parse_name(context, CFStringGetCStringPtr(serviceName, kCFStringEncodingMacRoman), &in_creds.server);
    if(myKRB5ErrCode != noErr)
    {
        printf("error parsing principal:%d\n", myKLErrCode);
        return myKLErrCode;        
    }
    
    in_creds.keyblock.enctype = 0;
    
    myKRB5ErrCode = krb5_get_credentials(context, 0, ccache, &in_creds, &out_creds);
    if(myKRB5ErrCode != noErr)
    {    
        printf("error getting credentials:%d\n", myKRB5ErrCode);
        return myKLErrCode;        
    }
        
    // clean up
    krb5_free_principal(context, me);
    krb5_free_principal(context, in_creds.server);    
    krb5_free_creds(context, out_creds);    
    krb5_cc_close(context, ccache);
    krb5_free_context(context);    
    
    return 0;
}

// creates a copy of the ENTIRE default credentials cache in a file cache
static OSStatus CreateFileCacheCopy(char * cache)
{
    krb5_context context;
    krb5_ccache fromCache;
    krb5_ccache toCache;
    krb5_principal defaultPrincipal;
    krb5_error_code errCode;
    
    
    errCode = krb5_init_context(&context);
    if(errCode != 0)
    {
        printf("failed kerberos init:%d", errCode);
        return errCode;        
    }    
    
   errCode = krb5_cc_default(context, &fromCache);
   if(errCode != 0)
   {
        printf("failed getting default cache:%d", errCode);
        return errCode;        
   }
    
    errCode = krb5_cc_get_principal(context, fromCache, &defaultPrincipal);
    if(errCode != 0)
    {
        printf("failed getting principal from cache:%d", errCode);
        return errCode;        
    }
    
    errCode = krb5_cc_resolve(context, cache, &toCache);
    if(errCode != 0)
    {
        printf("filed resolving file cache:%d", errCode);
        return errCode;        
    }
    
    errCode = krb5_cc_initialize(context, toCache, defaultPrincipal);
    if(errCode != 0)
    {
        printf("filed initializing file cache:%d", errCode);
        return errCode;        
    }
    
    errCode = krb5_cc_copy_creds(context, fromCache, toCache);
    if(errCode != 0)
    {
        printf("failed copying credentials:%d", errCode); 
        return errCode;        
    }

/*    // set permission bits to -rw-r--r--
    if(chmod("/private/tmp/printServiceTicket", S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH))
        printf("chmod failed");
*/
        
    krb5_free_principal(context, defaultPrincipal);
    krb5_cc_close(context, fromCache);
    krb5_cc_close(context, toCache);    
    krb5_free_context(context);    
    
    return 0;
}

void HandleError(Boolean isError, char * errorMessage, krb5_error_code errorCode)
{
    if(isError)
    {
        printf("%s error:%d", errorMessage, errorCode);
    }
}

// copies only the service ticket to a file cache
extern OSStatus WriteSGT(CFStringRef serviceName, const char * path)
{
    krb5_error_code errCode;
    krb5_cc_cursor cur;
    krb5_ccache cache = NULL;
    krb5_context kcontext;
//    char *clientName;
    char *serverName;
    krb5_creds creds;

//    KLPrincipal outPrincipal;
    
    // create login options, with service ticket of "lpr/print.ncsu.edu@EOS.NCSU.EDU "
/*    char *cacheName;
    
    KLStatus myKLStatus;
    KLLoginOptions myLoginOptions;
    
    myKLStatus = KLCreateLoginOptions(&myLoginOptions);
    
    printf("writing only sgt\n");

    // get service ticket        
    if(myKLStatus == klNoErr)
    {
        myKLStatus = KLLoginOptionsSetServiceName(myLoginOptions, gServiceName);
        myKLStatus = KLAcquireInitialTickets(NULL, myLoginOptions, &outPrincipal, &cacheName);            
        
        // @debug, bad cachename return , seg 11
//        if(myKLStatus == klNoErr && cacheName != NULL)            
//            NSLog("placed tickets in cache:%s", cacheName);        
    }
    else
    {    
        printf("Kerberos: get initial ticket failure\n");
        return myKLStatus;
    }
*/

    if((errCode = krb5_init_context(&kcontext)) != noErr)
    {
        printf("init_context error:%d\n",errCode);
        return errCode;
    }
    
    if((errCode = krb5_cc_default(kcontext, &cache)) != noErr)
    {
        printf("get default cache error:%d\n", errCode);
        return errCode;
    }
    
    if((errCode = krb5_cc_start_seq_get(kcontext, cache, &cur) != noErr))
    {
        printf("cc_start_seq_get error:%d\n", errCode);
        return errCode;
    }

    // look for the credential with the with the our service name
    while((errCode =  krb5_cc_next_cred(kcontext, cache, &cur, &creds)) == noErr)
    {
//        if(!(errCode = krb5_unparse_name(kcontext, creds.client, &clientName)))
//            printf("client:%s\n", clientName);
            
        if((errCode = krb5_unparse_name(kcontext, creds.server, &serverName)) == noErr)
        {
            // if this is the right credential stop seraching
            if(strcmp(serverName, CFStringGetCStringPtr(serviceName, kCFStringEncodingMacRoman)) == kCompareEqual)
            {
                krb5_ccache newFileCache;    
                
                if((errCode = krb5_cc_resolve(kcontext, path, &newFileCache)) == noErr)
                {
                    if((errCode = krb5_cc_initialize(kcontext, newFileCache, creds.client) == noErr))
                    {
                        if(errCode = krb5_cc_store_cred(kcontext, newFileCache, &creds) != noErr)
                        {
                            printf("cc_store_cred error:%d\n", errCode);
                        }
                    }
                }                
                break;
            }
            
        }        
    }
        
    // cleanup
    krb5_cc_close(kcontext, cache);
    krb5_free_context(kcontext);

    fprintf (stdout, "Write SGT To:%s with statusCode:%d\n", path, errCode);
    fflush (stdout);
    
    
    return errCode;
/*    // set permission bits to -rw-r--r--
    if(chmod("/private/tmp/printServiceTicket", S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH))
    {
        printf("chmod failed\n");    
    } */
}

// END OF SOURCE
