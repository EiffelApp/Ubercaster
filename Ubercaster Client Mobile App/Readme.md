#Android App

* We have been only using the Google Nexus 7 (2013) with Android 4.3 to do testing. Android is notorious for having inconsistent and high audio latency, this is due to hardware variance. Supposedely [Android 4.1 has 12ms on the Samsung Galaxy Nexus](http://createdigitalmusic.com/2012/07/android-high-performance-audio-in-4-1-and-what-it-means-plus-libpd-goodness-today/) however we have not been able to acheive anything remotely close to 12ms. We followed the guidelines presented at the [Google I/O 2013 conference](https://developers.google.com/events/io/sessions/325993827), but there has not been a significant change. For audio hardcore android developers, [you should check this article out!](http://heatvst.com/wp/2013/11/30/high-performance-low-latency-audio-on-android-why-it-still-doesnt-work/)

* Even though the Source Code is available, the [App](https://play.google.com/store/apps/details?id=com.uebercaster.android) has been submitted to the Google Play store in January.
* If you want to save time, just download and see if it works for you. I cannot guarantee that it will work for all Android phones or other versions of Android.
* For those a bit more technical, can play around with the Android and see if you can help me out. :)

#iOS App

* We have been testing the iOS app on the iPhone 5S and 5C. It has worked for us very well. The audio latency is lower than the Android for sure. 
* The iOS source code available as well, however it is not available on the App Store.The Opus library has been compiled for iOS. 
* For those a bit more technical, please help me improve the iOS app!

* Emannuel worked on the iOS app and he explains "The reasons that usually makes 20 ms the best option in these cases are two: on one hand codecs like Opus works better with this size. In the docs, they tell that using shorter frames require higher bitrate for equivalent audio quality. On the other hand, iphone's audio card minimum stable interruption is every 10,7 ms (512 samples at 48 kHz) so the first useful size is 20 ms. Android usually accepts any value because its internal layers adapts the sizes but finally the audio card in the phone has its limitations. But thinking in the wifi net problem, where packets are delayed because of multiple nets using the same "air medium", using smaller and more frequent packets can improve sometimes the total delay, so it comes the dilemma.
What I will do for this demo is maintaining 20 ms and adding the possibility of increasing the jitter buffer size as much as needed in your situation (thinking in wifi saturation where you were). For the future I'm thinking in a new mix solution for taking profit of 20 ms at beginning and end but using shorter packets for transmission with lower delay. In the start function there is a parameter for the jitter buffer size. This value can be between 4 and 100, being the unit 20 ms, so 4units=4*20ms=80 ms and 100units=100*20ms= 2s. When you select a value, the algorithm will try to use a delay of half of that size. For example if the size is 10units, usually the delay will be of (10/2)*20ms=100 ms, but depending on the net it will move from 20ms as a minimum to 10*20ms=200ms as a maximum."

#How to connect to the Ubercaster device

* Assuming the device is running, go to the *Wi-Fi* settings.
* You should see the *Ubercaster* network. If you don't see it, refresh or turn off the Wi-Fi network. If you just turned on the Ubercaster device, it takes up to a 30 seconds for the client devices to recognize the Wi-Fi network.
* Connect
* Open up the app and start listening!

##Important Tips

* If you have pause the Android app and start listening again, you will notice that it continues from the point the point where it was stopped. Just restart the app and it should be good.
* On the iOS app, there is a little bar to control the the jitter buffer size.

