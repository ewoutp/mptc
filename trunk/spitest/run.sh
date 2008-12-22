#!/bin/sh
#
# $Id: run.sh,v 1.1 2002/11/27 09:22:24 epr Exp $
#

d=`dirname $0`

LD_LIBRARY_PATH=$d/linux
JEDITS_CLASSPATH=$d/linux:.:$d/jedits.jar:$d/jdom.jar:$d/xerces.jar:$d/linux/jcl.jar:$d/win-rxtx/RXTXcomm.jar

export LD_LIBRARY_PATH
java -cp $JEDITS_CLASSPATH test 


