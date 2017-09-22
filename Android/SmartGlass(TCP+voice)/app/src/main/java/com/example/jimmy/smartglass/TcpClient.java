package com.example.jimmy.smartglass;

import android.util.Log;															//	引入android.util.Log函式庫

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;												//	引入java.io.BufferedOutputStream函式庫
import java.io.BufferedReader;														//	引入java.io.BufferedReader函式庫
import java.io.BufferedWriter;														//	引入java.io.BufferedWriter函式庫
import java.io.InputStreamReader;													//	引入java.io.InputStreamReader函式庫
import java.io.OutputStreamWriter;													//	引入java.io.OutputStreamWriter函式庫
import java.io.PrintWriter;															//	引入java.io.PrintWriter函式庫
import java.net.InetAddress;														//	引入java.net.InetAddress函式庫
import java.net.Socket;																//	引入java.net.Socket函式庫

/**
 * Created by jimmy on 2017/5/19.
 */

public class TcpClient																//	TcpClient類別
{																					//	進入TcpClient類別
	//public static final String SERVER_IP = "140.124.73.201";						//	set server IP address, 設定server位址
	public static final String SERVER_IP = "192.168.0.139";							//	set server IP address, 設定server位址
	//public static final int SERVER_PORT = 16388;									//	設定server連接埠
	public static final int SERVER_PORT = 5000;										//	設定server連接埠
	// message receive from the server
	private String mServerMessage;
	//  binary data receive from the server
	private int ReceiveBinary;
	// sends message received notifications
	private OnMessageReceived mMessageListener = null;
	// while this is true, the server will continue running
	private boolean mRun = false;
	// used to send messages
	private PrintWriter mBufferOut;
	//  used to send binary data
	private BufferedOutputStream BinaryOut;
	// used to read messages from the server
	private BufferedReader mBufferIn;
	// used to read binary data from the server
	private BufferedInputStream BinaryIn;
	/**
	 * Constructor of the class. OnMessagedReceived listens for the messages received from server
	 */
	public TcpClient(OnMessageReceived listener) {
		mMessageListener = listener;
	}

	/**
	 * Sends the message entered by client to the server
	 *
	 * @param message text entered by client
	 */
	public void sendMessage(String message) {
		if (mBufferOut != null && !mBufferOut.checkError()) {
			mBufferOut.println(message);
			mBufferOut.flush();
		}
	}

	public  void sendBinary(byte[] data)
	{
		try {
			BinaryOut.write(data);
			BinaryOut.flush();
		} catch ( Exception e )
		{
			Log.e("TCP", "sendBinary Error", e);
		}
	}
	/**
	 * Close the connection and release the members
	 */
	public void stopClient() {

		mRun = false;

		if (mBufferOut != null) {
			mBufferOut.flush();
			mBufferOut.close();
		}

		mMessageListener = null;
		mBufferIn = null;
		mBufferOut = null;
		mServerMessage = null;
	}

	public void run() {

		mRun = true;

		try {
			//here you must put your computer's IP address.
			InetAddress serverAddr = InetAddress.getByName(SERVER_IP);

			Log.e("TCP Client", "C: Connecting...");

			//create a socket to make the connection with the server
			Socket socket = new Socket(serverAddr, SERVER_PORT);

			try {

				//sends the message to the server
				mBufferOut = new PrintWriter(new BufferedWriter(new OutputStreamWriter(socket.getOutputStream())), true);

				//sends the binary data to server
				BinaryOut = new BufferedOutputStream(socket.getOutputStream());

				//receives the message which the server sends back
				mBufferIn = new BufferedReader(new InputStreamReader(socket.getInputStream()));

				//receives the binary data which the server sends back
				BinaryIn = new BufferedInputStream(socket.getInputStream());

				//in this while the client listens for the messages sent by the server
				//***從server接收文字資料***
				while (mRun) {

					mServerMessage = mBufferIn.readLine();

					if (mServerMessage != null && mMessageListener != null) {
						//call the method messageReceived from MyActivity class
						mMessageListener.messageReceived(mServerMessage);
						Log.e("RESPONSE FROM SERVER", "S: Received Message: '" + mServerMessage + "'");

					}

				}
			} catch (Exception e) {

				Log.e("TCP", "S: Error", e);

			} finally {
				//the socket must be closed. It is not possible to reconnect to this socket
				// after it is closed, which means a new socket instance has to be created.
				socket.close();
			}

		} catch (Exception e) {

			Log.e("TCP", "C: Error", e);

		}

	}

	//Declare the interface. The method messageReceived(String message) will must be implemented in the MyActivity
	//class at on asynckTask doInBackground
	public interface OnMessageReceived {
		public void messageReceived(String message);
	}
}																					//	結束TcpClient類別
