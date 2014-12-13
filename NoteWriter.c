#include <os.h>
#include <SDL\SDL.h>
#include <Winbox.h>
#include <SDL/SDL_rotozoom.h>

#include "SmallFont.h"
#include "Images.h"


#define NFONTS 4
enum FONT_TYPE {
	FONT_NORMAL=0,
	FONT_SMALL,
	FONT_VERY_SMALL,
	FONT_BOLD,
};

int changeOSscreen;

BOOL mainWindow(int argc, char *argv[]);
char *OpenBox(char *cpath);
char *SaveBox(char *fstr);
int AddFileToWindow(Widget ***wFiles, char ***files, int **isSaved, int *nFiles, const char *file);
void fsave(const char *file, Widget *w);
wTHEME *NoteWriterClassicTheme();
wTHEME *NoteWriterDarkTheme();
void HelpBox();
void CreditsBox();
int OptionsBox(int *th, int *rfOS, int *font_size, int *mono, char *path_python);  // theme, refreshOS?, font size, path python
void SetHighlightSyntax(Widget *w, char *fileName, nSDL_Font *f, int font);




void MSG(char *msg)
{
	SDL_Surface *scr = SDL_GetVideoSurface();
	int W = 140, H = 18;
	DrawFillRectXY(scr, 160-W, 120-H, 2*W, 2*H, RGB(219,215,232));
	DrawRectXY(scr, 160-W+2, 120-H+2, 2*W-4, 2*H-4, RGB(43,45,57));
	nSDL_Font *font = nSDL_LoadFont(NSDL_FONT_TINYTYPE, 35,0,0);
	nSDL_DrawString(scr, font, 160-nSDL_GetStringWidth(font, msg)/2, 120-nSDL_GetStringHeight(font, msg)/2, msg);
	SDL_Flip(scr);
	nSDL_FreeFont(font);
}






int main(int argc, char *argv[])
{
	SDL_Surface *scr = wInitSDL();
	DrawFillRectXY(scr, 0,0,16,16, 0);
	
	mainWindow(argc, argv);
	
	wCloseTheme();
	SDL_Quit();
	return 1;
}



BOOL mainWindow(int argc, char *argv[])
{
	// Déclaration des variables
	static unsigned short image_NoteWriter[] = {
    0x2a01,0x000B,0x000B,0x0000,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,
    0xf800,0xf800,0x8802,0x8802,0xf800,0xf800,0xf800,0xf800,0xf800,0xf800,
    0xf800,0xf800,0x8802,0xf3d0,0x8802,0x8802,0xf800,0xf800,0xf800,0xf800,
    0xf800,0xf800,0x8802,0xf30c,0xf30c,0xf3d0,0x8802,0xf800,0xf800,0xf800,
    0xf800,0xf800,0x8802,0xf30c,0xf30c,0xf30c,0x8802,0xf800,0xf800,0xf800,
    0xf800,0xf800,0x8802,0xe9c8,0xe9c8,0xf30c,0x8802,0xf800,0xf800,0xf800,
    0xf800,0xf800,0x8802,0xe9c8,0xe9c8,0xe9c8,0x8802,0xf800,0xf800,0xf800,
    0xf800,0xf800,0x8802,0xe8e4,0xe8e4,0xe9c8,0x8802,0xf800,0xf800,0xf800,
    0xf800,0xf800,0x8802,0xe8e4,0xe8e4,0xe8e4,0x8802,0xf800,0xf800,0xf800,
    0xf800,0xf800,0x8802,0xef5d,0x8802,0xe8e4,0x8802,0xf800,0xf800,0xf800,
    0xf800,0xf800,0xf800,0x4020,0xef5d,0xef5d,0x8802,0xf800,0xf800,0xf800,
    0xf800,0xf800,0xf800,0xf800,0x0000,0x4020,0x8802,0xf800,0xf800,0xf800,
    0xf800,0xf800,0xf800,0xf800,0xf800
  };
	int nFiles = 0;
	int nfiles = 0;  // only used when opening cfg file
	int x, y;
	char name[32];
	int ok;
	Widget **wFiles = NULL;
	char **files = NULL;
	int *isSaved = NULL;
	FILE *cfg = NULL;
	int cTab=0;
	int yRow=0;
	int cRow=0;
	int cChar=0;
	wMENU *wMenu = wMenu_AddItemList(NULL, "New|Open...|Save|Save as...|Help|Options|Credits|Exit");
	int th=0;  // theme
	int rfOS=0;  // refreshOS
	int font_size=0;  // 0=normal, 1 = bold, 0=small
	char path_python[512]= "/documents/ndless/micropython.tns";
	char fname[32];
	changeOSscreen = 0;
	char msg[512];
	int monospace = 0;
	nSDL_Font *font[2*NFONTS] = {nSDL_LoadFont(NSDL_FONT_THIN, 20, 20, 20), // theme 1
										  nSDL_LoadFont(NSDL_FONT_TINYTYPE, 20, 20, 20),
										  nSDL_LoadCustomFont(smallFontChars, 20, 20, 20),
										  nSDL_LoadFont(NSDL_FONT_VGA, 20, 20, 20),
										  
										  nSDL_LoadFont(NSDL_FONT_THIN, 247, 247, 247),  // theme 2
										  nSDL_LoadFont(NSDL_FONT_TINYTYPE, 247, 247, 247),
										  nSDL_LoadCustomFont(smallFontChars, 247, 247, 247),
										  nSDL_LoadFont(NSDL_FONT_VGA, 247, 247, 247)
										 };
	font[FONT_VERY_SMALL]->hspacing = 1;
	font[FONT_VERY_SMALL+NFONTS]->hspacing = 1;
	font[FONT_BOLD]->hspacing = 1;
	font[FONT_BOLD+NFONTS]->hspacing = 1;
	
	
	// on ouvre et lit le fichier de configuration
	cfg = fopen("/notewriter.cfg", "rb");
	if (cfg) {
		int l;
		char path[512];
		fread(&th, sizeof(int), 1, cfg);
		fread(&rfOS, sizeof(int), 1, cfg);
		fread(&font_size, sizeof(int), 1, cfg);
		fread(&monospace, sizeof(int), 1, cfg);
		fread(&l, sizeof(int), 1, cfg);
		fread(path_python, l+1, 1, cfg);
		fread(&cTab, sizeof(int), 1, cfg);
		fread(&yRow, sizeof(int), 1, cfg);
		fread(&cRow, sizeof(int), 1, cfg);
		fread(&cChar, sizeof(int), 1, cfg);
		fread(&nfiles, sizeof(int), 1, cfg);
		
		// on lit à présent les noms de chacun des fichiers
		for (x=0; x < nfiles; x++) {
			fread(&l, sizeof(int), 1, cfg);
			if (l) {
				fread(path, l+1, 1, cfg);
				ok = AddFileToWindow(&wFiles, &files, &isSaved, &nFiles, path);
				if (ok==0) wFastMsg("ERROR : Insuffisant memory\nWhile loading file :\n%s", path);
				else if (ok == -1) wFastMsg("ERROR : Text too large\nWhile loading file :\n%s", path);
				else if (ok == -2) wFastMsg("ERROR\nCan't find the following file :\n%s", path);
			}
		}
		
		fclose(cfg);
	}
	wInitTheme(th? NoteWriterDarkTheme():NoteWriterClassicTheme());
	for (x=0; x<2*NFONTS; x++) nSDL_EnableFontMonospaced(font[x], monospace);
	
	
	// on ouvre les fichier appelés
	for (x=1; x < argc; x++) {
		char *p;
		p = argv[x];
		if (p[1] == '/') p++;
		
		// on vérifie en premier lieu que le fichier n'est pas déjà ouvert
		for (y=0; y < nFiles; y++) {
			if (files[y] && !strcmp(files[y], p)) break;
		}
			
		if (y < nFiles) {  // alors le fichier est déjà ouvert
			if (cTab != y) {  // on l'active si ce n'est déjà fait
				cTab = y;
				cRow=0, yRow=0, cChar=0;
			}
			continue;
		}
		
		// sinon, on l'ajoute
		ok = AddFileToWindow(&wFiles, &files, &isSaved, &nFiles, p);
		if (ok==0) wFastMsg("ERROR : Insuffisant memory\nWhile loading file :\n%s", p);
		else if (ok == -1) wFastMsg("ERROR : Text too large\nWhile loading file :\n%s", p);
		else if (ok == -2) wFastMsg("ERROR\nCan't find the following file :\n%s", p);
		else {
			cTab = nfiles;
			cRow=0, yRow=0, cChar=0;
		}
	}
	
	
	// Déclaration des widgets
	Widget *mainWidget = wExWindow("NoteWriter 2.0", image_NoteWriter, wMenu);
	Widget *tabs = wTab();
		
	// Ajout des Widgets
	wAddWidget(mainWidget, tabs);
		
	for (x=0; x < nFiles; x++) {
		// on modifie le titre de l'onglet
		y = strlen(files[x])-1;
		while ((files[x])[y] != '/') y--;
		strcpy(name, files[x]+y+1);
		if (!strcmp(name+strlen(name)-4, ".tns"))
			name[strlen(name)-4] = 0;  // alors on enlève le .tns
		
		wTab_AddExTab(tabs, name, wFiles[x], 1);
		SetHighlightSyntax(wFiles[x], name, font[NFONTS*th+font_size], font_size);
		wText_SetFont(wFiles[x], font[NFONTS*th+font_size]);
		if (x == cTab) {  // c'est le dernier fichier ouvert ; on revient là où on en était
			wSetActiveWidget(wFiles[x]);
			TextArgs *args = wFiles[x]->args;
			args->cRow = cRow;
			args->yRow = yRow;
			args->cChar = cChar;
		}
	}
	
	
	
	// Création des fonctions callback
	int CB_ChangeText(Widget *w, int signal)
	{
		if (signal != SIGNAL_ACTION || w->type != WIDGET_TEXT) return ACTION_CONTINUE;
		int n = wTab_GetCurrentTab(tabs);
		if (isSaved[n]) {
			isSaved[n] = 0;
			
			// on modifie l'affichage du tab (on rajoute un tilde)
			TabLayoutArgs *args = tabs->args;
			strcat(args->tabs[n].text, "~");
			wDrawWidget(tabs);
		}
		
		return ACTION_CONTINUE;
	}
	
	
	int CB_Tabs(Widget *w, int signal)
	{
		if (signal == SIGNAL_ACTION2) {
			// un onglet a été supprimé : args->hWidget
			TabLayoutArgs *args = w->args;
			
			if (args->hWidget == -1) {
				wFastMsg("ERROR\nargs->hWidget = -1");
				return ACTION_CONTINUE;
			}
			
			if (!isSaved[args->hWidget]) {
				if (files[args->hWidget]) {  // on propose de sauvegarder les modifications
					char msg[128];
					sprintf(msg, "Save modifications to document \'%s\' ?", files[args->hWidget]);
					if (wInputMsg("Save", msg)) {
						fsave(files[args->hWidget], wFiles[args->hWidget]);
						isSaved[args->hWidget] = 1;
					}
				}
			}

			// on actualise nFiles, wFiles, files et isSaved
			nFiles--;
			for (x=args->hWidget; x < nFiles; x++) {
				wFiles[x]	= wFiles[x+1];
				files[x]		= files[x+1];
				isSaved[x]	= isSaved[x+1];
			}
		}
		
		else if (signal == SIGNAL_KEY) {
			if (K_DOC()) {  // quicksave
				
				if (K_SHIFT()) {  // auicksave all files
					TabLayoutArgs *args = tabs->args;
					char *t;
					
					for (x=0; x < nFiles; x++) {
						if (files[x] && !isSaved[x]) {
							fsave(files[x], wFiles[x]);
							isSaved[x] = 1;
							t = args->tabs[x].text;
							t[strlen(t)-1] = 0;
						}
					}
				}
				
				else {  // quicksave current file
					x = wTab_GetCurrentTab(tabs);
					TabLayoutArgs *args = tabs->args;
					if (files[x] && !isSaved[x]) {
						fsave(files[x], wFiles[x]);
						isSaved[x] = 1;
						char *t = args->tabs[x].text;
						t[strlen(t)-1] = 0;
					}
				}
				
				return ACTION_REFRESH;
			}
		}
		
		return ACTION_CONTINUE;
	}
	
	
	int CB_Menu(Widget *w, int signal)
	{
		if (!w) return ACTION_CONTINUE;
		int n = wMenu_GetSelectedItem(wMenu);
		wMenu_DeselectAll(wMenu);
		TabLayoutArgs *args = tabs->args;
		
		
		if (signal == SIGNAL_KEY && K_ENTER()) {
			if (!nFiles) return ACTION_CONTINUE;
			int t = wTab_GetCurrentTab(tabs);
			
			if (!files[t] || !strcmp(files[t]+strlen(files[t])-3, ".py") || !strcmp(files[t]+strlen(files[t])-7, ".py.tns")) {
				// alors on a affaire à un fichier python ; on doit l'exécuter
				if (!isFile(path_python)) {
					wFastMsg("ERROR\nThe path for Micropython is wrong. Change it in the options.");
					return ACTION_CONTINUE;
				}
				
				// on enregistre tout d'abord le programme
				fsave("/documents/tmp.py", wFiles[t]);
				
				// puis on l'exécute
				MSG("Executing...");
				char *args[] = {"/documents/tmp.py"};
				nl_exec(path_python, 1, args);
				
				// et enfin on supprime le fichier temporaire
				remove("/documents/tmp.py");
				return ACTION_REFRESH;
			}
			
			else wFastMsg("%s\nThis is not a .py file", files[t]);
		}
		
		
		if (signal != SIGNAL_CLICK) return ACTION_CONTINUE;
		
		if (n==0) { // Nouveau
			Widget *new = wText(NULL, 20);
			void *tmp;
			
			if (new) {
				tmp = realloc(wFiles, (nFiles+1) * sizeof(Widget *));
				if (!tmp) goto NEW_ERROR;
				wFiles = tmp;
				tmp = realloc(files, (nFiles+1) * sizeof(char *));
				if (!tmp) goto NEW_ERROR;
				files = tmp;
				tmp = realloc(isSaved, (nFiles+1) * sizeof(int));
				if (!tmp) goto NEW_ERROR;
				isSaved = tmp;
				
				wText_SetFont(new, font[NFONTS*th+font_size]);
				wTab_AddExTab(tabs, "New", new, 1);
				wFiles[nFiles] = new;
				wConnect(new, CB_ChangeText);
				wSetActiveWidget(new);
				files[nFiles] = NULL;
				isSaved[nFiles] = 1;
				nFiles++;
				wDrawWidget(mainWidget);
			}
			else {
			  NEW_ERROR:
				wFastMsg("ERRoR\nInsuffisant memory");
			}
		}
		
		else if (n==1) {  // Ouvrir
			char *file = OpenBox(nFiles? files[wTab_GetCurrentTab(tabs)]:NULL);
			if (!file[0]) return ACTION_CONTINUE;
			
			// on vérifie tout d'abord que le fichier n'est pas déjà ouvert
			for (x=0; x < nFiles; x++)
				if (files[x] && !strcmp(files[x], file)) break;
			
			if (x < nFiles) {
				// alors x est le numéro du fichier en cours d'utilisation
				wSetActiveWidget(wFiles[x]);
				wDrawWidget(mainWidget);
				return ACTION_CONTINUE;
			}
			
			ok = AddFileToWindow(&wFiles, &files, &isSaved, &nFiles, file);
			if (ok == 1) {
				x = nFiles-1;
				
				// on modifie le titre de l'onglet
				int t = strlen(files[x])-1;
				while ((files[x])[t] != '/') t--;
				strcpy(fname, files[x]+t+1);
				if (!strcmp(fname+strlen(fname)-4, ".tns"))
					fname[strlen(fname)-4] = 0;  // alors on enlève le .tns
				
				wTab_AddExTab(tabs, fname, wFiles[x], 1);
				SetHighlightSyntax(wFiles[x], fname, font[NFONTS*th+font_size], font_size);
				wText_SetFont(wFiles[x], font[NFONTS*th+font_size]);
				wConnect(wFiles[x], CB_ChangeText);
				wSetActiveWidget(wFiles[x]);
				wDrawWidget(mainWidget);
			}
			else if (ok == -2) wFastMsg("ERROR\nFailed to open the following file :\n%s", file);
			else if (ok == -1) wFastMsg("ERROR\nFile too large\%s", file);
			else if (ok == 0) wFastMsg("ERROR\nInsuffisant memory");
		}
		
		else if (n==2) {  // Sauvegarder
			if (!args->nWidgets) return ACTION_CONTINUE;
			Widget *w = wTab_GetCurrentWidget(tabs);
			
			for (x=0; x < nFiles; x++) if (w == wFiles[x]) break;
			if (x == nFiles) goto SAVE_AS;
			if (!files[x]) goto SAVE_AS;
			
			if (!isSaved[x]) {
				fsave(files[x], w);
				isSaved[x] = 1;
				TabLayoutArgs *args = tabs->args;
				char *t = args->tabs[x].text;
				t[strlen(t)-1] = 0;
				wDrawWidget(tabs);
			}
		}
		
		else if (n==3) {  // Sauvegarder sous...
			if (!args->nWidgets) return ACTION_CONTINUE;
		  SAVE_AS:;
			char *file = SaveBox(files[wTab_GetCurrentTab(tabs)]);
			
			if (file[0]) {
				if (isFile(file)) {
					sprintf(msg, "This file : \'%s\' already exists. Do you want to replace it ?", file);
					if (!wInputMsg("Save", msg)) return ACTION_CONTINUE;
				}
				Widget *w = wTab_GetCurrentWidget(tabs);
				fsave(file, w);
				
				// on modifie le titre de l'onglet
				int t = strlen(file)-1;
				while (file[t] != '/') t--;
				strcpy(fname, file+t+1);
				if (!strcmp(fname+strlen(fname)-4, ".tns"))
					fname[strlen(fname)-4] = 0;  // alors on enlève le .tns
				wTab_SetTabTitle(tabs, wTab_GetCurrentTab(tabs), fname);
				
				// on change la syntaxe, si changement d'extension
				SetHighlightSyntax(w, fname, font[NFONTS*th+font_size], font_size);
				
				// on modifie files[x]
				for (x=0; x < nFiles; x++) if (w == wFiles[x]) break;
				if (x < nFiles) {
					files[x] = realloc(files[x], strlen(file)+1);
					strcpy(files[x], file);
				}
				
				// on modifie isSaved
				isSaved[x] = 1;
				
				wDrawWidget(mainWidget);
			}
		}
		
		else if (n==4) {  // Aide
			HelpBox();
		}
		
		else if (n==5) {  // Options
			int ok = OptionsBox(&th, &rfOS, &font_size, &monospace, path_python);
			
			if (ok) {
				if (ok == 2) wConstruct_ChangeTheme(mainWidget, th? NoteWriterDarkTheme():NoteWriterClassicTheme());
				
				for (x=0; x<2*NFONTS; x++) nSDL_EnableFontMonospaced(font[x], monospace);
				for (x=0; x < nFiles; x++) SetHighlightSyntax(wFiles[x], files[x], font[NFONTS*th+font_size], font_size);
				for (x=0; x < nFiles; x++) wText_SetFont(wFiles[x], font[NFONTS*th+font_size]);
				
				return ACTION_REFRESH;
			}
		}
		
		else if (n==6) { // Credits
			CreditsBox();
		}
		
		else if (n==7) {  // quitter
			return ACTION_EXIT;
		}
		
		return ACTION_CONTINUE;
	}
	
	
	// Connection
	for (x=0; x < nFiles; x++)
		wConnect(wFiles[x], CB_ChangeText);
	wConnect(mainWidget, CB_Menu);
	wConnect(tabs, CB_Tabs);
	
	// Exécution
  ACTIVATE:
	wActivateConstruct(mainWidget);
	
	
	// on propose de sauvegarder les fichiers
	ok = 0;
	for (x=0; x < nFiles; x++) {
		if (!isSaved[x]) {
			
			if (files[x]) {  // on propose de sauvegarder les modifications
				sprintf(msg, "Save modifications to document \'%s\' ?", files[x]);
				if (wInputMsg("Save", msg)) {
					fsave(files[x], wFiles[x]);
					isSaved[x] = 1;
				}
			}
			
			else if (!ok) {  // on ne fait apparaître ce message qu'une seule fois
				ok = wInputMsg("Warning", "One or more files \'new\' are not saved. Do you really want to quit ? Their datas will be lost.");
				if (!ok) goto ACTIVATE;
			}
			
		}
	}
	
	// on actualise le fichier contenant les options et noms de tous les fichiers en cours d'édition
	cfg = fopen("/notewriter.cfg", "wb");
	if (cfg) {
		int l = strlen(path_python);
		int n = wTab_GetCurrentTab(tabs);
		int x;
		
		if (!nFiles || !isSaved[n]) {
			yRow = 0;
			cRow = 0;
			cChar = 0;
		}
		else {
			TextArgs *args = wTab_GetCurrentWidget(tabs)->args;
			yRow = args->yRow;
			cRow = args->cRow;
			cChar = args->cChar;
		}
		
		fwrite(&th, sizeof(int), 1, cfg);
		fwrite(&rfOS, sizeof(int), 1, cfg);
		fwrite(&font_size, sizeof(int), 1, cfg);
		fwrite(&monospace, sizeof(int), 1, cfg);
		fwrite(&l, sizeof(int), 1, cfg);
		fwrite(path_python, l+1, 1, cfg);
		fwrite(&n, sizeof(int), 1, cfg);
		fwrite(&yRow, sizeof(int), 1, cfg);
		fwrite(&cRow, sizeof(int), 1, cfg);
		fwrite(&cChar, sizeof(int), 1, cfg);
		fwrite(&nFiles, sizeof(int), 1, cfg);
		// on écrit à présent les noms de chacun des fichiers
		for (x=0; x < nFiles; x++) {
			if (files[x]) {
				l = strlen(files[x]);
				fwrite(&l, sizeof(int), 1, cfg);
				fwrite(files[x], l+1, 1, cfg);
			}
			else {
				l = 0;
				fwrite(&l, sizeof(int), 1, cfg);
			}
		}
		
		fclose(cfg);
	}
	else wFastMsg("ERROR\nCan't open the following file :\n\'\\notewriter.cfg\'");
	
	
	// on libère la mémoire
	if (rfOS && changeOSscreen) {
		MSG("Refreshing OS document browser screen...");
		refresh_osscr();
	}
	
	// on vérifie que les quatre polices ouvertes vont bien être fermées
	wCONSTRUCT *c = mainWidget->construct;
	for (x=0; x < 2*NFONTS; x++) wAddFontToConstruct(c, font[x]);
// wFastMsg("CONSTRUCT ITEMS\nWidgets : %i\nFonts : %i\nSurfaces : %i\nOthers : %i", c->nWidgets, c->nFonts, c->nSurfaces, c->nElts);
	
	wCloseConstruct(mainWidget);
	for (x=0; x < nFiles; x++) free(files[x]);
	return 1;
}




int AddFileToWindow(Widget ***wFiles, char ***files, int **isSaved, int *nFiles, const char *file)
{
	int n = *nFiles;
	void *tmp;
	FILE *f;
	char *text;
	int size = 1024;
	int x=0;
	int c;
	
	
	tmp = realloc(*wFiles, (n+1) * sizeof(Widget *));
	if (!tmp) return 0;
	*wFiles = tmp;
	tmp = realloc(*files, (n+1) * sizeof(char *));
	if (!tmp) return 0;
	*files = tmp;
	tmp = realloc(*isSaved, (n+1) * sizeof(int));
	if (!tmp) return 0;
	*isSaved = tmp;
	
	
	f = fopen(file, "r");
	if (!f) return -2;
	text = malloc(1024);
	if (!text) return 0;
	
	c = fgetc(f);
	while (c != EOF) {
		if (x >= size-1) {
			size += 1024;
			tmp = realloc(text, size);
			if (!tmp) {
				free(text);
				fclose(f);
				return -1;
			}
			text = tmp;
		}
		text[x++] = (c==10? '\25':(char) c);
		c = fgetc(f);
	}
	
	text[x] = 0;
		
	
	(*nFiles)++;
	(*wFiles)[n] = wText(text, 20);
	(*files)[n] = malloc(strlen(file));
	strcpy((*files)[n], file);
	(*isSaved)[n] = 1;
	
	
	free(text);
	fclose(f);
	return 1;
}





char *OpenBox(char *cpath)
{
	static char file[512] = "";
	char tmpFile[512] = "";
	char path[512];
	char *p;
	file[0] = 0;
	char c;
	
	if (cpath) {
		p = strrchr(cpath, '/');
		if (p) {
			c=*p, *p=0;
			strcpy(tmpFile, p+1);
			strcpy(path, cpath);
			*p=c;
		}
		else {
			strcpy(tmpFile, cpath);
			strcpy(path, "/");
		}
	}
	
	else strcpy(path, "/documents");
	
	
	int ok = wOpenBox(path, NULL, tmpFile);
	if (ok) strcpy(file, tmpFile);
	
	return file;
}


char *SaveBox(char *fullPath)
{
	static char file[512] = "";
	char tmpFile[512] = "";
	char path[512];
	char *p;
	file[0] = 0;
	char c;
	
	if (fullPath) {
		p = strrchr(fullPath, '/');
		if (p) {
			c=*p, *p=0;
			strcpy(tmpFile, p+1);
			strcpy(path, fullPath);
			*p=c;
		}
		else {
			strcpy(tmpFile, fullPath);
			strcpy(path, "/");
		}
	}
	
	else strcpy(path, "/documents");
	
	
	int ok = wSaveBox(path, tmpFile);
	if (ok) strcpy(file, tmpFile);
	
	return file;
}



void fsave(const char *file, Widget *w)
{
	if (!isFile(file)) changeOSscreen = 1;
	FILE *f = fopen(file, "w");
	char *text = wText_GetText(w);
	int x=0;
	char c;
	
	while ((c = text[x++])) fputc(c=='\25'? '\n':c, f);
	
	fclose(f);
}


void HelpBox()
{	
	// Déclaration des widgets
	Widget *mainWidget = wWindow("Help");
	Widget *tabs = wTab();
	
	// 1er onglet
	Widget *body1 = wBasicLayout(0);
	Widget *subody1 = wHorizontalLayout(0);
	Widget *im_tbtton = wPixmapNTI(image_buttonScratchpad);
	Widget *navigation = wText("Use the T-Button to navigate between tabs.\n\nIf you maintain this button while using RIGHT or LEFT buttons, you will be able to select your tab.\n\nMaintaining the T-Button plus pushing DEL will delete the selected tab from your work space.\n\nTo navigate faster in your texts, use CTRL+Directional Arrow. To delete faster, use CTRL+DEL.", 15);
	
	//2e onglet
	Widget *body2 = wBasicLayout(0);
	Widget *keys = wText("DOC - Quicksave the current file\nSHIFT+Directional Arrow - Select text\nCTRL+VAR - Copy selection\nVAR - Paste selection", 5);
	Widget *keyTable = wExTable(3, 10, ALIGN_CENTER, nSDL_LoadFont(NSDL_FONT_THIN, 12,12,12), NULL);
	
	wTable_AddExString(keyTable, "Character", ALIGN_CENTER, nSDL_LoadFont(NSDL_FONT_VGA, 200,200,200), wBackgroundColor(RGB(0,0,0), RGB(100,100,100), BACKG_HGRAD));
	wTable_AddExString(keyTable, "+CTRL", ALIGN_CENTER, nSDL_LoadFont(NSDL_FONT_VGA, 200,200,200), wBackgroundColor(RGB(10,10,50), RGB(80,80,140), BACKG_HGRAD));
	wTable_AddExString(keyTable, "+SHIFT", ALIGN_CENTER, nSDL_LoadFont(NSDL_FONT_VGA, 200,200,200), wBackgroundColor(RGB(10,50,10), RGB(80,140,80), BACKG_HGRAD));
	
	int x;
	char charList[10] = ",0.\372123()";
	char str[3*9][2] = {};
	for (x=0; x < 9; x++) {
		str[3*x][0] = charList[x];
		wTable_AddString(keyTable, str[3*x]);
		
		str[3*x+1][0] = enctrl(NULL, charList[x]);
		wTable_AddString(keyTable, str[3*x+1]);
		
		str[3*x+2][0] = enshift(charList[x]);
		wTable_AddString(keyTable, str[3*x+2]);
	}
	
	// 3e onglet
	Widget *body3 = wGridLayout(1, 3,0);
	Widget *text1 = wText("Rename your file with '.py' or '.py.tns' extension to get Python's syntax highlights.", 3);
	Widget *text2 = wText("If you placed the Micropython executable into a specific location, indicate his path via the 'Options' dialog box.", 3);
	Widget *text3 = wText("You can then click ENTER at any time to execute and test your program.", 3);
	
	
	wText_SetUnEditable(navigation);
	wText_SetUnEditable(keys);
	wText_SetUnEditable(text1);
	wText_SetUnEditable(text2);
	wText_SetUnEditable(text3);
	wTab_AddTab(tabs, "Navigation", body1);
	wTab_AddTab(tabs, "Keys", body2);
	wTab_AddTab(tabs, "Python", body3);
	
	wAddWidget(mainWidget, tabs);
	
	wAddWidget(body1, subody1);
	wAddWidget(subody1, im_tbtton);
	wAddWidget(subody1, wExLabel("<--- This is the awesome T-BUTTON.", ALIGN_LEFT, nSDL_LoadFont(NSDL_FONT_VGA, 236,7,254)));
	wAddWidget(body1, navigation);
	
	wAddWidget(body2, keys);
	wAddWidget(body2, keyTable);
	
	wAddWidget(body3, text1);
	wAddWidget(body3, text2);
	wAddWidget(body3, text3);
	
	wSetHeight(subody1, 24);
	wSetWidth(mainWidget, 300);
	wSetHeight(mainWidget, 220);
		
	wExecConstruct(mainWidget);
}


int OptionsBox(int *th, int *rfOS, int *font_size, int *monospace, char *path_python)
{
	char path[512];
	
	// déclaration des widgets
	Widget *mainWidget = wDialog2("Options", "OK", "Cancel");
	Widget *body = wBasicLayout(0);
	Widget *theme = wList(wMenu_NewItemList("Theme :", "Classic|Dark"));
	Widget *font = wList(wMenu_NewItemList("Font size :", "Normal|Small|Very Small|Bold"));
	Widget *mono = wExCheck("Monospaced font", *monospace, ALIGN_RIGHT);
	Widget *refreshOS = wExCheck("Refresh OS document browser ?", *rfOS, ALIGN_RIGHT);
	Widget *bodyPP = wGridLayout(2, 1, 0);
	Widget *browse = wButton("Select File...", BUTTON_PRESS);
	Widget *pythonPath = wText(path_python, 3);
	
	// on initialise les widgets
	wList_SelectItem(theme, NULL, *th);
	wList_SelectItem(font, NULL, *font_size);
	
	// on ajoute les widgets
	wAddWidget(mainWidget, body);
	wAddWidget(body, theme);
	wAddWidget(body, font);
	wAddWidget(body, mono);
	wAddWidget(body, refreshOS);
	wAddWidget(body, wLabel("(refreshing OS screen can last a", ALIGN_CENTER));
	wAddWidget(body, wLabel("long time)", ALIGN_CENTER));
	wAddWidget(body, bodyPP);
	wAddWidget(bodyPP, wLabel("Path to Micropython :", ALIGN_LEFT));
	wAddWidget(bodyPP, browse);
	wAddWidget(body, pythonPath);
	
	// on ajuste les tailles
	wSetWidth(mainWidget, 300);
	wSetHeight(mainWidget, 210);
	wSetHeight(bodyPP, 24);
	wGrid_SetColumnSize(bodyPP, 1, 120);
	
	// on créé les fonctions callback
	int CB_Browse(Widget *w, int signal)
	{
		if (signal != SIGNAL_CLICK || !w) return ACTION_CONTINUE;
		int ok = wOpenBox("/documents", "?icro?ython*", path);
		if (ok) {
			wText_SetText(pythonPath, path);
			wDrawWidget(pythonPath);
		}
		return ACTION_CONTINUE;
	}
	
	// on connecte
	wConnect(browse, CB_Browse);
	
	// on exécute
	int ok = wActivateConstruct(mainWidget);
	if (ok) {
		int n;
		wList_GetSelectedItem(theme, &n);
		if (n != *th) {
			*th = n;
			ok = 2;
		}
		
		wList_GetSelectedItem(font, font_size);
		*rfOS = wCheck_IsChecked(refreshOS);
		*monospace = wCheck_IsChecked(mono);
		strcpy(path_python, wText_GetText(pythonPath));
	}
	wCloseConstruct(mainWidget);
	
	return ok;
}




void SetHighlightSyntax(Widget *w, char *name, nSDL_Font *font, int font_type)
{
	wText_RemoveKeyWords(w);
	
	// dans le cas du python
	if (!strcmp(name+strlen(name)-3, ".py")  || !strcmp(name+strlen(name)-7, ".py.tns")) {
		nSDL_Font *redFont;
		nSDL_Font *orangeFont;
		nSDL_Font *bluegreenFont;
		nSDL_Font *fuschiaFont;
		
		if (font_type == FONT_SMALL) {
			redFont			= nSDL_LoadFont(NSDL_FONT_TINYTYPE , 226,44,44);
			orangeFont		= nSDL_LoadFont(NSDL_FONT_TINYTYPE , 245,114,50);
			bluegreenFont	= nSDL_LoadFont(NSDL_FONT_TINYTYPE , 37,158,109);
			fuschiaFont		= nSDL_LoadFont(NSDL_FONT_TINYTYPE , 232,179,17);
		}
		else if (font_type == FONT_BOLD) {
			redFont			= nSDL_LoadFont(NSDL_FONT_VGA , 226,44,44);
			orangeFont		= nSDL_LoadFont(NSDL_FONT_VGA , 245,114,50);
			bluegreenFont	= nSDL_LoadFont(NSDL_FONT_VGA , 37,158,109);
			fuschiaFont		= nSDL_LoadFont(NSDL_FONT_VGA , 232,179,17);
			redFont->hspacing = 1;
			orangeFont->hspacing = 1;
			bluegreenFont->hspacing = 1;
			fuschiaFont->hspacing = 1;
		}
		else if (font_type == FONT_VERY_SMALL) {
			redFont			= nSDL_LoadCustomFont(smallFontChars , 226,44,44);
			orangeFont		= nSDL_LoadCustomFont(smallFontChars , 245,114,50);
			bluegreenFont	= nSDL_LoadCustomFont(smallFontChars , 37,158,109);
			fuschiaFont		= nSDL_LoadCustomFont(smallFontChars , 232,179,17);
			redFont->hspacing = 1;
			orangeFont->hspacing = 1;
			bluegreenFont->hspacing = 1;
			fuschiaFont->hspacing = 1;
		}
		else {  // FONT_NORMAL
			redFont			= nSDL_LoadFont(NSDL_FONT_THIN , 226,44,44);
			orangeFont		= nSDL_LoadFont(NSDL_FONT_THIN , 245,114,50);
			bluegreenFont	= nSDL_LoadFont(NSDL_FONT_THIN , 37,158,109);
			fuschiaFont		= nSDL_LoadFont(NSDL_FONT_THIN , 232,179,17);
		}
		
		redFont->monospaced			= font->monospaced;
		orangeFont->monospaced		= font->monospaced;
		bluegreenFont->monospaced	= font->monospaced;
		fuschiaFont->monospaced 	= font->monospaced;
		
		redFont->hspacing			= font->hspacing;
		orangeFont->hspacing		= font->hspacing;
		bluegreenFont->hspacing	= font->hspacing;
		fuschiaFont->hspacing 	= font->hspacing;
		
		
		// on s'occupe des symboles
		wText_AddKeyWord(w, "=", fuschiaFont);
		wText_AddKeyWord(w, "*", fuschiaFont);
		wText_AddKeyWord(w, "+", fuschiaFont);
		wText_AddKeyWord(w, "-", fuschiaFont);
		wText_AddKeyWord(w, "/", fuschiaFont);
		wText_AddKeyWord(w, "(", fuschiaFont);
		wText_AddKeyWord(w, ")", fuschiaFont);
		wText_AddKeyWord(w, "[", fuschiaFont);
		wText_AddKeyWord(w, "]", fuschiaFont);
		wText_AddKeyWord(w, "{", fuschiaFont);
		wText_AddKeyWord(w, "}", fuschiaFont);
		wText_AddKeyWord(w, "}", fuschiaFont);
		wText_AddKeyWord(w, ":", fuschiaFont);
		wText_AddKeyWord(w, "<", fuschiaFont);
		wText_AddKeyWord(w, ">", fuschiaFont);
		wText_AddKeyWord(w, "#", fuschiaFont);
		
		// on s'occupe des mots-clés qui démarrent un bloc
		wText_AddKeyWord(w, "if", redFont);
		wText_AddKeyWord(w, "elif", redFont);
		wText_AddKeyWord(w, "while", redFont);
		wText_AddKeyWord(w, "for", redFont);
		wText_AddKeyWord(w, "else", redFont);
		wText_AddKeyWord(w, "class", redFont);
		wText_AddKeyWord(w, "def", redFont);
		wText_AddKeyWord(w, "for", redFont);
		wText_AddKeyWord(w, "try", redFont);
		wText_AddKeyWord(w, "import", redFont);
		wText_AddKeyWord(w, "assert", redFont);
		wText_AddKeyWord(w, "finally", redFont);
		wText_AddKeyWord(w, "except", redFont);
		wText_AddKeyWord(w, "with", redFont);
		wText_AddKeyWord(w, "global", redFont);
		wText_AddKeyWord(w, "nonlocal", redFont);
		
		// on s'occupe des autres mots-clés
		wText_AddKeyWord(w, "in", orangeFont);
		wText_AddKeyWord(w, "and", orangeFont);
		wText_AddKeyWord(w, "del", orangeFont);
		wText_AddKeyWord(w, "from", orangeFont);
		wText_AddKeyWord(w, "none", orangeFont);
		wText_AddKeyWord(w, "true", orangeFont);
		wText_AddKeyWord(w, "as", orangeFont);
		wText_AddKeyWord(w, "break", orangeFont);
		wText_AddKeyWord(w, "or", orangeFont);
		wText_AddKeyWord(w, "false", orangeFont);
		wText_AddKeyWord(w, "pass", orangeFont);
		wText_AddKeyWord(w, "yield", orangeFont);
		wText_AddKeyWord(w, "continue", orangeFont);
		wText_AddKeyWord(w, "is", orangeFont);
		wText_AddKeyWord(w, "raise", orangeFont);
		wText_AddKeyWord(w, "lambda", orangeFont);
		wText_AddKeyWord(w, "return", orangeFont);
		wText_AddKeyWord(w, "print", orangeFont);
		
		// on s'occupe du texte
		wText_AddKeyWord(w, "\'", bluegreenFont);
		wText_AddKeyWord(w, "\"", bluegreenFont);
	}
	
}




void CreditsBox()
{
	Widget *mainWidget = wBasicLayout(1);
	Widget *body = wGridLayout(1,2,0);
	Widget *credits = wText("Program created by Lepzulnag, thanks to Hoffa's nSDL and Lepzulnag's Winbox, the framework for nSpire native programmation !", 5);
	Widget *subody = wGridLayout(2,1,0);
	Widget *panel = wPanel(1);
	Widget *thanks = wText("Thank you for using Notewriter 2.0 ! I hope you enjoy it !", 4);
	
	wAddWidget(mainWidget, body);
	wAddWidget(body, credits);
	wAddWidget(body, subody);
	wAddWidget(subody, panel);
	wAddWidget(subody, thanks);
	
	wSetSize(mainWidget, 280,150);
	wSetSize(panel, 52,52);
	wGrid_SetColumnSize(subody, 0, 64);
	
	SDL_Surface *scr = SDL_GetVideoSurface();
	SDL_Surface *image = nSDL_LoadImage(image_tortue_avatar2);
	SDL_Surface *image_;
	
	double angle=0, zoom=1;
	int smooth = 0, zoomSens = -1;
	
	
	void DrawPanel(Widget *w)
	{
		SDL_Rect r = wPanel_GetArea(w);
		image_ = rotozoomSurface(image, angle, zoom, smooth);
		DrawSurface(image_, &((SDL_Rect) {image_->w/2-24,image_->h/2-24,48,48}), scr, &r);
		SDL_FreeSurface(image_);
	}
	
	
	wPanel_Connect(panel, DrawPanel);
	
	while (K_ENTER());
	wDrawConstruct(mainWidget);
	
	while (!K_ESC() && !K_ENTER()) {
		if (zoom > 1 || zoom < 0.5) zoomSens *= -1;
		zoom += 0.02 * zoomSens;
		wDrawWidget(panel);
		SDL_Flip(scr);
	}
	
	
	SDL_FreeSurface(image);
	wCloseConstruct(mainWidget);
	
}



