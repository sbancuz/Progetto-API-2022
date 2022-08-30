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
  input_string_lenght = getline(&a, &len, stdin);                              \
  a[input_string_lenght - 1] = '\0'

typedef struct node_s {
  char *str;
  uint16_t str_lenght;
  uint16_t node_lenght;
  uint8_t connected_nodes;
  uint8_t deleted;
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

uint16_t *no;
uint16_t *co;
uint16_t *ns;
uint16_t *sc;

void *init_node(char *c, uint32_t len) {
  node *n = calloc(1, sizeof(node));
  n->str = malloc(sizeof(char) * len + 1);
  n->str = memcpy(n->str, c, sizeof(char) * len);
  n->str[len] = '\0';
  n->str_lenght = len + 1;
  n->node_lenght = len;
  n->next = malloc(sizeof(node));
  return (void *)n;
}

inline static size_t get_index(node *nod, char c) {
  size_t i = 0;
  for (i = 0; i < nod->connected_nodes; i++) {
    if (nod->next[i]->str[0] >= c)
      return i;
  }
  return nod->connected_nodes;
}

inline bool has_char(node *nod, char c) {
  for (size_t i = 0; i < nod->connected_nodes; i++)
    if (nod->next[i]->str[0] == c)
      return true;
  return false;
}

inline size_t get_inlined_index(node *nod, char c, size_t inline_lenght) {
  for (size_t i = nod->node_lenght + 1; i + 1 < nod->str_lenght;
       i += 1 + inline_lenght) {
    if (*(nod->str + i + 1) == c)
      return i;
  }
  return 0;
}

void new_node_from_root(node *nod, char *new_word) {
#ifdef DEBUG
  printf("NEW_NODE_FROM_ROOT\n");
#endif
  size_t new_next_index;

  nod->next = realloc(nod->next, sizeof(node) * (nod->connected_nodes + 1));
  new_next_index = get_index(nod, new_word[0]);
  if (new_next_index < nod->connected_nodes)
    for (size_t j = nod->connected_nodes; j > new_next_index; j--)
      nod->next[j] = nod->next[j - 1];

  nod->next[new_next_index] = init_node((new_word), words_lenght);
  nod->connected_nodes++;
}

void add_to_inline(node *nod, char *new_word, size_t i) {
  uint16_t new_index_in_inline = -1;
  nod->str_lenght += words_lenght - i + 1;

  nod->str = realloc(nod->str, sizeof(char) * (nod->str_lenght));
  size_t new_inlined_lenght = words_lenght - i;

  // slide to the right to order
  for (uint16_t j = nod->node_lenght + 1;
       j < nod->str_lenght - new_inlined_lenght - 1;
       j += new_inlined_lenght + 1) {
    if (nod->str[j + 1] > new_word[i]) {
      new_index_in_inline = j;
      break;
    }
  }
  if (new_index_in_inline == (uint16_t)-1)
    new_index_in_inline = nod->str_lenght - new_inlined_lenght - 2;

  memcpy(nod->str + new_index_in_inline + new_inlined_lenght + 1,
         nod->str + new_index_in_inline,
         sizeof(char) *
             (nod->str_lenght - new_index_in_inline - new_inlined_lenght - 1));
  nod->str[new_index_in_inline] = '#';

  memcpy(nod->str + new_index_in_inline + 1, new_word + i,
         sizeof(char) * (new_inlined_lenght));
  nod->str[nod->str_lenght - 1] = '\0';
}

void init_inline(node *nod, char *new_word, size_t same_chars, size_t i) {
#ifdef DEBUG
  printf("INIT_INLINE\n");
#endif
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

  add_to_inline(nod, new_word, i);
}

void init_inline_with_new(node *nod, char *new_word, size_t i) {
#ifdef DEBUG
  printf("INIT_INLINE_WITH_NEW\n");
#endif
  // 3 = 1 per \0 dopo parola, 1 per # e 1 per ultimo \0
  // nel calcolo same_chars si cancella
  nod->str_lenght = nod->node_lenght + 3 + words_lenght - i;
  nod->str = realloc(nod->str, sizeof(char) * (nod->str_lenght));
  memcpy(nod->str + nod->node_lenght + 2, new_word + i,
         sizeof(char) * (words_lenght - i));

  nod->str[nod->node_lenght + 1] = '#';
  nod->str[nod->str_lenght] = '\0';
}

void cut_node(node *nod, char *new_word, size_t same_chars, size_t i) {
#ifdef DEBUG
  printf("CUT_NODE\n");
#endif
  node *new_node = init_node(nod->str, same_chars);
  node *tmp_node = malloc(sizeof(node));

  nod->node_lenght -= same_chars;
  nod->str_lenght -= same_chars;
  nod->str = memmove(nod->str, nod->str + same_chars,
                     sizeof(char) * (nod->str_lenght));
  nod->str = realloc(nod->str, sizeof(char) * (nod->str_lenght));

  nod->str[nod->node_lenght] = '\0';
  tmp_node = memcpy(tmp_node, new_node, sizeof(node));
  new_node = memcpy(new_node, nod, sizeof(node));
  nod = memcpy(nod, tmp_node, sizeof(node));
  nod->next[0] = new_node;
  nod->connected_nodes++;

  free(tmp_node);

  nod->next = realloc(nod->next, sizeof(node) * (nod->connected_nodes + 1));
  uint16_t new_next_index = get_index(nod, new_word[i]);

  if (new_next_index < nod->connected_nodes)
    for (size_t j = nod->connected_nodes; j > new_next_index; j--)
      nod->next[j] = nod->next[j - 1];

  nod->next[new_next_index] = init_node(new_word + i, words_lenght - i);
  nod->connected_nodes++;
}

void make_inbetween_node(node *nod, char *new_word, size_t same_chars,
                         size_t i) {
#ifdef DEBUG
  printf("MAKE_INBETWEEN_NODE\n");
#endif
  node *new_node = init_node(nod->str, same_chars);
  node *tmp_node = malloc(sizeof(node));
  nod->node_lenght -= same_chars;
  nod->str_lenght -= same_chars;

  nod->str = memmove(nod->str, nod->str + same_chars,
                     sizeof(char) * (nod->str_lenght));

  nod->str = realloc(nod->str, sizeof(char) * (nod->str_lenght));
  nod->str[nod->node_lenght] = '\0';
  tmp_node = memcpy(tmp_node, new_node, sizeof(node));
  new_node = memcpy(new_node, nod, sizeof(node));
  nod = memcpy(nod, tmp_node, sizeof(node));
  nod->next[0] = new_node;
  nod->connected_nodes++;
  free(tmp_node);

  init_inline_with_new(nod, new_word, i);
}

void cut_from_inline(node *nod, char *new_word, size_t inline_index, size_t i) {
#ifdef DEBUG
  printf("CUT_FROM_INLINE\n");
#endif
  size_t new_node_lenght = 0;

  // il +1 è per skippare il #
  while (nod->str[inline_index + new_node_lenght + 1] ==
         new_word[i + new_node_lenght])
    new_node_lenght++;
  node *new_node = init_node(new_word + i, new_node_lenght);
  size_t new_inlined_lenght = words_lenght - new_node_lenght - i;

  new_node->str_lenght = new_node->node_lenght + 3 + new_inlined_lenght;
  new_node->str = realloc(new_node->str, sizeof(char) * (new_node->str_lenght));

  memcpy(new_node->str + new_node->node_lenght + 2,
         nod->str + inline_index + 1 + new_node_lenght,
         sizeof(char) * (new_inlined_lenght));

  new_node->str[new_node->node_lenght + 1] = nod->str[inline_index];
  new_node->str[new_node->str_lenght - 1] = '\0';
  nod->next = realloc(nod->next, sizeof(node) * (nod->connected_nodes + 1));
  size_t new_next_index = get_index(nod, *new_node->str);
  if (new_next_index < nod->connected_nodes)
    for (size_t j = nod->connected_nodes; j > new_next_index; j--)
      nod->next[j] = nod->next[j - 1];
  nod->connected_nodes++;

  nod->next[new_next_index] = new_node;

  new_inlined_lenght += new_node_lenght;

  if (inline_index < nod->str_lenght - new_inlined_lenght) {
    memmove(nod->str + inline_index,
            nod->str + inline_index + new_inlined_lenght + 1,
            sizeof(char) * (nod->str_lenght - inline_index));
  }
  nod->str_lenght -= 1 + new_inlined_lenght;
  nod->str = realloc(nod->str, sizeof(char) * (nod->str_lenght));
  nod->str[nod->str_lenght - 1] = '\0';
  i += new_node_lenght;
  nod = new_node;

  add_to_inline(nod, new_word, i);
}

#define has_inline(x) (x->node_lenght + 1 != x->str_lenght)
void add_word(node *nod, char *new_word) {
  node *tmp = nod;
  size_t inline_index = 0;
  uint32_t i = 0, same_chars = 0;
  if (!has_char(nod, *new_word)) {
    new_node_from_root(nod, new_word);
  } else {
    nod = nod->next[get_index(nod, *new_word)];
    for (i = 0; i < words_lenght; i++) {
      nod->deleted = 0;

      if (nod->str[same_chars] == new_word[i]) {
        same_chars++;
        continue;
      }
      if (nod->str[same_chars] == '\0') {
        same_chars = 0;
        if (has_char(nod, new_word[i])) {
          nod = nod->next[get_index(nod, *(new_word + i))];
          nod->deleted = 0;
          if (nod->node_lenght == 1) {
            i--;
            continue;
          }
          while (nod->str[same_chars] == new_word[same_chars + i])
            same_chars++;
          i += same_chars;

          if (has_char(nod, new_word[i])) {
            i--;
            continue;
          }

          if (same_chars != nod->node_lenght) {
            // se inline_lenght nuova è diversa dalla vecchia allora faccio
            // in-between
            // onestamente non so bene perché funzioni sta cosa, ma
            // words_lenght - i si semplificava da tutte e due le parti
            if (same_chars - nod->node_lenght != 0) {
              make_inbetween_node(nod, new_word, same_chars, i);
              break;

            } else if ((inline_index = get_inlined_index(
                            nod, *(new_word + i), words_lenght - i)) != 0) {
              cut_from_inline(nod, new_word, same_chars, i);
              break;
            } else {
              init_inline(nod, new_word, same_chars, i);
              break;
            }
          } else if ((inline_index = get_inlined_index(
                          nod, *(new_word + i), words_lenght - i)) != 0) {
            cut_from_inline(nod, new_word, inline_index, i);
            break;
          } else {
            add_to_inline(nod, new_word, i);
            break;
          }
        } else if ((inline_index = get_inlined_index(nod, *(new_word + i),
                                                     words_lenght - i)) != 0) {
          cut_from_inline(nod, new_word, inline_index, i);
          break;
        } else {
          if (has_inline(nod))
            add_to_inline(nod, new_word, i);
          else
            init_inline_with_new(nod, new_word, i);
          break;
        }
        same_chars = 0;
      } else {
        if (!has_inline(nod)) {
          init_inline(nod, new_word, same_chars, i);
          break;
        } else {
          make_inbetween_node(nod, new_word, same_chars, i);
          break;
        }
      }
    }
  }

  // in case of some bug too annoying to catch
  nod->str[nod->str_lenght - 1] = '\0';
  nod = tmp;
  words_count++;
}
size_t conta = 0;
void __stampa_filtrate(node *nod, size_t depth, char *working_str) {
  if (nod->deleted == 0 && depth == words_lenght) {
    working_str[words_lenght] = '\0';
    printf("%s\n", working_str);
    conta++;
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
        __stampa_filtrate(nod->next[i], depth + nod->next[i]->node_lenght,
                          working_str);
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
      __stampa_filtrate(nod->next[i], depth + nod->next[i]->node_lenght,
                        working_str);
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
  conta = 0;
  __stampa_filtrate(nod, 0, working_str);
  free(working_str);
}

bool in_tree(node *nod, char *needle) {
  node *tmp = nod;
  size_t inline_index = 0;
  for (size_t i = 0; i < words_lenght; i++) {
    //     printf("%s %d %d\n", tmp->str, tmp->node_lenght, tmp->str_lenght);
    if (has_inline(tmp)) {
      //       printf("%c\n", *needle);
      if ((inline_index = get_inlined_index(tmp, *needle, words_lenght - i)) !=
          0) {
        bool same = true;
        for (size_t j = inline_index + 1;
             j < tmp->str_lenght && *(tmp->str + j) != '#' &&
             *(tmp->str + j) != '|';
             j++) {
          if (tmp->str[j] != needle[j - inline_index - 1]) {
            same = false;
            break;
          }
        }
        if (same) {
          return true;
        }
      }
    }

    if (!has_char(tmp, *needle)) {
      return false;
    }
    tmp = tmp->next[get_index(tmp, *needle)];
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

    if (filter[i] == '|') {
      if ((r[i] == p[i] || no[(uint8_t)p[i]] == 0 ||
           sc[i] >= no[(uint8_t)p[i]] - co[(uint8_t)p[i]]))
        return false;
      else
        continue;
    }

    if (filter[i] == '/' && (sc[i] < no[(uint8_t)p[i]] - co[(uint8_t)p[i]] ||
                             (no[(uint8_t)p[i]] != 0 && r[i] == p[i])))
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
    printf("len -> %d strLen = %d connected -> %d deleted -> %hd \n",
           nod->node_lenght, nod->str_lenght, nod->connected_nodes,
           nod->deleted);
  else {
    cont++;
    printf("len -> %d deleted -> %hd %d strlen = %d depth = %d\n",
           nod->node_lenght, nod->deleted, cont, nod->str_lenght, depth);
  }

  for (uint32_t i = 0; i < nod->connected_nodes; i++)
    print_tree(nod->next[i], depth + nod->node_lenght);
}

void delete_tree(node *nod) {
  if (nod->deleted == 1)
    return;

  nod->deleted = 1;
  if (has_inline(nod)) {
    size_t inline_lenght = 1;
    for (size_t i = nod->node_lenght + 2;
         i < nod->str_lenght && *(nod->str + i) != '#' &&
         *(nod->str + i) != '|';
         i++)
      inline_lenght++;

    for (size_t i = nod->node_lenght + 1; i < nod->str_lenght;
         i += inline_lenght) {
      if (nod->str[i] == '#') {
        nod->str[i] = '|';
        words_count--;
      }
    }
  } else if (nod->connected_nodes == 0) {
    words_count--;
  }

  for (uint32_t i = 0; i < nod->connected_nodes; i++)
    delete_tree(nod->next[i]);
}

size_t cont_tree(node *nod) {
  if (nod->deleted == 1)
    return 0;

  size_t cont = 0;
  if (has_inline(nod)) {
    size_t inline_lenght = 1;
    for (size_t i = nod->node_lenght + 2;
         i < nod->str_lenght && *(nod->str + i) != '#' &&
         *(nod->str + i) != '|';
         i++)
      inline_lenght++;

    for (size_t i = nod->node_lenght + 1; i < nod->str_lenght;
         i += inline_lenght) {
      if (nod->str[i] == '#') {
        cont++;
      }
    }
  } else if (nod->connected_nodes == 0) {
    return cont + 1;
  }

  for (uint32_t i = 0; i < nod->connected_nodes; i++)
    cont += cont_tree(nod->next[i]);

  return cont;
}

size_t __remove_incompatibile(char *filter, node *nod, size_t depth, char *str,
                              char *working_str, int inline_index) {
  if ((nod->deleted == 0 || inline_index != -1) && depth == words_lenght) {
    working_str[depth] = '\0';

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
      if (inline_index < 0) {
        nod->deleted = 1;
      } else
        nod->str[inline_index] = '|';

      return 1;
    }
#ifdef DEBUG
    else {
      printf("compatibile\n");
      printf("--------------------------\n");
    }
#endif
    return 0;
  }

  if (nod->str[0] != '#') {
    // preemptive check to skip a lot of stuff
    //     printf("-->%ld\n", depth - 1);
    for (size_t d = depth - nod->node_lenght, l = 0; l < nod->node_lenght;
         d++, l++) {
      if ((filter[d] == '+' && nod->str[l] != str[d]) ||
          (filter[d] == '|' && nod->str[l] == str[d])) {
        // set everything to deleted

        delete_tree(nod);
        return 1;
      }
    }
  }
  size_t deleted_count = 0;
  bool finished = false;
  size_t i = 0, j = nod->node_lenght + 1, next_lenght;
  node *next = nod;

  while (!finished) {
    char *tmp = working_str;
    inline_index = -1;
    next_lenght = 0;

    // get next working string
    if (j + 1 < nod->str_lenght && i < nod->connected_nodes) {
      next = nod->next[i];
      if (next->deleted > 0) {
        i++;
        deleted_count++;
        continue;
      }
      if (*(nod->str + j) == '|') {
        j += 1 + (words_lenght - depth);
        deleted_count++;
        continue;
      }
      if (*(next->str) < *(nod->str + j + 1)) {
        memcpy(tmp + depth, next->str, next->node_lenght);
        next_lenght = depth + next->node_lenght;
        i++;
      } else {
        memcpy(tmp + depth, nod->str + j + 1, words_lenght - depth);
        inline_index = j;
        next_lenght = words_lenght;
        next = nod;

        j += 1 + (words_lenght - depth);
      }
    } else if (!(j + 1 < nod->str_lenght) && i < nod->connected_nodes) {
      next = nod->next[i];
      if (next->deleted > 0) {
        i++;
        deleted_count++;
        continue;
      }

      next_lenght = depth + next->node_lenght;
      memcpy(tmp + depth, next->str, next->node_lenght);

      i++;
    } else if ((j + 1 < nod->str_lenght && !(i < nod->connected_nodes))) {
      if (*(nod->str + j) == '|') {
        j += 1 + (words_lenght - depth);
        deleted_count++;
        continue;
      }
      memcpy(tmp + depth, nod->str + j + 1, words_lenght - depth);
      inline_index = j;
      next_lenght = words_lenght;
      next = nod;

      j += 1 + (words_lenght - depth);
    } else {
      finished = true;
      break;
    }

    //     printf("-------------------\n");
    //     for (size_t d = depth; d < next_lenght; d++)
    //       printf("%c", working_str[d]);
    //     printf(" %ld\n", depth);
    //     printf("-------------------\n");

    // setup counters for compatibility check
    for (size_t d = depth; d < next_lenght; d++) {
      sc[d] = ns[(uint8_t)str[d]] - co[(uint8_t)str[d]];
      no[(uint8_t)working_str[d]]++;
      ns[(uint8_t)str[d]]++;
      if (str[d] == working_str[d])
        co[(uint8_t)working_str[d]]++;
    }

    deleted_count += __remove_incompatibile(filter, next, next_lenght, str,
                                            working_str, inline_index);

    // undo this step counters
    for (size_t d = depth; d < next_lenght; d++) {
      no[(uint8_t)working_str[d]]--;
      ns[(uint8_t)str[d]]--;

      if (str[d] == working_str[d])
        co[(uint8_t)working_str[d]]--;
      sc[d] = 0;
    }
  }

  nod->deleted =
      deleted_count ==
      (nod->connected_nodes +
       ((nod->str_lenght - nod->node_lenght - 2) / (words_lenght - depth + 1)));
  return nod->deleted;
}

void remove_incompatibile(char *filter, node *nod, char *str) {
  char *working_str = malloc(sizeof(char) * (words_lenght + 1));
  __remove_incompatibile(filter, nod, 0, str, working_str, -1);
  free(working_str);
}

void reset_deleted_nodes(node *nod) {
  nod->deleted = 0;
  for (size_t i = 0; i < nod->connected_nodes; i++) {
    reset_deleted_nodes(nod->next[i]);
  }
  if (has_inline(nod)) {
    size_t inline_lenght = 1;
    for (size_t i = nod->node_lenght + 2;
         i < nod->str_lenght && *(nod->str + i) != '#' &&
         *(nod->str + i) != '|';
         i++)
      inline_lenght++;

    for (size_t i = nod->node_lenght + 1; i + 1 < nod->str_lenght;
         i += inline_lenght) {
      nod->str[i] = '#';
    }
  }
}

int main() {
  char *line;
  no = calloc(UINT8_MAX, sizeof(uint16_t));
  co = calloc(UINT8_MAX, sizeof(uint16_t));
  ns = calloc(UINT8_MAX, sizeof(uint16_t));
  sc = calloc(words_lenght, sizeof(uint16_t));
  INIT_LINE_BUFFER();
  NEW_LINE(line);

  words_lenght = strtol(line, NULL, 10);
  node *words = init_node("#", 1);
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

    // il coso che crea i test è buggato, mette +inserisci prima della prima
    // nuova partita :^)
    if (line[0] != '+') {
      add_word(words, line);
    }
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
        printf("%ld\n", words_count);
#endif
        NEW_LINE(line);
        while (parse_command(line) != INS_FIN) {
#ifdef DEBUG
          printf("---------------------------\n");
          stampa_filtrate(words);
          printf("%s\n", line);
#endif
          add_word(words, line);
          words_count_bkp++;
          NEW_LINE(line);
        }
#ifdef DEBUG
        printf("---------------------------\n");
        stampa_filtrate(words);
        printf("---Fine--------------------\n");
#endif
        //                 printf("%ld\n", words_count);

        for (size_t j = 0; j < saved_results->len; j++) {
          //           printf("--->%ld", words_count);
          remove_incompatibile(saved_results->v[j], words, saved_guesses->v[j]);
          //           printf(" %s %s ", saved_results->v[j],
          //           saved_guesses->v[j]); printf("%ld\n", words_count);
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
        printf("-->%s\n", line);
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

      //       printf("----------------------------------\n");
      //       print_tree(words, 0);

      remove_incompatibile(res, words, line);

#ifdef DEBUG
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
    //     print_tree(words, 0);
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
        saved_results->v =
            realloc(saved_results->v, sizeof(char *) * num_of_guesses);
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
  //   print_tree(words, 0);
  // TODO Free everything
  return 0;
}
