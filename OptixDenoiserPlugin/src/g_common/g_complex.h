#ifndef G_COMPLEX_H
#define G_COMPLEX_H

#include <math.h>

class GComplex
{
	public:

		static GComplex cis( double theta );

		double m_re;
		double m_im;

		GComplex();
		GComplex(double re, double im);
		GComplex(double val);

		GComplex conj() const;
		double abs2() const;
		double abs() const;
		double ang() const;

		GComplex& operator=(const double val);
		GComplex& operator=(const GComplex &val);
		GComplex operator+(const GComplex& other) const;
		GComplex operator-(const GComplex& other) const;
		GComplex operator*(const GComplex& other) const;
		GComplex operator/(const GComplex& other) const;
		GComplex& operator+=(const GComplex& other);
		GComplex& operator-=(const GComplex& other);
		GComplex& operator*=(const GComplex& other);
		GComplex& operator/=(const GComplex& other);
		GComplex operator+(const double val) const;
		GComplex operator-(const double val) const;
		GComplex operator*(const double val) const;
		GComplex operator/(const double val) const;
		GComplex& operator+=(const double val);
		GComplex& operator-=(const double val);
		GComplex& operator*=(const double val);
		GComplex& operator/=(const double val);
		bool operator==(const double val) const;
		bool operator!=(const double val) const;
		bool operator==(const GComplex &other) const;
		bool operator!=(const GComplex &other) const;

		GComplex& operator++();
		GComplex& operator--();
		GComplex operator++(int);
		GComplex operator--(int);

		friend GComplex operator+(const double left, const GComplex& right);
		friend GComplex operator-(const double left, const GComplex& right);
		friend GComplex operator*(const double left, const GComplex& right);
		friend GComplex operator/(const double left, const GComplex& right);
		friend bool    operator==(const double left, const GComplex& right);
		friend bool    operator!=(const double left, const GComplex& right);
};

#endif