// hpp_xyp.h: interface for the hpp_xyp class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HPP_XYP_H__D0E1192A_450F_4B4E_9A97_23B067F133F6__INCLUDED_)
#define AFX_HPP_XYP_H__D0E1192A_450F_4B4E_9A97_23B067F133F6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class hpp_xyp  
{
public:
	hpp_xyp();
	virtual ~hpp_xyp();
public:
	void hp2xy(unsigned int hpindex, unsigned int* xptr, unsigned int* yptr);
	void xy2hp(unsigned int x, unsigned int y, unsigned int* hpindexptr);
};

#endif // !defined(AFX_HPP_XYP_H__D0E1192A_450F_4B4E_9A97_23B067F133F6__INCLUDED_)
