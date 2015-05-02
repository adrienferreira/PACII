#define KEY_BUFF 32

#define TOT_WIN 6
#define NBR_MAX_OW 10

#define W_MAIN 0 
#define W_USER 1 
#define W_PASS 2 
#define W_VALI 3
#define W_ERRO 4
#define W_QUIT 5

#define F_NAME "POP-ACII"
#define TXT_CONF "Valider"

#define MSG_EMPT_USR "Login vide."
#define MSG_EMPT_PASS "MDP vide."

#define MSG_WRONG_USR "Login reffusé."
#define MSG_WRONG_PASS "MDP reffusé."

#define ITEM_HEIGHT 25

#define OW_HEIGHT 25
#define OW_OFFSET 5

#define OW_ID_WIDTH 50

#define OW_BUTTON_WIDTH 80

#define MAINW_WIDTH 800
#define MAINW_HEIGHT (OW_HEIGHT + OW_OFFSET + 1) * NBR_MAX_OW

#define COLOR_DOWL  "#00FF00"
#define COLOR_HOVER "#D3D3D3"
#define COLOR_BASE  "#FFFFFF"

int focus;
char strUser[MAXLINE];
char strPass[MAXLINE];

Display *dis;
Window wins[TOT_WIN];
GC popGC;


void putStringWindow(Window *w, char*str);
int checkUserPassStr(char*user,char*pass);
void clickUserPass(pop*p,FILE*fSo,char*user,char*pass);
void linkMailEnd(pop*p,mails* newM);
void clicList(pop*p,FILE*fSo);
void fButtonPress(pop*p,FILE*fSo,XButtonEvent * e);
int isCharUserOk(char c);
void fKeyPress(pop*p,FILE*fSo,XKeyEvent * e);
void getOverviewInfo(FILE*fSo,char**from,char**date);
void drawOverview(pop*p);
void emptyUserPassBoxes(void);
void changeWindowBgColor(Window*w,char*colorName);
void fEnter(XEnterWindowEvent *e, pop*p);
void fLeave(XLeaveWindowEvent *e, pop*p);
