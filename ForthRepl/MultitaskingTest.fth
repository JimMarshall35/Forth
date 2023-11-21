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
		recieve ( while pausing, wait for a message to appear in my mailbox  )
		drop
		s" taskB recieved message: message box value: " print . cr	
	0 until 
;

: main
	startA
	startB
	30 0 do
		s" main. counter val: " print counter @ . cr 

		counter @ 1 +                  ( increment counter )
		counter !
		counter @ 10 = if 
			messageBoxVal @ taskB send ( send message to taskB's maibox )
			messageBoxVal @ 1 +        ( increment messageBoxVal )
			messageBoxVal !
			0 counter !                ( reset counter )
			taskA sleep
		then
		
		pause 
	loop
;
