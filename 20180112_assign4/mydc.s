### --------------------------------------------------------------------
### mydc.s
###
### Desk Calculator (dc)
### --------------------------------------------------------------------

	.equ   ARRAYSIZE, 20
	.equ   EOF, -1
	.equ   p, 112
	.equ   q, 113
	.equ   plus, 43
	.equ   minus, 45
	.equ   multiple, 42
	.equ   division, 47
	.equ   remainder, 37
	.equ   power, 94
	.equ   sign, 95
	.equ   f, 102
	.equ   c, 99
	.equ   d, 100
	.equ   r, 114
	.equ   null, 0
	
.section ".rodata"

scanfFormatChar:
	.asciz "%s"

scanfFormatInt:
	.asciz "%d\n"

cEmtpyResult:
	.asciz "dc: stack empty\n"

cCheckSign:
	.asciz "cheking sign\n"
### --------------------------------------------------------------------

        .section ".data"

### --------------------------------------------------------------------

        .section ".bss"
buffer:
        .skip  ARRAYSIZE

### --------------------------------------------------------------------

	.section ".text"

	## -------------------------------------------------------------
	## int power(base, exp)
	## Calcuates base^exp and returns it.
	## -------------------------------------------------------------

	## Parameter offests:
	.equ BASE,		8
	.equ EXP,		12

	## Local variable offests:
	.equ POWER, 	-4
	.equ INDEX, 	-8

powerfunction:
	pushl 	%ebp
	movl	%esp, %ebp

	## int power = 1
	pushl 	$1

	## int index = 1
	subl 	$4, %esp
	movl 	$1, INDEX(%ebp)

powerloop:
	## if (index > exp) goto powerloopend
	movl 	INDEX(%ebp), %eax
	cmpl 	EXP(%ebp), %eax
	jg		powerloopend

	# power *= base
	movl 	POWER(%ebp), %eax
	imull	BASE(%ebp), %eax
	movl 	%eax, POWER(%ebp)

	## index++
	incl 	INDEX(%ebp)

	## goto powerloop
	jmp 	powerloop

powerloopend:
	## return power
	movl 	POWER(%ebp), %eax
	movl	%ebp, %esp
	popl 	%ebp
	ret

	## -------------------------------------------------------------
	## int main(void)
	## Runs desk calculator program.  Returns 0.
	## -------------------------------------------------------------

	.globl  main
	.type   main,@function

main:

	pushl   %ebp
	movl    %esp, %ebp

input:
	## dc number stack initialized. %esp = %ebp
	
	## scanf("%s", buffer)
	pushl	$buffer
	pushl	$scanfFormatChar
	call    scanf
	addl    $8, %esp

	## check if user input EOF
	cmp	$EOF, %eax
	je	quit

	movl 	$0, %ebx

checkdigit:
	## isdigit(buffer[0])
	movzx	buffer(%ebx), %eax
	pushl	%eax
	call 	isdigit
	addl	$4, %esp

	## check if user input is a digit
	cmp 	$0, %eax
	je		checkp

	## check if next byte is null
	addl 	$1, %ebx
	movzx 	buffer(%ebx), %eax
	cmp 	$null, %eax
	je 		digit
	jmp 	checkdigit

checkp:
	## check if user input is 'p'
	movl	$0, %ebx
	movzx	buffer(%ebx), %eax
	cmp		$p, %eax
	jne 	checkq

	## check if stack is empty
	cmp		%esp, %ebp
	jne 	pNotEmpty

	## printf("dc: stack emtpy\n")
	pushl	$cEmtpyResult
	call 	printf
	addl	$4, %esp

	jmp		input

pNotEmpty:
	## printf("%d\n", (int)stack.top())
	pushl	(%esp)
	pushl	$scanfFormatInt
	call 	printf
	addl	$8, %esp
	jmp 	input

checkq:
	## check if user input is 'q'
	movl	$0, %ebx
	movzx	buffer(%ebx), %eax
	cmp 	$q, %eax
	jne	 	checkplus
	jmp 	quit

checkplus:
	## check if suer input is '+'
	movl	$0, %ebx
	movzx	buffer(%ebx), %eax
	cmp 	$plus, %eax
	jne 	checkminus

	## check if stack is empty
	cmp 	%esp, %ebp
	jne		plusNotEmpty1

	## printf("dc: stack emtpy\n")
	pushl	$cEmtpyResult
	call 	printf	
	addl	$4, %esp
	jmp 	input

plusNotEmpty1:
	## stack.pop()
	movl 	(%esp), %eax
	addl	$4, %esp

	## check if stack is emtpy
	cmp 	%esp, %ebp
	jne 	plusNotEmpty2

	## printf("dc: stack empty\n")
	pushl 	%eax
	pushl 	$cEmtpyResult
	call 	printf
	addl 	$4, %esp
	jmp 	input

plusNotEmpty2:
	## stack.pop()
	movl 	(%esp), %ebx
	addl	$4, %esp

	## add two operands and push it
	addl	%eax, %ebx
	pushl 	%ebx
	jmp 	input

checkminus:	
	## check if user input is '-'
	movl 	$0, %ebx
	movzx 	buffer(%ebx), %eax
	cmp 	$minus, %eax
	jne 	checkmultiple

	## check if stack is empty
	cmp 	%esp, %ebp
	jne		minusNotEmpty1

	## printf("dc: stack emtpy\n")
	pushl	$cEmtpyResult
	call 	printf	
	addl	$4, %esp
	jmp 	input

minusNotEmpty1:
	## stack.pop()
	movl 	(%esp), %ebx
	addl	$4, %esp

	## check if stack is emtpy
	cmp 	%esp, %ebp
	jne 	minusNotEmpty2

	## printf("dc: stack empty\n")
	pushl 	%ebx
	pushl 	$cEmtpyResult
	call 	printf
	addl 	$4, %esp
	jmp 	input

minusNotEmpty2:
	## stack.pop()
	movl 	(%esp), %eax
	addl 	$4, %esp

	## substract two operands and push it
	subl 	%ebx, %eax	
	pushl 	%eax
	jmp 	input

checkmultiple:
	## check if user input is '*'
	movl 	$0, %ebx
	movzx 	buffer(%ebx), %eax
	cmp 	$multiple, %eax
	jne 	checkdivision

	## check if stack is empty
	cmp 	%esp, %ebp
	jne		multipleNotEmpty1

	## printf("dc: stack emtpy\n")
	pushl	$cEmtpyResult
	call 	printf	
	addl	$4, %esp
	jmp 	input

multipleNotEmpty1:
	## stack.pop()
	movl 	(%esp), %ebx
	addl	$4, %esp

	## check if stack is emtpy
	cmp 	%esp, %ebp
	jne 	multipleNotEmpty2

	## printf("dc: stack empty\n")
	pushl 	%ebx
	pushl 	$cEmtpyResult
	call 	printf
	addl 	$4, %esp
	jmp 	input

multipleNotEmpty2:
	## stack.pop()
	movl 	(%esp), %eax
	addl 	$4, %esp

	## multiply two operands and push it
	imull 	%eax, %ebx
	pushl 	%ebx
	jmp 	input	

checkdivision:
	## check if user input is '/'
	movl 	$0, %ebx
	movzx 	buffer(%ebx), %eax
	cmp 	$division, %eax
	jne 	checkremainder

	## check if stack is empty
	cmp 	%esp, %ebp
	jne		divisionNotEmpty1

	## printf("dc: stack emtpy\n")
	pushl	$cEmtpyResult
	call 	printf	
	addl	$4, %esp
	jmp 	input

divisionNotEmpty1:
	## stack.pop()
	movl 	(%esp), %ebx
	addl	$4, %esp

	## check if stack is emtpy
	cmp 	%esp, %ebp
	jne 	divisionNotEmpty2

	## printf("dc: stack empty\n")
	pushl 	%ebx
	pushl 	$cEmtpyResult
	call 	printf
	addl 	$4, %esp
	jmp 	input

divisionNotEmpty2:
	## stack.pop()
	movl 	(%esp), %eax
	addl 	$4, %esp

	## divide two operands and push it
	xor		%edx, %edx
	idivl 	%ebx
	pushl 	%eax	
	jmp 	input	

checkremainder:
	## check if user input is '%'
	movl 	$0, %ebx
	movzx 	buffer(%ebx), %eax
	cmp 	$remainder, %eax
	jne 	checkpower

	## check if stack is empty
	cmp 	%esp, %ebp
	jne		remainderNotEmpty1

	## printf("dc: stack emtpy\n")
	pushl	$cEmtpyResult
	call 	printf	
	addl	$4, %esp
	jmp 	input

remainderNotEmpty1:
	## stack.pop()
	movl 	(%esp), %ebx
	addl	$4, %esp

	## check if stack is emtpy
	cmp 	%esp, %ebp
	jne 	remainderNotEmpty2

	## printf("dc: stack empty\n")
	pushl 	%ebx
	pushl 	$cEmtpyResult
	call 	printf
	addl 	$4, %esp
	jmp 	input

remainderNotEmpty2:
	## stack.pop()
	movl 	(%esp), %eax
	addl 	$4, %esp

	## calculate remainder and push it
	xor		%edx, %edx
	idivl 	%ebx
	pushl 	%edx	
	jmp 	input	

checkpower:
	## check if user input is '^'
	movl 	$0, %ebx
	movzx	buffer(%ebx), %eax
	cmpl	$power, %eax
	jne 	checksign

	## check if stack is emtpy
	cmp 	%esp, %ebp
	jne		powerNotEmpty1

	pushl 	$cEmtpyResult
	call 	printf 
	addl 	$4, %esp
	jmp 	input

powerNotEmpty1:
	## stack.pop()
	## exp to ebx
	movl 	(%esp), %ebx
	addl 	$4, %esp

	## check if stack is emtpy
	cmp 	%esp, %ebp
	jne 	powerNotEmpty2

	pushl 	%ebx
	pushl 	$cEmtpyResult
	call 	printf
	addl 	$4, %esp
	jmp 	input

powerNotEmpty2:
	## stack.pop()
	## base to eax
	movl 	(%esp), %eax
	addl 	$4, %esp

	## powerfunction(base, exp)
	pushl 	%ebx
	pushl	%eax
	call 	powerfunction
	addl 	$8, %esp

	## push the result to stack
	pushl 	%eax
	jmp 	input

checksign:
	## check if user input is '_'
	movl 	$0, %ebx
	movzx	buffer(%ebx), %eax
	cmp 	$sign, %eax
	jne		checkf

	## replace '_' with '-'
	movb 	$minus, buffer(%ebx)
	jmp		digit

checkf:
	## check if user input is 'f'
	movl 	$0, %ebx
	movzx	buffer(%ebx), %eax
	cmp 	$f, %eax
	jne 	checkc

	movl 	%esp, %ebx

floop:
	## check if stack is empty
	cmpl	%ebx, %ebp
	je 		floopend

	## printf(stack.peek())
	pushl 	(%ebx)
	pushl 	$scanfFormatInt
	call 	printf
	addl 	$8, %esp

	addl 	$4, %ebx

	jmp 	floop

floopend:
	jmp 	input

checkc:
	## check if user input is 'c'
	movl 	$0, %ebx
	movzx 	buffer(%ebx), %eax
	cmp 	$c, %eax
	jne 	checkd

	## clear the contents of the stack
	movl 	%ebp, %esp

	jmp 	input

checkd:
	## check if user input is 'd'
	movl 	$0, %ebx
	movzx	buffer(%ebx), %eax
	cmp 	$d, %eax
	jne 	checkr

	## check if stack is empty
	cmpl 	%esp, %ebp
	jne 	dNotEpmty 

	## printf("dc: stack empty\n")
	pushl 	$cEmtpyResult
	call 	printf
	addl 	$4, %esp

	jmp 	input

dNotEpmty:
	## duplicate the top-most entry of the stack and push it
	pushl 	(%esp)

	jmp 	input

checkr:
	## check if user input is 'r'
	movl 	$0, %ebx
	movzx	buffer(%ebx), %eax
	cmp 	$r, %eax
	jne		input

	## check if stack is empty
	cmpl 	%esp, %ebp
	jne		rNotEmpty1

	## printf("dc: stack empty\n")
	pushl 	$cEmtpyResult
	call 	printf
	addl 	$4, %esp

	jmp 	input

rNotEmpty1:
	## stack.pop()
	movl 	(%esp), %ebx
	addl 	$4, %esp

	## check if stack is empty
	cmpl 	%esp, %ebp
	jne		rNotEmpty2

	## printf("dc: stack empty\n")
	pushl 	%ebx
	pushl 	$cEmtpyResult
	call 	printf
	addl 	$4, %esp

	jmp 	input

rNotEmpty2:
	## stack.pop()
	movl 	(%esp), %ecx
	addl 	$4, %esp

	## reverse the order of the top two values of the stack
	pushl 	%ebx
	pushl	%ecx

	jmp 	input	

digit:
	## atoi(buffer)
	movl	$buffer, %eax
	pushl	%eax
	call 	atoi
	addl 	$4, %esp

	## stack.push()
	pushl 	%eax

	jmp input
	
	## PSEUDO-CODE
	## /*
	##  * In this pseudo-code we are assuming that no local variables are created
	##  * in the _main_ process stack. In case you want to allocate space for local
	##  * variables, please remember to update logic for 'empty dc stack' condition
	##  * (lines 6, 15 and 20) accordingly.
	##  */
	##
	##1 while (1) {
	##2	if (scanf("%s", buffer) == EOF)
	##3		return 0;
	##4 	if (!isdigit(buffer[0])) {
	##5			if (buffer[0] == 'p') {
	##6				if (stack.peek() == NULL) { /* is %esp == %ebp? */
	##7					printf("dc: stack empty\n");
	##8				} else {
	##9					printf("%d\n", (int)stack.top()); /* value is already pushed in the stack */
	##10			}
	##11		} else if (buffer[0] == 'q') {
	##12			goto quit;
	##13		} else if (buffer[0] == '+') {
	##14			int a, b;
	##15			if (stack.peek() == NULL) {
	##16				printf("dc: stack empty\n");
	##17				continue;
	##18			}
	##19			a = (int)stack.pop();
	##20			if (stack.peek() == NULL) {
	##21				printf("dc: stack empty\n");
	##22				stack.push(a); /* pushl some register value */
	##23				continue;
	##24			}
	##25			b = (int)stack.pop(); /* popl to some register */
	##26			res = a + b;
	##27 			stack.push(res);
	##28		} else if (buffer[0] == '-') {
	##29			/* ... */
	##30		} else if (buffer[0] == '^') {
	##31			/* ... powerfunc() ... */
	##32		} else if { /* ... and so on ... */
	##33 	} else { /* the first no. is a digit */
	##34		int no = atoi(buffer);
	##35		stack.push(no);	/* pushl some register value */
	##36	}
	##37 }

quit:	
	## return 0
	movl    $0, %eax
	movl    %ebp, %esp
	popl    %ebp
	ret
