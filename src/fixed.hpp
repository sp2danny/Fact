
#pragma once

template<int N, int M>
struct fixed
{
	unsigned char ip[N] = {0};
	unsigned char fp[M] = {0};

	fixed() = default;
	fixed(int val)
	{
		for (int i = N - 1; i >= 0; --i)
		{
			ip[i] = val % 256;
			val /= 256;
		}
	}

};

template<int N, int M>
fixed<N,M>& operator += (fixed<N, M>& lhs, const fixed<N,M>& rhs)
{
	short s, l, r, c=0;
	for (int i = M - 1; i >= 0; --i)
	{
		l = lhs.fp[i];
		r = rhs.fp[i];
		s = l + r + c;
		lhs.fp[i] = s % 256;
		c = s / 256;
	}
	for (int i = N - 1; i >= 0; --i)
	{
		l = lhs.ip[i];
		r = rhs.ip[i];
		s = l + r + c;
		lhs.ip[i] = s % 256;
		c = s / 256;
	}
	return lhs;
}

template<int N, int M>
fixed<N, M> operator + (const fixed<N, M>& lhs, const fixed<N, M>& rhs)
{
	fixed<N, M> res = lhs;
	res += rhs;
	return res;
}





