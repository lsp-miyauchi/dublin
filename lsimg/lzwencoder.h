////////////////////////////////////////////////////////////////////////
//

#include "memstream.h"

#define MAX_LZW_BITS			12
#define LZW_TABLE_SIZE			((short)1 << MAX_LZW_BITS)
#define HSIZE					5003		// hash table size for 80% occupancy
#define MAXCODE(a)				(((short)1 << (a)) - 1)
#define HASH_ENTRY(a,b) 		((((long)(a)) << 8) | (b))

////////////////////////////////////////////////////////////////////////
//

class CLzwEncoder
{
public:
	CLzwEncoder(size_t size);
	~CLzwEncoder();

	memstream* m_pm;

	// State for packing variable-width codes into a bitstream
	int m_nbits;						// current number of bits/code
	short m_maxcode;					// maximum code, given m_nbits
	int m_init_bits;					// initial m_nbits ... restored after clear
	long m_cur_accum;					// holds bits not yet output
	int m_cur_bits;						// # of bits in m_cur_accum

	// LZW string construction
	short m_waiting_code;				// symbol not yet output; may be extendable
	BOOL m_first_byte;					// if TRUE, m_waiting_code is not valid

	// State for LZW code assignment
	short m_code_clear;					// clear code (doesn't change)
	short m_code_eof;					// EOF code (ditto)
	short m_code_free;					// first not-yet-used symbol code

	// LZW hash table
	short* m_hash_code;					// => hash table of symbol codes
	long* m_hash_value;					// => hash table of symbol values

	// GIF data packet construction buffer
	int bytesinpkt;						// # of bytes in current packet
	char packetbuf[256];				// workspace for accumulating packet
};

CLzwEncoder::CLzwEncoder(size_t size)
{
	m_pm = new memstream(size);
	m_hash_code = new short[HSIZE];
	m_hash_value = new long[HSIZE];
}

CLzwEncoder::~CLzwEncoder()
{
	if (m_pm) delete m_pm;
	if (m_hash_value) delete [] m_hash_value;
	if (m_hash_code) delete [] m_hash_code;
}

void CLzwEncoder::flush_packet()
{
	if (bytesinpkt > 0) {
		// never write zero-length packet
		packetbuf[0] = (char)bytesinpkt++;
		if (m_pm->write(packetbuf, bytesinpkt) != (size_t)bytesinpkt)
			return;
		bytesinpkt = 0;
	}
}

////////////////////////////////////////////////////////////////////////
// Add a character to current packet; flush to disk if necessary

void CLzwEncoder::char_out(int c)
{
	packetbuf[++bytesinpkt] = (char)(c);
	if (bytesinpkt >= 255) flush_packet();
}

////////////////////////////////////////////////////////////////////////
// Emit a code of m_nbits bits
// Uses m_cur_accum and m_cur_bits to reblock into 8-bit bytes

void CLzwEncoder::output(short code)
{
	m_cur_accum |= ((long) code) << m_cur_bits;
	m_cur_bits += m_nbits;

	while (m_cur_bits >= 8) {
		char_out(m_cur_accum & 0xFF);
		m_cur_accum >>= 8;
		m_cur_bits -= 8;
	}

	// If the next entry is going to be too big for the code size,
	// then increase it, if possible.  We do this here to ensure
	// that it's done in sync with the decoder's codesize increases.
	if (m_code_free > m_maxcode) {
		m_nbits++;
		if (m_nbits == MAX_LZW_BITS)
			m_maxcode = LZW_TABLE_SIZE;		// m_code_free will never exceed this
		else
			m_maxcode = MAXCODE(m_nbits);
	}
}


////////////////////////////////////////////////////////////////////////
// Fill the hash table with empty entries

void CLzwEncoder::clear_hash()
{
	// It's sufficient to zero m_hash_code[]
	memset(m_hash_code, 0, HSIZE * sizeof(short));
}


////////////////////////////////////////////////////////////////////////
// Reset compressor and issue a Clear code

void CLzwEncoder::clear_block()
{
	clear_hash();							// delete all the symbols
	m_code_free = m_code_clear + 2;
	output(pgs, m_code_clear);					// inform decoder
	m_nbits = m_init_bits;						// reset code size
	m_maxcode = MAXCODE(m_nbits);
}

////////////////////////////////////////////////////////////////////////
// Initialize LZW compressor

void CLzwEncoder::compress_init(int i_bits)
{
	// init all the state variables
	m_nbits = m_init_bits = i_bits;
	m_maxcode = MAXCODE(m_nbits);
	m_code_clear = ((short) 1 << (i_bits - 1));
	m_code_eof = m_code_clear + 1;
	m_code_free = m_code_clear + 2;
	m_first_byte = TRUE;				// no waiting symbol yet

	// init output buffering vars
	bytesinpkt = 0;
	m_cur_accum = 0;
	m_cur_bits = 0;

	// clear hash table
	clear_hash();

	// GIF specifies an initial Clear code
	output(m_code_clear);
}

////////////////////////////////////////////////////////////////////////
// Accept and compress one 8-bit byte

void CLzwEncoder::compress_byte(int c)
{
	register short i;
	register short disp;
	register long probe_value;

	if (m_first_byte) {
		// need to initialize m_waiting_code
		m_waiting_code = c;
		m_first_byte = FALSE;
		return;
	}

	// Probe hash table to see if a symbol exists for
	// m_waiting_code followed by c.
	// If so, replace m_waiting_code by that symbol and return.
	i = ((short) c << (MAX_LZW_BITS - 8)) + m_waiting_code;

	// i is less than twice 2**MAX_LZW_BITS, therefore less than twice HSIZE
	if (i >= HSIZE)
		i -= HSIZE;

	probe_value = HASH_ENTRY(m_waiting_code, c);

	if (m_hash_code[i] != 0) {
		// is first probed slot empty?
		if (m_hash_value[i] == probe_value) {
			m_waiting_code = m_hash_code[i];
			return;
		}

		// secondary hash (after G. Knott)
		if (i == 0)
			disp = 1;
		else
			disp = HSIZE - i;

		for (;;) {
			i -= disp;
			if (i < 0)
				i += HSIZE;
			if (m_hash_code[i] == 0)
				break;						// hit empty slot
			if (m_hash_value[i] == probe_value) {
				m_waiting_code = m_hash_code[i];
				return;
			}
		}
	}

	// here when hashtable[i] is an empty slot; desired symbol not in table
	output(m_waiting_code);
	if (m_code_free < LZW_TABLE_SIZE) {
		// add symbol to hashtable
		m_hash_code[i] = m_code_free++;
		m_hash_value[i] = probe_value;
	}
	else
		clear_block();
	m_waiting_code = c;
}

////////////////////////////////////////////////////////////////////////
// Clean up at end

void CLzwEncoder::compress_term()
{
	// Flush out the buffered code
	if (!m_first_byte)
		output(m_waiting_code);

	// Send an EOF code
	output(m_code_eof);

	// Flush the bit-packing buffer
	if (m_cur_bits > 0)
		char_out(m_cur_accum & 0xFF);

	// Flush the packet buffer
	flush_packet();
}

////////////////////////////////////////////////////////////////////////
// Write some pixel data.
// In this module rows_supplied will always be 1.

void LzwEncode(BITMAPINFO* pbi, BYTE* bits)
{
	BITMAPINFOHEADER* pbh = (BITMAPINFOHEADER*)pbi;

	// Initialize for LZW compression of image data
	CLzwEncoder encode(pbh->biWidth * pbh->biHeight * 4 + 128);
	enc.compress_init(8 + 1);

	for (long y = 0; y < pbh->biHeight; y++) {
		long x;
		for (x = 0; x < pbh->biWidth; x++) pe->compress_byte(bits[x * 4]);
		for (x = 0; x < pbh->biWidth; x++) pe->compress_byte(bits[x * 4 + 1]);
		for (x = 0; x < pbh->biWidth; x++) pe->compress_byte(bits[x * 4 + 2]);
		for (x = 0; x < pbh->biWidth; x++) pe->compress_byte(bits[x * 4 + 3]);
		bits += 4 * pbh->biWidth;
	}

	// Flush LZW mechanism
	encode.compress_term();
}
