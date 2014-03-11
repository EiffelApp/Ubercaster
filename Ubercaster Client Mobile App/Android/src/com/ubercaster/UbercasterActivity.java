package com.ubercaster;

import com.ubercaster.android.R;
import com.ubercaster.receiver.NativeReceiver;

import android.os.Bundle;
import android.provider.Settings;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.Intent;
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;

public class UbercasterActivity extends Activity {

	private boolean mCanPlay = false;
	private boolean mPlaying = false;

	private NativeReceiver mNativeReceiver;
	private PacketSender mPacketSender;
	
	private Button mPlayBtn;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_ubercaster);
		
	

		mNativeReceiver = new NativeReceiver();
		mCanPlay = mNativeReceiver.init();
		
		

		mPlayBtn = (Button) findViewById(R.id.play_btn);
		mPlayBtn.setOnClickListener(mPlayBtnOnClickListener);
		
		
		if(!mCanPlay){
			Log.e("Ubercaster", "Native receiver is not initialized");
			
			AlertDialog alertDialog;
			alertDialog = new AlertDialog.Builder(this).create();
			alertDialog.setTitle("Error");
			alertDialog.setMessage("Can not initialize!");
			alertDialog.show();
			
			mPlayBtn.setEnabled(false);
			
		}

	}

	private OnClickListener mPlayBtnOnClickListener = new OnClickListener() {

		@Override
		public void onClick(View v) {

			if (!mPlaying) {
				// start receiver

				if (mCanPlay) {
					mNativeReceiver.startNativeReceiver();
					mPlayBtn.setText("Stop Listening");
					mPlaying = true;
					
					if(mPacketSender!= null){
						mPacketSender.stop();
						mPacketSender = null;
					}
					mPacketSender = new PacketSender(getApplicationContext());
					mPacketSender.star();
					
				} else {
					Log.e("Ubercaster", "Native receiver is not initialized");
				}

			} else {
				
				// stop receiver
				if (mNativeReceiver != null) {
					mNativeReceiver.stop();
				}
				mPlayBtn.setText("Listen");
				mPlaying = false;
			}

		}
	};

	protected void onDestroy() {
		super.onDestroy();
		if (mNativeReceiver != null) {
			if (mPlaying) {
				mNativeReceiver.stop();
			}
			mNativeReceiver.terminate();
			mNativeReceiver = null;
		}
		
		if(mPacketSender!= null){
			mPacketSender.stop();
			mPacketSender = null;
		}
	};
	
	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
	    MenuInflater inflater = getMenuInflater();
	    inflater.inflate(R.menu.ubercaster, menu);
	    return true;
	}
	
	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
	    // Handle item selection
	    switch (item.getItemId()) {
	        case R.id.action_settings:
	           showWifiSettingsActivity();
	            return true;

	        default:
	            return super.onOptionsItemSelected(item);
	    }
	}

	private void showWifiSettingsActivity() {
		startActivity(new Intent(Settings.ACTION_WIFI_SETTINGS));
	}

}
