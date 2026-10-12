/////////////////////////////////////////////////////////////////////////////
// exifinfo.h

#pragma once
#ifndef __EXIFINFO_H__
#define __EXIFINFO_H__

class CExifInformation
{
public:
	CString m_strModel;
	CString m_strLabel;
	CSize m_sizeImage;
	SYSTEMTIME m_dtDigitized;
	int m_nExposureProgram;
	float m_fFocalLength;
	float m_fFocalLength35;
	float m_fExposureTime;
	float m_fExposureBias;
	float m_fAperture;
	int m_nISOEquiv;
	BOOL m_bFlashFired;

	CExifInformation()
	{
		init();
	}

	CExifInformation(const CExifInformation& src)
	{
		CopyFrom(src);
	}

	void init()
	{
		m_sizeImage = CSize(0, 0);
		m_dtDigitized.wYear = 0;
		m_nExposureProgram = INT_MAX;
		m_fFocalLength = FLT_MAX;
		m_fFocalLength35 = FLT_MAX;
		m_fExposureTime = FLT_MAX;
		m_fExposureBias = FLT_MAX;
		m_fAperture = FLT_MAX;
		m_nISOEquiv = INT_MAX;
		m_bFlashFired = FALSE;
	}

	void Set(EXIFINFOSTRUCT* info)
	{
		m_strModel = info->ModelName;
		m_strLabel = info->pszLabelText;
		m_sizeImage = CSize(info->ImageWidth, info->ImageHeight);
		m_nExposureProgram = info->ExposureProgram;
		m_dtDigitized = info->DateTime;
		m_fFocalLength = info->FocalLength;
		m_fFocalLength35 = info->FocalLength35;
		m_fExposureTime = info->ExposureTime;
		m_fExposureBias = info->ExposureBias;
		m_fAperture = info->Aperture;
		m_nISOEquiv = info->ISOEquiv;
		m_bFlashFired = info->FlashFired;
	}

	CExifInformation& CopyFrom(const CExifInformation& src)
	{
		if (this != &src) {
			m_strModel = src.m_strModel;
			m_strLabel = src.m_strLabel;
			m_sizeImage = src.m_sizeImage;
			m_nExposureProgram = src.m_nExposureProgram;
			m_dtDigitized = src.m_dtDigitized;
			m_fFocalLength = src.m_fFocalLength;
			m_fFocalLength35 = src.m_fFocalLength35;
			m_fExposureTime = src.m_fExposureTime;
				m_fExposureBias = src.m_fExposureBias;
			m_fAperture = src.m_fAperture;
			m_nISOEquiv = src.m_nISOEquiv;
			m_bFlashFired = src.m_bFlashFired;
		}
		return (*this);
	}

	CExifInformation& operator =(const CExifInformation& src)
	{
		CopyFrom(src);
		return *this;
	}

	bool GetShellProperties(IShellFolder* psf, LPCITEMIDLIST pidl)
	{
		CComPtr<IShellItem2> psi2;
		psf->BindToObject(pidl, NULL, IID_PPV_ARGS(&psi2));
		if (!psi2)
			return false;

		CComHeapPtr<WCHAR> hpw;		// 
		// EXIF情報あり？
		if (psi2->GetString(PKEY_Photo_EXIFVersion, &hpw) != S_OK)
			return false;
		hpw.Free();

		init();
		// Camera model
		if (psi2->GetString(PKEY_Photo_CameraModel, &hpw) == S_OK) {
			m_strModel = hpw;
			hpw.Free();
		}
		// ExposureProgram
		ULONG ul;
		if (psi2->GetUInt32(PKEY_Photo_ExposureProgram, &ul) == S_OK) {
			m_nExposureProgram = ul;
		}
		// FocalLength
		PROPVARIANT pvr;
		if (psi2->GetProperty(PKEY_Photo_FocalLength, &pvr) == S_OK) {
			PropVariantChangeType(&pvr, pvr, 0, VT_R4);
			if (pvr.fltVal != 0.0F)
				m_fFocalLength = pvr.fltVal;
		}
		// FocalLength35
		if (psi2->GetProperty(PKEY_Photo_FocalLengthInFilm, &pvr) == S_OK) {
			PropVariantChangeType(&pvr, pvr, 0, VT_R4);
			if (pvr.fltVal != 0.0F)
				m_fFocalLength35 = pvr.fltVal;
		}
		// ExposureTime
		if (psi2->GetProperty(PKEY_Photo_ExposureTime, &pvr) == S_OK) {
			PropVariantChangeType(&pvr, pvr, 0, VT_R4);
			if (pvr.fltVal != 0.0F)
				m_fExposureTime = pvr.fltVal;
		}
		// ExposureBias
		if (psi2->GetProperty(PKEY_Photo_ExposureBias, &pvr) == S_OK) {
			PropVariantChangeType(&pvr, pvr, 0, VT_R4);
			m_fExposureBias = pvr.fltVal;
		}
		// Aperture
		if (psi2->GetProperty(PKEY_Photo_FNumber, &pvr) == S_OK) {
			PropVariantChangeType(&pvr, pvr, 0, VT_R4);
			if (pvr.fltVal != 0.0F)
				m_fAperture = pvr.fltVal;
		}
		if (m_fAperture == FLT_MAX) {
			if (psi2->GetProperty(PKEY_Photo_Aperture, &pvr) == S_OK) {
				PropVariantChangeType(&pvr, pvr, 0, VT_R4);
				if (pvr.fltVal != 0.0F)
					m_fAperture = pvr.fltVal;
			}
		}
		// ISO
		if (psi2->GetProperty(PKEY_Photo_ISOSpeed, &pvr) == S_OK) {
			m_nISOEquiv = pvr.uiVal;
		}
		// Flash
		if (psi2->GetProperty(PKEY_Photo_Flash, &pvr) == S_OK) {
			m_bFlashFired = (pvr.bVal != PHOTO_FLASH_NONE);
		}
		return true;
	}

	__inline LPCTSTR GetLabel()
	{
		if (!m_strLabel.IsEmpty())
			return m_strLabel;
		return NULL;
	}

	LPTSTR FormatExposureProgram(LPTSTR psz)
	{
		LPTSTR p = psz;
		if (m_nExposureProgram != INT_MAX) {
			switch (m_nExposureProgram) {
			case 1: *p++ = _T('M'); break;
			case 2: *p++ = _T('P'); break;
			case 3: *p++ = _T('A'); break;
			case 4: *p++ = _T('S'); break;
			case 5: *p++ = _T('C'); *p++ = _T('r'); break;
			case 6: *p++ = _T('A'); *p++ = _T('c'); break;
			case 7: *p++ = _T('P'); *p++ = _T('o'); break;
			case 8: *p++ = _T('L'); *p++ = _T('a'); break;
			}
		}
		*p = 0;
		return psz;
	}

	LPTSTR FormatExposureTime(LPTSTR psz, bool bRev = false)
	{
		*psz = 0;
		if (m_fExposureTime != FLT_MAX) {
			// ex. 1/250, 1, 0.4, …
			if (bRev) {
				if (m_fExposureTime < 0.3F)
					wsprintf(psz, _T("1/%d"), (int)(0.5F + 1.0F / m_fExposureTime));
				else {
					if ((int)floor((m_fExposureTime + 0.05F) * 10.0F) % 10 == 0)
						_format_int(psz, (int)(m_fExposureTime + 0.05F));
					else
						_format_float(psz, m_fExposureTime);
					lstrcat(psz, _T("\""));
				}
			}
			else {
				// ex. 250, 1"0, 0"4, …
				if (m_fExposureTime < 0.3F)
					wsprintf(psz, _T("%d"), (int)(0.5F + 1.0F / m_fExposureTime));
				else {
					float f = m_fExposureTime + 0.05F;
					wsprintf(psz, _T("%d\"%d"), (int)f, (int)floorf(f * 10) % 10);
				}
			}
		}
		return psz;
	}

	LPTSTR FormatAperture(LPTSTR psz, bool bRev = false)
	{
		*psz = 0;
		if (m_fAperture != FLT_MAX && m_fAperture > 0.0F) {
			LPTSTR p = psz;
			*p++ = _T('F');
			if (bRev)
				*p++ = _T('/');
			if (m_fAperture >= 10.0F)
				_format_int(p, (int)(m_fAperture + 0.5F));
			else {
				if ((int)floor((m_fAperture + 0.05F) * 10.0F) % 10 == 0)
					_format_int(p, (int)(m_fAperture + 0.05F));
				else
					_format_float(p, m_fAperture);
			}
		}
		return psz;
	}

	LPTSTR FormatExposureBias(LPTSTR psz)
	{
		*psz = 0;
		if (m_fExposureBias != FLT_MAX && m_fExposureBias != 0.0F)
			_format_ev_comp(psz, m_fExposureBias);
		return psz;
	}

	LPTSTR FormatFocalLength(LPTSTR psz)
	{
		*psz = 0;
		if (m_fFocalLength != FLT_MAX) {
			_format_float_int(psz, m_fFocalLength);
			lstrcat(psz, _T("mm"));
		}
		return psz;
	}

	LPTSTR FormatFocalLength35(LPTSTR psz)
	{
		*psz = 0;
		if (m_fFocalLength35 != FLT_MAX) {
			_format_float_int(psz, m_fFocalLength35);
			lstrcat(psz, _T("mm"));
		}
		return psz;
	}

	LPTSTR FormatDigitizedTime(LPTSTR psz, int cchMax)
	{
		*psz = 0;
		if (m_dtDigitized.wYear != 0) {
			GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &m_dtDigitized, NULL, psz, cchMax);
			int len = lstrlen(psz);
			psz[len++] = _T(' ');
			GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, &m_dtDigitized, NULL, psz + len, cchMax - len);
		}
		return psz;
	}

	LPTSTR FormatISOEquiv(LPTSTR psz)
	{
		*psz = 0;
		if (m_nISOEquiv != INT_MAX) {
			if (m_nISOEquiv != 0)
				wsprintf(psz, _T("ISO%d"), m_nISOEquiv);
			else
				lstrcpy(psz, _T("ISOオート"));
		}
		return psz;
	}

	LPTSTR FormatImageSize(LPTSTR psz)
	{
		*psz = 0;
		if (m_sizeImage.cx != INT_MAX && m_sizeImage.cy != INT_MAX &&
			m_sizeImage.cx > 0 && m_sizeImage.cy > 0) {
			wsprintf(psz, _T("%d x %d"), m_sizeImage.cx, m_sizeImage.cy);
		}
		return psz;
	}

	LPTSTR FormatOutlineInfo(LPTSTR psz)
	{
		LPTSTR p = psz;
		FormatExposureProgram(p);
		p += lstrlen(p);
		*p++ = _T(' ');
		FormatExposureTime(p);
		p += lstrlen(p);
		*p++ = _T(' ');
		FormatAperture(p);
		p += lstrlen(p);
		*p++ = _T(' ');
		FormatExposureBias(p);
		if (*p) {
			p += lstrlen(p);
			*p++ = _T(' ');
		}
		FormatFocalLength(p);
		p += lstrlen(p);
		*p++ = _T(' ');
		FormatISOEquiv(p);

		return psz;
	}

	__inline BOOL IsFlashFired() { return m_bFlashFired; }

private:
	LPTSTR _format_int(LPTSTR psz, int n)
	{
		wsprintf(psz, _T("%d"), n); return psz;
	}

	LPTSTR _format_float(LPTSTR psz, float f)
	{
		if (f != 0.0F) f += 0.05F;
		wsprintf(psz, _T("%d.%d"), (int)f, (int)floorf(fabsf(f) * 10) % 10); return psz;
	}

	LPTSTR _format_float_int(LPTSTR psz, float f)
	{
		if (fmod(f, 1.0F) > 0.05F)
			_format_float(psz, f);
		else
			_format_int(psz, (int)f);
		return psz;
	}

	LPTSTR _format_float_comp(LPTSTR psz, float f)
	{
		if (f == 0.0F) return lstrcpy(psz, _T("±0"));

		LPTSTR p = psz;
		*p++ = (f < 0.0F) ? '-' : '+';
		_format_float(p, fabsf(f));
		return psz;
	}

	LPTSTR _format_ev_comp(LPTSTR psz, float f)
	{
		_format_float_comp(psz, f);
		lstrcat(psz, _T("EV"));
		return psz;
	}
};


#endif		// __EXIFINFO_H__
