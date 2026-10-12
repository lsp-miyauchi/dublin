/////////////////////////////////////////////////////////////////////////////////
// rex_supp.h

#pragma once

#define S_PARSE_FAILED			0x2
#define S_NO_MATCH				0x3
#define S_INVALID_REPLACE_EXP	0x4

//@ #include <atlrx.h>			// VS2008～では未サポート？
#include "..\lscore\atl_special_rx.h"

template <class CharTraits = CAtlRECharTraits>
class CRexReplace
{
private:
	CAtlRegExp<CharTraits> m_rex;

	// Helper function for s(), substitutes variables($N)
	HRESULT SubstituteVars(CString& src, CAtlREMatchContext<CharTraits>& rc){
		CAtlREMatchContext<CharTraits>::MatchGroup tempGroup;
		HRESULT hr = S_OK;
		UINT i, y;
		CString variableid, temp;
		for (i = 1; i <= rc.m_uNumGroups; i++){
			temp = _T("");
			rc.GetMatch(i - 1, &tempGroup);
			for (y = 0; y + tempGroup.szStart < tempGroup.szEnd; y++){
				temp = temp + tempGroup.szStart[y];
			}

			variableid.Format(_T("{$%d}"), i);
			hr = s(src, variableid, temp, true, false);
			if (hr != S_OK && hr !=  MAKE_HRESULT(1, FACILITY_ITF, S_NO_MATCH))
				return hr;
		}
		return hr;
	}

	// Helper for FindAllInstances
	CString getFoundat(int pos, CAtlREMatchContext<CharTraits> &rc){
		CString temp = _T("");
		for (int y = 0; y + rc.m_pMatches[pos].szStart < rc.m_pMatches[pos].szEnd; y++) {
			temp = temp + rc.m_pMatches[pos].szStart[y];
		}
		return temp;
	}

public:
	CRexReplace(){ }
	~CRexReplace(){ }

	typedef typename CharTraits::RECHARTYPE RECHAR;

	/*
		Regular expression substitution:
		baseStr:		Original String,
		oldexp:			Regular expression to look for inside 'baseStr'
		newexp:			Instances of 'oldexp' inside 'baseStr' will be replaced by this
		global:			Substitute all instances of 'oldexp' found or only the first one
		usingvars:		Use variables in replacements, you can specify variables using $(var#) (see example)
		casesensitive:	Case sensitive or insesitive search for 'oldexp'

		Example:
			baseStr			="Hello World! Goodbye World!"
			oldexp			="{.*?}World{.}"
			newexp			="<b>$1</b>World?"
			global			=true
			usingvars		=true
			casesensitive	=true
			the output would be:	"<b>Hello </b>World?<b> Goodbye </b>World?"
	*/
	HRESULT s(CString& baseStr, LPCTSTR oldexp, LPCTSTR newexp, const bool& global = false, const bool& usingvars = false, const bool& casesensitive = true)
	{
		HRESULT hr;
		CAtlREMatchContext<CharTraits> rc;
		CString substitutedexpr, strRightSide;
		if (m_rex.Parse(oldexp, casesensitive) == REPARSE_ERROR_OK) {
			if (m_rex.Match(baseStr, &rc)) {
				strRightSide = rc.m_Match.szEnd;
				if (usingvars) {
					substitutedexpr = newexp;
					SubstituteVars(substitutedexpr, rc);
				}
				else
					substitutedexpr = newexp;
				if (global) {
					hr = s(strRightSide, oldexp, newexp, true, usingvars, casesensitive);	//substitute recursivly
					if (hr != S_OK && hr != MAKE_HRESULT(1, FACILITY_ITF, S_NO_MATCH))
						return hr;
				}
				baseStr = baseStr.Left(baseStr.GetLength() - lstrlen(rc.m_Match.szStart)) + substitutedexpr + strRightSide;
				return S_OK;
			}
			else
			 	return MAKE_HRESULT(1,FACILITY_ITF,S_NO_MATCH);
		}
		else
			return MAKE_HRESULT(1,FACILITY_ITF,S_PARSE_FAILED); 
	}
	// Will return in 'allinstances' every occurrance it finds of 'oldexp' inside 'basestring'
	int FindAllInstances(CAtlArray<CString>& allinstances, CString basestring, CString oldexp)
	{
		int count = 0;
		m_rex.Parse(oldexp);
		CAtlREMatchContext<CharTraits> rc;
		while (m_rex.Match(basestring,&rc)) {
			count++;
			allinstances.Add(getFoundat(0,rc));
			basestring=rc.m_Match.szEnd;
		}
		return count;
	}

	// Will return the occurance number 'pos' of 'oldexp' in finds inside 'basestring'
	CString GetInstanceNum(int pos,CString basestring,CString oldexp)
	{
		m_rex.Parse(oldexp);
		CAtlREMatchContext<CharTraits> rc;
		for (int i = 1; i <= pos; i++) {
			if (m_rex.Match(basestring,&rc)) {
				if (i == pos)
					return getFoundat(0,rc);
				else
					basestring = rc.m_Match.szEnd;
			}
			else break;
		}
		return "";
	}

	CString Replace(LPCTSTR src, CString patt, CString replacer, bool global = false, bool casesensitive = true)
	{
		CString temp = patt;
		temp.Trim();

		bool usingvars = false;

		CString dst = src;
		int pos;
		LPCTSTR p = temp.GetString();
		if (*p == _T('/')) {
			pos = temp.ReverseFind(_T('/'));
			if (pos < 1)
				return dst;

			LPCTSTR q = p + pos + 1;
			while (*q) {
				switch(*q) {
				case _T('g'): global = true; break;
				case _T('i'): casesensitive = false; break;
				default: return dst;
				}
				q++;
			}
			temp = temp.Left(pos + 1);
			temp.Trim(_T('/'));
		}

		pos = replacer.Find(_T('$'));
		if (pos >= 0) {
			if (replacer[pos + 1] >= _T('0') && replacer[pos + 1] <= _T('9'))
				usingvars = true;
		}

		HRESULT hr = s(dst, temp, replacer, global, usingvars, casesensitive);
		if ((S_OK != hr) && (MAKE_HRESULT(1,FACILITY_ITF,S_NO_MATCH) != hr))
			dst = src;
		return dst;
	}
};

