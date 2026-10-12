
function doExpand(para,lead) {
	if (para.style.display == "") {
		para.style.display = "none";
		lead.src = "images/treeopen.gif"
	}
	else{
		para.style.display= "";
		lead.src = "images/treeclose.gif"
	}
}

function doExpandArr(para,lead) {
	if (para.style.display == "") {
		para.style.display = "none";
		lead.src = "images/arrrt.gif"
	}
	else{
		para.style.display= "";
		lead.src = "images/arrdn.gif"
	}
}

function ParaIn(){
	var a=ParaIn.arguments;
	document.write('<table border=0 cellpadding=0 cellspacing=1 bgcolor=lavender><tr><td width="100%">&nbsp;');
	if (a.length > 0) document.write(a[0]);
	document.write('&nbsp;</td></tr></table><div class="framerect"><blockquote>');
}

function ParaOut(){
	document.write('</blockquote></div><p class="br">Å@</p>');
}

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

