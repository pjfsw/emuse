#!/bin/sh

echo U > /tmp/emuse-uart && sleep 0.5 &&  cat build/$1 > /tmp/emuse-uart
