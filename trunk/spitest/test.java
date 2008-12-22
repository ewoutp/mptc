import java.io.*;
import java.util.*;
import gnu.io.*;


public class test {

	public static void main(String args[])
        throws Exception {

        SerialPort port = (SerialPort)CommPortIdentifier.getPortIdentifier("COM1").open("test", 2000);
//        port.setFlowControlMode(SerialPort.FLOWCONTROL_RTSCTS_OUT | SerialPort.FLOWCONTROL_RTSCTS_IN);
        port.setFlowControlMode(SerialPort.FLOWCONTROL_NONE);
        port.setSerialPortParams(9600,
              	SerialPort.DATABITS_8,
              	SerialPort.STOPBITS_1,
                SerialPort.PARITY_NONE);

		Mptc mptc = new Mptc(port);
		
        // Initialize by sending a single NOP
		mptc.nop();

        BufferedReader in = new BufferedReader(new InputStreamReader(System.in));

        int addr = 0;
        int speed = 0;
        boolean forward = true;
        boolean lights = true;

        while (true) {
        	System.out.print("a(" + addr + ") s(" + speed + ") d(" + (forward ? "->" : "<-") + ") l(" + (lights ? "on" : "off") + "): ");
        	String line = in.readLine().trim();
                StringTokenizer tok = new StringTokenizer(line);
                String cmd = tok.hasMoreElements() ? tok.nextToken() : "";
                if (cmd.equals("a") && tok.hasMoreTokens()) {
                	addr = parseInt(tok.nextToken());
	                mptc.setSpeed(addr, speed, forward, lights);
                } else if (cmd.equals("d")) {
                	forward = !forward;
	                mptc.setSpeed(addr, speed, forward, lights);
                } else if (cmd.equals("dcc2") && tok.countTokens() == 1) {
                	int d1 = parseInt(tok.nextToken());
                        mptc.dcc2(addr, d1);
                } else if (cmd.equals("dcc3") && tok.countTokens() == 2) {
                	int d1 = parseInt(tok.nextToken());
                	int d2 = parseInt(tok.nextToken());
                        mptc.dcc3(addr, d1, d2);
                } else if (cmd.equals("dcc4") && tok.countTokens() == 3) {
                	int d1 = parseInt(tok.nextToken());
                	int d2 = parseInt(tok.nextToken());
                	int d3 = parseInt(tok.nextToken());
                        mptc.dcc4(addr, d1, d2, d3);
                } else if ((cmd.length() > 0) && Character.isDigit(cmd.charAt(0))) {
                	speed = Math.min(14, parseInt(cmd));
	                mptc.setSpeed(addr, speed, forward, lights);
                } else if (cmd.equals("fb") && tok.hasMoreTokens()) {
                	int mod = parseInt(tok.nextToken());
                    System.out.println("Module " + mod + " = b" + bin(mptc.getFeedback(mod)));
                } else if (cmd.equals("l")) {
                	lights = !lights;
	                mptc.setSpeed(addr, speed, forward, lights);
                } else if (cmd.equals("lc")) {
                	System.out.println("Last changed: " + mptc.getLastChanged());
                } else if (cmd.equals("n")) {
                	mptc.nop();
		} else if (cmd.equals("pcv") && tok.countTokens() == 2) {
			int cvReg = parseInt(tok.nextToken());
			int cvData = parseInt(tok.nextToken());
			mptc.programCV(cvReg, cvData);
		} else if (cmd.equals("pa") && tok.countTokens() == 1) {
			int a = parseInt(tok.nextToken());
			mptc.programAddress(a);
                } else if (cmd.equals("r")) {
                	addr = 0;
                    speed = 0;
					forward = true;
                    lights = true;
                    mptc.reset();
                } else if (cmd.startsWith("rs")) {
                	mptc.resetSwitches();
                } else if (cmd.equals("s")) {
                	addr = 0;
                    speed = -1; // Emergency stop
	                mptc.setSpeed(addr, speed, forward, lights);
                } else if (cmd.equals("so") && tok.hasMoreTokens()) {
                	mptc.setSwitch(parseInt(tok.nextToken()), false);
                } else if (cmd.equals("ss") && tok.hasMoreTokens()) {
                	mptc.setSwitch(parseInt(tok.nextToken()), true);
                } else if (cmd.startsWith("q")) {
                	mptc.setSpeed(0, 0, true, lights);
                	System.exit(0);
                } else {
                	System.out.println("Usage: <cmd> <option> ...");
                	System.out.println("\ta <addr>             - Set address");
                	System.out.println("\td                    - Toggle direction");
                	System.out.println("\tdcc2 <d1>            - Send 2 byte dcc command (used address)");
                	System.out.println("\tdcc3 <d1> <d2>       - Send 3 byte dcc command (used address)");
                	System.out.println("\tdcc4 <d1> <d2> <d3>  - Send 4 byte dcc command (used address)");
	                System.out.println("\tfb <0..7>            - Read feedback module");
                	System.out.println("\tl                    - Toggle lights");
                	System.out.println("\t0..14                - Set speed");
                	System.out.println("\tlc                   - Get last changed feedback module");
                	System.out.println("\tn                    - NOP");
                	System.out.println("\tpcv <cvreg> <cvdata> - Program CV");
                	System.out.println("\tpa <addr>            - Program Address");
                	System.out.println("\tr                    - Reset controller");
                    	System.out.println("\trs                   - Reset switches");
                	System.out.println("\ts                    - Stop all");
                	System.out.println("\tso <0..63>           - Switch off");
                	System.out.println("\tss <0..63>           - Switch straight");
                	System.out.println("\tq");
                }
        }
    }

    public static String bin(int v) {
    	String s = Integer.toBinaryString(v);
        while (s.length() < 8) { s = '0' + s; }
        return s;
    }
    
    public static int parseInt(String v) {
    	v = v.trim().toUpperCase();
        if (v.startsWith("B")) {
        	return Integer.parseInt(v.substring(1), 2);
        } else if (v.startsWith("0X")) {
        	return Integer.parseInt(v.substring(2), 16);
        } else {
        	return Integer.parseInt(v);
        }
    }
}
