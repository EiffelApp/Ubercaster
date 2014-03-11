* [Download the Ubercaster Odroid U3 Linux Image](https://docs.google.com/file/d/0B_teFC78aOcaVEZyQ1U3ZXZwQ1U/edit)

* Uncompress and then flash the image onto the SD card 
* Boot the Odroid (make sure the Odroid U3 is connected to the local area network via Ethernet. dhcp is enabled, so it will take a bit of time before it times out and boots. You can disable the ethernet port via editing interfaces located /etc/network/ folder. Just delete "auto eth0
iface eth0 inet dhcp")

* SSH login into the Odroid (ID:root PW:odroid)
