#include <common-pop.h>


void txtUser(FILE * fSo, char* name){
	int ret;

	sendUser(fSo,name);
	ret = isServerOk(fSo);

	if(!ret)fatal("USER échoue");
}

void txtPass(FILE * fSo, char* pass){
	int ret;

	sendPass(fSo,pass);
	ret = isServerOk(fSo);

	if(!ret)fatal("PASS échoue");
}

void txtTop(FILE*fSo,char*mailNbr)
{
	int ret;
	char buff[MAXLINE];
	int cont;

	sendTop(fSo, mailNbr);
	ret = isServerOk(fSo);

	if(!ret)fatal("TOP échoue");

	do{
		fgets(buff, MAXLINE, fSo);
		printf("%s\n",buff);
		cont = strncmp(buff,END,strlen(END));
	}
	while(cont);
}

void txtList(FILE*fSo)
{
	int ret;
	char buff[MAXLINE];
	int endOfHeader;

	sendList(fSo);
	ret = isServerOk(fSo);

	if(!ret)fatal("LIST échoue");

	do{
		fgets(buff, MAXLINE, fSo);
		printf("%s\n",buff);
		endOfHeader = !strncmp(buff,END,strlen(END));
	}
	while(endOfHeader);
}

void txtQuit(FILE*fSo)
{
	int ret;

	sendQuit(fSo);
	ret = isServerOk(fSo);

	exit(ret?EXIT_SUCCESS:EXIT_FAILURE);
}

void txtRetr(pop*p,FILE*fSo,char*mailNbr){}
