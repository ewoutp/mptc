
import gnu.io.*;
import java.io.*;
import java.util.*;

/**
 * Protocol driver for the Multi Protocol Train Controller.
 *
 * @author    epr
 * @created   August 30, 2002
 */
public class Mptc {

	/**
	 * Description of the Field
	 */
	protected final static int[] SWITCH_DATA = {0, 3, 12, 15, 48, 51, 60, 63};

	/**
	 * Description of the Field
	 */
	protected final static int[] TRIT = {0, 3, 1};
	private InputStream is;
	private OutputStream os;
	private SerialPort port;

	/**
	 * Constructor for the Mptc object
	 *
	 * @param port  Description of Parameter
	 */
	public Mptc(SerialPort port)
	throws IOException {
		this.port = port;
		is = port.getInputStream();
		os = port.getOutputStream();
	}

	/**
	 * Sets the switch attribute of the Mptc class
	 *
	 * @param nr               The new switch value
	 * @param straight         The new switch value
	 * @exception IOException  Description of Exception
	 */
	public void setSwitch(int nr, boolean straight)
			 throws IOException {
		send(144);
		int addr = address2byte(nr / 4);
		int ofs = nr % 4;
		int data = (straight ? SWITCH_DATA[ofs * 2] : SWITCH_DATA[ofs * 2 + 1]) | 192;
		System.out.println("addr=" + addr + ", data=b" + bin(data));
		send(addr);
		send(data);
	}

	/**
	 * Gets the number of the module that has a changed line since the last
	 * read of this module.
	 *
	 * @return                 The lastChanged value
	 * @exception IOException  Description of Exception
	 */
	public int getLastChanged()
			 throws IOException {
		return send(190);
	}

	/**
	 * Gets the lines of the feedback module with the given number.
	 *
	 * @param module           The number of the module 0..7
	 * @return                 The feedback value
	 * @exception IOException  Description of Exception
	 */
	public int getFeedback(int module)
			 throws IOException {
		return send(192 + module);
	}

	/**
	 * Description of the Method
	 *
	 * @param v  Description of Parameter
	 * @return   Description of the Returned Value
	 */
	public static String bin(int v) {
		String s = Integer.toBinaryString(v);
		while (s.length() < 8) {
			s = '0' + s;
		}
		return s;
	}

	/**
	 * Convert a train/switch address to an address-byte
	 *
	 * @param address  Description of Parameter
	 * @return         Description of the Returned Value
	 */
	public static int address2byte(int address) {
		int res = 0;
		int shift = 0;
		while (address != 0) {
			int part = address % 3;
			address = address / 3;
			res = res + (TRIT[part] << shift);
			shift += 2;
		}
		return res;
	}

	/**
	 * Sets the speed attribute of the Mptc class
	 *
	 * @param addr             The new speed value
	 * @param speed            The new speed value
	 * @param forward          The new speed value
	 * @param lights           The new speed value
	 * @exception IOException  Description of Exception
	 */
	public void setSpeed(int addr, int speed, boolean forward, boolean lights)
			 throws IOException {
		if (speed < 0) {
			send(1);
		} else if (speed == 0) {
			send(0);
		} else {
			send(speed + 1);
		}
		send(addr);
		int b3 = 0;
		if (forward) {
			b3 |= 0x01;
		}
		if (lights) {
			b3 |= 0x02;
		}
		send(b3);
	}

	/**
	 * Send a reset command to the controller.
	 *
	 * @exception IOException  Description of Exception
	 */
	public void reset()
	throws IOException {
		send(136);
	}

	/**
	 * Send a nop command to the controller
	 *
	 * @exception IOException  Description of Exception
	 */
	public void nop()
			 throws IOException {
		send(137);
	}

	/**
	 * Send a reset switches command to the controller.
	 *
	 * @exception IOException  Description of Exception
	 */
	public void resetSwitches()
			 throws IOException {
		send(32);
	}
	
	/**
	* Set a Configuration Value (CV) to a given value
	**/
	public void programCV(int cvReg, int cvData)
	throws IOException {
		cvReg = cvReg - 1;
		dcc3(0x70 | 0x0c | ((cvReg >> 8) & 0x03), cvReg & 0xFF, cvData);
	}

	/**
	* Set the address of the decoder.
        **/
	public void programAddress(int addr)
	throws IOException {
		dcc2(0x78, addr);
	}

	/**
	* Send a 2-byte DCC command
	**/
	public void dcc2(int addr, int data1)
	throws IOException {
		send(128);
		send(addr);
		send(data1);
	}

	/**
	* Send a 3-byte DCC command
	**/
	public void dcc3(int addr, int data1, int data2)
	throws IOException {
		send(129);
		send(addr);
		send(data1);
		send(data2);
	}

	/**
	* Send a 4-byte DCC command
	**/
	public void dcc4(int addr, int data1, int data2, int data3)
	throws IOException {
		send(130);
		send(addr);
		send(data1);
		send(data2);
		send(data3);
	}

	/**
	 * Send a single byte to the controller and receive a single byte.
	 *
	 * @param v                The byte to send
	 * @return                 The received byte
	 * @exception IOException  Description of Exception
	 */
	public int send(int v)
			 throws IOException {
		port.setRTS(true);
		while (!port.isCTS()) {
			/*
			 *  System.out.print('c');
			 */
		}
//                Thread.sleep(1);
		os.write(v);
		port.setRTS(false);
//		System.out.print('-');
		int res = is.read();
		return res;
	}
}

