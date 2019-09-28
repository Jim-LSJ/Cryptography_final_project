1.file name n_3217 means length of modulo n = 3217 bits corresponding to message length h = 64 bits

2.the output of the file

	KeyGen time: the time that key-generation take
	Enc time: the time that encrpytion take
	Dec time: the time that decryption take
	
	Equal(1) or not(0): If the message and the result of decryption are the same, output is 1; if not, ouput is 0.
	
	repeat: the times that error-correcting-code repeat a bit of the message, equal to (n / h)
	
	max error: During decoding error-correcting-code, we count the errors in each bit. The max error is the largest error.
	average error: the average error is (the total error bits) / (h bits)
	
	max error percentage: the percentage that max error takes among the repeat times.  (max error / repeat) 
	average error percentage: the percentage that average error takes among the repeat times.  (average error / repeat)
	
	total correct: the counter of Equal(1) or not(0)
