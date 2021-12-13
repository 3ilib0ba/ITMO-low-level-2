%define NEXT 0
%macro colon 2
	%ifid %2			
		%2: dq NEXT
		%define NEXT %2
	%else
		%fatal "Expected label ID as the second argument"
	%endif
	%ifstr %1
		db %1, 0
	%else
		%fatal "Expected key value string as the first argument"
	%endif
%endmacro 

