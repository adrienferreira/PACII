// TODO expose redraw, car XSelectInput ExposureMask
//change focus, cursor XC_xterm 
//largeur de la chaine supérieur taille fenetre
#include <common-pop.h>


void putStringWindow(Window*w, char*str)
{
	if( str)
	{
		printf("%s\n",str);
		XUnmapWindow(dis, *w);
		XMapWindow(dis, *w);
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

void clickUserPass(pop*p,FILE*fSo,char*user,char*pass)
{
	char* errMsg;	
	errMsg = NULL;
	

	if(checkUserPassStr(user,pass))
	{
		sendUser(fSo, user);
		if(!isServerOk(fSo)){
			putStringWindow(&(wins[W_ERRO]),MSG_WRONG_USR);
		}else{
			sendPass(fSo, pass);
			if(!isServerOk(fSo)){
				putStringWindow(&(wins[W_ERRO]),MSG_WRONG_PASS);
			}else{
				XUnmapWindow(dis, wins[W_USER]);
				XUnmapWindow(dis, wins[W_PASS]);
				XUnmapWindow(dis, wins[W_VALI]);
				XUnmapWindow(dis, wins[W_ERRO]);
				//TODO ancienne position
				XMoveResizeWindow(dis, wins[W_MAIN], 1, 1, MAINW_WIDTH, MAINW_HEIGHT);
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
	int nbrOW;
	char buff[MAXLINE];
	int endOfHeader;
	mails*m;

	sendList(fSo);
	ret = isServerOk(fSo);

	if(!ret)fatal("LIST échoue");
	
	while(fgets(buff, MAXLINE, fSo) && strncmp(buff,END,strlen(END)))
	{
		printf("%s\n",buff);
		m = (mails*)malloc(sizeof(mails));
		linkMailEnd(p,m);
		m->id = strndup(buff,index(buff,' ')-buff);
	}

	for(m=p->first_mail,nbrOW=0 ; m && nbrOW<NBR_MAX_OW; m=m->next,nbrOW++)
	{
		sendTop(fSo,(m->id));
		ret = isServerOk(fSo);
		if(!ret)fatal("TOP échoue");
		getOverviewInfo(fSo,&(m->from),&(m->date));
	}
	
	drawOverview(p);
}

void drawOverview(pop*p)
{
	mails*m;
	int fenOvId;
	Window curW;
	int fromDateWidth;
	
	fromDateWidth = (MAINW_WIDTH - (4*OW_OFFSET) - OW_ID_WIDTH)/2;
	
	for(m=p->first_mail,fenOvId=0 ; m; m=m->next,fenOvId++)
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
			(3*OW_OFFSET) + OW_ID_WIDTH + fromDateWidth, 
			(fenOvId*OW_HEIGHT) + ((1+fenOvId)*OW_OFFSET), 
			fromDateWidth, 
			OW_HEIGHT, 
			0, 
			BlackPixel(dis, 0), 
			WhitePixel(dis, 0)
		);
		
		putStringWindow(&(m->ow_id), m->id);
		putStringWindow(&(m->ow_from), m->from);
		putStringWindow(&(m->ow_date), m->date);
		
		XSelectInput(dis, m->ow_id, ExposureMask | ButtonPressMask);
		
		XMapWindow(dis, m->ow_id);
		XMapWindow(dis, m->ow_from);
		XMapWindow(dis, m->ow_date);
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
		if(e->window == m->ow_id){
			if(!(m->is_downloaded)){
				m->is_downloaded = 1;
				processMail(p,fSo,m->id);
			}			
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
	int so;
	FILE*fSo;
	pop p;
	
	p.last_mail=NULL;
	p.first_mail=NULL;
	
	so = open("./obj/scenario5.pop", O_RDONLY);
	fSo = fdopen(so, "r");
	
	initMimes(fdopen(open("/etc/mime.types", O_RDONLY), "r"),&p);

	memset(strUser,0,MAXLINE);
	memset(strPass,0,MAXLINE);
	
	focus = W_USER;
	dis = XOpenDisplay(NULL);

	wins[W_MAIN] = XCreateSimpleWindow(dis, RootWindow(dis, 0), 1, 1, 500, 120, 0, BlackPixel(dis, 0), BlackPixel(dis, 0));
	wins[W_USER] = XCreateSimpleWindow(dis, wins[W_MAIN], 5, 5, 490, 25, 0, BlackPixel(dis, 0), WhitePixel(dis, 0));
	wins[W_PASS] = XCreateSimpleWindow(dis, wins[W_MAIN], 5, 35, 490, 25, 0, BlackPixel(dis, 0), WhitePixel(dis, 0));
	wins[W_VALI] = XCreateSimpleWindow(dis, wins[W_MAIN], 400, 65, 80, 25, 0, BlackPixel(dis, 0), WhitePixel(dis, 0));
	wins[W_ERRO] = XCreateSimpleWindow(dis, wins[W_MAIN], 5, 65, 200, 25, 0, BlackPixel(dis, 0), WhitePixel(dis, 0));
		
	XSelectInput(dis, wins[W_MAIN], ExposureMask | ButtonPressMask | KeyPressMask);
	XSelectInput(dis, wins[W_USER], ExposureMask | ButtonPressMask);
	XSelectInput(dis, wins[W_PASS], ExposureMask | ButtonPressMask);
	XSelectInput(dis, wins[W_VALI], ExposureMask | ButtonPressMask);

	popGC = XCreateGC(dis, wins[W_USER], 0, NULL);
	XStoreName(dis, wins[W_MAIN], F_NAME);

	XMapWindow(dis, wins[W_MAIN]);
	XMapWindow(dis, wins[W_USER]);
	XMapWindow(dis, wins[W_PASS]);
	XMapWindow(dis, wins[W_VALI]);
	XMapWindow(dis, wins[W_ERRO]);

	// XFlush(dis); 
	while (1)
	{
		XNextEvent(dis, &report);
		switch (report.type)
		{
		case Expose:
			{
				XDrawString(dis, wins[W_VALI], popGC, 10,15, TXT_CONF,strlen(TXT_CONF));				
				XFlush(dis);
				drawOverview(&p);
				/*
				if(report.xexpose.window == wins[W_MAIN]){
					clicList(&p,fSo);
				}
				*/
				break;
			}

		case KeyPress:
			{
				fKeyPress(&p,fSo, &(report.xkey));


				// Close the program if q is pressed. 
				if(XLookupKeysym(&report.xkey, 0)== XK_q)
				{
					exit(0);
				}
				break;

				break;
			}
		case ButtonPress:
			{
				fButtonPress(&p,fSo,&report.xbutton);
				printf("button\n");
				break;
			}
		}
		
		/*case LeaveNotify: {fLeave(&e.xcrossing, &own); break;}
		case EnterNotify: {fEnter(&e.xcrossing, &own); break;}*/
	}

	return 0;
}
