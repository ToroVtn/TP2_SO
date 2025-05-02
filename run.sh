#!/bin/bash

if [[ $1 = '-d' ]]; then
  qemu-system-x86_64 -hda Image/x64BareBonesImage.qcow2 -m 512 -audiodev pa,id=random_id -machine pcspk-audiodev=random_id -S -gdb tcp::1234 -d int
else
  qemu-system-x86_64 -hda Image/x64BareBonesImage.qcow2 -m 512 -audiodev pa,id=random_id -machine pcspk-audiodev=random_id
fi
