LINES=`cat test`
for line in $LINES 
do
    #echo $line
    grep $line test
done