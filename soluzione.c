#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NEXT_ARG()                                                             \
  assert(argc > 0 && "Args finiti\n");                                         \
  argc--;                                                                      \
  argv++

#define LOOP_OVER_INPUT(pre_s, expr, post_s)                                   \
  pre_s char **pr = in->arr + in->start;                                       \
  for (size_t j = in->start; j < in->len && in->jumps[j] != -1; pr++, j++) {   \
                                                                               \
    expr if (j != in->len - 1 && in->jumps[j + 1] != -1) {                     \
      pr += in->jumps[j + 1];                                                  \
      j += in->jumps[j + 1];                                                   \
    }                                                                          \
  }                                                                            \
  post_s

#define SWAP(tmp, a, b)                                                        \
  tmp = a;                                                                     \
  a = b;                                                                       \
  b = tmp

typedef struct input_s {
  char **arr;
  size_t len;
  int64_t *jumps;
  size_t start;
} input;

typedef struct sizedArr_s {
  char **v;
  size_t len;
} szArr;

size_t count(char *str, char needle) {
  size_t cont = 0;

  for (size_t i = 0; str[i] != '\0'; ++i) {
    if (str[i] == needle)
      cont++;
  }

  return cont;
}

size_t countCorr(char *str, char *str2, char needle) {
  size_t cont = 0;

  for (size_t i = 0; str[i] != '\0'; ++i) {
    if (str[i] == needle && str[i] == str2[i])
      cont++;
  }

  return cont;
}

size_t countScorr(char *str, char *str2, size_t len, char needle) {
  size_t cont = 0;

  for (size_t i = 0; i < len; ++i) {
    if (str[i] == needle && str[i] != str2[i]) {
      cont++;
    }
  }

  return cont;
}

void computeRes(char *r, char *p, char *res) {
  for (size_t i = 0; r[i] != '\0'; ++i) {
    size_t n = count(r, p[i]);
    if (r[i] == p[i]) {
      res[i] = '+';
    } else if (n == 0)
      res[i] = '/';
    else {
      size_t c = countCorr(r, p, p[i]);
      size_t s = countScorr(p, r, i, p[i]);

      if (s >= (n - c))
        res[i] = '/';
      else
        res[i] = '|';
    }
  }
}

bool inInput(char *needle, input *in) {

  char **pr = in->arr;
  for (size_t j = 0; j < in->len; pr++, j++) {
    if (strcmp(*pr, needle) == 0)
      return true;
  }
  return false;
}

bool compatible(char *filter, char *r, char *p) {
  for (size_t i = 0; filter[i] != '\0'; ++i) {
    if (filter[i] == '+' && r[i] != p[i])
      return false;

    size_t n = count(r, p[i]);
    size_t c = countCorr(r, p, p[i]);
    size_t s = countScorr(p, r, i, p[i]);
    //     printf(" %c - n = %ld, c = %ld, s = %ld\n", p[i], n, c, s);

    if (filter[i] == '|' && (r[i] == p[i] || n == 0 || s >= n - c))
      return false;

    if (filter[i] == '/' && (s < n - c || (n != 0 && s == 0)))
      return false;
  }

  return strcmp(r, p) != 0;
}

size_t removeIncompatible(char *filter, input *in, char *str) {
  size_t cont = 0;
  size_t pin = 0;
  //   printf("----------------\n");
  //   printf("|     %s     |\n", str);
  //   printf("----------------\n");

  LOOP_OVER_INPUT(
      pin = in->start;
      ,
      if (compatible(filter, *pr, str)) {
        cont++;
        pin = j + 1;
        //         printf(">> %s\n", *pr);
      } else {
        //         printf("XX %s\n", *pr);

        // se deve saltare su un indice che ha un salto a sua volta,
        // allora aggiungo i salti del secondo nel primo, ricordati di
        // ringraziare Luca passato per questa spiegazione :)
        in->jumps[pin]++;
        if (pin + in->jumps[pin] < in->len &&
            in->jumps[pin + in->jumps[pin]] != -1)
          in->jumps[pin] += in->jumps[pin + in->jumps[pin]];
      },
      if (in->jumps[in->start] != 0) in->start += in->jumps[in->start];
      if (pin + 1 + in->jumps[pin] > in->len) in->jumps[pin] = -1;)

  return cont;
}

int64_t binSearch(input *in, char *str) {
  int64_t low = 0, high = in->len - 1, mid;

  while (low <= high) {
    mid = (low + high) / 2;
    if (strcmp(str, in->arr[mid]) > 0)
      low = mid + 1;
    else
      high = mid - 1;
  }

  return low;
}

void insert(input *in, char *str) {
  int64_t new = binSearch(in, str);

  for (int64_t i = in->len - 1; i != new; i--) {
    in->arr[i] = in->arr[i - 1];
    in->jumps[i] = in->jumps[i - 1];
  }

  in->arr[new] = str;
  in->jumps[new] = 0;

  int64_t jump = -1;
  for (int64_t i = 0; i < new; i++) {
    if (in->jumps[i] != 0)
      jump = in->jumps[i];
    jump--;
  }

  if (jump > 0) {
    in->jumps[new + 1] += jump;
    for (int64_t i = new; i >= 0; i--) {
      if (in->jumps[i] != 0) {
        in->jumps[i] -= jump;
        break;
      }
    }
  }
}

int main(int argc, char *argv[]) {

  bool doCount = true;
  size_t a;
  char *arg;

  getline(&arg, &a, stdin);
  size_t wordsLen = strtol(arg, NULL, 10);
  input *in = malloc(sizeof(input));
  in->len = 0;

  getline(&arg, &a, stdin);
  while (!feof(stdin)) {
    if (strcmp(arg, "+nuova_partita") == 0) {
      doCount = false;
      continue;
    } else if (strcmp(arg, "+inserisci_inizio") == 0) {
      doCount = true;
      continue;
    } else if (strcmp(arg, "+inserisci_fine") == 0) {
      doCount = false;
      continue;
    }
    if (doCount)
      in->len++;

    getline(&arg, &a, stdin);
  }
  printf("%s\n", arg);
  freopen("a", "r", stdin);

  getline(&arg, &a, stdin);
  printf("%s\n", arg);

  in->arr = malloc(sizeof(char *) * in->len);

  char **ptr = in->arr;
  int64_t len = 0;
  while (strcmp(arg, "+nuova_partita") != 0) {
    getline(&arg, &a, stdin);
    *ptr = arg;
    ptr++;
    len++;
  }

  char *referenceWord;
  getline(&referenceWord, NULL, stdin);

  getline(&arg, &a, stdin);
  size_t guessesNumber = (size_t)strtol(arg, NULL, 10);

  bool hasWon = false;
  char *res = malloc(sizeof(char) * wordsLen);

  in->jumps = malloc(sizeof(int64_t) * in->len);

  for (size_t i = 0; i < in->len; i++)
    *ptr = "~";
  in->len = len;

  // TODO find something better than insertion sort
  char *tmp;
  for (size_t i = 1; i < in->len; i++) {
    int64_t j = i - 1;
    tmp = in->arr[i];

    while (j >= 0 && strcmp(tmp, in->arr[j]) < 0) {
      in->arr[j + 1] = in->arr[j];
      j--;
    }

    in->arr[j + 1] = tmp;
  }

  szArr *storedRes = malloc(sizeof(szArr));
  szArr *storedGuesses = malloc(sizeof(szArr));

  storedRes->v = malloc(sizeof(char *) * guessesNumber);
  storedGuesses->v = malloc(sizeof(char *) * guessesNumber);
  for (size_t i = 0; i < guessesNumber; i++) {
    storedRes->v[i] = malloc(sizeof(char) * wordsLen);
    storedGuesses->v[i] = malloc(sizeof(char) * wordsLen);
  }

  while (!feof(stdin)) {
    for (size_t i = 0; guessesNumber > 0; ++i) {
      if (strcmp("+stampa_filtrate", arg) == 0) {
        LOOP_OVER_INPUT(;, printf("%s\n", *pr);, ;)
        getline(&arg, &a, stdin);

        continue;

      } else if (strcmp("+inserisci_inizio", arg) == 0) {
        getline(&arg, &a, stdin);

        for (size_t j = 0; j < in->len; j++)
          if (in->jumps[j] == -1) {
            in->jumps[j] = in->len - j;
            break;
          }

        while (strcmp("+inserisci_fine", arg) != 0) {
          getline(&arg, &a, stdin);
          in->len++;
          insert(in, arg);
        }

        for (size_t j = 0; j < storedRes->len; j++) {
          removeIncompatible(storedRes->v[j], in, storedGuesses->v[j]);
        }

        getline(&arg, &a, stdin);
        continue;

      } else if (strcmp(referenceWord, arg) == 0) {
        printf("ok\n");
        hasWon = true;
        if (argc > 0)
          getline(&arg, &a, stdin);
        break;
      } else if (!inInput(arg, in)) {
        printf("not_exists\n");
        getline(&arg, &a, stdin);
        continue;
      }

      computeRes(referenceWord, arg, res);
      storedRes->len++;
      strcpy(storedRes->v[storedRes->len - 1], res);
      storedGuesses->len++;
      strcpy(storedGuesses->v[storedGuesses->len - 1], arg);

      size_t remaining = removeIncompatible(res, in, arg);
      printf("%s\n%ld\n", res, remaining);
      guessesNumber--;
      getline(&arg, &a, stdin);
    }

    if (!hasWon)
      printf("ko\n");

    if (argc == 0)
      break;

    if (strcmp("+inserisci_inizio", *argv) == 0) {
      getline(&arg, &a, stdin);

      size_t oldLen = in->len;
      while (strcmp("+inserisci_fine", *argv) != 0) {
        getline(&arg, &a, stdin);
        in->arr[in->len] = "~"; // this is the ASCII max char
        in->len++;
      }
      in->jumps = realloc(in->jumps, sizeof(int64_t) * in->len);
      for (size_t j = oldLen; j < in->len; j++)
        in->jumps[j] = 0;
      char **tmp2 = argv;
      for (size_t j = in->len - oldLen; j > 0; j--)
        insert(in, *--tmp2);

      getline(&arg, &a, stdin);
    }
    if (strcmp("+nuova_partita", *argv) == 0) {
      storedRes->len = 0;
      storedGuesses->len = 0;
      for (size_t i = 0; i < in->len; i++)
        in->jumps[i] = 0;
      in->start = 0;
      getline(&arg, &a, stdin);
      referenceWord = *argv;
      getline(&arg, &a, stdin);
      guessesNumber = strtol(*argv, NULL, 10);
    }
    getline(&arg, &a, stdin);
  }
  // TODO Free everything
  free(res);
  //  TODO find a way to free in->arr, not that important, leak is only 8
  //  bytes
  //   free(in->arr);
  free(in->jumps);
  free(in);
  for (size_t i = 0; i < storedRes->len; i++) {
    free(storedRes->v[i]);
    free(storedGuesses->v[i]);
  }
  free(storedRes->v);
  free(storedGuesses->v);
  free(storedRes);
  free(storedGuesses);

  return 0;
}
