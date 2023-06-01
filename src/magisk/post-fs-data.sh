#!/system/bin/sh

MODDIR="${0%/*}"
LOGFILE="${MODDIR}/post-fs-data.log"
PATCHER="${MODDIR}/gnss_patcher"
MODFILE="${MODDIR}/patched_oplusstanvbk"

SLOT="$(resetprop ro.boot.slot_suffix)"
PATH1="/dev/block/by-name/oplusstanvbk${SLOT}"
PATH2="/dev/block/bootdevice/by-name/oplusstanvbk${SLOT}"

exec > "$LOGFILE"
exec 2>&1

echo "Slot: ${SLOT}"
echo
ls -laZ "$PATH1" "$PATH2"
echo
if [ -L "$PATH1" ] || [ -L "$PATH2" ]; then
  rm -f "$MODFILE"
  if [ -L "$PATH1" ]; then
    ORIG="$(readlink -fn "$PATH1")"
  else
    ORIG="$(readlink -fn "$PATH2")"
  fi
  DEV="${ORIG}_mod"
  chmod +x "$PATCHER"
  echo "Running patcher"
  "$PATCHER" -i "$ORIG" -o "$MODFILE"
  ret=$?
  if [ $ret -ne 0 ]; then
    echo "Patcher failed!!!"
  else
    echo
    # The loop device path must be in the form '/dev/block/sdX...' because mark_boot_successful()
    # will ultimately call gpt_get_header() in gpt-utils and it will try to open the device name
    # truncated to length 'sizeof("/dev/block/sda") - 1'. This means that if the original partition
    # is '/dev/block/sdX00', it will try to open '/dev/block/sdX', so if we just use the next available
    # loop device, it will fail opening '/dev/block/loo'. Using the same prefix as the original
    # partition will solve the issue, but requires a "hack" to work.
    # We first associate our patched file with the next available loop device, then we rename it.
    NEXT_LOOP="$(/system/bin/losetup -sf "$MODFILE")"
    if [ -z "$NEXT_LOOP" ]; then
      echo "ERROR: Cannot create loop device"
    else
      mv -f "$NEXT_LOOP" "$DEV"
      ret=$?
      if [ $ret -ne 0 ]; then
        echo "ERROR: Cannot rename loop device"
      else
        chcon -v "u:object_r:vendor_custom_ab_block_device:s0" "$DEV" # Set correct SELinux context
        echo "Created loop device" "$DEV"
        ls -laZ "$DEV"
        echo
        # Create symlinks to newly created loop device
        if [ -L "$PATH1" ]; then
          ln -sfv "$DEV" "$PATH1"
        fi
        if [ -L "$PATH2" ]; then
          ln -sfv "$DEV" "$PATH2"
        fi
        echo
        ls -laZ "$PATH1" "$PATH2"
      fi
    fi
  fi
fi
