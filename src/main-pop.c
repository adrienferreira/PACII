#include <common-pop.h>

void r_processMail(pop*p, FILE*fSo, char*paramFolderName, char*mailNbr,  int depth);

int isServerOk(FILE * fSo)
{
	char buff[MAXLINE];

	fgets(buff, MAXLINE, fSo);
	if (!strncmp(buff, SERVER_OK, strlen(SERVER_OK))){
		printf("Réponse du serveur : " SERVER_OK "\n");
		return 1;
	}

	if (!strncmp(buff, SERVER_ERR, strlen(SERVER_ERR))){
		printf("Réponse du serveur : " SERVER_ERR "\n");
		return 0;
	}

	fatal("Réponse du serveur invalide");
	return-1;//unreached
}

void sendCmd(FILE * fSo, char *cmd, char *param1, char *param2)
{
	if(!cmd || (!param1 && param2))
		fatal("Format de commande incorrect");

	#ifndef DEBUG
	fwrite(cmd, sizeof(char), strlen(cmd), fSo);
	fwrite(" ", sizeof(char), 1, fSo);

	if(param1)
		fwrite(param1, sizeof(char), strlen(param1), fSo);

	if(param2){
		fwrite(", ", sizeof(char), 2, fSo);
		fwrite(param2, sizeof(char), strlen(param2), fSo);
	}

	fwrite(CRLF, sizeof(char), strlen(CRLF), fSo);
	#endif
}

void sendUser(FILE * fSo, char *name){
	sendCmd(fSo, CMD_USER, name, NULL);
}

void sendPass(FILE * fSo, char *pass){
	sendCmd(fSo, CMD_PASS, pass, NULL);
}

void sendList(FILE * fSo){
	sendCmd(fSo, CMD_LIST, NULL, NULL);
}

void sendQuit(FILE * fSo){
	sendCmd(fSo, CMD_QUIT, NULL, NULL);
}

void sendRetr(FILE * fSo, char *mailNbr)
{
	if(!atoi(mailNbr))
		fatal("Numéro de mail invalide");
	sendCmd(fSo, CMD_RETR, mailNbr, NULL);
}

void sendTop(FILE * fSo, char *mailNbr)
{
	if(!atoi(mailNbr))
		fatal("Numéro de mail invalide");
	sendCmd(fSo, CMD_TOP, mailNbr, "0");
}

void saveSimpleContent(FILE*fSo, char*folderName, char*fileName,char* canonical)
{
	char buff[MAXLINE];
	char fullPath[MAXFILENAME];
	int lenFN;
	FILE *dest;

	sprintf(fullPath,"%s/%s/%s.%s",MAIL_BASE_FOLDER, folderName, fileName, canonical);
	dest=fopen(fullPath, "w+");

	while(fgets(buff, MAXLINE, fSo) && strncmp(buff,CRLF,strlen(CRLF)))
		fwrite(buff,sizeof(char),strlen(buff),dest);
}


void r_processMail(pop*p, FILE*fSo, char*paramFolderName, char*mailNbr,  int depth)
{
	char buff[MAXLINE];
	char popBound[MAXLINE];
	int isMultipart;
	int endOfHeader, endOfMail, boundFound;	
	char *mime, *canonical, *boundary;
	char *folderName;
	
	endOfHeader =0;
	endOfMail = 0;
	boundFound=0;
	folderName = paramFolderName;
	
	while(!endOfHeader)
	{
		fgets(buff, MAXLINE, fSo);
		printf("%s\n",buff);
		endOfHeader = !strncmp(buff,CRLF,strlen(CRLF));

		if(!strncmp(buff,H_CONTTYPE,strlen(H_CONTTYPE)))
			processContentType(p, buff, &mime, &canonical, &boundary);
	}

	while(!endOfMail)
	{
		if(!depth && boundary)
		{
			sprintf(popBound,"--%s--",boundary);
			folderName = mailNbr;
		
			do{
				fgets(buff, MAXLINE, fSo);
				boundFound = !strncmp(buff,popBound,strlen(popBound)-2);
				endOfMail= boundFound && !strncmp(buff,popBound,strlen(popBound));
			}
			while(!boundFound);

			if(!endOfMail){
				r_processMail(p,fSo,folderName,mailNbr,depth+1);
				continue;
			}
		}
		
		if(!endOfMail){
			saveSimpleContent(fSo, folderName, mailNbr, canonical);
			break;
		}
	}

	free(mime);
	free(canonical);
	free(boundary);

}

void processMail(pop*p, FILE*fSo, char*mailNbr){
	char buff[MAXLINE];
	r_processMail(p,fSo,NULL,mailNbr,0);
	while(fgets(buff, MAXLINE, fSo)&& !strncmp(buff,END,strlen(END)));
}

//TODO free regex, malloc, strdup
//TODO content-type abscent => txt
//TODO const char* strRegex, const inutile chaine non mutable
void processContentType(pop*p, char*search, char**mime, char**canonical, char**boundary)
{
	const char* strRegex ="^Content-Type: ([-0-9A-Za-z\\/\\+\\.]+)(.*boundary=\"(.*)\")?";
	const int NB_PAR = 3+1;
	
	regex_t preg;
	regmatch_t matches[NB_PAR];
	
	int isMultipart;
	char* startMime, *startBoudary;
	int sizeMime, sizeBoundary;
	
	isMultipart = 0;
	regcomp (&preg, strRegex, REG_EXTENDED);
	
	*mime = NULL;
	*canonical=NULL;
	*boundary=NULL;
	
	if(!regexec(&preg, search, NB_PAR, matches, 0))
	{
		startMime = search+matches[1].rm_so;
		sizeMime = matches[1].rm_eo - matches[1].rm_so;
		
		*mime=strndup(startMime, sizeMime);
		
		if(!strncmp(*mime, MIME_MULT, strlen(MIME_MULT)))
		{
			startBoudary = search+matches[3].rm_so;
			sizeBoundary = matches[3].rm_eo - matches[3].rm_so;
			*boundary = strndup(startBoudary, sizeBoundary);
		}
		else{
			*canonical = getCanonical(p->first_mime, *mime);
			if(!(*canonical))
				*canonical = strdup(DEFAULT_EXT);
		}		
	}
	else{
		fatal("Type invalide");
	}

	regfree(&preg);
}

int main(int argc, char **argv)
{
	int so;
	FILE *fSo;
	pop p;
	int quit;
	char saisie[MAXLINE];
	char param[MAXLINE];

	p.last_mime=NULL;
	p.first_mime=NULL;
	quit = 0;
	
	initMimes(fdopen(open("/etc/mime.types", O_RDONLY), "r"),&p);

	// so = InitConnexion(argv[1],argv[2]);
	so = open("./obj/scenario1.pop", O_RDONLY);
	fSo = fdopen(so, "r");
	//processMail(&p,fSo,"1");

	for(;;)
	{
		memset(saisie,0,MAXLINE);
		memset(param,0,MAXLINE);
		printf("ol> ");
		fgets (saisie, MAXLINE , stdin);

		if(!strncasecmp(saisie, CMD_USER, strlen(CMD_USER))){
			sscanf(saisie,"user %s",&param);
			printf("User : %s\n",param);
			txtUser(fSo, param);
		}
		if(!strncasecmp(saisie, CMD_PASS, strlen(CMD_PASS))){
			sscanf(saisie,CMD_PASS" %s",param);
			printf("Pass : %s\n",param);
			txtPass(fSo, param);
		}
		if(!strncasecmp(saisie, CMD_LIST, strlen(CMD_LIST))){
			printf("List\n");
			txtList(fSo);
		}
		if(!strncasecmp(saisie, CMD_TOP, strlen(CMD_TOP))){
			sscanf(saisie,CMD_TOP" %s, 0",param);
			printf("Top : %s\n",param);
			txtTop(fSo,param);
		}
		if(!strncasecmp(saisie, CMD_QUIT, strlen(CMD_QUIT))){
			printf("Quit\n");
			txtQuit(fSo);
		}
		if(!strncasecmp(saisie, CMD_RETR, strlen(CMD_RETR))){
			sscanf(saisie,CMD_RETR" %s",param);
			printf("Retr : %s\n",param);
			txtRetr(&p,fSo,param);
		}
	}

	//txtQuit(fSo);

	//printf("%d\n", atoi("0"));
	exit(EXIT_SUCCESS);
}
