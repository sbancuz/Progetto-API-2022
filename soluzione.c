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
  ssize_t input_string_lenght = 0

#define NEW_LINE(a)                                                            \
  input_string_lenght = getline(&a, &len, stdin);                                     \
  a[input_string_lenght - 1] = '\0'

#define SET_BIT(m, i) m = ((uint64_t)1 << i) | m
#define GET_BIT(m, i) (m >> i) & 1

typedef struct node_s {
  char *str;
  uint16_t str_lenght;
  uint16_t node_lenght;
  uint8_t connected_nodes;
  uint8_t deleted;
  uint64_t mask;
  struct node_s **next;
} node;

typedef struct sizedArr_s {
  char **v;
  uint32_t len;
} szArr;

enum command { INS_INI, INS_FIN, PRINT, INIZIO, ELSE };
enum command parse_command(char *comm) {
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

size_t words_lenght;
size_t words_count = 0;
uint8_t masks_lookup_table[UINT8_MAX + 1] = {
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

inline size_t get_index(char c, uint64_t mask) {
  size_t target = alph[(uint8_t)c];
  size_t cont = 0;
  for (size_t i = 0; i < target / 8; i++) {
    cont += masks_lookup_table[(mask >> (i * 8)) & 0xFF];
  }
  for (size_t i = 0; i < target % 8; i++) {
    cont += (mask >> (target - i - 1)) & 1;
  }
  return cont;
}

void *init_node(char *c, uint32_t len) {
  node *n = malloc(sizeof(node));
  memset(n, 0, sizeof(node));
  n->str = malloc(sizeof(char) * len + 1);
  n->str = memcpy(n->str, c, sizeof(char) * len);
  n->str[len] = '\0';
  n->str_lenght = len + 1;
  n->node_lenght = len;
  n->next = malloc(sizeof(node));
  return (void *)n;
}

size_t get_inlined_index(node *nod, char c, size_t inline_lenght) {
  for (size_t i = nod->node_lenght + 1; i + 1 < nod->str_lenght; i += 1 + inline_lenght) {
    if (*(nod->str + i + 1) == c)
      return i;
  }
  return 0;
}

#define has_inline(x) (x->node_lenght + 1 != x->str_lenght)
#define NOTHING_TO_DO 0
#define NEW_NODE_FROM_ROOT 1
#define INIT_INLINE 3
#define INIT_INLINE_WITH_NEW 4
#define ADD_TO_INLINE 5
#define CUT_NODE 6
#define CUT_FROM_INLINE 7
#define MAKE_INBETWEEN_NODE 8
void add_word(node *nod, char *new_word) {
  node *tmp = nod;
  size_t inline_index = 0;
  size_t state = NOTHING_TO_DO;
  uint32_t i = 0, same_chars = 0;
  if ((GET_BIT(nod->mask, alph[(uint8_t)*new_word])) == 0) {
    state = NEW_NODE_FROM_ROOT;
  } else {
    nod = nod->next[get_index(*new_word, nod->mask)];

    for (i = 0; i < words_lenght; i++) {
      if (nod->deleted == 1)
        nod->deleted = 0;

      if (nod->str[same_chars] == new_word[i]) {
        same_chars++;
        continue;
      }
      if (nod->str[same_chars] == '\0') {

        if ((GET_BIT(nod->mask, alph[(uint8_t) * (new_word + i)])) == 1) {
          node *parent = nod;
          nod = nod->next[get_index(*(new_word + i), nod->mask)];
          if (nod->node_lenght == 1) {
            same_chars = 0;
            i--;
            continue;
          }
          if (same_chars != nod->node_lenght) {
            if ((GET_BIT(parent->mask, alph[(uint8_t) * (new_word + i)])) == 1 &&
                same_chars == 1) {
              i += same_chars;
              state = MAKE_INBETWEEN_NODE;
            } else
              state = INIT_INLINE;
            break;
          } else {
            i += same_chars;
            state = ADD_TO_INLINE;
            break;
          }
        } else if ((inline_index = get_inlined_index(nod, *(new_word + i),
                                                words_lenght - i)) != 0) {
          state = CUT_FROM_INLINE;
          break;
        } else {
          if (has_inline(nod))
            state = ADD_TO_INLINE;
          else
            state = INIT_INLINE_WITH_NEW;
          break;
        }
        same_chars = 0;
      } else {
        if (!has_inline(nod)) {
          state = INIT_INLINE;
          break;
        } else {
          state = CUT_NODE;
          break;
        }
      }
    }
  }
  node *new_node;
  node *tmp_node;
  uint16_t new_index_in_inline = 0;
  size_t new_next_index;
  size_t new_inlined_lenght = words_lenght - i, new_nodeLen = 0;
  while (state != NOTHING_TO_DO) {
    switch (state) {
    case NEW_NODE_FROM_ROOT:
//       printf("NEW_NODE_FROM_ROOT\n");

      nod->next = realloc(nod->next, sizeof(node) * (nod->connected_nodes + 1));
      new_next_index = get_index(new_word[0], nod->mask);
      if (new_next_index < nod->connected_nodes)
        for (size_t j = nod->connected_nodes; j > new_next_index; j--)
          nod->next[j] = nod->next[j - 1];

      nod->next[new_next_index] = init_node((new_word), words_lenght);
      SET_BIT(nod->mask, alph[(uint8_t)*new_word]);
      nod->connected_nodes++;

      state = NOTHING_TO_DO;
      break;
    case INIT_INLINE:
//       printf("INIT_INLINE\n");

      if (same_chars == 0)
        same_chars = i;
      // 3 = 1 per \0 dopo parola, 1 per # e 1 per ultimo \0
      // nel calcolo same_chars si cancella
      nod->str_lenght = nod->node_lenght + 3;
      nod->str = realloc(nod->str, sizeof(char) * (nod->str_lenght));
      memcpy(nod->str + same_chars + 2, nod->str + same_chars,
             sizeof(char) * (nod->node_lenght - same_chars));
      nod->node_lenght = same_chars;
      nod->str[same_chars + 1] = '#';
      nod->str[nod->str_lenght - 1] = '\0';
      nod->str[same_chars] = '\0';
      state = ADD_TO_INLINE;
      continue;
    case ADD_TO_INLINE:
//       printf("ADD_TO_INLINE\n");

      new_index_in_inline = nod->str_lenght;
      nod->str_lenght += words_lenght - i + 1;
      nod->str = realloc(nod->str, sizeof(char) * (nod->str_lenght));
      new_inlined_lenght = words_lenght - i;
      // slide to the right to order
      for (uint16_t j = nod->str_lenght - (new_inlined_lenght + 1);
           j > nod->node_lenght + 3 + new_inlined_lenght + 1; j -= (1 + new_inlined_lenght)) {
        if (*(nod->str + j) < *(new_word + i)) {
          memcpy(nod->str + j, nod->str + j - (new_inlined_lenght + 1),
                 sizeof(char) * new_inlined_lenght + 1);
          new_index_in_inline -= 1 + new_inlined_lenght;
        }
      }
      nod->str[new_index_in_inline - 1] = '#';
      memcpy(nod->str + new_index_in_inline, new_word + i, sizeof(char) * (new_inlined_lenght));
      nod->str[nod->str_lenght - 1] = '\0';
      state = NOTHING_TO_DO;
      break;
    case CUT_NODE:
//       printf("CUT_NODE\n");

      new_node = init_node(nod->str, same_chars);
      tmp_node = malloc(sizeof(node));

      nod->node_lenght -= same_chars;
      nod->str_lenght -= same_chars;
      nod->str = memmove(nod->str, nod->str + same_chars,
                         sizeof(char) * (nod->str_lenght));
      nod->str = realloc(nod->str, sizeof(char) * (nod->str_lenght));
      nod->str[same_chars] = '\0';
      tmp_node = memcpy(tmp_node, new_node, sizeof(node));
      new_node = memcpy(new_node, nod, sizeof(node));
      nod = memcpy(nod, tmp_node, sizeof(node));
      nod->next[0] = new_node;
      nod->connected_nodes++;

      SET_BIT(nod->mask, alph[(uint8_t)*new_node->str]);
      free(tmp_node);

      nod->next = realloc(nod->next, sizeof(node) * (nod->connected_nodes + 1));
      new_next_index = get_index(new_word[i], nod->mask);

      if (new_next_index < nod->connected_nodes)
        for (size_t j = nod->connected_nodes; j > new_next_index; j--)
          nod->next[j] = nod->next[j - 1];

      nod->next[new_next_index] = init_node(new_word + i, words_lenght - i);
      SET_BIT(nod->mask, alph[(uint8_t) * (new_word + i)]);
      nod->connected_nodes++;

      state = NOTHING_TO_DO;
      break;
    case MAKE_INBETWEEN_NODE:
//       printf("MAKE_INBETWEEN_NODE\n");
      new_node = init_node(nod->str, same_chars);
      tmp_node = malloc(sizeof(node));
      nod->node_lenght -= same_chars;
      nod->str_lenght -= same_chars;

      nod->str = memmove(nod->str, nod->str + same_chars,
                         sizeof(char) * (nod->str_lenght));
      nod->str = realloc(nod->str, sizeof(char) * (nod->str_lenght));
      nod->str[same_chars] = '\0';
      tmp_node = memcpy(tmp_node, new_node, sizeof(node));
      new_node = memcpy(new_node, nod, sizeof(node));
      nod = memcpy(nod, tmp_node, sizeof(node));
      nod->next[0] = new_node;
      nod->connected_nodes++;

      SET_BIT(nod->mask, alph[(uint8_t)*new_node->str]);
      free(tmp_node);
      state = INIT_INLINE_WITH_NEW;
      continue;
    case INIT_INLINE_WITH_NEW:
//       printf("INIT_INLINE_WITH_NEW\n");
      // 3 = 1 per \0 dopo parola, 1 per # e 1 per ultimo \0
      // nel calcolo same_chars si cancella
      nod->str_lenght = nod->node_lenght + 3 + words_lenght - i;
      nod->str = realloc(nod->str, sizeof(char) * (nod->str_lenght));
      memcpy(nod->str + nod->node_lenght + 2, new_word + i,
             sizeof(char) * (words_lenght - i));

      nod->str[nod->node_lenght + 1] = '#';
      nod->str[nod->str_lenght] = '\0';

      state = NOTHING_TO_DO;
      break;
    case CUT_FROM_INLINE:
//       printf("CUT_FROM_INLINE\n");
      new_nodeLen = 0;
      // il +1 è per skippare il #
      while (nod->str[inline_index + new_nodeLen + 1] == new_word[i + new_nodeLen])
        new_nodeLen++;
      new_node = init_node(new_word + i, new_nodeLen);
      new_inlined_lenght = words_lenght - new_nodeLen - i;

      new_node->str_lenght = new_node->node_lenght + 3 + new_inlined_lenght;
      new_node->str = realloc(new_node->str, sizeof(char) * (new_node->str_lenght));
      memcpy(new_node->str + new_node->node_lenght + 2,
             nod->str + inline_index + 1 + new_nodeLen,
             sizeof(char) * (words_lenght - i));

      new_node->str[new_node->node_lenght + 1] = '#';
      new_node->str[new_node->str_lenght - 1] = '\0';
      nod->next = realloc(nod->next, sizeof(node) * (nod->connected_nodes + 1));

      new_next_index = get_index(*new_node->str, nod->mask);
      if (new_next_index < nod->connected_nodes)
        for (size_t j = nod->connected_nodes; j > new_next_index; j--)
          nod->next[j] = nod->next[j - 1];
      nod->connected_nodes++;

      nod->next[new_next_index] = new_node;
      SET_BIT(nod->mask, alph[(uint8_t)*new_node->str]);

      new_inlined_lenght += new_nodeLen;
      for (uint16_t j = inline_index; j < nod->str_lenght - new_inlined_lenght - 1;
           j += 1 + nod->node_lenght) {
        memcpy(nod->str + j, nod->str + j + new_inlined_lenght + 1,
               sizeof(char) * (new_inlined_lenght + 1));
      }

      nod->str_lenght -= 1 + new_inlined_lenght;
      nod->str = realloc(nod->str, sizeof(char) * (nod->str_lenght));
      nod->str[nod->str_lenght - 1] = '\0';
      i += new_nodeLen;

      nod = new_node;
      state = ADD_TO_INLINE;
      break;
    }
  }
  nod = tmp;
  words_count++;
}

void __stampa_filtrate(node *nod, size_t depth, char *working_str) {
  if (nod->deleted == 0 && depth == words_lenght) {
    working_str[words_lenght] = '\0';
    printf("%s\n", working_str);
    return;
  }

  bool finished = false;
  size_t i = 0, j = nod->node_lenght + 1;
  while (!finished) {
    char *tmp = working_str;

    if (j + 1 < nod->str_lenght && i < nod->connected_nodes) {
      if (nod->next[i]->deleted > 0) {
        i++;
        continue;
      }
      if (*(nod->str + j) == '|') {
        j += 1 + (words_lenght - depth);
        continue;
      }
      if (*(nod->next[i]->str) < *(nod->str + j + 1)) {
        memcpy(tmp + depth, nod->next[i]->str, nod->next[i]->node_lenght);
        __stampa_filtrate(nod->next[i], depth + nod->next[i]->node_lenght, working_str);
        i++;
      } else {
        memcpy(tmp + depth, nod->str + j + 1, words_lenght - depth);
        __stampa_filtrate(nod, words_lenght, working_str);
        j += 1 + (words_lenght - depth);
      }
    } else if (!(j + 1 < nod->str_lenght) && i < nod->connected_nodes) {
      if (nod->next[i]->deleted > 0) {
        i++;
        continue;
      }

      memcpy(tmp + depth, nod->next[i]->str, nod->next[i]->node_lenght);
      __stampa_filtrate(nod->next[i], depth + nod->next[i]->node_lenght, working_str);
      i++;
    } else if ((j + 1 < nod->str_lenght && !(i < nod->connected_nodes))) {
      if (*(nod->str + j) == '|') {
        j += 1 + (words_lenght - depth);
        continue;
      }
      memcpy(tmp + depth, nod->str + j + 1, words_lenght - depth);
      __stampa_filtrate(nod, words_lenght, working_str);
      j += 1 + (words_lenght - depth);
    } else
      finished = true;
  }
}

void stampa_filtrate(node *nod) {
  char *working_str = malloc(sizeof(char) * (words_lenght + 1));
  __stampa_filtrate(nod, 0, working_str);
  free(working_str);
}

bool in_tree(node *nod, char *needle) {
  node *tmp = nod;
  for (size_t i = 0; i < words_lenght; i++) {
    if ((GET_BIT(tmp->mask, alph[(uint8_t)*needle])) == 0) {
      return false;
    }
    tmp = tmp->next[get_index(*needle, tmp->mask)];
    size_t len = tmp->node_lenght;
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

void compute_res(char *r, char *p, char *res) {
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
  bool is_same_str = true;
  for (size_t i = 0; filter[i] != '\0'; ++i) {
    if (p[i] != r[i])
      is_same_str = false;

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

  return !is_same_str;
}

void print_tree(node *nod, int depth) {
  int cont = 0;
  if (nod->str[0] == '#')
    cont = 0;
  printf("%*s%s ", depth, "", nod->str);
  if (nod->node_lenght != nod->str_lenght)
    printf("%s ", nod->str + nod->node_lenght + 1);

  if (nod->connected_nodes != 0)
    printf("len -> %d strLen = %d connected -> %d\n", nod->node_lenght, nod->str_lenght,
           nod->connected_nodes);
  else {
    cont++;
    printf("len -> %d %d\n", nod->node_lenght, cont);
  }

  for (uint32_t i = 0; i < nod->connected_nodes; i++)
    print_tree(nod->next[i], depth + nod->node_lenght);
}

void __remove_incompatibile(char *filter, node *nod, size_t depth, char *str,
                            char *working_str) {
  if (nod->deleted == 0 && depth == words_lenght) {
    working_str[depth + 1] = '\0';

#ifdef DEBUG
    printf("----compatibility test----\n");
    printf("%s - %s - %s\n", filter, working_str, str);
#endif
    if (!compatible(filter, working_str, str)) {
#ifdef DEBUG
      printf("incompatibile\n");
      printf("--------------------------\n");
#endif
      words_count--;
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

  size_t deleted_count = 0;

  for (size_t i = 0; i < nod->connected_nodes; i++) {
    node *next = nod->next[i];
    deleted_count += next->deleted;
    if (next->deleted == 1)
      continue;

    size_t len = next->str_lenght;
    char *tmp = working_str;
    tmp += depth;
    memcpy(tmp, next->str, len);

    __remove_incompatibile(filter, next, depth + len, str, working_str);
  }
  if (deleted_count == nod->connected_nodes && nod->connected_nodes != 0) {
    nod->deleted = 1;
  }
  return;
}

void remove_incompatibile(char *filter, node *nod, char *str) {
  char *working_str = malloc(sizeof(char) * (words_lenght + 1));
  __remove_incompatibile(filter, nod, 0, str, working_str);
  free(working_str);
}

void reset_deleted_nodes(node *nod) {
  nod->deleted = 0;
  for (size_t i = 0; i < nod->connected_nodes; i++) {
    reset_deleted_nodes(nod->next[i]);
  }
  if (has_inline(nod)){
    for (size_t i = nod->node_lenght + 2; i < nod->str_lenght; i++)
      *nod->str = '#';
  }
}

int main() {
  char *line;
  no = malloc(sizeof(uint16_t) * ALPHALEN);
  co = malloc(sizeof(uint16_t) * ALPHALEN);
  sc = malloc(sizeof(uint16_t) * words_lenght);
  INIT_LINE_BUFFER();
  NEW_LINE(line);

  words_lenght = strtol(line, NULL, 10);
  node *words = init_node("#", 1);
  for (size_t i = 0; i < ALPHALEN; i++) {
    no[i] = 0;
    co[i] = 0;
    if (i < words_lenght)
      sc[i] = 0;
  }
  line = malloc(sizeof(char) * words_lenght);
  NEW_LINE(line);
#ifdef DEBUG
  printf("---Input check------------\n");
#endif
  while (!feof(stdin)) {
    if (parse_command(line) == INIZIO) {
      break;
    }
#ifdef DEBUG
    printf("----%s---------------\n", line);
#endif
    add_word(words, line);
    NEW_LINE(line);
  }

  size_t words_count_bkp = words_count;
#ifdef DEBUG
  printf("--------------------------\n");
#endif
//   print_tree(words, 0);
#ifdef DEBUG
  printf("---Initial words----------\n");
  stampa_filtrate(words);
  printf("--------------------------\n");
#endif
  char *reference_word = malloc(sizeof(char) * words_lenght);
  NEW_LINE(reference_word);
#ifdef DEBUG
  printf("---Reference word---------\n");
  printf("%s\n", reference_word);
  printf("--------------------------\n");
#endif
  NEW_LINE(line);
  size_t num_of_guesses = (size_t)strtol(line, NULL, 10);
#ifdef DEBUG
  printf("---guesses number---------\n");
  printf("%ld\n", num_of_guesses);
  printf("--------------------------\n");
#endif

  bool has_won = false;
  bool finished = false;
  char *res = malloc(sizeof(char) * words_lenght);
  szArr *saved_results = malloc(sizeof(szArr));
  szArr *saved_guesses = malloc(sizeof(szArr));

  saved_results->len = 0;
  saved_guesses->len = 0;
  saved_results->v = malloc(sizeof(char *) * num_of_guesses);
  saved_guesses->v = malloc(sizeof(char *) * num_of_guesses);
  for (size_t i = 0; i < num_of_guesses; i++) {
    saved_results->v[i] = malloc(sizeof(char) * words_lenght);
    saved_guesses->v[i] = malloc(sizeof(char) * words_lenght);
  }

  NEW_LINE(line);
  while (!feof(stdin)) {
    for (size_t i = 0; num_of_guesses > 0; ++i) {
#ifdef DEBUG
      printf("---Input------------------\n");
      printf("%s\n", line);
      printf("--------------------------\n");
#endif
      if (parse_command(line) == PRINT) {
        stampa_filtrate(words);
        NEW_LINE(line);
        continue;

      } else if (parse_command(line) == INS_INI) {
#ifdef DEBUG
        printf("---Input------------------\n");
#endif
        NEW_LINE(line);
        while (parse_command(line) != INS_FIN) {
#ifdef DEBUG
          printf("%s\n", line);
#endif
          add_word(words, line);
          words_count_bkp++;
          NEW_LINE(line);
        }
#ifdef DEBUG
        printf("---------------------------\n");
        printf("---Fine--------------------\n");
#endif
        for (size_t j = 0; j < saved_results->len; j++) {
          remove_incompatibile(saved_results->v[j], words, saved_guesses->v[j]);
        }

        NEW_LINE(line);
        continue;

      } else if (strcmp(reference_word, line) == 0) {
        has_won = true;
        if (!feof(stdin)) {
          NEW_LINE(line);
        }
        num_of_guesses = 0;
        break;
      } else if (!in_tree(words, line)) {
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
      compute_res(reference_word, line, res);
      res[words_lenght] = '\0';
      saved_results->len++;
      strcpy(saved_results->v[saved_results->len - 1], res);
      saved_guesses->len++;
      strcpy(saved_guesses->v[saved_guesses->len - 1], line);

      remove_incompatibile(res, words, line);
#ifdef DEBUG
//       print_tree(words, 0);
#endif
      printf("%s\n%ld\n", res, words_count);
      num_of_guesses--;
      NEW_LINE(line);
    }
    if (!finished) {
      printf("%s\n", has_won ? "ok" : "ko");
      has_won = false;
      finished = true;
    }

    if (parse_command(line) == INS_INI) {
#ifdef DEBUG
      printf("---Input------------------\n");
#endif
      NEW_LINE(line);
      while (parse_command(line) != INS_FIN) {
#ifdef DEBUG
        printf("%s\n", line);
#endif
        // does't matter if it's compatibile or not because it's going to be
        // reset after
        add_word(words, line);
        words_count_bkp++;
        NEW_LINE(line);
      }
#ifdef DEBUG
      printf("---------------------------\n");
      printf("---Fine--------------------\n");
#endif

      for (size_t j = 0; j < saved_results->len; j++) {
        remove_incompatibile(saved_results->v[j], words, saved_guesses->v[j]);
      }

      NEW_LINE(line);
      continue;
    }
    if (parse_command(line) == INIZIO) {
      reset_deleted_nodes(words);
#ifdef DEBUG
      printf("---Initial words----------\n");
      printf("%ld\n", words_count);
      stampa_filtrate(words);
      printf("--------------------------\n");
#endif
      NEW_LINE(reference_word);
#ifdef DEBUG
      printf("---Reference word---------\n");
      printf("%s\n", reference_word);
      printf("--------------------------\n");
#endif
      NEW_LINE(line);
      num_of_guesses = (size_t)strtol(line, NULL, 10);
#ifdef DEBUG
      printf("---guesses number---------\n");
      printf("%ld\n", num_of_guesses);
      printf("--------------------------\n");
#endif
      if (saved_results->len < num_of_guesses) {
        saved_results->v = realloc(saved_results->v, sizeof(char *) * num_of_guesses);
        saved_guesses->v =
            realloc(saved_guesses->v, sizeof(char *) * num_of_guesses);
        for (size_t i = saved_results->len; i < num_of_guesses; i++) {
          saved_results->v[i] = malloc(sizeof(char) * words_lenght);
          saved_guesses->v[i] = malloc(sizeof(char) * words_lenght);
        }
      }
      saved_results->len = 0;
      saved_guesses->len = 0;
      finished = false;
      words_count = words_count_bkp;
    }
    NEW_LINE(line);
  }
  // TODO Free everything
  return 0;
}
