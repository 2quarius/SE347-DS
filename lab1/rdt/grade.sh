#!/bin/bash
make clean
make rdt_sim

rm -rf test/
mkdir test/

sim_time=100
msg_arrivalint=0.1
msg_size=100
outoforder_loss_corrupt=(0.02 0.15 0.30)
tracing_level=0

echo "\n" > test/input

for var in ${outoforder_loss_corrupt[@]}
do
    ./rdt_sim $sim_time $msg_arrivalint $msg_size ${var} 0 0 $tracing_level < test/input > test/test_${var}_0_0_0.log
done

for var in ${outoforder_loss_corrupt[@]}
do
    ./rdt_sim $sim_time $msg_arrivalint $msg_size 0 ${var} 0 $tracing_level < test/input > test/test_0_${var}_0_0.log
done

for var in ${outoforder_loss_corrupt[@]}
do
    ./rdt_sim $sim_time $msg_arrivalint $msg_size 0 0 ${var} $tracing_level < test/input > test/test_0_0_${var}_0.log
done

for var in ${outoforder_loss_corrupt[@]}
do
    ./rdt_sim $sim_time $msg_arrivalint $msg_size ${var} ${var} ${var} $tracing_level < test/input > test/test_${var}_${var}_${var}_0.log
done