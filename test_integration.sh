printf '\n%s' ".....running cunit tests....."
./tests #running cunit tests
printf '\n%s' ".....running integrations tests......"
printf '\n%s' ".....veryfing that sender is working on a perfect network....."
./receiver -o "lorem_received.txt" :: 55555 &
./sender -f lorem.txt ::1 55555
printf '\n%s\n' ".....using diff command on the two files : " &
diff lorem.txt lorem_received.txt
printf '\n%s' ".....veryfing that sender is working a network with packet loses....."
./link_sim -p 55554 -P 55555 -l 20 &>out.log &
./receiver -o "lorem_received_imperfect.txt" :: 55555 &
./sender -f lorem_imperfect.txt ::1 55554
printf '\n%s\n' ".....using diff command on the two files : " &
diff lorem_received.txt lorem_received_imperfect.txt
