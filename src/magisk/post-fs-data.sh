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
ls -la "$PATH1" "$PATH2"
echo
if [ -L "$PATH1" ] || [ -L "$PATH2" ]; then
  rm -f "$MODFILE"
  if [ -L "$PATH1" ]; then
    orig="$(readlink -fn ${PATH1})"
  else
    orig="$(readlink -fn ${PATH2})"
  fi
  chmod +x "$PATCHER"
  echo "Running patcher"
  "$PATCHER" -i "$orig" -o "$MODFILE"
  ret=$?
  if [ $ret -ne 0 ]; then
    echo "Patcher failed!!!"
  else
    echo
    DEV="$(/system/bin/losetup -sf $MODFILE)"
    if [ -z "$DEV" ]; then
     echo "ERROR: Cannot create loop device"
    else
      echo "Created loop device" "$DEV"
      chcon -v "u:object_r:vendor_custom_ab_block_device:s0" "${DEV}"
      if [ -L "$PATH1" ]; then
        ln -sfv "${DEV}" "$PATH1"
      fi
      if [ -L "$PATH2" ]; then
        ln -sfv "${DEV}" "$PATH2"
      fi
      echo
      ls -la "$PATH1" "$PATH2"
    fi
  fi
fi
