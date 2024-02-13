#!/bin/sh

srcfile="$1"
asmfile="$2"

parse=false
n=0
cat "$srcfile" | while read line; do
  if ! $parse; then
    [ "$line" = "/* codegen" ] && parse=true
  else
    case "$line" in
      '')
        n=$((n+1))
        ;;

      *'*/')
        parse=false
        ;;

      '^'*)
        regex="$line"
        n=1
        ;;

      *)
        asm="$(grep -A$n "$regex" "$asmfile" | tail -n1)"
        if ! echo "$asm" | grep --color=auto "$line"; then
          echo "Failure on '$regex' in line $n: $asm."
          echo "Expected '$line'."
          exit 2
        fi
        n=$((n+1))
        ;;
    esac
  fi
done
[ $? = 1 ]
