: <= ( a b -- isA<=B )
	2dup
	<
	( a b a<b )
	rot
	( b a<b a )
	rot
	( a<b a b )
	=
	( a<b a=b )
	or
	( isA<=B )
;

: >= ( a b -- isA<=B )
	2dup
	>
	( a b a<b )
	rot
	( b a<b a )
	rot
	( a<b a b )
	=
	( a<b a=b )
	or
	( isA<=B )
;

: moveCells ( addr1 addr2 numCellsToMove -- )
	0 do
		( addr1 addr2 )
		2dup
		swap 
		( addr1 addr2 addr2 addr1 )
		i cell * + @                        ( add i cells to addr1 and dereference )
		( addr1 addr2 addr2 addr1[i] )
		swap
		( addr1 addr2 addr1[i] addr2 )
		i cell * +
		( addr1 addr2 addr1[i] addr2+i*cell )
		!                                   ( store at addr2 + i*cell )
		( addr1 addr2 )
	loop
	drop drop 
;

: moveChars ( addr1 addr2 numCharsToMove -- )
	0 do
		2dup
		swap
		i + c@
		swap
		i + c!
	loop
	drop drop 
;

: memsetCells ( val startAddr numCells -- )
	0 do
		2dup
		i cell * +
		!
	loop
	drop drop
;

: memsetChars ( val startAddr numChars -- )
	rot
	dup
	dup
	256 < swap -1 > and if
		rot rot
		( val startAddr numChars )
		0 do
			2dup
			i +
			c!
		loop
	else
		s" val: " print
		.
		s" is not within range for a char " print
	then
	drop drop
;