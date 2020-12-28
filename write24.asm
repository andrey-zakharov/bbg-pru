 .global write24
;
r_bit_num	.set	r6
r_sleep_counter	.set	r7
r_high_time	.set	r15
r_low_time	.set	r16
value	.set	r14
CYCLE_TIME	.set	10 ; 2 ops in cycle by 5 ns each
P9_29	.set	1
T0H	.set	(300 / CYCLE_TIME)
T0L	.set	(900 / CYCLE_TIME)
T1H	.set	(900 / CYCLE_TIME)
T1L	.set	(300 / CYCLE_TIME)
write24:
	ldi	r_bit_num, 24
l_bit_loop:
	sub r_bit_num, r_bit_num, 1
	qbbs write_one, value, r_bit_num
	ldi	r_high_time, T0H - 1
	ldi	r_low_time, T0L - 4
non_exit_pad: 	
	ldi	r_low_time, T0L - 4
	jmp signal
write_one:
	ldi	r_high_time, T1H - 4
	ldi	r_low_time, T1L - 8
signal:
 	set r30, r30, P9_29
l:	sub r_high_time, r_high_time, 1
 	qbne l, r_high_time, 0
 	clr r30, r30, P9_29
l2:	sub r_low_time, r_low_time, 1
 	qbne l2, r_low_time, 0
 	qbne l_bit_loop, r_bit_num, 0
	JMP r3.w2