#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ALPHALEN 64
#define INIT_LINE_BUFFER()                                                     \
  size_t len = 0;                                                              \
  ssize_t numCharsRead = 0
#define NEW_LINE(a)                                                            \
  numCharsRead = getline(&a, &len, stdin);                                     \
  a[numCharsRead - 1] = '\0'

typedef struct node_s {
  struct node_s **next;
  struct node_s *prev;
  uint32_t connected;
  char val;
} node;

typedef struct sizedArr_s {
  char **v;
  size_t len;
} szArr;

size_t wordsLen;

size_t getIndex(char c) {
  if (c >= '0' && c <= '9')
    return c - '0' + 1;
  if (c >= 'a' && c <= 'z')
    return c - 'a' + 38;
  if (c >= 'A' && c <= 'Z')
    return c - 'A' + 11;
  if (c == '-')
    return 0;
  if (c == '_')
    return 37;

  return ALPHALEN + 1;
}
void *initNode(node *prev, char c) {
  node *n = malloc(sizeof(node));
  n->val = c;
  n->connected = 0;
  n->next = malloc(sizeof(node) * ALPHALEN);
  n->prev = prev;
  return (void *)n;
}

void addWord(node *nod, char *str) {
  if (*str == '\n' || *str == '\0')
    return;

  if (nod->next[getIndex(*str)] == NULL) {
    nod->next[getIndex(*str)] = initNode(nod, *str);
  }
  nod->connected++;
  char c = *str;
  str++;
  addWord(nod->next[getIndex(c)], str);
}

void dumpTreeImpl(node *nod, size_t d, char *tmpStr) {
  if (nod->connected == 0 && d == wordsLen - 1) {
    node *tmp = nod;
    while (tmp->val != '#') {
      tmpStr[d] = tmp->val;
      d--;
      tmp = tmp->prev;
    }

    printf("%s\n", tmpStr);
    return;
  }

  for (size_t i = 0; i < ALPHALEN; i++) {
    if (nod->next[i] != NULL) {
      //    prob a bug with freeing mem
      if (nod->next[i]->val != '\0')
        dumpTreeImpl(nod->next[i], d + 1, tmpStr);
    }
  }
}

void dumpTree(node *nod) {
  char *tmpStr = malloc(sizeof(char) * wordsLen);
  dumpTreeImpl(nod, -1, tmpStr);
  free(tmpStr);
}

bool inTree(node *nod, char *needle) {
  node *tmp = nod;
  for (size_t i = 0; i < wordsLen; i++) {
    if (tmp->next[getIndex(*needle)] == NULL) {
      return false;
    }
    tmp = tmp->next[getIndex(*needle)];
    needle++;
  }
  return true;
}

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

size_t removeIncompatibleImpl(char *filter, node *nod, size_t d, char *str,
                              char *tmpStr) {
  if (nod->connected == 0 && d == wordsLen - 1) {
    node *tmp = nod;
    while (tmp->val != '#') {
      tmpStr[d] = tmp->val;
      d--;
      tmp = tmp->prev;
    }
#ifdef DEBUG
    printf("----compatibility test----\n");
    printf("%s - %s - %s\n", filter, tmpStr, str);
#endif
    if (!compatible(filter, tmpStr, str)) {
#ifdef DEBUG
      printf("incompatibile\n");
      printf("--------------------------\n");
#endif
      return -1;
    }
#ifdef DEBUG
    else {
      printf("compatibile\n");
      printf("--------------------------\n");
    }
#endif
    return -2;
  }

  for (size_t i = 0; i < ALPHALEN; i++) {
    if (nod->next[i] != NULL) {
      //    prob a bug with freeing mem
      if (nod->next[i]->val != '\0') {
        size_t ret =
            removeIncompatibleImpl(filter, nod->next[i], d + 1, str, tmpStr);
        if (ret == (size_t)-1) {
          nod->next[i]->connected--;
          if (nod->val != '#')
            return (size_t)-1;
          nod->connected--;
          i--;
        }
      }
    }
  }
  return nod->connected;
}

size_t removeIncompatible(char *filter, node *nod, char *str) {
  char *tmpStr = malloc(sizeof(char) * wordsLen);
  size_t ret = removeIncompatibleImpl(filter, nod, -1, str, tmpStr);
  free(tmpStr);
  return ret;
}

void resetCounters(node *nod) {
  nod->connected = 0;
  for (size_t i = 0; i < ALPHALEN; i++) {
    if (nod->next[i] != NULL) {
      //    prob a bug with freeing mem
      if (nod->next[i]->val != '\0') {
        nod->connected += 1;
        resetCounters(nod->next[i]);
      }
    }
  }
}

void freeTree(node *nod) {
  for (size_t i = 0; i < ALPHALEN; i++) {
    if (nod->next[i] != NULL) {
      //    prob a bug with freeing mem
      if (nod->next[i]->val != '\0') {
        freeTree(nod->next[i]);
      }
    }
  }
  free(nod->next);
  free(nod);
}

int main() {
  char *line;
  INIT_LINE_BUFFER();
  fscanf(stdin, "%zu\n", &wordsLen);
  //   wordsLen = strtol(line, NULL, 10);
  node *words = initNode(NULL, '#');

  line = malloc(sizeof(char) * wordsLen);
  NEW_LINE(line);
  while (!feof(stdin)) {
    if (strcmp(line, "+nuova_partita") == 0) {
      break;
    }

    addWord(words, line);
    NEW_LINE(line);
  }

#ifdef DEBUG
  printf("---Initial words----------\n");
  dumpTree(words);
  printf("--------------------------\n");
#endif
  char *referenceWord = malloc(sizeof(char) * wordsLen);
  NEW_LINE(referenceWord);
#ifdef DEBUG
  printf("---Reference word---------\n");
  printf("%s\n", referenceWord);
  printf("--------------------------\n");
#endif
  NEW_LINE(line);
  size_t guessesNumber = (size_t)strtol(line, NULL, 10);
#ifdef DEBUG
  printf("---guesses number---------\n");
  printf("%ld\n", guessesNumber);
  printf("--------------------------\n");
#endif

  bool hasWon = false;
  char *res = malloc(sizeof(char) * wordsLen);
  szArr *storedRes = malloc(sizeof(szArr));
  szArr *storedGuesses = malloc(sizeof(szArr));

  storedRes->v = malloc(sizeof(char *) * guessesNumber);
  storedGuesses->v = malloc(sizeof(char *) * guessesNumber);
  for (size_t i = 0; i < guessesNumber; i++) {
    storedRes->v[i] = malloc(sizeof(char) * wordsLen);
    storedGuesses->v[i] = malloc(sizeof(char) * wordsLen);
  }

  NEW_LINE(line);
  while (!feof(stdin)) {
    for (size_t i = 0; guessesNumber > 0; ++i) {
#ifdef DEBUG
      printf("---Input------------------\n");
      printf("%s\n", line);
      printf("--------------------------\n");
#endif
      if (strcmp("+stampa_filtrate", line) == 0) {
        dumpTree(words);
        NEW_LINE(line);
        continue;

      } else if (strcmp("+inserisci_inizio", line) == 0) {
#ifdef DEBUG
        printf("---Input------------------\n");
#endif
        NEW_LINE(line);
        while (strcmp("+inserisci_fine", line) != 0) {
#ifdef DEBUG
          printf("%s\n", line);
#endif
          addWord(words, line);
          NEW_LINE(line);
        }
#ifdef DEBUG
        printf("---------------------------\n");
        printf("---Fine--------------------\n");
#endif

        for (size_t j = 0; j < storedRes->len; j++) {
          removeIncompatible(storedRes->v[j], words, storedGuesses->v[j]);
        }

        NEW_LINE(line);
        continue;

      } else if (strcmp(referenceWord, line) == 0) {

        printf("ok\n");
        hasWon = true;
        if (!feof(stdin)) {
          NEW_LINE(line);
        }
        guessesNumber = 0;
        break;
      } else if (!inTree(words, line)) {
        printf("not_exists\n");
        NEW_LINE(line);
        continue;
      }

      computeRes(referenceWord, line, res);
      storedRes->len++;
      strcpy(storedRes->v[storedRes->len - 1], res);
      storedGuesses->len++;
      strcpy(storedGuesses->v[storedGuesses->len - 1], line);

      size_t remaining = removeIncompatible(res, words, line);
      printf("%s\n%ld\n", res, remaining);
      guessesNumber--;
      NEW_LINE(line);
    }

    if (!hasWon) {
      hasWon = !hasWon;
      printf("ko\n");
    }
    if (strcmp("+inserisci_inizio", line) == 0) {
#ifdef DEBUG
      printf("---Input------------------\n");
#endif
      NEW_LINE(line);
      while (strcmp("+inserisci_fine", line) != 0) {
#ifdef DEBUG
        printf("%s\n", line);
#endif
        addWord(words, line);
        NEW_LINE(line);
      }
#ifdef DEBUG
      printf("---------------------------\n");
      printf("---Fine--------------------\n");
#endif

      for (size_t j = 0; j < storedRes->len; j++) {
        removeIncompatible(storedRes->v[j], words, storedGuesses->v[j]);
      }

      NEW_LINE(line);
      continue;
    }
    if (strcmp("+nuova_partita", line) == 0) {
      resetCounters(words);
#ifdef DEBUG
      printf("---Initial words----------\n");
      dumpTree(words);
      printf("--------------------------\n");
#endif
      NEW_LINE(referenceWord);
#ifdef DEBUG
      printf("---Reference word---------\n");
      printf("%s\n", referenceWord);
      printf("--------------------------\n");
#endif
      NEW_LINE(line);
      guessesNumber = (size_t)strtol(line, NULL, 10);
#ifdef DEBUG
      printf("---guesses number---------\n");
      printf("%ld\n", guessesNumber);
      printf("--------------------------\n");
#endif
      storedRes->len = 0;
      storedGuesses->len = 0;
    }
    NEW_LINE(line);
  }
  // TODO Free everything
  free(res);
  freeTree(words);
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
