#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma pack(1)

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
  uint16_t strLen;
  uint16_t nodeLen;
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

inline size_t getIndex(char c, uint64_t mask) {
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
  n->strLen = len + 1;
  n->nodeLen = len;
  n->next = malloc(sizeof(node));
  return (void *)n;
}

size_t getInlinedIndex(node *nod, char c, size_t inlineLen) {
  for (size_t i = nod->nodeLen + 1; i + 1 < nod->strLen; i += 1 + inlineLen) {
    if (*(nod->str + i + 1) == c)
      return i;
  }
  return 0;
}

#define hasInline(x) (x->nodeLen + 1 != x->strLen)
#define NOTHING_TO_DO 0
#define NEW_NODE_FROM_ROOT 1
#define INIT_INLINE 3
#define INIT_INLINE_WITH_NEW 4
#define ADD_TO_INLINE 5
#define CUT_NODE 6
#define CUT_FROM_INLINE 7
#define MAKE_INBETWEEN_NODE 8
void addWord(node *nod, char *newWord) {
  node *tmp = nod;
  size_t inlinePos = 0;
  size_t state = NOTHING_TO_DO;
  uint32_t i = 0, sameChars = 0;
  if ((GET_BIT(nod->mask, alph[(uint8_t)*newWord])) == 0) {
    state = NEW_NODE_FROM_ROOT;
  } else {
    nod = nod->next[getIndex(*newWord, nod->mask)];

    for (i = 0; i < wordsLen; i++) {
      if (nod->deleted == 1)
        nod->deleted = 0;

      if (nod->str[sameChars] == newWord[i]) {
        sameChars++;
        continue;
      }
      if (nod->str[sameChars] == '\0') {

        if ((GET_BIT(nod->mask, alph[(uint8_t) * (newWord + i)])) == 1) {
          node *parent = nod;
          nod = nod->next[getIndex(*(newWord + i), nod->mask)];
          if (nod->nodeLen == 1) {
            sameChars = 0;
            i--;
            continue;
          }
          if (sameChars != nod->nodeLen) {
            if ((GET_BIT(parent->mask, alph[(uint8_t) * (newWord + i)])) == 1 &&
                sameChars == 1) {
              i += sameChars;
              state = MAKE_INBETWEEN_NODE;
            } else
              state = INIT_INLINE;
            break;
          } else {
            i += sameChars;
            state = ADD_TO_INLINE;
            break;
          }
        } else if ((inlinePos = getInlinedIndex(nod, *(newWord + i),
                                                wordsLen - i)) != 0) {
          state = CUT_FROM_INLINE;
          break;
        } else {
          if (hasInline(nod))
            state = ADD_TO_INLINE;
          else
            state = INIT_INLINE_WITH_NEW;
          break;
        }
        sameChars = 0;
      } else {
        if (!hasInline(nod)) {
          state = INIT_INLINE;
          break;
        } else {
          state = CUT_NODE;
          break;
        }
      }
    }
  }
  node *newNode;
  node *tmpNode;
  uint16_t newPos = 0;
  size_t newInd;
  size_t newLen = wordsLen - i, newNodeLen = 0;
  while (state != NOTHING_TO_DO) {
    switch (state) {
    case NEW_NODE_FROM_ROOT:
//       printf("NEW_NODE_FROM_ROOT\n");

      nod->next = realloc(nod->next, sizeof(node) * (nod->connectedNodes + 1));
      newInd = getIndex(newWord[0], nod->mask);
      if (newInd < nod->connectedNodes)
        for (size_t j = nod->connectedNodes; j > newInd; j--)
          nod->next[j] = nod->next[j - 1];

      nod->next[newInd] = initNode((newWord), wordsLen);
      SET_BIT(nod->mask, alph[(uint8_t)*newWord]);
      nod->connectedNodes++;

      state = NOTHING_TO_DO;
      break;
    case INIT_INLINE:
//       printf("INIT_INLINE\n");

      if (sameChars == 0)
        sameChars = i;
      // 3 = 1 per \0 dopo parola, 1 per # e 1 per ultimo \0
      // nel calcolo sameChars si cancella
      nod->strLen = nod->nodeLen + 3;
      nod->str = realloc(nod->str, sizeof(char) * (nod->strLen));
      memcpy(nod->str + sameChars + 2, nod->str + sameChars,
             sizeof(char) * (nod->nodeLen - sameChars));
      nod->nodeLen = sameChars;
      nod->str[sameChars + 1] = '#';
      nod->str[nod->strLen - 1] = '\0';
      nod->str[sameChars] = '\0';
      state = ADD_TO_INLINE;
      continue;
    case ADD_TO_INLINE:
//       printf("ADD_TO_INLINE\n");

      newPos = nod->strLen;
      nod->strLen += wordsLen - i + 1;
      nod->str = realloc(nod->str, sizeof(char) * (nod->strLen));
      newLen = wordsLen - i;
      // slide to the right to order
      for (uint16_t j = nod->strLen - (newLen + 1);
           j > nod->nodeLen + 3 + newLen + 1; j -= (1 + newLen)) {
        if (*(nod->str + j) < *(newWord + i)) {
          memcpy(nod->str + j, nod->str + j - (newLen + 1),
                 sizeof(char) * newLen + 1);
          newPos -= 1 + newLen;
        }
      }
      nod->str[newPos - 1] = '#';
      memcpy(nod->str + newPos, newWord + i, sizeof(char) * (newLen));
      nod->str[nod->strLen - 1] = '\0';
      state = NOTHING_TO_DO;
      break;
    case CUT_NODE:
//       printf("CUT_NODE\n");

      newNode = initNode(nod->str, sameChars);
      tmpNode = malloc(sizeof(node));

      nod->nodeLen -= sameChars;
      nod->strLen -= sameChars;
      nod->str = memmove(nod->str, nod->str + sameChars,
                         sizeof(char) * (nod->strLen));
      nod->str = realloc(nod->str, sizeof(char) * (nod->strLen));
      nod->str[sameChars] = '\0';
      tmpNode = memcpy(tmpNode, newNode, sizeof(node));
      newNode = memcpy(newNode, nod, sizeof(node));
      nod = memcpy(nod, tmpNode, sizeof(node));
      nod->next[0] = newNode;
      nod->connectedNodes++;

      SET_BIT(nod->mask, alph[(uint8_t)*newNode->str]);
      free(tmpNode);

      nod->next = realloc(nod->next, sizeof(node) * (nod->connectedNodes + 1));
      newInd = getIndex(newWord[i], nod->mask);

      if (newInd < nod->connectedNodes)
        for (size_t j = nod->connectedNodes; j > newInd; j--)
          nod->next[j] = nod->next[j - 1];

      nod->next[newInd] = initNode(newWord + i, wordsLen - i);
      SET_BIT(nod->mask, alph[(uint8_t) * (newWord + i)]);
      nod->connectedNodes++;

      state = NOTHING_TO_DO;
      break;
    case MAKE_INBETWEEN_NODE:
//       printf("MAKE_INBETWEEN_NODE\n");
      newNode = initNode(nod->str, sameChars);
      tmpNode = malloc(sizeof(node));
      nod->nodeLen -= sameChars;
      nod->strLen -= sameChars;

      nod->str = memmove(nod->str, nod->str + sameChars,
                         sizeof(char) * (nod->strLen));
      nod->str = realloc(nod->str, sizeof(char) * (nod->strLen));
      nod->str[sameChars] = '\0';
      tmpNode = memcpy(tmpNode, newNode, sizeof(node));
      newNode = memcpy(newNode, nod, sizeof(node));
      nod = memcpy(nod, tmpNode, sizeof(node));
      nod->next[0] = newNode;
      nod->connectedNodes++;

      SET_BIT(nod->mask, alph[(uint8_t)*newNode->str]);
      free(tmpNode);
      state = INIT_INLINE_WITH_NEW;
      continue;
    case INIT_INLINE_WITH_NEW:
//       printf("INIT_INLINE_WITH_NEW\n");
      // 3 = 1 per \0 dopo parola, 1 per # e 1 per ultimo \0
      // nel calcolo sameChars si cancella
      nod->strLen = nod->nodeLen + 3 + wordsLen - i;
      nod->str = realloc(nod->str, sizeof(char) * (nod->strLen));
      memcpy(nod->str + nod->nodeLen + 2, newWord + i,
             sizeof(char) * (wordsLen - i));

      nod->str[nod->nodeLen + 1] = '#';
      nod->str[nod->strLen] = '\0';

      state = NOTHING_TO_DO;
      break;
    case CUT_FROM_INLINE:
//       printf("CUT_FROM_INLINE\n");
      newNodeLen = 0;
      // il +1 è per skippare il #
      while (nod->str[inlinePos + newNodeLen + 1] == newWord[i + newNodeLen])
        newNodeLen++;
      newNode = initNode(newWord + i, newNodeLen);
      newLen = wordsLen - newNodeLen - i;

      newNode->strLen = newNode->nodeLen + 3 + newLen;
      newNode->str = realloc(newNode->str, sizeof(char) * (newNode->strLen));
      memcpy(newNode->str + newNode->nodeLen + 2,
             nod->str + inlinePos + 1 + newNodeLen,
             sizeof(char) * (wordsLen - i));

      newNode->str[newNode->nodeLen + 1] = '#';
      newNode->str[newNode->strLen - 1] = '\0';
      nod->next = realloc(nod->next, sizeof(node) * (nod->connectedNodes + 1));

      newInd = getIndex(*newNode->str, nod->mask);
      if (newInd < nod->connectedNodes)
        for (size_t j = nod->connectedNodes; j > newInd; j--)
          nod->next[j] = nod->next[j - 1];
      nod->connectedNodes++;

      nod->next[newInd] = newNode;
      SET_BIT(nod->mask, alph[(uint8_t)*newNode->str]);

      newLen += newNodeLen;
      for (uint16_t j = inlinePos; j < nod->strLen - newLen - 1;
           j += 1 + nod->nodeLen) {
        memcpy(nod->str + j, nod->str + j + newLen + 1,
               sizeof(char) * (newLen + 1));
      }

      nod->strLen -= 1 + newLen;
      nod->str = realloc(nod->str, sizeof(char) * (nod->strLen));
      nod->str[nod->strLen - 1] = '\0';
      i += newNodeLen;

      nod = newNode;
      state = ADD_TO_INLINE;
      break;
    }
  }
  nod = tmp;
  wordsCount++;
}
int dumpCont = 0;
void dumpTreeImpl(node *nod, size_t depth, char *tmpStr) {
  if (nod->deleted == 0 && depth == wordsLen) {
    dumpCont++;
    tmpStr[wordsLen] = '\0';
    printf("%s %d\n", tmpStr, dumpCont);
    return;
  }

  bool finished = false;
  size_t i = 0, j = nod->nodeLen + 1;
  while (!finished) {
    char *tmp = tmpStr;

    if (j + 1 < nod->strLen && i < nod->connectedNodes) {
      if (nod->next[i]->deleted > 0) {
        i++;
        continue;
      }
      if (*(nod->str + j) == '|') {
        j += 1 + (wordsLen - depth);
        continue;
      }
      if (*(nod->next[i]->str) < *(nod->str + j + 1)) {
        memcpy(tmp + depth, nod->next[i]->str, nod->next[i]->nodeLen);
        dumpTreeImpl(nod->next[i], depth + nod->next[i]->nodeLen, tmpStr);
        i++;
      } else {
        memcpy(tmp + depth, nod->str + j + 1, wordsLen - depth);
        dumpTreeImpl(nod, wordsLen, tmpStr);
        j += 1 + (wordsLen - depth);
      }
    } else if (!(j + 1 < nod->strLen) && i < nod->connectedNodes) {
      if (nod->next[i]->deleted > 0) {
        i++;
        continue;
      }

      memcpy(tmp + depth, nod->next[i]->str, nod->next[i]->nodeLen);
      dumpTreeImpl(nod->next[i], depth + nod->next[i]->nodeLen, tmpStr);
      i++;
    } else if ((j + 1 < nod->strLen && !(i < nod->connectedNodes))) {
      if (*(nod->str + j) == '|') {
        j += 1 + (wordsLen - depth);
        continue;
      }
      memcpy(tmp + depth, nod->str + j + 1, wordsLen - depth);
      dumpTreeImpl(nod, wordsLen, tmpStr);
      j += 1 + (wordsLen - depth);
    } else
      finished = true;
  }
}

void dumpTree(node *nod) {
  char *tmpStr = malloc(sizeof(char) * (wordsLen + 1));
  dumpTreeImpl(nod, 0, tmpStr);
  free(tmpStr);
}

bool inTree(node *nod, char *needle) {
  node *tmp = nod;
  for (size_t i = 0; i < wordsLen; i++) {
    if ((GET_BIT(tmp->mask, alph[(uint8_t)*needle])) == 0) {
      return false;
    }
    tmp = tmp->next[getIndex(*needle, tmp->mask)];
    size_t len = tmp->nodeLen;
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
  printf("%*s%s ", depth, "", nod->str);
  if (nod->nodeLen != nod->strLen)
    printf("%s ", nod->str + nod->nodeLen + 1);

  if (nod->connectedNodes != 0)
    printf("len -> %d strLen = %d connected -> %d\n", nod->nodeLen, nod->strLen,
           nod->connectedNodes);
  else {
    cont++;
    printf("len -> %d %d\n", nod->nodeLen, cont);
  }

  for (uint32_t i = 0; i < nod->connectedNodes; i++)
    printTree(nod->next[i], depth + nod->nodeLen);
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

  size_t contDeleted = 0;

  for (size_t i = 0; i < nod->connectedNodes; i++) {
    node *next = nod->next[i];
    contDeleted += next->deleted;
    if (next->deleted == 1)
      continue;

    size_t len = next->strLen;
    char *tmp = tmpStr;
    tmp += depth;
    memcpy(tmp, next->str, len);

    removeIncompatibleImpl(filter, next, depth + len, str, tmpStr);
  }
  if (contDeleted == nod->connectedNodes && nod->connectedNodes != 0) {
    nod->deleted = 1;
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
  if (hasInline(nod)){
    for (size_t i = nod->nodeLen + 2; i < nod->strLen; i++)
      *nod->str = '#';
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
//   printTree(words, 0);
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
