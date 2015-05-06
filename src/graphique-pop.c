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

	m->contw_scrl_curs = XCreateSimpleWindow(
		dis, 
		m->contw_scrl, 
		1, 
		1, 
		SHOWM_SCRLBR_WIDTH-2, 
		100, 
		0, 
		WhitePixel(dis, 0), 
		WhitePixel(dis, 0)
	);	
	
	XSelectInput(dis, m->contw_main,  ExposureMask|KeyPressMask);
	XSelectInput(dis, m->contw_txt,  ExposureMask|KeyPressMask);
	XSelectInput(dis, m->contw_scrl,  ExposureMask|ButtonPress|ButtonReleaseMask|ButtonMotionMask);
	
	XMapWindow(dis,m->contw_main);
	XMapWindow(dis,m->contw_txt);
	XMapWindow(dis,m->contw_scrl);
	//XMapWindow(dis,m->contw_scrl_curs);
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
			m->text_height=cursY;
		}

		XDrawString(dis, m->contw_txt, popGC, cursX, cursY, token,strlen(token));
		cursX+=xcsTxt.width + spaceWidth;
		token = strtok(NULL, space);
		if(token)
			*(token-1) = space[0];	  
	}	
}


void graphButtonPressScroll(XButtonEvent *e, mails*m)
{
	XEvent xe;

	Window root;
    int x, y;
    unsigned int width, height;
    unsigned int border_width;
    unsigned int depth;
    int posMouse;
    int percCurs;
    int sizeOffset;
	
    posMouse = e->y;

	while (1) 
	{
		XClearWindow(dis, m->contw_txt);
		XGetGeometry(dis, m->contw_scrl, &root, &x, &y, &width, &height, &border_width, &depth);

		percCurs = ((posMouse * 100)/height);
		sizeOffset = (percCurs * (m->text_height)) / 100;

		XResizeWindow(dis, m->contw_txt, SHOWM_WIDTH - SHOWM_SCRLBR_WIDTH, m->text_height);
		XMoveWindow(dis, m->contw_txt, 1, 0-sizeOffset);
		graphRefreshMail(m);
		XMaskEvent(dis, ButtonReleaseMask|ButtonMotionMask,&xe);

		if (xe.type == MotionNotify)
		{
			if(xe.xmotion.y<0)
				posMouse= 0;
			else
				if(xe.xmotion.y>SHOWM_HEIGHT)
					posMouse= SHOWM_HEIGHT;
				else
					posMouse= xe.xmotion.y;

		}
		else{
			if (xe.type == ButtonRelease) 
				break;
		}          
	}


/*
	w-=(CURSOR_H>>1);
	if (w<=0)
	w=1;
	else { if (w> (COORDCOL(MAXCOLUMN+3)-BORDERLENGTH-(CURSOR_H)))
	w=(COORDCOL(MAXCOLUMN+3)-BORDERLENGTH-(CURSOR_H));
	}*/
	//XResizeWindow(dis, m->contw_scrl_curs, SHOWM_SCRLBR_WIDTH-2, (20 * (m->text_height)) / 100);
}