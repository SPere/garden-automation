# Guide: Loading GitHub App Key from a USB Drive on Ubuntu Server

Follow these terminal steps to mount your USB drive and generate the Kubernetes secret without manually typing out the cryptographic key data.

---

## Step 1: Locate the USB Drive Partition

Plug the USB drive into your Ubuntu server hardware. Run the following command to identify the drive name and partition:

```bash
lsblk
```

Look for a storage block that matches the size of your USB drive (commonly designated as `sdb` or `sdc`). Note down the specific partition name, which is typically **`sdb1`** or **`sdc1`**.

---

## Step 2: Mount the USB Drive

Ubuntu Server does not mount external storage drives automatically. Create a temporary folder and mount the target partition manually:

```bash
# 1. Create a mounting directory
sudo mkdir -p /mnt/usb

# 2. Mount the partition (replace 'sdb1' with your actual partition name)
sudo mount /dev/sdb1 /mnt/usb
```

---

## Step 3: Verify the File Status

Confirm the file is visible by listing the contents of your newly mounted folder:

```bash
ls -l /mnt/usb
```

*(Ensure you see your file listed, for example: `k3s-arc-runner.private-key.pem`)*

---

## Step 4: Generate the K3s Secret Directly from the USB Path

Use the `kubectl --from-file` parameter to read the token directly from the mounted partition. This completely eliminates the need to copy, paste, or type the raw text contents:

```bash
# 1. Export your unique GitHub App IDs
export APP_ID="YOUR_GITHUB_APP_ID"
export INSTALL_ID="YOUR_GITHUB_INSTALLATION_ID"

# 2. Automatically generate the secret from the file path
kubectl create secret generic arc-github-app-secret \
  --namespace=arc-runners \
  --create-namespace \
  --from-literal=github_app_id="\$APP_ID" \
  --from-literal=github_app_installation_id="\$INSTALL_ID" \
  --from-file=github_app_private_key=/mnt/usb/YOUR_KEY_FILE_NAME.pem
```

---

## Step 5: Safely Unmount the USB Hardware

Once the secret is successfully saved inside the cluster, safely unmount the partition before unplugging the physical USB drive:

```bash
sudo umount /mnt/usb
```
