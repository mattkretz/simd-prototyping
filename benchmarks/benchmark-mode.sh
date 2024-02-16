#!/bin/bash
## SPDX-License-Identifier: GPL-3.0-or-later
## Copyright © 2019-2024 GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
##                       Matthias Kretz <m.kretz@gsi.de>

no_turbo=/sys/devices/system/cpu/intel_pstate/no_turbo

write() {
  #echo "write $*"
  val="$1"
  file="$2"
  if [[ -w "$file" ]]; then
    echo "$1" > "$file"
  else
    echo "$1" | sudo tee "$file" >/dev/null
  fi
}

turn_on() {
  echo "enabling benchmark mode (no clock scaling/turbo)"
  for i in /sys/devices/system/cpu/cpu[0-9]*/cpufreq/scaling_governor; do
    write performance $i
  done
  if test -f $no_turbo; then
    write 1 $no_turbo
  #else
    #freq=$(cut -d" " -f1,2 /sys/devices/system/cpu/cpu0/cpufreq/scaling_available_frequencies)
    #freq1=${freq% *}
    #freq2=${freq#* }
    #test $(($freq2+1000)) -eq $freq1 && \
      #echo $freq2 | sudo tee /sys/devices/system/cpu/cpu[0-9]*/cpufreq/scaling_max_freq >/dev/null
  fi
}

turn_off() {
  echo "disabling benchmark mode"
  for i in /sys/devices/system/cpu/cpu[0-9]*/cpufreq/scaling_governor; do
    write powersave $i
  done
  if test -f $no_turbo; then
    write 0 $no_turbo
  #else
    #freq=$(cut -d" " -f1 /sys/devices/system/cpu/cpu0/cpufreq/scaling_available_frequencies)
    #echo $freq | sudo tee /sys/devices/system/cpu/cpu[0-9]*/cpufreq/scaling_max_freq >/dev/null
  fi
}

while (($# > 0)); do
  case "$1" in
    -h|--help)
      usage
      exit
      ;;
    --chown)
      sudo chown $USER \
        $no_turbo \
        /sys/devices/system/cpu/cpu[0-9]*/cpufreq/scaling_max_freq \
        /sys/devices/system/cpu/cpu[0-9]*/cpufreq/scaling_governor
      ;;
    on|start) turn_on ;;
    off|stop) turn_off ;;
  esac
  shift
done

# vim: tw=0 si sw=2
