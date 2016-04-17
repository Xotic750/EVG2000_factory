version=LPRng-${VERSION}
PATH=$PATH:$FILTER_DIR
export PATH
progs="lp cancel lpq lprm lpr lpstat lpc lpd lpf lpbanner pclbanner psbanner checkpc lprng_certs lprng_index_certs "
usage(){
	cat <<EOF
use: $0 [-r][-f][version]
  find lpr client programs with an older LPRng version
  or which are not LPRng
   -r  - remove them
   -f  - remove any that are found
EOF
	exit 1;
}
force= remove=
while [ "$*" ] ; do
	case "$1" in
		-f ) force=1;;
		-r ) remove=1;;
		-* ) usage;;
	esac
	shift
done
if [ -n "$1" ] ; then
	version=$1;
	shift;
fi
if [ -n "$1" ] ; then usage; fi

for i in $progs ; do
	p=`which $i 2>/dev/null`;
	echo "Program $i '$p'"
	if [ -n "$p" ] ; then
		found=
		if [ -n "$force" ] ; then
			echo "REMOVING $p"
			rm -f $p; continue; fi
		$p -V </dev/null 2>&1 |grep "$version" >/dev/null && found=1;
		if [ -z "$found" ] ; then
			echo trying -v
			$p -v </dev/null 2>&1 |grep "$version" >/dev/null && found=1;
		fi
		if [ -z "$found" ] ; then
			echo strings
			strings $p 2>&1 | grep "$version" >/dev/null && found=1 
		fi
		if [ -z "$found" ] ; then
			echo BAD $p;
			if [ -n "$remove" ] ; then
				echo "REMOVING $p"
				rm -f $p; continue; fi
		fi
	fi
done
