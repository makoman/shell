#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

/*
  fonctions pour les commandes
 */
int cd(char **args);
int help(char **args);
int exits(char **args);
int cat(char **args);
int copy(char **args);
int history(char **args);
int find(char **args);
int touch(char **args);


/*
  liste des commandes, suivies par leurs fonctions.
 */
char *builtin_str[] = {
  "cd",
  "help",
  "exits",
  "cat",
  "copy",
  "history",
  "find",
  "touch"

};

int (*builtin_func[]) (char **) = {
  &cd,
  &help,
  &exits,
  &cat,
  &copy,
  &history,
  &find,
  &touch
};

int lsh_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

/*
  fonctions
*/

/*
   Commande built-in: changer repertoire.
   args[0] est "cd".  args[1] est le repertoire.
   Return 1 pour continuer.
 */
int cd(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "lsh: manque d'argument a \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("lsh");
    }
  }
  return 1;
}

// TOUCH pour creer un fichier vide
int touch(char **args)
{
    FILE *ptr;
    int i = 1;

  if (args[1] == NULL) {
    fprintf(stderr, "lsh: manque d'argument a \"touch\"\n");
  } else {

      while(args[i] != NULL){
      printf("Creation du fichier \"%s\"...\n", args[i]);
      ptr = fopen(args[i], "w");
      fclose(ptr);
      i++;
      }
  }
  return 1;
}

// CAT pour afficher le contenu d'un fichier
int cat(char **args)
{
    int c;
    char z;
    FILE *ptr;
    int i = 1;

  if (args[1] == NULL) {
    fprintf(stderr, "lsh: manque d'argument a \"cat\"\n");
  } else {

      while(args[i] != NULL){
      printf("Affichage du contenu de \"%s\"...\n", args[i]);
      ptr = fopen(args[i], "r");
      if (ptr) {
            while ((c = getc(ptr)) != EOF){
                putchar(c);

            }
        printf("\n");
        fclose(ptr);
        }
        i++;
      }
  }
  return 1;
}

// COPY pour copier des fichiers
int copy(char **args)
{
    FILE *fp1, *fp2;
    char ch;

    if (args[1] == NULL || args[2] == NULL){
        fprintf(stderr, "lsh: manque d'argument a \"copy\"\n");
    }else{

    fp1 = fopen(args[1], "r");
    fp2 = fopen(args[2], "w");

    while (1) {
        ch = fgetc(fp1);

        if (ch == EOF)
        break;
    else
        putc(ch, fp2);
   }

    printf("Copie du fichier reussie!\n");
    fclose(fp1);
    fclose(fp2);
    }
    return 1;
}

// HISTORY pour afficher les commandes saisies
int history(char **args){

    FILE *ptr;
    int c;

    printf("Affichage history...\n");
    ptr = fopen("hist", "r");
    while(1){
      c = fgetc(ptr);
      if( feof(ptr) )
      {
         break ;
      }
      printf("%c", c);
    }
    fclose(ptr);
    return 1;
}

// fonction recursive pour FIND
void listdir(const char *name, int level)
{
    DIR *dir;
    struct dirent *entry;

    if (!(dir = opendir(name)))
        return;
    if (!(entry = readdir(dir)))
        return;

    do {
        if (entry->d_type == DT_DIR) {
            char path[1024];
            int len = snprintf(path, sizeof(path)-1, "%s/%s", name, entry->d_name);
            path[len] = 0;
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            printf("%*s[%s]\n", level*2, "", entry->d_name);
            listdir(path, level + 1);
        }
        else
            printf("%*s- %s\n", level*2, "", entry->d_name);
    } while (entry = readdir(dir));
    closedir(dir);
}


// FIND
int find(char **args){
    DIR *dp;
    DIR *dq;
    char * d_name;
    struct dirent *ep;

    if (args[1] == NULL) {
    fprintf(stderr, "lsh: manque d'argument a \"find\"\n");
  } else {

        dp = opendir(".");
        // trouver le fichier ou le repertoire
        while ((ep = readdir (dp)) != NULL) {
            d_name = ep->d_name;
            if (strcmp(d_name, args[1]) == 0)
                if (!(dq = opendir(args[1]))){
                    printf("- %s\n", d_name);
                }else{
                    printf("[[%s]]\n", d_name);
                }
        }

        listdir(args[1], 0);


      /*if (dp != NULL){
        while (ep = readdir (dp))
            puts (ep->d_name);
      (void) closedir (dp);
    }else
        perror ("Couldn't open the directory");*/
  }

    return 1;
}

/*
    Commande built-in: afficher help
    Pas d'args
    Return 1 pour continuer l'execution
 */
int help(char **args)
{
  int i;
  printf("SHELL\n");
  printf("Commandes et arguments.\n");
  printf("Les commandes suivantes sont built-in:\n");

  for (i = 0; i < lsh_num_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }

  printf("Utiliser man pour aide.\n");
  return 1;
}

/*
    Commande built-in: exit
    Pas d'args
    Return 0 pour terminer
 */
int exits(char **args)
{
  return 0;
}

/*
   Lancer un programme, attendre qui'il termine
  args Null terminated list of arguments (including program).
  Return 1 pour continuer l'execution
 */
int launch(char **args)
{
  pid_t pid, wpid;
  int status;

  pid = fork();
  if (pid == 0) {

    // Child process
    if (execvp(args[0], args) == -1){
      perror("lsh");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    // Error forking
    perror("lsh");
  } else {


    // Parent process
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

/*
   lancer built-in or programme.
   args Null terminated list of arguments.
   return 1 pour continuer, 0 pour terminer
 */
int execute(char **args)
{
  int i;

  if (args[0] == NULL) {
    // Une commande nulle saisie
    return 1;
  }

    // verifier si c'est un built-in
  for (i = 0; i < lsh_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  return launch(args);
}

#define LSH_RL_BUFSIZE 1024
/*
   Lire une ligne input de stdin
   Return la ligne de l'input de stdin.
 */
char *read_line(void)
{
  int bufsize = LSH_RL_BUFSIZE;
  int position = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  int c;

  if (!buffer) {
    fprintf(stderr, "lsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  while (1) {
    // Lire un caractere
    c = getchar();

    // A EOF, le remplacer avec un caractere nul et return
    if (c == EOF || c == '\n') {
      buffer[position] = '\0';
      return buffer;
    } else {
      buffer[position] = c;
    }
    position++;

    // Si plus de buffer, reallocation.
    if (position >= bufsize) {
      bufsize += LSH_RL_BUFSIZE;
      buffer = realloc(buffer, bufsize);
      if (!buffer) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
}

#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"

/*
   diviser l'entree en plusieurs arguments
   Null-terminated array of arguments (tokens).
 */
char **split_line(char *line)
{
  int bufsize = LSH_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token;


  if (!tokens) {
    fprintf(stderr, "lsh: allocation error\n");
    exit(EXIT_FAILURE);
  }


  token = strtok(line, LSH_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += LSH_TOK_BUFSIZE;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, LSH_TOK_DELIM);
  }
  tokens[position] = NULL;

    FILE *ptr;
    ptr = fopen("hist", "a");
    fprintf(ptr, "%s\r\n", tokens[0]);  // Windows newline
    fclose(ptr);

  return tokens;
}

/*
   Loop attend une entree au clavier
 */
void loop(void)
{
  char *line;
  char **args;
  int status;

  do {

    char cwd[1024];
    getcwd(cwd, sizeof(cwd));

    printf("%s> ",cwd);
    line = read_line();
    args = split_line(line);
    status = execute(args);

    free(line);
    free(args);
  } while (status);
}


int main(int argc, char **argv)
{


    // vider historybook
    FILE *ptr;
    ptr = fopen("hist", "w");
    fclose(ptr);

  // executer loop, attend une entree de l'utilisateur
    loop();

  return EXIT_SUCCESS;
}
