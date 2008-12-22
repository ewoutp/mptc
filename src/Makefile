# $Id: Makefile,v 1.1 2002/11/27 09:22:25 epr Exp $
 
compile:
	gpasm -p p16c84 -o main84c.hex main.asm
	gpasm -p p16f84 -o main84.hex main.asm
	gpasm -p p16f628 -o main628.hex main.asm
	@tar zcf mptc.tgz *.asm *.inc *.hex *.cod protocol.htm Makefile

prog84:
	prog84 -T 1684 -x main84.hex

prog84c:
	prog84 -T 1684 -x main84c.hex

prog628:
	prog84 -T 16x628 -x main628.hex

sim84:
	gpsim -s main84.cod

sim628:
	gpsim -s main628.cod

clean:
	@rm -rf *.cod *.lst *.hex mptc.tgz
