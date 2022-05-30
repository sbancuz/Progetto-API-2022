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
  for (size_t s = 0; s < in->len; s++) {                                       \
    pre_s char **pr = in->slice[s].arr + in->slice[s].start;                   \
    for (size_t j = in->slice[s].start;                                        \
         j < in->slice[s].len && in->slice[s].jumps[j] != (size_t)-1;          \
         pr++, j++) {                                                          \
                                                                               \
      expr if (j != in->slice[s].len - 1 &&                                    \
               in->slice[s].jumps[j + 1] != (size_t)-1) {                      \
        pr += in->slice[s].jumps[j + 1];                                       \
        j += in->slice[s].jumps[j + 1];                                        \
      }                                                                        \
    }                                                                          \
    post_s                                                                     \
  }

typedef struct slice_s {
  char **arr;
  size_t len;
  size_t *jumps;
  size_t start;
} slice;

typedef struct input_s {
  slice *slice;
  size_t len;
  size_t totInputLen;
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
  for (size_t s = 0; s < in->len; s++) {

    char **pr = in->slice[s].arr;
    for (size_t j = 0; j < in->slice[s].len; pr++, j++) {
      if (strcmp(*pr, needle) == 0)
        return true;
    }
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
//         printf(" %c - n = %ld, c = %ld, s = %ld\n", p[i], n, c, s);

    if (filter[i] == '|' && ((r[i] == p[i] && n == 0) || s >= n - c))
      return false;

    if (filter[i] == '/' && (s < n - c))
      return false;
  }

  return strcmp(r, p) != 0;
}

size_t removeIncompatible(char *filter, input *in, char *str) {
  size_t cont = 0;
  size_t pin = 0;
//     printf("----------------\n");
//     printf("|     %s     |\n", str);
//     printf("----------------\n");

  LOOP_OVER_INPUT(
      pin = in->slice[s].start;
      ,
      if (compatible(filter, *pr, str)) {
        cont++;
        pin = j + 1;
//         printf(">> %s\n", *pr);
      } else {
        // se deve saltare su un indice che ha un salto a sua volta,
        // allora aggiungo i salti del secondo nel primo, ricordati di
        // ringraziare Luca passato per questa spiegazione :)
        in->slice[s].jumps[pin]++;
        if (pin + in->slice[s].jumps[pin] < in->slice[s].len && in->slice[s].jumps[pin + in->slice[s].jumps[pin]] != (size_t)-1)
          in->slice[s].jumps[pin] +=
              in->slice[s].jumps[pin + in->slice[s].jumps[pin]];

        if (in->slice[s].start == pin)
          in->slice[s].start = in->slice[s].jumps[pin];
      },
      if (pin + 1 + in->slice[s].jumps[pin] > in->slice[s].len) in->slice[s]
          .jumps[pin] = -1;)

  return cont;
}

int main(int argc, char *argv[]) {
  // getting rid of the first argument witch is the file name
  NEXT_ARG();
  assert(argc != 0 && "Please put some arguments to run this program!\n");

  size_t wordsLen = (size_t)strtol(*argv, NULL, 10);
  NEXT_ARG();

  // init input
  input *in = malloc(sizeof(input));
  in->slice = malloc(sizeof(slice));
  in->slice[0].arr = argv;
  in->len = 1;
  while (true) {
    if (strcmp("+nuova_partita", *argv) == 0) {
      NEXT_ARG();
      break;
    }

    in->slice[0].len++;
    NEXT_ARG();
  }

  char *referenceWord = *argv;
  NEXT_ARG();

  size_t guessesNumber = (size_t)strtol(*argv, NULL, 10);
  NEXT_ARG();

  bool hasWon = false;
  char *res = malloc(sizeof(char) * wordsLen);

  in->slice[0].jumps = malloc(sizeof(size_t) * in->slice[0].len);

  szArr *storedRes = malloc(sizeof(szArr));
  storedRes->v = malloc(sizeof(char *) * guessesNumber);
  for (size_t i = 0; i < guessesNumber; i++)
    storedRes->v[i] = malloc(sizeof(char) * wordsLen);

  szArr *storedGuesses = malloc(sizeof(szArr));
  storedGuesses->v = malloc(sizeof(char *) * guessesNumber);
  //     char ** a = input;
  //     for (size_t i = 0; i < inputLen; i++, a++) printf("%s\n", *a);
  for (size_t i = 0; guessesNumber > 0; ++i) {
    if (strcmp("+stampa_filtrate", *argv) == 0) {

      LOOP_OVER_INPUT(;, printf("%s\n", *pr);, ;)
      NEXT_ARG();
      continue;

    } else if (strcmp("+inserisci_inizio", *argv) == 0) {
      NEXT_ARG();
      in->len++;
      in->slice = realloc(in->slice, sizeof(slice) * in->len);
      in->slice[in->len - 1].arr = argv;
      while (strcmp("+inserisci_fine", *argv) != 0) {
        NEXT_ARG();
        in->slice[in->len - 1].len++;
      }
      in->slice[in->len - 1].jumps =
          malloc(sizeof(size_t) * in->slice[in->len - 1].len);
      // TODO guardare solo questo slice
      for (size_t j = 0; j < storedRes->len; j++) {
        removeIncompatible(storedRes->v[j], in, storedGuesses->v[j]);
      }

//       for (size_t s = 0; s < in->len; s++)
//         for (size_t k = 0; k < in->slice[s].len; k++)
//           printf("%ld", in->slice[s].jumps[k]);
//       printf("\n");

      NEXT_ARG();
      continue;

    } else if (strcmp(referenceWord, *argv) == 0) {
      printf("ok\n");
      hasWon = true;
      break;
    } else if (!inInput(*argv, in)) {
      printf("not_exists\n");
      NEXT_ARG();
      continue;
    }

    computeRes(referenceWord, *argv, res);
    storedRes->len++;
    strcpy(storedRes->v[storedRes->len - 1], res);
    storedGuesses->len++;
    storedGuesses->v[storedGuesses->len - 1] = *argv;

    size_t remaining = removeIncompatible(res, in, *argv);
//     for (size_t s = 0; s < in->len; s++)
//       for (size_t k = 0; k < in->slice[s].len; k++)
//         printf("%ld", in->slice[s].jumps[k]);
//     printf("\n");
    printf("%s\n%ld\n", res, remaining);
    guessesNumber--;
    NEXT_ARG();
  }

  if (!hasWon)
    printf("ko\n");

  free(res);

  return 0;
}
