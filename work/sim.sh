#1/usr/bin/bash

read -p "1 for perl other for compress " p
if [ $p -eq 1 ]; then 
	echo "running $1 $1 $1 $2 perl"
	max="15000000" 
else 
	echo "running $1 $1 $1 $2 compress"
	max="50000000"
fi

if [ $1 -eq 8 ]; then 
	latfast="1"
	latslow="2"
else 
	latfast="2"
	latslow="3"
fi


configvar="
-seed                             1 
-nice                             0 
-max:inst                         $max 
-fastfwd                          5000000 
-fetch:ifqsize                    $1 
-fetch:mplat                      3 
-fetch:speed                      1 
-bpred                         2lev 
-bpred:bimod           2048 
-bpred:2lev            1 4096 8 0 
-bpred:comb            1024 
-bpred:ras                        8 
-bpred:btb             128 2 
-decode:width                     $1 
-issue:width                      $1 
-issue:inorder                false 
-issue:wrongpath               true 
-commit:width                     $2 
-ruu:size                         8 
-lsq:size                         16 
-cache:dl1             dl1:32:64:4:l 
-cache:dl1lat                     $latfast 
-cache:dl2             ul2:512:128:8:l 
-cache:dl2lat                     $latslow 
-cache:il1             il1:64:64:2:f 
-cache:il1lat                     $latfast 
-cache:il2                      dl2 
-cache:il2lat                     $latslow 
-cache:flush                  false 
-cache:icompress              false 
-mem:lat               100 20 
-mem:width                        8 
-tlb:itlb              itlb:16:4096:4:l 
-tlb:dtlb              dtlb:32:4096:4:l 
-tlb:lat                         100 
-res:ialu                         1 
-res:imult                        1 
-res:memport                      2 
-res:fpalu                        1 
-res:fpmult                       1 
-bugcompat                    false 
"

echo "$configvar" > newconfig
wait 
echo "Starting sim" 
if [ $p -eq 1 ]; then 
	sim-outorder -config newconfig -redir:sim perl_$1_$2.out spec95-little/perl.ss perl-tests.pl
	cat perl_$1_$2.out
else 
	sim-outorder -config test.config -redir:sim compress_$1_$2.out spec95-little/compress95.ss < compress95.in cat compress_$1_$2.out 

fi
