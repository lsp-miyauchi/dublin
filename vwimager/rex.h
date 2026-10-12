#pragma once
#ifndef __REX_H__
#define __REX_H__

#include "..\lscore\atl_special_rx.h"
#include <regex>



///////////////////////////////////////////////////////
// rex.h

class CRexMatchResult {
public:
  int m_index;
  int m_start;
  CString m_result;
  CRexMatchResult()
    : m_index(0)
    , m_start(0)
  {}
  CRexMatchResult(int index, LPCTSTR res, int start) {
    m_index = index;
    m_result = res;
    m_start = start;
  }
  CRexMatchResult& operator=(CRexMatchResult& src)
  {
    m_index = src.m_index;
    m_start = src.m_start;
    m_result = src.m_result;
    return *this;
  }
};

class CRex {
public:
  CAtlRegExp<> m_jre;
  CAtlREMatchContext<> m_matchContext;
  CRex() {}
  ~CRex() {}
  CAtlArray<CRexMatchResult>* match(LPCTSTR exp, LPCTSTR pszTarget)
  {
    REParseError status = m_jre.Parse(exp);
    if (REPARSE_ERROR_OK != status) {
      return NULL;
    }
    if (!m_jre.Match(pszTarget, &m_matchContext) || m_matchContext.m_uNumGroups == 0) {
      return NULL;
    }
    CAtlArray<CRexMatchResult>* arr = new CAtlArray<CRexMatchResult>();
    for (UINT i = 0; i < m_matchContext.m_uNumGroups; i++) {
      LPCTSTR st;
      LPCTSTR ed;
      m_matchContext.GetMatch(i, &st, &ed);
      CRexMatchResult res((int)i, st, PtrToInt(st) - PtrToInt(pszTarget));
      res.m_result.Truncate(PtrToInt(ed) - PtrToInt(st));
      arr->Add(res);
    }
    return arr;
  }
};

#endif // __REX_H__