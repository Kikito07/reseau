#!/bin/sh
printf '\n%s' ".....running cunit tests....."
./tests #running cunit tests
printf '\n%s' ".....running integrations tests......"
printf '\n%s' ".....veryfing that sender is working on a perfect network....."

./receiver1 -o "lorem_received.txt" :: 55555 &>out.log &
./sender -f lorem.txt ::1 55555
printf '\n\n' ""
if diff lorem.txt lorem_received.txt >/dev/null ; then
  echo Same files
else
  echo Different files
fi
printf '\n%s\n\n' ".....veryfing memory leaks and that sender is working in an imperfect network....."
./link_sim -p 55554 -P 55555 -l 10 -d 100 -j 10 -e 3 -c 3 &>out.log &
./receiver1 -o "lorem_received_imperfect.txt" :: 55555 &>out.log&
valgrind ./sender -f lorem_imperfect.txt ::1 55554
printf '\n' ""
if diff lorem.txt lorem_received.txt >/dev/null ; then
  echo Same files
else
  echo Different files
fi

