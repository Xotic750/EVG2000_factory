Express Installation For The Terminally Impatient:
1.  Install klprPDE.plugin in /Library/Printers/PPD Plugins/klprPDE.plugin
    cp -r klprPDE.plugin '/Library/Printers/PPD Plugins/'
2.  Install klpr in /usr/libexec/cups/backend/klpr
    (copy lpr to /usr/libexec/cups/backend/klpr or make a symbolic
    link from /usr/libexec/cups/backend/klpr to lpr

Configuration:

add the kerberos principal names for the remote print servers in
the Settings.strings in the /Contents/Resources/ folder of the
klprPDE.plugin bundle.

FILE:

/* these strings are matched in this order
- print server  (the hostname of the print server)
- printer type
- service name
    at least one of these must match the printer or job settings
    or the PDE will not load
*/

/*
  if the hostname of the print server matches the first string
  then the use the second string as the service name*/
  Example:
  "net-print.cit.cornell.edu" = "lpr/net-print.cit.cornell.edu@CIT.CORNELL.EDU";
  "page3.cit.cornell.edu" = "lpr/page3.cit.cornell.edu@CIT.CORNELL.EDU";
  "irene.cit.cornell.edu" = "lpr/irene.cit.cornell.edu@CIT.CORNELL.EDU";
*/


/*
  if the printer type matches the first string
  then use the second string as the service name
  By default, the printer type is 'klpr'
  Example:
   "klpr" = "lpr/printservername.whatever.dom@REALM.WHATEVER.DOM";
*/

/*
  Match 'Service Name' - the default name for the service
  Example:
    "Service Name" = "lpr/printservername.whatever.dom@REALM.WHATEVER.DOM";
*/
