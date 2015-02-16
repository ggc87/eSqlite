%{#include<string.h>
	int nCols=0;
	int nVals=0;
	int type=0;
	char *tabNames;
	char **ccols;
	char **vvals;
	int pprimary;
%}
%%

CREATE[" "]TABLE { type=4; }

PRIMARY {}

KEY {
	pprimary = nCols-1;
    }

NOT {}

NULL {}

INT {}

VARCHAR {}

TEXT {}

CHAR["("[0-9]*")"]* {}

REAL {}

SELECT { type=2; }

INSERT { type=2; }

AND { type=2; }

VALUES { type=3; }

FROM { type=4; }

WHERE { pprimary=nCols;type=2; }

"="   { type=3; }

INTO { type=4; }

[*||(||)||'||;||,] 

[A-Za-zA-Z0-9]* { 
		switch (type){
		case 1:
		case 2:
			ccols = realloc(ccols,sizeof(char*)*(nCols+1));
			ccols[nCols] = malloc(sizeof(char)*strlen(yytext)+1);
			strcpy(ccols[nCols],yytext); 	
			nCols++;
			break;
		case 3:
			vvals = realloc(vvals,sizeof(char*)*(nVals+1));
			vvals[nVals] = malloc(sizeof(char)*strlen(yytext)+1);
			strcpy(vvals[nVals],yytext);
			nVals++;
			break;
		case 4:
			if(!tabNames)
				tabNames=malloc(sizeof(char)*strlen(yytext));
			strcpy(tabNames,yytext);
			type=2;
			break;
		default:
			break;
		}

	  }

%%
int extractDataSql(char *argv, char **tabName, char ***cols, char ***vals,int *info, int *colSize){
	nCols=0;
	nVals=0;
	type=0;
	pprimary = 0;
	yy_scan_string(argv);
	yylex();
	*tabName=tabNames;
	*cols=ccols;
	*vals=vvals;
	*colSize=nCols;
	if(info)
		*info = pprimary;
	printf("\n");
	return 0;
}
