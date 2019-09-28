#include <iostream>
#include <cstring>
#include <bitset>
#include <ctime>
#include <vector>
using namespace std;

typedef bitset<11213> N;
typedef bitset<32> lambda;
typedef bitset<11213 - 32> N_lam;

N add(N a, N& b)
{
	N sum;
	bool carry = 0;
	for (size_t i = 0; i < a.size(); i++)
	{
		sum[i] = a[i] ^ b[i] ^ carry;
		carry = a[i] & b[i] | carry & (a[i] ^ b[i]);
	}
	if (sum.count() == sum.size()) // mod (2^N - 1)
		sum = 0;
	if (!carry) // �S���i��
		return sum;

	// ���i��Asum = sum + 1 (mod 2^N - 1)
	// e.g. 1 010 (mod 111) = 010 + 1 000 (mod 111) = 010 + 001 = 011
	for (size_t i = 0; i < a.size(); i++)
	{
		bool c1 = sum[i];
		sum[i] = sum[i] ^ carry;
		carry = c1 & carry;
		if (!carry)
			break;
	}
	if (sum.count() == sum.size()) // mod (2^N - 1)
		sum = 0;

	return sum;
}

N multiple(N& a, N& b) // multiple + mod
{
	N mul = { 0 };
	N sum = { 0 };
	for (size_t i = 0; i < a.size(); i++)
	{
		if (!a[i])
			continue;
		// e.g. 110 * 010 (mod 111) = 1 100  (mod 111) = 101 = 100 + 001 = (110 << 1) | (110 >> (3-1))
		// 110 * 100 (mod 111) = 11 000 (mod 111) = 011 = 000 + 011 = (110 << 2) | (110 >> (3-2))
		sum = (b >> (b.size() - i)) | (b << i);
		mul = add(sum, mul);
	}
	return mul;
}

N ECC(lambda m)
{
	N s = { 0 };
	int repeat = s.size() / m.size();
	for (int i = 0; i < m.size(); i++)
	{
		for (int j = 0; j < repeat ; j++)
		{
			s[j + i * repeat] = m[i];
		}
	}
	return s;
}
lambda DECC(N s , int& maxError , int& aveError)
{
	lambda m = { 0 };
	int repeat = s.size() / m.size();

	for (int i = 0 ; i < m.size() ; i++)
	{
		int count = 0;
		for (int j = 0; j < repeat; j++)
		{
			count += s[j + i * repeat];
		}
		m[i] = (count > (repeat / 2));

		int error = count;
		if (m[i])
			error = (repeat - count);

		aveError += error;
		maxError = (maxError > error ? maxError : error) ;
	}
	aveError /= m.size();
	return m;
}

/*
1.choose Mersenne prime p = 2^n - 1
2.choose an integer h = lambda and 10 * h^2 < n <= 16 * h^2
3.choose two independent two n-bits string F , G uniformly at random from all n-bit strings of Hamming weight h
4.choose a uniformly random n-bit strings R
5.public-key pk = (R , F * R + G) = (R , T) , and private-key sk = F
return pk , sk
*/
vector<N> KeyGen(int lambda)
{
	N f = { 0 }, g = { 0 }, r = { 0 };
	unsigned seed;
	seed = (unsigned)time(NULL);
	srand(seed);
	//set hamming weight h = lambda
	for (int i = 0; i < lambda; i++)
	{
		f[i] = 1;
		g[i] = 1;
	}
	//random shuffle
	for (int i = 0; i < f.size() ; i++) 
	{
		int j = (rand() % f.size());
		bool re = f[j];
		f[j] = f[i];
		f[i] = re;

		j = (rand() % g.size());
		re = g[j];
		g[j] = g[i];
		g[i] = re;

		r[i] = !(bool)(rand() % 2);
	}
	
	N t = add(multiple(f , r) , g);
	vector<N> k;
	k.push_back(r);
	k.push_back(t);
	k.push_back(f);
	return k;
}
/*
1.choose three independent strings A , B1 , B2 uniformly at random from all strings with Hamming weight h
2.write an error correcting encoding function E transforms the length of m from lambda to n
length of m = lambda , length of E(m) = n
3.Enc(pk , m) = (C1 , C2) = (A * R + B1  , (A * T + B2) xor E(m))
C1 = A * R + B1 , C2 = (A * T + B2) xor E(m)
3.return C1 , C2
*/
vector<N> Enc(vector<N> pk, lambda m)
{
	N a = { 0 }, b1 = { 0 }, b2 = { 0 };
	unsigned seed;
	seed = (unsigned)time(NULL); 
	srand(seed);
	//set hamming weight = h
	for (int i = 0; i < m.size(); i++)
	{
		a[i] = 1;
		b1[i] = 1;
		b2[i] = 1;
	}
	//random shuffle
	for (int i = 0; i < a.size(); i++)
	{
		int j = (rand() % a.size());
		bool re = a[j];
		a[j] = a[i];
		a[i] = re;

		j = (rand() % b1.size());
		re = b1[j];
		b1[j] = b1[i];
		b1[i] = re;

		j = (rand() % b2.size());
		re = b2[j];
		b2[j] = b2[i];
		b2[i] = re;
	}

	N m_ecc = ECC(m);
	N c1 = add(multiple(a, pk[0]) , b1);
	N c2 = add(multiple(a, pk[1]), b2) ^ m_ecc;
	vector<N> c;
	c.push_back(c1);
	c.push_back(c2);
	return c;
}
/*
sk = F
1.write an error correctin decoding function D that transforms the length of input from n to lambda
D = inverse of E
2.d = D( (F * C1) xor C2 )
2.return d or rejection
*/
lambda Dec(N sk, vector<N> c , int& maxError , int& aveError)
{
	lambda d = DECC( (multiple(sk , c[0]) ^ c[1]) , maxError , aveError);
	return d;
}

int main()
{
	int count = 0;
	unsigned seed;
	seed = (unsigned)time(NULL); // ���o�ɶ��ǦC
	srand(seed);
	for(size_t times = 1 ; times < 100 ; times ++)
	{
		cout << "==============================" << endl;
		cout << "times = " << times << endl;

		lambda m;
		seed = (unsigned)time(NULL); // ���o�ɶ��ǦC
		srand(seed);
		while(time(NULL) <= seed + 0.00000001){ }
		for (int i = 0; i < m.size(); i++)
		{
			m[i] = !(bool)(rand() % 2);
		}
		
		clock_t start, stop;
		
		start = clock();

		vector<N> k = KeyGen(m.size());	
	
		stop = clock();
		cout << "KeyGen time : " << double(stop - start) / CLOCKS_PER_SEC << endl;

		vector<N> pk;
		pk.push_back(k[0]);
		pk.push_back(k[1]);

		start = clock();

		vector<N> c = Enc(pk, m);

		stop = clock();
		cout << "Enc time : " << double(stop - start) / CLOCKS_PER_SEC << endl;

		N sk = k[2];
		start = clock();

		int maxError = 0, aveError = 0;
		lambda m_dec = Dec(sk , c , maxError , aveError);

		stop = clock();
		cout << "Dec time : " << double(stop - start) / CLOCKS_PER_SEC << endl;
		cout << "hamming weight of public keys r and t: " << pk[0].count() << " , " << pk[1].count() << endl;
		cout << "hamming weights of ciphertexts c1 and c2: " << c[0].count() << " , " << c[1].count() << endl;
		cout << "���� : " << m << endl;
		cout << "�ѱK : " << m_dec << endl;
		cout << "Equal(1)  or  not(0) : " << !((m ^ m_dec).count()) << endl; //���xor�ѱK��1���Ӽ��`�M = 0�A��NOT = 1 

		count += !((m ^ m_dec).count());
		
		int repeat = sk.size() / m.size();
		cout << "max error: " << maxError << endl;
		cout << "average error: " << aveError << endl;
		cout << "repeat: " << repeat << endl;
		cout << "max error percentage: " << (double)(maxError) / (double)(repeat) <<endl ;
		cout << "average error percentage: " << (double)(aveError) / (double)(repeat) << endl;
		cout << "total correct : " << count << endl;
		cout << "==============================" << endl;
	}
	
	system("pause");
	return 0;
}