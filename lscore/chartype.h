//////////////////////////////////////////////////////////////////////
// chartype.h

#define IsUpper(c) (c>='A' && c<='Z')
#define IsLower(c) (c>='a' && c<='z')
#define IsDigit(c) (c>='0' && c<='9')
#define IsAlpha(c) (IsUpper(c) || IsLower(c))
#define ToLower(c) (IsUpper(c) ? (unsigned)c | 0x20 : (unsigned)c)
#define ToUpper(c) (IsLower(c) ? (unsigned)c & ~0x20 : (unsigned)c)
