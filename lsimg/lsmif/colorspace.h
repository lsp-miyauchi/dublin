////////////////////////////////////////////////////////////////////////
// colorspace.h

class rgb2any
{
public:
	rgb2any(int color_mode);
	~rgb2any();
	void xfer(BYTE* p);

	double* x;
	double* y;
	double* z;
	BYTE* range_table;
	BYTE* range_limit;
	int tx;
	int ty;
	int tz;
};

class any2rgb
{
public:
	any2rgb(int color_mode);
	~any2rgb();
	void xfer(BYTE* p);

	double* red;
	double* green;
	double* blue;
	BYTE* range_table;
	BYTE* range_limit;
};

