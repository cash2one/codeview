#include <iostream>



class Rational
{
public:
	Rational(int tn = 0,int td = 0)	{	n=tn;d=td;	}

	bool operator > (const Rational& r)
	{
		return r.n > this->n;
	}

private:
	int n;
	int d;

	friend std::ostream & operator << (std::ostream & os,Rational r);

	friend bool operator > (const Rational& r1,const Rational &r2);
};

std::ostream & operator << (std::ostream & os,Rational r)
{
	os << r.n << r.d;
	return os;
}

bool operator > (const Rational& r1,const Rational &r2)
{
	return r1.n > r2.n;
}

template <class T>
inline const T & My_Max(const T& t1,const T& t2)
{
	return t1 > t2 ? t1 : t2;
}

int main()
{
	Rational r;

	std::cout<<r<<std::endl;

	int i;

	std::cin>>i;

	std::cout<<i<<std::endl;

	int a = 10, b = 20;

	int c = My_Max(10,20);

	Rational r2(10,10);

	const Rational & t3 = My_Max(r,r2);

	std::cout<<"t3"<<t3<<std::endl;

	std::cout<<c<<std::endl;

	std::cin>>i;

	return 0;
}
