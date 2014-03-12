#Android App

* We have been only using the Google Nexus 7 (2013) with Android 4.3 to do testing. Android is notorious for having inconsistent and high audio latency, this is due to hardware variance. Supposedely [Android 4.1 has 12ms on the Samsung Galaxy Nexus](http://createdigitalmusic.com/2012/07/android-high-performance-audio-in-4-1-and-what-it-means-plus-libpd-goodness-today/) however we have not been able to acheive anything remotely close to 12ms. We followed the guidelines presented at the [Google I/O 2013 conference](https://developers.google.com/events/io/sessions/325993827), but there has not been a significant change. For audio hardcore android developers, [you should check this article out!](http://heatvst.com/wp/2013/11/30/high-performance-low-latency-audio-on-android-why-it-still-doesnt-work/)

* Even though the Source Code is available, the [App](https://play.google.com/store/apps/details?id=com.uebercaster.android) has been submitted to the Google Play store in January.
* If you want to save time, just download and see if it works for you. I cannot guarantee that it will work for all Android phones or other versions of Android.
* For those a bit more technical, can play around with the Android and see if you can help me out. :)

#iOS App

* We have been testing the iOS app on the iPhone 5S and 5C. It has worked for us very well. The audio latency is lower than the Android for sure.
* The iOS source code available as well, however it is not available on the App Store.The Opus library has been compiled for iOS. 
* For those a bit more technical, please help me improve the iOS app!


#How to connect to the Ubercaster device

* Assuming the device is running, go to the *Wi-Fi* settings.
* You should see the *Ubercaster* network. If you don't see it, refresh or turn off the Wi-Fi network. If you just turned on the Ubercaster device, it takes up to a 30 seconds for the client devices to recognize the Wi-Fi network.
* Connect
* Open up the app and start listening!

##Important Tips

*If you have pause the Android app and start listening again, you will notice that it continues from the point the point where it was stopped. Just restart the app and it should be good.
*On the iOS app, there is a little bar to control the 

