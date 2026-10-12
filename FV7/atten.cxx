////////////////////////////////
// atten.cxx

static void __attension_app()
{
	if (!glFVValid) {
		if (!(glFVAttension % 7)) {
			APPABOUTINFO ai;
			memset(&ai, 0, sizeof(ai));
//@			lccShowAppAbout(fvApp.wndMain, &ai);
		}
		glFVAttension++;
	}
}
