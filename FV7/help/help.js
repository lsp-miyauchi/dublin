function NoteIn(){
	var a=NoteIn.arguments;
	document.write('<table border=0 cellspacing=0 cellpadding=2>');
	document.write('<tr><td>');
	if (a.length > 0) {
		document.write('<img border=0 src="');
		document.write(a[0]);
		document.write('">');
	}
	document.write('</td><td style="vertical-align: top;">');
}

function NoteOut(){
	document.write('</td></tr></table><p class="br">Å@</p>');
}

