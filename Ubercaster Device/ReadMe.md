#Hardware Setup

![Ubercaster](http://i.imgur.com/OXULcMG.jpg)

##Flashing the Ubercaster Linux Image

* [Download the Ubercaster Odroid U3 Linux Image](https://docs.google.com/file/d/0B_teFC78aOcaVEZyQ1U3ZXZwQ1U/edit)

* Uncompress and then flash the image onto the SD card

* Boot the Odroid (make sure the Odroid U3 is connected to the local area network via Ethernet. dhcp is enabled, so it will take a bit of time before it times out and boots. You can disable the ethernet port via editing interfaces located /etc/network/ folder. Just delete "auto eth0
iface eth0 inet dhcp")

* SSH login into the Odroid (ID:root PW:odroid)
 
##Setting up the environment to run the Ubercaster Stream Application
####Assuming you have ssh into the Odroid U3

* Check `alsamixer` and scroll left to find `The Left ADC Mixer MIC1` Make sure this is on!
* Check `ifconfig -a` on the command line. Find out whether the name is wlan1, wlan2, wlan3, etc.
* Go to /home/ directory and open hostapd5.conf. Make sure the `interface` matches what you found on `ifconfig -a`
* Save and exit
* Copy and paste `ifconfig wlan1 up 10.0.0.1 netmask 255.255.255.0` and hit enter (We are assumming the name is `wlan1`)
* Copy and paste `service isc-dhcp-server restart`
* Copy and paste `hostapd hostapd5.conf -B`

####You should now see the blue LED on the WiFi module blinking.

* Now `cd /home/sender` and then run `./sender`
* Congrats, it should start broadcasting!

##Now you can automate all of this process so that when you turn on the Odroid U3, it automatically starts broadcasting

* Check `vim /etc/init.d/Ubercaster` and you should be able to see all of the command line input we typed. Simply modify the init.d script so that it matches your settings and save.
* Now reboot and your Odroid U3 should start broadcasting!
