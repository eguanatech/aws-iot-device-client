# AWS IoT Tunneling Guide

**Version:** 1.0.0  
**Device Client version:** v1.1.1-8ed405b  
**ECI version:** v1.91  
**Last Edited by:** Joe Ng

---

## Table of Contents

- [Introduction](#introduction)
- [Current Capabilities](#current-capabilities)
- [Supported Platform](#supported-platforms)
- [Prerequisites](#prerequisites)
- [Feature Tutorials](#feature-tutorials)
  - [SSH connection to the EMC](#ssh-connection-to-the-emc)
  - [Local Web Server](#local-web-server)
  - [ECI connection to single node inverter (TCP)](#connecting-eci-to-single-node-inverter-tcp)
  - [ECI connection to single node inverter (RS485)](#connecting-eci-to-single-node-inverter-rs485)
  - [ECI connection to Interlakes inverter (TCP)](#connecting-eci-to-interlakes-system-tcp)
    - [Connecting to the Master inverter](#connecting-to-the-master-inverter)
    - [Flashing the TIVA/DSP firmware on the Master inverter](#flashing-the-tivadsp-firmware-on-the-master-inverter)
    - [Connecting to the Slave inverter](#connecting-to-the-slave-inverter)
  - [ECI connection to MUC / Cascade inverter (TCP)](#connecting-eci-to-muc--cascade-system-tcp)
  - [AWS IoT Jobs](#aws-iot-jobs)
- [Compiling the device client](#compiling-the-device-client)

---

## Introduction

AWS IoT Core is utilized to provide remote tunneling service to our EMC (imx6ul/raspberry pi).  

This feature relys heavily on the Internet connectivity. Please ensure you have a stable Internet connection.

## Current Capabilities

- **SSH** connection to the EMC
- **Local Web Server** hosted on the EMC
- **ECI** connection to the inverters
- **AWS IoT Jobs**

## Supported Platforms

- **imx6ul** EMC
- **raspberry pi 3**

## Prerequisites

- Make sure the EMC is already provisioned with a valid **DIGI UUID**. Otherwise the device client will not run.
- Make sure the EMC has stable Internet connection. It should be online on the cloud and DIGI.
- Make sure the **DIGI UUID**(omit all hyphens in between e.g. rpi-00080003000000000300072692e93b4c) is present on the **AWS IoT Portal**.
- If you are using the **ECI** connection feature, make sure the inverter **is not communicating with the inverter**. You need to set _Inverter Type_ to _No Battery_ before connecting ECI to the inverter, otherwise it might not work properly.
- If you are using the **ECI** connection feature to connect to a **RS485** inverter, make sure you have installed the **[Virtual Serial Port Tools](https://freevirtualserialports.com/)**.
- If you are using the **Local Web Server**, it is recommended to use **[Firefox](https://www.mozilla.org/en-CA/firefox/new/)** browser.
- Make sure your PC is **within the office LAN**. Currently only devices in the LAN can access the proxy server.

## Feature Tutorials

### SSH connection to the EMC

Go to the AWS IoT portal and find your target EMC.  
Click on the _Create secure tunnel_ button.  
Choose _Create new tunnel_ and _Manual setup_.
Click _Add new service_ button and enter `SSH` as the service id.  
Copy the source access token when you create the tunnel.

If you are using Windows machine, launch **cmd** and enter the following command.  
If you are using Linux machine, launch **Terminal** and enter the following command.

```bash
#!/bin/bash
ssh ikayzo@10.25.1.210
```

You will be prompted for a password. Enter `canada1234` to proceed.  
A welcome message will be displayed. You are now connected to the proxy server.

```bash
joe@joe-HP-ENVY-x360-Convertible-15-ed1xxx:~/Projects/20250715/rpi/egear-manifest$ ssh ikayzo@10.25.1.210
ikayzo@10.25.1.210's password: 
Welcome to Ubuntu 20.04.6 LTS (GNU/Linux 5.15.0-139-generic x86_64)

 * Documentation:  https://help.ubuntu.com
 * Management:     https://landscape.canonical.com
 * Support:        https://ubuntu.com/pro

1 device has a firmware upgrade available.
Run `fwupdmgr get-upgrades` for more information.


Expanded Security Maintenance for Infrastructure is not enabled.

43 updates can be applied immediately.
To see these additional updates run: apt list --upgradable

45 additional security updates can be applied with ESM Infra.
Learn more about enabling ESM Infra service for Ubuntu 20.04 at
https://ubuntu.com/20-04

New release '22.04.5 LTS' available.
Run 'do-release-upgrade' to upgrade to it.

Your Hardware Enablement Stack (HWE) is supported until April 2025.
Last login: Tue Jul 15 17:31:45 2025 from 10.25.1.63
ikayzo@ikayzo-ThinkCentre-M710e:~$
```

Enter the following command after you are connected to the proxy server.

```bash
tunnel
```

You will be prompted for the source access token. Enter the source access token you got from the AWS IoT portal.
Then you will be prompted for the region of AWS account. Simply press _Enter_ to use the default.

```bash
ikayzo@ikayzo-ThinkCentre-M710e:~$ tunnel 
Enter the source access token for the tunnel: AQGAAXine86zkSKAClVrjttJ0897ABsAAgABQQAMNzAwNTAyODI1NjQ3AAFUAANDQVQAAQAHYXdzLWttcwBLYXJuOmF3czprbXM6dXMtZWFzdC0yOjY4MzQ5NzY0MDMyOTprZXkvOGIzMTdjOTItZWFhNi00OWYzLTljNGEtY2IyMmFjMzc3NWE1ALgBAgEAeEwqwYCMTR47-U0mnhSrg_YRIo5UVD0XFJs8rlsJRth3ASQDIGZp0-7wdBVei6RC0i0AAAB-MHwGCSqGSIb3DQEHBqBvMG0CAQAwaAYJKoZIhvcNAQcBMB4GCWCGSAFlAwQBLjARBAyX4NwRmriD8Xcn70wCARCAO349EXrpQQ2shKeowOLuKq5QnQIpKc9NAgw1LE2JioERucvSULUTK7tI9kWYPfhS8yCEznPavQPqwla4AgAAAAAMAAAQAAAAAAAAAAAAAAAAAACckU72DzUUzquOZ4qmnFf_____AAAAAQAAAAAAAAAAAAAAAQAAAC_6x0yYzkXU0sCbkm4SRGUORrmgI478irdxZE7tTWZ14wLksYEhQosyZqIqMBL64S0MvZXipnlJ2iBXkYLJ8pw=
Enter the region of your AWS account[default us-east-2]: 
[2025-07-15T17:49:15.075660]{83944}[warning] Found access token supplied via CLI arg. Consider using environment variable AWSIOT_TUNNEL_ACCESS_TOKEN instead
[2025-07-15T17:49:15.075718]{83944}[info]    Starting proxy in source mode
[2025-07-15T17:49:15.076767]{83944}[info]    Attempting to establish web socket connection with endpoint wss://data.tunneling.iot.us-east-2.amazonaws.com:443
[2025-07-15T17:49:15.407148]{83944}[info]    Web socket session ID: 0630d2fffe2ac3eb-000032aa-00031874-bfad8979e0c115d7-4f0bca79
[2025-07-15T17:49:15.407239]{83944}[info]    Successfully established websocket connection with proxy server: wss://data.tunneling.iot.us-east-2.amazonaws.com:443
[2025-07-15T17:49:15.407503]{83944}[info]    Use port mapping:
[2025-07-15T17:49:15.407558]{83944}[info]    ---------------------------------
[2025-07-15T17:49:15.407581]{83944}[info]    SSH = 0
[2025-07-15T17:49:15.407602]{83944}[info]    ---------------------------------
[2025-07-15T17:49:15.407847]{83944}[info]    Listening for new connection on port 37635
[2025-07-15T17:49:15.407910]{83944}[info]    Listen port assigned for service id SSH. New port mapping: 
[2025-07-15T17:49:15.407934]{83944}[info]    SSH = 37635
```

Verify you are connecting to service id **SSH**.  
Now copy the port opened by the proxy server(i.e. 37635 from the above message).

Open a new **terminal**/**cmd** depending on your OS. Enter the following command.

```bash
#ssh root@10.25.1.210 -p <the port you copied from previous step i.e. 37635>
ssh root@10.25.1.210 -p 37635
```

You will likely be asked if you really want to continue connecting. Enter `yes` to proceed.  
Then you will be prompted to enter a password. Enter `rootpassword` to proceed.

```bash
joe@joe-HP-ENVY-x360-Convertible-15-ed1xxx:~/Projects/20250715/rpi/egear-manifest$ ssh root@10.25.1.210 -p 37635
The authenticity of host '[10.25.1.210]:37635 ([10.25.1.210]:37635)' can't be established.
RSA key fingerprint is SHA256:jJ9VISjgs+2o5Ur7X1s+fyHZ2Qe51ytWK6fuLyBSiYk.
Are you sure you want to continue connecting (yes/no/[fingerprint])? y
Please type 'yes', 'no' or the fingerprint: yes 
Warning: Permanently added '[10.25.1.210]:37635' (RSA) to the list of known hosts.
root@10.25.1.210's password:


BusyBox v1.35.0 () built-in shell (ash)
Enter 'help' for a list of built-in commands.

Starting Dropbear SSH server: /usr/sbin/dropbear is already running
root@raspberrypi3:~#
```

Now you have successfully connected to the shell on the EMC. You can do almost anything you want including but not limited to checking the logs, updating the gwappd application, rebooting the EMC, etc.  
> ⚠️ **Warning:** If you are not familiar with the shell environment it is NOT RECOMMENDED to use this feature as you may break something by accident.

### Local Web Server

Go to the AWS IoT portal and find your target EMC.  
Click on the _Create secure tunnel_ button.  
Choose _Create new tunnel_ and _Manual setup_.
Click _Add new service_ button and enter `GW` as the service id.  
Copy the source access token when you create the tunnel.

If you are using Windows machine, launch **cmd** and enter the following command.  
If you are using Linux machine, launch **Terminal** and enter the following command.

```bash
#!/bin/bash
ssh ikayzo@10.25.1.210
```

You will be prompted for a password. Enter `canada1234` to proceed.  
A welcome message will be displayed. You are now connected to the proxy server.

```bash
joe@joe-HP-ENVY-x360-Convertible-15-ed1xxx:~/Projects/20250715/rpi/egear-manifest$ ssh ikayzo@10.25.1.210
ikayzo@10.25.1.210's password: 
Welcome to Ubuntu 20.04.6 LTS (GNU/Linux 5.15.0-139-generic x86_64)

 * Documentation:  https://help.ubuntu.com
 * Management:     https://landscape.canonical.com
 * Support:        https://ubuntu.com/pro

1 device has a firmware upgrade available.
Run `fwupdmgr get-upgrades` for more information.


Expanded Security Maintenance for Infrastructure is not enabled.

43 updates can be applied immediately.
To see these additional updates run: apt list --upgradable

45 additional security updates can be applied with ESM Infra.
Learn more about enabling ESM Infra service for Ubuntu 20.04 at
https://ubuntu.com/20-04

New release '22.04.5 LTS' available.
Run 'do-release-upgrade' to upgrade to it.

Your Hardware Enablement Stack (HWE) is supported until April 2025.
Last login: Tue Jul 15 17:35:22 2025 from 10.25.1.63
ikayzo@ikayzo-ThinkCentre-M710e:~$
```

Enter the following command after you are connected to the proxy server.

```bash
tunnel
```

You will be prompted for the source access token. Enter the source access token you got from the AWS IoT portal.
Then you will be prompted for the region of AWS account. Simply press _Enter_ to use the default.

```bash
ikayzo@ikayzo-ThinkCentre-M710e:~$ tunnel 
Enter the source access token for the tunnel: AQGAAXjvPUcfaAbRzT5gi5kjtaEdABsAAgABQQAMNzAwNTAyODI1NjQ3AAFUAANDQVQAAQAHYXdzLWttcwBLYXJuOmF3czprbXM6dXMtZWFzdC0yOjY4MzQ5NzY0MDMyOTprZXkvOGIzMTdjOTItZWFhNi00OWYzLTljNGEtY2IyMmFjMzc3NWE1ALgBAgEAeEwqwYCMTR47-U0mnhSrg_YRIo5UVD0XFJs8rlsJRth3AS1mSTKdOzJTsU81hy4S4nMAAAB-MHwGCSqGSIb3DQEHBqBvMG0CAQAwaAYJKoZIhvcNAQcBMB4GCWCGSAFlAwQBLjARBAzMMCt_JIbtiNoUB58CARCAO4zPmZToVQnCITSgThY-VcBXpFCl9KgoykmPWoYkmFF8FFZ82FS9pScR5NmAfE7EfRmzy8D5QA_fNa1qAgAAAAAMAAAQAAAAAAAAAAAAAAAAAGtsvXbOaC-0w1_Y2RGCDI3_____AAAAAQAAAAAAAAAAAAAAAQAAAC_pI2-CFln0JYO89wGZRYFywYsfrFDH_8IQdTd04g8-n_Y1szhdAzvEnHl_PQCGQsh8Fb2XP0Lphaw8dMq3WE4=
Enter the region of your AWS account[default us-east-2]: 
[2025-07-15T18:28:23.546221]{84125}[warning] Found access token supplied via CLI arg. Consider using environment variable AWSIOT_TUNNEL_ACCESS_TOKEN instead
[2025-07-15T18:28:23.546279]{84125}[info]    Starting proxy in source mode
[2025-07-15T18:28:23.547336]{84125}[info]    Attempting to establish web socket connection with endpoint wss://data.tunneling.iot.us-east-2.amazonaws.com:443
[2025-07-15T18:28:23.840053]{84125}[info]    Web socket session ID: 020e49fffe893acf-00006f9b-000486e1-d038dcd91d5eeb6a-b122b483
[2025-07-15T18:28:23.840145]{84125}[info]    Successfully established websocket connection with proxy server: wss://data.tunneling.iot.us-east-2.amazonaws.com:443
[2025-07-15T18:28:23.840432]{84125}[info]    Use port mapping:
[2025-07-15T18:28:23.840489]{84125}[info]    ---------------------------------
[2025-07-15T18:28:23.840513]{84125}[info]    GW = 0
[2025-07-15T18:28:23.840535]{84125}[info]    ---------------------------------
[2025-07-15T18:28:23.840780]{84125}[info]    Listening for new connection on port 35581
[2025-07-15T18:28:23.840860]{84125}[info]    Listen port assigned for service id GW. New port mapping: 
[2025-07-15T18:28:23.840887]{84125}[info]    GW = 35581
[2025-07-15T18:28:23.840918]{84125}[info]    Please use FIREFOX browser to access the local gateway for better compactibility!
```

Verify you are connecting to service id **GW**.  
Now copy the port opened by the proxy server(i.e. 35581 from the above message).

Launch **[Firefox](https://www.mozilla.org/en-CA/firefox/new/)** browser and navigate to this address `https://10.25.1.210:35581`.
You will be warned about potential security risk.  
Click on the _Advanced..._ button and _Accept the Risk and Continue_ button.

You are now accessing the local admin page. You can change settings and check system status here.

### Connecting ECI to single node inverter (TCP)

> ⚠️ **Note:** To use ECI connection you would need to disable the communication between the EMC and the inverter first, or else you may observe unexpected behaviour. Either go to DIGI or setup a GW connection, then set **Inverter Type** to **No Battery**.

Go to the AWS IoT portal and find your target EMC.  
Click on the _Create secure tunnel_ button.  
Choose _Create new tunnel_ and _Manual setup_.
Click _Add new service_ button and enter `TIVA` as the service id.  
Copy the source access token when you create the tunnel.

If you are using Windows machine, launch **cmd** and enter the following command.  
If you are using Linux machine, launch **Terminal** and enter the following command.

```bash
#!/bin/bash
ssh ikayzo@10.25.1.210
```

You will be prompted for a password. Enter `canada1234` to proceed.  
A welcome message will be displayed. You are now connected to the proxy server.

```bash
joe@joe-HP-ENVY-x360-Convertible-15-ed1xxx:~/Projects/20250715/rpi/egear-manifest$ ssh ikayzo@10.25.1.210
ikayzo@10.25.1.210's password: 
Welcome to Ubuntu 20.04.6 LTS (GNU/Linux 5.15.0-139-generic x86_64)

 * Documentation:  https://help.ubuntu.com
 * Management:     https://landscape.canonical.com
 * Support:        https://ubuntu.com/pro

1 device has a firmware upgrade available.
Run `fwupdmgr get-upgrades` for more information.


Expanded Security Maintenance for Infrastructure is not enabled.

43 updates can be applied immediately.
To see these additional updates run: apt list --upgradable

45 additional security updates can be applied with ESM Infra.
Learn more about enabling ESM Infra service for Ubuntu 20.04 at
https://ubuntu.com/20-04

New release '22.04.5 LTS' available.
Run 'do-release-upgrade' to upgrade to it.

Your Hardware Enablement Stack (HWE) is supported until April 2025.
Last login: Tue Jul 15 17:35:22 2025 from 10.25.1.63
ikayzo@ikayzo-ThinkCentre-M710e:~$
```

Enter the following command after you are connected to the proxy server.

```bash
tunnel
```

You will be prompted for the source access token. Enter the source access token you got from the AWS IoT portal.
Then you will be prompted for the region of AWS account. Simply press _Enter_ to use the default.

```bash
ikayzo@ikayzo-ThinkCentre-M710e:~$ tunnel 
Enter the source access token for the tunnel: AQGAAXiCa1GtjG1r1We9_15SgwQyABsAAgABQQAMNzAwNTAyODI1NjQ3AAFUAANDQVQAAQAHYXdzLWttcwBLYXJuOmF3czprbXM6dXMtZWFzdC0yOjY4MzQ5NzY0MDMyOTprZXkvOGIzMTdjOTItZWFhNi00OWYzLTljNGEtY2IyMmFjMzc3NWE1ALgBAgEAeEwqwYCMTR47-U0mnhSrg_YRIo5UVD0XFJs8rlsJRth3AexP0rF8CWIPOM0B3IkxZ1cAAAB-MHwGCSqGSIb3DQEHBqBvMG0CAQAwaAYJKoZIhvcNAQcBMB4GCWCGSAFlAwQBLjARBAx_0MQQw0DK-lgkPiwCARCAO4-OSyWzroyn104YTVBXSpzErVTn7KfuChGravk_Typ5of4R1cEaNZvkTWFcvoXnwEifYPNrBSZs71DbAgAAAAAMAAAQAAAAAAAAAAAAAAAAAMkw8vxB9LRA0olofEdcKsX_____AAAAAQAAAAAAAAAAAAAAAQAAAC-H3-Cb4hazmIUWUeNXhCPmWdMwssB9jl8OTs2lg8D9z0caku7JB6huhRiFZtwoLHqxuVchl3S4IOAwm1t8S0Q=
Enter the region of your AWS account[default us-east-2]: 
[2025-07-16T09:57:44.156116]{85876}[warning] Found access token supplied via CLI arg. Consider using environment variable AWSIOT_TUNNEL_ACCESS_TOKEN instead
[2025-07-16T09:57:44.156175]{85876}[info]    Starting proxy in source mode
[2025-07-16T09:57:44.157226]{85876}[info]    Attempting to establish web socket connection with endpoint wss://data.tunneling.iot.us-east-2.amazonaws.com:443
[2025-07-16T09:57:44.454247]{85876}[info]    Web socket session ID: 020e49fffe893acf-00006f9b-00060cdd-04b172caf5b84258-53e11d24
[2025-07-16T09:57:44.454341]{85876}[info]    Successfully established websocket connection with proxy server: wss://data.tunneling.iot.us-east-2.amazonaws.com:443
[2025-07-16T09:57:44.454616]{85876}[info]    Use port mapping:
[2025-07-16T09:57:44.454672]{85876}[info]    ---------------------------------
[2025-07-16T09:57:44.454710]{85876}[info]    TIVA = 0
[2025-07-16T09:57:44.454779]{85876}[info]    ---------------------------------
[2025-07-16T09:57:44.455059]{85876}[info]    Listening for new connection on port 36107
[2025-07-16T09:57:44.455135]{85876}[info]    Listen port assigned for service id TIVA. New port mapping: 
[2025-07-16T09:57:44.455179]{85876}[info]    TIVA = 36107
```

Verify you are connecting to service id **TIVA**.  
Now copy the port opened by the proxy server(i.e. 36107 from the above message).

Launch ECI. Click the _Open Network_ button. Enter `10.25.1.210` as the **IP Address** and 36107 as the **Port**. Then Click the _Connect_ button.

You can now use ECI to read inverter / battery readings, change settings on the inverter or even flash the firmware on the TIVA / DSP.

### Connecting ECI to single node inverter (RS485)

> ⚠️ **Note:** To use ECI connection you would need to disable the communication between the EMC and the inverter first, or else you may observe unexpected behaviour. Either go to DIGI or setup a GW connection, then set **Inverter Type** to **No Battery**.  
⚠️ **Note:** To connect to a RS485 inverter you need the **[Virtual Serial Port Tools](https://freevirtualserialports.com/)** tool. Please ensure that you have installed the software before creating the tunnel.

Go to the AWS IoT portal and find your target EMC.  
Click on the _Create secure tunnel_ button.  
Choose _Create new tunnel_ and _Manual setup_.
Click _Add new service_ button and enter `TIVA` as the service id.  
Copy the source access token when you create the tunnel.

If you are using Windows machine, launch **cmd** and enter the following command.  
If you are using Linux machine, launch **Terminal** and enter the following command.

```bash
#!/bin/bash
ssh ikayzo@10.25.1.210
```

You will be prompted for a password. Enter `canada1234` to proceed.  
A welcome message will be displayed. You are now connected to the proxy server.

```bash
joe@joe-HP-ENVY-x360-Convertible-15-ed1xxx:~/Projects/20250715/rpi/egear-manifest$ ssh ikayzo@10.25.1.210
ikayzo@10.25.1.210's password: 
Welcome to Ubuntu 20.04.6 LTS (GNU/Linux 5.15.0-139-generic x86_64)

 * Documentation:  https://help.ubuntu.com
 * Management:     https://landscape.canonical.com
 * Support:        https://ubuntu.com/pro

1 device has a firmware upgrade available.
Run `fwupdmgr get-upgrades` for more information.


Expanded Security Maintenance for Infrastructure is not enabled.

43 updates can be applied immediately.
To see these additional updates run: apt list --upgradable

45 additional security updates can be applied with ESM Infra.
Learn more about enabling ESM Infra service for Ubuntu 20.04 at
https://ubuntu.com/20-04

New release '22.04.5 LTS' available.
Run 'do-release-upgrade' to upgrade to it.

Your Hardware Enablement Stack (HWE) is supported until April 2025.
Last login: Tue Jul 15 17:35:22 2025 from 10.25.1.63
ikayzo@ikayzo-ThinkCentre-M710e:~$
```

Enter the following command after you are connected to the proxy server.

```bash
tunnel
```

You will be prompted for the source access token. Enter the source access token you got from the AWS IoT portal.
Then you will be prompted for the region of AWS account. Simply press _Enter_ to use the default.

```bash
ikayzo@ikayzo-ThinkCentre-M710e:~$ tunnel 
Enter the source access token for the tunnel: AQGAAXiCa1GtjG1r1We9_15SgwQyABsAAgABQQAMNzAwNTAyODI1NjQ3AAFUAANDQVQAAQAHYXdzLWttcwBLYXJuOmF3czprbXM6dXMtZWFzdC0yOjY4MzQ5NzY0MDMyOTprZXkvOGIzMTdjOTItZWFhNi00OWYzLTljNGEtY2IyMmFjMzc3NWE1ALgBAgEAeEwqwYCMTR47-U0mnhSrg_YRIo5UVD0XFJs8rlsJRth3AexP0rF8CWIPOM0B3IkxZ1cAAAB-MHwGCSqGSIb3DQEHBqBvMG0CAQAwaAYJKoZIhvcNAQcBMB4GCWCGSAFlAwQBLjARBAx_0MQQw0DK-lgkPiwCARCAO4-OSyWzroyn104YTVBXSpzErVTn7KfuChGravk_Typ5of4R1cEaNZvkTWFcvoXnwEifYPNrBSZs71DbAgAAAAAMAAAQAAAAAAAAAAAAAAAAAMkw8vxB9LRA0olofEdcKsX_____AAAAAQAAAAAAAAAAAAAAAQAAAC-H3-Cb4hazmIUWUeNXhCPmWdMwssB9jl8OTs2lg8D9z0caku7JB6huhRiFZtwoLHqxuVchl3S4IOAwm1t8S0Q=
Enter the region of your AWS account[default us-east-2]: 
[2025-07-16T09:57:44.156116]{85876}[warning] Found access token supplied via CLI arg. Consider using environment variable AWSIOT_TUNNEL_ACCESS_TOKEN instead
[2025-07-16T09:57:44.156175]{85876}[info]    Starting proxy in source mode
[2025-07-16T09:57:44.157226]{85876}[info]    Attempting to establish web socket connection with endpoint wss://data.tunneling.iot.us-east-2.amazonaws.com:443
[2025-07-16T09:57:44.454247]{85876}[info]    Web socket session ID: 020e49fffe893acf-00006f9b-00060cdd-04b172caf5b84258-53e11d24
[2025-07-16T09:57:44.454341]{85876}[info]    Successfully established websocket connection with proxy server: wss://data.tunneling.iot.us-east-2.amazonaws.com:443
[2025-07-16T09:57:44.454616]{85876}[info]    Use port mapping:
[2025-07-16T09:57:44.454672]{85876}[info]    ---------------------------------
[2025-07-16T09:57:44.454710]{85876}[info]    TIVA = 0
[2025-07-16T09:57:44.454779]{85876}[info]    ---------------------------------
[2025-07-16T09:57:44.455059]{85876}[info]    Listening for new connection on port 36107
[2025-07-16T09:57:44.455135]{85876}[info]    Listen port assigned for service id TIVA. New port mapping: 
[2025-07-16T09:57:44.455179]{85876}[info]    TIVA = 36107
```

Verify you are connecting to service id **TIVA**.  
Now copy the port opened by the proxy server(i.e. 36107 from the above message).

Launch **[Virtual Serial Port Tools](https://freevirtualserialports.com/)**. Click on _Create TCP/IP Serial Port_. Set **Protocol** to **Raw**. Enter `10.25.1.210` as **Remote host IP address or name** and 36107 as **Remote TCP port**. Then click the _Create_ button.

The software will warn you about non-permanent devices. Check the _Create non-permanent device_ box and click the _OK_ button.

Launch ECI. Click the _Open RS485_ button. Make sure you select the COM port that is created by the **[Virtual Serial Port Tools](https://freevirtualserialports.com/)**(by default it should be  COM1). Then click the _Connect_ button.

You can now use ECI to read inverter / battery readings, change settings on the inverter or even flash the firmware on the TIVA.

### Connecting ECI to Interlakes inverter (TCP)

> ⚠️ **Note:** To use ECI connection you would need to disable the communication between the EMC and the inverter first, or else you may observe unexpected behaviour. Either go to DIGI or setup a GW connection, then set **Inverter Type** to **No Battery**.  
⚠️ **Note:** By default the IP address of the Master inverter is `169.254.0.5`and the IP address of the slave inverter is `169.254.0.6`. However, if you intend to flash TIVA/DSP firmware with remote tunneling, you would need to change the IP address of the Master inverter to `169.254.0.7`.

#### Connecting to the Master inverter

Go to the AWS IoT portal and find your target EMC.  
Click on the _Create secure tunnel_ button.  
Choose _Create new tunnel_ and _Manual setup_.
Click _Add new service_ button and enter `TIVA` as the service id.  
Copy the source access token when you create the tunnel.

If you are using Windows machine, launch **cmd** and enter the following command.  
If you are using Linux machine, launch **Terminal** and enter the following command.

```bash
#!/bin/bash
ssh ikayzo@10.25.1.210
```

You will be prompted for a password. Enter `canada1234` to proceed.  
A welcome message will be displayed. You are now connected to the proxy server.

```bash
joe@joe-HP-ENVY-x360-Convertible-15-ed1xxx:~/Projects/20250715/rpi/egear-manifest$ ssh ikayzo@10.25.1.210
ikayzo@10.25.1.210's password: 
Welcome to Ubuntu 20.04.6 LTS (GNU/Linux 5.15.0-139-generic x86_64)

 * Documentation:  https://help.ubuntu.com
 * Management:     https://landscape.canonical.com
 * Support:        https://ubuntu.com/pro

1 device has a firmware upgrade available.
Run `fwupdmgr get-upgrades` for more information.


Expanded Security Maintenance for Infrastructure is not enabled.

43 updates can be applied immediately.
To see these additional updates run: apt list --upgradable

45 additional security updates can be applied with ESM Infra.
Learn more about enabling ESM Infra service for Ubuntu 20.04 at
https://ubuntu.com/20-04

New release '22.04.5 LTS' available.
Run 'do-release-upgrade' to upgrade to it.

Your Hardware Enablement Stack (HWE) is supported until April 2025.
Last login: Tue Jul 15 17:35:22 2025 from 10.25.1.63
ikayzo@ikayzo-ThinkCentre-M710e:~$
```

Enter the following command after you are connected to the proxy server.

```bash
tunnel
```

You will be prompted for the source access token. Enter the source access token you got from the AWS IoT portal.
Then you will be prompted for the region of AWS account. Simply press _Enter_ to use the default.

```bash
ikayzo@ikayzo-ThinkCentre-M710e:~$ tunnel 
Enter the source access token for the tunnel: AQGAAXiCa1GtjG1r1We9_15SgwQyABsAAgABQQAMNzAwNTAyODI1NjQ3AAFUAANDQVQAAQAHYXdzLWttcwBLYXJuOmF3czprbXM6dXMtZWFzdC0yOjY4MzQ5NzY0MDMyOTprZXkvOGIzMTdjOTItZWFhNi00OWYzLTljNGEtY2IyMmFjMzc3NWE1ALgBAgEAeEwqwYCMTR47-U0mnhSrg_YRIo5UVD0XFJs8rlsJRth3AexP0rF8CWIPOM0B3IkxZ1cAAAB-MHwGCSqGSIb3DQEHBqBvMG0CAQAwaAYJKoZIhvcNAQcBMB4GCWCGSAFlAwQBLjARBAx_0MQQw0DK-lgkPiwCARCAO4-OSyWzroyn104YTVBXSpzErVTn7KfuChGravk_Typ5of4R1cEaNZvkTWFcvoXnwEifYPNrBSZs71DbAgAAAAAMAAAQAAAAAAAAAAAAAAAAAMkw8vxB9LRA0olofEdcKsX_____AAAAAQAAAAAAAAAAAAAAAQAAAC-H3-Cb4hazmIUWUeNXhCPmWdMwssB9jl8OTs2lg8D9z0caku7JB6huhRiFZtwoLHqxuVchl3S4IOAwm1t8S0Q=
Enter the region of your AWS account[default us-east-2]: 
[2025-07-16T09:57:44.156116]{85876}[warning] Found access token supplied via CLI arg. Consider using environment variable AWSIOT_TUNNEL_ACCESS_TOKEN instead
[2025-07-16T09:57:44.156175]{85876}[info]    Starting proxy in source mode
[2025-07-16T09:57:44.157226]{85876}[info]    Attempting to establish web socket connection with endpoint wss://data.tunneling.iot.us-east-2.amazonaws.com:443
[2025-07-16T09:57:44.454247]{85876}[info]    Web socket session ID: 020e49fffe893acf-00006f9b-00060cdd-04b172caf5b84258-53e11d24
[2025-07-16T09:57:44.454341]{85876}[info]    Successfully established websocket connection with proxy server: wss://data.tunneling.iot.us-east-2.amazonaws.com:443
[2025-07-16T09:57:44.454616]{85876}[info]    Use port mapping:
[2025-07-16T09:57:44.454672]{85876}[info]    ---------------------------------
[2025-07-16T09:57:44.454710]{85876}[info]    TIVA = 0
[2025-07-16T09:57:44.454779]{85876}[info]    ---------------------------------
[2025-07-16T09:57:44.455059]{85876}[info]    Listening for new connection on port 36107
[2025-07-16T09:57:44.455135]{85876}[info]    Listen port assigned for service id TIVA. New port mapping: 
[2025-07-16T09:57:44.455179]{85876}[info]    TIVA = 36107
```

Verify you are connecting to service id **TIVA**.  
Now copy the port opened by the proxy server(i.e. 36107 from the above message).

Launch ECI. Click the _Open Network_ button. Enter `10.25.1.210` as the **IP Address** and 36107 as the **Port**. Then Click the _Connect_ button.

You can now use ECI to read inverter / battery readings, change settings on the inverter or even flash the firmware on the TIVA / DSP.

#### Flashing the TIVA/DSP firmware on the Master inverter

On ECI, click the _PCS Settings_ button. Then expand _Network Settings_. Set Item 12 **Default IP Address** to `169.254.0.7`. Then set **PCS Controller Reset** to 1. Now the inverter will restart with active IP address `169.254.0.7`.

Close the ECI for now.

Go to the AWS IoT portal and find your target EMC.  
Click on the _Create secure tunnel_ button.  
Choose _Create new tunnel_ and _Manual setup_.
Click _Add new service_ button and enter `TIVA_1` as the service id.  
Copy the source access token when you create the tunnel.

If you are using Windows machine, launch **cmd** and enter the following command.  
If you are using Linux machine, launch **Terminal** and enter the following command.

```bash
#!/bin/bash
ssh ikayzo@10.25.1.210
```

You will be prompted for a password. Enter `canada1234` to proceed.  
A welcome message will be displayed. You are now connected to the proxy server.

```bash
joe@joe-HP-ENVY-x360-Convertible-15-ed1xxx:~/Projects/20250715/rpi/egear-manifest$ ssh ikayzo@10.25.1.210
ikayzo@10.25.1.210's password: 
Welcome to Ubuntu 20.04.6 LTS (GNU/Linux 5.15.0-139-generic x86_64)

 * Documentation:  https://help.ubuntu.com
 * Management:     https://landscape.canonical.com
 * Support:        https://ubuntu.com/pro

1 device has a firmware upgrade available.
Run `fwupdmgr get-upgrades` for more information.


Expanded Security Maintenance for Infrastructure is not enabled.

43 updates can be applied immediately.
To see these additional updates run: apt list --upgradable

45 additional security updates can be applied with ESM Infra.
Learn more about enabling ESM Infra service for Ubuntu 20.04 at
https://ubuntu.com/20-04

New release '22.04.5 LTS' available.
Run 'do-release-upgrade' to upgrade to it.

Your Hardware Enablement Stack (HWE) is supported until April 2025.
Last login: Tue Jul 15 17:35:22 2025 from 10.25.1.63
ikayzo@ikayzo-ThinkCentre-M710e:~$
```

Enter the following command after you are connected to the proxy server.

```bash
tunnel
```

You will be prompted for the source access token. Enter the source access token you got from the AWS IoT portal.
Then you will be prompted for the region of AWS account. Simply press _Enter_ to use the default.

```bash
ikayzo@ikayzo-ThinkCentre-M710e:~$ tunnel 
Enter the source access token for the tunnel: AQGAAXh5wuGFN7TCpQFuloMLeBbtABsAAgABQQAMNzAwNTAyODI1NjQ3AAFUAANDQVQAAQAHYXdzLWttcwBLYXJuOmF3czprbXM6dXMtZWFzdC0yOjY4MzQ5NzY0MDMyOTprZXkvOGIzMTdjOTItZWFhNi00OWYzLTljNGEtY2IyMmFjMzc3NWE1ALgBAgEAeEwqwYCMTR47-U0mnhSrg_YRIo5UVD0XFJs8rlsJRth3AeVOVUZg_iJOw66K4afBlnQAAAB-MHwGCSqGSIb3DQEHBqBvMG0CAQAwaAYJKoZIhvcNAQcBMB4GCWCGSAFlAwQBLjARBAxhP-_ujUKA7vo0bEACARCAO-DQvqZAl2z_pfvzhHJ8HgABI5Yb45Bgn-g_O_ri3qbtaZY2z12nQnPF-htFm0BaFu-5AMYBebVebN7yAgAAAAAMAAAQAAAAAAAAAAAAAAAAAH1Y8peoqcEAR8fNgMBMG5n_____AAAAAQAAAAAAAAAAAAAAAQAAAC9sqx-YTINZdhGE7P36iXecfwYyIBbKLx_0FKUdw0jHWiJrG-3_x2RkxXEnmskWuwhrkluGI9G0NGIafwEVnus=
Enter the region of your AWS account[default us-east-2]: 
[2025-07-16T10:52:24.973352]{86086}[warning] Found access token supplied via CLI arg. Consider using environment variable AWSIOT_TUNNEL_ACCESS_TOKEN instead
[2025-07-16T10:52:24.973410]{86086}[info]    Starting proxy in source mode
[2025-07-16T10:52:24.974458]{86086}[info]    Attempting to establish web socket connection with endpoint wss://data.tunneling.iot.us-east-2.amazonaws.com:443
[2025-07-16T10:52:25.249876]{86086}[info]    Web socket session ID: 02ccf6fffeb84d85-000046d8-0006657e-78e227a9b38a51fd-32753eb9
[2025-07-16T10:52:25.249968]{86086}[info]    Successfully established websocket connection with proxy server: wss://data.tunneling.iot.us-east-2.amazonaws.com:443
[2025-07-16T10:52:25.250226]{86086}[info]    Use port mapping:
[2025-07-16T10:52:25.250281]{86086}[info]    ---------------------------------
[2025-07-16T10:52:25.250305]{86086}[info]    TIVA_1 = 0
[2025-07-16T10:52:25.250326]{86086}[info]    ---------------------------------
[2025-07-16T10:52:25.250554]{86086}[info]    Listening for new connection on port 36147
[2025-07-16T10:52:25.250625]{86086}[info]    Listen port assigned for service id TIVA_1. New port mapping: 
[2025-07-16T10:52:25.250650]{86086}[info]    TIVA_1 = 36147
```

Verify you are connecting to service id **TIVA_1**.  
Now copy the port opened by the proxy server(i.e. 36147 from the above message).

Launch ECI. Click the _Open Network_ button. Enter `10.25.1.210` as the **IP Address** and 36147 as the **Port**. Then Click the _Connect_ button.

Now you can proceed with the TIVA/DSP flashing procedures.

> ⚠️ **Warning:** Remember to set **Default IP Address** back to `169.254.0.5` after you finish TIVA/DSP flashing.

#### Connecting to the Slave inverter

Go to the AWS IoT portal and find your target EMC.  
Click on the _Create secure tunnel_ button.  
Choose _Create new tunnel_ and _Manual setup_.
Click _Add new service_ button and enter `TIVA_0` as the service id.  
Copy the source access token when you create the tunnel.

If you are using Windows machine, launch **cmd** and enter the following command.  
If you are using Linux machine, launch **Terminal** and enter the following command.

```bash
#!/bin/bash
ssh ikayzo@10.25.1.210
```

You will be prompted for a password. Enter `canada1234` to proceed.  
A welcome message will be displayed. You are now connected to the proxy server.

```bash
joe@joe-HP-ENVY-x360-Convertible-15-ed1xxx:~/Projects/20250715/rpi/egear-manifest$ ssh ikayzo@10.25.1.210
ikayzo@10.25.1.210's password: 
Welcome to Ubuntu 20.04.6 LTS (GNU/Linux 5.15.0-139-generic x86_64)

 * Documentation:  https://help.ubuntu.com
 * Management:     https://landscape.canonical.com
 * Support:        https://ubuntu.com/pro

1 device has a firmware upgrade available.
Run `fwupdmgr get-upgrades` for more information.


Expanded Security Maintenance for Infrastructure is not enabled.

43 updates can be applied immediately.
To see these additional updates run: apt list --upgradable

45 additional security updates can be applied with ESM Infra.
Learn more about enabling ESM Infra service for Ubuntu 20.04 at
https://ubuntu.com/20-04

New release '22.04.5 LTS' available.
Run 'do-release-upgrade' to upgrade to it.

Your Hardware Enablement Stack (HWE) is supported until April 2025.
Last login: Tue Jul 15 17:35:22 2025 from 10.25.1.63
ikayzo@ikayzo-ThinkCentre-M710e:~$
```

Enter the following command after you are connected to the proxy server.

```bash
tunnel
```

You will be prompted for the source access token. Enter the source access token you got from the AWS IoT portal.
Then you will be prompted for the region of AWS account. Simply press _Enter_ to use the default.

```bash
ikayzo@ikayzo-ThinkCentre-M710e:~$ tunnel 
Enter the source access token for the tunnel: AQGAAXj3Cpeyk0UB8jhfSMJrv5cxABsAAgABQQAMNzAwNTAyODI1NjQ3AAFUAANDQVQAAQAHYXdzLWttcwBLYXJuOmF3czprbXM6dXMtZWFzdC0yOjY4MzQ5NzY0MDMyOTprZXkvOGIzMTdjOTItZWFhNi00OWYzLTljNGEtY2IyMmFjMzc3NWE1ALgBAgEAeEwqwYCMTR47-U0mnhSrg_YRIo5UVD0XFJs8rlsJRth3AdsJsNNOzYg7KlvW5b_rzlMAAAB-MHwGCSqGSIb3DQEHBqBvMG0CAQAwaAYJKoZIhvcNAQcBMB4GCWCGSAFlAwQBLjARBAyUhYAv3ZR76V4FebsCARCAO7THCiWiKG169oaBuBPW2-qDZQnL9xeU4-2qkzBCl4p2eah9qhZWhx2pLkdgIHZvF9Hp5WyO9Gwa0hPkAgAAAAAMAAAQAAAAAAAAAAAAAAAAAIoNLR_7PF3C0yPoj6U5Bz______AAAAAQAAAAAAAAAAAAAAAQAAAC92bQP28RzuZc50Mpc_EZfmlAHlj6oA_KBR5alPDU1tUjPfkFAH51l1_ib7v8jHtisCCcPTfN9q-5PFqRBRpNY=
Enter the region of your AWS account[default us-east-2]: 
[2025-07-16T11:13:55.403889]{86118}[warning] Found access token supplied via CLI arg. Consider using environment variable AWSIOT_TUNNEL_ACCESS_TOKEN instead
[2025-07-16T11:13:55.403947]{86118}[info]    Starting proxy in source mode
[2025-07-16T11:13:55.405003]{86118}[info]    Attempting to establish web socket connection with endpoint wss://data.tunneling.iot.us-east-2.amazonaws.com:443
[2025-07-16T11:13:55.686212]{86118}[info]    Web socket session ID: 0630d2fffe2ac3eb-000032aa-0004ceec-52d083ae9c5602c6-ccb8d8d0
[2025-07-16T11:13:55.686304]{86118}[info]    Successfully established websocket connection with proxy server: wss://data.tunneling.iot.us-east-2.amazonaws.com:443
[2025-07-16T11:13:55.686596]{86118}[info]    Use port mapping:
[2025-07-16T11:13:55.686651]{86118}[info]    ---------------------------------
[2025-07-16T11:13:55.686676]{86118}[info]    TIVA_0 = 0
[2025-07-16T11:13:55.686697]{86118}[info]    ---------------------------------
[2025-07-16T11:13:55.686924]{86118}[info]    Listening for new connection on port 49097
[2025-07-16T11:13:55.686991]{86118}[info]    Listen port assigned for service id TIVA_0. New port mapping: 
[2025-07-16T11:13:55.687015]{86118}[info]    TIVA_0 = 49097
```

Verify you are connecting to service id **TIVA_0**.  
Now copy the port opened by the proxy server(i.e. 49097 from the above message).

Launch ECI. Click the _Open Network_ button. Enter `10.25.1.210` as the **IP Address** and 49097 as the **Port**. Then Click the _Connect_ button.

You can now use ECI to read inverter / battery readings, change settings on the inverter or even flash the firmware on the TIVA / DSP.

### Connecting ECI to MUC / Cascade inverter (TCP)

> ⚠️ **Note:** To use ECI connection you would need to disable the communication between the EMC and the inverter first, or else you may observe unexpected behaviour. Either go to DIGI or setup a GW connection, then set **Inverter Type** to **No Battery**.  
⚠️ **Note:** To connect to a MUC / Cascade inverter, you need to find out the IP address assigned to that inverter first. You can see the assigned IP addresses on DIGI or on the local web server.

Go to the AWS IoT portal and find your target EMC.  
Click on the _Create secure tunnel_ button.  
Choose _Create new tunnel_ and _Manual setup_.
Click _Add new service_ button and enter the appropriate service id.

For example, if your target inverter is `169.254.0.8`, enter `TIVA_2` as the service id.
If your target inverter is `169.254.0.9`, enter `TIVA_3` as the service id.

Copy the source access token when you create the tunnel.

If you are using Windows machine, launch **cmd** and enter the following command.  
If you are using Linux machine, launch **Terminal** and enter the following command.

```bash
#!/bin/bash
ssh ikayzo@10.25.1.210
```

You will be prompted for a password. Enter `canada1234` to proceed.  
A welcome message will be displayed. You are now connected to the proxy server.

```bash
joe@joe-HP-ENVY-x360-Convertible-15-ed1xxx:~/Projects/20250715/rpi/egear-manifest$ ssh ikayzo@10.25.1.210
ikayzo@10.25.1.210's password: 
Welcome to Ubuntu 20.04.6 LTS (GNU/Linux 5.15.0-139-generic x86_64)

 * Documentation:  https://help.ubuntu.com
 * Management:     https://landscape.canonical.com
 * Support:        https://ubuntu.com/pro

1 device has a firmware upgrade available.
Run `fwupdmgr get-upgrades` for more information.


Expanded Security Maintenance for Infrastructure is not enabled.

43 updates can be applied immediately.
To see these additional updates run: apt list --upgradable

45 additional security updates can be applied with ESM Infra.
Learn more about enabling ESM Infra service for Ubuntu 20.04 at
https://ubuntu.com/20-04

New release '22.04.5 LTS' available.
Run 'do-release-upgrade' to upgrade to it.

Your Hardware Enablement Stack (HWE) is supported until April 2025.
Last login: Tue Jul 15 17:35:22 2025 from 10.25.1.63
ikayzo@ikayzo-ThinkCentre-M710e:~$
```

Enter the following command after you are connected to the proxy server.

```bash
tunnel
```

You will be prompted for the source access token. Enter the source access token you got from the AWS IoT portal.
Then you will be prompted for the region of AWS account. Simply press _Enter_ to use the default.

```bash
ikayzo@ikayzo-ThinkCentre-M710e:~$ tunnel 
Enter the source access token for the tunnel: AQGAAXjup21ZZirKo3K_Uof_yba4ABsAAgABQQAMNzAwNTAyODI1NjQ3AAFUAANDQVQAAQAHYXdzLWttcwBLYXJuOmF3czprbXM6dXMtZWFzdC0yOjY4MzQ5NzY0MDMyOTprZXkvOGIzMTdjOTItZWFhNi00OWYzLTljNGEtY2IyMmFjMzc3NWE1ALgBAgEAeEwqwYCMTR47-U0mnhSrg_YRIo5UVD0XFJs8rlsJRth3ASeMnJpRwQWFPm-ipFXp-k4AAAB-MHwGCSqGSIb3DQEHBqBvMG0CAQAwaAYJKoZIhvcNAQcBMB4GCWCGSAFlAwQBLjARBAyUKNc4oXdD_Zi1ToACARCAO3YO9B8bBfA6Txm5erh6qmYaOzCjOnxRvFUiAlhLhTgnr24wV4ivXxbGwk7o3n0e9fIkkTL8PtvF8Lc0AgAAAAAMAAAQAAAAAAAAAAAAAAAAAO83tC0gZMC14PhEQ_6GNtT_____AAAAAQAAAAAAAAAAAAAAAQAAAC-MKL1mGlnemUjOlvaO1oHTnH-uctmzmhO7oTRB5vkrs86PTPFt6WqN2PuJBqIT5KWG8770j9Ewiv4XQhSwjTs=
Enter the region of your AWS account[default us-east-2]: 
[2025-07-16T13:27:45.700546]{96937}[warning] Found access token supplied via CLI arg. Consider using environment variable AWSIOT_TUNNEL_ACCESS_TOKEN instead
[2025-07-16T13:27:45.700604]{96937}[info]    Starting proxy in source mode
[2025-07-16T13:27:45.701665]{96937}[info]    Attempting to establish web socket connection with endpoint wss://data.tunneling.iot.us-east-2.amazonaws.com:443
[2025-07-16T13:27:45.994080]{96937}[info]    Web socket session ID: 0a3adbfffeb935a5-00007025-00065b3a-467e5fd3e9698b2d-1c76a364
[2025-07-16T13:27:45.994172]{96937}[info]    Successfully established websocket connection with proxy server: wss://data.tunneling.iot.us-east-2.amazonaws.com:443
[2025-07-16T13:27:45.994445]{96937}[info]    Use port mapping:
[2025-07-16T13:27:45.994499]{96937}[info]    ---------------------------------
[2025-07-16T13:27:45.994523]{96937}[info]    TIVA_2 = 0
[2025-07-16T13:27:45.994543]{96937}[info]    ---------------------------------
[2025-07-16T13:27:45.994809]{96937}[info]    Listening for new connection on port 45981
[2025-07-16T13:27:45.994877]{96937}[info]    Listen port assigned for service id TIVA_2. New port mapping: 
[2025-07-16T13:27:45.994903]{96937}[info]    TIVA_2 = 45981
```

Verify that the service id printed from the proxy server matches what you enter on the AWS IoT portal.  
Now copy the port opened by the proxy server(i.e. 45981 from the above message).

Launch ECI. Click the _Open Network_ button. Enter `10.25.1.210` as the **IP Address** and 45981 as the **Port**. Then Click the _Connect_ button.

You can now use ECI to read inverter / battery readings, change settings on the inverter or even flash the firmware on the TIVA / DSP.

### AWS IoT Jobs

You can use the AWS IoT Jobs feature to execute shell command on the EMC without tunneling into the EMC.

Go to the AWS IoT portal. Click on _Remote actions_, then click on _Jobs_.

Click the _Create Job_ button. Then select _Create custom job_.  
Enter a meaningful name as the _Name_, for example `reboot`.  
Choose the target EMC as the **Things to run this job**.
Select **From template** as the **Job document**.
Select **AWS-Run-Command** as the template.
Then enter the command you want to run on the EMC as the **command**.  
For example enter `reboot` to reboot the EMC.
Then select **Snapshot** as the **Job run type**.

The EMC will run the command you enter.
You are expected to see **Completed** in the job execution status.

## Compiling the device client

- Installing all dependencies.

```bash
sudo apt-get update
sudo apt-get install --assume-yes software-properties-common
sudo apt-get install --assume-yes build-essential
sudo apt-get install --assume-yes g++-arm-linux-gnueabihf
sudo apt-get install --assume-yes gcc-arm-linux-gnueabihf
sudo apt-get install --assume-yes gdb-multiarch
wget --ca-certificate=/etc/ssl/certs/ca-certificates.crt https://www.openssl.org/source/openssl-1.1.1n.tar.gz
tar -xvzf openssl-1.1.1n.tar.gz
export INSTALL_DIR=/usr/lib/arm-linux-gnueabihf
cd openssl-1.1.1n
./Configure linux-generic32 shared \
      --prefix=$INSTALL_DIR --openssldir=$INSTALL_DIR/openssl \
      --cross-compile-prefix=/usr/bin/arm-linux-gnueabihf-
make depend
make
sudo make install
```

- Compiling the device client binary and the unit test binary

```bash
git clone https://github.com/eguanatech/aws-iot-device-client.git
cd aws-iot-device-client
mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=../cmake-toolchain/Toolchain-armhf.cmake ../
cmake --build . --target aws-iot-device-client
cmake --build . --target test-aws-iot-device-client
```
