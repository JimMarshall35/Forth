task taskA
task taskB

: startA taskA activate begin s" taskA" print cr pause 0 until ;
: startB taskB activate begin s" taskB" print cr pause 0 until ;

: main
	startA
	startB
	begin s" main" print cr pause 0 until
;
