#!/bin/bash

for ((i=1; i<=10; i++)); do
    param1=$i
    param2=$((i * 2))

    echo "{\"param1\":\"$param1\", \"param2\":\"$param2\"}" | socat - TCP:127.0.0.1:2323

    sleep 1   # пауза 1 сек (можно убрать)
done

