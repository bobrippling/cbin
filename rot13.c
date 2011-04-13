#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void rot13(char *);


void rot13(char *arg)
{
  unsigned int i;
  for(i = 0; i < strlen(arg); i++)
    if(isalpha(arg[i]))
      arg[i] = (arg[i] - (isupper(arg[i]) ? 'A' : 'a') + 13) % 26 + (isupper(arg[i]) ? 'A' : 'a');

}


int main(int argc, char **argv)
{
  int i, ret = EXIT_SUCCESS;

  if(argc == 1){
    char *s = NULL;
    size_t nread;

    while(getline(&s, &nread, stdin) != -1){
      rot13(s);
      fputs(s, stdout);
    }

    if(ferror(stdin)){
      perror("stdin");
      ret = EXIT_FAILURE;
    }

  }else{
    for(i = 1; i < argc; i++){
      rot13(argv[i]);
      printf("%s ", argv[i]);
    }

    putchar('\n');
  }

  return ret;
}
