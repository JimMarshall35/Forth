task taskA
task taskB

0 var counter
0 var messageBoxVal

: startA taskA activate 
	begin 
		s" taskA" print cr pause 0
	until 
;

: startB taskB activate 
	begin
		
		recieve 
		
		drop
		s" taskB recieved message: message box value: " print . cr
		0
	until 
;

: main
	startA
	startB
	begin
		
		counter @ 1 +
		counter !
		counter @ 10 = if
			messageBoxVal @ taskB send
			messageBoxVal @ 1 +
			messageBoxVal !
			0 counter !
		then
		counter @ .
		s" main" print cr pause 
	0 until
;
