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

typedef struct jumps_s {
  size_t *arr;
  size_t start;
} jump;

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

bool inInput(char *needle, char **arr, size_t arrLen) {
  while (arrLen != 0) {
    //         printf("%s - %s\n", needle, *arr);
    if (strcmp(needle, *arr) == 0)
      return true;

    arr++;
    arrLen--;
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

    if (filter[i] == '|' && (r[i] == p[i] && n == 0))
      return false;

    if (filter[i] == '/' && (s < n - c || n != 0))
      return false;
  }

  //   printf("%s - %s\n", r, p);
  return strcmp(r, p) != 0;
}

size_t removeIncompatible(char *filter, char **arr, char *str, jump *jumps,
                          size_t arrLen) {
  size_t cont = 0;
  size_t pin = 0;

  arr += jumps->start;
  for (size_t i = jumps->start; i < arrLen; i++, arr++) {

    //     printf("%s - %s\n", *arr, str);
    if (compatible(filter, *arr, str)) {
      cont++;
      pin = i + 1;
      //             printf("%ld", i);
    } else {
      printf("%ld - %ld\n", i, pin);
      // se deve saltare su un indice che ha un salto a sua volta, allora
      // aggiungo i salti del secondo nel primo, ricordati di ringraziare Luca
      // passato per questa spiegazione :)

      jumps->arr[pin]++;
      if (pin + 1 + jumps->arr[pin] < arrLen)
        jumps->arr[pin] += jumps->arr[pin + 1 + jumps->arr[pin]];
    }

    if (i != arrLen - 1 && jumps->arr[i + 1] != (size_t)-1) {
      arr += jumps->arr[i + 1];
      i += jumps->arr[i + 1];
    }
  }

  if (pin + 1 + jumps->arr[pin] > arrLen)
    jumps->arr[pin] = -1;

  return cont;
}

int main(int argc, char *argv[]) {
  // getting rid of the first argument wich is the file name
  NEXT_ARG();
  assert(argc != 0 && "Please put some arguments to run this program!\n");

  // getting rid of the first argument wich is the file name
  size_t wordsLen = (size_t)strtol(*argv, NULL, 10);
  NEXT_ARG();

  char **input = argv;
  //     size_t inputIndex = argc;
  size_t inputLen = 0;

  while (true) {
    if (strcmp("+nuova_partita", *argv) == 0) {
      NEXT_ARG();
      break;
    }

    inputLen++;
    NEXT_ARG();
  }

  char *referenceWord = *argv;
  NEXT_ARG();

  size_t guessesNumber = (size_t)strtol(*argv, NULL, 10);
  NEXT_ARG();

  bool hasWon = false;
  char *res = malloc(sizeof(char) * wordsLen);
  jump *jumps = malloc(sizeof(jump));
  jumps->arr = malloc(sizeof(size_t) * inputLen);
  jumps->start = 0;

  //     char ** a = input;
  //     for (size_t i = 0; i < inputLen; i++, a++) printf("%s\n", *a);

  for (size_t i = 0; guessesNumber > 0; ++i) {
    if (strcmp("+stampa_filtrante", *argv) == 0) {
      char **pr = input + jumps->start;
      for (size_t j = jumps->start; j < inputLen; pr++, j++) {

        // 0010-1000

        printf("%s - %ld\n", *pr, j);
        if (j != inputLen - 1 && jumps->arr[j + 1] != (size_t)-1) {
          pr += jumps->arr[j + 1];
          j += jumps->arr[j + 1];
        }
      }

      NEXT_ARG();
      continue;
    } else if (strcmp(referenceWord, *argv) == 0) {
      printf("ok\n");
      hasWon = true;
      break;
    } else if (!inInput(*argv, input, inputLen)) {
      printf("not_exist\n");
      NEXT_ARG();
      continue;
    }
    computeRes(referenceWord, *argv, res);

    size_t remaining = removeIncompatible(res, input, *argv, jumps, inputLen);
    printf("\n");
    for (size_t k = 0; k < inputLen; k++)
      printf("%ld", jumps->arr[k]);
    printf("\n");
    printf("%s, %ld\n", res, remaining);
    guessesNumber--;
    NEXT_ARG();
  }

  if (!hasWon)
    printf("ko\n");

  free(res);
  free(jumps->arr);
  free(jumps);

  return 0;
}
