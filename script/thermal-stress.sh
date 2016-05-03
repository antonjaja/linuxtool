#! /bin/sh


# =================================
# time : Test time
# path : save message path
# =================================
#Test time.Unit is second.
time=`expr 3600 \* 96`

#save data to this path.Android OS p4 is data partition.
mount /dev/mmcblk0p4 /mnt
path="/mnt/media/0/Download/"

#Print message
message1="/dev/tty1"
message2="/dev/ttymxc0"


print() {
	echo $1 > $message2
	echo $1 > $message1
}

showdegree(){
	echo  "degree(C): $(cat /sys/class/thermal/thermal_zone0/temp)"
	echo  "degree(C): $(cat /sys/class/thermal/thermal_zone0/temp)" > $message1
	echo  "degree(C): $(cat /sys/class/thermal/thermal_zone0/temp)" >> $path/thermal$filename.txt
}

run() {
print "start........."

cp /data/mp /bin/
chmod 777 /bin/mp
cp /data/stressapptest /bin/
chmod  777 /bin/stressapptest

#Save data filename because every device ethermac is uniquie
ethaddr=`cat /sys/class/net/eth0/address`
filename=${ethaddr:15:2}

#clear file
echo "" > $path/thermal$filename.txt 
echo "" > $path/stressapptest$filename.txt

#save degree every second.
#mp --cat /sys/class/thermal/thermal_zone0/temp $time > $path/thermal$filename.txt &

#Run stressapptest, print and save message.
stressapptest -s $time 2>&1 | tee $path/stressapptest$filename.txt | tee $message1 &

#check parameter
stresstest="dogain"
while [ $stresstest == "dogain" ]; do
	#avoid screen to save mode
	echo 0 > /sys/class/graphics/fb0/blank
	#print and save degree
	showdegree
	#Check The stressapptest was been done
	result=`tail -n 15 $path/stressapptest$filename.txt`
	if [ $(echo "$result" | grep "Stats: Completed:" | wc -l) = 1 ]; then
		stresstest="finish"
	fi
	#sleep ten second because stressapptest print message interval
	sleep 10
	#sync data to file
	sync
done

} > $message2

#first run function
run

#finish test umount save path partition
sync
sleep 3
umount /mnt
sync

#finish test reboot system
reboot

