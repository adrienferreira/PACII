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
	m->cont_text = (char*)mmap(NULL, sizeFichMail, PROT_READ, MAP_PRIVATE, fichMail, (off_t)0);
}

void graphShowMailContent(mails*m)
{
	m->cont_win = XCreateSimpleWindow(
		dis, 
		RootWindow(dis, 0), 
		1, 1, 
		SHOWM_WIDTH, SHOWM_HEIGHT, 
		0, 
		BlackPixel(dis, 0), 
		WhitePixel(dis, 0)
	);

	m->cont_scrl = XCreateSimpleWindow(
		dis, 
		m->cont_win, 
		SHOWM_WIDTH-SHOWM_SCRLBR_WIDTH, 1, 
		SHOWM_SCRLBR_WIDTH, SHOWM_HEIGHT, 
		0, 
		BlackPixel(dis, 0), 
		BlackPixel(dis, 0)
	);
	
	XSelectInput(dis, m->cont_scrl,  ExposureMask|EnterWindowMask |LeaveWindowMask);
	XSelectInput(dis, m->cont_win,  ExposureMask|KeyPressMask);
	
	XMapWindow(dis,m->cont_scrl);	
	XMapWindow(dis,m->cont_win);	
}