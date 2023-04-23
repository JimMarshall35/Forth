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

: moveChars ( src dest numCharsToMove -- )
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

: c, ( valueToCompile -- ) here ! 1 allot ; 

: align ( -- )
	begin
		here cell % 0 = not if
			1 allot
		then
		
		here cell % not
	until
;

( string type )

: string ( capacity -- )
	create
	here
	swap
	2 cell * + allot ( + 2 cells for length and pointer to string data )
	align
	dup
	0 swap !
	cell +
	dup cell + swap !
	does>
		dup cell +
		swap @
		swap
;

: stringset ( lengthSrc dataSrc lengthDes dataDes -- )
	swap drop ( don't care about the length of the destination )
	( lengthSrc dataSrc dataDes )
	rot        ( dataSrc dataDes lengthSrc )
	dup        ( dataSrc dataDes lengthSrc lengthSrc )
	rot        ( dataSrc lengthSrc lengthSrc dataDes )
	dup        ( dataSrc lengthSrc lengthSrc dataDes dataDes )
	1 cell * - ( dataSrc lengthSrc lengthSrc dataDes lengthDesPtr )
	rot        ( dataSrc lengthSrc dataDes lengthDesPtr lengthSrc )
	( set length )
	swap !     ( dataSrc lengthSrc dataDes )
	swap       ( dataSrc dataDes lengthSrc)
	rot        ( dataSrc dataDes lengthSrc )
	( copy chars )
	moveChars
;

: stringlen ( len data -- len ) drop ;

: stringget ( index len data -- char )
	( will get char of string )
;

: stringequals ( s1len s1data s2len s2data -- areEqual )
	2dup      ( s1len s1data s2len s2data s2len s2data )
	2rot      ( s2len s2data s2len s2data s1len s1data )
	2dup      ( s2len s2data s2len s2data s1len s1data s1len s1data )
	2rot      ( s2len s2data s1len s1data s1len s1data s2len s2data )
	stringlen ( s2len s2data s1len s1data s1len s1data s2len )
	rot       ( s2len s2data s1len s1data s1data s2len s1len )
	rot       ( s2len s2data s1len s1data s2len s1len s1data )
	stringlen ( s2len s2data s1len s1data s2len s1len )
	= not if
		( s2len s2data s1len s1data )	
		drop drop drop drop
		0
	else
		( s2len s2data s1len s1data )	
		swap 0 do
			( s2len s2data s1data )
			2dup ( s2len s2data s1data s2data s1data )
			i + c@ 
			swap
			i + c@
			= not if
				drop drop drop
				R> R> drop drop
				0 return
			then 
			( s2len s2data s1data )
		loop
		drop drop drop
		-1
	then
	
;

( c++ type vectors, except of fixed capacity )

: vector ( capacity -- )
	create
	dup , ( store capacity at the start of the word )
	0 , ( then store current amount )
	0 do 0 , loop ( initialise array to 0 )
	does>
		( capacityAddr countAddr dataAddr )
		dup cell 1 * +
		dup cell 1 * +
;

: incrementCellAt ( addressOfCell -- )
	dup dup @ 1 + swap ! 
;

: decrementCellAt ( addressOfCell -- )
	dup dup @ 1 - swap ! 
;

: pushvec ( value capacityAddr countAddr dataAddr -- )
	rot rot ( value dataAddr capacityAddr countAddr )
	2dup    ( value dataAddr capacityAddr countAddr capacityAddr countAddr )
	@       ( value dataAddr capacityAddr countAddr capacityAddr count )
	swap    ( value dataAddr capacityAddr countAddr count capacityAddr )	
	@       ( value dataAddr capacityAddr countAddr count capacity )
	swap    ( value dataAddr capacityAddr countAddr capacity count )
	1 +
	swap > if
		( value dataAddr capacityAddr countAddr )
		s" cellVector is full! " print
		drop drop drop drop
	else
		( value dataAddr capacityAddr countAddr )
		incrementCellAt ( value dataAddr capacityAddr countAddr )
		swap            ( value dataAddr countAddr capacityAddr )		
		drop            ( value dataAddr countAddr )
		@               ( value dataAddr count )
		1 -             ( value dataAddr count - 1 )
		cell * +        ( value &dataAddress[count-1] )
		!
	then
;

: popvec ( capacityAddr countAddr dataAddr -- value )
	rot rot ( dataAddr capacityAddr countAddr )
	dup     ( dataAddr capacityAddr countAddr countAddr )
	@       ( dataAddr capacityAddr countAddr count )
	0 = if
		( dataAddr capacityAddr countAddr )
		s" vector is empty can't pop! " print
		drop drop drop
	else
		( dataAddr capacityAddr countAddr )
		decrementCellAt
		swap drop
		( dataAddr countAddr )
		@ cell * ( index into vector array )
		+ @      ( return cell at index )
	then
;

: countvec ( capacityAddr countAddr dataAddr -- count )
	swap @
	rot rot
	drop drop
;

: getvec ( index capacityAddr countAddr dataAddr -- value )
	rot drop rot
	( countAddr dataAddr index )
	cell * +
	@
	swap drop
;

: setvec ( value index capacityAddr countAddr dataAddr -- )
	rot drop rot
	( value countAddr dataAddr index )
	cell * +
	( value countAddr addrToStore )
	rot swap
	( countAddr value addrToStore )
	!
	drop
;