#include <common-pop.h>

void linkMimeEnd(pop*p,mimes* newM)
{
	newM->next = NULL;
	if(!(p->first_mime)){
		p->first_mime = newM;
		p->last_mime = newM;
	}
	else{
		p->last_mime->next = newM;
		p->last_mime = newM;	
	}
}

void initMimes(FILE*fMime, pop*p)
{
	const char* strRegex ="^([-0-9A-Za-z\\/\\+\\.]+)\\s*([-0-9A-Za-z\\/\\+\\.]*)";
	const int NB_PAR = 2+1;

	char buff[MAXLINE];
	mimes* newM;
	regex_t preg;
	regmatch_t matches[NB_PAR];

	regcomp (&preg, strRegex, REG_EXTENDED);

	while(fgets(buff,MAXLINE,fMime))
	{
		if(!regexec(&preg, buff,NB_PAR, matches, 0))
		{
			if(matches[1].rm_eo != matches[1].rm_so && matches[2].rm_eo != matches[2].rm_so)
			{
				newM = (mimes*)malloc(sizeof(mimes));
				linkMimeEnd(p,newM);

				newM->mimetype = strndup(buff+matches[1].rm_so,(matches[1].rm_eo-matches[1].rm_so));
				newM->canonical = strndup(buff+matches[2].rm_so,(matches[2].rm_eo-matches[2].rm_so));
			}
		}
	}
	
	regfree(&preg);
}

char*getCanonical(mimes*m,char*mimetype)
{
	mimes*i;
	for(i=m;i;i=i->next)
		if(!strncmp(mimetype, i->mimetype, strlen(mimetype)))
			return i->canonical;
	return NULL;
}

