// TODO expose redraw, car XSelectInput ExposureMask
//change focus, cursor XC_xterm 
//largeur de la chaine supérieur taille fenetre
//contexte SaisieMdp, overview, consultation
#include <common-pop.h>


void putStringWindow(Window*w, char*str)
{
	Window root;
    int x, y;
    unsigned int width, height;
    unsigned int border_width;
    unsigned int depth;
	
	XGetGeometry(dis, *w, &root, &x, &y, &width, &height, &border_width, &depth);

	if(str)
	{
		XClearArea(dis, *w, x, y, width, height, 0);
		XDrawString(dis, *w, popGC, 10, 15, str,strlen(str));	
	}
}

int checkUserPassStr(char*user,char*pass)
{
	char* errMsg;	
	errMsg = NULL;
	
	if(!strlen(user))
		errMsg = MSG_EMPT_USR;
	else{
		if(!strlen(pass))
			errMsg = MSG_EMPT_PASS;
	}

	if(errMsg)
		putStringWindow(&(wins[W_ERRO]),errMsg);

	return !errMsg;
}


void emptyUserPassBoxes(void)
{
	XUnmapWindow(dis, wins[W_USER]);
	XMapWindow(dis, wins[W_USER]);
	XUnmapWindow(dis, wins[W_PASS]);
	XMapWindow(dis, wins[W_PASS]);
}

void changeWindowBgColor(Window*w,char*colorName)
{
	XColor color;

	XAllocNamedColor(
		dis,
		DefaultColormap(dis, DefaultScreen(dis)),			       
		colorName,
		&color, &color
	);

	XSetWindowBackground(dis, *w, color.pixel);
	
	XUnmapWindow(dis, *w);
	XMapWindow(dis, *w);
}
//TODO À supprimer
void getWindowWidthHeight(Window*w,int*paramWidth,int*paramHeight)
{
	Window root;
    int x, y;
    unsigned int width, height;
    unsigned int border_width;
    unsigned int depth;
	
	XGetGeometry(dis, *w, &root, &x, &y, &width, &height, &border_width, &depth);

	*paramWidth = width;
	*paramHeight = height;
}

void clickUserPass(pop*p,FILE*fSo,char*user,char*pass)
{
	if(checkUserPassStr(user,pass))
	{
		sendUser(fSo, user);
		if(!isServerOk(fSo)){
			putStringWindow(&(wins[W_ERRO]),MSG_WRONG_USR);
			emptyUserPassBoxes();
		}else{
			sendPass(fSo, pass);
			if(!isServerOk(fSo)){
				putStringWindow(&(wins[W_ERRO]),MSG_WRONG_PASS);
				emptyUserPassBoxes();
			}else{
				XUnmapWindow(dis, wins[W_USER]);
				XUnmapWindow(dis, wins[W_PASS]);
				XUnmapWindow(dis, wins[W_VALI]);
				XUnmapWindow(dis, wins[W_ERRO]);
				XResizeWindow(dis, wins[W_MAIN], MAINW_WIDTH, MAINW_HEIGHT);
				clicList(p,fSo);
			}
		}
	}	
}

void linkMailEnd(pop*p,mails* newM)
{
	newM->next = NULL;
	if(!(p->first_mail)){
		p->first_mail = newM;
		p->last_mail = newM;
	}
	else{
		p->last_mail->next = newM;
		p->last_mail = newM;	
	}
}

void getOverviewInfo(FILE*fSo,char**from,char**date)
{
	char buff[MAXLINE];
	
	*from = NULL;
	*date = NULL;
	
	while(fgets(buff, MAXLINE, fSo) && strncmp(buff,END,strlen(END)))
	{
		if(!strncmp(buff,H_FROM,strlen(H_FROM)))
			*from = strndup(buff+strlen(H_FROM),strlen(buff+strlen(H_FROM))-1);
		if(!strncmp(buff,H_DATE,strlen(H_DATE)))
			*date = strndup(buff+strlen(H_DATE),strlen(buff+strlen(H_DATE))-1);
	}
}

void clicList(pop*p,FILE*fSo)
{
	int ret;
	int fenOvId;
	char buff[MAXLINE];
	mails*m;

	sendList(fSo);
	ret = isServerOk(fSo);

	if(!ret)fatal("LIST échoue");
	
	while(fgets(buff, MAXLINE, fSo) && strncmp(buff,END,strlen(END)))
	{
		printf("%s\n",buff);
		m = (mails*)malloc(sizeof(mails));
		memset(m, 0, sizeof(mails));
		linkMailEnd(p,m);
		m->id = strndup(buff,index(buff,' ')-buff);
	}

	for(m=p->first_mail,fenOvId=0 ; m && fenOvId<NBR_MAX_OW; m=m->next,fenOvId++)
	{
		sendTop(fSo,(m->id));
		ret = isServerOk(fSo);
		if(!ret)fatal("TOP échoue");
		getOverviewInfo(fSo,&(m->from),&(m->date));
	}
	
	drawOverview(p);
}

int getOWFromDateWidth(Window*main)
{
	const int MIN_SIZE_WIN = 5;
	int width,height;
    int calcWidth;
	
	getWindowWidthHeight(main,&width,&height);
	calcWidth = (width - (4*OW_OFFSET) - OW_ID_WIDTH)/2;

	return calcWidth < MIN_SIZE_WIN ? MIN_SIZE_WIN : calcWidth;
}

int getOWFromX(int fromDateWidth){
	return (3*OW_OFFSET) + OW_ID_WIDTH + fromDateWidth;
}

int getOWFromY(int fromDateWidth, int id){
	return (id*OW_HEIGHT) + ((1+id)*OW_OFFSET);
}

void drawOverview(pop*p)
{
	mails*m;
	int fenOvId;
	int fromDateWidth;
	Cursor hand;		
	
	fromDateWidth = getOWFromDateWidth(&(wins[W_MAIN]));
	hand = XCreateFontCursor (dis, XC_hand2) ;
	
	for(m=p->first_mail,fenOvId=0 ; m && fenOvId<NBR_MAX_OW; m=m->next,fenOvId++)
	{
		m->ow_id = XCreateSimpleWindow(
			dis, 
			wins[W_MAIN], 
			OW_OFFSET, 
			(fenOvId*OW_HEIGHT) + ((1+fenOvId)*OW_OFFSET), 
			OW_ID_WIDTH, 
			OW_HEIGHT, 
			0, 
			BlackPixel(dis, 0), 
			WhitePixel(dis, 0)
		);

		m->ow_from = XCreateSimpleWindow(
			dis, 
			wins[W_MAIN], 
			(2*OW_OFFSET) + (OW_ID_WIDTH), 
			(fenOvId*OW_HEIGHT) + ((1+fenOvId)*OW_OFFSET), 
			fromDateWidth, 
			OW_HEIGHT, 
			0, 
			BlackPixel(dis, 0), 
			WhitePixel(dis, 0)
		);

		m->ow_date = XCreateSimpleWindow(
			dis, 
			wins[W_MAIN], 
			getOWFromX(fromDateWidth), 
			getOWFromY(fromDateWidth,fenOvId) , 
			fromDateWidth, 
			OW_HEIGHT, 
			0, 
			BlackPixel(dis, 0), 
			WhitePixel(dis, 0)
		);
		
		
		XDefineCursor (dis, m->ow_id, hand);
		XDefineCursor (dis, m->ow_from, hand);
		XDefineCursor (dis, m->ow_date, hand);
		
		XSelectInput(dis, m->ow_id,  EnterWindowMask |LeaveWindowMask| ButtonPressMask);

		XMapWindow(dis,m->ow_id);
		XMapWindow(dis,m->ow_from);
		XMapWindow(dis,m->ow_date);
		
		putStringWindow(&(m->ow_id), m->id);
		putStringWindow(&(m->ow_from), m->from);
		putStringWindow(&(m->ow_date), m->date);
	}
}

void refresh(pop*p)
{
	mails*m;
	int fenOvId;
	int fromDateWidth;	
	
	//TEST
	//fromDateWidth = getOWFromDateWidth(&(wins[W_MAIN]));
	//XClearWindow(dis, wins[W_MAIN]);
	
	fprintf(stderr,"r");
	for(m=p->first_mail,fenOvId=0 ; m && fenOvId<NBR_MAX_OW; m=m->next,fenOvId++)
	{
		//TEST
		//XResizeWindow(dis, (m->ow_from), fromDateWidth, OW_HEIGHT);
		//XMoveResizeWindow(dis, (m->ow_date), getOWFromX(fromDateWidth), getOWFromY(fromDateWidth,fenOvId) ,  fromDateWidth, OW_HEIGHT);
		//TEST
		//XDrawString(dis, (m->ow_id), popGC, 10, 15, m->id,strlen(m->id));	
		//XDrawString(dis, (m->ow_from), popGC, 10, 15, m->from,strlen(m->from));	
		//XDrawString(dis, (m->ow_date), popGC, 10, 15, m->date,strlen(m->date));

		graphRefreshMail(m);
	}
}

void fButtonPress(pop*p,FILE*fSo,XButtonEvent * e)
{
	mails*m;
	
	if (e->window == wins[W_VALI]){
		clickUserPass(p,fSo,strUser,strPass);
	}

	if (e->window == wins[W_MAIN])
		focus = W_MAIN;
	if (e->window == wins[W_USER])
		focus = W_USER;
	if (e->window == wins[W_PASS])
		focus = W_PASS;	
		
	for(m=p->first_mail; m; m=m->next)
	{
		if(e->window == m->ow_id)
		{
			if(!(m->is_downloaded))
			{
				m->is_downloaded = 1;
				processMail(p,fSo,m->id);
				changeWindowBgColor(&(m->ow_id),COLOR_DOWL);
				XUnmapWindow(dis,(m->ow_id));
				XMapWindow(dis,(m->ow_id));
			}
			else{
				graphLoadMailContent(m);
				graphShowMailContent(m);
			}
		}
	
		if(e->window == m->contw_scrl){
			graphButtonPressScroll(e, m);
		}
	}
}

void fEnter(pop*p,XEnterWindowEvent *e)
{
	mails*m;

	fprintf(stderr,"e");
	for(m=p->first_mail; m; m=m->next)
	{
		if(e->window==m->ow_id ||e->window==m->ow_from ||e->window==m->ow_date)
		{				
			changeWindowBgColor(&(m->ow_from), COLOR_HOVER);
			changeWindowBgColor(&(m->ow_date), COLOR_HOVER);
		}
	}
}

void fLeave(pop*p, XLeaveWindowEvent *e)
{
	mails*m;
		
	fprintf(stderr,"e");
	for(m=p->first_mail; m; m=m->next)
	{
		if(e->window==m->ow_id ||e->window==m->ow_from ||e->window==m->ow_date)
		{
			changeWindowBgColor(&(m->ow_from), COLOR_BASE);
			changeWindowBgColor(&(m->ow_date), COLOR_BASE);
		}
	}
}

int isCharUserOk(char c)
{
	printf("%c",c);
	int lowL = (c>='a' && c<='z');
	int capL = (c>='A' && c<='Z');
	int dig  = (c>='0' && c<='9');
	int spec = (c=='_' || c=='-');

	return lowL||capL||dig||spec;
}

void fKeyPress(pop*p,FILE*fSo,XKeyEvent * e)
{
	char keyBuff[KEY_BUFF];
	char*strFocus;
	
	strFocus = NULL;

	if(focus == W_USER) strFocus = strUser;
	if(focus == W_PASS) strFocus = strPass;

	if(strFocus)
	{
		XLookupString(e, keyBuff, KEY_BUFF, NULL, NULL);		

		if(XLookupKeysym(e, 0) == XK_BackSpace && strlen(strFocus))
			strFocus[strlen(strFocus)-1]='\0';

		if(isCharUserOk(keyBuff[0]))
			strncat(strFocus, keyBuff, 1);

		putStringWindow(&(wins[focus]),strFocus);
	}
	
	if(XLookupKeysym(e, 0) == XK_Tab){
		if(focus == W_USER){
			focus = W_PASS;
		}else{
			if(focus == W_PASS)
				focus = W_USER;
		}
	}

	if(XLookupKeysym(e, 0) == XK_Return)
		clickUserPass(p,fSo,strUser,strPass);

}

int main(int argc,char**argv)
{
	XEvent report;
	int so;
	FILE*fSo;
	pop p;
	
	p.first_mail=NULL;
	p.last_mail=NULL;
	p.last_mime=NULL;
	p.first_mime=NULL;
	
	so = open("./scenarios/5.pop", O_RDONLY);
	fSo = fdopen(so, "r");
	
	initMimes(fdopen(open("/etc/mime.types", O_RDONLY), "r"),&p);

	memset(strUser,0,MAXLINE);
	memset(strPass,0,MAXLINE);
	
	focus = W_USER;
	dis = XOpenDisplay(NULL);
	
	wins[W_MAIN] = XCreateSimpleWindow(dis, RootWindow(dis, 0), 1, 1, 500/*MAINW_WIDTH*/, 120/*MAINW_HEIGHT*/, 0, BlackPixel(dis, 0), BlackPixel(dis, 0));
	wins[W_USER] = XCreateSimpleWindow(dis, wins[W_MAIN], 5, 5, 490, OW_HEIGHT, 0, BlackPixel(dis, 0), WhitePixel(dis, 0));
	wins[W_PASS] = XCreateSimpleWindow(dis, wins[W_MAIN], 5, 35, 490, OW_HEIGHT, 0, BlackPixel(dis, 0), WhitePixel(dis, 0));
	wins[W_ERRO] = XCreateSimpleWindow(dis, wins[W_MAIN], 5, 65, 200, OW_HEIGHT, 0, BlackPixel(dis, 0), WhitePixel(dis, 0));
	wins[W_VALI] = XCreateSimpleWindow(dis, wins[W_MAIN], 400, 65, 	OW_BUTTON_WIDTH, OW_HEIGHT, 0, BlackPixel(dis, 0), WhitePixel(dis, 0));
	wins[W_QUIT] = XCreateSimpleWindow(dis, wins[W_MAIN], 0, 0, 	OW_BUTTON_WIDTH, OW_HEIGHT, 0, BlackPixel(dis, 0), WhitePixel(dis, 0));

	XSelectInput(dis, wins[W_MAIN], ExposureMask | ButtonPressMask | KeyPressMask);
	XSelectInput(dis, wins[W_USER], ExposureMask | ButtonPressMask);
	XSelectInput(dis, wins[W_PASS], ExposureMask | ButtonPressMask);
	XSelectInput(dis, wins[W_VALI], ExposureMask | ButtonPressMask);
	XSelectInput(dis, wins[W_QUIT], ExposureMask | ButtonPressMask);

	popGC = XDefaultGC(dis, 0);
	XStoreName(dis, wins[W_MAIN], F_NAME);
    
	//XMapWindow(dis, wins[W_MAIN]);
	//XMapWindow(dis, wins[W_USER]);
	//XMapWindow(dis, wins[W_PASS]);
	//XMapWindow(dis, wins[W_VALI]);
	//XMapWindow(dis, wins[W_ERRO]);

	//clicList(&p,fSo);
	//drawOverview(&p);
/*
	XGCValues values_return;
	XFontStruct *xfs;
	int l,s,a,d;
	XCharStruct xcs;

	xfs = XQueryFont(dis, XGContextFromGC(popGC));
	XTextExtents(xfs, " " , 1, &s,&a,&d,&xcs);	

	char txt[2048];
	strcpy(txt,"Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.");
    char *token;
    int curX,curY;
    char space=' ';
    int spaceWidth;
    int curMaxHeight;
	
	curX = 0;
	curY = 0;
	token = strtok(txt, &space);
	spaceWidth = xcs.width;
	curMaxHeight = -1;

	while( token != NULL ) 
	{
		XTextExtents(xfs, token, strlen(token) , &s,&a,&d,&xcs);

		if((curX+xcs.width)>MAINW_WIDTH){
			curX=0;
			printf("\n");
			//curY+=xcs.   cur max height
		}
				
		printf("%s",token);
		printf(" ");
		curX+=xcs.width + +spaceWidth;
		token = strtok(NULL, &space);	  
	}
*/
	/*
		XGCValues values_return;
		XFontStruct *xfs;
		int l,s,a,d;
		XCharStruct xcs;

		xfs = XQueryFont(dis, XGContextFromGC(popGC));
		XTextExtents(xfs, TXT_CONF, strlen(TXT_CONF) , &s,&a,&d,&xcs);
		printf("%d\n",xcs.width);
	*/
	
	/*TEST*/
	struct mail m;
	m.id="1";
	m.from="adrien ferreira <adrien.ferreira@upmc.fr>";
	m.date="30 decembre 1990";
	m.is_downloaded=1;
	m.mime="plain/text";
	m.canonical="asc";
	m.cont_text=NULL;
	m.next=NULL;
	p.first_mail = &m;
	p.last_mail = &m;
	graphLoadMailContent(&m);
	graphShowMailContent(&m);
	/*TEST*/

	for(;;)
	{
		XNextEvent(dis, &report);
		switch (report.type)
		{
			case Expose:
			{
				XDrawString(dis, wins[W_VALI], popGC, 10,15, TXT_CONF,strlen(TXT_CONF));				
				XFlush(dis);
				if(report.xexpose.count == 0){
					refresh(&p);
				}
				break;
			}

			case KeyPress:
			{
				fKeyPress(&p,fSo, &(report.xkey));
				
				// Close the program if q is pressed. 
				if(XLookupKeysym(&report.xkey, 0)== XK_q)
				{
					if(report.xkey.window==wins[W_MAIN]){
						exit(EXIT_SUCCESS);
					}
					else{
						XUnmapWindow(dis,report.xkey.window);
						exit(EXIT_SUCCESS);//@TODO
					}
				}
				break;
			}

			case ButtonPress:
			{
				fButtonPress(&p,fSo,&report.xbutton);
				printf("button\n");
				break;
			}
			
			case LeaveNotify: 
			{
				//fLeave(&e.xcrossing, &own);
				printf("leave\n"); 
				fLeave(&p,&report.xcrossing);
				break;
			}
			case EnterNotify: 
			{
				fEnter(&p,&report.xcrossing);
				break;
			}
		}
	}

	exit(EXIT_SUCCESS);
}
