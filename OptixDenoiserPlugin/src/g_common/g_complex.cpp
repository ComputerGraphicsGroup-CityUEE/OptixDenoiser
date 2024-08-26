#include "g_complex.h"

GComplex::GComplex(): m_re(0.), m_im(0.) {}
GComplex::GComplex(double re, double im): m_re(re), m_im(im) {}
GComplex::GComplex(double val): m_re(val), m_im(0.) {}

GComplex& GComplex::operator=(const double val)
{
	m_re = val;
	m_im = 0.;
	return *this;
}
GComplex& GComplex::operator=(const GComplex &val)
{
	m_re = val.m_re;
	m_im = val.m_im;
	return *this;
}

GComplex GComplex::conj() const
{
	return GComplex(m_re, -m_im);
}
GComplex GComplex::cis( double theta )
{
	return GComplex( cos(theta), sin(theta) );
}

double GComplex::abs2() const
{
	return m_re * m_re + m_im * m_im;
}

double GComplex::abs() const
{
	return sqrt( m_re*m_re + m_im*m_im );
}

double GComplex::ang() const
{
	if( m_re==0 && m_im==0 )
		return 0;
	else
	  return atan2(m_im,m_re);
}

GComplex GComplex::operator+(const GComplex& other) const
{
	return GComplex(m_re + other.m_re, m_im + other.m_im);
}

GComplex GComplex::operator-(const GComplex& other) const
{
	return GComplex(m_re - other.m_re, m_im - other.m_im);
}

GComplex GComplex::operator*(const GComplex& other) const
{
	return GComplex(m_re * other.m_re - m_im * other.m_im,
		m_re * other.m_im + m_im * other.m_re);
}

GComplex GComplex::operator/(const GComplex& other) const
{
	const double denominator = other.m_re * other.m_re + other.m_im * other.m_im;
	return GComplex((m_re * other.m_re + m_im * other.m_im) / denominator,
		(m_im * other.m_re - m_re * other.m_im) / denominator);
}

GComplex& GComplex::operator+=(const GComplex& other)
{
	m_re += other.m_re;
	m_im += other.m_im;
	return *this;
}

GComplex& GComplex::operator-=(const GComplex& other)
{
	m_re -= other.m_re;
	m_im -= other.m_im;
	return *this;
}

GComplex& GComplex::operator*=(const GComplex& other)
{
	const double temp = m_re;
	m_re = m_re * other.m_re - m_im * other.m_im;
	m_im = m_im * other.m_re + temp * other.m_im;
	return *this;
}

GComplex& GComplex::operator/=(const GComplex& other)
{
	const double denominator = other.m_re * other.m_re + other.m_im * other.m_im;
	const double temp = m_re;
	m_re = (m_re * other.m_re + m_im * other.m_im) / denominator;
	m_im = (m_im * other.m_re - temp * other.m_im) / denominator;
	return *this;
}






GComplex& GComplex::operator++()
{
	++m_re;
	return *this;
}
GComplex& GComplex::operator--()
{
	--m_re;
	return *this;
}
GComplex GComplex::operator++(int)
{
	GComplex temp(*this);
	++m_re;
	return temp;
}
GComplex GComplex::operator--(int)
{
	GComplex temp(*this);
	--m_re;
	return temp;
}





GComplex GComplex::operator+(const double val) const
{
	return GComplex(m_re + val, m_im);
}

GComplex GComplex::operator-(const double val) const
{
	return GComplex(m_re - val, m_im);
}

GComplex GComplex::operator*(const double val) const
{
	return GComplex(m_re * val, m_im * val);
}

GComplex GComplex::operator/(const double val) const
{
	return GComplex(m_re / val, m_im / val);
}

GComplex& GComplex::operator+=(const double val)
{
	m_re += val;
	return *this;
}

GComplex& GComplex::operator-=(const double val)
{
	m_re -= val;
	return *this;
}

GComplex& GComplex::operator*=(const double val)
{
	m_re *= val;
	m_im *= val;
	return *this;
}

GComplex& GComplex::operator/=(const double val)
{
	m_re /= val;
	m_im /= val;
	return *this;
}

bool GComplex::operator==(const GComplex &other) const
{
	return m_re == other.m_re && m_im == other.m_im;
}

bool GComplex::operator!=(const GComplex &other) const
{
	return m_re != other.m_re || m_im != other.m_im;
}

bool GComplex::operator==(const double val) const
{
	return m_re == val && m_im == 0.;
}

bool GComplex::operator!=(const double val) const
{
	return m_re != val || m_im != 0.;
}



// the friend functions of GComplex

GComplex operator+(const double left, const GComplex& right)
{
	return GComplex(left + right.m_re, right.m_im);
}

GComplex operator-(const double left, const GComplex& right)
{
	return GComplex(left - right.m_re, -right.m_im);
}

GComplex operator*(const double left, const GComplex& right)
{
	return GComplex(left * right.m_re, left * right.m_im);
}

GComplex operator/(const double left, const GComplex& right)
{
	const double denominator = right.m_re * right.m_re + right.m_im * right.m_im;
	return GComplex(left * right.m_re / denominator,
		-left * right.m_im / denominator);
}

bool operator==(const double left, const GComplex& right)
{
	return left == right.m_re && right.m_im == 0.;
}

bool operator!=(const double left, const GComplex& right)
{
	return left != right.m_re || right.m_im != 0.;
}
