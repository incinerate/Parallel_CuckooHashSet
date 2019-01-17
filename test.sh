for i in 4 8 16 32 64
do
    for j in {1..10}
    do
        ./obj32/test 8 50 16000 $i-2 $i
    done
done
