package com.ubercaster;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketException;
import java.net.UnknownHostException;
import java.util.Random;

import android.content.Context;
import android.net.wifi.WifiManager;
import android.text.format.Formatter;

public class PacketSender implements Runnable {

	private Thread mThread;

	private Context mContext;
	private WifiManager mWifiManager;

	private InetAddress mAddress;
	private int mPort = 4321;
	
	private DatagramSocket mDatagramSocket;

	public PacketSender(Context context) {
		mContext = context;

		mWifiManager = (WifiManager) mContext
				.getSystemService(Context.WIFI_SERVICE);

		String ip = Formatter.formatIpAddress(mWifiManager.getConnectionInfo()
				.getIpAddress());
		try {
			mAddress = InetAddress.getByName(ip);

			byte[] addressBytes = mAddress.getAddress();
			addressBytes[3] = (byte) 255;
			mAddress = InetAddress.getByAddress(addressBytes);
			
			mDatagramSocket = new DatagramSocket();

		} catch (UnknownHostException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (SocketException e) { 
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

	}

	public void star() {
		if (mThread == null) {
			mThread = new Thread(this, "MyThread");
			mThread.setDaemon(true);
			mThread.start();
		}
	}

	public void stop() {
		if (mThread != null) {
			mThread = null;
		}
	}

	@Override
	public void run() {

		while (Thread.currentThread() == mThread) {
			
			byte[] data = new byte[1];
			Random r = new Random();
			r.nextBytes(data);
			
			DatagramPacket dp = new DatagramPacket(data, data.length, mAddress, mPort);
			try {
				mDatagramSocket.send(dp);
			} catch (IOException e1) {
				e1.printStackTrace();
			}
			
			System.out.println("->" + mAddress);

			try {
				Thread.sleep(200);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}

		}
		
		mDatagramSocket.close();

	}



}