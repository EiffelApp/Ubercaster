Übercaster: A Real-Time Audio Broadcaster Hotspot

K.J Yoo <KJ@EchosDesign.com>, 
Roman Belda <robelor@gmail.com>
Emanuel Aguilera Martí <emaguimar@hotmail.com>
Artem Bagautdinov <artem.bagautdinov@gmail.com>

Inception

The year was 2010. On the streets of the altstadt in Marburg, Germany, I was playing the violin as street musician. Some found my music distracting and yet some found it beautiful. As a curious engineering student, I thought about a better medium to present my music so that only those who were interested may hear what I played seamlessly. After realizing FM transmitter systems were quite expensive, bulky, old and simply not practical. I decided to take matter into my own hands. The solution was simple: broadcast audio to people’s favorite device, the smartphone. 

Design Goal

I wanted anyone to easily plug in any audio into the Übercaster; whether it came from an instrument, TV, iPod or microphone, it didn’t matter. The Übercaster starts broadcasting the sound locally. Then multiple listeners would use their smartphone devices to connect to the Übercaster like a Wi-Fi hotspot to “tune-in.” I also wanted the Übercaster to be an elegant and intuitive device in-line with Dieter Ram’s 10 Principles of Good Design. 

In Development

I have been developing the Übercaster with Odroid X2/U2/U3 development boards since August 2013. I will now share how the Übercaster fundamentally functions.

The Übercaster consists of device and client mobile apps. 

Essentially the Übercaster device is an Odroid U3 running Hostap. (For those of you who are not familiar with Hostap, check out Mauro Ribeiro’s article from the February issue “Using an Odroid-XU as a WiFI Router”) The device is running Ubuntu 13.06 with a custom Odroid-3.8.y kernel. The Übercaster application captures audio with ALSA, encodes the captured audio with OPUS (http://www.opus-codec.org/) and then it packetizes the raw OPUS packets for UDP based multicasting. (This process takes on average 8ms and requires about 6-9% of the CPU. I will admit Odroid U3 might be overkill for what I am doing, but I was not able to find a small dev board with a high quality audio codec. So Odroid works perfectly! Go Hardkernel!)

Now via Hostap, Wi-Fi capable device such as smartphones, tablets and computers can connect to the Übercaster device, which is running isc-dhcp-server running to handle all the clients. Now as soon as a connection is established, the Übercaster mobile client app can be used to listen to whatever the Übercaster device is broadcasting. The app listens to the broadcast IP address on the device, receives the packets, decodes it, and playbacks the sound.  

Now at a first glance, it seems like a basic streaming application like VLC or Icecast. Übercaster offers real-time. Real-time is relative and subjective depending on the applications, but for the Übercaster system, the goal is to have the total audio latency below 25ms. How I measure audio latency is the delay between the time audio goes into the Übercaster device and when it playbacks on the iPhone 5S. (iOS has a lower audio latency than Android devices.) 25ms audio latency is not an arbitrary number, but rather the supposed maximum audio latency before a person is able to perceive the delay. Currently the audio latency is < 50ms on iOS devices and on Android device it varies significantly from device to device, but on the Google Nexus 7 (2013) the latency is 80ms. I have tested the Übercaster with multiple participants and even though the total latency is currently double of my goal and the latency varies from iOS and Android devices, 95% of the listeners were not able to perceive any delay when watching TV or Movie. 

So how many clients can the Übercaster support? I have tested up to 25 clients, however it is theoretically possible to have many more. After server-client relationship is established, the Übercaster is basically a one-way system. The Übercaster broadcasts UDP-based packets and the clients merely tune in on an ip address. That is it. However there is a trade off: UDP isn’t always reliable, however it delivers packets faster and more efficient than TCP because it uses non-ack. This is why the Übercaster sends in small packet frame sizes to hedge against high packet loss rate, which gives a smoother playback. 

Demonstration Video

Please view the following demonstration of the Übercaster.

Übercaster Zwei: vimeo.com/85006122
Übercaster Drei:  vimeo.com/88467399 

Dealing with Interesting Issues

1. To reduce frequency interference, I am mainly using 802.11n at the 5Ghz band. The 2.4Ghz never works even in a moderately crowded area. Using the 5Ghz band, the range is shorter and requires a bit more power, but it is very stable. Hence at CES 2014 in Las Vegas, I had no problem making demonstration in the middle of the densely packed South Hall. (It will be very interesting to work with an 802.11ac module very soon!)

2. To tackle reducing Latency, I use Opus, SPSC Circular Queue and a custom protocol that is based on UDP. I tried RTMP, RTSP and HTTP, but it really didn't work out for me. Originally I wanted to use VLC or other RTSP client to stream content on client devices, but the latency was very high. This is why I chose to go with native apps. The apps are very light and I am currently creating an API that is very easy for mobile developers to integrate the Übercaster stream function. Quick tip concerning Android: it is important to match the sample rate and buffer size for minimum latency. Check this interesting talk from Google I/O 2013 about High Performance Audio on Android. (https://www.youtube.com/watch?v=d3kfEeMZ65c)

The Application

Übercaster started with a simple question: How can individuals have complete freedom and seamless control what they hear in a local surrounding? Or how can individuals have complete freedom to seamlessly and easily broadcast sound to audience members in the local surrounding? 

It turns out that place like gyms, restaurants, tour guides, music venues, sports bars and airports have been thinking of innovative ways to broadcast sound. There have been attempted solutions using FM and Infrared, however it didn’t prove to be practical, it is expensive and complicating to use.

From the very beginning, mobile was at the heart of the product. Currently 65% of all mobile phone users in the U.S use a smartphone. It is widely adopted and it is growing at a staggering rate. So everyone essentially has Übercaster receiver devices already. 

Imagine going into a sports bar and listening to any TV or tuning into the breaking news while you wait for the flight to Frankfurt or listening with perfect clarity to the street musician playing guitar 50 feet away or experiencing a tour of Rome through your smartphone.

Übercaster not only offers a richer and higher audio quality than current products, but also it makes an incredible seamless experience for both users and listeners. Übercaster simplifies, reduces and enhances local audio broadcasting into just a single device. 

The Vision 

Sound is a stepping-stone for me to test if local public content distribution works. I want to broadcast video in real-time. I think of the future a lot and it is clear that frequency band is getting crowded; people want more bandwidth, faster information. However I think in public spaces, there are too many data/bit redundancies. If a lot people in a public area are interested in knowing more about say the Real Madrid game, it is redundant for their devices to access information from the same server a thousand miles away in Texas or California. TVs in a public area are in essence a form of local broadcast. People within 50 feet see the TV. However I am not satisfied with how it works currently. So my goal is local distribution of content. Someone sees in the airport TV from CNN with a breaking news, they should be have at least access to the sound, eventually real-time HD video streaming to their phone at a local distance and also additional web content relating to that news that is constantly aggregating on the Übercaster device for distribution. It is more efficient; people get information quicker and seamlessly.

If you would be interested to knowing more about the Übercaster or have interest in the technology. Please shoot me an email at KJ@EchosDesign.com.

Thanks World!

