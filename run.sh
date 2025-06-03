#!/bin/bash

export XDG_RUNTIME_DIR=${XDG_RUNTIME_DIR:-/tmp}

# Determine OS
#if [[ "$OSTYPE" == "darwin"* ]]; then
#  AUDIO_DRIVER="coreaudio"
#else
#  AUDIO_DRIVER="pa"
#fi

if [[ $1 = '-d' ]]; then
  qemu-system-x86_64 \
    -hda Image/x64BareBonesImage.qcow2 \
    -m 512 \
    -S -gdb tcp::1234 -d int
else
  qemu-system-x86_64 \
    -hda Image/x64BareBonesImage.qcow2 \
    -m 512
fi



if [[ $1 = '-d' ]]; then
  qemu-system-x86_64 \
    -hda Image/x64BareBonesImage.qcow2 \
    -m 512 \
    -audiodev ${AUDIO_DRIVER},id=snd0 \
    -machine pcspk-audiodev=snd0 \
    -S -gdb tcp::1234 -d int
else
  qemu-system-x86_64 \
    -hda Image/x64BareBonesImage.qcow2 \
    -m 512 \
    -audiodev ${AUDIO_DRIVER},id=snd0 \
    -machine pcspk-audiodev=snd0
fi
