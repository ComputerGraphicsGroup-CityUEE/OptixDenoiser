#ifndef DCTF2CLASS_H
#define DCTF2CLASS_H


class dctf2class  
{
public:
	dctf2class(int dim);
	virtual ~dctf2class();

    void ForwardDCT(float *input,float *output);
	void InverseDCT(float *input,float *output);

    int	m_dim;
	int	m_imgsize;
	float **m_C;
	float **m_Ct;
	float **m_Temp;

};

#endif
