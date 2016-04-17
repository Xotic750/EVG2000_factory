var HelpOptionsVar = "width=480,height=420,scrollbars,toolbar,resizable,dependent=yes";
var GlossOptionsVar = "width=420,height=180,scrollbars,toolbar,resizable,dependent=yes";
var bigsub   = "width=540,height=440,scrollbars,menubar,resizable,status,dependent=yes";
var smallsub = "width=440,height=320,scrollbars,resizable,dependent=yes";
var sersub   = "width=500,height=380,scrollbars,resizable,status,dependent=yes";
var memsub   = "width=630,height=320,scrollbars,menubar,resizable,status,dependent=yes";
var helpWinVar = null;
var glossWinVar = null;
var datSubWinVar = null;
var ValidStr = 'abcdefghijklmnopqrstuvwxyz-';
var ValidStrUpper = ValidStr.toUpperCase();
var ValidStr_ddns = 'abcdefghijklmnopqrstuvwxyz-1234567890';
var hex_str = "ABCDEFabcdef0123456789";
var invalidMSNameStr = "\"/\\[<]>.:;,|=+*?";
var symbol = invalidMSNameStr + "#!@$%^&()";
var symbol_space = symbol + " ";

var numericStr = "0123456789";
var hexVals = new Array("0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
              "A", "B", "C", "D", "E", "F");
var unsafeString = "\"<>%\\^[]`\+\$\,'#&";
// deleted these chars from the include list ";", "/", "?", ":", "@", "=", "&" and #
// so that we could analyze actual URLs
if (typeof Array.prototype.splice == "undefined") {
	Array.prototype.splice = Array_splice
}

//remove specified index
function Array_splice(index, delTotal) { // ex: (5, 2) --> delete 2 contiguos items start from index 5
  var temp = new Array()
  var response = new Array()
  var A_s = 0
  for (A_s = 0; A_s < index; A_s++) {
   temp[temp.length] = this[A_s]
   }
  for (A_s = 2; A_s < arguments.length; A_s++) {
   temp[temp.length] = arguments[A_s]
   }
  for (A_s = index + delTotal; A_s < this.length; A_s++) {
   temp[temp.length] = this[A_s]
   }
  for (A_s = 0; A_s < delTotal; A_s++) {
   response[A_s] = this[index + A_s]
   }
  this.length = 0
  for (A_s = 0; A_s < temp.length; A_s++) {
   this[this.length] = temp[A_s]
   }
  return response
}

function isArray() {
	if (typeof arguments[0] == 'object') {  
		var criterion = arguments[0].constructor.toString().match(/array/i); 
		 return (criterion != null);  
	}
	 return false;
 }


function isString() {
	if (typeof arguments[0] == 'string') return true;
	if (typeof arguments[0] == 'object') {
		var criterion =  arguments[0].constructor.toString().match(/string/i); 
		 return (criterion != null);  
	}
	return false;
}



function tmdashboard()
{
    window.open("setup.cgi?todo=save&next_file=tmdashboard.htm&this_file=tmservice.htm&h_tmss=enable&h_tmp_enable=always");
    //window.open("http://tmss.trendmicro.com/dashboard/dashboard.aspx?ABCDEDFGBFB5A72BCE20DEACDERYYYHUCUUIO", "tmd", "width=800, height=550, left=10, top=10, scrollbars=yes, toolbar=no, resizable=yes, menubar=no, status=yes");

}


//  display
var showit = "block";
var hideit = "none";

function setDisplay(el,shownow)  // IE & NS6; shownow = true, false
{
	if (document.all)
		document.all(el).style.display = (shownow) ? showit : hideit ;
	else if (document.getElementById)
		document.getElementById(el).style.display = (shownow) ? showit : hideit ;
}



function showMsg()
{
	var msgVar=document.forms[0].message.value;
	if (msgVar.length > 1) 
		alert(msgVar);
}

function closeWin(win_var)
{
	if(document.layers)
		return;
	if ( ((win_var != null) && (win_var.close)) || ((win_var != null) && (win_var.closed==false)) )
		win_var.close();
}

function openHelpWin(file_name)
{
   helpWinVar = window.open(file_name,'help_win',HelpOptionsVar);
   if (helpWinVar.focus)
		setTimeout('helpWinVar.focus()',200);
}

function openGlossWin()
{
	glossWinVar = window.open('','gloss_win',GlossOptionsVar);
	if (glossWinVar.focus)
		setTimeout('glossWinVar.focus()',200);
}

function openDataSubWin(filename,win_type)
{
	closeWin(datSubWinVar);
	datSubWinVar = window.open(filename,'datasub_win',win_type);
	if (datSubWinVar.focus)
		setTimeout('datSubWinVar.focus()',200); 
}

function closeSubWins()
{
	closeWin(helpWinVar);
	closeWin(glossWinVar);
	closeWin(datSubWinVar);
}


function checkBlank(fieldObj, fname)
{
	var msg = "";	
	if (fieldObj.value.length < 1){	
		msg = addstr(msg_blank,fname);
        }
	return msg;
}

function checkNoBlanks(fObj, fname)
{
	var space = " ";
 	if (fObj.value.indexOf(space) >= 0 )
			return msg_space;
	else return "";
}

function hasSpaces(str){
	var space = " ";
 	if (fObj.value.indexOf(space) >= 0 )
		return true;
	return false;
}

function checkValid(text_input_field, field_name, Valid_Str, max_size, mustFill)
{
	var error_msg= "";
	var size = text_input_field.value.length;
	var str = text_input_field.value;

	if ((mustFill) && (size != max_size) )
		error_msg = addstr(msg_blank_in,field_name);

//	if(str.charAt(0) == '0')
//	{	
//		error_msg = addstr(msg_outofrange,field_name,"100","FFF");
//		error_msg += "\n";
//		return error_msg;
//	}

 	for (var i=0; i < size; i++)
  	{
    	if (!(Valid_Str.indexOf(str.charAt(i)) >= 0))
    	{
			error_msg = addstr(msg_invalid,field_name,Valid_Str);
			break;
    	}
  	}
  	return error_msg;
}

function isValidStr(inputStr,validChars,len)  // returns true or false, no msg
{
	if (len <= 0) // no set length
		;
	else if (inputStr.length != len)
		return false;
    for(i=0; i < inputStr.length; i++) 
	{
        var c = inputStr.charAt(i);
		if (validChars.indexOf(c) == -1 )
			return false;
    }
    return true;
}

function isInvalidStr(fobj,invalidChars,fname)  // returns true or false, no msg
{
    var error_msg = addstr(msg_invalid2,fname);
    var ValidStr = 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789';

    var inputStr = fobj.value;
    if(inputStr.length==0){
		fobj.focus();
		alert(error_msg);
		return true;
    }
    for(i=0; i < inputStr.length; i++) 
	{
        var c = inputStr.charAt(i);
		if (invalidChars.indexOf(c) >=0 ){
			fobj.focus();
			alert(error_msg);
			return true;
		}
		/*if (ValidStr.indexOf(c) == -1 ){
			fobj.focus();
			alert(error_msg);
			return true;
		}*/
   	 }
    return false;
}

function isValidRangeField(startobj,stopobj){
	var str_start = startobj.value;
	var str_stop = stopobj.value;
	var start, stop;
	var msg = checkInt(startobj, "Starting Port", 0, 65535, true);
	if (msg.length > 1)	{
		alert(msg);
		startobj.focus();
		return false;
	}

	start = parseInt(str_start);
	stop = parseInt(str_stop);
	if(str_stop != ""){
		if(start < 0 || start > stop || start > 65535){
			alert("Port End must be greater than Starting Port and less than 65535.");
			return false;
		}
	}
	else{//end point is either empty or invalid
		return true;
	}

	return true;
}

function checkInt(text_input_field, field_name, min_value, max_value, required)
// NOTE: Doesn't allow negative numbers, required is true/false
{
	var str = text_input_field.value;
	var error_msg= "";
	
	if (text_input_field.value.length==0) // blank
	{
		if (required)
			error_msg = addstr(msg_blank,field_name);
	}
	else // not blank, check contents
	{
		for (var i=0; i < str.length; i++)
		{
			if ((str.charAt(i) < '0') || (str.charAt(i) > '9'))
				error_msg = addstr(msg_check_invalid,field_name);
		}
		if (error_msg.length < 2) // don't parse if invalid
		{
			var int_value = parseInt(str);
			if (int_value < min_value || int_value > max_value)
				error_msg = addstr(msg_outofrange,field_name,min_value,max_value);
		}
	}
	if (error_msg.length > 1)
		error_msg = error_msg + "\n";
	return(error_msg);
}


function sumvalue(F)
{    
	alert("Called sumvalue(), but should not.\nUse badMacField instead");

}
function MACAddressBlur(address)
{
	alert("Called MACAddressBlur(), but should not.\nUse badMacField instead");
}

// =============================   msg.js
//public message
var msg_blank = "%s can not be blank.\n";
var msg_space = "Blanks or spaces are not allowed in %s\n";
var msg_blank_in = "Blanks are not allowed in %s\n";
var msg_invalid = "\nInvalid character or characters in %s\nValid characters are: \n%s\n\n";
var msg_invalid2 = "\nInvalid character or characters in %s\n";
var msg_check_invalid = "%s contains an invalid number.";
var msg_greater = "%s must be greater than %s";
var msg_less = "%s must be less than %s";
var msg_outofrange = "%s is out of range [%s ~ %s]";
var msg_invalid_range = "%s is invalid.";

var msg_first = "First";
var msg_second = "Second";
var msg_third = "Third";
var msg_fourth = "Fourth";

function addstr(input_msg)
{
	var last_msg = "";
	var str_location;
	var temp_str_1 = "";
	var temp_str_2 = "";
	var str_num = 0;
	temp_str_1 = addstr.arguments[0];
	while(1)
	{
		str_location = temp_str_1.indexOf("%s");
		if(str_location >= 0)
		{
			str_num++;
			temp_str_2 = temp_str_1.substring(0,str_location);
			last_msg += temp_str_2 + addstr.arguments[str_num];
			temp_str_1 = temp_str_1.substring(str_location+2,temp_str_1.length);
			continue;
		}
		if(str_location < 0)
		{
			last_msg += temp_str_1;
			break;
		}
	}
	return last_msg;
}

// =============================   browser.js

function isIE(){
    var browser = new Object();
    browser.version = parseInt(navigator.appVersion);
    browser.isNs = false;
    browser.isIe = false;
    if(navigator.appName.indexOf("Netscape") != -1)
        browser.isNs = true;
    else if(navigator.appName.indexOf("Microsoft") != -1)
        browser.isIe = true;
    if(browser.isNs) return false;
    else if (browser.isIe) return true;
}

function add(out,in1,in2,in3,in4) {
    var Total;
    Total=in1.value+"."+in2.value+"."+in3.value+"."+in4.value;
    out.value=Total; 
}

function load4(Mydata,ip1,ip2,ip3,ip4) {
    var len; var ad; var temp;
    var Myall;
    Myall=Mydata.value;    //ip1 
    len=Myall.length;
    temp=Myall.indexOf(".");
    ad=Myall.substring(0,temp); 
    ip1.value=ad;
    //ip2 
    Myall=Myall.substring(temp+1,len);
    len=Myall.length;
    temp=Myall.indexOf(".");
    ad=Myall.substring(0,temp);
    ip2.value=ad;
    //ip3 
    Myall=Myall.substring(temp+1,len);
    len=Myall.length;
    temp=Myall.indexOf(".");
    ad=Myall.substring(0,temp);
    ip3.value=ad;
    //ip4 
    Myall=Myall.substring(temp+1,len);
    ad=Myall; ip4.value=ad;
} 

// =============================   utility.js
var msg_invalid_ip = "Invalid IP, please enter again!\n";
function isIPaddr(addr) {
    var i;
    var a;
    if(addr.split) {	
        a = addr.split(".");
    }else {	
        a = cdisplit(addr,".");
    }
    if(a.length != 4) {
        return false;
    }
    for(i = 0; i<a.length; i++) {
        var x = a[i];
        if( x == null || x == "" || !isNumeric(x) || x<0 || x>255 ) {
            return false;
        }
    }
    return true;
}

function isIntStr(str) {
    var i;
    for(i = 0; i<str.length; i++) {
        var c = str.substring(i, i+1);
        if("0" <= c && c <= "9") {
            continue;
        }
        return false;
    }
    return true;
}

function isHexStr(str) {
    var i;
    for(i = 0; i<str.length; i++) {
        var c = str.substring(i, i+1);
        if(("0" <= c && c <= "9") || ("a" <= c && c <= "f") || ("A" <= c && c <= "F")) {
            continue;
        }
        return false;
    }
    return true;
}

function checkMacStr(macField) 
{
	var MacStrOrg = macField.value;
	var MacStrNoSep = MacStrOrg;
	var macArray; var c; var i;

	if(MacStrOrg.indexOf(":") > -1)
		c = ":";
	else if(MacStrOrg.indexOf("-") > -1)
		c = "-";
	else 
		return isValidStr(MacStrOrg,hex_str,12);
	macArray = MacStrOrg.split(c);
	if(macArray.length != 6)
		return false;
	for ( i = 0; i < macArray.length; i++)
	{
		if (macArray[i].length != 2 )
			return false;
	}
	MacStrNoSep =  MacStrNoSep.replace(/:/g,"");
	MacStrNoSep =  MacStrNoSep.replace(/-/g,"");
	return isValidStr(MacStrNoSep,hex_str,12);
}

function checkMac(macstr) 
{
	var MacStrOrg = macstr;
	var MacStrNoSep = MacStrOrg;
	var macArray; var c; var i;

	if(MacStrOrg.indexOf(":") > -1)
		c = ":";
	else if(MacStrOrg.indexOf("-") > -1)
		c = "-";
	else 
		return isValidStr(MacStrOrg,hex_str,12);
	macArray = MacStrOrg.split(c);
	if(macArray.length != 6)
		return false;
	for ( i = 0; i < macArray.length; i++)
	{
		if (macArray[i].length != 2 )
			return false;
	}
	MacStrNoSep =  MacStrNoSep.replace(/:/g,"");
	MacStrNoSep =  MacStrNoSep.replace(/-/g,"");
	return isValidStr(MacStrNoSep,hex_str,12);
}



/* Check IP Address Format*/
function checkIPMain(ip,max) {
    if( isNumeric(ip, max) ) {
        ip.focus();
        return true;
    }
}
function badLANIP(ip1, ip2, ip3, ip4,max) {
    if(checkIPMain(ip1,223)) return true; 
    if(checkIPMain(ip2,255)) return true;
    if(checkIPMain(ip3,255)) return true;
    if(checkIPMain(ip4,max)) return true;
    if((parseInt(ip1.value)==0)||(parseInt(ip1.value)==0)&&(parseInt(ip2.value)==0)&&(parseInt(ip3.value)==0)&&(parseInt(ip4.value)==0))
    	return true;
    return false;
}

function badIP(ip1, ip2, ip3, ip4,max) {
    if(checkIPMain(ip1,255)) return true; 
    if(checkIPMain(ip2,255)) return true;
    if(checkIPMain(ip3,255)) return true;
    if(checkIPMain(ip4,max)) return true;
    if((parseInt(ip1.value)==0)||(parseInt(ip1.value)==0)&&(parseInt(ip2.value)==0)&&(parseInt(ip3.value)==0)&&(parseInt(ip4.value)==0))
    	return true;
    return false;
}
function badMask(ip1, ip2, ip3, ip4)
{
	if(badIP(ip1, ip2, ip3, ip4,255))
		return true;
	
	if(validate(ip1.value)) return true;
	if((ip1.value <255)&&(ip2.value>0)) return true;
	
	if(validate(ip2.value)) return true;
	if((ip2.value <255)&&(ip3.value>0)) return true;
	
	if(validate(ip3.value)) return true;
	if((ip3.value <255)&&(ip4.value>0)) return true;
	
	if(validate(ip4.value)) return true;
}
function validate(sec)
{
	var mask1=128;
	for(var i=0;i<8;i++)
	{
		if((sec>=mask1))
		{
			sec -= mask1;
			mask1 /= 2;
		}
		else
			if(sec!=0)
			return true;
	}
}

/* Check Numeric*/
function isNumeric(str, max) {
    if(str.value.length == 0 || str.value == null || str.value == "") {
        str.focus();
        return true;
    }
    
    var i = parseInt(str.value);
    if(i>max) {
        str.focus();
        return true;
    }
    for(i=0; i<str.value.length; i++) {
        var c = str.value.substring(i, i+1);
        if("0" <= c && c <= "9") {
            continue;
        }
        str.focus();
        return true;
    }
    return false;
}

/* Check Blank*/
function isBlank(str) {
    if(str.value == "") {
        str.focus();
        return true;
    } else 
        return false;
}

/* Check Phone Number*/
function isPhonenum(str) {
    var i;
    if(str.value.length == 0) {
        str.focus();
        return true;
    }
    for (i = 0; i<str.value.length; i++) {
        var c = str.value.substring(i, i+1);
        if (c>= "0" && c <= "9")
            continue;
        if ( c == '-' && i !=0 && i != (str.value.length-1) )
            continue;
        if ( c == ',' ) continue;
        if (c == ' ') continue;
        if (c>= 'A' && c <= 'Z') continue;
        if (c>= 'a' && c <= 'z') continue;
        str.focus();
        return true;
    }
    return false;
}

/* 0:close 1:open*/
function openHelpWindow(filename) {
    helpWindow = window.open(filename,"thewindow","width=300,height=400,scrollbars=yes,resizable=yes,menubar=no");
}

function checkSave() {
    answer = confirm("Did you save this page?");
    if (answer !=0) {
        return true;
    } else return false;
}

function alertPassword(formObj) {
    alert("Re-Confirm the password!");
    formObj.focus();
}
function isEqual(cp1,cp2)
{
	if(parseInt(cp1.value) == parseInt(cp2.value))
	{
		cp2.focus();
		return true;
	}	
	else return false;
}
function setDisabled(OnOffFlag,formFields)
{
	for (var i = 1; i < setDisabled.arguments.length; i++)
		setDisabled.arguments[i].disabled = OnOffFlag;
}

function cp_ip(from1,from2,from3,from4,to1,to2,to3,to4)
//true invalid from and to ip;  false valid from and to ip;
{
    var total1 = 0;
    var total2 = 0;
    
    total1 += parseInt(from4.value,10);
    total1 += parseInt(from3.value,10)*256;
    total1 += parseInt(from2.value,10)*256*256;
    total1 += parseInt(from1.value,10)*256*256*256;
    
    total2 += parseInt(to4.value,10);
    total2 += parseInt(to3.value,10)*256;
    total2 += parseInt(to2.value,10)*256*256;
    total2 += parseInt(to1.value,10)*256*256*256;
    if(total1 > total2)
        return true;
    return false;
}

function pi(val)
{
    return parseInt(val,10);
}    
function alertR(str)    
{
    alert(str);
    return false;
}    



function blankIP(f1,f2,f3,f4) // true if 0 or blank
{
	if( containStr(f1,"") && containStr(f2,"") && containStr(f3,"") && containStr(f4,"") )
		return true;
	if( containStr(f1,"0") && containStr(f2,"0") && containStr(f3,"0") && containStr(f4,"0") )
		return true;
	return false;
}


function containStr(fn,str) 
{
	return  (fn.value == str);
}


// =====  end  utility.js


function checkIntStr(str,minval,maxval)
{
	if(!(isIntStr(str)))
		return false;	
	var ival = parseInt(str);
	if(ival < minval || ival > maxval)
		return false;
	else return true;
}	
		

function isIpStr(addrStr) 
{
    var i;
    var a = addrStr.split(".");
    if(a.length != 4) 
        return false;
    if(checkIntStr(a[0],127,127))
		return false;
    if(checkIntStr(a[0],224,239))
		return false;
    if(!checkIntStr(a[0],1,254))
		return false;
    if(!checkIntStr(a[1],0,255))
		return false;
    if(!checkIntStr(a[2],0,255))
		return false;
    if(!checkIntStr(a[3],1,254))
		return false;
    if(checkIntStr(a[0],255,255) && checkIntStr(a[1],255,255) && checkIntStr(a[2],255,255) && checkIntStr(a[3],255,255))
		return false;
    return true;
}

// -1 for error
//  0  for ip
//  1 for doamin name

function formatType(addrStr)
{
    var pos = addrStr.indexOf("..");
    if(pos != -1)
	return -1; 
    var a = addrStr.split(".");
    if(a.length < 2)
		return false;
    var allInt = true;
    for(var i=0; i < a.length; i++)
    {
         if(!isIntStr(a[i]))
		allInt = false;
    }
    if(allInt)
    {
	if(a.length == 4)
		return 0; //ip
	return -1;//incorrect ip
    }
    return 1; // domain
}

function isValidStr(str)
{
	for (var i=0; i < str.length; i++)
	{
		if ( (str.charAt(i) >= '0') && (str.charAt(i) <= '9') )
                                 continue;
		else if ((str.charAt(i) >= 'a') && (str.charAt(i) <= 'z'))
                                 continue;
		else if ((str.charAt(i) >= 'A') && (str.charAt(i) <= 'Z'))
                                 continue;
		else if(str.charAt(i) == '-')
                                 continue;
		else if(str.charAt(i) == '_')
                                 continue;
		else
			return false;
	}
	return true;
}

function isDomainName(str)
{
	var a = str.split(".");
	if(a.length < 2)
		return false; 
	for (var i=0; i < a.length; i++)
	{
		if(isValidStr(a[i]) == false)
			return false;
	}
	return true;
}



function chkIP_MSname(str) // input could be ip or name
{
	var type = formatType(str);
	if(type == -1)
		return false;
	else if(type == 0)
	{
		return isIpStr(str);
	}
	else
	{
	//maybe it is a domain name
		return isDomainName(str);
	}
}

function isEmailAddr(str)
{
	var pos = str.indexOf("@");
	if(pos == -1)
		return false;
	var userName = str.substr(0, pos);
	var serverName = str.substr(pos + 1);
	if(isValidStr(userName) == false)
		return false;
	if(chkIP_MSname(serverName) == false)
		return false;
	return true;
}

/* check mac */
function checkmac6to1(mac1,mac2,mac3,mac4,mac5,mac6)
{
	var macaddr;
	if (mac1.value.length>0)
	{ macaddr = mac1.value+":"+mac2.value+":"+mac3.value+":"+mac4.value+":"+mac5.value+":"+mac6.value; } 
	else
	{ macaddr=""; }
	
	var MacStrOrg = macaddr;
	var MacStrNoSep = MacStrOrg;
	var macArray; var c; var i;

	if(MacStrOrg.indexOf(":") > -1)
	c = ":";
	else
	return isValidStr(MacStrOrg,hex_str,12);
	macArray = MacStrOrg.split(c);
	
	if(macArray.length != 6)
		return false;
	for ( i = 0; i < macArray.length; i++)
	{
		
		if (macArray[i].length != 2 )
			return false;
	}
	MacStrNoSep =  MacStrNoSep.replace(/:/g,"");
	return isValidStr(MacStrNoSep,hex_str,12);
} 

function mac6in1(mac1,mac2,mac3,mac4,mac5,mac6)
{
	var macaddr;
	if (mac1.value.length>0)
	{ macaddr = mac1.value+":"+mac2.value+":"+mac3.value+":"+mac4.value+":"+mac5.value+":"+mac6.value; } 
	else
	{ macaddr=""; }
} 
function encodeUrl(val)
{
   var len     = val.length;
   var i       = 0;
   var newStr  = "";
   var original = val;
                                                                                
   for ( i = 0; i < len; i++ ) {
      if ( val.substring(i,i+1).charCodeAt(0) < 255 ) {
         // hack to eliminate the rest of unicode from this
         if (isUnsafe(val.substring(i,i+1)) == false)
            newStr = newStr + val.substring(i,i+1);
         else
            newStr = newStr + convert(val.substring(i,i+1));
      } else {
         // woopsie! restore.
         alert ("Found a non-ISO-8859-1 character at position: " + (i+1) +
",\nPlease eliminate before continuing.");
         newStr = original;
         // short-circuit the loop and exit
         i = len;
      }
   }
   return newStr;
}


function isValidIpAddress(address) {
   var i = 0;

   if ( address == '0.0.0.0' ||
        address == '255.255.255.255' )
      return false;

   addrParts = address.split('.');
   if ( addrParts.length != 4 ) return false;
   for (i = 0; i < 4; i++) {
      if (isNaN(addrParts[i]) || addrParts[i] =="")
         return false;
      num = parseInt(addrParts[i]);
      if ( num < 0 || num > 255 )
         return false;
   }
   return true;
}


function isValidMacAddress(address) {
   var c = '';
   var i = 0, j = 0;

   if ( address == 'ff:ff:ff:ff:ff:ff' ) return false;

   addrParts = address.split(':');
   if ( addrParts.length != 6 ) return false;

   for (i = 0; i < 6; i++) {
      if ( addrParts[i] == '' )
         return false;
      for ( j = 0; j < addrParts[i].length; j++ ) {
         c = addrParts[i].toLowerCase().charAt(j);
         if ( (c >= '0' && c <= '9') ||
              (c >= 'a' && c <= 'f') )
            continue;
         else
            return false;
      }
   }

   return true;
}


/*Frederick, 060731 move some functions here*/
//Function Name:isOverlapModemIp(EndIp, StartIp, ModemIp)
//Description: Check if the StartIp and EndIp is overlapping ModemIp
//Parameters: EndIp, StartIp, ModemIp
//output: true - no error	
//		  false - error
function isOverlapModemIp(EndIp, StartIp, ModemIp)
{
   addrEnd = EndIp.split('.');
   addrStart = StartIp.split('.');
   addrModem = ModemIp.split('.');
   for(i=2;i>=0;i--)
   	{
   		E = parseInt(addrEnd[i],10) + 1;
    		S = parseInt(addrStart[i],10) + 1;
		M = parseInt(addrModem[i],10) + 1;
		if((S!=M) || (M!=E))
			return false;
		
   	}
	E = parseInt(addrEnd[3],10) + 1;
    S = parseInt(addrStart[3],10) + 1;
	M = parseInt(addrModem[3],10) + 1;

	//it is assumed that end ip and start ip lie in the same subnet as checked by previous validation
	//check that modem ip it doesn't lie within ip range
	
	if ((S<=M) && (M<=E))
		return true;
	else
		return false;
}



function isUnsafe(compareChar)
// this function checks to see if a char is URL unsafe.
// Returns bool result. True = unsafe, False = safe
{
   if ( unsafeString.indexOf(compareChar) == -1 && compareChar.charCodeAt(0) >
32
        && compareChar.charCodeAt(0) < 123 )
      return false; // found no unsafe chars, return false
   else
      return true;
}
function convert(val)
// this converts a given char to url hex form
{
   return  "%" + decToHex(val.charCodeAt(0), 16);
}
function decToHex(num, radix)
// part of the hex-ifying functionality
{
   var hexString = "";
   while ( num >= radix ) {
      temp = num % radix;
      num = Math.floor(num / radix);
      hexString += hexVals[temp];
   }
   hexString += hexVals[num];
   return reversal(hexString);
}
function reversal(s)
// part of the hex-ifying functionality
{
   var len = s.length;
   var trans = "";
   for (i = 0; i < len; i++)
      trans = trans + s.substring(len-i-1, len-i);
   s = trans;
   return s;
}

var msg_cwmp_pinterval="Periodic Inform Interval";
var msg_cwmp_cpeport="CPE Port for ACS Access";
var msg_cwmp_acsurl="Invalid ACS URL (Must use 'http://' or 'https://' as prefix).\n";
var msg_cwmp_ptime_year="Periodic Inform Time (Year)";
var msg_cwmp_ptime_month="Periodic Inform Time (Month)";
var msg_cwmp_ptime_day="Periodic Inform Time (Day)";
var msg_cwmp_ptime_hour="Periodic Inform Time (Hour)";
var msg_cwmp_ptime_minute="Periodic Inform Time (Minute)";
var msg_cwmp_ptime_second="Periodic Inform Time (Second)";

var showit = "block";
var hideit = "none";

function show_hide(el,shownow)  // IE & NS6; shownow = true, false
{
//	alert("el = " + el);
	if (document.all)
		document.all(el).style.display = (shownow) ? showit : hideit ;
	else if (document.getElementById)
		document.getElementById(el).style.display = (shownow) ? showit : hideit ;
}

function isValidSsid(str) {
    var ValidStr = 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789';
    var i;
    var c;
    for(i=0; i < str.length; i++) 
   {
       c = str.charAt(i);
	if (ValidStr.indexOf(c) == -1 )
        	return alertR("The ssid should be made up of alphanumeric characters!");
    }
    return true;
}
function isValidPcname(str) {
    var ValidStr = 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789';
    var i;
    var c;
    for(i=0; i < str.length; i++) 
   {
       c = str.charAt(i);
	if (ValidStr.indexOf(c) == -1 )
        	return alertR("The  name should be made up of alphanumeric characters!");
    }
    return true;
}

function isValidNum(str) {
    var ValidStr = '0123456789';
    var i;
    var c;
    for(i=0; i < str.length; i++) 
   {
       c = str.charAt(i);
	if (ValidStr.indexOf(c) == -1 )
        	return alertR("All the  input should be made up of number!");
    }
    return true;
}

