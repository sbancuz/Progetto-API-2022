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

#define SET_BIT(m, i) m = ((uint64_t)1 << i) | m
#define GET_BIT(m, i) (m >> i) & 1

typedef struct node_s {
  char *str;
  uint8_t connectedNodes;
  uint8_t deleted;
  uint64_t mask;
  struct node_s **next;
} node;

typedef struct sizedArr_s {
  char **v;
  uint32_t len;
} szArr;

enum command { INS_INI, INS_FIN, PRINT, INIZIO, ELSE };
enum command getCommand(char *comm) {
  if (comm[0] != '+')
    return ELSE;
  if (comm[1] == 'i') {
    if (comm[11] == 'i')
      return INS_INI;
    else
      return INS_FIN;
  }
  if (comm[1] == 's')
    return PRINT;
  return INIZIO;
}

size_t wordsLen;
size_t wordsCount = 0;
uint8_t maskRefs[UINT8_MAX + 1] = {
    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4, 1, 2, 2, 3, 2, 3, 3, 4,
    2, 3, 3, 4, 3, 4, 4, 5, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 1, 2, 2, 3, 2, 3, 3, 4,
    2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6,
    4, 5, 5, 6, 5, 6, 6, 7, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 2, 3, 3, 4, 3, 4, 4, 5,
    3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6,
    4, 5, 5, 6, 5, 6, 6, 7, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8};

uint8_t alph[INT8_MAX] = {
    65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
    65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
    65, 65, 65, 65, 65, 65, 65, 0,  65, 65, 1,  2,  3,  4,  5,  6,  7,  8,  9,
    10, 65, 65, 65, 65, 65, 65, 65, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21,
    22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 65, 65, 65, 65,
    37, 65, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54,
    55, 56, 57, 58, 59, 60, 61, 62, 63, 65, 65, 65, 65};

uint16_t *no;
uint16_t *co;
uint16_t *sc;

size_t getIndex(char c, uint64_t mask) {
  size_t target = alph[(uint8_t)c];
  size_t cont = 0;
  for (size_t i = 0; i < target / 8; i++) {
    cont += maskRefs[(mask >> (i * 8)) & 0xFF];
  }
  for (size_t i = 0; i < target % 8; i++) {
    cont += (mask >> (target - i - 1)) & 1;
  }
  return cont;
}

void *initNode(char *c, uint32_t len) {
  node *n = malloc(sizeof(node));
  memset(n, 0, sizeof(node));
  n->str = malloc(sizeof(char) * len + 1);
  n->str = memcpy(n->str, c, sizeof(char) * len);
  n->str[len] = '\0';
  n->next = malloc(sizeof(node));
  return (void *)n;
}

void addWord(node *nod, char *newWord) {
  node *tmp = nod;
  if ((GET_BIT(nod->mask, alph[(uint8_t)*newWord])) == 0) {
    nod->next = realloc(nod->next, sizeof(node) * (nod->connectedNodes + 1));
    size_t newInd = getIndex(newWord[0], nod->mask);
    if (newInd < nod->connectedNodes)
      for (size_t j = nod->connectedNodes; j > newInd; j--)
        nod->next[j] = nod->next[j - 1];

    nod->next[newInd] = initNode((newWord), wordsLen);
    SET_BIT(nod->mask, alph[(uint8_t)*newWord]);
    nod->connectedNodes++;
  }
  nod = nod->next[getIndex(*newWord, nod->mask)];

  for (uint32_t i = 0; i < wordsLen; i++) {
    if (nod->str[i] == newWord[i])
      continue;

    if (nod->connectedNodes == 0 ||
        (nod->str[i] == '\0' &&
         (GET_BIT(nod->mask, alph[(uint8_t) * (newWord + i)])) == 1)) {

      if (nod->str[i] == '\0') {
        nod = nod->next[getIndex(*(newWord + i), nod->mask)];
        if (strlen(nod->str) == 1)
          continue;
        if (nod->connectedNodes > 0) {
          uint32_t sameChars = 1;
          while (nod->str[sameChars] == newWord[i + sameChars])
            sameChars++;
          node *newNode = initNode(nod->str, sameChars);
          node *tmpNode = malloc(sizeof(node));
          nod->str += sameChars;
          // can use memmove to not make memleaks...
          tmpNode = memcpy(tmpNode, newNode, sizeof(node));
          newNode = memcpy(newNode, nod, sizeof(node));
          nod = memcpy(nod, tmpNode, sizeof(node));
          nod->next[0] = newNode;
          SET_BIT(nod->mask, alph[(uint8_t) * (newNode->str)]);
          i += sameChars;
        } else {
          uint32_t sameChars = 0;
          while (nod->str[sameChars] == newWord[i + sameChars])
            sameChars++;
          nod->next[0] =
              initNode(nod->str + sameChars, wordsLen - i - sameChars);
          i += sameChars;
          SET_BIT(nod->mask, alph[(uint8_t) * (nod->str + sameChars)]);
          nod->str[sameChars] = '\0';
        }
      } else {
        nod->next[0] = initNode(nod->str + i, wordsLen - i);
        SET_BIT(nod->mask, alph[(uint8_t) * (nod->str + i)]);
      }
      nod->connectedNodes++;
      nod->str[i] = '\0';
    } else if (nod->str[i] != '\0' && strlen(nod->str) != 1) {
      uint32_t sameChars = 0;
      while (nod->str[sameChars] == newWord[sameChars])
        sameChars++;
      node *newNode = initNode(nod->str, sameChars);
      node *tmpNode = malloc(sizeof(node));
      // can use memmove to not make memleaks...
      nod->str += sameChars;
      tmpNode = memcpy(tmpNode, newNode, sizeof(node));
      newNode = memcpy(newNode, nod, sizeof(node));
      nod = memcpy(nod, tmpNode, sizeof(node));
      nod->next[0] = newNode;
      nod->connectedNodes++;
      SET_BIT(nod->mask, alph[(uint8_t)*newNode->str]);
      free(tmpNode);
    }
    nod->next = realloc(nod->next, sizeof(node) * (nod->connectedNodes + 1));
    size_t newInd = getIndex(newWord[i], nod->mask);

    if (newInd < nod->connectedNodes)
      for (size_t j = nod->connectedNodes; j > newInd; j--)
        nod->next[j] = nod->next[j - 1];

    nod->next[newInd] = initNode(newWord + i, wordsLen - i);
    SET_BIT(nod->mask, alph[(uint8_t) * (newWord + i)]);
    nod->connectedNodes++;
    break;
  }
  nod = tmp;
  wordsCount++;
}


void dumpTreeImpl(node *nod, size_t depth, char *tmpStr) {
  if (nod->deleted == 0 && depth == wordsLen) {
    tmpStr[wordsLen] = '\0';
    printf("%s\n", tmpStr);
    return;
  }

  for (size_t i = 0; i < nod->connectedNodes; i++) {
    if (nod->next[i]->deleted == 1)
      continue;
    size_t len = strlen(nod->next[i]->str);
    char *tmp = tmpStr;
    tmp += depth;
    memcpy(tmp, nod->next[i]->str, len);
    size_t oldDepth = depth;
    depth += len;
    dumpTreeImpl(nod->next[i], depth, tmpStr);
    depth = oldDepth;
  }
}

void dumpTree(node *nod) {
  char *tmpStr = malloc(sizeof(char) * (wordsLen + 1));
  dumpTreeImpl(nod, 0, tmpStr);
}

bool inTree(node *nod, char *needle) {
  node *tmp = nod;
  for (size_t i = 0; i < wordsLen; i++) {
    if ((GET_BIT(tmp->mask, alph[(uint8_t)*needle])) == 0) {
      return false;
    }
    tmp = tmp->next[getIndex(*needle, tmp->mask)];
    size_t len = strlen(tmp->str);
    for (size_t j = 0; j < len; j++) {
      if (needle[j] != tmp->str[j]) {
        return false;
      }
    }
    needle += len;
    i += len - 1;
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
  size_t c = 0, s = 0;
  for (size_t i = 0; r[i] != '\0'; ++i) {
    size_t n = count(r, p[i]);
    if (r[i] == p[i]) {
      res[i] = '+';
    } else if (n == 0)
      res[i] = '/';
    else {
      c = 0;
      s = 0;
      for (size_t j = 0; r[j] != '\0'; ++j) {
        if (j < i && p[j] == p[i] && p[j] != r[j])
          s++;
        if (p[i] == r[j] && p[j] == r[j])
          c++;
      }

      if (s >= (n - c))
        res[i] = '/';
      else
        res[i] = '|';
    }
  }
}

bool compatible(char *filter, char *r, char *p) {
  size_t n = 0, c = 0, s = 0;
  bool sameStr = true;
  for (size_t i = 0; filter[i] != '\0'; ++i) {
    if (p[i] != r[i])
      sameStr = false;

    if (filter[i] == '+') {
      if (r[i] != p[i])
        return false;
      else
        continue;
    }

    n = 0;
    c = 0;
    s = 0;
    for (size_t j = 0; filter[j] != '\0'; ++j) {
      if (j < i && p[j] == p[i] && p[j] != r[j])
        s++;
      if (p[i] == r[j]) {
        if (p[j] == r[j])
          c++;
        n++;
      }
    }

#ifdef DEBUG
    printf(" %c - n = %ld, c = %ld, s = %ld\n", p[i], n, c, s);
#endif
    if (filter[i] == '|') {
      if ((r[i] == p[i] || n == 0 || s >= n - c))
        return false;
      else
        continue;
    }

    if (filter[i] == '/' && (s < n - c || (n != 0 && r[i] == p[i])))
      return false;
  }

  return !sameStr;
}

void printTree(node *nod, int depth) {
  int cont = 0;
  if (nod->str[0] == '#')
    cont = 0;
  if (nod->connectedNodes != 0)
    printf("%*s%s %d\n", depth, "", nod->str, nod->connectedNodes);
  else {
    cont++;
    printf("%*s%s %d\n", depth, "", nod->str, cont);
  }
  for (uint32_t i = 0; i < nod->connectedNodes; i++)
    printTree(nod->next[i], depth + strlen(nod->str));
}

void removeIncompatibleImpl(char *filter, node *nod, size_t depth, char *str,
                            char *tmpStr) {
  if (nod->deleted == 0 && depth == wordsLen) {
    tmpStr[depth + 1] = '\0';

#ifdef DEBUG
    printf("----compatibility test----\n");
    printf("%s - %s - %s\n", filter, tmpStr, str);
#endif
    if (!compatible(filter, tmpStr, str)) {
#ifdef DEBUG
      printf("incompatibile\n");
      printf("--------------------------\n");
#endif
      wordsCount--;
      nod->deleted = 1;
    }
#ifdef DEBUG
    else {
      printf("compatibile\n");
      printf("--------------------------\n");
    }
#endif
    return;
  }

  for (size_t i = 0; i < nod->connectedNodes; i++) {
    if (nod->next[i]->deleted == 1)
      continue;
    size_t len = strlen(nod->next[i]->str);
    char *tmp = tmpStr;
    tmp += depth;
    memcpy(tmp, nod->next[i]->str, len);
    size_t oldDepth = depth;
    depth += len;
    removeIncompatibleImpl(filter, nod->next[i], depth, str, tmpStr);
    depth = oldDepth;
  }
  return;
}

void removeIncompatible(char *filter, node *nod, char *str) {
  char *tmpStr = malloc(sizeof(char) * (wordsLen + 1));
  removeIncompatibleImpl(filter, nod, 0, str, tmpStr);
  free(tmpStr);
}

void resetCounters(node *nod) {
  nod->deleted = 0;
  for (size_t i = 0; i < nod->connectedNodes; i++) {
    resetCounters(nod->next[i]);
  }
}

void freeTree(node *nod) {
  for (size_t i = 0; i < nod->connectedNodes; i++) {
    freeTree(nod->next[i]);
  }
  free(nod->next);
  free(nod);
}

int main() {
  char *line;
  no = malloc(sizeof(uint16_t) * ALPHALEN);
  co = malloc(sizeof(uint16_t) * ALPHALEN);
  sc = malloc(sizeof(uint16_t) * wordsLen);
  INIT_LINE_BUFFER();
  NEW_LINE(line);

  //   (void) fscanf(stdin, "%zu\n", &wordsLen);
  wordsLen = strtol(line, NULL, 10);
  node *words = initNode("#", 1);
  for (size_t i = 0; i < ALPHALEN; i++) {
    no[i] = 0;
    co[i] = 0;
    if (i < wordsLen)
      sc[i] = 0;
  }
  line = malloc(sizeof(char) * wordsLen);
  NEW_LINE(line);
#ifdef DEBUG
  printf("---Input check------------\n");
#endif
  while (!feof(stdin)) {
    if (getCommand(line) == INIZIO) {
      break;
    }
#ifdef DEBUG
    printf("----%s---------------\n", line);
#endif
    addWord(words, line);
    NEW_LINE(line);
  }

  size_t wordsCountBkp = wordsCount;
#ifdef DEBUG
  printf("--------------------------\n");
#endif
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
  bool finished = false;
  char *res = malloc(sizeof(char) * wordsLen);
  szArr *storedRes = malloc(sizeof(szArr));
  szArr *storedGuesses = malloc(sizeof(szArr));

  storedRes->len = 0;
  storedGuesses->len = 0;
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
      if (getCommand(line) == PRINT) {
        dumpTree(words);
        NEW_LINE(line);
        continue;

      } else if (getCommand(line) == INS_INI) {
#ifdef DEBUG
        printf("---Input------------------\n");
#endif
        NEW_LINE(line);
        while (getCommand(line) != INS_FIN) {
#ifdef DEBUG
          printf("%s\n", line);
#endif
          addWord(words, line);
          wordsCountBkp++;
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
        hasWon = true;
        if (!feof(stdin)) {
          NEW_LINE(line);
        }
        guessesNumber = 0;
        break;
      } else if (!inTree(words, line)) {
#ifdef DEBUG
        printf("---------------------------\n");
        printf("%s\n", line);
#endif
        printf("not_exists\n");
#ifdef DEBUG
        printf("---------------------------\n");
#endif
        NEW_LINE(line);
        continue;
      }
      computeRes(referenceWord, line, res);
      res[wordsLen] = '\0';
      storedRes->len++;
      strcpy(storedRes->v[storedRes->len - 1], res);
      storedGuesses->len++;
      strcpy(storedGuesses->v[storedGuesses->len - 1], line);

      removeIncompatible(res, words, line);
#ifdef DEBUG
//       printTree(words, 0);
#endif
      printf("%s\n%ld\n", res, wordsCount);
      guessesNumber--;
      NEW_LINE(line);
    }
    if (!finished) {
      printf("%s\n", hasWon ? "ok" : "ko");
      hasWon = false;
      finished = true;
    }

    if (getCommand(line) == INS_INI) {
#ifdef DEBUG
      printf("---Input------------------\n");
#endif
      NEW_LINE(line);
      while (getCommand(line) != INS_FIN) {
#ifdef DEBUG
        printf("%s\n", line);
#endif
        // does't matter if it's compatibile or not because it's going to be
        // reset after
        addWord(words, line);
        wordsCountBkp++;
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
    if (getCommand(line) == INIZIO) {
      resetCounters(words);
#ifdef DEBUG
      printf("---Initial words----------\n");
      printf("%ld\n", wordsCount);
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
      if (storedRes->len < guessesNumber) {
        storedRes->v = realloc(storedRes->v, sizeof(char *) * guessesNumber);
        storedGuesses->v =
            realloc(storedGuesses->v, sizeof(char *) * guessesNumber);
        for (size_t i = storedRes->len; i < guessesNumber; i++) {
          storedRes->v[i] = malloc(sizeof(char) * wordsLen);
          storedGuesses->v[i] = malloc(sizeof(char) * wordsLen);
        }
      }
      storedRes->len = 0;
      storedGuesses->len = 0;
      finished = false;
      wordsCount = wordsCountBkp;
    }
    NEW_LINE(line);
  }
  // TODO Free everything
  //   free(res);
  //   freeTree(words);
  //   for (size_t i = 0; i < storedRes->len; i++) {
  //     free(storedRes->v[i]);
  //     free(storedGuesses->v[i]);
  //   }
  //   free(storedRes->v);
  //   free(storedGuesses->v);
  //   free(storedRes);
  //   free(storedGuesses);
  return 0;
}
