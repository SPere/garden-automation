# Guide: Configuring Wi-Fi via Command Line on Ubuntu Server

Follow these terminal steps to identify your network hardware interface, update your Netplan configuration, and permanently connect your server to a local Wi-Fi access point.

---

## Step 1: Identify Your Wi-Fi Interface Name

Plug in or power on your network hardware. Execute the following command to find the system designated name for your Wi-Fi card:

```bash
ip link
```

Look for a network block starting with a **`w`** (commonly **`wlan0`**, **`wlp2s0`**, or similar). 

*Note: If the terminal interface status states `DOWN`, activate the card's physical power lane by running:*
```bash
sudo ip link set YOUR_INTERFACE_NAME up
```

---

## Step 2: Write the Netplan Network Profile

Ubuntu Server processes network changes using configuration files managed inside the `/etc/netplan/` directory. Run this block to cleanly write your access parameters to disk without opening an interactive editor:

```bash
sudo cat << 'EOF' > /etc/netplan/01-netcfg.yaml
network:
  version: 2
  renderer: networkd
  wifis:
    YOUR_INTERFACE_NAME:
      dhcp4: true
      dhcp6: true
      access-points:
        "YOUR_SSID_NAME":
          password: "YOUR_WIFI_PASSWORD"
EOF
```

*Make sure to change `YOUR_INTERFACE_NAME`, `YOUR_SSID_NAME`, and `YOUR_WIFI_PASSWORD` to match your actual local hardware and network values.*

---

## Step 3: Test and Apply Configuration Changes

Safely parse the syntax validation tree of your new settings profile to prevent breaking system networking:

```bash
# Safely test configurations (automatically rolls back if you lose connection)
sudo netplan try

# Commit the configurations permanently to the system
sudo netplan apply
```

---

## Step 4: Verify Local IP Address Allocation

Confirm that your network adapter successfully requested and received a clean IP binding from your router's local DHCP pool:

```bash
ip a show dev YOUR_INTERFACE_NAME
```

If your configuration worked, you will see a valid internal IP assignment string immediately following the **`inet`** text tag.
