%{
#include <ctype.h>  
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int fileno(FILE* );


unsigned int len = 4;
unsigned int cp = 0;
unsigned int size = 0;

unsigned int
rm_space(char *str, unsigned int size)
{
  unsigned int cp = 0;
  for(unsigned int i = 0; i < size; i++)
  {
    while (isblank(str[i + cp]) && (i + cp) < size)
      cp++;
    str[i] = str[i + cp];
  }
  return cp;
}

%}
%option noinput nounput
%x LINE

HEX     ([0-9]|[a-f])
OCTET   (({HEX}{2})+([[:blank:]])?)
OCTETS  {OCTET}+
%%
(.|\n)*"section"[[:blank:]]\."text" {;}
{HEX}+\:[[:blank:]]+/{HEX}+   { BEGIN LINE; }
<LINE>{OCTETS}   {
                          size = yyleng;
                          size -= rm_space(yytext, size);
			  if (size > 0 && (size % 2))
			    size--;
                          while (size > 1 && (size % 2) == 0)
                          {
                            size -= 2;
                            printf ("\\x%c%c", yytext[size], yytext[size + 1]);
                            cp++;
                          }
                        }
<LINE>[[:blank:]]+          { printf ("\"\n\t\""); BEGIN INITIAL; } 
(.|\n)                  {;}

%%
/* 
<LINE>[[:blank:]]           {;}
 */
int
main(int argc, char **argv)
{
  if(argc == 2)
     yyin = fopen(argv[1],"r");
  else
     yyin = stdin;
  printf ("Payload :\n\t\"");
  yylex();
  printf ("\"\n");
  printf ("Nombre d'octets : %u\n", cp);
  return EXIT_SUCCESS;
}

