#!/bin/tcsh
#############################################
# Initialize
#############################################
if (-e totalgoodruns.out) rm totalgoodruns.out
cat > totalgoodruns.out << @EOI
@EOI




#############################################
# find runs
#############################################
setenv finishedruns `ls -ltr  *.stdout | wc -l`
setenv finishedrunlist `ls -ltr *.stdout | awk '{print $9}'`

echo Num of runs : ${finishedruns}
echo  ${finishedrunlist}

#############################################
# loop over runs
#############################################
echo
echo
echo loop over runs ..
echo 

set count = 0
foreach finishedrun ($finishedrunlist)
echo filename : ${finishedrun}

if (-e filteredoutput) rm filteredoutput

cat ${finishedrun} | grep EVENTINFO > filteredoutput
#setenv runnumber `cat ${finishedrun} | grep "Reading SimSignal" | awk -F "#" '{print $3}' | tail -1`
setenv runnumber `cat ${finishedrun} | grep EVENTINFO | head -1 | awk '{print $2}'`

echo runnumber : ${runnumber}

#setenv linemax `cat -n filteredoutput | grep ${runnumber} |  tail -1 | awk '{print $1}'`
set line = `cat -n filteredoutput | grep ${runnumber} |  tail -1 | awk '{print $1}'`
@ line = $line - 1
# how many events?
#echo ${linemax}
echo ${line}


setenv numofevents `cat ${finishedrun} | grep ${runnumber} | wc`
echo $numofevents

head -${line} filteredoutput  > ${finishedrun}.fil
cat ${finishedrun}.fil | awk '{for(i=2;i<=NF;i++) printf("%s ",$i); printf("\n");}' >> totalgoodruns.out
rm filteredoutput
rm ${finishedrun}.fil

end


