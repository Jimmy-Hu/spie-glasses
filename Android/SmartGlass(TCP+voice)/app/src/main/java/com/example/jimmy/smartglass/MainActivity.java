//	SmartGlass智慧眼鏡APP
//	參考資料：
//	http://kuosun.blogspot.tw/2013/12/androidosnetworkonmainthreadexception.html
//	http://stackoverflow.com/questions/12726377/java-printwriter-not-sending-byte-array

package com.example.jimmy.smartglass;

import android.app.Activity;															//	引入android.app.Activity函式庫
import android.media.AudioManager;                                                      //  引入android.media.AudioManager函式庫
import android.media.MediaPlayer;                                                       //  引入android.media.MediaPlayer函式庫
import android.os.AsyncTask;															//	引入android.os.AsyncTask函式庫
import android.os.Bundle;																//	引入android.os.Bundle函式庫
import android.view.View;																//	引入android.view.View函式庫
import android.widget.Button;															//	引入android.widget.Button函式庫
import android.widget.EditText;															//	引入android.widget.EditText函式庫
import android.widget.TextView;															//	引入android.widget.TextView函式庫
import android.view.View.OnClickListener;												//	引入android.view.View.OnClickListener函式庫
import android.support.v7.app.AppCompatActivity;										//	引入android.support.v7.app.AppCompatActivity函式庫

import java.io.BufferedReader;															//	引入java.io.BufferedReader函式庫
import java.io.BufferedWriter;															//	引入java.io.BufferedWriter函式庫
import java.io.DataOutputStream;														//	引入java.io.DataOutputStream函式庫
import java.io.IOException;																//	引入java.io.IOException函式庫
import java.io.InputStreamReader;														//	引入java.io.InputStreamReader函式庫
import java.io.OutputStreamWriter;														//	引入java.io.OutputStreamWriter函式庫
import java.io.PrintWriter;																//	引入java.io.PrintWriter函式庫
import java.net.InetAddress;															//	引入java.net.InetAddress函式庫
import java.net.InetSocketAddress;														//	引入java.net.InetSocketAddress函式庫
import java.net.Socket;																	//	引入java.net.Socket函式庫
import java.net.SocketAddress;															//	引入java.net.SocketAddress函式庫
import java.net.SocketException;														//	引入java.net.SocketException函式庫
import java.net.UnknownHostException;													//	引入java.net.UnknownHostException函式庫
import java.util.ArrayList;																//	引入java.util.ArrayList函式庫
import java.util.List;																	//	引入java.util.List函式庫

import android.util.Log;																//	引入android.util.Log函式庫

public class MainActivity extends AppCompatActivity										//	MainActivity類別
{																						//	進入MainActivity類別
	private EditText txtMessage;														//	宣告txtMessage為EditText物件
	private Button sendBtn;																//	宣告sendBtn為傳送資料至server的Button物件
	public TcpClient mTcpClient;														//	宣告mTcpClient為TcpClient
	private PeopleDatabase peopleDBRun;

	public boolean voiceOn = true;														//	以voiceOn控制語音信號是否發聲

	private final static char[] hexArray = "0123456789ABCDEF".toCharArray();			//	設定hexArray字元陣列供bytesToHex副程式使用

	@Override
	public void onCreate(Bundle savedInstanceState)										//	onCreate程式
	{																					//	進入onCreate程式
		super.onCreate(savedInstanceState);												//	
		setContentView(R.layout.activity_main);											//	

		txtMessage = (EditText) findViewById(R.id.txt_message);							//	設定txtMessage物件連結至txt_message物件
		sendBtn = (Button) findViewById(R.id.send_btn);									//	設定sendBtn物件連結至send_btn物件
		sendBtn.setOnClickListener(new Button.OnClickListener()							//	按鈕按下事件
		{																				//	進入OnClickListener函式
			public void onClick(View v){
				
				if (v == sendBtn){
					if (txtMessage != null)
					{
						/* 這邊要用 Thread 是因為 Android 改版之後會對在主程式裡跑網路連接的程式碼做 Exception 的意外排除動作
						* 因此要把網路連線使用多執行緒的方式去運行，才不會被當成例外錯誤拋出
						*/
						Thread thread = new Thread(mutiThread);							//	建立網路server連線Thread
						thread.start();													//	啟動網路server連線Thread
					}
				}
			}
		});																				//	結束按鈕按下事件
		
		peopleDBRun = new PeopleDatabase();												//	初始化peopleDBRun物件
		peopleDBRun.start();															//	執行peopleDBRun物件
	}																					//	結束onCreate程式
	private Runnable mutiThread = new Runnable(){
		public void run(){
			// 運行網路連線的程式
			new ConnectTask().execute("");
			if (mTcpClient != null)
			{
				//String msg = txtMessage.getEditableText().toString();					   //  取得要傳送的字串
				//mTcpClient.sendMessage(msg);

				//byte[] BinaryData = new byte[]{(byte)0x65,0x65,0x21,0x22,0x12};		   //
				byte[] BinaryData = hexStringToByteArray("93cc03cf000000000000000192a56c6f67696e92ac617564696f2d636c69656e7401");
				//	登入binary
				mTcpClient.sendBinary(BinaryData);
			}
		}
	};
	/******連線工作處理類別******/
	public class ConnectTask extends AsyncTask<String, String, TcpClient> 
	{

		@Override
		protected TcpClient doInBackground(String... message) {

			//we create a TCPClient object
			mTcpClient = new TcpClient(new TcpClient.OnMessageReceived() {
				@Override
				//here the messageReceived method is implemented
				public void messageReceived(String message) {
					//this method calls the onProgressUpdate
					publishProgress(message);
				}
			});
			mTcpClient.run();

			return null;
		}
		
		@Override
		protected void onProgressUpdate(String... values) {
			super.onProgressUpdate(values);
			//response received from server
			Log.d("test", "response " + values[0]);
			//process server response here....
			txtMessage.setText(values[0]);
			
			if(txtMessage.getText().equals(values[0]) == false)
			{
				voiceOn = true;
			}
			
		}
	}
	
	
	
	public void say(final int id){
		new Runnable() {
			@Override
			public void run() {
			try {
					final MediaPlayer player = MediaPlayer.create(MainActivity.this, id);
					if (player != null) {
						player.stop();
					}
					if (!player.isPlaying())
					{player.prepare();
						player.start();
					}
					player.setOnCompletionListener(new MediaPlayer.OnCompletionListener() {
						@Override
						public void onCompletion(MediaPlayer mp) {
							player.release();
						}
					});
				} catch (Exception e) {
					e.printStackTrace();
				}
			}
		}.run();
	}
	
	public static String bytesToHex(byte[] bytes) {
		char[] hexChars = new char[bytes.length * 2];
		for ( int j = 0; j < bytes.length; j++ ) {
			int v = bytes[j] & 0xFF;
			hexChars[j * 2] = hexArray[v >>> 4];
			hexChars[j * 2 + 1] = hexArray[v & 0x0F];
		}
		return new String(hexChars);
	}

	public static byte[] hexStringToByteArray(String s) 
	{																					//	進入hexStringToByteArray副程式
		int len = s.length();
		byte[] data = new byte[len / 2];
		for (int i = 0; i < len; i += 2) {
			data[i / 2] = (byte) ((Character.digit(s.charAt(i), 16) << 4)
					+ Character.digit(s.charAt(i+1), 16));
		}
		return data;
	}																					//	結束hexStringToByteArray副程式
	/******People類別******/
	/*	People類別設定用於資料庫個人資料屬性紀錄、存取方法函式，屬性方面包含name與mp3ID
		People建構子用於建立個人資料(姓名與音檔)
	 */
	public class People																	//	People類別
	{																					//	進入People類別
		//---屬性宣告---
		public String name;																//	建立姓名屬性
		public int mp3ID;																//	建立人名音檔
		//---方法---
		public People(String Name, int mp3ID)											//	People建構子
		{																				//	進入People建構子
			this.name = Name;															//	建立姓名
			this.mp3ID = mp3ID;															//	建立人名音檔
		}																				//	結束People建構子
	}																					//	結束People類別
	/******PeopleDatabase類別*****
		PeopleDatabase類別用於建立人員資料庫
	 */
	public class PeopleDatabase	extends Thread											//	PeopleDatabase類別
	{																					//	進入PeopleDatabase類別
		public boolean isRun = true;													//	以isRun做為執行boolean控制變數
		

		private ArrayList<People> PeopleDB;												//	宣告PeopleDB為一People類別之ArrayList
		
		public PeopleDatabase()															//	PeopleDatabase建構子
		{																				//	進入PeopleDatabase建構子
			PeopleDB = new ArrayList<People>();											//	初始化PeopleDB物件
			
			PeopleDB.add( new People("Jimmy", R.raw.jimmy) );							//	新增人員資料
			PeopleDB.add( new People("Lin", R.raw.lin) );								//	新增人員資料
			PeopleDB.add( new People("Hex", R.raw.hex) );								//	新增人員資料
			PeopleDB.add( new People("teacher1", R.raw.teacher1) );						//	新增人員資料
			PeopleDB.add( new People("teacher2", R.raw.teacher2) );						//	新增人員資料
			PeopleDB.add( new People("teacher3", R.raw.teacher3) );						//	新增人員資料
			PeopleDB.add( new People("teacher4", R.raw.teacher4) );						//	新增人員資料
			PeopleDB.add( new People("teacher5", R.raw.teacher5) );						//	新增人員資料
			//PeopleDB.add()
			//PeopleDB.add()
		}																				//	結束PeopleDatabase建構子
		
		@Override
		public void run()
		{
			super.run();
			while(this.isRun)
			{
				//---測試姓名朗讀---
				//	以EditableText輸入人名，朗讀對應音檔
				String EditableTextData = txtMessage.getEditableText().toString();
				for(int loopnum = 0; loopnum < PeopleDB.size(); loopnum++ )
				//	以for迴圈依序比對資料庫
				{																		//	進入for迴圈
					//Log.d("EditableTextData", EditableTextData);
					//Log.d("PeopleDB", PeopleDB.get(loopnum).name);
					if( EditableTextData.contains(PeopleDB.get(loopnum).name) )			//	若比對得對應音檔
					{																	//	進入if敘述
						//Log.d("Match!!", "");
						Log.d("voiceOn", ""+voiceOn);
						if(voiceOn)
						{
							say(PeopleDB.get(loopnum).mp3ID);
							try {
								Thread.sleep(1000);

							} catch (InterruptedException e) {
								// TODO Auto-generated catch block
								e.printStackTrace();
							}
						}
						
					}																	//	結束if敘述
				}																		//	結束for迴圈
				voiceOn = false;

				try {
                        Thread.sleep(3000);

				} catch (InterruptedException e) {
                // TODO Auto-generated catch block
					e.printStackTrace();
				}
				
			}
		}
	}																					//	結束PeopleDatabase類別
	
}
