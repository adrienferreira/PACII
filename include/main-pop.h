
extern int isServerOk(FILE * fSo);
extern void sendCmd(FILE * fSo, char *cmd, char *param1, char *param2);
extern void sendUser(FILE * fSo, char *name);
extern void sendPass(FILE * fSo, char *pass);
extern void sendList(FILE * fSo);
extern void sendQuit(FILE * fSo);
extern void sendRetr(FILE * fSo, char *mailNbr);
extern void sendTop(FILE * fSo, char *mailNbr);
extern void processContentType(pop*p, char*search, char**mime, char**canonical, char**boundary);

extern void saveSimpleContent(FILE*fSo, char*folderName, char*fileName,char* canonical);
extern void processMail(pop*p, FILE*fSo, char*mailNbr);
