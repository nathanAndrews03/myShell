setenv count 5
while [ ${count} -ne 0 ]; do
echo count: ${count}
setenv count `expr ${count} - 1`
setenv count2 ${count}
while [ ${count2} -ne 0 ]; do
echo count2: ${count2}
setenv count2 `expr ${count2} - 1`
done
done

