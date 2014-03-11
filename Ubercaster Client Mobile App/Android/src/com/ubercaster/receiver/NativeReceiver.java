package com.ubercaster.receiver;

public class NativeReceiver {

	public void startNativeReceiver() {
		new Thread(new Runnable() {
			@Override
			public void run() {
				// set priority
				android.os.Process.setThreadPriority(android.os.Process.THREAD_PRIORITY_URGENT_AUDIO);
				start();
			}
		}).start();
	}

	public native boolean init();

	private native void start();

	public native void stop();

	public native void terminate();

	static {
		System.loadLibrary("receiver-jni");
	}

}
