#!/bin/bash

set -euo pipefail

DEVICE="${1:-}"
ARCH="${2:-x86_64}"

if [[ -z "$DEVICE" ]]; then
    echo "Usage: $0 /dev/sdX [arch]"
    echo "Example: $0 /dev/sdc riscv64"
    exit 1
fi

if [[ ! -b "$DEVICE" ]]; then
    echo "Error: '$DEVICE' is not a block device."
    lsblk -o NAME,SIZE,TYPE,MOUNTPOINT
    exit 1
fi

IMAGE_NAME="air-${ARCH}.hdd"

if [[ ! -f "$IMAGE_NAME" ]]; then
    echo "Error: $IMAGE_NAME not found. Try running 'make all-hdd ARCH=$ARCH' first."
    exit 1
fi

echo "About to write $IMAGE_NAME to $DEVICE. This will DESTROY all data on $DEVICE."
echo -n "Are you really, truly, absolutely sure? (yes/no): "
read CONFIRM

if [[ "$CONFIRM" != "yes" ]]; then
    echo "Wise choice. Exiting."
    exit 1
fi

echo "Writing $IMAGE_NAME to $DEVICE..."
sudo dd if="$IMAGE_NAME" of="$DEVICE" bs=4M status=progress conv=fsync

echo "Syncing..."
sync

echo "Done. $DEVICE now contains the ISO data WOOP WOOP!!"