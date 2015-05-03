#include <common-pop.h>

void graphLoadMailContent(mails*m)
{
	int fichMail;
	size_t sizeFichMail;
	struct stat statsFichMail;
	char relPath[MAXLINE];
	
	sprintf(relPath,"%s/%s.%s",MAIL_BASE_FOLDER, m->id, m->canonical);
	fichMail = open(relPath, O_RDONLY);
	fstat(fichMail, &statsFichMail);
	sizeFichMail=(size_t)statsFichMail.st_size;	
	m->cont_text = (char*)mmap(
		NULL, 
		sizeFichMail, 
		PROT_WRITE, 
		MAP_PRIVATE, 
		fichMail, 
		(off_t)0
	);
}

void graphShowMailContent(mails*m)
{
	m->contw_main = XCreateSimpleWindow(
		dis, 
		RootWindow(dis, 0), 
		1, 1, 
		SHOWM_WIDTH, 
		SHOWM_HEIGHT, 
		0, 
		BlackPixel(dis, 0), 
		WhitePixel(dis, 0)
	);

	m->contw_txt = XCreateSimpleWindow(
		dis, 
		m->contw_main, 
		1, 1, 
		SHOWM_WIDTH - SHOWM_SCRLBR_WIDTH, 
		SHOWM_HEIGHT, 
		0, 
		BlackPixel(dis, 0), 
		WhitePixel(dis, 0)
	);

	m->contw_scrl = XCreateSimpleWindow(
		dis, 
		m->contw_main, 
		SHOWM_WIDTH-SHOWM_SCRLBR_WIDTH, 
		1, 
		SHOWM_SCRLBR_WIDTH, 
		SHOWM_HEIGHT, 
		0, 
		BlackPixel(dis, 0), 
		BlackPixel(dis, 0)
	);
	
	XSelectInput(dis, m->contw_main,  ExposureMask|KeyPressMask);
	XSelectInput(dis, m->contw_txt,  ExposureMask|KeyPressMask);
	XSelectInput(dis, m->contw_scrl,  ExposureMask|EnterWindowMask |LeaveWindowMask);

	XMapWindow(dis,m->contw_main);
	XMapWindow(dis,m->contw_txt);
	XMapWindow(dis,m->contw_scrl);		
}

void graphRefreshMail(mails*m)
{
	XFontStruct *xfs;
	int s,a,d;
	XCharStruct xcsTxt,xcsSpace;

	char *token;
	int cursX,cursY;
	char *space=" ";
	int spaceWidth;
	int curMaxHeight;
	int curTxtHeight;

	cursX = SHOWM_TXT_HOFFSET;
	cursY = SHOWM_TXT_VOFFSET*4;
	curMaxHeight = -1;
	curTxtHeight = 0;

	xfs = XQueryFont(dis, XGContextFromGC(popGC));
	XTextExtents(xfs, space, strlen(space) , &s,&a,&d,&xcsSpace);
	spaceWidth = xcsSpace.width;
	token = strtok(m->cont_text, space);

	while(token) 
	{
		XTextExtents(xfs, token, strlen(token) , &s,&a,&d,&xcsTxt);

		curTxtHeight = (xcsTxt.ascent+xcsTxt.descent);
		if(curTxtHeight>curMaxHeight)
			curMaxHeight = curTxtHeight;

		if((cursX+xcsTxt.width)>(MAINW_WIDTH-SHOWM_SCRLBR_WIDTH-SHOWM_TXT_HOFFSET))
		{
			cursX=SHOWM_TXT_HOFFSET;
			cursY+=curMaxHeight+SHOWM_TXT_VOFFSET;
			curMaxHeight = -1;
		}

		XDrawString(dis, m->contw_txt, popGC, cursX, cursY, token,strlen(token));
		cursX+=xcsTxt.width + spaceWidth;
		token = strtok(NULL, space);	  
	}	
}