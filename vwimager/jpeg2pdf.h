////////////////////////////////////////////////////////////////////////
// jpeg2pdf.h

#ifndef _JPEG2PDF_H_
#define _JPEG2PDF_H_

/* Defined for Compiling on Windows. Might need to be changed for other compiler */
typedef unsigned char BYTE;
typedef unsigned int UINT32;
typedef int STATUS;

#define MAX_PDF_PAGES			256
#define PDF_TOP_MARGIN			(0.0 * PDF_DOT_PER_INCH)
#define PDF_LEFT_MARGIN			(0.0 * PDF_DOT_PER_INCH)
#define PDF_DOT_PER_INCH		72
#define MAX_PDF_PREFORMAT_SIZE 	256		/* Format Before each image, Usually less than 142 Bytes */
#define MAX_PDF_PSTFORMAT_SIZE 	512		/* Format After  each image, Usually less than 400 Bytes */

struct jpeg2pdf_Node_struct {
	struct jpeg2pdf_Node_struct *pNext;
	size_t JpegSize;
	int JpegW, JpegH;
	int PageObj;
	char  preFormat[MAX_PDF_PREFORMAT_SIZE];
	char  pstFormat[MAX_PDF_PSTFORMAT_SIZE];
	WCHAR jpegFile[MAX_PATH];
};

typedef struct jpeg2pdf_Node_struct JPEG2PDF_NODE, *PJPEG2PDF_NODE;

#define XREF_ENTRY_LEN			20		/* Each XREF entry is 20 Bytes */
#define OBJNUM_EXTRA			3		/* First Free Object; Kids Object; Catalog Object */
#define OBJNUM_PER_IMAGE		5
#define MAX_KIDS_STRLEN			10		/* Kids Str Looks Like: "X 0 R ", X = OBJNUM_EXTRA + OBJNUM_PER_IMAGE * (pageNum - 1) */

#define MAX_PDF_XREF			(MAX_PDF_PAGES * OBJNUM_PER_IMAGE + OBJNUM_EXTRA)
#define MAX_PDF_HEADER			64		/* PDF Header, Usually less than 40 Bytes */
#define MAX_PDF_TAILER			( ( MAX_PDF_PAGES * (MAX_KIDS_STRLEN + (OBJNUM_PER_IMAGE * XREF_ENTRY_LEN)) ) + (OBJNUM_EXTRA * XREF_ENTRY_LEN) + 256 )
#define INDEX_USE_PPDF			(-1)


/*
	CJpeg2Pdf jp;
	jp.BeginDocument(8.5, 11.0);	// Letter is 8.5x11 inch
	while (numJpegFiles) {
		jp.InsertJPEG(jpeg_file, jpeg_file_size);
	}
	UINT32 total = jp.EndDocument();
	void* buffer = malloc(total);
	jp.GetFinalDocument(buffer, &total);

	WriteToFile(file, buffer, total);
*/

class CJpeg2Pdf
{
public:
	/* Link List Stuff */
	PJPEG2PDF_NODE m_pFirstNode;
	PJPEG2PDF_NODE m_pLastNode;
	int m_numNodes;
	/* PDF Stuff */
	char* m_baseBuffer;
	char* m_pdfHeader;
	char* m_pdfTailer;
	char* m_pdfXREF[MAX_PDF_XREF];
	int m_width;
	int m_height;
	int m_numObjects;
	int m_numImages;
	bool m_customSizing;
	size_t m_curOffset;

	CJpeg2Pdf()
		: m_pFirstNode(NULL)
		, m_pLastNode(NULL)
		, m_numNodes(0)
		, m_width(0)
		, m_height(0)
		, m_numObjects(0)
		, m_curOffset(0)
		, m_numImages(0)
		, m_customSizing(true)
	{
		size_t size = MAX_PDF_HEADER + MAX_PDF_TAILER + (MAX_PDF_XREF * (XREF_ENTRY_LEN + 1));
		m_baseBuffer = new char[size];
		memset(m_baseBuffer, 0, size * sizeof(char));
		m_pdfHeader = m_baseBuffer;
		m_pdfTailer = m_baseBuffer + MAX_PDF_HEADER;
		m_pdfXREF[0] = m_pdfTailer + MAX_PDF_TAILER;
		for (int i = 1; i < MAX_PDF_XREF; i++)
			m_pdfXREF[i] = m_pdfXREF[i - 1] + (XREF_ENTRY_LEN + 1);
	}

	~CJpeg2Pdf()
	{
		PJPEG2PDF_NODE pNode = m_pFirstNode;
		while (pNode != NULL) {
			PJPEG2PDF_NODE pFreeCurrent = pNode;
			pNode = pNode->pNext;
			free(pFreeCurrent);
		}
		delete [] m_baseBuffer;
	}

	void SetXREF(int index, size_t offset, char c)
	{
		if (INDEX_USE_PPDF == index)
			index = m_numObjects;
		if ('f' == c) 
			sprintf_s(m_pdfXREF[index], XREF_ENTRY_LEN + 1, "%010d 65535 f\r\n", (int)offset);
		else
			sprintf_s(m_pdfXREF[index], XREF_ENTRY_LEN + 1, "%010d 00000 %c\r\n", (int)offset, c);
	}

	void BeginDocument(double pdfW, double pdfH)
	{
		m_width = (int)(pdfW * PDF_DOT_PER_INCH);
		m_height = (int)(pdfH * PDF_DOT_PER_INCH);
		SetXREF(0, 0, 'f');
		m_curOffset = sprintf_s(m_pdfHeader, MAX_PDF_HEADER, "%%PDF-1.3\r\n%%%cJ2PDF%c\r\n", 0xFF, 0xFF);
		m_numImages = 0;
		m_numObjects = 2;		/* 0 & 1 was reserved for xref & document Root */
	}

	bool AddJpeg(LPCWSTR jpegFile, int imgW, int imgH, size_t fileSize, BYTE isColor)
	{
		bool result = false;
		if (m_numNodes >= MAX_PDF_PAGES) {
			// 256まで
			return result;
		}

		PJPEG2PDF_NODE pNode = (PJPEG2PDF_NODE)malloc(sizeof(JPEG2PDF_NODE));
		if (pNode) {
			pNode->JpegW = imgW;
			pNode->JpegH = imgH;
			pNode->JpegSize = fileSize;
			lstrcpyW(pNode->jpegFile, jpegFile);
			pNode->pNext = NULL;

			/* Image Object */
			SetXREF(INDEX_USE_PPDF, m_curOffset, 'n');
			int currentImageObject = m_numObjects;

			m_curOffset += sprintf_s(pNode->preFormat, MAX_PDF_PREFORMAT_SIZE,
					"\r\n%d 0 obj\r\n<</Type/XObject/Subtype/Image/Filter/DCTDecode/BitsPerComponent 8/ColorSpace/%s/Width %d/Height %d/Length %d>>\r\nstream\r\n",
					m_numObjects, ((isColor)? "DeviceRGB" : "DeviceGray"), pNode->JpegW, pNode->JpegH, (int)pNode->JpegSize);
			m_curOffset += pNode->JpegSize;

			CStringA pst = "\r\nendstream\r\nendobj\r\n";
			m_numObjects++;

			/* Page Object */
			int pageWidth = m_width, pageHeight = m_height;
			// 縦横比がページとイメージで異なればページの縦横を入れ替え
			if ((pageHeight > pageWidth && pNode->JpegH < pNode->JpegW) || (pageHeight < pageWidth && pNode->JpegH > pNode->JpegW)) {
				int tmp = pageWidth; pageWidth = pageHeight; pageHeight = tmp;
			}
			double imageRatio = (double)pNode->JpegH / (double)pNode->JpegW;
			double pageRatio = (double)pageHeight / (double)pageWidth;
			if (m_customSizing) {
				if (pageRatio < imageRatio)
					pageWidth = (int)((double)(pNode->JpegW * pageHeight) / (double)pNode->JpegH);
				else if (pageRatio > imageRatio)
					pageHeight = (int)((double)(pNode->JpegH * pageWidth) / (double)pNode->JpegW);
			}
			SetXREF(INDEX_USE_PPDF, m_curOffset + pst.GetLength(), 'n');
			pNode->PageObj = m_numObjects;
			pst.AppendFormat("%d 0 obj\r\n<</Type/Page/Parent 1 0 R/MediaBox[0 0 %d %d]/Contents %d 0 R/Resources %d 0 R>>\r\nendobj\r\n",
						m_numObjects, pageWidth, pageHeight, m_numObjects + 1, m_numObjects + 3);
			m_numObjects++;

			/* Contents Object in Page Object */
			SetXREF(INDEX_USE_PPDF, m_curOffset + pst.GetLength(), 'n');

			// 左右上下のマージンをイメージの縦横比率、ページの縦横比率から求める
			double marginHorz = 0.0, marginVert = 0.0;
			if (!m_customSizing) {
				if (pageRatio < imageRatio)
					marginHorz = ((double)pageWidth - (double)(pNode->JpegW * pageHeight) / (double)pNode->JpegH) / 2.0;
				else if (pageRatio > imageRatio)
					marginVert = ((double)pageHeight - (double)(pNode->JpegH * pageWidth) / (double)pNode->JpegW) / 2.0;
			}
			CStringA tmp;
			tmp.Format("q\r\n1 0 0 1 %.2f %.2f cm\r\n%.2f 0 0 %.2f 0 0 cm\r\n/I%d Do\r\nQ\r\n",
					marginHorz, marginVert, pageWidth - 2 * marginHorz, pageHeight - 2 * marginVert, m_numImages);
			pst.AppendFormat("%d 0 obj\r\n<</Length %d 0 R>>stream\r\n%sendstream\r\nendobj\r\n", m_numObjects, m_numObjects+1, tmp.GetString());
			m_numObjects++;

			/* Length Object in Contents Object */
			SetXREF(INDEX_USE_PPDF, m_curOffset + pst.GetLength(), 'n');
			pst.AppendFormat("%d 0 obj\r\n%ld\r\nendobj\r\n", m_numObjects, tmp.GetLength());
			m_numObjects++;

			/* Resources Object in Page Object */
			SetXREF(INDEX_USE_PPDF, m_curOffset + pst.GetLength(), 'n');
			pst.AppendFormat("%d 0 obj\r\n<</ProcSet[/PDF/%s]/XObject<</I%d %d 0 R>>>>\r\nendobj\r\n",
					m_numObjects, ((isColor)? "ImageC" : "ImageB"), m_numImages, currentImageObject);
			m_numObjects++;
			m_numImages++;
			m_curOffset += pst.GetLength();

			strcpy_s(pNode->pstFormat, MAX_PDF_PSTFORMAT_SIZE, pst.GetString());

			/* Update the Link List */
			m_numNodes++;
			if (1 == m_numNodes) {
				m_pFirstNode = pNode;
			} else {
				m_pLastNode->pNext = pNode;
			}
			m_pLastNode = pNode;
			result = true;
		} /* pNode is valid */
		return result;
	}

	size_t EndDocument()
	{
		/* Catalog Object. This is the Last Object */
		SetXREF(INDEX_USE_PPDF, m_curOffset, 'n');

		CStringA tailer;
		tailer.Format("%d 0 obj\r\n<</Type/Catalog/Pages 1 0 R>>\r\nendobj\r\n", m_numObjects);

		/* Pages Object. It's always the Object 1 */
		SetXREF(1, m_curOffset + tailer.GetLength(), 'n');

		CStringA kids;
		PJPEG2PDF_NODE pNode = m_pFirstNode;
		while (pNode != NULL) {
			kids.AppendFormat("%d 0 R ", pNode->PageObj);
			pNode = pNode->pNext;
		}
		kids.TrimRight();

		tailer.AppendFormat("1 0 obj\r\n<</Type /Pages /Kids [%s] /Count %d>>\r\nendobj\r\n", kids.GetString(), m_numNodes);

		/* The xref & the rest of the tail */
		size_t xrefOffSet = m_curOffset + tailer.GetLength();
		tailer.AppendFormat("xref\r\n0 %d\r\n", m_numObjects+1);

		for (int i = 0; i <= m_numObjects; i++) {
			tailer += m_pdfXREF[i];
		}

		tailer.AppendFormat("trailer\r\n<</Root %d 0 R /Size %d>>\r\n", m_numObjects, m_numObjects+1);
		tailer.AppendFormat("startxref\r\n%d\r\n%%%%EOF\r\n", xrefOffSet);
		m_curOffset += tailer.GetLength();

		strcpy_s(m_pdfTailer, MAX_PDF_TAILER, tailer.GetString());

		size_t hlen = strlen(m_pdfHeader);
		size_t tlen = strlen(m_pdfTailer);
		return (hlen && tlen && (m_curOffset > (hlen + tlen))) ? m_curOffset : 0;
	}

	bool GetFinalDocument(HANDLE pdf)
	{
		SetFilePointer(pdf, 0, NULL, FILE_BEGIN);
		DWORD written;
		size_t len = strlen(m_pdfHeader);
		WriteFile(pdf, m_pdfHeader, (DWORD)len, &written, NULL);

		PJPEG2PDF_NODE pNode = m_pFirstNode;
		while (pNode != NULL) {
			len = strlen(pNode->preFormat);
			WriteFile(pdf, pNode->preFormat, (DWORD)len, &written, NULL);

			void* buffer = readjpeg(pNode->jpegFile, len);
			if (buffer) {
				len = pNode->JpegSize;			// 違っていたら誤動作になるなぁ
				WriteFile(pdf, buffer, (DWORD)len, &written, NULL);
				free(buffer);

				len = strlen(pNode->pstFormat);
				WriteFile(pdf, pNode->pstFormat, (DWORD)len, &written, NULL);
				pNode = pNode->pNext;
			}
			else
				return false;
		}

		len = strlen(m_pdfTailer);
		WriteFile(pdf, m_pdfTailer, (DWORD)len, &written, NULL);

		return true;
	}

	bool InsertJPEG(LPCWSTR fileName)
	{
		bool ok = false;
		size_t size;
		BYTE* buffer = (BYTE*)readjpeg(fileName, size);
		if (buffer) {
			int width, height, colors;
			if (jpeginfo(buffer, size, width, height, colors)) {
				/* Add JPEG File into PDF */
				ok = AddJpeg(fileName, width, height, size, (3 == colors));
			}
			free(buffer);
		}
		return ok;
	}

private:
	void* readjpeg(LPCWSTR fileName, size_t& dataSize)
	{
		void* buffer = NULL;
		HANDLE hf = CreateFileW(fileName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
		if (hf != INVALID_HANDLE_VALUE) {
			DWORD size = ::GetFileSize(hf, NULL);
			buffer = malloc(size);
			if (buffer) {
				DWORD num;
				if (ReadFile(hf, buffer, size, &num, NULL))
					dataSize = size;
				else {
					free(buffer);
					buffer = NULL;
				}
			}
			CloseHandle(hf);
		}
		return buffer;
	}

	bool jpeginfo(BYTE* data, size_t data_size, int& width, int& height, int& colors)
	{
		bool bOK = false;
		BYTE* endPtr = data + data_size;
		data += 2;
		BYTE cb[4];
		while (data + 4 < endPtr) {
			// マーカーを取得
			memcpy(cb, data, 4); data += 4;
			if (cb[0] != 0xff) break;

			// ハフマン式コードフレーム？
			if (0xc0 <= cb[1] && cb[1] <= 0xc3) {
				BYTE info[6];
				memcpy(info, data, 6);
				height = (info[1] << 8) + info[2];		// イメージ内の行数
				width  = (info[3] << 8) + info[4];		// イメージ内の桁数
				colors = info[5];
				return true;
			}
			data += (cb[2] << 8) + cb[3] - 2;
		}
		return false;
	}
};


#endif /* _JPEG2PDF_H_ */
