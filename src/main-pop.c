#include <common-pop.h>

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

	fwrite(CLRF, sizeof(char), strlen(CLRF), fSo);
	#endif
}

void sendUser(FILE * fSo, char *name){
	sendCmd(fSo, "USER", name, NULL);
}

void sendPass(FILE * fSo, char *pass){
	sendCmd(fSo, "PASS", pass, NULL);
}

void sendList(FILE * fSo){
	sendCmd(fSo, "LIST", NULL, NULL);
}

void sendQuit(FILE * fSo){
	sendCmd(fSo, "QUIT", NULL, NULL);
}

void sendRetr(FILE * fSo, char *mailNbr)
{
	if(!atoi(mailNbr))
		fatal("Numéro de mail invalide");
	sendCmd(fSo, "RETR", mailNbr, NULL);
}

void sendTop(FILE * fSo, char *mailNbr)
{
	if(!atoi(mailNbr))
		fatal("Numéro de mail invalide");
	sendCmd(fSo, "TOP", mailNbr, "0");
}

void saveSimpleContent(FILE*fSo, char*mailNbr,char* canonical)
{
	char buff[MAXLINE];
	char *fileName;
	FILE *dest;

	fileName = (char*)malloc(strlen(mailNbr)+strlen(canonical)+1+1);
	sprintf(fileName,"%s.%s",mailNbr,canonical);
	dest=fopen(fileName, "w+");

	while(fgets(buff, MAXLINE, fSo) && !strncmp(buff,END,strlen(END)))
		fwrite(buff,sizeof(char),MAXLINE,dest);
	
	free(fileName);
}

//TODO content-type abscent => txt
void saveMixedContent(FILE*fSo, char*mailNbr, char*boundary)
{
	char buff[MAXLINE];
	char *fileName;
	FILE *dest;
	
	char *mime, *canonical, *unused;

	mkdir(mailNbr, S_IRUSR | S_IWUSR);

	fileName = (char*)malloc(strlen(mailNbr)+strlen(canonical)+1+1);
	sprintf(fileName,"%s.%s",mailNbr,canonical);
	dest=fopen(fileName, "w+");

	while(fgets(buff, MAXLINE, fSo) && !strncmp(buff,END,strlen(END)))
	{
		if(strncmp(buff,"--",2) && strncmp(buff+2,boundary,strlen(boundary))){
			fgets(buff, MAXLINE, fSo) && !strncmp(buff,END,strlen(END));
			if(strncmp(buff,H_CONTTYPE,strlen(H_CONTTYPE))){
				
			}
		}
	}
	
	free(fileName);
	
/*
	 if(mkdir(, mode_t mode))
	 	fatal("Création dossier");
*/
}

//TODO free regex, malloc, strdup
//TODO content-type abscent => txt
//TODO const char* strRegex, const inutile chaine non mutable
void processContentType(pop*p, char*search, char**mime, char**canonical, char**boundary)
{
	const char* strRegex ="^Content-Type: ([-0-9A-Za-z\\/\\+\\.]+);(.*boundary=\"(.*)\")?";
	const int NB_PAR = 3+1;
	
	regex_t preg;
	regmatch_t matches[NB_PAR];
	
	char* startMime, *startBoudary;
	int sizeMime, sizeBoundary;
	
	regcomp (&preg, strRegex, REG_EXTENDED);
	
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

	p.last_mime=NULL;
	p.first_mime=NULL;
	
	initMimes(fdopen(open("/etc/mime.types", O_RDONLY), "r"),&p);
	printf("%s\n",getCanonical(p.first_mime,"image/png"));

	// so = InitConnexion(argv[1],argv[2]);
	so = open("scenario.pop", O_RDONLY);
	fSo = fdopen(so, "r");
	txtRetr(&p,fSo,"1");

	//txtQuit(fSo);

	//printf("%d\n", atoi("0"));
	exit(EXIT_SUCCESS);
}
